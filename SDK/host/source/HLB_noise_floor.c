/********************************************************************
*
* Module Name: HLB_noise_floor
* Design:
* Implement noise floor api
*
********************************************************************/
/*******************************************************************
* IMPORTS
********************************************************************/
#include "HLB_noise_floor.h"
#include "HLB_nscm.h"
#include "osal.h"
#include "common.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
/*******************************************************************
* CONSTANTS
********************************************************************/
#define MODEM_RESET_FIRST_ADDR 0x706C0208U
#define MODEM_RESET_FIRST_VAL 0x00U
#define MODEM_RESET_SECOND_ADDR 0x61030014U
#define MODEM_RESET_SECOND_VAL 0x01U
#define RESET_MODEM_WAIT 50U
#define WAIT_FW_LOADING 8U
#define BUSY_WAIT_TIMEOUT 1000U
#define BUSY_WAIT_SLEEP 1U
#define AMB_TOP_HW_MEM_ACCESS_PERMISSION_ADDRESS 0x61050050U
#define AMB_TOP_HW_MEM_ACCESS_PERMISSION_VAL_FIRST 0x0001U
#define AMB_TOP_HW_MEM_ACCESS_PERMISSION_VAL_SECOND 0x0011U
#define AMB_TOP_I_MEM_DC_TABLE_A_0 0x61050700U
#define AMB_TOP_PDC_BYPASS_SELECTOR 0x61050404U
#define TDU_AGC_GAIN_OUT_OVERRIDE 0x71520450U
#define SLOG_IF_SLOG_ADD_REG 0x61010028U
#define FW_SLOG_BUF_START_ADDR 0x00130000U
/*******************************************************************
* TYPES
********************************************************************/
typedef enum
{
    BUSY_WAIT_PHY = 0,
    BUSY_WAIT_WRITE = 1,
} HLB_busy_wait_type;
/*******************************************************************
* STATIC and GLOBAL DATA
********************************************************************/
/*******************************************************************
* MACROS
********************************************************************/
/*******************************************************************
* INTERNAL FUNCTIONS
********************************************************************/
static RES_result_t HLB_busy_wait_cnf(const comm_handle_t handle, HLB_req_id_t req_id,
                                      HLB_busy_wait_type type)
{
    uint32_t i;
    RES_result_t res = RES_RESULT_GENERAL_ERROR;
    HLB_hpgp_result_t result;

    for (i = 0; i < BUSY_WAIT_TIMEOUT; i++)
    {
        if (type == BUSY_WAIT_PHY)
        {
            res = HLB_nscm_enter_phy_mode_cnf_receive(handle, req_id, &result);
        }
        else
        {
            res = HLB_nscm_write_mem_cnf_receive(handle, req_id, &result);
        }

        if (res == RES_RESULT_OK)
        {
            if (result != HLB_HPGP_RESULT_SUCCESS)
            {
                HLB_LOG_ERR("busy wait cnf failed, result=%d\n", result);
                res = RES_RESULT_GENERAL_ERROR;
            }

            break;
        }

        osal_msleep(BUSY_WAIT_SLEEP);
    }

    return res;
}

static RES_result_t HLB_reset_modem(const comm_handle_t handle)
{
    RES_result_t res;
    HLB_req_id_t req_id = 1;
    HLB_write_mem_req_t write_mem_req;

    write_mem_req.size = sizeof(uint32_t);
    MEMSET(write_mem_req.data, 0U, write_mem_req.size);
    write_mem_req.data[0] = MODEM_RESET_FIRST_VAL;
    write_mem_req.address = MODEM_RESET_FIRST_ADDR;

    res = HLB_nscm_write_mem_req_send(handle, req_id, &write_mem_req);
    LOG_IF_ERR(res, "write mem request failed")

    if (res == RES_RESULT_OK)
    {
        res = HLB_busy_wait_cnf(handle, req_id, BUSY_WAIT_WRITE);
        LOG_IF_ERR(res, "write mem wait for cnf failed")
    }

    if (res == RES_RESULT_OK)
    {
        write_mem_req.data[0] = MODEM_RESET_SECOND_VAL;
        write_mem_req.address = MODEM_RESET_SECOND_ADDR;

        res = HLB_nscm_write_mem_req_send(handle, req_id, &write_mem_req);
        LOG_IF_ERR(res, "second write mem request failed")
    }

    return res;
}

static RES_result_t HLB_enter_phy_mode(const comm_handle_t handle)
{
    RES_result_t res;
    HLB_req_id_t req_id = 1;

    res = HLB_nscm_enter_phy_mode_req_send(handle, req_id);
    LOG_IF_ERR(res, "enter phy mode request failed")

    if (res == RES_RESULT_OK)
    {
        res = HLB_busy_wait_cnf(handle, req_id, BUSY_WAIT_PHY);
        LOG_IF_ERR(res, "enter phy mode wait for cnf failed")
    }

    return res;
}

static RES_result_t HLB_parse_slog_ocla_buffer(const comm_handle_t handle,
                                               char* slog_ocla_buffer)
{
    HLB_write_mem_req_t write_mem_req;
    RES_result_t res = RES_RESULT_OK;
    HLB_req_id_t req_id = 1;
    char* str = slog_ocla_buffer;
    uint32_t address;
    uint32_t value;
    uint32_t delay;

    while ((str[0] != '\0') && (res == RES_RESULT_OK))
    {
        if (str[0] == '\n')
        {
            str++;
            continue;
        }

        delay = strtoul(str, &str, 10);
        if (str[0] == ',')
        {
            str++;
            address = strtoul(str, &str, 16);
            if (str[0] == ',')
            {
                str++;
                value = strtoul(str, &str, 16);

                if ((str[0] != '\n') && (str[0] != '\0'))
                {
                    res = RES_RESULT_BAD_PARAMETER;
                }
            }
            else
            {
                res = RES_RESULT_BAD_PARAMETER;
            }
        }
        else
        {
            res = RES_RESULT_BAD_PARAMETER;
        }

        if (res == RES_RESULT_OK)
        {
            write_mem_req.address = address;
            write_mem_req.size = sizeof(value);
            MEMCPY(write_mem_req.data, &value, sizeof(value));
            res = HLB_nscm_write_mem_req_send(handle, req_id, &write_mem_req);
            LOG_IF_ERR(res, "write mem request failed")
        }

        if (res == RES_RESULT_OK)
        {
            res = HLB_busy_wait_cnf(handle, req_id, BUSY_WAIT_WRITE);
            LOG_IF_ERR(res, "write mem wait for cnf failed")
        }

        if (res == RES_RESULT_OK)
        {
            osal_msleep(delay);
        }
    }

    return res;
}

#ifdef DC_CALIB
static RES_result_t HLB_get_dc_callib(const comm_handle_t handle, HLB_dc_calib_cnf_t *dc_calib)
{
    RES_result_t res;
    uint32_t i;
    HLB_req_id_t req_id = 1;

    res = HLB_nscm_get_dc_calib_req_send(handle, req_id);
    LOG_IF_ERR(res, "get DC calibration request failed")

    if (res == RES_RESULT_OK)
    {
        for (i = 0; i < BUSY_WAIT_TIMEOUT; i++)
        {
            res = HLB_nscm_get_dc_calib_cnf_receive(handle, req_id, dc_calib);
            if (res == RES_RESULT_OK)
            {
                break;
            }

            osal_msleep(BUSY_WAIT_SLEEP);
        }
    }

    LOG_IF_ERR(res, "get DC calibration wait for cnf failed")

    return res;
}

static RES_result_t HLB_update_permissions(const comm_handle_t handle, uint32_t address,
                                            uint8_t data)
{
    HLB_write_mem_req_t write_mem_req;
    HLB_req_id_t req_id = 1;
    RES_result_t res;

    write_mem_req.address = address;
    write_mem_req.size = sizeof(uint32_t);
    MEMSET(write_mem_req.data, 0U, write_mem_req.size);
    write_mem_req.data[0] = data;

    res = HLB_nscm_write_mem_req_send(handle, req_id, &write_mem_req);
    LOG_IF_ERR(res, "failed to update permission: %u", data)

    if (res == RES_RESULT_OK)
    {
        res = HLB_busy_wait_cnf(handle, req_id, BUSY_WAIT_WRITE);
    }

    return res;
}

static RES_result_t HLB_update_single_dc_val(const comm_handle_t handle, uint32_t index,
                                             HLB_dc_calib_cnf_t *dc_calib,
                                             HLB_req_id_t req_id)
{
    HLB_write_mem_req_t write_mem_req;
    RES_result_t res;

    write_mem_req.address = (uint32_t)(AMB_TOP_I_MEM_DC_TABLE_A_0 + ((index - 1U) * 4U));
    write_mem_req.size = sizeof(uint32_t);
    MEMSET(write_mem_req.data, 0U, write_mem_req.size);
    if (index <= 53U)
    {
        write_mem_req.data[0] = dc_calib->dc_offset_lo_ch1;
    }
    else
    {
        write_mem_req.data[0] = dc_calib->dc_offset_hi_ch1;
    }

    res = HLB_nscm_write_mem_req_send(handle, req_id, &write_mem_req);
    LOG_IF_ERR(res, "index:%u write mem request failed", index)

    if (res == RES_RESULT_OK)
    {
        res = HLB_busy_wait_cnf(handle, req_id, BUSY_WAIT_WRITE);
    }

    return res;
}

static RES_result_t HLB_update_dc_callib(const comm_handle_t handle)
{
    RES_result_t res;
    HLB_dc_calib_cnf_t dc_calib;
    HLB_req_id_t req_id = 0;
    uint32_t i;

    /* Get initial DC callibration values */
    res = HLB_get_dc_callib(handle, &dc_calib);
    LOG_IF_ERR(res, "get DC callibration failed")

    if (res == RES_RESULT_OK)
    {
        /* Update First Access permission */
        res = HLB_update_permissions(handle,
                                     AMB_TOP_HW_MEM_ACCESS_PERMISSION_ADDRESS,
                                     AMB_TOP_HW_MEM_ACCESS_PERMISSION_VAL_FIRST);
        LOG_IF_ERR(res, "failed to update first permission")
    }

    if (res == RES_RESULT_OK)
    {
        for (i = 1; (i <= 64) && (res == RES_RESULT_OK); i++)
        {
            req_id++;
            res = HLB_update_single_dc_val(handle, i, &dc_calib, req_id);
            LOG_IF_ERR(res, "failed to update DC callibration value at index: %u", i)
        }
    }

    if (res == RES_RESULT_OK)
    {
        /* Update Second Access permission */
        res = HLB_update_permissions(handle,
                                     AMB_TOP_HW_MEM_ACCESS_PERMISSION_ADDRESS,
                                     AMB_TOP_HW_MEM_ACCESS_PERMISSION_VAL_SECOND);
        LOG_IF_ERR(res, "failed to update second permission")
    }

    return res;
}

#endif

static RES_result_t HLB_update_force_gain(const comm_handle_t handle, int32_t gain)
{
    HLB_write_mem_req_t write_mem_req;
    HLB_req_id_t req_id = 1;
    RES_result_t res;
    int32_t tmp_gain, PN_Gain_writeValue;
    uint32_t write_UINT32;

    tmp_gain = (gain < 0) ? (gain + 128) : gain;
    PN_Gain_writeValue = tmp_gain + tmp_gain + 1;
    write_UINT32 = ((uint32_t)PN_Gain_writeValue & 0xFFU) + (((uint32_t)PN_Gain_writeValue & 0xFFU) << 8U);

    write_mem_req.address = AMB_TOP_PDC_BYPASS_SELECTOR;
    write_mem_req.size = sizeof(uint32_t);
    MEMSET(write_mem_req.data, 0U, write_mem_req.size);
    res = HLB_nscm_write_mem_req_send(handle, req_id, &write_mem_req);
    LOG_IF_ERR(res, "failed to write first value")

    if (res == RES_RESULT_OK)
    {
        res = HLB_busy_wait_cnf(handle, req_id, BUSY_WAIT_WRITE);
        LOG_IF_ERR(res, "write mem wait for cnf failed")
    }

    if (res == RES_RESULT_OK)
    {
        write_mem_req.address = TDU_AGC_GAIN_OUT_OVERRIDE;
        write_mem_req.size = sizeof(write_UINT32);
        MEMCPY(write_mem_req.data, &write_UINT32, sizeof(write_UINT32));
        res = HLB_nscm_write_mem_req_send(handle, req_id, &write_mem_req);
        LOG_IF_ERR(res, "failed to write second value")
    }

    if (res == RES_RESULT_OK)
    {
        res = HLB_busy_wait_cnf(handle, req_id, BUSY_WAIT_WRITE);
        LOG_IF_ERR(res, "write mem wait for cnf failed")
    }

    return res;
}

static RES_result_t HLB_get_stop_pos(const comm_handle_t handle, uint32_t* stop_pos)
{
    RES_result_t res;
    HLB_req_id_t req_id = 1;
    uint32_t i;
    HLB_read_mem_req_t read_mem_req;
    HLB_read_mem_cnf_t read_mem_cnf;

    read_mem_req.address = SLOG_IF_SLOG_ADD_REG;
    read_mem_req.size = sizeof(uint32_t);

    res = HLB_nscm_read_mem_req_send(handle, req_id, &read_mem_req);
    LOG_IF_ERR(res, "read mem request failed")

    if (res == RES_RESULT_OK)
    {
        for (i = 0; i < BUSY_WAIT_TIMEOUT; i++)
        {
            res = HLB_nscm_read_mem_cnf_receive(handle, req_id, &read_mem_cnf);
            if (res == RES_RESULT_OK)
            {
                if(read_mem_cnf.size != read_mem_req.size)
                {
                    hlb_log_error("HLB_get_noise_floor: HLB_get_stop_pos HLB_busy_wait_cnf received size 0\n");
                    res = RES_RESULT_INVALID_FW_PCKT;
                    break;
                }

                MEMCPY(stop_pos, read_mem_cnf.data, sizeof(*stop_pos));
                *stop_pos = letohl(*stop_pos);
                break;
            }
            osal_msleep(BUSY_WAIT_SLEEP);
        }

        LOG_IF_ERR(res, "read mem wait for cnf failed")
    }

    return res;
}

static RES_result_t HLB_read_single_sample(const comm_handle_t handle,
                                           uint32_t len, uint32_t fw_slog_buf_size,
                                           uint8_t *sample, uint32_t stop_pos)
{
    RES_result_t res = RES_RESULT_OK;
    HLB_req_id_t req_id = 2;
    uint32_t i;
    uint32_t offset = 0;
    HLB_read_mem_req_t read_mem_req;
    HLB_read_mem_cnf_t read_mem_cnf;
    uint32_t length_left = len;
    uint32_t start_pos;

    /* Jump to the start of the capture */
    start_pos = stop_pos + 4U;

    while ((length_left > 0) && (res == RES_RESULT_OK))
    {
        read_mem_req.address = FW_SLOG_BUF_START_ADDR + ((offset + start_pos) % fw_slog_buf_size);
        read_mem_req.size = (length_left > HLB_READ_MEM_MAX_LEN)? HLB_READ_MEM_MAX_LEN : length_left;
        if ((read_mem_req.address + read_mem_req.size) > (FW_SLOG_BUF_START_ADDR + fw_slog_buf_size))
        {
            read_mem_req.size -= (read_mem_req.address + read_mem_req.size) - (FW_SLOG_BUF_START_ADDR + fw_slog_buf_size);
        }

        res = HLB_nscm_read_mem_req_send(handle, req_id, &read_mem_req);
        LOG_IF_ERR(res, "read mem request failed")

        if (res == RES_RESULT_OK)
        {
            for (i = 0; i < BUSY_WAIT_TIMEOUT; i++)
            {
                res = HLB_nscm_read_mem_cnf_receive(handle, req_id, &read_mem_cnf);
                if (res == RES_RESULT_OK)
                {
                    if (read_mem_cnf.size != read_mem_req.size)
                    {
                        HLB_LOG_ERR("received size %u not equal to expected size %u\n", read_mem_cnf.size, read_mem_req.size);
                        res = RES_RESULT_INVALID_FW_PCKT;
                        break;
                    }

                    MEMCPY(&sample[offset], read_mem_cnf.data, read_mem_cnf.size);
                    offset += read_mem_cnf.size;
                    length_left -= read_mem_cnf.size;
                    break;
                }

                osal_msleep(BUSY_WAIT_SLEEP);
            }

            LOG_IF_ERR(res, "read mem wait for cnf failed")
        }
    }

    return res;
}

static RES_result_t HLB_get_samples(const comm_handle_t handle, char* slog_ocla_buffer,
                                    uint8_t iterations_num, uint8_t* samples_buffer,
                                    uint32_t sample_size, uint32_t fw_slog_buf_size)
{
    RES_result_t res = RES_RESULT_OK;
    uint32_t stop_pos;
    uint8_t i = 0;

    for (i = 0; (i < iterations_num) && (res == RES_RESULT_OK); i++)
    {
        /* Go over slog ocla list */
        res = HLB_parse_slog_ocla_buffer(handle, slog_ocla_buffer);
        LOG_IF_ERR(res, "parse slog ocla buffer failed")

        if (res == RES_RESULT_OK)
        {
            /* Get stop position */
            res = HLB_get_stop_pos(handle, &stop_pos);
            LOG_IF_ERR(res, "get stop position failed")
        }

        if (res == RES_RESULT_OK)
        {
            /* Read a Noise Floor sample */
            res = HLB_read_single_sample(handle, sample_size, fw_slog_buf_size, &samples_buffer[i * sample_size], stop_pos);
            LOG_IF_ERR(res, "read single sample %u failed", i)
        }
    }

    return res;
}

static RES_result_t HLB_convert_to_time_domain(const uint8_t* samples_buffer,
                                               uint32_t samples_buffer_size,
                                               int16_t *time_domain_buffer)
{
    int32_t sample;
    uint32_t temp_sample;
    uint32_t i;

    for (i = 0; i < (samples_buffer_size / 4U); i++)
    {
        COPY_UNALIGNED_LE_32_BIT_TO_HOST(temp_sample, samples_buffer[i * 4U])

		sample = (int32_t)((temp_sample & 0xfff000U) >> 12U);
		/* Setting sample to signed */
		if (sample > 2047)
		{
			sample = sample - 4096;
		}

        time_domain_buffer[2U * i] = sample;

		sample = (int32_t)(temp_sample & 0xfffU);
        /* Setting sample to signed */
		if (sample > 2047)
		{
			sample = sample - 4096;
		}

        time_domain_buffer[(2U * i) + 1U] = sample;
    }

    return RES_RESULT_OK;
}
/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/
RES_result_t HLB_get_noise_floor(const comm_handle_t handle,
                                 uint8_t iterations_num,
                                 int32_t gain, char* slog_ocla_buffer,
                                 uint32_t fw_slog_buf_size,
                                 uint32_t sample_size,
                                 int16_t *time_domain_buffer)
{
    uint32_t samples_buffer_size = iterations_num * sample_size;
    uint8_t samples_buffer[samples_buffer_size];
    RES_result_t res;

    /* Hardware reset the modem */
    res = HLB_reset_modem(handle);
    LOG_IF_ERR(res, "reset modem failed")

    if (res == RES_RESULT_OK)
    {
        /* Wait for the FW to finish loading */
        osal_sleep(WAIT_FW_LOADING);

        /* Enter the modem into PHY only mode */
        res = HLB_enter_phy_mode(handle);
        LOG_IF_ERR(res, "enter phy mode failed")
    }

    if (res == RES_RESULT_OK)
    {
        /* Go over slog ocla list */
        res = HLB_parse_slog_ocla_buffer(handle, slog_ocla_buffer);
        LOG_IF_ERR(res, "parse slog ocla buffer failed")
    }

#ifdef DC_CALIB
    if (res == RES_RESULT_OK)
    {
        /* Update DC callibration values */
        res = HLB_update_dc_callib(handle);
        LOG_IF_ERR(res, "update dc callib failed")
    }
#endif

    if (res == RES_RESULT_OK)
    {
        /* Update Force Gain */
        res = HLB_update_force_gain(handle, gain);
        LOG_IF_ERR(res, "update force gain failed")
    }

    if (res == RES_RESULT_OK)
    {
        /* Collect the Noise samples from the modem */
        res = HLB_get_samples(handle, slog_ocla_buffer, iterations_num,
                              samples_buffer, sample_size, fw_slog_buf_size);
        LOG_IF_ERR(res, "get samples failed")
    }

    if (res == RES_RESULT_OK)
    {
        /*Convert the samples buffer into a time domain buffer */
        res = HLB_convert_to_time_domain(samples_buffer, samples_buffer_size, time_domain_buffer);
        LOG_IF_ERR(res, "convert to time domain failed")
    }

    if (res == RES_RESULT_OK)
    {
        res = HLB_reset_modem(handle);
    }

    return res;
}
