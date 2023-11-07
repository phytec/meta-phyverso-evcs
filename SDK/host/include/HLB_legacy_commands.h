/**
 * @file HLB_legacy_commands.h
 * @author Daniel Varennikov
 * @date 22 August 2021
 * @brief File containing header for fw loading commands
 *
 */
/********************************************************************
*
* Name: HLB FW load commands
* Title: HLB FW load commands
* Package title: host library
* Abstract: provide the commands needed
* for the FW loading
*
*
********************************************************************/
#ifndef HLB_legacy_commands_H
#define HLB_legacy_commands_H
/********************************************************************
* IMPORTS
********************************************************************/
#include "common.h"
#include "shared.h"
#include "comm_mgr_lib.h"
/********************************************************************
* EXPORTED CONSTANTS
********************************************************************/
#define VSM_MSG_SET_MEM 1					/*!< VSM_MSG_SET_MEM message id */
#define VSM_MSG_QUERY_DEVICE 3				/*!< VSM_MSG_QUERY_DEVICE message id */
#define VSM_MSG_SET_IMAGE_HEADER 14			/*!< VSM_MSG_SET_IMAGE_HEADER message id */
#define VSM_MSG_EXECUTE_CMD 16				/*!< VSM_MSG_EXECUTE_CMD message id */
#define VSM_MSG_SET_IMAGE_DATA 19			/*!< VSM_MSG_SET_IMAGE_DATA message id */
#define VSM_MSG_INIT_COPY 31				/*!< VSM_MSG_INIT_COPY message id */
#define VSM_MSG_DECOMPRESS 32				/*!< VSM_MSG_DECOMPRESS message id */

#define BUFFER_PAYLOAD_LIMIT (1460U - 8U)	/*!< The maximal buffer size over eth */
#define TIMEOUT_MSEC 1000					/*!< The amount of time to wait before declaring timeout when receiving */
#define LEGACY_MSG_PRIORITY 4				/*!< Legacy */
#define LEGACY_ETHER_TYPE 0x1200			/*!< Legacy */
#define ETH_MAX_PAYLOAD_SIZE 1500			/*!< The maximal payload size over eth */
#define MAX_PACKET_LENGTH 1514				/*!< The maximal packet size over eth */
#define MAX_VERSION_NAME_LEN 24				/*!< The maximal version name length */
#define BROADCAST_MAC_ADDRESS {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
/********************************************************************
* EXPORTED TYPES
********************************************************************/
typedef enum {

	CP_e_NO_ERROR = 0,
	CP_e_RETVAL_FAIL = 1, 
	CP_e_INVALID_REQ  =2 ,
	CP_e_MEM_ERROR =3,
	CP_e_INVALID_MODE=4 ,
	CP_e_INTERNAL_ERROR =5,
	CP_e_RSP_MAX_LEN_EXCEEDED=6,
	CP_e_RETRANSMISSION_FLAG =7,
	CP_e_SP_DM_NOT_CAPABLE = 20,   	  /*!< device Strapping is not DM capable */
	CP_e_MAP_VSM_BUF_IS_LOCKED=21,    /*!< VSM MAP buffer is locked either for read or write */
	CP_e_MSG_LENGTH_IS_INVALID=22,    /*!< new map size is greater than FW_PRM_MAX_MAP_SIZE, or ch param config message has invalid msg length */
	CP_e_MAP_CHECKSUM_ERR=23,         /*!< map checksum error */
	CP_e_MAP_MNGR_BUF_IS_NOT_READY=24,/*!< MAP VSM buffer has not copied yet to map manager use */
	CP_e_MAP_VSM_BUF_IS_INVALID =25   /*!< VSM buffer is not valid. */

} CP_error_codes_t;

/*! SPI connections Structure */
PACK(
	uint8_t			did;									/* Destination Host ID */
	uint8_t			sid;									/* Source Host ID */
	uint8_t			priority;								/* Stream Priority identifier */
	uint8_t			padding;								/*!< Padding to 32bit alignment */
 ) host_loading_spi_general_header_t;

/*! Host Loading management header Structure */
PACK(
	uint8_t			flags;									/*!< Control Flags (Req/Resp bit) */
	uint16_t		msg_id;									/*!< Unique ID of the message */
	uint16_t		session_id;								/*!< session ID used to associate Resp with Req */
	uint16_t		length;									/*!< Message Payload length */
	uint8_t			status_code;							/*!< Optional response code, mainly for errors */
) host_loading_management_header_t;

/*! Host Loading packet Structure */
PACK(
	layer_2_header_t layer_2_header;					 /*!< Ethernet Layer 2 Header */
	uint8_t	padding[2];									 /*!< Padding */
	host_loading_spi_general_header_t spi_general_header;/*!< SPI header */
	host_loading_management_header_t management_header;	 /*!< managemnt header */
	uint8_t	payload[MSG_PAYLOAD_LIMIT];					 /*!< Ethernet payload */
) host_loading_packet_t;

/*! Ethernet Layer 2 Header Structure */
PACK(
	uint8_t DestMACAddr[HMAC_LEN]; /*!< Destination MAC address */
	uint8_t SrcMACAddr[HMAC_LEN];  /*!< Source MAC address */
	uint16_t EtherType;			   /*!< Ethernet type */
) host_loading_layer2header;

/*! Ethernet Packet */
PACK(
	host_loading_layer2header Layer2Header;			   /*!< Ethernet Layer 2 Header */
	uint8_t Payload[ETH_MAX_PAYLOAD_SIZE]; /*!< Ethernet payload */
) host_loading_eth_packet;

/*! The payload of VSM_MSG_SET_MEM */
PACK(
	uint32_t		address;						/*!<  Address */
	uint32_t		size;							/*!<  Size  */
	uint8_t			buffer[BUFFER_PAYLOAD_LIMIT];	/*!<  Buffer */
) HLB_set_memory_t;

/*! The payload of the response from VSM_MSG_QUERY_DEVICE */
PACK(
	uint8_t au8MACAddr[HMAC_LEN]; /*!< Device MAC address */
	uint8_t DeviceState;		  /*!< Device State */
	uint8_t	is_new_vsm_format;	  /*!< 0 old VSM format, 1 new VSM format */
	uint32_t OptionFlags;		  /*!< Option Flags */
	uint32_t svn_rev_ver;		  /*!< SVN revision version */
	char sw_version[32];		  /*!< FW Version */
	char build_date[16];		  /*!< Build Date */
	char build_time[16];		  /*!< BuildTime */
) HLB_query_device_t;

/*! The payload of VSM_MSG_SET_IMAGE_DATA  */
PACK(
	uint32_t Address;					   /*!< Address */
	uint32_t Size;						   /*!< Size */
	uint8_t Payload[BUFFER_PAYLOAD_LIMIT]; /*!< Payload */
) HLB_set_image_data_t;

/*! The payload of VSM_MSG_EXECUTE_CMD */
PACK(
	uint32_t Address;					/*!< The address to start the execution from */
	uint32_t ignore_checksum;			/*!< Ignore checksum */
	uint32_t image_checksum;			/*!< The checksum of the image */
	uint32_t cpu_id;					/*!< The id of the CPU to execute */
) HLB_execute_command_t;

/*! The payload of VSM_MSG_DECOMPRESS  */
PACK(
	uint32_t src_addr;					/*!< The source of the data to decompress */
	uint32_t dest_addr;					/*!< The destination of the decompressed data */
) HLB_decompress_command_t;

/*! The payload struct of VSM_MSG_SET_IMAGE_HEADER */
PACK(
	uint8_t version[MAX_VERSION_NAME_LEN];	/*!< Version */
	uint32_t cpu_id;						/*!< The CPU id */
	uint32_t numOfSections;					/*!< The number of sections */
	uint32_t total_image_size;				/*!< The size of the image */
	uint32_t checksum;						/*!< checksum */
) BIN_image_header_t;

/*! The legacy communication handler struct */
typedef struct
{
	uint32_t session_id;						/*!< The id of the session */
	HLB_hpgp_communication_t *communication;	/*!< The communication struct */
} HLB_legacy_comm_handler_t;
/********************************************************************
* EXPORTED MACROS
********************************************************************/
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/

/**
 *   Function Name: HLB_write_to_mem
 *
 *  @brief Description: write to the memory of the modem
 *
 *  @param  input : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input : address - The address in the modem to write to.
 *  @param  input : size - The amount to be written.
 *  @param  input : buffer - The data to be written.
 *  @param  input : timeout - The timeout for the confirmation from the modem, or 0 if reply not expected.
 *
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_write_to_mem(HLB_legacy_comm_handler_t *handle, uint32_t address, uint32_t size, uint8_t *buffer, uint32_t timeout);

/**
 *   Function Name: HLB_query_device
 *
 *  @brief Description: Get information about the device
 *
 *  @param  input : handle - the communication handle to the device
 *  @param  input : timeout_ms - timeout in ms to wait for the cnf
 *  @param  output : query_device -  the struct containing information about the device
 *  @return RES_result_t : result status code
 *
 */
RES_result_t HLB_query_device(HLB_legacy_comm_handler_t *handle, HLB_query_device_t *query_device, uint32_t timeout_ms);

/**
 *   Function Name: HLB_set_image_data
 *
 *  @brief Description: update the cpu image data
 * on the firmware from the BIN file data
 *
 *  @param  input : handler - the communication handler
 *  @param  input : address - the address of the data
 *  @param  input : size - the size of the data
 *  @param  input : ptr - the data itself
 *  @return RES_result_t : result status code
 *
 */
RES_result_t HLB_set_image_data(HLB_legacy_comm_handler_t *handler,
										uint32_t address,
										uint32_t size,
										uint8_t *ptr);

/**
 *   Function Name: HLB_set_image_header
 *
 *  @brief Description: update the cpu image header
 * on the firmware from the BIN file header
 *
 *  @param  input : handler - the communication handler
 *  @param  input : ptr - the BIN file header
 *  @return RES_result_t : result status code
 *
 */
RES_result_t HLB_set_image_header(HLB_legacy_comm_handler_t *handler,
											BIN_image_header_t *ptr);

/**
 *   Function Name: HLB_init_copy
 *
 *  @brief Description: Send an VSM_MSG_INIT_COPY packet to the modem
 *
 *  @param  input : handler - the communication handler
 *  @return RES_result_t : result status code
 *
 */
RES_result_t HLB_init_copy(HLB_legacy_comm_handler_t *handler);

/**
 *   Function Name: HLB_execute_command
 *
 *  @brief Description: Send an VSM_MSG_EXECUTE_CMD packet to the modem
 *
 *  @param  input : handler - the communication handler
 *  @param  input : ignore_checksum
 *  @param  input : image_checksum
 *  @param  input : cpu_id
 *  @return input : result status code
 *
 */
RES_result_t HLB_execute_command(HLB_legacy_comm_handler_t *handler,
												   uint32_t address,
												   uint32_t ignore_checksum,
												   uint32_t image_checksum,
												   uint32_t cpu_id);

/**
 *   Function Name: HLB_decompress_command
 *
 *  @brief Description: Send an VSM_MSG_DECOMPRESS packet to the modem
 *
 *  @param  input : handler - the communication handler
 *  @param  input : src_addr
 *  @param  input : dest_addr
 *  @return input : result status code
 *
 */
RES_result_t HLB_decompress_command(HLB_legacy_comm_handler_t *handler,
											uint32_t src_addr,
											uint32_t dest_addr);

#endif