/********************************************************************
*
* Module Name: HLB_apcm
* Design:
* Implement apcm api
*
********************************************************************/
/*******************************************************************
* IMPORTS
********************************************************************/
#include "HLB_apcm.h"
#include "HLB_protocol.h"
#include "HLB_helper.h"
#include "comm_mgr_lib.h"
#include <string.h>
#include <stdlib.h>
/*******************************************************************
* CONSTANTS
********************************************************************/
/*******************************************************************
* TYPES
********************************************************************/
/*******************************************************************
* STATIC and GLOBAL DATA
********************************************************************/
/*******************************************************************
* MACROS
********************************************************************/
/*******************************************************************
* INTERNAL FUNCTIONS
********************************************************************/
/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/

RES_result_t HLB_apcm_set_key_req_send(const comm_handle_t handle, HLB_req_id_t req_id,
									   const HLB_hpgp_set_key_req_t *set_key_req)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm || !set_key_req) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_set_key_req_create(comm->dest_mac_address, comm->nic_mac_addr,
										  set_key_req, req_id, &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_apcm_set_key_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
										  HLB_hpgp_result_t *result)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !result) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_APCM_SET_KEY_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_set_key_cnf_parse(&packet, result);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_apcm_get_key_req_send(const comm_handle_t handle, HLB_req_id_t req_id)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_get_key_req_create(comm->dest_mac_address, comm->nic_mac_addr, req_id,
										  &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_apcm_get_key_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
										  HLB_hpgp_get_key_cnf_t *get_key_cnf)
{
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm || !get_key_cnf) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_APCM_GET_KEY_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_get_key_cnf_parse(&packet, get_key_cnf);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_apcm_set_cco_req_send(const comm_handle_t handle, HLB_req_id_t req_id,
									   HLB_hpgp_cco_mode_t cco_mode)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_set_cco_req_create(comm->dest_mac_address, comm->nic_mac_addr,
										  req_id, cco_mode, &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_apcm_set_cco_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
										  HLB_hpgp_result_t *result)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !result) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_APCM_SET_CCO_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_set_cco_cnf_parse(&packet, result);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_apcm_get_ntb_req_send(const comm_handle_t handle, HLB_req_id_t req_id)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_get_ntb_req_create(comm->dest_mac_address, comm->nic_mac_addr, req_id,
										  &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_apcm_get_ntb_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
										  uint32_t *ntb)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !ntb) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_APCM_GET_NTB_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_get_ntb_cnf_parse(&packet, ntb);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_apcm_get_security_mode_req_send(const comm_handle_t handle, HLB_req_id_t req_id)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_get_security_mode_req_create(comm->dest_mac_address, comm->nic_mac_addr, req_id,
													&hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_apcm_get_security_mode_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
													HLB_hpgp_get_security_mode_cnf_t *security_mode_cnf)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !security_mode_cnf) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_APCM_GET_SECURITY_MODE_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_get_security_mode_cnf_parse(&packet, security_mode_cnf);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_apcm_set_security_mode_req_send(const comm_handle_t handle, HLB_req_id_t req_id,
												 HLB_hpgp_security_mode_t security_mode)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_set_security_mode_req_create(comm->dest_mac_address, comm->nic_mac_addr,
													req_id, security_mode, &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_apcm_set_security_mode_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
													HLB_hpgp_result_t *result)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !result) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_APCM_SET_SECURITY_MODE_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_set_security_mode_cnf_parse(&packet, result);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_apcm_get_networks_req_send(const comm_handle_t handle, HLB_req_id_t req_id)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_get_networks_req_create(comm->dest_mac_address, comm->nic_mac_addr, req_id,
											   &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_apcm_get_networks_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											   HLB_hpgp_get_networks_cnf_t *networks_cnf)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !networks_cnf) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_APCM_GET_NETWORKS_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_get_networks_cnf_parse(&packet, networks_cnf);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_apcm_set_networks_req_send(const comm_handle_t handle, HLB_req_id_t req_id,
											const HLB_hpgp_set_networks_req_t *networks_req)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm || !networks_req) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_set_networks_req_create(comm->dest_mac_address, comm->nic_mac_addr,
											   req_id, networks_req, &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_apcm_set_networks_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											   HLB_hpgp_result_t *result)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !result) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_APCM_SET_NETWORKS_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_set_networks_cnf_parse(&packet, result);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_apcm_get_new_sta_req_send(const comm_handle_t handle, HLB_req_id_t req_id)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_get_new_sta_req_create(comm->dest_mac_address, comm->nic_mac_addr, req_id,
											  &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_apcm_get_new_sta_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											  HLB_hpgp_get_new_sta_cnf_t *new_sta_cnf)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !new_sta_cnf) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_APCM_GET_NEW_STA_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_get_new_sta_cnf_parse(&packet, new_sta_cnf);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_apcm_get_new_sta_ind_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											  HLB_hpgp_get_new_sta_ind_t *new_sta_ind)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !new_sta_ind) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_APCM_GET_NEW_STA_IND,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_get_new_sta_ind_parse(&packet, new_sta_ind);
		LOG_IF_ERR(res, "indication parse failed");
	}

	return res;
}

RES_result_t HLB_apcm_sta_restart_req_send(const comm_handle_t handle, HLB_req_id_t req_id)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_sta_restart_req_create(comm->dest_mac_address, comm->nic_mac_addr, req_id,
											  &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_apcm_sta_restart_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											  HLB_hpgp_result_t *result)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !result) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_APCM_STA_RESTART_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_sta_restart_cnf_parse(&packet, result);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_apcm_net_exit_req_send(const comm_handle_t handle, HLB_req_id_t req_id)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_net_exit_req_create(comm->dest_mac_address, comm->nic_mac_addr, req_id,
										   &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_apcm_net_exit_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
										   HLB_hpgp_result_t *result)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !result) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_APCM_NET_EXIT_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_net_exit_cnf_parse(&packet, result);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_apcm_set_tone_mask_req_send(const comm_handle_t handle, HLB_req_id_t req_id,
											 uint16_t tone_mask)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_set_tone_mask_req_create(comm->dest_mac_address, comm->nic_mac_addr,
												req_id, tone_mask, &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_apcm_set_tone_mask_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
												HLB_hpgp_result_t *result)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !result) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_APCM_SET_TONE_MASK_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_set_tone_mask_cnf_parse(&packet, result);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_apcm_sta_cap_req_send(const comm_handle_t handle, HLB_req_id_t req_id)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_sta_cap_req_create(comm->dest_mac_address, comm->nic_mac_addr, req_id,
										  &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_apcm_sta_cap_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
										  HLB_hpgp_sta_cap_cnf_t *sta_cap_cnf)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !sta_cap_cnf) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_APCM_STA_CAP_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_sta_cap_cnf_parse(&packet, sta_cap_cnf);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_apcm_nw_info_req_send(const comm_handle_t handle, HLB_req_id_t req_id)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_nw_info_req_create(comm->dest_mac_address, comm->nic_mac_addr, req_id,
										  &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_apcm_nw_info_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
										  HLB_hpgp_nw_info_cnf_t *nw_info_cnf)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !nw_info_cnf) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_APCM_NW_INFO_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_nw_info_cnf_parse(&packet, nw_info_cnf);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_apcm_get_beacon_req_send(const comm_handle_t handle, HLB_req_id_t req_id,
										  const HLB_hpgp_nid_t *nid)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm || !nid) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_get_beacon_req_create(comm->dest_mac_address, comm->nic_mac_addr,
											 req_id, nid, &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_apcm_get_beacon_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											 HLB_apcm_get_beacon_cnf_t *get_beacon_cnf)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !get_beacon_cnf) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_APCM_GET_BEACON_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_get_beacon_cnf_parse(&packet, get_beacon_cnf);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_apcm_get_hfid_req_send(const comm_handle_t handle, HLB_req_id_t req_id,
										const HLB_hpgp_get_hfid_req_t *hfid_req)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm || !hfid_req) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_get_hfid_req_create(comm->dest_mac_address, comm->nic_mac_addr,
										   req_id, hfid_req, &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_apcm_get_hfid_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
										   HLB_hpgp_get_hfid_cnf_t *hfid_cnf)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !hfid_cnf) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_APCM_GET_HFID_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_get_hfid_cnf_parse(&packet, hfid_cnf);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_apcm_set_hfid_req_send(const comm_handle_t handle, HLB_req_id_t req_id,
										const HLB_hpgp_hfid_t *hfid)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm || !hfid) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_set_hfid_req_create(comm->dest_mac_address, comm->nic_mac_addr,
										   req_id, hfid, &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_apcm_set_hfid_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
										   HLB_hpgp_result_t *result)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !result) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_APCM_SET_HFID_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_set_hfid_cnf_parse(&packet, result);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_apcm_set_hd_duration_req_send(const comm_handle_t handle, HLB_req_id_t req_id,
											   uint8_t hd_duration)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_set_hd_duration_req_create(comm->dest_mac_address, comm->nic_mac_addr,
												  req_id, hd_duration, &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_apcm_set_hd_duration_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
												  HLB_hpgp_result_t *result)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !result) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_APCM_SET_HD_DURATION_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_set_hd_duration_cnf_parse(&packet, result);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_apcm_unassociated_sta_ind_receive(const comm_handle_t handle, HLB_req_id_t req_id,
												   HLB_hpgp_unassociated_sta_ind_t *unassociated_sta_ind)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !unassociated_sta_ind) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_APCM_UNASSOCIATED_STA_IND,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_unassociated_sta_ind_parse(&packet, unassociated_sta_ind);
		LOG_IF_ERR(res, "indication parse failed");
	}

	return res;
}

RES_result_t HLB_apcm_set_ppkeys_req_send(const comm_handle_t handle, HLB_req_id_t req_id,
										  const HLB_hpgp_set_ppkeys_req_t *ppkeys_req)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm || !ppkeys_req) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_set_ppkeys_req_create(comm->dest_mac_address, comm->nic_mac_addr,
											 req_id, ppkeys_req, &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_apcm_set_ppkeys_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											 HLB_hpgp_result_t *result)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !result) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_APCM_SET_PPKEYS_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_set_ppkeys_cnf_parse(&packet, result);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_apcm_conf_slac_req_send(const comm_handle_t handle, HLB_req_id_t req_id,
										 HLB_hpgp_slac_conf_t slac_conf)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_conf_slac_req_create(comm->dest_mac_address, comm->nic_mac_addr,
											req_id, slac_conf, &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_apcm_conf_slac_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											HLB_hpgp_result_t *result)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !result) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_APCM_CONF_SLAC_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_apcm_conf_slac_cnf_parse(&packet, result);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}
