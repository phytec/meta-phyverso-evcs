/********************************************************************
*
* Module Name: HLB_legacy_commands.c
* Implement fwload_commands API
*
********************************************************************/
/*******************************************************************
* IMPORTS
********************************************************************/
#include "HLB_legacy_commands.h"
#include "osal.h"
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


static RES_result_t HLB_host_eth_tx_rx_packet(HLB_legacy_comm_handler_t *comm,
												uint8_t *Packet, uint16_t Packet_Length,
												host_loading_packet_t *host_loading_packet, size_t *rx_packet_length,
												uint32_t timeout)
{
	host_loading_eth_packet *eth_packet;
	host_loading_eth_packet rx_eth_packet;
	uint8_t Buffer[MAX_PACKET_LENGTH];
	RES_result_t res = RES_RESULT_GENERAL_ERROR;
	uint32_t start_time;
	uint32_t timeout_per_loop = timeout;										 // In milliseconds
	uint32_t left_timeout_per_loop;												 // In milliseconds
	uint8_t broadcast_mac_addr[HMAC_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; /* broadcast MAC address */
	uint8_t host_boot_mac_addr[HMAC_LEN] = {0x00, 0xC5, 0xD9, 0x51, 0x00, 0x00};
	host_loading_packet_t *tx_packet;

	eth_packet = (host_loading_eth_packet *)Packet;

	/* Send the packet */
	ETH_tx(comm->communication->m_eth_handle_t, eth_packet, Packet_Length);
	tx_packet = (host_loading_packet_t *)Packet;

	start_time = get_msectime();

	/* Try to get the reply before sending retransmission */
	while ((left_timeout_per_loop = (get_msectime() - start_time)) < (timeout_per_loop))
	{
		*rx_packet_length = sizeof(host_loading_eth_packet); // Set as the max Packet length
		memset(Buffer, 0x00, MAX_PACKET_LENGTH);
		if ((res = ETH_rx(comm->communication->m_eth_handle_t, (unsigned char *)&Buffer[0], rx_packet_length,
						  timeout_per_loop - left_timeout_per_loop)) == RES_RESULT_OK)
		{

			memcpy(&rx_eth_packet.Layer2Header, (host_loading_eth_packet *)&Buffer[0], sizeof(host_loading_eth_packet));

			rx_eth_packet.Layer2Header.EtherType = betohs(rx_eth_packet.Layer2Header.EtherType);
			if (rx_eth_packet.Layer2Header.EtherType == LEGACY_ETHER_TYPE)
			{
				memcpy(host_loading_packet, &rx_eth_packet.Layer2Header, sizeof(host_loading_packet_t));

				if (host_loading_packet->management_header.msg_id != tx_packet->management_header.msg_id)
				{
					hlb_log_error("Got packet with wrong msg_id, req=%d res=%d", tx_packet->management_header.msg_id, host_loading_packet->management_header.msg_id);
					continue;
				}

				if (host_loading_packet->management_header.session_id != tx_packet->management_header.session_id)
				{
					hlb_log_error("Got packet with wrong session_id, req=%d res=%d", tx_packet->management_header.session_id, host_loading_packet->management_header.session_id);
					continue;
				}

				// If we send a Broadcast msg, allow incoming packets from any MAC-Address
				// If we send a Unicast msg, allow incoming packets only from the relevant MAC-Address
				if ((memcmp(&(eth_packet->Layer2Header.SrcMACAddr[0]), &broadcast_mac_addr[0], HMAC_LEN) == 0) ||
					(memcmp(&(eth_packet->Layer2Header.SrcMACAddr[0]), &rx_eth_packet.Layer2Header.DestMACAddr, HMAC_LEN) == 0) ||
					(memcmp(&(eth_packet->Layer2Header.SrcMACAddr[0]), &host_boot_mac_addr[0], HMAC_LEN) == 0))
				{
					// Check status code
					if (host_loading_packet->management_header.status_code != CP_e_NO_ERROR)
					{
						hlb_log_error("Got packet with error %d", host_loading_packet->management_header.status_code);
						// in case retransmission flag than status is OK
						if (host_loading_packet->management_header.status_code == CP_e_RETRANSMISSION_FLAG)
						{
							hlb_log_info("Got packet with CP_e_RETRANSMISSION_FLAG: SA=%s EtherType=0x%4x Packet_Length=%d\n",
											host_loading_packet->layer_2_header.src_mac_addr,
											host_loading_packet->layer_2_header.eth_type, *rx_packet_length);
						}
						// status code is error
						else if (host_loading_packet->management_header.status_code == CP_e_INTERNAL_ERROR)
						{
							return RES_RESULT_BAD_STATE;
						}
						else
						{
							if (host_loading_packet->management_header.status_code == CP_e_MAP_VSM_BUF_IS_LOCKED)
							{
								res = RES_RESULT_RESOURCE_IN_USE;
							}
							else
							{
								res = RES_RESULT_GENERAL_ERROR;
							}

							return res;
						}
					}
					else
					{
						res = RES_RESULT_OK;
						break;
					}
				}
			}
		}
	}

	if (timeout_per_loop <= left_timeout_per_loop)
	{
		hlb_log_info("send receive: got timeout in send receive (time count: %d)", left_timeout_per_loop);
		res = RES_RESULT_TIMEOUT;
	}
	return res;
}

static RES_result_t HLB_host_loading_update_layer_2_header(uint8_t dest_mac_addr[HMAC_LEN],
															uint8_t src_mac_addr[HMAC_LEN],
															layer_2_header_t *layer_2_header)
{
	memcpy(layer_2_header->dest_mac_addr, dest_mac_addr, HMAC_LEN);
	memcpy(layer_2_header->src_mac_addr, src_mac_addr, HMAC_LEN);
	layer_2_header->eth_type = htobes(LEGACY_ETHER_TYPE);

	return RES_RESULT_OK;
}

static RES_result_t HLB_host_loading_update_spi_general_header(uint8_t did, uint8_t sid,
															host_loading_spi_general_header_t *spi_general_header)
{
	spi_general_header->did = did;
	spi_general_header->sid = sid;
	spi_general_header->priority = LEGACY_MSG_PRIORITY;
	spi_general_header->padding = 0x00;

	return RES_RESULT_OK;
}

static RES_result_t HLB_host_update_management_header(
		uint16_t msg_id,
		uint16_t length,
		host_loading_management_header_t *management_header)
{
	management_header->flags = 0x00;
	management_header->msg_id = htoles(msg_id);
	management_header->session_id = 0;
	management_header->length = htoles(length);
	management_header->status_code = CP_e_NO_ERROR;

	return RES_RESULT_OK;
}

static RES_result_t HLB_host_send_receive_packet_timeout(HLB_legacy_comm_handler_t *comm,
																host_loading_packet_t *tx_host_loading_packet,
																host_loading_packet_t *rx_host_loading_packet,
																size_t *rx_packet_length,
																uint32_t timeout)

{
	uint16_t size;
	RES_result_t res = RES_RESULT_OK;
	
	tx_host_loading_packet->management_header.session_id = comm->session_id;
	size = sizeof(layer_2_header_t) + 2 + sizeof(host_loading_spi_general_header_t) + 
										  	sizeof(host_loading_management_header_t) + 
											tx_host_loading_packet->management_header.length;

	/* Send the packet (and receive answer) */
	res = HLB_host_eth_tx_rx_packet(comm, (uint8_t *)tx_host_loading_packet,
									size, rx_host_loading_packet, rx_packet_length, timeout);
	if (res != RES_RESULT_OK)
	{
		hlb_log_error("HLB_host_eth_tx_rx_packet failed, res=%d", res);
		return res;
	}
	if(tx_host_loading_packet->management_header.session_id != rx_host_loading_packet->management_header.session_id)
	{
		hlb_log_error("HLB_host_send_receive_packet_timeout: Rx and Tx session id mismatch\n");
		return RES_RESULT_BAD_STATE;
	}

	comm->session_id++;
	return res;
}

/*******************************************************************
* EXPORTED FUNCTIONS
********************************************************************/
RES_result_t HLB_write_to_mem(HLB_legacy_comm_handler_t *comm, uint32_t address, uint32_t size, uint8_t *buffer, uint32_t timeout)
{

	RES_result_t res;
	host_loading_packet_t tx_packet;
	host_loading_packet_t rx_packet;
	HLB_set_memory_t *set_memory;

	uint32_t offset_index = 0;
	uint32_t block_size;
	size_t rx_length;
	uint16_t msg_id = VSM_MSG_SET_MEM;

	do
	{
		block_size = MIN(size, BUFFER_PAYLOAD_LIMIT);
		HLB_host_loading_update_layer_2_header(comm->communication->dest_mac_address,
												comm->communication->nic_mac_addr,
												&tx_packet.layer_2_header);

		HLB_host_loading_update_spi_general_header(0, 0, &tx_packet.spi_general_header);
		HLB_host_update_management_header(msg_id, 2 * sizeof(uint32_t) + block_size, &tx_packet.management_header);

		set_memory = (HLB_set_memory_t *)tx_packet.payload;
		set_memory->address = address + offset_index;
		set_memory->size = block_size;
		memcpy(set_memory->buffer, &buffer[offset_index], block_size);

		set_memory->address = htolel(set_memory->address);
		set_memory->size = htolel(set_memory->size);

		if (timeout != 0U)
		{
			res = HLB_host_send_receive_packet_timeout(comm, &tx_packet, &rx_packet, &rx_length, timeout);
			if (res != RES_RESULT_OK)
			{
				hlb_log_error("HLB_write_to_mem: HLB_host_send_receive_packet failed\n");
				return res;
			}
		}
		else
		{
			res = ETH_tx(comm->communication->m_eth_handle_t, (uint8_t *)&tx_packet,
						 offsetof(host_loading_packet_t, payload) + tx_packet.management_header.length);
			if (res != RES_RESULT_OK)
			{
				hlb_log_error("HLB_write_to_mem: ETH_tx failed\n");
				return res;
			}
		}

		offset_index = offset_index + block_size;
		size = size - block_size;

	} while (size > 0);

	return res;
}

RES_result_t HLB_query_device(HLB_legacy_comm_handler_t *comm, HLB_query_device_t *query_device, uint32_t timeout_msec)
{
	RES_result_t res = RES_RESULT_OK;
	host_loading_packet_t host_loading_packet;
	host_loading_packet_t rx_host_loading_packet;
	uint8_t did = 0;
	uint8_t sid = 0;
	size_t rx_length;
	uint16_t msg_id = VSM_MSG_QUERY_DEVICE;
	HLB_query_device_t temp_query_device;

	memset(&host_loading_packet, 0x00, sizeof(host_loading_packet_t));
	HLB_host_loading_update_layer_2_header(comm->communication->dest_mac_address,
											comm->communication->nic_mac_addr,
											&host_loading_packet.layer_2_header);

	HLB_host_loading_update_spi_general_header(did, sid, &host_loading_packet.spi_general_header);
	HLB_host_update_management_header(msg_id, 0, &host_loading_packet.management_header);

	res = HLB_host_send_receive_packet_timeout(comm, &host_loading_packet,
												&rx_host_loading_packet,
												&rx_length, timeout_msec);

	if (res != RES_RESULT_OK)
	{
		hlb_log_error("HLB_query_device: HLB_host_send_receive_packet failed\n");
		return res;
	}
	else
	{
		hlb_log_error("HLB_query_device: HLB_host_send_receive_packet succeeded\n");
		/* Query succeeded */
		if (rx_host_loading_packet.management_header.length == sizeof(HLB_query_device_t))
		{

			/* Copy data from the payload returned from the device to the query struct */
			memcpy(&temp_query_device, rx_host_loading_packet.payload, rx_host_loading_packet.management_header.length);
			memcpy(query_device->build_date, temp_query_device.build_date, sizeof(temp_query_device.build_date));
			memcpy(query_device->build_time, temp_query_device.build_time, sizeof(temp_query_device.build_time));
			memcpy(query_device->au8MACAddr, temp_query_device.au8MACAddr, sizeof(temp_query_device.au8MACAddr));
			query_device->DeviceState = temp_query_device.DeviceState;
			query_device->svn_rev_ver = temp_query_device.svn_rev_ver;
			memcpy(query_device->sw_version, temp_query_device.sw_version, sizeof(temp_query_device.sw_version));
		}
		else
		{
			hlb_log_error("HLB_query_device: rx payload size received : %d, shoud be: %d\n",
							rx_host_loading_packet.management_header.length, sizeof(HLB_query_device_t));
			res = RES_RESULT_GENERAL_ERROR;
		}
	}
	return res;
}

RES_result_t HLB_set_image_data(HLB_legacy_comm_handler_t *comm,
										uint32_t address,
										uint32_t size,
										uint8_t *ptr)
{
	RES_result_t res = RES_RESULT_OK;
	host_loading_packet_t tx_packet;
	host_loading_packet_t rx_packet;
	uint16_t msg_id = VSM_MSG_SET_IMAGE_DATA;
	HLB_set_image_data_t *set_img_data = (HLB_set_image_data_t *)tx_packet.payload;
	size_t rx_length;

	HLB_host_loading_update_layer_2_header(comm->communication->dest_mac_address,
											comm->communication->nic_mac_addr,
											&tx_packet.layer_2_header);

	HLB_host_loading_update_spi_general_header(0, 0, &tx_packet.spi_general_header);
	HLB_host_update_management_header(msg_id, 2 * sizeof(uint32_t) + size,
										&tx_packet.management_header);

	memcpy(set_img_data->Payload, ptr, size);

	set_img_data->Address = htolel(address);
	set_img_data->Size = htolel(size);

	res = HLB_host_send_receive_packet_timeout(comm, &tx_packet, &rx_packet, &rx_length, TIMEOUT_MSEC);
	if (res != RES_RESULT_OK)
	{
		hlb_log_error("HLB_set_image_data: HLB_host_send_receive_packet_timeout failed\n");
	}
	return res;
}

RES_result_t HLB_set_image_header(HLB_legacy_comm_handler_t *comm,
											BIN_image_header_t *ptr)
{
	RES_result_t res = RES_RESULT_OK;
	host_loading_packet_t tx_packet;
	host_loading_packet_t rx_packet;
	size_t rx_length;
	uint16_t msg_id = VSM_MSG_SET_IMAGE_HEADER;
	BIN_image_header_t *BIN_image_header = (BIN_image_header_t *)tx_packet.payload;

	HLB_host_loading_update_layer_2_header(comm->communication->dest_mac_address,
											comm->communication->nic_mac_addr,
											&tx_packet.layer_2_header);
	HLB_host_loading_update_spi_general_header(0, 0, &tx_packet.spi_general_header);
	HLB_host_update_management_header(msg_id, sizeof(BIN_image_header_t), &tx_packet.management_header);

	/* fill  message payload */
	memcpy(BIN_image_header, ptr, sizeof(BIN_image_header_t));

	res = HLB_host_send_receive_packet_timeout(comm, &tx_packet, &rx_packet, &rx_length, TIMEOUT_MSEC);
	if (res != RES_RESULT_OK)
	{
		hlb_log_error("HLB_set_image_header: HLB_host_send_receive_packet_timeout failed\n");
	}
	return res;
}

RES_result_t HLB_init_copy(HLB_legacy_comm_handler_t *comm)
{
	RES_result_t res;
	host_loading_packet_t tx_packet;
	host_loading_packet_t rx_packet;
	uint16_t msg_id = VSM_MSG_INIT_COPY;	
	size_t rx_length;

	memset(&tx_packet, 0x00, sizeof(host_loading_packet_t));

	HLB_host_loading_update_layer_2_header(comm->communication->dest_mac_address, comm->communication->nic_mac_addr, &tx_packet.layer_2_header);
	HLB_host_loading_update_spi_general_header(0, 0, &tx_packet.spi_general_header);
	HLB_host_update_management_header(msg_id, 0, &tx_packet.management_header);

	res = HLB_host_send_receive_packet_timeout(comm, &tx_packet, &rx_packet, &rx_length, TIMEOUT_MSEC);
	if (res != RES_RESULT_OK)
	{
		hlb_log_error("HLB_init_copy: HLB_host_send_receive_packet_timeout failed\n");
	}
	return res;
}

RES_result_t HLB_execute_command(HLB_legacy_comm_handler_t *comm,
												   uint32_t address,
												   uint32_t ignore_checksum,
												   uint32_t image_checksum,
												   uint32_t cpu_id)
{
	RES_result_t res = RES_RESULT_OK;
	host_loading_packet_t tx_packet;
	host_loading_packet_t rx_packet;
	size_t rx_length;
	uint16_t msg_id = VSM_MSG_EXECUTE_CMD;
	HLB_execute_command_t *exec_command = (HLB_execute_command_t *)tx_packet.payload;

	memset(&tx_packet, 0x00, sizeof(host_loading_packet_t));

	HLB_host_loading_update_layer_2_header(comm->communication->dest_mac_address, comm->communication->nic_mac_addr, &tx_packet.layer_2_header);
	HLB_host_loading_update_spi_general_header(0, 0, &tx_packet.spi_general_header);
	HLB_host_update_management_header(msg_id, sizeof(HLB_execute_command_t), &tx_packet.management_header);

	exec_command->Address = htolel(address);
	exec_command->ignore_checksum = htolel(ignore_checksum);
	exec_command->image_checksum = htolel(image_checksum);
	exec_command->cpu_id = htolel(cpu_id);

	res = HLB_host_send_receive_packet_timeout(comm, &tx_packet, &rx_packet, &rx_length, TIMEOUT_MSEC);

	if (cpu_id == 1)
	{
		res = RES_RESULT_OK;
	}

	if (res != RES_RESULT_OK)
	{
		hlb_log_error("HLB_execute_command: execute command error (status=%d)\n", res);
	}
	return res;
}

RES_result_t HLB_decompress_command(HLB_legacy_comm_handler_t *comm,
											uint32_t src_addr,
											uint32_t dest_addr)

{

	RES_result_t res = RES_RESULT_OK;
	host_loading_packet_t tx_packet;
	host_loading_packet_t rx_packet;
	uint16_t msg_id = VSM_MSG_DECOMPRESS;
	HLB_decompress_command_t decompress_msg;
	size_t rx_length;

	HLB_host_loading_update_layer_2_header(comm->communication->dest_mac_address, comm->communication->nic_mac_addr, &tx_packet.layer_2_header);
	HLB_host_loading_update_spi_general_header(0, 0, &tx_packet.spi_general_header);
	HLB_host_update_management_header(msg_id, sizeof(HLB_decompress_command_t), &tx_packet.management_header);

	decompress_msg.src_addr = htolel(src_addr);
	decompress_msg.dest_addr = htolel(dest_addr);

	memcpy(&tx_packet.payload, &decompress_msg, sizeof(HLB_decompress_command_t));
	res = HLB_host_send_receive_packet_timeout(comm, &tx_packet, &rx_packet, &rx_length, TIMEOUT_MSEC);
	if (res != RES_RESULT_OK)
	{
		hlb_log_error("HLB_decompress_command: HLB_host_send_receive_packet_timeout failed\n");
	}
	return res;
}
