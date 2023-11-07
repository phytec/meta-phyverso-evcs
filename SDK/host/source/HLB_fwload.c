/********************************************************************
*
* Module Name: HLB_fwload
* Design:
* Implement fwload api
*
********************************************************************/
/*******************************************************************
* IMPORTS
********************************************************************/
#include "HLB_fwload.h"
#include "osal.h"
#include "common.h"
#include "PROD_lib.h"
#include <stdlib.h>
#include <string.h>

/*******************************************************************
* CONSTANTS
********************************************************************/
#define MAX_CHUNK_SIZE              (BUFFER_PAYLOAD_LIMIT * 2U) /* must be divisible by 4 */
#define LZOZ_FILE_SIGNATURE         "LZOZ100"
#define DEFAULT_DEVICE_ID           (0U)
#define DEFAULT_DOMAIN_ID           (0U)
#define DEFAULT_LL_TX_IND_OFFSET    (50U)
#define MAX_TRIES_QUERY             (100U)
#define TIMEOUT_QUERY_MS            (50U) /* MAX_TRIES_QUERY * TIMEOUT_QUERY_MS = ~5 seconds timeout */

#define MAX_TRIES_RMII_CFG          (3)
#define TIMEOUT_RMII_CGF            (100U)

#define HOST_CLOCK_OUT_CONTROL_REG  (0x6103003CU)

/*******************************************************************
* TYPES
********************************************************************/
typedef struct
{
    uint32_t CP_addr;
    uint32_t BIN_FILE_HEADER_addr;
    uint32_t HW_VECTORS_addr;
    uint32_t GP_addr;
    uint32_t USER_CONFIG_addr;
} EBL_sections_target_addr_table_t;

typedef enum
{
    FW_BIN_SECTION_IDX_BIN_FILE_HEADER = 0,
    FW_BIN_SECTION_IDX_EBL_SPI_BCB,
    FW_BIN_SECTION_IDX_HW_VECTORS,
    FW_BIN_SECTION_IDX_GLOBAL_PARAMS,
    FW_BIN_SECTION_IDX_CPUS,

    /* keep last */
    FW_BIN_SECTION_IDX_LAST = FW_BIN_SECTION_IDX_CPUS,
} FW_bin_section_idx_t;

typedef struct
{
    uint32_t offset;
    uint32_t size;
    uint32_t checksum;
} FW_bin_section_info_t;

/*******************************************************************
* STATIC and GLOBAL DATA
********************************************************************/
/*******************************************************************
* MACROS
********************************************************************/
#define TWOS_COMPLEMENT(x) ((~(x)) + 1U)

#ifdef FW_LOAD_TIMING
#define MARK_TIMESTAMP(message) hlb_log_info("[%u]: %s", get_msectime(), message);
#else
#define MARK_TIMESTAMP(message)
#endif

/*******************************************************************
* INTERNAL FUNCTIONS
********************************************************************/
static RES_result_t HLB_exec_CP(HLB_legacy_comm_handler_t *handle, uint32_t exec_addr)
{
    RES_result_t res;

    res = HLB_execute_command(handle, exec_addr, 0, 0, (uint32_t)CPU_ID_CP);

    return res;
}

static void HBL_section_checksum_update(uint32_t *checksum,
                                        const uint8_t* data, uint32_t count)
{
    uint32_t chs = 0U;
    uint32_t tmp = 0U;
    uint32_t limit = (count / sizeof(uint32_t));
    uint32_t extrabytes;

    for (uint32_t i = 0U; i < limit * sizeof(uint32_t); i += sizeof(uint32_t))
    {
        COPY_UNALIGNED_LE_32_BIT_TO_HOST(tmp, data[i])
        chs += tmp;
    }

    extrabytes = (count & 0x03U);

    if (extrabytes != 0U)
    {
        tmp = 0;
        MEMCPY((uint8_t*)&tmp, &data[limit * sizeof(uint32_t)], extrabytes);
        chs += letohl(tmp);
    }

    chs += count;

    *checksum += chs;
}

static void HBL_section_checksum_finalize(uint32_t *checksum)
{
    *checksum = TWOS_COMPLEMENT(*checksum);
}

static RES_result_t HLB_send_data_in_chunks(HLB_legacy_comm_handler_t *handle,
                                            IO_access_read_func read_fn,
                                            void *callback_priv,
                                            uint32_t offset, uint32_t count,
                                            uint32_t target_addr,
                                            uint32_t expected_checksum,
                                            uint32_t curr_checksum)
{
    RES_result_t res = RES_RESULT_OK;
    uint8_t buffer[MAX_CHUNK_SIZE];
    uint32_t packet_size;
    /* MISRA.FUNC.MODIFIEDPAR.2012 */
    uint32_t tmp_curr_checksum = curr_checksum;
    uint32_t tmp_offset = offset;
    uint32_t tmp_count = count;
    uint32_t tmp_target_addr = target_addr;

    _Static_assert((sizeof(buffer) % sizeof(uint32_t)) == 0U, "ASSERT");

    while ((tmp_count > 0U) && (res == RES_RESULT_OK))
    {
        packet_size = MIN(tmp_count, sizeof(buffer));

        res = read_fn(callback_priv, buffer, tmp_offset, packet_size);
        LOG_IF_ERR(res, "Failed to read section data");
        if (res == RES_RESULT_OK)
        {
            if (expected_checksum != 0U)
            {
                HBL_section_checksum_update(&tmp_curr_checksum, buffer, packet_size);
            }

            res = HLB_write_to_mem(handle, tmp_target_addr, packet_size, buffer, TIMEOUT_MSEC);
            LOG_IF_ERR(res, "Set mem failed. offset=%u size=%u addr=0x%08X",
                       tmp_offset, packet_size, tmp_target_addr);
            if (res == RES_RESULT_OK)
            {
                tmp_count -= packet_size;
                tmp_offset += packet_size;
                tmp_target_addr += packet_size;
            }
        }
    }

    if ((res == RES_RESULT_OK) && (expected_checksum != 0U))
    {
        HBL_section_checksum_finalize(&tmp_curr_checksum);
        if (tmp_curr_checksum != expected_checksum)
        {
            res = RES_RESULT_GENERAL_ERROR;
            LOG_IF_ERR(res, "checksum missmatch 0x%08X != 0x%08X", tmp_curr_checksum, expected_checksum);
        }
    }

    return res;
}

static RES_result_t HLB_load_user_config_bin(HLB_legacy_comm_handler_t *handle,
                                             IO_access_read_func user_config_bin_file_read,
                                             void *callback_priv,
                                             const EBL_sections_target_addr_table_t *ebl_target_offset_table_ptr)
{
    RES_result_t res;

    res = HLB_send_data_in_chunks(handle, user_config_bin_file_read, callback_priv,
                                  0, sizeof(PROD_configuration_content_packed_t),
                                  ebl_target_offset_table_ptr->USER_CONFIG_addr,
                                  0, 0);

    return res;
}

static RES_result_t HLB_load_hw_vectors(HLB_legacy_comm_handler_t *handle,
                                        IO_access_read_func fw_bin_file_read,
                                        void *callback_priv,
                                        const FW_bin_section_info_t *fw_bin_sections_info_ptr,
                                        const EBL_sections_target_addr_table_t *ebl_target_offset_table_ptr)
{
    RES_result_t res;
    uint8_t signature[sizeof(LZOZ_FILE_SIGNATURE)];
    uint32_t target_offset;
    bool compressed;

    res = fw_bin_file_read(callback_priv, signature,
                           fw_bin_sections_info_ptr[FW_BIN_SECTION_IDX_HW_VECTORS].offset,
                           sizeof(LZOZ_FILE_SIGNATURE) - 1U);
    LOG_IF_ERR(res, "Failed to hw vectors section file signature");

    if (res == RES_RESULT_OK)
    {
        if (memcmp(signature, (const uint8_t*)LZOZ_FILE_SIGNATURE, sizeof(LZOZ_FILE_SIGNATURE) - 1U) == 0)
        {
            /* HW vectors section is compressed inside the FW bin, load it to user config target address
             * and instruct the CG FW to decompress it from there to the HW vect section target address. */
            compressed = true;
            target_offset = ebl_target_offset_table_ptr->USER_CONFIG_addr;
        }
        else
        {
            target_offset = ebl_target_offset_table_ptr->HW_VECTORS_addr;
            compressed = false;
        }

        res = HLB_send_data_in_chunks(handle, fw_bin_file_read, callback_priv,
                                      fw_bin_sections_info_ptr[FW_BIN_SECTION_IDX_HW_VECTORS].offset,
                                      fw_bin_sections_info_ptr[FW_BIN_SECTION_IDX_HW_VECTORS].size,
                                      target_offset,
                                      fw_bin_sections_info_ptr[FW_BIN_SECTION_IDX_HW_VECTORS].checksum,
                                      0);
    }

    if ((res == RES_RESULT_OK) && (compressed == true))
    {
        res = HLB_decompress_command(handle, target_offset,
                                     ebl_target_offset_table_ptr->HW_VECTORS_addr);
        LOG_IF_ERR(res, "Decompress command failed");
    }

    return res;
}

static RES_result_t HLB_load_global_params(HLB_legacy_comm_handler_t *handle,
                                           IO_access_read_func fw_bin_file_read,
                                           void *callback_priv,
                                           const FW_bin_section_info_t *fw_bin_sections_info_ptr,
                                           const EBL_sections_target_addr_table_t *ebl_target_offset_table_ptr)
{
    RES_result_t res;

    res = HLB_send_data_in_chunks(handle, fw_bin_file_read, callback_priv,
                                  fw_bin_sections_info_ptr[FW_BIN_SECTION_IDX_GLOBAL_PARAMS].offset,
                                  fw_bin_sections_info_ptr[FW_BIN_SECTION_IDX_GLOBAL_PARAMS].size,
                                  ebl_target_offset_table_ptr->GP_addr,
                                  fw_bin_sections_info_ptr[FW_BIN_SECTION_IDX_GLOBAL_PARAMS].checksum,
                                  0);

    return res;
}

static RES_result_t HLB_load_bin_file_header(HLB_legacy_comm_handler_t *handle,
                                             IO_access_read_func fw_bin_file_read,
                                             void *callback_priv,
                                             const FW_bin_section_info_t *fw_bin_sections_info_ptr,
                                             const EBL_sections_target_addr_table_t *ebl_target_offset_table_ptr)
{
    RES_result_t res;

    res = HLB_send_data_in_chunks(handle, fw_bin_file_read, callback_priv,
                                  fw_bin_sections_info_ptr[FW_BIN_SECTION_IDX_BIN_FILE_HEADER].offset,
                                  fw_bin_sections_info_ptr[FW_BIN_SECTION_IDX_BIN_FILE_HEADER].size,
                                  ebl_target_offset_table_ptr->BIN_FILE_HEADER_addr,
                                  fw_bin_sections_info_ptr[FW_BIN_SECTION_IDX_BIN_FILE_HEADER].checksum,
                                  0);

    return res;
}

static void prep_cpu_section_for_DMI_access_imp(const BIN_image_header_t *image_header_ptr,
                                                BIN_image_section_header_t *image_section_headers_arr,
                                                uint32_t ICCM_Base_Address,
                                                uint32_t ICCM_Size,
                                                uint32_t DCCM_Base_Address,
                                                uint32_t DCCM_Size)
{
    uint32_t ICCM_Compiled_Address = ICCM_ADDRESS;
    uint32_t DCCM_Compiled_Address = DCCM_ADDRESS;
    uint32_t ICCM_Base_Addr_low = ICCM_Compiled_Address;
    uint32_t ICCM_Base_Addr_high = ICCM_Compiled_Address + ICCM_Size;
    uint32_t DCCM_Base_Addr_low = DCCM_Compiled_Address;
    uint32_t DCCM_Base_Addr_high = DCCM_Compiled_Address + DCCM_Size;
    uint32_t address;
    uint32_t i;

    for (i = 0; i < image_header_ptr->numOfSections; i++)
    {
        /* address to load to (should be used by the SET_MEM command) */
        address = image_section_headers_arr[i].address;

        if ((address >= ICCM_Base_Addr_low) && (address <= ICCM_Base_Addr_high))
        {
            /* ICCM */
            address = address - ICCM_Compiled_Address + ICCM_Base_Address;
        }
        else if ((address >= DCCM_Base_Addr_low) && (address <= DCCM_Base_Addr_high))
        {
            /* DCCM */
            address = address - DCCM_Compiled_Address + DCCM_Base_Address;
        }
        else
        {
            /* MISRA.IF.NO_ELSE */
        }

        /* Update the address in the original table */
        image_section_headers_arr[i].address = address;
    }
}

static RES_result_t prep_cpu_section_for_DMI_access(const BIN_image_header_t *image_header_ptr,
                                                    BIN_image_section_header_t *image_section_headers_arr)
{
    RES_result_t res = RES_RESULT_OK;
    uint32_t ICCM_Base_Address;
    uint32_t ICCM_Size;
    uint32_t DCCM_Base_Address;
    uint32_t DCCM_Size;

    switch (image_header_ptr->cpu_id)
    {
    case (uint8_t)CPU_ID_CMU:
        ICCM_Base_Address = MAC_MEM_DMI_CMP_CODE_DMI_MEMORY_BASE_ADDR;
        ICCM_Size = MAC_MEM_DMI_CMP_CODE_DMI_MEMORY_SIZE;
        DCCM_Base_Address = MAC_MEM_DMI_CMP_LDST_DMI_MEMORY_BASE_ADDR;
        DCCM_Size = MAC_MEM_DMI_CMP_LDST_DMI_MEMORY_SIZE;
        break;

    case (uint8_t)CPU_ID_SEG:
        ICCM_Base_Address = MAC_MEM_DMI_SGP_CODE_DMI_MEMORY_BASE_ADDR;
        ICCM_Size = MAC_MEM_DMI_SGP_CODE_DMI_MEMORY_SIZE;
        DCCM_Base_Address = MAC_MEM_DMI_SGP_LDST_DMI_MEMORY_BASE_ADDR;
        DCCM_Size = MAC_MEM_DMI_SGP_LDST_DMI_MEMORY_SIZE;
        break;

    case (uint8_t)CPU_ID_LCU:
        ICCM_Base_Address = PHY_MEM_DMI_LCU_CODE_DMI_MEMORY_BASE_ADDR;
        ICCM_Size = PHY_MEM_DMI_LCU_CODE_DMI_MEMORY_SIZE;
        DCCM_Base_Address = PHY_MEM_DMI_LCU_LDST_DMI_MEMORY_BASE_ADDR;
        DCCM_Size = PHY_MEM_DMI_LCU_LDST_DMI_MEMORY_SIZE;
        break;

    case (uint8_t)CPU_ID_PCU:
        ICCM_Base_Address = PHY_MEM_DMI_PCU_CODE_DMI_MEMORY_BASE_ADDR;
        ICCM_Size = PHY_MEM_DMI_PCU_CODE_DMI_MEMORY_SIZE;
        DCCM_Base_Address = PHY_MEM_DMI_PCU_LDST_DMI_MEMORY_BASE_ADDR;
        DCCM_Size = PHY_MEM_DMI_PCU_LDST_DMI_MEMORY_SIZE;
        break;

    default:
        res = RES_RESULT_GENERAL_ERROR;
        break;
    }

    if (res == RES_RESULT_OK)
    {
        /* The size of the DMI memory is in unit of 32bit */
        ICCM_Size = ICCM_Size * sizeof(uint32_t);
        DCCM_Size = DCCM_Size * sizeof(uint32_t);

        prep_cpu_section_for_DMI_access_imp(image_header_ptr,
                                            image_section_headers_arr,
                                            ICCM_Base_Address, ICCM_Size,
                                            DCCM_Base_Address, DCCM_Size);
    }

    return res;
}

static void HLB_cpu_image_checksum_update(uint32_t *checksum, const uint8_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        *checksum += data[i];
    }
}

static void HLB_cpu_image_checksum_finalize(uint32_t *checksum)
{
    *checksum = TWOS_COMPLEMENT(*checksum);
}

static RES_result_t HLB_load_cpu_images_imp(HLB_legacy_comm_handler_t *handle,
                                            IO_access_read_func fw_bin_file_read,
                                            void *callback_priv,
                                            uint32_t fw_bin_offset,
                                            const BIN_image_header_t *image_header_ptr,
                                            const BIN_image_section_header_t *image_section_headers_arr)
{
    RES_result_t res = RES_RESULT_OK;
    uint8_t buffer[MAX_CHUNK_SIZE];
    uint32_t checksum = 0;
    uint32_t size;
    uint32_t offset;
    uint32_t address;
    uint32_t packet_size;
    uint32_t read_amount;
    uint32_t buf_offset;
    uint32_t i;

    (void)checksum;

    for (i = 0; (i < image_header_ptr->numOfSections) && (res == RES_RESULT_OK); i++)
    {
        size = image_section_headers_arr[i].size;
        offset = image_section_headers_arr[i].offset;
        address = image_section_headers_arr[i].address;

        while ((size > 0U) && (res == RES_RESULT_OK))
        {
            read_amount = MIN(size, MAX_CHUNK_SIZE);
            buf_offset = 0;
            res = fw_bin_file_read(callback_priv, buffer, fw_bin_offset + offset, read_amount);
            LOG_IF_ERR(res, "Failed to read image section data");

            while ((read_amount > 0U) && (res == RES_RESULT_OK))
            {
                packet_size = MIN(read_amount, BUFFER_PAYLOAD_LIMIT);

#ifndef DO_NOT_PERFORM_CPU_IMAGES_CHECKSUM_CALC
                HLB_cpu_image_checksum_update(&checksum, &buffer[buf_offset], packet_size);
#endif /* DO_NOT_PERFORM_CPU_IMAGES_CHECKSUM_CALC */

                res = HLB_set_image_data(handle, address, packet_size, &buffer[buf_offset]);
                LOG_IF_ERR(res, "Set Image data failed. i=%u offset=%u size=%u", i, offset, size);
                if (res == RES_RESULT_OK)
                {
                    size -= packet_size;
                    offset += packet_size;
                    address += packet_size;
                    read_amount -= packet_size;
                    buf_offset += packet_size;
                }
            }
        }
    }

#ifndef DO_NOT_PERFORM_CPU_IMAGES_CHECKSUM_CALC
    if (res == RES_RESULT_OK)
    {
        HLB_cpu_image_checksum_finalize(&checksum);
        if (checksum != image_header_ptr->checksum)
        {
            res = RES_RESULT_GENERAL_ERROR;
            LOG_IF_ERR(res, "CPU ID %u checksum missmatch 0x%08X != 0x%08X",
                       image_header_ptr->cpu_id, checksum, image_header_ptr->checksum);
        }
    }
#endif /* DO_NOT_PERFORM_CPU_IMAGES_CHECKSUM_CALC */

    return res;
}

static RES_result_t HLB_load_cpu_imp(HLB_legacy_comm_handler_t *handle,
                                     IO_access_read_func fw_bin_file_read,
                                     void *callback_priv,
                                     uint32_t fw_bin_offset,
                                     BIN_image_header_t *image_header_ptr,
                                     uint32_t *cp_exec_addr_ptr)
{
    RES_result_t res = RES_RESULT_OK;
    BIN_image_section_header_t image_section_headers[image_header_ptr->numOfSections];

    res = fw_bin_file_read(callback_priv, (uint8_t*)&image_section_headers,
                           fw_bin_offset + sizeof(BIN_image_header_t),
                           sizeof(BIN_image_section_header_t) * image_header_ptr->numOfSections);
    LOG_IF_ERR(res, "Failed to read image section headers");

    if ((res == RES_RESULT_OK) && (image_header_ptr->cpu_id != (uint8_t)CPU_ID_CP))
    {
        /* Applicable only for "CPU_ID_CMU", "CPU_ID_SEG", "CPU_ID_LCU", "CPU_ID_PCU" */
        res = prep_cpu_section_for_DMI_access(image_header_ptr,
                                              image_section_headers);
        LOG_IF_ERR(res, "Failed to update load addresses");
    }

    if (res == RES_RESULT_OK)
    {
        /* load the CPU image header to CG FW */
        res = HLB_set_image_header(handle, image_header_ptr);
        LOG_IF_ERR(res, "Failed to load image header to CG5317");
    }

    if (res == RES_RESULT_OK)
    {
        /* Load CPU image data to CG FW */
        res = HLB_load_cpu_images_imp(handle,
                                      fw_bin_file_read,
                                      callback_priv,
                                      fw_bin_offset,
                                      image_header_ptr,
                                      image_section_headers);
        LOG_IF_ERR(res, "Failed to load image data to CG5317");
    }

    if (res == RES_RESULT_OK)
    {
        /* EXEC for CP CPU happens at the end of the fw load process */
        if (image_header_ptr->cpu_id == (uint8_t)CPU_ID_CP)
        {
            *cp_exec_addr_ptr = image_section_headers[0].address;
            res = HLB_init_copy(handle);
            LOG_IF_ERR(res, "Init copy failed");
        }
        else
        {
            res = HLB_execute_command(handle, image_section_headers[0].address,
                                      0, 0, image_header_ptr->cpu_id);
            LOG_IF_ERR(res, "Exec command failed");
        }
    }

    return res;
}

static RES_result_t HLB_load_cpu(HLB_legacy_comm_handler_t *handle,
                                 IO_access_read_func fw_bin_file_read,
                                 void *callback_priv,
                                 uint32_t fw_bin_offset,
                                 uint32_t *offset_ptr,
                                 uint32_t *cp_exec_addr_ptr)
{
    RES_result_t res = RES_RESULT_OK;
    BIN_image_header_t image_header;

    /* Read the CPU image header */
    res = fw_bin_file_read(callback_priv, (uint8_t*)&image_header,
                           *offset_ptr + fw_bin_offset,
                           sizeof(image_header));
    LOG_IF_ERR(res, "Failed to read image header");

    if (res == RES_RESULT_OK)
    {
        /* Load the image header and the image data to CG FW */
        res = HLB_load_cpu_imp(handle, fw_bin_file_read, callback_priv,
                               *offset_ptr + fw_bin_offset,
                               &image_header, cp_exec_addr_ptr);
        *offset_ptr += sizeof(BIN_image_header_t);
        *offset_ptr += sizeof(BIN_image_section_header_t) * image_header.numOfSections;
        *offset_ptr += image_header.total_image_size;
    }

    return res;
}

static RES_result_t HLB_load_cpus(HLB_legacy_comm_handler_t *handle,
                                  IO_access_read_func fw_bin_file_read,
                                  void *callback_priv,
                                  const FW_bin_section_info_t *fw_bin_sections_info_ptr,
                                  const EBL_sections_target_addr_table_t *ebl_target_offset_table_ptr,
                                  uint32_t *cp_exec_addr_ptr)
{
    RES_result_t res = RES_RESULT_OK;
    uint32_t i;
    uint32_t offset = 0;
    uint32_t value = 0;

    res = HLB_write_to_mem(handle,
                           ebl_target_offset_table_ptr->CP_addr,
                           sizeof(value), (uint8_t*)&value, TIMEOUT_MSEC);
    LOG_IF_ERR(res, "CP write CP addr write zero failed");

    for (i = 0; (res == RES_RESULT_OK) && (i <= (uint8_t)FW_BIN_SECTION_IDX_LAST); i++)
    {
        /* Load all CPU images (5 of them) to CG FW in the oder at which they appear in the CP section.
         * First CPU image starts at offset 0, and the CPU images are concatenated to each other.
         * offset field is sent by reference and is updated to the offset of the next image */
        res = HLB_load_cpu(handle, fw_bin_file_read, callback_priv,
                           fw_bin_sections_info_ptr[FW_BIN_SECTION_IDX_CPUS].offset,
                           &offset, cp_exec_addr_ptr);
        LOG_IF_ERR(res, "Failed to load CPU (i=%u)", i);
    }

    return res;
}

static RES_result_t HLB_parse_ebl(EBL_SPI_header_t *ebl_spi_header_ptr,
                                  EBL_sections_target_addr_table_t *ebl_target_offset_table_ptr)
{
    RES_result_t res = RES_RESULT_OK;
    APP_flash_header_t *app_header_ptr = &ebl_spi_header_ptr->app_header;
    ROOT_header_t *root_header_ptr = &ebl_spi_header_ptr->root_header;
    SPI_flash_section_t *ebl_flash_sec_ptr;
    uint32_t magic_num;
    uint8_t i;

    /* FW target address for CP section */
    COPY_UNALIGNED_LE_32_BIT_TO_HOST(ebl_target_offset_table_ptr->CP_addr,
                                     root_header_ptr->app_target_adrr);

    for (i = 0; (i < app_header_ptr->num_sections) && (res == RES_RESULT_OK); i++)
    {
        ebl_flash_sec_ptr = &app_header_ptr->flash_sections[i];

        COPY_UNALIGNED_LE_32_BIT_TO_HOST(magic_num, ebl_flash_sec_ptr->magic_num);
        switch (magic_num)
        {
        case BIN_FILE_HEADER_MAGIC_NUM:
            /* FW target address for BIN File header section */
            COPY_UNALIGNED_LE_32_BIT_TO_HOST(ebl_target_offset_table_ptr->BIN_FILE_HEADER_addr,
                                             ebl_flash_sec_ptr->target_addr);
            break;

        case GLOBAL_PARAMS_MAGIC_NUM:
            /* FW target address for Global Params section */
            COPY_UNALIGNED_LE_32_BIT_TO_HOST(ebl_target_offset_table_ptr->GP_addr,
                                             ebl_flash_sec_ptr->target_addr);
            break;

        case HW_VECT_MAGIC_NUM:
            /* FW target address for HW vectors section */
            COPY_UNALIGNED_LE_32_BIT_TO_HOST(ebl_target_offset_table_ptr->HW_VECTORS_addr,
                                             ebl_flash_sec_ptr->target_addr);
            break;

        case USER_CONFIG_SECTION_MAGIC_NUM:
            /* FW target address for user fw config */
            COPY_UNALIGNED_LE_32_BIT_TO_HOST(ebl_target_offset_table_ptr->USER_CONFIG_addr,
                                             ebl_flash_sec_ptr->target_addr);
            break;

        case APP_MAGIC_NUM:
            /* FW target address for CP section (will usually not appear here) */
            COPY_UNALIGNED_LE_32_BIT_TO_HOST(ebl_target_offset_table_ptr->CP_addr,
                                             ebl_flash_sec_ptr->target_addr);
            break;

        default:
            res = RES_RESULT_GENERAL_ERROR;
            break;
        }
    }

    return res;
}



static RES_result_t HLB_load_and_parse_ebl(HLB_legacy_comm_handler_t *handle,
                                           IO_access_read_func fw_bin_file_read,
                                           void *callback_priv,
                                           const FW_bin_section_info_t *fw_bin_sections_info_ptr,
                                           EBL_sections_target_addr_table_t *ebl_target_offset_table_ptr)
{
    RES_result_t res = RES_RESULT_OK;
    uint8_t buffer[fw_bin_sections_info_ptr[FW_BIN_SECTION_IDX_EBL_SPI_BCB].size];
    EBL_SPI_header_t *ebl_spi_header_ptr = (EBL_SPI_header_t *)buffer;
    uint32_t checksum = 0;

    res = fw_bin_file_read(callback_priv, buffer,
                           fw_bin_sections_info_ptr[FW_BIN_SECTION_IDX_EBL_SPI_BCB].offset,
                           fw_bin_sections_info_ptr[FW_BIN_SECTION_IDX_EBL_SPI_BCB].size);
    LOG_IF_ERR(res, "FW bin EBL section read failed");

    if (res == RES_RESULT_OK)
    {
        HBL_section_checksum_update(&checksum, buffer,
                                    fw_bin_sections_info_ptr[FW_BIN_SECTION_IDX_EBL_SPI_BCB].size);
        HBL_section_checksum_finalize(&checksum);

        if (checksum != fw_bin_sections_info_ptr[FW_BIN_SECTION_IDX_EBL_SPI_BCB].checksum)
        {
            res = RES_RESULT_GENERAL_ERROR;
            LOG_IF_ERR(res, "checksum missmatch 0x%08X != 0x%08X",
                       checksum, fw_bin_sections_info_ptr[FW_BIN_SECTION_IDX_EBL_SPI_BCB].checksum);
        }
    }

    if (res == RES_RESULT_OK)
    {
        res = HLB_parse_ebl(ebl_spi_header_ptr, ebl_target_offset_table_ptr);
        LOG_IF_ERR(res, "Failed to parse EBL");
    }

    if (res == RES_RESULT_OK)
    {
        res = HLB_write_to_mem(handle, EBL_LOAD_ADDRESS,
                               fw_bin_sections_info_ptr[FW_BIN_SECTION_IDX_EBL_SPI_BCB].size,
                               buffer, TIMEOUT_MSEC);
        LOG_IF_ERR(res, "Err loading EBL to CG5317");
    }

    return res;
}

static RES_result_t HLB_fw_bin_section_id_to_section_idx(uint32_t section_id,
                                                         FW_bin_section_idx_t *section_idx_ptr)
{
    RES_result_t res = RES_RESULT_OK;

    switch (section_id)
    {
    case (uint32_t)eBIN_Section_EBL_SPI_BCB:
        *section_idx_ptr = FW_BIN_SECTION_IDX_EBL_SPI_BCB;
        break;
    case (uint32_t)eBIN_Section_BIN_FILE_HEADER:
        *section_idx_ptr = FW_BIN_SECTION_IDX_BIN_FILE_HEADER;
        break;
    case (uint32_t)eBIN_Section_FW_CP:
        *section_idx_ptr = FW_BIN_SECTION_IDX_CPUS;
        break;
    case (uint32_t)eBIN_Section_FW_HW:
        *section_idx_ptr = FW_BIN_SECTION_IDX_HW_VECTORS;
        break;
    case (uint32_t)eBIN_Section_FW_GP:
        *section_idx_ptr = FW_BIN_SECTION_IDX_GLOBAL_PARAMS;
        break;

    default:
        res = RES_RESULT_NOT_SUPPORTED;
        break;
    }

    return res;
}

static RES_result_t HLB_create_fw_bin_sections_info_table(IO_access_read_func fw_bin_file_read,
                                                          void *callback_priv,
                                                          FW_bin_section_info_t *fw_bin_sections_info_ptr)
{
    RES_result_t res = RES_RESULT_OK;
    BIN_section_header_t BIN_section_header;
    FW_bin_section_idx_t section_idx;
    uint32_t sections_found = 0;
    uint32_t offset = 0;

    while ((res == RES_RESULT_OK) && (sections_found <= (uint8_t)FW_BIN_SECTION_IDX_LAST))
    {
        /* first section starts at offset 0 in the FW bin.
         * Sections are concatenated on top of each other so the next offset is the size of the
         * previous section + section header (12 bytes) */
        res = fw_bin_file_read(callback_priv, (uint8_t*)&BIN_section_header, offset, sizeof(BIN_section_header));
        if (res == RES_RESULT_OK)
        {
            res = HLB_fw_bin_section_id_to_section_idx(BIN_section_header.Section,
                                                       &section_idx);
            LOG_IF_ERR(res, "Failed get section index from section id 0x%X", BIN_section_header.Section);
            if (res == RES_RESULT_OK)
            {
                fw_bin_sections_info_ptr[section_idx].offset = offset + sizeof(BIN_section_header);
                COPY_UNALIGNED_LE_32_BIT_TO_HOST(fw_bin_sections_info_ptr[section_idx].size, BIN_section_header.Length);
                COPY_UNALIGNED_LE_32_BIT_TO_HOST(fw_bin_sections_info_ptr[section_idx].checksum, BIN_section_header.CheckSum);

                offset += sizeof(BIN_section_header) + BIN_section_header.Length;
                sections_found++;
            }
        }
    }

    return res;
}

static RES_result_t HLB_reconf_cg_to_rmii_timing(HLB_legacy_comm_handler_t *handle)
{
    RES_result_t res;
    uint32_t val = 0U;
    int32_t tries = MAX_TRIES_RMII_CFG;

    res = HLB_write_to_mem(handle, HOST_CLOCK_OUT_CONTROL_REG,
                           sizeof(val), (uint8_t*)&val, 0);
    LOG_IF_ERR(res, "write to memory error");
    if (res == RES_RESULT_OK)
    {
        do
        {
            res = HLB_write_to_mem(handle, HOST_CLOCK_OUT_CONTROL_REG,
                                   sizeof(val), (uint8_t*)&val, TIMEOUT_RMII_CGF);
            LOG_IF_ERR(res, "write to memory error");
        } while ((res == RES_RESULT_TIMEOUT) && (tries-- > 0));
    }

    return res;
}

static RES_result_t HLB_load_fw_imp(const mac_address_t addr,
                                    IO_access_read_func fw_bin_file_read,
                                    IO_access_read_func user_config_bin_file_read,
                                    void *callback_priv,
                                    uint32_t size_eth_handle,
                                    bool is_rmii)
{
    RES_result_t res = RES_RESULT_OK;
    uint8_t eth_buffer[size_eth_handle];
    HLB_hpgp_communication_t com;
    HLB_legacy_comm_handler_t legacy_comm;
    legacy_comm.communication = &com;
    mac_address_t broadcast_dest_mac_address = BROADCAST_MAC_ADDRESS;
    legacy_comm.communication->m_eth_handle_t = (void *)eth_buffer;
    HLB_query_device_t query_device;
    FW_bin_section_info_t fw_bin_sections_info[(uint8_t)FW_BIN_SECTION_IDX_LAST + 1U] = { 0 };
    EBL_sections_target_addr_table_t ebl_target_offset_table = { 0 };
    uint32_t cp_exec_addr = 0;
    bool connected = false;
    bool query_status = false;
    uint32_t i;

    MARK_TIMESTAMP("Entry")

    res = CM_connect(addr, broadcast_dest_mac_address, LEGACY_ETHER_TYPE,
                     legacy_comm.communication->m_eth_handle_t);
    LOG_IF_ERR(res, "Failed to init communication");
    if (res == RES_RESULT_OK)
    {
        connected = true;
        legacy_comm.session_id = 0x0100;
        MEMCPY(com.nic_mac_addr, addr, MAC_LEN);
        MEMCPY(com.dest_mac_address, broadcast_dest_mac_address, MAC_LEN);
    }

    if ((res == RES_RESULT_OK) && (is_rmii == true))
    {
        res = HLB_reconf_cg_to_rmii_timing(&legacy_comm);
        LOG_IF_ERR(res, "Failed to configure CG RMII timing");
    }

    MARK_TIMESTAMP("Com channel opened")

    /* QUERY device to verify FW is in BOOT CODE state & to get its MAC address */

    if (res == RES_RESULT_OK)
    {
        res = HLB_query_device(&legacy_comm, &query_device, TIMEOUT_MSEC);
        LOG_IF_ERR(res, "Failed to query device");
    }

    if (res == RES_RESULT_OK)
    {
        /* Verify that the device is in host loading mode */
        if (query_device.DeviceState != (uint8_t)DEVICE_STATE_BOOTCODE)
        {
            res = RES_RESULT_GENERAL_ERROR;
            LOG_IF_ERR(res, "Device not in host boot mode");
        }
    }

    MARK_TIMESTAMP("Device queried")

    if (res == RES_RESULT_OK)
    {
        /* Update the destination mac address to be the address of the modem */
        connected = false;
        CM_disconnect(legacy_comm.communication->m_eth_handle_t);
        res = CM_connect(addr, query_device.au8MACAddr, LEGACY_ETHER_TYPE, legacy_comm.communication->m_eth_handle_t);
        LOG_IF_ERR(res, "Failed to update the destination mac address to be the address of the modem");
        if (res == RES_RESULT_OK)
        {
            connected = true;
            legacy_comm.session_id = 0x0100;
            MEMCPY(com.nic_mac_addr, addr, MAC_LEN);
            MEMCPY(com.dest_mac_address, query_device.au8MACAddr, MAC_LEN);
        }
    }

    MARK_TIMESTAMP("Com channel re-opened, starting fw loading")

    /* FW is in BOOT rom and we know its MAC address */

    if (res == RES_RESULT_OK)
    {
        /* Create offset table of sections (ebl,GP,HW,CP...) inside the FW bin */
        res = HLB_create_fw_bin_sections_info_table(fw_bin_file_read,
                                                    callback_priv,
                                                    fw_bin_sections_info);
        LOG_IF_ERR(res, "Failed to create fw bin file offsets table");
    }

    if (res == RES_RESULT_OK)
    {
        /* Parse EBL section to get the CG FW 'target address' of all sections + usr config bin
         * And send EBL section to CG FW */
        res = HLB_load_and_parse_ebl(&legacy_comm,
                                     fw_bin_file_read,
                                     callback_priv,
                                     fw_bin_sections_info,
                                     &ebl_target_offset_table);
        LOG_IF_ERR(res, "Failed to load & parse EBL section");
    }

    if (res == RES_RESULT_OK)
    {
        /* Load all CPU images inside the CP section and send them to CG FW */
        res = HLB_load_cpus(&legacy_comm,
                            fw_bin_file_read,
                            callback_priv,
                            fw_bin_sections_info,
                            &ebl_target_offset_table,
                            &cp_exec_addr);
        LOG_IF_ERR(res, "Failed to load CPUs");
    }

    if (res == RES_RESULT_OK)
    {
        /* Load FW bin file header section and send it to CG FW */
        res = HLB_load_bin_file_header(&legacy_comm,
                                       fw_bin_file_read,
                                       callback_priv,
                                       fw_bin_sections_info,
                                       &ebl_target_offset_table);
        LOG_IF_ERR(res, "Failed to load bin file header section");
    }

    if (res == RES_RESULT_OK)
    {
        /* Load (And update based on user config bin) the GP section and send it to CG FW */
        res = HLB_load_global_params(&legacy_comm,
                                     fw_bin_file_read,
                                     callback_priv,
                                     fw_bin_sections_info,
                                     &ebl_target_offset_table);
        LOG_IF_ERR(res, "Failed to load global parameters section");
    }

    if (res == RES_RESULT_OK)
    {
        /* Load the HW section and send it to CG FW (Or decompress if needed) */
        res = HLB_load_hw_vectors(&legacy_comm,
                                  fw_bin_file_read,
                                  callback_priv,
                                  fw_bin_sections_info,
                                  &ebl_target_offset_table);
        LOG_IF_ERR(res, "Failed to load hw vectors section");
    }

    if (res == RES_RESULT_OK)
    {
        /* Load the user fw config and send it to CG FW */
        res = HLB_load_user_config_bin(&legacy_comm,
                                       user_config_bin_file_read,
                                       callback_priv,
                                       &ebl_target_offset_table);
        LOG_IF_ERR(res, "Failed to load user config bin");
    }

    if (res == RES_RESULT_OK)
    {
        /* Execute the CP CPU (others were executed during HLB_load_cpus function) */
        res = HLB_exec_CP(&legacy_comm, cp_exec_addr);
        LOG_IF_ERR(res, "Failed to exec main CPU");
    }

    MARK_TIMESTAMP("finished sending fw loading commands -> waiting for FW up")

    /* Wait until FW responds to QUERY and reports 'In FW' state */

    if (res == RES_RESULT_OK)
    {
        /* Update the destination mac address to be the broadcast */
        connected = false;
        CM_disconnect(legacy_comm.communication->m_eth_handle_t);
        res = CM_connect(addr, broadcast_dest_mac_address, LEGACY_ETHER_TYPE,
                         legacy_comm.communication->m_eth_handle_t);
        LOG_IF_ERR(res, "Failed to init communication");
        if (res == RES_RESULT_OK)
        {
            connected = true;
            legacy_comm.session_id = 0x0100;
            MEMCPY(com.nic_mac_addr, addr, MAC_LEN);
            MEMCPY(com.dest_mac_address, broadcast_dest_mac_address, MAC_LEN);
        }
    }

    if (res == RES_RESULT_OK)
    {
        /* turn log off to not be flooded with errors from failed queries */
        hlb_log_set(false);

        for (i = 0; (i < MAX_TRIES_QUERY) && (res == RES_RESULT_OK); i++)
        {
            res = HLB_query_device(&legacy_comm, &query_device, TIMEOUT_QUERY_MS);
            if (res == RES_RESULT_OK)
            {
                query_status = true;
                break;
            }
            else if (res == RES_RESULT_TIMEOUT)
            {
                res = RES_RESULT_OK;
            }
            else
            {
                /* MISRA.IF.NO_ELSE */
            }
        }

        hlb_log_set(true);
        LOG_IF_ERR(res, "Query returned error");

        if (res == RES_RESULT_OK)
        {
            if (query_status == false)
            {
                res = RES_RESULT_TIMEOUT;
                MARK_TIMESTAMP("Timeout while waiting for active FW after FW loading")
                LOG_IF_ERR(res, "Timeout while waiting for active FW after FW loading");
            }
            else if (query_device.DeviceState != (uint8_t)DEVICE_STATE_FW)
            {
                res = RES_RESULT_GENERAL_ERROR;
                LOG_IF_ERR(res, "FW loading done, but CG state not correct");
            }
            else
            {
                /* MISRA.IF.NO_ELSE */
                MARK_TIMESTAMP("Query reply received and FW is running")
            }
        }
    }

    if (connected)
    {
        CM_disconnect(legacy_comm.communication->m_eth_handle_t);
    }

    MARK_TIMESTAMP("Com channel closed")

    return res;
}

/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/
RES_result_t HLB_load_fw(const mac_address_t addr,
                         IO_access_read_func fw_bin_file_read,
                         IO_access_read_func user_config_bin_file_read,
                         void *callback_priv,
                         bool is_rmii)
{
    RES_result_t res;

    res = HLB_load_fw_imp(addr,
                          fw_bin_file_read, user_config_bin_file_read,
                          callback_priv, ETH_handle_alloc_size(), is_rmii);

    return res;
}
