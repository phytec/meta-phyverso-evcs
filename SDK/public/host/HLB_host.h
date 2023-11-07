/**
 * @file HLB_host.h
 * @author Orr Mazor
 * @date 2 May 2021
 * @brief File containing header for hlb host
 *
 *
 *
 * This module includes init/deinit functions and
 * the rx loop handling


 */
/********************************************************************
*
* Name: HLB_host
* Title: host library basic
* Package title: host library
* Abstract: provide init/deinit functions and definitions for host library
*
*
********************************************************************/
#ifndef HLB_HOST_H
#define HLB_HOST_H
/********************************************************************
* IMPORTS
********************************************************************/
#include "common.h"
#include <stdlib.h>
#include <stdint.h>
#include "version.h"
#include "HLB_protocol.h"

/********************************************************************
* EXPORTED CONSTANTS
********************************************************************/
/********************************************************************
* EXPORTED TYPES
********************************************************************/
/*! The msg id that we send to the user on rx_callback */
typedef enum
{
	HLB_HOST_MSG_ID_APCM_SET_KEY_REQ = 0,
	HLB_HOST_MSG_ID_APCM_SET_KEY_CNF,
	HLB_HOST_MSG_ID_APCM_GET_KEY_REQ,
	HLB_HOST_MSG_ID_APCM_GET_KEY_CNF,
	HLB_HOST_MSG_ID_APCM_SET_CCO_REQ,
	HLB_HOST_MSG_ID_APCM_SET_CCO_CNF,
	HLB_HOST_MSG_ID_APCM_CONN_ADD_REQ,
	HLB_HOST_MSG_ID_APCM_CONN_ADD_CNF,
	HLB_HOST_MSG_ID_APCM_CONN_ADD_IND,
	HLB_HOST_MSG_ID_APCM_CONN_ADD_RESP,
	HLB_HOST_MSG_ID_APCM_CONN_MOD_REQ,
	HLB_HOST_MSG_ID_APCM_CONN_MOD_CNF,
	HLB_HOST_MSG_ID_APCM_CONN_MOD_IND,
	HLB_HOST_MSG_ID_APCM_CONN_MOD_RESP,
	HLB_HOST_MSG_ID_APCM_CONN_REL_REQ,
	HLB_HOST_MSG_ID_APCM_CONN_REL_CNF,
	HLB_HOST_MSG_ID_APCM_CONN_REL_IND,
	HLB_HOST_MSG_ID_APCM_GET_NTB_REQ,
	HLB_HOST_MSG_ID_APCM_GET_NTB_CNF,
	HLB_HOST_MSG_ID_APCM_AUTHORIZE_REQ,
	HLB_HOST_MSG_ID_APCM_AUTHORIZE_CNF,
	HLB_HOST_MSG_ID_APCM_AUTHORIZE_IND,
	HLB_HOST_MSG_ID_APCM_GET_SECURITY_MODE_REQ,
	HLB_HOST_MSG_ID_APCM_GET_SECURITY_MODE_CNF,
	HLB_HOST_MSG_ID_APCM_SET_SECURITY_MODE_REQ,
	HLB_HOST_MSG_ID_APCM_SET_SECURITY_MODE_CNF,
	HLB_HOST_MSG_ID_APCM_GET_NETWORKS_REQ,
	HLB_HOST_MSG_ID_APCM_GET_NETWORKS_CNF,
	HLB_HOST_MSG_ID_APCM_SET_NETWORKS_REQ,
	HLB_HOST_MSG_ID_APCM_SET_NETWORKS_CNF,
	HLB_HOST_MSG_ID_APCM_GET_NEW_STA_REQ,
	HLB_HOST_MSG_ID_APCM_GET_NEW_STA_CNF,
	HLB_HOST_MSG_ID_APCM_GET_NEW_STA_IND,
	HLB_HOST_MSG_ID_APCM_STA_RESTART_REQ,
	HLB_HOST_MSG_ID_APCM_STA_RESTART_CNF,
	HLB_HOST_MSG_ID_APCM_NET_EXIT_REQ,
	HLB_HOST_MSG_ID_APCM_NET_EXIT_CNF,
	HLB_HOST_MSG_ID_APCM_SET_TONE_MASK_REQ,
	HLB_HOST_MSG_ID_APCM_SET_TONE_MASK_CNF,
	HLB_HOST_MSG_ID_APCM_STA_CAP_REQ,
	HLB_HOST_MSG_ID_APCM_STA_CAP_CNF,
	HLB_HOST_MSG_ID_APCM_NW_INFO_REQ,
	HLB_HOST_MSG_ID_APCM_NW_INFO_CNF,
	HLB_HOST_MSG_ID_NSCM_LINK_STATS_REQ,
	HLB_HOST_MSG_ID_NSCM_LINK_STATS_CNF,
	HLB_HOST_MSG_ID_APCM_GET_BEACON_REQ,
	HLB_HOST_MSG_ID_APCM_GET_BEACON_CNF,
	HLB_HOST_MSG_ID_APCM_GET_HFID_REQ,
	HLB_HOST_MSG_ID_APCM_GET_HFID_CNF,
	HLB_HOST_MSG_ID_APCM_SET_HFID_REQ,
	HLB_HOST_MSG_ID_APCM_SET_HFID_CNF,
	HLB_HOST_MSG_ID_APCM_SET_HD_DURATION_REQ,
	HLB_HOST_MSG_ID_APCM_SET_HD_DURATION_CNF,
	HLB_HOST_MSG_ID_APCM_UNASSOCIATED_STA_IND,
	HLB_HOST_MSG_ID_APCM_SC_JOIN_REQ,
	HLB_HOST_MSG_ID_APCM_SC_JOIN_CNF,
	HLB_HOST_MSG_ID_APCM_SET_PPKEYS_REQ,
	HLB_HOST_MSG_ID_APCM_SET_PPKEYS_CNF,
	HLB_HOST_MSG_ID_APCM_CONF_SLAC_REQ,
	HLB_HOST_MSG_ID_APCM_CONF_SLAC_CNF,
	HLB_HOST_MSG_ID_NSCM_GET_VERSION_REQ,
	HLB_HOST_MSG_ID_NSCM_GET_VERSION_CNF,
	HLB_HOST_MSG_ID_NSCM_RESET_DEVICE_REQ,
	HLB_HOST_MSG_ID_NSCM_GET_CE2_INFO_REQ,
	HLB_HOST_MSG_ID_NSCM_GET_CE2_INFO_CNF,
	HLB_HOST_MSG_ID_NSCM_GET_CE2_DATA_REQ,
	HLB_HOST_MSG_ID_NSCM_GET_CE2_DATA_CNF,
	HLB_HOST_MSG_ID_NSCM_GET_LNOE_REQ,
	HLB_HOST_MSG_ID_NSCM_GET_LNOE_CNF,
	HLB_HOST_MSG_ID_NSCM_GET_SNRE_REQ,
	HLB_HOST_MSG_ID_NSCM_GET_SNRE_CNF,
	HLB_HOST_MSG_ID_NSCM_ABORT_DUMP_ACTION_REQ,
	HLB_HOST_MSG_ID_NSCM_ABORT_DUMP_ACTION_CNF,
	HLB_HOST_MSG_ID_NSCM_ENTER_PHY_MODE_REQ,
	HLB_HOST_MSG_ID_NSCM_ENTER_PHY_MODE_CNF,
	HLB_HOST_MSG_ID_NSCM_READ_MEM_REQ,
	HLB_HOST_MSG_ID_NSCM_READ_MEM_CNF,
	HLB_HOST_MSG_ID_NSCM_WRITE_MEM_REQ,
	HLB_HOST_MSG_ID_NSCM_WRITE_MEM_CNF,
	HLB_HOST_MSG_ID_NSCM_GET_DC_CALIB_REQ,
	HLB_HOST_MSG_ID_NSCM_GET_DC_CALIB_CNF,
	HLB_HOST_MSG_ID_NSCM_GET_DEVICE_STATE_REQ,
	HLB_HOST_MSG_ID_NSCM_GET_DEVICE_STATE_CNF,
	HLB_HOST_MSG_ID_NSCM_GET_D_LINK_STATUS_REQ,
	HLB_HOST_MSG_ID_NSCM_GET_D_LINK_STATUS_CNF,
	HLB_HOST_MSG_ID_NSCM_VS_HOST_MESSAGE_STATUS_IND,
	HLB_HOST_MSG_ID_NSCM_D_LINK_READY_IND,
	HLB_HOST_MSG_ID_NSCM_D_LINK_TERMINATE_REQ,
	HLB_HOST_MSG_ID_NSCM_D_LINK_TERMINATE_CNF,
	HLB_HOST_MSG_ID_NSCM_DEVICE_INFO_REQ,
	HLB_HOST_MSG_ID_NSCM_DEVICE_INFO_CNF,
	HLB_HOST_MSG_ID_NSCM_GET_AMP_MAP_REQ,
	HLB_HOST_MSG_ID_NSCM_GET_AMP_MAP_CNF,

	/*! Enum value used to indicate last msg id */
	HLB_HOST_MSG_ID_LAST
} HLB_host_msg_id;

/*! the rx callback that should be implemented by the user. */
typedef void (*hlb_rx_callback_t)(HLB_host_msg_id, uint8_t);


/********************************************************************
* EXPORTED MACROS
********************************************************************/
/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/
/**
 *   Function Name: HLB_init
 *
 * @brief Description: this function inits host library.
 * It initialize the memory pool, the rx db, the rx thread,
 * the communication with the transport module
 *
 *
 *  @param  input : adapter_mac_address - Source adapter mac address
 *  @param  input : dest_mac_address - Destination mac address
 *  @param  input : callback - Rx callback to be called when rx packet is arrived
 *  @param  input : allocated_area - Preallocated area to be used by the host library
 *  @param  input : allocated_area_size - Size of the preallocated area
 *  @return comm handle item if Ok ,NULL if error.
 *
 *
 */
comm_handle_t HLB_init(const uint8_t *adapter_mac_address,
					   const uint8_t *dest_mac_address,
					   hlb_rx_callback_t callback, void *allocated_area,
					   size_t allocated_area_size);

/**
 *   Function Name: HLB_deinit
 *
 * @brief Description: this function deinits host library.
 * It uninitialize the memory pool, the rx db, the rx thread,
 * the communication with the transport module
 *
 *  @param  input : handle - Communication handle
 *
 */
void HLB_deinit(comm_handle_t handle);

/**
 *   Function Name: HLB_get_version
 *
 * @brief Description: this function gets the current version
 * 
 * @param  hlb_ver : the object that will be 
 * assigned the current version values
 */
void HLB_get_version(version_t* hlb_ver);


/********************************************************************
* INTERNAL FUNCTIONS DECLARATIONS (FOR UNIT TESTING)
********************************************************************/

#endif // HLB_HOST_H
