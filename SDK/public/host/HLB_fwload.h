/**
 * @file HLB_fwload.h
 * @author Michael Vassernis
 * @date 23 March 2022
 * @brief File containing the API for the firmware loading
 *
 *
 *
  */
/********************************************************************
*
* Module Name: Firmware loading API Header
* Title:Firmware loading API Header
* Package title
* Abstract:
*   This module is the API for the CG5317 firmware loading
*
********************************************************************/
#ifndef HLB_FWLOAD_H
#define HLB_FWLOAD_H

/********************************************************************
* IMPORTS
********************************************************************/
#include "common.h"
#include "shared.h"
#include "HLB_legacy_commands.h"
#include <stdbool.h>

/*******************************************************************
* CONSTANTS
********************************************************************/
#define BCB_MAGIC_NUM                   (0xBCB56BCBU)
#define APP_MAGIC_NUM                   (0xB17890B1U)
#define SPI_HEADER_MAGIC_NUM            (0x2222AACDU)
#define BIN_FILE_HEADER_MAGIC_NUM       (0xAABBFFEEU)
#define GLOBAL_PARAMS_MAGIC_NUM         (0xAABBDDCCU)
#define HW_VECT_MAGIC_NUM               (0xAABBAABBU)
#define USER_CONFIG_SECTION_MAGIC_NUM   (0xCCCBBCCCU)

/*! Max number of section that can be added to the SPI.
 *  Does not include BCB/ESCAPE/B1 and application sections. */
#define SPI_MAX_SECTIONS        (4)
#define EBL_LOAD_ADDRESS        (0x149F00U)
#define MAX_SECTION_NAME_LEN    (24)
#define ICCM_ADDRESS            (0x00000000U)
#define DCCM_ADDRESS            (0x80000000U)
#define MAX_SECTION_NUM         (64)

#define MAC_MEM_DMI_CMP_CODE_DMI_MEMORY_BASE_ADDR (0x70810000U)
#define MAC_MEM_DMI_CMP_CODE_DMI_MEMORY_SIZE (10240U)
#define MAC_MEM_DMI_CMP_LDST_DMI_MEMORY_BASE_ADDR (0x7081A000U)
#define MAC_MEM_DMI_CMP_LDST_DMI_MEMORY_SIZE (4096U)
#define MAC_MEM_DMI_SGP_LDST_DMI_MEMORY_BASE_ADDR (0x7080A000U)
#define MAC_MEM_DMI_SGP_LDST_DMI_MEMORY_SIZE (4096U)
#define PHY_MEM_DMI_LCU_CODE_DMI_MEMORY_BASE_ADDR (0x71180000U)
#define PHY_MEM_DMI_LCU_CODE_DMI_MEMORY_SIZE (8192U)
#define PHY_MEM_DMI_LCU_LDST_DMI_MEMORY_BASE_ADDR (0x71190000U)
#define PHY_MEM_DMI_LCU_LDST_DMI_MEMORY_SIZE (4096U)
#define PHY_MEM_DMI_PCU_CODE_DMI_MEMORY_BASE_ADDR (0x71100000U)
#define PHY_MEM_DMI_PCU_CODE_DMI_MEMORY_SIZE (16384U)
#define PHY_MEM_DMI_PCU_LDST_DMI_MEMORY_BASE_ADDR (0x71110000U)
#define PHY_MEM_DMI_PCU_LDST_DMI_MEMORY_SIZE (8192U)
#define MAC_MEM_DMI_SGP_CODE_DMI_MEMORY_BASE_ADDR (0x70800000U)
#define MAC_MEM_DMI_SGP_CODE_DMI_MEMORY_SIZE (10240U)

/*******************************************************************
* TYPES
********************************************************************/

/**
 * @brief Callback function to perform IO read access for host loading process
 *
 * @param callback_priv - user defined data.
 * @param buffer        - buffer to store the read bytes.
 * @param offset        - offset in the file to read from.
 * @param count         - amount of bytes to read.
 *
 */
typedef RES_result_t (*IO_access_read_func)(void *callback_priv,
                                            uint8_t *buffer,
                                            uint32_t offset,
                                            uint32_t count);

/*! The sections in the FW bin file */
typedef enum
{
    eBIN_Section_EBL_SPI_BCB = 1,                 /*!< EBL's SPI and BCB section */
    eBIN_Section_FW_HW = 5,                       /*!< HW-Vector section */
    eBIN_Section_FW_GP = 6,                       /*!< Global Param Section */
    eBIN_Section_FW_CP = 7,                       /*!< FW CP section */
    eBIN_Section_BIN_FILE_HEADER = 0x4E484753,    /*!< BIN file Header (ASCII: "SGHN") */
} eBIN_Section;

/*! The posssible device states */
typedef enum
{
    DEVICE_STATE_BOOTCODE   = 0,    /*!< Device in Bootrom */
    DEVICE_STATE_FW         = 1,    /*!< Device in FW */
} device_state_t;

/*! The CPUs numeration */
typedef enum
{
    CPU_ID_CP = 1,    /*!< CP CPU */
    CPU_ID_CMU = 2,   /*!< CMU CPU */
    CPU_ID_SEG = 4,   /*!< SEG CPU */
    CPU_ID_LCU = 5,   /*!< LCU CPU */
    CPU_ID_PCU = 6,   /*!< PCU CPU */
} CPU_ID_t;

/*! The sections header in the BIN file */
PACK(
    uint32_t Section;   /*!< The numeration of the sections (eBIN_Section) */
    uint32_t CheckSum;  /*!< The sections checksum */
    uint32_t Length;    /*!< The sections length */
) BIN_section_header_t;

/*! Root header structure. */
PACK(
    uint32_t BCB_magic_num;   /*!< The magic number representing the BCB */
    uint32_t BCB_offset;      /*!< The BCB offset within the image file */
    uint32_t app_magic_num;   /*!< The magic number representing the app */
    uint32_t app_offset;      /*!< The APP offset within the image file */
    uint32_t app_launch_addr; /*!< The APP launch address */
    uint32_t app_target_adrr; /*!< The APP target address */
) ROOT_header_t;

/*! Use to define an SPI section which contains a specific BIN file */
PACK(
    uint32_t magic_num;       /*!< Identifier the section type: app/B1/BCB... */
    uint32_t offset;          /*!< Section's offset in the flash */
    uint32_t target_addr;     /*!< Memory address to load the section */
) SPI_flash_section_t;

/*! App flash header structure. */
PACK(
    uint32_t SPI_header_magic_num;                        /*!< The SPI header magic number */
    uint32_t SPI_header_version;                          /*!< The SPI header version */

    /* Common section to the Application,B1 and the Flash Upgrade */
    uint8_t  num_sections;                                /*!< The number of sections */
    uint8_t  reserved[3];

    SPI_flash_section_t flash_sections[SPI_MAX_SECTIONS]; /*!< The flash sections */
) APP_flash_header_t;

/*! Extended boot loader SPI header. This is the SPI header that sits at the beginning of the flash */
PACK(
    ROOT_header_t root_header;
    APP_flash_header_t app_header;
) EBL_SPI_header_t;

/*! The image section header struct */
PACK(
    uint8_t sectionName[MAX_SECTION_NAME_LEN];    /*!< Name of the section */
    uint32_t address;                             /*!< address to load to (should be used by the SET_MEM command) */
    uint32_t offset;                              /*!< offset from start of the image file */
    uint32_t size;                                /*!< size of section (should be used by the SET_MEM command) */
) BIN_image_section_header_t;

/*******************************************************************
* MACROS
********************************************************************/

/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/

/**
 * Function Name: HLB_load_fw
 *
 * @brief Description: this function implements loading the firmware binary, configuring the CG5317 memory, and executing the firmware.
 *
 * @param input : addr - the adapter mac address.
 * @param input : fw_bin_file_read - callback to perform reads from fw_bin file.
 * @param input : user_config_bin_file_read - callback to perform reads from user config bin file.
 * @param input : callback_priv - user private data passed to IO read APIs.
 * @param input : is_rmii - is the CG configured to work in RMII mode (instead of MII).
 *
 * @return RES_result_t : result status code
 *
 */
RES_result_t HLB_load_fw(const mac_address_t addr,
                         IO_access_read_func fw_bin_file_read,
                         IO_access_read_func user_config_bin_file_read,
                         void *callback_priv,
                         bool is_rmii);

#endif /* HLB_FWLOAD_H */
