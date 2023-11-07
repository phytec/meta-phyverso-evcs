/********************************************************************
*
* Module Name: HLB_ind_listener
*
********************************************************************/
/*******************************************************************
* IMPORTS
********************************************************************/
#include "HLB_ind_listener.h"
#include "HLB_host.h"
#include "osal.h"
#include "common.h"
#include "mem_pool.h"
#include "shared.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

/*******************************************************************
* CONSTANTS
********************************************************************/
#define CALLBACK_LOOP_SLEEP_TIME 2
#define ALLOCATED_AREA_SIZE 65536
/*******************************************************************
* TYPES
********************************************************************/
typedef void (*func_converter)(void*);
/*******************************************************************
* STATIC and GLOBAL DATA
********************************************************************/
/*******************************************************************
* MACROS
********************************************************************/
/*******************************************************************
* INTERNAL FUNCTIONS
********************************************************************/
static RES_result_t HLB_init_registered_messages_list(HLB_registered_messages_t *registered_messages)
{
    int i;
    for(i = 0;i < SUPPORTED_IND_MESSAGES;i++)
    {
        registered_messages->registered_message[i].filled = false;
    }

    registered_messages->lock = malloc(osal_lock_alloc_size());
	if (!registered_messages->lock)
	{
		printf("%s: failed to alloc lock errno=%d\n", __func__, errno);
		return RES_RESULT_NO_MEMORY;
	}

	osal_lock_init(registered_messages->lock);
    return RES_RESULT_OK;
}

static void HLB_remove_registered_messages_list(HLB_registered_messages_t *registered_messages)
{
    int i;
    for(i = 0;i < SUPPORTED_IND_MESSAGES;i++)
    {
        registered_messages->registered_message[i].filled = false;
    }
    osal_lock_destroy(registered_messages->lock);
	free(registered_messages->lock);
}

static void HLB_register_msg_callback(HLB_registered_messages_t *registered_messages,
                                        HLB_protocol_msg_id_t msg_id,
                                        void *callback)
{
    int i;
    osal_lock_lock(registered_messages->lock);
    for(i = 0;i < SUPPORTED_IND_MESSAGES;i++)
    {
        if(!registered_messages->registered_message[i].filled)
        {
            registered_messages->registered_message[i].filled = true;
            registered_messages->registered_message[i].msg_id = msg_id;
            registered_messages->registered_message[i].callback = callback;
            osal_lock_unlock(registered_messages->lock);
            return;
        }
    }
    osal_lock_unlock(registered_messages->lock);
}

static void HLB_unregister_msg_callback(HLB_registered_messages_t *registered_messages,
                                        HLB_protocol_msg_id_t msg_id)
{
    int i;
    osal_lock_lock(registered_messages->lock);
    for(i = 0;i < SUPPORTED_IND_MESSAGES;i++)
    {
        if(registered_messages->registered_message[i].filled &&
            registered_messages->registered_message[i].msg_id == msg_id)
        {
            registered_messages->registered_message[i].filled = false;
            registered_messages->registered_message[i].callback = NULL;
            osal_lock_unlock(registered_messages->lock);
            return;
        }
    }
    osal_lock_unlock(registered_messages->lock);
}

static bool HLB_check_registered_messages(HLB_registered_messages_t *registered_messages,
                                            HLB_protocol_msg_id_t msg_id)
{
    int i;
    osal_lock_lock(registered_messages->lock);
    for(i = 0;i < SUPPORTED_IND_MESSAGES;i++)
    {
        if(registered_messages->registered_message[i].filled &&
            registered_messages->registered_message[i].msg_id == msg_id)
        {
            osal_lock_unlock(registered_messages->lock);
            return true;
        }
    }
    osal_lock_unlock(registered_messages->lock);
    return false;
}

static bool HLB_check_pending_messages_empty(HLB_pending_messages_list_t *pending_messages)
{
    return (!pending_messages->full &&
            (pending_messages->head == pending_messages->tail));
}

static HLB_pending_messages_list_t *HLB_init_pending_messages_list()
{
	HLB_pending_messages_list_t *pending_messages;
	
	pending_messages = malloc(sizeof(HLB_pending_messages_list_t));
	if (!pending_messages)
	{
		printf("%s: failed to alloc pending_messages errno=%d\n", __func__, errno);
		return NULL;
	}

	pending_messages->lock = malloc(osal_lock_alloc_size());
	if (!pending_messages->lock)
	{
		printf("%s: failed to alloc lock errno=%d", __func__, errno);
		free(pending_messages);
		return NULL;
	}

	osal_lock_init(pending_messages->lock);
    pending_messages->head = 0;
    pending_messages->tail = 0;
    pending_messages->full = false;

	return pending_messages;
}

static void HLB_deinit_pending_messages_list(HLB_pending_messages_list_t *pending_messages)
{
    osal_lock_destroy(pending_messages->lock);
	free(pending_messages->lock);
}

static void HLB_remove_msg_from_pending_messages_list(HLB_pending_messages_list_t *pending_messages)
{
    if(!HLB_check_pending_messages_empty(pending_messages))
    {
        pending_messages->full = false;
	    if (++(pending_messages->tail) == SUPPORTED_PENDING_MESSAGES)
	    {
		    pending_messages->tail = 0;
	    }
    }
}

static void HLB_add_msg_to_pending_messages_list(HLB_pending_messages_list_t *pending_messages,
                                                            HLB_packet_t *received_packet)
{
    memcpy(&pending_messages->packet[pending_messages->head], received_packet, sizeof(HLB_packet_t));
	if(pending_messages->full)
    {
		if (++(pending_messages->tail) == SUPPORTED_PENDING_MESSAGES)
		{
			pending_messages->tail = 0;
		}
	}

	if (++(pending_messages->head) == SUPPORTED_PENDING_MESSAGES)
	{
		pending_messages->head = 0;
	}
	pending_messages->full = (pending_messages->head == pending_messages->tail);
}

static void HLB_apply_callback(HLB_packet_t *received_packet,
                                HLB_registered_messages_t *registered_messages,
                                HLB_pending_messages_list_t *pending_messages)
{
    int i;
    RES_result_t res;
    uint16_t curr_message_id;
    osal_lock_lock(registered_messages->lock);
    for(i = 0;i < SUPPORTED_IND_MESSAGES;i++)
    {
        curr_message_id = letohs(received_packet->management_header.msg_id);
        if(registered_messages->registered_message[i].filled &&
            curr_message_id == registered_messages->registered_message[i].msg_id)
        {
            if(curr_message_id == HLB_PROTOCOL_MSG_ID_NSCM_D_LINK_READY_IND)
            {
                HLB_hpgp_d_link_ready_status_ind_t d_link;
                res = HLB_nscm_d_link_ready_ind_parse(received_packet, &d_link);
                if(res != RES_RESULT_OK)
                {
                    printf("%s: Failed to parse packed d_link_ready packet\n", __func__);
                }
                (*(void(**)(HLB_hpgp_d_link_ready_status_ind_t*))
                    &registered_messages->registered_message[i].callback)(&d_link);
            }
            else if(curr_message_id == HLB_PROTOCOL_MSG_ID_NSCM_VS_HOST_MESSAGE_STATUS_IND)
            {
                HLB_hpgp_host_message_status_ind_t host_message_status;
                res = HLB_nscm_host_message_status_ind_parse(received_packet, &host_message_status);
                if(res != RES_RESULT_OK)
                {
                    printf("%s: Failed to parse packed host_message_status packet\n", __func__);
                }
                (*(void(**)(HLB_hpgp_host_message_status_ind_t*))
                    &registered_messages->registered_message[i].callback)(&host_message_status);
            }

            HLB_remove_msg_from_pending_messages_list(pending_messages);
            break;
        }

    }
    osal_lock_unlock(registered_messages->lock);
}

static void *HLB_RX_loop(void *param)
{
	HLB_rx_handler_struct_t *rx_loop_struct = (HLB_rx_handler_struct_t *)param;
	void *eth_handle = rx_loop_struct->com->m_eth_handle_t;
	HLB_pending_messages_list_t *pending_messages = rx_loop_struct->pending_messages;
	HLB_packet_t Buffer;
	size_t rx_packet_length;
	RES_result_t res = RES_RESULT_OK; 
	memset(&Buffer, 0x00, sizeof(HLB_packet_t));
	while (res == RES_RESULT_OK)
	{
		rx_packet_length = HLB_PACKET_SIZE;
		res = ETH_rx(eth_handle, &Buffer, &rx_packet_length, -1);
		if (res == RES_RESULT_OK)
		{
			Buffer.management_header.msg_id = letohs(Buffer.management_header.msg_id);
			if(HLB_check_registered_messages(rx_loop_struct->registered_messages ,Buffer.management_header.msg_id))
            {
                HLB_add_msg_to_pending_messages_list(pending_messages, &Buffer);
                osal_cond_lock_lock(rx_loop_struct->cond_lock);
                osal_release_condition(rx_loop_struct->cond_lock);
                osal_cond_lock_unlock(rx_loop_struct->cond_lock);
            }
		}
	}
	return NULL;
}

static void *HLB_message_executor_loop(void *param)
{
    HLB_message_executor_handler_struct_t *callback_loop_struct =
                                        (HLB_message_executor_handler_struct_t *)param;
    while(true)
    {
        osal_cond_lock_lock(callback_loop_struct->cond_lock);
        if(HLB_check_pending_messages_empty(callback_loop_struct->pending_messages))
        {
            osal_wait_on_condition(callback_loop_struct->cond_lock);
        }
        osal_cond_lock_unlock(callback_loop_struct->cond_lock);
        while(!HLB_check_pending_messages_empty(callback_loop_struct->pending_messages))
        {
            HLB_packet_t *oldest_packet =
                &callback_loop_struct->pending_messages->packet[callback_loop_struct->pending_messages->tail];
            HLB_apply_callback(oldest_packet, callback_loop_struct->registered_messages,
                                callback_loop_struct->pending_messages);    
        }
    }
	return NULL;
}
/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/
RES_result_t HLB_init_ind_listener(mac_address_t adapter_addr, mac_address_t dest_addr,
                                    HLB_ind_listener_handler_struct_t *ind_listener_handler)
{
    RES_result_t res;
    HLB_hpgp_communication_t *com;
    uint8_t mem_area[ALLOCATED_AREA_SIZE];
    uint8_t condition_lock[osal_cond_lock_alloc_size()];
    memory_pool_t *pool;

	if (!adapter_addr || !ind_listener_handler)
	{
		printf("%s: HLB_init_ind_listener received NULL PTR\n", __func__);
		return RES_RESULT_NULL_PTR;
	}

    memory_pool_init(mem_area, ALLOCATED_AREA_SIZE, &pool);
	memory_pool_set(pool);
    HLB_registered_messages_t *registered_messages = malloc(sizeof(HLB_registered_messages_t));
    ind_listener_handler->message_executor_handler = malloc(sizeof(HLB_message_executor_handler_struct_t));
    if(!ind_listener_handler->message_executor_handler)
    {
        printf("%s: Failed to allocate message executor handler\n", __func__);
        return RES_RESULT_NO_MEMORY;
    }

    ind_listener_handler->rx_handler = malloc(sizeof(HLB_rx_handler_struct_t));
    if(!ind_listener_handler->rx_handler)
    {
        printf("%s: Failed to allocate rx handler\n", __func__);
        free(ind_listener_handler->message_executor_handler);
        return RES_RESULT_NO_MEMORY;
    }

	com = malloc(sizeof(HLB_hpgp_communication_t));
	if (!com)
	{
		printf("%s: malloc com failed\n", __func__);
        free(ind_listener_handler->message_executor_handler);
        free(ind_listener_handler->rx_handler);
		return RES_RESULT_NO_MEMORY;
	}
    
	res = HLB_init_communication(adapter_addr, dest_addr, com);
	if (res != RES_RESULT_OK)
	{
		printf("%s: HLB_init_communication failed, res=%d\n", __func__, res);
        free(ind_listener_handler->message_executor_handler);
        free(ind_listener_handler->rx_handler);
		free(com);
		return res;
	}

    res = HLB_init_registered_messages_list(registered_messages);
    if(res != RES_RESULT_OK)
    {
        printf("%s: HLB_init_registered_messages_list failed, res=%d\n", __func__, res);
        free(ind_listener_handler->message_executor_handler);
        free(ind_listener_handler->rx_handler);
        HLB_deinit_communication(com);
		free(com);
		return res;
    }

    ind_listener_handler->rx_handler->registered_messages = registered_messages;
	ind_listener_handler->rx_handler->com = com;
	ind_listener_handler->rx_handler->pending_messages = HLB_init_pending_messages_list();
    ind_listener_handler->rx_handler->cond_lock = condition_lock;
	if (!ind_listener_handler->rx_handler->pending_messages)
	{
		printf("%s: HLB_init_pending_messages_list failed\n", __func__);
        free(ind_listener_handler->message_executor_handler);
        free(ind_listener_handler->rx_handler);
        HLB_remove_registered_messages_list(registered_messages);
		HLB_deinit_communication(com);
		free(com);
		return RES_RESULT_NO_MEMORY;
	}

	res = osal_thread_create(&ind_listener_handler->rx_thread_handle, HLB_RX_loop,
                                ind_listener_handler->rx_handler);
	if (res != RES_RESULT_OK)
	{
		printf("%s: osal_thread_create receiver failed, res=%d\n", __func__, res);
        free(ind_listener_handler->message_executor_handler);
        free(ind_listener_handler->rx_handler);
        HLB_remove_registered_messages_list(registered_messages);
		HLB_deinit_pending_messages_list(ind_listener_handler->rx_handler->pending_messages);
		HLB_deinit_communication(com);
		free(com);
		return res;
	}

    ind_listener_handler->message_executor_handler->registered_messages = registered_messages;
	ind_listener_handler->message_executor_handler->pending_messages =
                                            ind_listener_handler->rx_handler->pending_messages;
    ind_listener_handler->message_executor_handler->cond_lock = condition_lock;
    res = osal_thread_create(&ind_listener_handler->message_executor_thread_handle,
                                HLB_message_executor_loop,
                                ind_listener_handler->message_executor_handler);
	if (res != RES_RESULT_OK)
	{
		printf("%s: osal_thread_create callback failed, res=%d\n", __func__, res);
        free(ind_listener_handler->message_executor_handler);
        free(ind_listener_handler->rx_handler);
	    osal_thread_delete(ind_listener_handler->rx_thread_handle);
        HLB_remove_registered_messages_list(registered_messages);
		HLB_deinit_pending_messages_list(ind_listener_handler->rx_handler->pending_messages);
		HLB_deinit_communication(com);
		free(com);
		return res;
	}
    return RES_RESULT_OK;
}

void HLB_terminate_ind_listener(HLB_ind_listener_handler_struct_t *ind_listener_handler)
{
	osal_thread_delete(&ind_listener_handler->rx_thread_handle);
	osal_thread_delete(&ind_listener_handler->message_executor_thread_handle);
    HLB_remove_registered_messages_list(ind_listener_handler->rx_handler->registered_messages);
	HLB_deinit_pending_messages_list(ind_listener_handler->rx_handler->pending_messages);
	HLB_deinit_communication(ind_listener_handler->rx_handler->com);
	free(ind_listener_handler->rx_handler->com);
    free(ind_listener_handler->rx_handler);
    free(ind_listener_handler->message_executor_handler);
}

RES_result_t HLB_register_d_link_ready(HLB_ind_listener_handler_struct_t *ind_listener_handler,
                                            void (*callback)(HLB_hpgp_d_link_ready_status_ind_t*))
{
    func_converter d_link_ready_pointer_to_pass = ((void (*)(void *))callback);
    HLB_register_msg_callback(ind_listener_handler->message_executor_handler->registered_messages,
                                        HLB_PROTOCOL_MSG_ID_NSCM_D_LINK_READY_IND,
                                        *(void**)(&d_link_ready_pointer_to_pass));

    return RES_RESULT_OK;                                    
}

RES_result_t HLB_unregister_d_link_ready(HLB_ind_listener_handler_struct_t *ind_listener_handler)
{
    HLB_unregister_msg_callback(ind_listener_handler->message_executor_handler->registered_messages,
                                        HLB_PROTOCOL_MSG_ID_NSCM_D_LINK_READY_IND);

    return RES_RESULT_OK;  
}

RES_result_t HLB_register_host_message_status(HLB_ind_listener_handler_struct_t *ind_listener_handler,
                                                void (*callback)(HLB_hpgp_host_message_status_ind_t*))
{
    func_converter host_message_pointer_to_pass = (void (*)(void *))callback;
    HLB_register_msg_callback(ind_listener_handler->message_executor_handler->registered_messages,
                                        HLB_PROTOCOL_MSG_ID_NSCM_VS_HOST_MESSAGE_STATUS_IND,
                                        *(void**)(&host_message_pointer_to_pass));

    return RES_RESULT_OK;  
}

RES_result_t HLB_unregister_host_message_status(HLB_ind_listener_handler_struct_t *ind_listener_handler)
{
    HLB_unregister_msg_callback(ind_listener_handler->message_executor_handler->registered_messages,
                                        HLB_PROTOCOL_MSG_ID_NSCM_VS_HOST_MESSAGE_STATUS_IND);

    return RES_RESULT_OK;  
}

RES_result_t HLB_register_unassociated_sta(HLB_ind_listener_handler_struct_t *ind_listener_handler,
                                            void (*callback)(HLB_hpgp_unassociated_sta_ind_t*))
{
    func_converter host_message_pointer_to_pass = (void (*)(void *))callback;
    HLB_register_msg_callback(ind_listener_handler->message_executor_handler->registered_messages,
                                        HLB_PROTOCOL_MSG_ID_APCM_UNASSOCIATED_STA_IND,
                                        *(void**)(&host_message_pointer_to_pass));
    return RES_RESULT_OK;                                    
}