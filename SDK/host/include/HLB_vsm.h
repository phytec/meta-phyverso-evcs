#ifndef __HLB_VSM__H__
#define __HLB_VSM__H__

/********************************************************************
* IMPORTS
********************************************************************/
#include "common.h"
#include "shared.h"
#include "HLB_eth_driver.h"

/********************************************************************
* EXPORTED CONSTANTS
********************************************************************/
#define MAX_VERSION_NAME_LEN    (24U)	        /*!< The maximal version name length */

/********************************************************************
* EXPORTED TYPES
********************************************************************/

/*! The payload of the response from VSM_MSG_QUERY_DEVICE */
PACK(
    uint8_t au8MACAddr[HMAC_LEN];   /*!< Device MAC address */
    uint8_t DeviceState;            /*!< Device State */
    uint8_t	is_new_vsm_format;      /*!< 0 old VSM format, 1 new VSM format */
    uint32_t OptionFlags;           /*!< Option Flags */
    uint32_t svn_rev_ver;           /*!< SVN revision version */
    char sw_version[32];            /*!< FW Version */
    char build_date[16];            /*!< Build Date */
    char build_time[16];            /*!< BuildTime */
) HLB_query_device_t;

/*! The payload struct of VSM_MSG_SET_IMAGE_HEADER */
PACK(
    uint8_t version[MAX_VERSION_NAME_LEN];  /*!< Version */
    uint32_t cpu_id;                        /*!< The CPU id */
    uint32_t numOfSections;                 /*!< The number of sections */
    uint32_t total_image_size;              /*!< The size of the image */
    uint32_t checksum;                      /*!< checksum */
) HLB_fw_cpu_image_header_t;

typedef struct
{
    uint32_t session_id;                    /*!< The id of the session */
	uint8_t src_mac_addr[HMAC_LEN];         /*!< NIC mac address */
	uint8_t dst_mac_addr[HMAC_LEN];         /*!< device mac address */
} HLB_vsm_session_info_t;

/********************************************************************
* EXPORTED MACROS
********************************************************************/
/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/
RES_result_t HLB_vsm_set_mem(HLB_eth_drv_t *p_eth_drv,
                             HLB_vsm_session_info_t *p_session_info,
                             uint32_t address, uint32_t size, uint8_t *buffer,
                             uint32_t timeout_ms);

RES_result_t HLB_vsm_query_device(HLB_eth_drv_t *p_eth_drv,
                                  HLB_vsm_session_info_t *p_session_info,
                                  HLB_query_device_t *p_query_resp,
                                  uint32_t timeout_ms);

RES_result_t HLB_vsm_set_image_data(HLB_eth_drv_t *p_eth_drv,
                                    HLB_vsm_session_info_t *p_session_info,
                                    uint32_t address, uint32_t size, uint8_t *buffer,
                                    uint32_t timeout_ms);

RES_result_t HLB_vsm_set_image_header(HLB_eth_drv_t *p_eth_drv,
                                      HLB_vsm_session_info_t *p_session_info,
                                      HLB_fw_cpu_image_header_t *p_img_header,
                                      uint32_t timeout_ms);

RES_result_t HLB_vsm_init_copy(HLB_eth_drv_t *p_eth_drv,
                               HLB_vsm_session_info_t *p_session_info,
                               uint32_t timeout_ms);

RES_result_t HLB_vsm_execute_cmd(HLB_eth_drv_t *p_eth_drv,
                                 HLB_vsm_session_info_t *p_session_info,
                                 uint32_t address,
                                 uint32_t ignore_checksum,
                                 uint32_t image_checksum,
                                 uint32_t cpu_id,
                                 uint32_t timeout_ms);

RES_result_t HLB_vsm_decompress_cmd(HLB_eth_drv_t *p_eth_drv,
                                    HLB_vsm_session_info_t *p_session_info,
                                    uint32_t src_addr,
                                    uint32_t dest_addr,
                                    uint32_t timeout_ms);

#endif /* __HLB_VSM__H__ */
