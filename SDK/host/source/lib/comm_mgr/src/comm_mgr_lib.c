#include <string.h>
#include "comm_mgr_lib.h"
#include "osal.h"
#include "mem_pool.h"

RES_result_t HLB_init_communication(const uint8_t *adapter_mac_address, const uint8_t *dest_mac_address, HLB_hpgp_communication_t *communication)
{
	RES_result_t cg_stat;
	if (communication == NULL)
	{
		return RES_RESULT_NULL_PTR;
	}
	communication->m_eth_handle_t = malloc(ETH_handle_alloc_size());
	if (communication->m_eth_handle_t == NULL)
	{
		return RES_RESULT_NO_MEMORY;
	}

	// Bind Adapter
	if ((cg_stat = CM_connect(adapter_mac_address, dest_mac_address, ETHER_TYPE, communication->m_eth_handle_t)) != RES_RESULT_OK)
	{
		hlb_log_error("failed to connect device (stat=%d)\n", cg_stat);
		return RES_RESULT_GENERAL_ERROR;
	}

	memcpy(communication->nic_mac_addr, adapter_mac_address, MAC_LEN);
	if(!dest_mac_address)
	{
		memset(communication->dest_mac_address, 0xFF, MAC_LEN);
	}
	else
	{
		memcpy(communication->dest_mac_address, dest_mac_address, MAC_LEN);
	}

	return RES_RESULT_OK;
}

void HLB_deinit_communication(const HLB_hpgp_communication_t *communication)
{
	CM_disconnect(communication->m_eth_handle_t);
	free(communication->m_eth_handle_t);
}
