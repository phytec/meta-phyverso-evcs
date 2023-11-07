/********************************************************************
*
* Module Name: HLB_host
* Design:
* Implement apcm init/deinit api and rx loop
*
********************************************************************/
/*******************************************************************
* IMPORTS
********************************************************************/
#include "HLB_host.h"
#include "HLB_helper.h"
#include "comm_mgr_lib.h"
#include "osal.h"
#include "mem_pool.h"
#include <string.h>
/*******************************************************************
* CONSTANTS
********************************************************************/
/*******************************************************************
* TYPES
********************************************************************/
typedef struct
{
	HLB_hpgp_communication_t *com;
	hlb_rx_callback_t callback;
} HLB_rx_loop_struct_t;

typedef struct
{
	HLB_hpgp_communication_t com;
	void *rx_loop_p;
} HLB_com_rx_loop_t;

/*******************************************************************
* STATIC and GLOBAL DATA
********************************************************************/
void *thread_handle = NULL;
/*******************************************************************
* MACROS
********************************************************************/
/*******************************************************************
* INTERNAL FUNCTIONS
********************************************************************/

static RES_result_t HLB_protocol_msg_id_to_host_msg_id(HLB_protocol_msg_id_t prot_msg_id,
													   HLB_host_msg_id *host_msg_id)
{
	RES_result_t res = RES_RESULT_OK;

#define CASE_MSG_ID(id) case HLB_PROTOCOL_MSG_ID_##id: *host_msg_id = HLB_HOST_MSG_ID_##id; break;

	switch (prot_msg_id)
	{
		CASE_MSG_ID(APCM_SET_KEY_REQ)
		CASE_MSG_ID(APCM_SET_KEY_CNF)
		CASE_MSG_ID(APCM_GET_KEY_REQ)
		CASE_MSG_ID(APCM_GET_KEY_CNF)
		CASE_MSG_ID(APCM_SET_CCO_REQ)
		CASE_MSG_ID(APCM_SET_CCO_CNF)
		CASE_MSG_ID(APCM_CONN_ADD_REQ)
		CASE_MSG_ID(APCM_CONN_ADD_CNF)
		CASE_MSG_ID(APCM_CONN_ADD_IND)
		CASE_MSG_ID(APCM_CONN_ADD_RESP)
		CASE_MSG_ID(APCM_CONN_MOD_REQ)
		CASE_MSG_ID(APCM_CONN_MOD_CNF)
		CASE_MSG_ID(APCM_CONN_MOD_IND)
		CASE_MSG_ID(APCM_CONN_MOD_RESP)
		CASE_MSG_ID(APCM_CONN_REL_REQ)
		CASE_MSG_ID(APCM_CONN_REL_CNF)
		CASE_MSG_ID(APCM_CONN_REL_IND)
		CASE_MSG_ID(APCM_GET_NTB_REQ)
		CASE_MSG_ID(APCM_GET_NTB_CNF)
		CASE_MSG_ID(APCM_AUTHORIZE_REQ)
		CASE_MSG_ID(APCM_AUTHORIZE_CNF)
		CASE_MSG_ID(APCM_AUTHORIZE_IND)
		CASE_MSG_ID(APCM_GET_SECURITY_MODE_REQ)
		CASE_MSG_ID(APCM_GET_SECURITY_MODE_CNF)
		CASE_MSG_ID(APCM_SET_SECURITY_MODE_REQ)
		CASE_MSG_ID(APCM_SET_SECURITY_MODE_CNF)
		CASE_MSG_ID(APCM_GET_NETWORKS_REQ)
		CASE_MSG_ID(APCM_GET_NETWORKS_CNF)
		CASE_MSG_ID(APCM_SET_NETWORKS_REQ)
		CASE_MSG_ID(APCM_SET_NETWORKS_CNF)
		CASE_MSG_ID(APCM_GET_NEW_STA_REQ)
		CASE_MSG_ID(APCM_GET_NEW_STA_CNF)
		CASE_MSG_ID(APCM_GET_NEW_STA_IND)
		CASE_MSG_ID(APCM_STA_RESTART_REQ)
		CASE_MSG_ID(APCM_STA_RESTART_CNF)
		CASE_MSG_ID(APCM_NET_EXIT_REQ)
		CASE_MSG_ID(APCM_NET_EXIT_CNF)
		CASE_MSG_ID(APCM_SET_TONE_MASK_REQ)
		CASE_MSG_ID(APCM_SET_TONE_MASK_CNF)
		CASE_MSG_ID(APCM_STA_CAP_REQ)
		CASE_MSG_ID(APCM_STA_CAP_CNF)
		CASE_MSG_ID(APCM_NW_INFO_REQ)
		CASE_MSG_ID(APCM_NW_INFO_CNF)
		CASE_MSG_ID(NSCM_LINK_STATS_REQ)
		CASE_MSG_ID(NSCM_LINK_STATS_CNF)
		CASE_MSG_ID(APCM_GET_BEACON_REQ)
		CASE_MSG_ID(APCM_GET_BEACON_CNF)
		CASE_MSG_ID(APCM_GET_HFID_REQ)
		CASE_MSG_ID(APCM_GET_HFID_CNF)
		CASE_MSG_ID(APCM_SET_HFID_REQ)
		CASE_MSG_ID(APCM_SET_HFID_CNF)
		CASE_MSG_ID(APCM_SET_HD_DURATION_REQ)
		CASE_MSG_ID(APCM_SET_HD_DURATION_CNF)
		CASE_MSG_ID(APCM_UNASSOCIATED_STA_IND)
		CASE_MSG_ID(APCM_SC_JOIN_REQ)
		CASE_MSG_ID(APCM_SC_JOIN_CNF)
		CASE_MSG_ID(APCM_SET_PPKEYS_REQ)
		CASE_MSG_ID(APCM_SET_PPKEYS_CNF)
		CASE_MSG_ID(APCM_CONF_SLAC_REQ)
		CASE_MSG_ID(APCM_CONF_SLAC_CNF)
		CASE_MSG_ID(NSCM_GET_VERSION_REQ)
		CASE_MSG_ID(NSCM_GET_VERSION_CNF)
		CASE_MSG_ID(NSCM_RESET_DEVICE_REQ)
		CASE_MSG_ID(NSCM_GET_CE2_INFO_REQ)
		CASE_MSG_ID(NSCM_GET_CE2_INFO_CNF)
		CASE_MSG_ID(NSCM_GET_CE2_DATA_REQ)
		CASE_MSG_ID(NSCM_GET_CE2_DATA_CNF)
		CASE_MSG_ID(NSCM_GET_LNOE_REQ)
		CASE_MSG_ID(NSCM_GET_LNOE_CNF)
		CASE_MSG_ID(NSCM_GET_SNRE_REQ)
		CASE_MSG_ID(NSCM_GET_SNRE_CNF)
		CASE_MSG_ID(NSCM_ABORT_DUMP_ACTION_REQ)
		CASE_MSG_ID(NSCM_ABORT_DUMP_ACTION_CNF)
		CASE_MSG_ID(NSCM_ENTER_PHY_MODE_REQ)
		CASE_MSG_ID(NSCM_ENTER_PHY_MODE_CNF)
		CASE_MSG_ID(NSCM_READ_MEM_REQ)
		CASE_MSG_ID(NSCM_READ_MEM_CNF)
		CASE_MSG_ID(NSCM_WRITE_MEM_REQ)
		CASE_MSG_ID(NSCM_WRITE_MEM_CNF)
		CASE_MSG_ID(NSCM_GET_DC_CALIB_REQ)
		CASE_MSG_ID(NSCM_GET_DC_CALIB_CNF)
		CASE_MSG_ID(NSCM_GET_DEVICE_STATE_REQ)
		CASE_MSG_ID(NSCM_GET_DEVICE_STATE_CNF)
		CASE_MSG_ID(NSCM_GET_D_LINK_STATUS_REQ)
		CASE_MSG_ID(NSCM_GET_D_LINK_STATUS_CNF)
		CASE_MSG_ID(NSCM_VS_HOST_MESSAGE_STATUS_IND)
		CASE_MSG_ID(NSCM_D_LINK_READY_IND)
		CASE_MSG_ID(NSCM_D_LINK_TERMINATE_REQ)
		CASE_MSG_ID(NSCM_D_LINK_TERMINATE_CNF)
		CASE_MSG_ID(NSCM_DEVICE_INFO_REQ)
		CASE_MSG_ID(NSCM_DEVICE_INFO_CNF)
		CASE_MSG_ID(NSCM_GET_AMP_MAP_REQ)
		CASE_MSG_ID(NSCM_GET_AMP_MAP_CNF)

		default:
			res = RES_RESULT_BAD_PARAMETER;
	}

	return res;
}

static void *HLB_rx_loop(void *param)
{
	HLB_rx_loop_struct_t *rx_loop_struct = (HLB_rx_loop_struct_t *)param;
	void *eth_handle = rx_loop_struct->com->m_eth_handle_t;
	hlb_rx_callback_t callback = rx_loop_struct->callback;
	HLB_rx_db_t *rx_db = (HLB_rx_db_t *)rx_loop_struct->com->private_pointer;
	HLB_packet_t Buffer;
	size_t rx_packet_length;
	RES_result_t res = RES_RESULT_OK;
	HLB_req_id_t req_id;
	uint8_t num_frags, fmsn = 0;
	HLB_host_msg_id host_msg_id;

	memset(&Buffer, 0x00, sizeof(HLB_packet_t));
	while (res == RES_RESULT_OK)
	{
		rx_packet_length = HLB_PACKET_SIZE;
		res = ETH_rx(eth_handle, &Buffer, &rx_packet_length, -1);
		if (res == RES_RESULT_OK)
		{
			Buffer.management_header.msg_id = letohs(Buffer.management_header.msg_id);
			if (!HLB_is_control_path_message(&Buffer))
			{
				continue;
			}
			if (HLB_push_to_rx_db(rx_db, &Buffer, rx_packet_length) != RES_RESULT_OK)
			{
				hlb_log_error("failed to push msg id = %d to rx db, db is full",
							  Buffer.management_header.msg_id);
				continue;
			}
			num_frags = HLB_PACKET_GET_NUM_FRAGS(&Buffer);
			if (num_frags == HLB_PACKET_GET_FRAG_IDX(&Buffer) + 1)
			{
				if (num_frags == 1)
				{
					req_id = Buffer.vendor_header.req_id;
				}
				else
				{
					/* The request id appears inside the first fragment only,
					 * And Buffer contains the last fragment in the series
					 */
					fmsn = HLB_PACKET_GET_FSM(&Buffer);
					if (HLB_get_req_id_of_fragmented_msg(rx_db, Buffer.management_header.msg_id,
														 fmsn, &req_id) != RES_RESULT_OK)
					{
						hlb_log_error("failed to get the req id of msg id = %hu & FMSN = %hhu",
									  Buffer.management_header.msg_id, fmsn);
						continue;
					}
				}

				if (HLB_protocol_msg_id_to_host_msg_id(Buffer.management_header.msg_id,
													   &host_msg_id) != RES_RESULT_OK)
				{
					hlb_log_error("failed to get the host msg id of prot msg id = %hu",
								  Buffer.management_header.msg_id, fmsn);
					continue;
				}

				callback(host_msg_id, req_id);
			}
		}
	}
	return NULL;
}

/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/

comm_handle_t HLB_init(const uint8_t *adapter_mac_address,
					   const uint8_t *dest_mac_address,
					   hlb_rx_callback_t callback, void *allocated_area,
					   size_t allocated_area_size)
{
	RES_result_t res;
	HLB_hpgp_communication_t *com;
	HLB_rx_loop_struct_t *rx_loop_struct;
	memory_pool_t *pool;
	HLB_com_rx_loop_t *rx_loop_p;
	size_t rx_db_area_size;

	if (!adapter_mac_address || !allocated_area)
	{
		hlb_log_error("HLB_init received NULL PTR");
		return NULL;
	}

	memory_pool_init(allocated_area, allocated_area_size, &pool);
	memory_pool_set(pool);
	com = malloc(sizeof(HLB_com_rx_loop_t));
	if (com == NULL)
	{
		hlb_log_error("malloc com failed");
		return NULL;
	}
	res = HLB_init_communication(adapter_mac_address, dest_mac_address, com);
	if (res != RES_RESULT_OK)
	{
		hlb_log_error("HLB_init_communication failed, res=%d", res);
		free(com);
		return NULL;
	}

	rx_loop_struct = malloc(sizeof(*rx_loop_struct));
	if (rx_loop_struct == NULL)
	{
		hlb_log_error("malloc rx_loop_struct failed");
		HLB_deinit_communication(com);
		free(com);
		return NULL;
	}
	rx_loop_struct->callback = callback;
	rx_loop_struct->com = com;
	rx_loop_p = (HLB_com_rx_loop_t *)(com);
	rx_loop_p->rx_loop_p = rx_loop_struct;

	/* Assumption: the allocated_area is used for allocations only on init */
	rx_db_area_size = allocated_area_size - memory_pool_get_usage(pool);

	com->private_pointer = HLB_init_rx_db(rx_db_area_size);
	if (com->private_pointer == NULL)
	{
		hlb_log_error("HLB_init_rx_db failed");
		free(rx_loop_struct);
		HLB_deinit_communication(com);
		free(com);
		return NULL;
	}

	res = osal_thread_create(&thread_handle, HLB_rx_loop, rx_loop_struct);
	if (res != RES_RESULT_OK)
	{
		free(rx_loop_struct);
		hlb_log_error("osal_thread_create failed, res=%d", res);
		HLB_free_rx_db((HLB_rx_db_t *)com->private_pointer);
		HLB_deinit_communication(com);
		free(com);
		return NULL;
	}
	return (comm_handle_t)com;
}

void HLB_deinit(comm_handle_t handle)
{
	size_t cap, usage;
	HLB_com_rx_loop_t *rx_loop_p;
	HLB_hpgp_communication_t *com = (HLB_hpgp_communication_t *)handle;
	if (com == NULL)
	{
		hlb_log_error("handle provided in HLB_deinit is NULL!");
		return;
	}
	ETH_break_rx(com->m_eth_handle_t);
	osal_thread_join(&thread_handle);
	osal_thread_delete(&thread_handle);
	HLB_deinit_communication(com);
	HLB_free_rx_db((HLB_rx_db_t *)com->private_pointer);
	rx_loop_p = (HLB_com_rx_loop_t *)(com);
	free(rx_loop_p->rx_loop_p);
	free(com);
	cap = memory_pool_get_capacity(NULL);
	usage = memory_pool_get_usage(NULL);
	memory_pool_destroy(NULL);
	hlb_log_info("final mempool usage=%u, cap=%u", usage, cap);
}

void HLB_get_version(version_t* hlb_ver)
{
	hlb_ver->version_major = MAJOR_VERSION;
	hlb_ver->version_minor = MINOR_VERSION;
	hlb_ver->version_patch = PATCH_VERSION;
	hlb_ver->version_build = BUILD_VERSION;
}
