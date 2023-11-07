/**
 * @file HLB_helper.h
 * @author Orr Mazor
 * @date 27 April 2021
 * @brief File containing header for helper functions
 *
 *
 *
 * This module includes rx_db handling


 */
/********************************************************************
*
* Name: HLB helper functions
* Title: HLB helper functions
* Package title: host library
* Abstract: provide helper functions and structs,
* such as the rx db and stack implementation
*
*
********************************************************************/
#ifndef HLB_HELPER_H
#define HLB_HELPER_H
/********************************************************************
* IMPORTS
********************************************************************/
#include "HLB_apcm.h"
#include "osal.h"
#include <stdbool.h>
/********************************************************************
* EXPORTED CONSTANTS
********************************************************************/
#define INDICATION_DB_TIMEOUT 5
#define CLEANER_THREAD_SLEEP 1
/********************************************************************
* EXPORTED TYPES
********************************************************************/
typedef struct HLB_rx_db HLB_rx_db_t;
typedef struct
{
	HLB_packet_t packet;
	uint64_t timestamp_secs;
} HLB_timestamped_packet_t;
/********************************************************************
* EXPORTED MACROS
********************************************************************/
/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/
/**
 *   Function Name: HLB_init_rx_db
 *
 * @brief Description: this function creates a mempool
 * for HLB_packets
 *
 *
 *  @param  input : area_size - Preallocated size to use
 *  @return rx_db if Ok ,null if error.
 *
 *
 */
HLB_rx_db_t *HLB_init_rx_db(size_t area_size);

/**
 *   Function Name: HLB_push_to_rx_db
 *
 * @brief Description: this function allows you to
 * push a packet into the global mempool
 *
 *  @param  input : rx_db - the rx database pointer
 *  @param  input : packet - packet to push
 *  @param  input : packet_size - size of packet
 *  @return status 0 Ok ,<0 error.
 *
 *
 */
RES_result_t HLB_push_to_rx_db(HLB_rx_db_t *rx_db, HLB_packet_t *packet, uint16_t packet_size);

/**
 *   Function Name: HLB_get_req_id_of_fragmented_msg
 *
 * @brief Description: this function allows you to
 * find the request id of series of fragmented messages based on msg id and FMSN.
 * (The request id appears inside the first fragment only)
 *
 *  @param  input : rx_db - the rx database pointer
 *  @param  input  : msg_id - message id of the packet we are searching
 *  @param  input  : fmsn - Fragmentation Message Sequence Number
 *  @param  output : req_id - request id of the packet
 *  @return status 0 Ok ,<0 error.
 *
 *
 */
RES_result_t HLB_get_req_id_of_fragmented_msg(HLB_rx_db_t *rx_db,
											  HLB_msg_id_t msg_id, uint8_t fmsn,
											  HLB_req_id_t *req_id);

/**
 *   Function Name: HLB_find_and_pop_fragment_from_rx_db
 *
 * @brief Description: this function allows you to
 * find a packet that may be fragmented in the global mempool and pop it from it
 *
 *  @param  input : rx_db - the rx database pointer
 *  @param  input  : msg_id - message id of the packet we are searching
 *  @param  input  : req_id - request id of the packet we are searching (used when frag_idx is 0)
 *  @param  input  : frag_idx - Fragment Number (FN_MI) of the MMENTRY
 *  @param  input  : fmsn - Fragmentation Message Sequence Number
 *  @param  output : packet - copy of the packet found
 *  @param  output : packet_size - size of the packet found
 *  @return status 0 Ok ,<0 error.
 *
 *
 */
RES_result_t HLB_find_and_pop_fragment_from_rx_db(HLB_rx_db_t *rx_db,
												  HLB_protocol_msg_id_t msg_id,
												  HLB_req_id_t req_id,
												  uint8_t frag_idx, uint8_t fmsn,
												  HLB_packet_t *packet, size_t *packet_size);

/**
 *   Function Name: HLB_find_and_pop_from_rx_db
 *
 * @brief Description: this function allows you to
 * find a packet in the global mempool and pop it from it
 *
 *  @param  input : rx_db - the rx database pointer
 *  @param  input  : msg_id - message id of the packet we are searching
 *  @param  input  : req_id - request id of the packet we are searching
 *  @param  output : packet - copy of the packet found
 *  @return status 0 Ok ,<0 error.
 *
 *
 */
RES_result_t HLB_find_and_pop_from_rx_db(HLB_rx_db_t *rx_db, HLB_protocol_msg_id_t msg_id,
										 HLB_req_id_t req_id, HLB_packet_t *packet);

/**
 *   Function Name: HLB_free_rx_db
 *  @param  input : rx_db - the rx database pointer
 *  @brief Description: free the mempool
 *
 */
void HLB_free_rx_db(HLB_rx_db_t *rx_db);

/**
 *   Function Name: HLB_list_rx_db_main_db_of_msg_and_req_id
 *
 * @brief Description: this function return a list of
 * msg and req id of the packets in the rx db by order
 *
 *  @param  input : rx_db - the rx database pointer
 *  @param  output : req_id_arr - array to put req_id on
 *  @param  output : msg_id_arr - array to put msg_id on
 *  @param  input  : size_of_array - size of the array
 *  @return number of entries field (min(number of entries in rxdb, size of array))
 *
 *
 */
size_t HLB_list_rx_db_main_db_of_msg_and_req_id(HLB_rx_db_t *rx_db, HLB_req_id_t *req_id_arr, HLB_msg_id_t *msg_id_arr, size_t size_of_array);

/**
 *   Function Name: HLB_list_rx_db_main_db
 *
 * @brief Description: this function return a list of
 * the packets in the rx db main_db by order
 *
 *  @param  input : rx_db - the rx database pointer
 *  @param  output : arr - array to put packets in
 *  @param  input  : size_of_array - size of the array
 *  @return number of entries field (min(number of entries in rxdb, size of array))
 *
 *
 */
size_t HLB_list_rx_db_main_db(HLB_rx_db_t *rx_db, HLB_packet_t *arr, size_t size_of_array);

/**
 *   Function Name: HLB_get_packet_capacity_rx_db_main_db
 *
 * @brief Description: this function returns the
 * amount of packet slots in the main db of rx_db.
 *
 *  @param  input : rx_db - the rx database pointer
 *  @return size of rx_db main_db
 *
 *
 */
size_t HLB_get_packet_capacity_rx_db_main_db(HLB_rx_db_t *rx_db);


/**
 *   Function Name: HLB_get_packet_capacity_rx_db_indication_db
 *
 * @brief Description: this function returns the  
 * amount of packet slots in the indication db of rx_db.
 *
 *  @param  input : rx_db - the rx database pointer
 *  @return size of rx_db indication_db
 *
 *
 */
size_t HLB_get_packet_capacity_rx_db_indication_db(HLB_rx_db_t *rx_db);

/**
 *   Function Name: HLB_list_rx_db_indication_db
 *
 * @brief Description: this function return a list of
 * the packets in the rx db indication_db by order
 *
 *  @param  input : rx_db - the rx database pointer
 *  @param  output : arr - array to put packets in
 *  @param  input  : size_of_array - size of the array
 *  @return number of entries field (min(number of entries in rxdb, size of array))
 *
 *
 */
size_t HLB_list_rx_db_indication_db(HLB_rx_db_t *rx_db, HLB_timestamped_packet_t *arr,
                                    size_t size_of_array);

/**
 *   Function Name: HLB_get_remaining_space_rx_db_indication_db
 *
 * @brief Description: Return the remaining size (in packets)
 * of indication_db
 *
 *  @param  input : rx_db - the rx database pointer
 *  @return the remaining size
 *
 *
 */
size_t HLB_get_remaining_space_rx_db_indication_db(HLB_rx_db_t *rx_db);
/********************************************************************
* INTERNAL FUNCTIONS DECLARATIONS (FOR UNIT TESTING)
********************************************************************/

#endif // HLB_HELPER_H
