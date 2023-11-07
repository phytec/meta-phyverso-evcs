/**
 * @file HLB_ind_listener.h
 * @author Daniel Varennikov
 * @date 02 January 2022
 * @brief File containing the API for the indication listener
 *
 *
 *
 */
#ifndef HLB_IND_LISTENER_H
#define HLB_IND_LISTENER_H
/********************************************************************
* IMPORTS
********************************************************************/
#include "comm_mgr_lib.h"
#include "common.h"
#include "shared.h"
#include "HLB_protocol.h"
#include <stdbool.h>
/*******************************************************************
* CONSTANTS
********************************************************************/
/*******************************************************************
* STATIC and GLOBAL DATA
********************************************************************/
#define SUPPORTED_IND_MESSAGES 3
#define SUPPORTED_PENDING_MESSAGES 20
/*******************************************************************
* TYPES
********************************************************************/

typedef struct
{
	HLB_packet_t packet[SUPPORTED_PENDING_MESSAGES];
    int head;
    int tail;
    bool full;
	void *lock;
} HLB_pending_messages_list_t;

typedef struct
{
    bool filled;
    HLB_protocol_msg_id_t msg_id;
    void *callback;
} HLB_registered_message_t;

typedef struct
{
    HLB_registered_message_t registered_message[SUPPORTED_IND_MESSAGES];
    void *lock;
} HLB_registered_messages_t;

typedef struct
{
	HLB_hpgp_communication_t *com;
    HLB_pending_messages_list_t *pending_messages;
    HLB_registered_messages_t *registered_messages;
    void *cond_lock;
} HLB_rx_handler_struct_t;

typedef struct
{
	HLB_pending_messages_list_t *pending_messages;
    HLB_registered_messages_t *registered_messages;
    void *cond_lock;
} HLB_message_executor_handler_struct_t;

typedef struct
{
    HLB_rx_handler_struct_t *rx_handler;
    void *rx_thread_handle;
    HLB_message_executor_handler_struct_t *message_executor_handler;
    void *message_executor_thread_handle;
} HLB_ind_listener_handler_struct_t;

//Declaration of static data and global data
/*******************************************************************
* MACROS
********************************************************************/
/*******************************************************************
* INTERNAL FUNCTIONS
********************************************************************/
//Implementation of internal (static) functions.
/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/

/**
 *   Function Name: HLB_init_ind_listener
 *
 *  @brief Description: this function initialises the indication listener.
 *
 *
 *  @param  input : adapter_addr - the adapter mac address.
 *  @param  input : dest_addr - optional param, the adress of the modem we want to listen from.
 *  @param  output : ind_listener_handler - The indication listener handler object.
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_init_ind_listener(mac_address_t adapter_addr, mac_address_t dest_addr,
                                    HLB_ind_listener_handler_struct_t *ind_listener_handler);

/**
 *   Function Name: HLB_terminate_ind_listener
 *
 *  @brief Description: this function terminates the indication listener.
 *
 *
 *  @param  input : ind_listener_handler - The indication listener handler object.
 *
 *
 */
void HLB_terminate_ind_listener(HLB_ind_listener_handler_struct_t *ind_listener_handler);                                

/**
 *   Function Name: HLB_register_d_link_ready
 *
 *  @brief Description: this function adds the D link ready message to the messages
 * the indication listener listens to.
 *
 *
 *  @param  input : ind_listener_handler - The indication listener handler object.
 *  @param  input : callback - The callback activated when D link ready messages are received.
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_register_d_link_ready(HLB_ind_listener_handler_struct_t *ind_listener_handler,
                                            void (*callback)(HLB_hpgp_d_link_ready_status_ind_t*));

/**
 *   Function Name: HLB_unregister_d_link_ready
 *
 *  @brief Description: this function removes the D link ready message from the messages
 * the indication listener listens to.
 *
 *
 *  @param  input : ind_listener_handler - The indication listener handler object.
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_unregister_d_link_ready(HLB_ind_listener_handler_struct_t *ind_listener_handler);

/**
 *   Function Name: HLB_register_host_message_status
 *
 *  @brief Description: this function adds the Host message status message to the messages
 * the indication listener listens to.
 *
 *
 *  @param  input : ind_listener_handler - The indication listener handler object.
 *  @param  input : callback - The callback activated when Host message status messages are received.
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_register_host_message_status(HLB_ind_listener_handler_struct_t *ind_listener_handler,
                                            void (*callback)(HLB_hpgp_host_message_status_ind_t*));

/**
 *   Function Name: HLB_unregister_host_message_status
 *
 *  @brief Description: this function removes the Host message status message from the messages
 * the indication listener listens to.
 *
 *
 *  @param  input : ind_listener_handler - The indication listener handler object.
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_unregister_host_message_status(HLB_ind_listener_handler_struct_t *ind_listener_handler); 

#endif