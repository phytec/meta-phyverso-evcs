/**
 * @file comm_mgr_lib.h
 * @author Orr Mazor
 * @brief File containing init/deinit of communication
 *
 */
#ifndef COMM_MGR_LIB_H
#define COMM_MGR_LIB_H

#include "common.h"
#include <stdint.h>
#include <stddef.h>

/* Communication parameters struct */
typedef struct HLB_hpgp_communication
{
	uint8_t nic_mac_addr[MAC_LEN];		/*!< NIC mac address */
	uint8_t dest_mac_address[MAC_LEN];  /*!< device mac address */
	void *m_eth_handle_t;				/*!< communication handle */
	void *private_pointer;				/*!< to be used by user */
} HLB_hpgp_communication_t;

/**
 *   Function Name: HLB_init_communication
 *
 * @brief Description: this function starts communication between
 * the adapter and the destination
 *
 *  @param  input  : adapter_mac_address
 *  @param  input  : dest_mac_address
 *  @param  output : communication - Communication parameters struct
 *  @return status 0 Ok ,<0 error.
 *
 *
 */
RES_result_t HLB_init_communication(const uint8_t *adapter_mac_address, const uint8_t *dest_mac_address, HLB_hpgp_communication_t *communication);

/**
 *   Function Name: HLB_deinit_communication
 *
 * @brief Description: this function stops the communication between
 * the adapter and the destination
 *
 *  @param  input  : communication - Communication parameters struct
 *
 *
 */
void HLB_deinit_communication(const HLB_hpgp_communication_t *communication);

#endif // COMM_MGR_LIB_H