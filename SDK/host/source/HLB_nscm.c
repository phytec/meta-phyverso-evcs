/********************************************************************
*
* Module Name: HLB_nscm
* Design:
* Implement nscm api
*
********************************************************************/
/*******************************************************************
* IMPORTS
********************************************************************/
#include "HLB_nscm.h"
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
RES_result_t HLB_nscm_link_stats_req_send(const comm_handle_t handle, HLB_req_id_t req_id,
										  const HLB_hpgp_link_stats_req_t *link_stats_req)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm || !link_stats_req) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_link_stats_req_create(comm->dest_mac_address, comm->nic_mac_addr,
											 req_id, link_stats_req, &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_nscm_link_stats_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											 HLB_hpgp_link_stats_cnf_t *link_stats_cnf)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !link_stats_cnf) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_NSCM_LINK_STATS_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_link_stats_cnf_parse(&packet, link_stats_cnf);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_nscm_get_d_link_status_req_send(const comm_handle_t handle, HLB_req_id_t req_id)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_get_d_link_status_req_create(comm->dest_mac_address, comm->nic_mac_addr, req_id,
													&hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_nscm_get_d_link_status_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											        HLB_hpgp_d_link_status_cnf_t *d_link_status)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !d_link_status) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_NSCM_GET_D_LINK_STATUS_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_get_d_link_status_cnf_parse(&packet, d_link_status);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}													

RES_result_t HLB_nscm_get_device_state_req_send(const comm_handle_t handle, HLB_req_id_t req_id)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_get_device_state_req_create(comm->dest_mac_address, comm->nic_mac_addr, req_id,
												   &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_nscm_get_device_state_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											        HLB_hpgp_device_state_cnf_t *device_state)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !device_state) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_NSCM_GET_DEVICE_STATE_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_get_device_state_cnf_parse(&packet, device_state);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_nscm_get_dc_calib_req_send(const comm_handle_t handle, HLB_req_id_t req_id)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_get_dc_calib_req_create(comm->dest_mac_address, comm->nic_mac_addr, req_id,
											   &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_nscm_get_dc_calib_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											   HLB_dc_calib_cnf_t *dc_calib)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !dc_calib) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_NSCM_GET_DC_CALIB_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_get_dc_calib_cnf_parse(&packet, dc_calib);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}											

RES_result_t HLB_nscm_write_mem_req_send(const comm_handle_t handle,
										 HLB_req_id_t req_id,
										 const HLB_write_mem_req_t *write_mem_req)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm || !write_mem_req) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_write_mem_req_create(comm->dest_mac_address, comm->nic_mac_addr,
											write_mem_req, req_id, &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_nscm_write_mem_cnf_receive(const comm_handle_t handle,
											HLB_req_id_t req_id,
											HLB_hpgp_result_t *result)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !result) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_NSCM_WRITE_MEM_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_write_mem_cnf_parse(&packet, result);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}											

RES_result_t HLB_nscm_read_mem_cnf_receive(const comm_handle_t handle,
										   HLB_req_id_t req_id,
										   HLB_read_mem_cnf_t *read_mem_cnf)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !read_mem_cnf) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_NSCM_READ_MEM_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_read_mem_cnf_parse(&packet, read_mem_cnf);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}											

RES_result_t HLB_nscm_read_mem_req_send(const comm_handle_t handle,
										HLB_req_id_t req_id,
										const HLB_read_mem_req_t *read_mem_req)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm || !read_mem_req) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_read_mem_req_create(comm->dest_mac_address, comm->nic_mac_addr,
										   read_mem_req, req_id, &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_nscm_enter_phy_mode_req_send(const comm_handle_t handle, HLB_req_id_t req_id)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_enter_phy_mode_req_create(comm->dest_mac_address, comm->nic_mac_addr, req_id,
												 &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_nscm_enter_phy_mode_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
												 HLB_hpgp_result_t *result)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !result) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_NSCM_ENTER_PHY_MODE_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_enter_phy_mode_cnf_parse(&packet, result);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}																		

RES_result_t HLB_nscm_abort_dump_action_req_send(const comm_handle_t handle, HLB_req_id_t req_id)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_abort_dump_action_req_create(comm->dest_mac_address, comm->nic_mac_addr, req_id,
													&hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_nscm_abort_dump_action_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
													HLB_hpgp_result_t *result)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !result) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_NSCM_ABORT_DUMP_ACTION_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_abort_dump_action_cnf_parse(&packet, result);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_nscm_get_snre_req_send(const comm_handle_t handle, HLB_req_id_t req_id)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_get_snre_req_create(comm->dest_mac_address, comm->nic_mac_addr, req_id,
										   &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_nscm_get_snre_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
										   HLB_snre_info_t *snre_info)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !snre_info) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_NSCM_GET_SNRE_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_get_snre_cnf_parse(&packet, snre_info);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_nscm_get_ce2_info_req_send(const comm_handle_t handle, HLB_req_id_t req_id)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_get_ce2_info_req_create(comm->dest_mac_address, comm->nic_mac_addr, req_id,
											   &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_nscm_get_ce2_info_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											   HLB_ce2_info_t *ce2_info)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !ce2_info) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_NSCM_GET_CE2_INFO_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_get_ce2_info_cnf_parse(&packet, ce2_info);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_nscm_get_ce2_data_req_send(const comm_handle_t handle, HLB_req_id_t req_id,
											const HLB_ce2_data_req_t *data_req)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm || !data_req) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_get_ce2_data_req_create(comm->dest_mac_address, comm->nic_mac_addr,
											   data_req, req_id, &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_nscm_get_ce2_data_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											   HLB_ce2_data_cnf_t *ce2_data)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !ce2_data) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_NSCM_GET_CE2_DATA_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_get_ce2_data_cnf_parse(&packet, ce2_data);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_nscm_get_lnoe_req_send(const comm_handle_t handle, HLB_req_id_t req_id)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_get_lnoe_req_create(comm->dest_mac_address, comm->nic_mac_addr, req_id,
										   &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_nscm_get_lnoe_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
										   HLB_lnoe_info_t *lnoe_info)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !lnoe_info) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_NSCM_GET_LNOE_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_get_lnoe_cnf_parse(&packet, lnoe_info);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_nscm_get_fw_version_req_send(const comm_handle_t handle, HLB_req_id_t req_id)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_get_fw_version_req_create(comm->dest_mac_address, comm->nic_mac_addr, req_id,
												 &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_nscm_get_fw_version_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
												 HLB_fw_version_t *fw_version)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !fw_version) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_NSCM_GET_VERSION_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_get_fw_version_cnf_parse(&packet, fw_version);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_nscm_reset_device_req_send(const comm_handle_t handle,
											HLB_hpgp_reset_device_mode_t reset_mode)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_reset_device_req_create(comm->dest_mac_address, comm->nic_mac_addr,
											   reset_mode, &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_nscm_host_message_status_ind_receive(const comm_handle_t handle, HLB_req_id_t req_id,
													  HLB_hpgp_host_message_status_ind_t *host_message_status_ind)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !host_message_status_ind) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_NSCM_VS_HOST_MESSAGE_STATUS_IND,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_host_message_status_ind_parse(&packet, host_message_status_ind);
		LOG_IF_ERR(res, "indication parse failed");
	}

	return res;
}

RES_result_t HLB_nscm_d_link_ready_ind_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											   HLB_hpgp_d_link_ready_status_ind_t *d_link)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !d_link) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_NSCM_D_LINK_READY_IND,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_d_link_ready_ind_parse(&packet, d_link);
		LOG_IF_ERR(res, "indication parse failed");
	}

	return res;
}

RES_result_t HLB_nscm_d_link_terminate_req_send(const comm_handle_t handle, HLB_req_id_t req_id)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_d_link_terminate_req_create(comm->dest_mac_address, comm->nic_mac_addr, req_id,
												   &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_nscm_d_link_terminate_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_NSCM_D_LINK_TERMINATE_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	return RES_RESULT_OK;
}

RES_result_t HLB_nscm_device_info_req_send(const comm_handle_t handle, HLB_req_id_t req_id)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_device_info_req_create(comm->dest_mac_address, comm->nic_mac_addr,
											  req_id, &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_nscm_device_info_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											  HLB_device_info_t *device_info)
{
	RES_result_t res;
	HLB_packet_t packet;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;

	res = (!comm || !device_info) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
										  HLB_PROTOCOL_MSG_ID_NSCM_DEVICE_INFO_CNF,
										  req_id, &packet);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_device_info_cnf_parse(&packet, device_info);
		LOG_IF_ERR(res, "confirmation parse failed");
	}

	return res;
}

RES_result_t HLB_nscm_get_amp_map_req_send(const comm_handle_t handle, HLB_req_id_t req_id)
{
	HLB_packet_t hlb_packet;
	size_t hlb_packet_len = sizeof(HLB_packet_t);
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_get_amp_map_req_create(comm->dest_mac_address, comm->nic_mac_addr,
											  req_id, &hlb_packet, &hlb_packet_len);
		LOG_IF_ERR(res, "request create failed");
	}

	if (res == RES_RESULT_OK)
	{
		res = ETH_tx(comm->m_eth_handle_t, &hlb_packet, hlb_packet_len);
	}

	return res;
}

RES_result_t HLB_nscm_get_amp_map_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											  HLB_get_amp_map_t *amp_map)
{
	HLB_packet_t packet;
	size_t packet_size;
	uint8_t frag_idx = 0, num_frags, fmsn = 0;
	uint16_t parse_at = 0, parsed_amount;
	HLB_hpgp_communication_t *comm = (HLB_hpgp_communication_t *)handle;
	RES_result_t res;

	res = (!comm || !amp_map) ? RES_RESULT_NULL_PTR : RES_RESULT_OK;

	if (res == RES_RESULT_OK)
	{
		res = HLB_find_and_pop_fragment_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
												   HLB_PROTOCOL_MSG_ID_NSCM_GET_AMP_MAP_CNF,
												   req_id, frag_idx, fmsn, &packet, &packet_size);
		LOG_IF_ERR(res, "Rx db find and pod failed");
	}

	if (res == RES_RESULT_OK)
	{
		num_frags = HLB_PACKET_GET_NUM_FRAGS(&packet);
		fmsn = HLB_PACKET_GET_FSM(&packet);

		res = HLB_nscm_get_amp_map_cnf_parse(&packet, packet_size, amp_map, parse_at, &parsed_amount);
		if (res == RES_RESULT_OK)
		{
			parse_at += parsed_amount;
		}
	}

	if (res == RES_RESULT_OK)
	{
		frag_idx++;
		for (; (frag_idx < num_frags) && (res == RES_RESULT_OK); frag_idx++)
		{
			res = HLB_find_and_pop_fragment_from_rx_db((HLB_rx_db_t *)comm->private_pointer,
													   HLB_PROTOCOL_MSG_ID_NSCM_GET_AMP_MAP_CNF,
													   req_id, frag_idx, fmsn, &packet, &packet_size);
			LOG_IF_ERR(res, "Rx db find and pod failed");

			if (res == RES_RESULT_OK)
			{
				res = HLB_nscm_get_amp_map_cnf_parse(&packet, packet_size, amp_map, parse_at, &parsed_amount);
				if (res == RES_RESULT_OK)
				{
					parse_at += parsed_amount;
				}
			}
		}
	}

	return res;
}
