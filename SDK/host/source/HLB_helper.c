/********************************************************************
*
* Module Name: HLB helper functions
* Design:
* Implement rx db and other helper apis
*
********************************************************************/
/*******************************************************************
* IMPORTS
********************************************************************/
#include "mem_pool.h"
#include "osal.h"
#include "HLB_helper.h"
#include <string.h>
#include <errno.h>
/*******************************************************************
* CONSTANTS
********************************************************************/
#define MIN_MAIN_RX_DB_SIZE 5
#define MIN_INDICATION_RX_DB_SIZE 2
#define MAIN_DB_SIZE_PROPORTION 0.8

/* Due to alligment in the malloc function */
#define MALLOC_AMOUNT_RX_DB 10
/*******************************************************************
* TYPES
********************************************************************/
typedef struct
{
	int top;
	size_t capacity;
	int *array;
} HLB_stack_t;

typedef struct HLB_int_list
{
	int prev;
	int next;
} HLB_int_list_t;

typedef struct
{
	int HLB_packet_list_size;
	HLB_stack_t *HLB_packet_free_slots;
	HLB_int_list_t *HLB_list_of_taken;
	int HLB_int_list_last;
	int HLB_int_list_first;
	void *lock;
} HLB_core_db_fields;

typedef struct
{
	HLB_packet_t packet;
	uint16_t size;
} HLB_sized_packet_t;

typedef struct
{
	HLB_sized_packet_t *HLB_packet_list;
	HLB_core_db_fields core;
} HLB_main_rx_db_t;

typedef struct
{
	HLB_timestamped_packet_t *HLB_timestamped_packet_list;
	void *cond_lock;
	void *cleaner_thread_handle;
	bool running;
	HLB_core_db_fields core;
} HLB_indication_rx_db_t;

typedef struct HLB_rx_db
{
	HLB_main_rx_db_t main_db;
	HLB_indication_rx_db_t indication_db;
} HLB_rx_db_t;

typedef enum
{
	HLB_main_db = 0,
	HLB_indication_db = 1
} HLB_internal_db_types_t;

/*******************************************************************
* STATIC and GLOBAL DATA
********************************************************************/
/*******************************************************************
* MACROS
********************************************************************/
/*******************************************************************
* INTERNAL FUNCTIONS
********************************************************************/
static HLB_stack_t *HLB_stack_create(size_t capacity)
{
	HLB_stack_t *stack = (HLB_stack_t *)malloc(sizeof(HLB_stack_t));
	if (stack == NULL)
	{
		return NULL;
	}
	stack->capacity = capacity;
	stack->top = -1;
	stack->array = (int *)malloc(stack->capacity * sizeof(int));
	if (stack->array == NULL)
	{
		free(stack);
		return NULL;
	}
	return stack;
}

static void HLB_stack_free(HLB_stack_t *stack)
{
	if (stack == NULL)
	{
		hlb_log_error("stack provided in HLB_stack_free is NULL!");
		return;
	}
	free(stack->array);
	free(stack);
}

static bool HLB_stack_is_full(HLB_stack_t *stack)
{
	if (stack == NULL)
	{
		hlb_log_error("stack provided in HLB_stack_is_full is NULL!");
		return true;
	}
	return stack->top == (int)(stack->capacity - 1);
}

static bool HLB_stack_is_empty(HLB_stack_t *stack)
{
	if (stack == NULL)
	{
		hlb_log_error("stack provided in HLB_stack_is_empty is NULL!");
		return true;
	}
	return (stack->top == -1);
}

static RES_result_t HLB_stack_push(HLB_stack_t *stack, int item)
{
	if (stack == NULL)
	{
		hlb_log_error("stack provided in HLB_stack_push is NULL!");
		return RES_RESULT_NULL_PTR;
	}
	if (HLB_stack_is_full(stack))
		return RES_RESULT_GENERAL_ERROR;
	stack->array[++stack->top] = item;
	return RES_RESULT_OK;
}

static RES_result_t HLB_stack_pop(HLB_stack_t *stack, int *item)
{
	if (stack == NULL)
	{
		hlb_log_error("stack provided in HLB_stack_pop is NULL!");
		return RES_RESULT_NULL_PTR;
	}
	if (item == NULL)
	{
		hlb_log_error("item provided in HLB_stack_pop is NULL!");
		return RES_RESULT_NULL_PTR;
	}
	if (HLB_stack_is_empty(stack))
	{
		return RES_RESULT_GENERAL_ERROR;
	}
	*item = stack->array[stack->top--];
	return RES_RESULT_OK;
}

static bool HLB_is_empty_db(HLB_core_db_fields *core)
{
	return (core->HLB_int_list_first == -1);
}

static size_t HLB_get_possible_rx_db_size(size_t area_size, HLB_internal_db_types_t db_type)
{
	size_t global_needed_size;
	size_t per_slot_needed_size;

	if(db_type == HLB_main_db)
	{
		global_needed_size = sizeof(HLB_main_rx_db_t) + osal_lock_alloc_size() +
							sizeof(HLB_stack_t) + sizeof(void *) * MALLOC_AMOUNT_RX_DB;
		per_slot_needed_size = sizeof(HLB_packet_t) + sizeof(HLB_int_list_t) + sizeof(int);
		if (global_needed_size > area_size)
		{
			return 0;
		}
	}
	else
	{
		global_needed_size = sizeof(HLB_indication_rx_db_t) + osal_lock_alloc_size() +
							osal_cond_lock_alloc_size() + sizeof(HLB_stack_t) + sizeof(void *)
							* MALLOC_AMOUNT_RX_DB;
		per_slot_needed_size = sizeof(HLB_timestamped_packet_t) + sizeof(HLB_int_list_t) + sizeof(int);
		if (global_needed_size > area_size)
		{
			return 0;
		}
	}
	
	return ((area_size - global_needed_size)/per_slot_needed_size);
}

/* static RES_result_t HLB_stack_peek(HLB_stack_t* stack, int *item)
{
	if (isEmpty(stack))
	{
		return RES_RESULT_GENERAL_ERROR;
	}
	*item = stack->array[stack->top];
	return RES_RESULT_OK;
} */

static HLB_internal_db_types_t HLB_check_db_type(HLB_protocol_msg_id_t msg_id)
{
	HLB_internal_db_types_t ret;

	if ((msg_id == HLB_PROTOCOL_MSG_ID_NSCM_D_LINK_READY_IND) ||
		((msg_id & 0x3) == HLB_INDICATION))
	{
		ret = HLB_indication_db;
	}
	else
	{
		ret = HLB_main_db;
	}

	return ret;
}

static RES_result_t HLB_remove_packet(int index, HLB_core_db_fields *core)
{
	int new_next, new_prev;
	new_next = core->HLB_list_of_taken[index].next;
	new_prev = core->HLB_list_of_taken[index].prev;
	if (index == core->HLB_int_list_first)
	{
		core->HLB_int_list_first = new_next;
	}
	if (index == core->HLB_int_list_last)
	{
		core->HLB_int_list_last = new_prev;
	}
	core->HLB_list_of_taken[index].next = -1;
	core->HLB_list_of_taken[index].prev = -1;
	if (new_next != -1)
	{
		core->HLB_list_of_taken[new_next].prev = new_prev;
	}
	if (new_prev != -1)
	{
		core->HLB_list_of_taken[new_prev].next = new_next;
	}
	return HLB_stack_push(core->HLB_packet_free_slots, index);
}

static RES_result_t HLB_add_packet(HLB_core_db_fields *core, HLB_internal_db_types_t db_type,
									void *packet_list, HLB_packet_t *packet, uint16_t packet_size)
{	
	int free_slot;
	int *first = &core->HLB_int_list_first;
	int *last = &core->HLB_int_list_last;
	RES_result_t res;
	osal_lock_lock(core->lock);
	res = HLB_stack_pop(core->HLB_packet_free_slots, &free_slot);
	if (res != RES_RESULT_OK)
	{
		osal_lock_unlock(core->lock);
		return res;
	}
	if(db_type == HLB_main_db)
	{
		((HLB_sized_packet_t *)packet_list)[free_slot].packet = *packet;
		((HLB_sized_packet_t *)packet_list)[free_slot].size = packet_size;
	}
	else
	{
		((HLB_timestamped_packet_t *)packet_list)[free_slot].packet = *packet;
		osal_update_timestamp(&((HLB_timestamped_packet_t *)packet_list)[free_slot].timestamp_secs);
	}
	
	if (*first == -1)
	{
		*first = free_slot;
	}
	if (*last == -1)
	{
		core->HLB_list_of_taken[free_slot].next = -1;
		core->HLB_list_of_taken[free_slot].prev = -1;
		*last = free_slot;
	}
	else
	{
		core->HLB_list_of_taken[*last].next = free_slot;
		core->HLB_list_of_taken[free_slot].next = -1;
		core->HLB_list_of_taken[free_slot].prev = *last;
		*last = free_slot;
	}
	osal_lock_unlock(core->lock);
	return RES_RESULT_OK;
}

static void *HLB_cleaner_thread_loop(void *param)
{
	HLB_rx_db_t *rx_db = (HLB_rx_db_t *)param;
	HLB_indication_rx_db_t *indication_db = &rx_db->indication_db;
	uint64_t current_time;
	int i;
	while (indication_db->running == true)
	{
		osal_cond_lock_lock(indication_db->cond_lock);
		if (indication_db->running == true)
		{
			osal_wait_on_timed_condition(CLEANER_THREAD_SLEEP, indication_db->cond_lock);
		}
		osal_cond_lock_unlock(indication_db->cond_lock);

		if (indication_db->running == false)
		{
			break;
		}

		osal_lock_lock(indication_db->core.lock);
		if (HLB_is_empty_db(&indication_db->core))
		{
			osal_lock_unlock(indication_db->core.lock);
			continue;
		}
		osal_update_timestamp(&current_time);
		for (i = indication_db->core.HLB_int_list_first; i != -1;
				i = indication_db->core.HLB_list_of_taken[i].next)
		{
			if (((indication_db->HLB_timestamped_packet_list[i].timestamp_secs +
					INDICATION_DB_TIMEOUT) <= current_time))
			{
				HLB_remove_packet(i, &indication_db->core);
			}
		}
		osal_lock_unlock(indication_db->core.lock);
	}
	return NULL;
}
/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/
size_t HLB_get_packet_capacity_rx_db_main_db(HLB_rx_db_t *rx_db)
{
	return rx_db->main_db.core.HLB_packet_list_size;
}

size_t HLB_get_packet_capacity_rx_db_indication_db(HLB_rx_db_t *rx_db)
{
	return rx_db->indication_db.core.HLB_packet_list_size;
}

size_t HLB_get_remaining_space_rx_db_indication_db(HLB_rx_db_t *rx_db)
{
	return (rx_db->indication_db.core.HLB_packet_free_slots->top + 1);
}

HLB_rx_db_t *HLB_init_rx_db(size_t area_size)
{
	int i;
	HLB_rx_db_t *rx_db;
	int main_db_pckt_list_size;
	int indication_db_pckt_list_size;
	main_db_pckt_list_size = HLB_get_possible_rx_db_size((size_t)(area_size * MAIN_DB_SIZE_PROPORTION), HLB_main_db);
	if (main_db_pckt_list_size < MIN_MAIN_RX_DB_SIZE)
	{
		hlb_log_info("memory area is too small");
		hlb_log_info("have memory for %d slots in main rx_db, but minimum is %d slots",
						 main_db_pckt_list_size, MIN_MAIN_RX_DB_SIZE);
		return NULL;
	}
	indication_db_pckt_list_size = HLB_get_possible_rx_db_size(
									(size_t)(area_size * (1-MAIN_DB_SIZE_PROPORTION)), HLB_indication_db);
	if (indication_db_pckt_list_size < MIN_INDICATION_RX_DB_SIZE)
	{
		hlb_log_info("memory area is too small");
		hlb_log_info("have memory for %d slots in indication rx_db, but minimum is %d slots",
						 indication_db_pckt_list_size, MIN_MAIN_RX_DB_SIZE);
		return NULL;
	}
	hlb_log_info("rx_db main_size is %d slots", main_db_pckt_list_size);
	hlb_log_info("rx_db indication_size is %d slots", indication_db_pckt_list_size);
	rx_db = malloc(sizeof(HLB_rx_db_t));
	if (rx_db == NULL)
	{
		hlb_log_error("failed to alloc rx_db errno=%d", errno);
		return NULL;
	}

	rx_db->main_db.core.HLB_packet_list_size = main_db_pckt_list_size;
	rx_db->indication_db.core.HLB_packet_list_size = indication_db_pckt_list_size;
	rx_db->main_db.HLB_packet_list = malloc(main_db_pckt_list_size *
											sizeof(HLB_sized_packet_t));
	if (rx_db->main_db.HLB_packet_list == NULL)
	{
		hlb_log_error("failed to alloc HLB_packet_list main_db errno=%d", errno);
		free(rx_db);
		return NULL;
	}
	rx_db->indication_db.HLB_timestamped_packet_list =
		malloc(indication_db_pckt_list_size *
		sizeof(HLB_timestamped_packet_t));
	if (rx_db->indication_db.HLB_timestamped_packet_list == NULL)
	{
		hlb_log_error("failed to alloc HLB_timestamped_packet_list indication_db errno=%d",
						errno);
		free(rx_db->main_db.HLB_packet_list);
		free(rx_db);
		return NULL;
	}
	memset(rx_db->main_db.HLB_packet_list, 0, main_db_pckt_list_size *
			sizeof(*(rx_db->main_db.HLB_packet_list)));
	memset(rx_db->indication_db.HLB_timestamped_packet_list, 0, indication_db_pckt_list_size *
			sizeof(*(rx_db->indication_db.HLB_timestamped_packet_list)));
	rx_db->main_db.core.HLB_packet_free_slots = HLB_stack_create(main_db_pckt_list_size);
	if (rx_db->main_db.core.HLB_packet_free_slots == NULL)
	{
		hlb_log_error("failed to alloc HLB_packet_list main_db errno=%d", errno);
		free(rx_db->main_db.HLB_packet_list);
		free(rx_db->indication_db.HLB_timestamped_packet_list);
		free(rx_db);
		return NULL;
	}
	rx_db->indication_db.core.HLB_packet_free_slots =
			HLB_stack_create(indication_db_pckt_list_size);
	if (rx_db->indication_db.core.HLB_packet_free_slots == NULL)
	{
		hlb_log_error("failed to alloc HLB_packet_list indication_db errno=%d", errno);
		free(rx_db->main_db.core.HLB_packet_free_slots);
		free(rx_db->main_db.HLB_packet_list);
		free(rx_db->indication_db.HLB_timestamped_packet_list);
		free(rx_db);
		return NULL;
	}
	for (i = main_db_pckt_list_size - 1; i > -1; i--)
	{
		HLB_stack_push(rx_db->main_db.core.HLB_packet_free_slots, i);
	}
	for (i = indication_db_pckt_list_size - 1; i > -1; i--)
	{
		HLB_stack_push(rx_db->indication_db.core.HLB_packet_free_slots, i);
	}
	rx_db->main_db.core.HLB_list_of_taken = malloc(main_db_pckt_list_size
												 * sizeof(HLB_int_list_t));
	if (rx_db->main_db.core.HLB_list_of_taken == NULL)
	{
		hlb_log_error("failed to alloc HLB_list_of_taken main_db errno=%d", errno);
		HLB_stack_free(rx_db->main_db.core.HLB_packet_free_slots);
		free(rx_db->main_db.HLB_packet_list);
		HLB_stack_free(rx_db->indication_db.core.HLB_packet_free_slots);
		free(rx_db->indication_db.HLB_timestamped_packet_list);
		free(rx_db);
		return NULL;
	}
	rx_db->indication_db.core.HLB_list_of_taken = malloc(indication_db_pckt_list_size
													* sizeof(HLB_int_list_t));
	if (rx_db->indication_db.core.HLB_list_of_taken == NULL)
	{
		hlb_log_error("failed to alloc HLB_list_of_taken indication_db errno=%d", errno);
		HLB_stack_free(rx_db->main_db.core.HLB_packet_free_slots);
		free(rx_db->main_db.core.HLB_list_of_taken);
		free(rx_db->main_db.HLB_packet_list);
		HLB_stack_free(rx_db->indication_db.core.HLB_packet_free_slots);
		free(rx_db->indication_db.HLB_timestamped_packet_list);
		free(rx_db);
		return NULL;
	}
	rx_db->main_db.core.lock = malloc(osal_lock_alloc_size());
	if (rx_db->main_db.core.lock == NULL)
	{
		hlb_log_error("failed to alloc lock main_db errno=%d", errno);
		free(rx_db->main_db.core.HLB_list_of_taken);
		HLB_stack_free(rx_db->main_db.core.HLB_packet_free_slots);
		free(rx_db->main_db.HLB_packet_list);
		free(rx_db->indication_db.core.HLB_list_of_taken);
		HLB_stack_free(rx_db->indication_db.core.HLB_packet_free_slots);
		free(rx_db->indication_db.HLB_timestamped_packet_list);
		free(rx_db);
		return NULL;
	}
	rx_db->indication_db.core.lock = malloc(osal_lock_alloc_size());
	if (rx_db->indication_db.core.lock == NULL)
	{
		hlb_log_error("failed to alloc lock indication_db errno=%d", errno);
		free(rx_db->main_db.core.lock);
		free(rx_db->main_db.core.HLB_list_of_taken);
		HLB_stack_free(rx_db->main_db.core.HLB_packet_free_slots);
		free(rx_db->main_db.HLB_packet_list);
		free(rx_db->indication_db.core.HLB_list_of_taken);
		HLB_stack_free(rx_db->indication_db.core.HLB_packet_free_slots);
		free(rx_db->indication_db.HLB_timestamped_packet_list);
		free(rx_db);
		return NULL;
	}
	osal_lock_init(rx_db->main_db.core.lock);
	osal_lock_init(rx_db->indication_db.core.lock);
	rx_db->main_db.core.HLB_int_list_last = -1;
	rx_db->indication_db.core.HLB_int_list_last = -1;
	rx_db->main_db.core.HLB_int_list_first = -1;
	rx_db->indication_db.core.HLB_int_list_first = -1;
	rx_db->indication_db.cond_lock = malloc(osal_cond_lock_alloc_size());
	if (rx_db->indication_db.cond_lock == NULL)
	{
		hlb_log_error("failed to alloc cond_lock indication_db errno=%d", errno);
		osal_lock_destroy(rx_db->main_db.core.lock);
		free(rx_db->main_db.core.lock);
		free(rx_db->main_db.core.HLB_list_of_taken);
		HLB_stack_free(rx_db->main_db.core.HLB_packet_free_slots);
		rx_db->main_db.core.HLB_packet_free_slots = NULL;
		free(rx_db->main_db.HLB_packet_list);
		rx_db->main_db.HLB_packet_list = NULL;
		osal_lock_destroy(rx_db->indication_db.core.lock);
		free(rx_db->indication_db.core.lock);
		free(rx_db->indication_db.core.HLB_list_of_taken);
		HLB_stack_free(rx_db->indication_db.core.HLB_packet_free_slots);
		rx_db->indication_db.core.HLB_packet_free_slots = NULL;
		free(rx_db->indication_db.HLB_timestamped_packet_list);
		rx_db->indication_db.HLB_timestamped_packet_list = NULL;
		free(rx_db);
		return NULL;
	}
	osal_cond_lock_init(rx_db->indication_db.cond_lock);
	rx_db->indication_db.running = true;
	if(osal_thread_create(&rx_db->indication_db.cleaner_thread_handle, HLB_cleaner_thread_loop, rx_db) !=
		RES_RESULT_OK)
	{
		hlb_log_error("failed to pthread_create on cleaner thread!");
		osal_lock_destroy(rx_db->main_db.core.lock);
		free(rx_db->main_db.core.lock);
		free(rx_db->main_db.core.HLB_list_of_taken);
		HLB_stack_free(rx_db->main_db.core.HLB_packet_free_slots);
		rx_db->main_db.core.HLB_packet_free_slots = NULL;
		free(rx_db->main_db.HLB_packet_list);
		rx_db->main_db.HLB_packet_list = NULL;
		osal_lock_destroy(rx_db->indication_db.core.lock);
		free(rx_db->indication_db.core.lock);
		free(rx_db->indication_db.core.HLB_list_of_taken);
		HLB_stack_free(rx_db->indication_db.core.HLB_packet_free_slots);
		rx_db->indication_db.core.HLB_packet_free_slots = NULL;
		free(rx_db->indication_db.HLB_timestamped_packet_list);
		rx_db->indication_db.HLB_timestamped_packet_list = NULL;
		free(rx_db->indication_db.cond_lock);
		free(rx_db);
		return NULL;
	}
	return rx_db;
}

RES_result_t HLB_push_to_rx_db(HLB_rx_db_t *rx_db, HLB_packet_t *packet, uint16_t packet_size)
{
	RES_result_t res = RES_RESULT_OK;
	if (rx_db == NULL)
	{
		hlb_log_error("rx_db provided in HLB_push_to_rx_db is NULL!");
		return RES_RESULT_NULL_PTR;
	}
	if (packet == NULL)
	{
		hlb_log_error("packet provided in HLB_push_to_rx_db is NULL!");
		return RES_RESULT_NULL_PTR;
	}
	if(HLB_check_db_type(packet->management_header.msg_id) == HLB_main_db)
	{
		res = HLB_add_packet(&rx_db->main_db.core, HLB_main_db,
							 rx_db->main_db.HLB_packet_list,
							 packet, packet_size);
	}
	else
	{
		res = HLB_add_packet(&rx_db->indication_db.core, HLB_indication_db,
							 rx_db->indication_db.HLB_timestamped_packet_list,
							 packet, packet_size);
	}
	return res;
}

RES_result_t HLB_get_req_id_of_fragmented_msg(HLB_rx_db_t *rx_db,
											  HLB_msg_id_t msg_id, uint8_t fmsn,
											  HLB_req_id_t *req_id)
{
	int i;
	int res = RES_RESULT_NOT_FOUND;
	HLB_core_db_fields *core;
	HLB_packet_t *packet_ptr;
	uint8_t frag_idx_tmp, fmsn_tmp;

	if ((rx_db == NULL) || (req_id == NULL))
	{
		return RES_RESULT_NULL_PTR;
	}

	core = &rx_db->main_db.core;

	osal_lock_lock(core->lock);

	for (i = core->HLB_int_list_first; i != -1; i = core->HLB_list_of_taken[i].next)
	{
		packet_ptr = &rx_db->main_db.HLB_packet_list[i].packet;
		frag_idx_tmp = HLB_PACKET_GET_FRAG_IDX(packet_ptr);
		fmsn_tmp = HLB_PACKET_GET_FSM(packet_ptr);

		if ((packet_ptr->management_header.msg_id == msg_id) &&
			(fmsn_tmp == fmsn) && (frag_idx_tmp == 0))
		{
			*req_id = packet_ptr->vendor_header.req_id;
			res = RES_RESULT_OK;
		}
	}

	osal_lock_unlock(core->lock);

	return res;
}

RES_result_t HLB_find_and_pop_fragment_from_rx_db(HLB_rx_db_t *rx_db,
												  HLB_protocol_msg_id_t msg_id,
												  HLB_req_id_t req_id,
												  uint8_t frag_idx, uint8_t fmsn,
												  HLB_packet_t *packet, size_t *packet_size)
{
	int i;
	int res = RES_RESULT_NOT_FOUND;
	HLB_core_db_fields *core;
	HLB_packet_t *packet_ptr;
	uint8_t frag_idx_tmp, fmsn_tmp;

	if (rx_db == NULL)
	{
		hlb_log_error("rx_db provided in HLB_find_and_pop_from_rx_db is NULL!");
		return RES_RESULT_NULL_PTR;
	}
	if (packet == NULL)
	{
		hlb_log_error("packet provided in HLB_find_and_pop_from_rx_db is NULL!");
		return RES_RESULT_NULL_PTR;
	}

	if(HLB_check_db_type(msg_id) == HLB_main_db)
	{
		core = &rx_db->main_db.core;
	}
	else
	{
		core = &rx_db->indication_db.core;
	}
	osal_lock_lock(core->lock);
	if (HLB_is_empty_db(core))
	{
		osal_lock_unlock(core->lock);
		return RES_RESULT_NOT_FOUND;
	}

	for (i = core->HLB_int_list_first; i != -1;
			i = core->HLB_list_of_taken[i].next)
	{
		
		if(HLB_check_db_type(msg_id) == HLB_main_db)
		{
			packet_ptr = &rx_db->main_db.HLB_packet_list[i].packet;
			frag_idx_tmp = HLB_PACKET_GET_FRAG_IDX(packet_ptr);
			fmsn_tmp = HLB_PACKET_GET_FSM(packet_ptr);

			if ((frag_idx_tmp == frag_idx) &&
				(packet_ptr->management_header.msg_id == (HLB_msg_id_t)msg_id))
			{
				if (((frag_idx_tmp > 0) && (fmsn_tmp == fmsn)) ||
					(packet_ptr->vendor_header.req_id == req_id))
				{
					memcpy(packet, packet_ptr, sizeof(*packet));
					if (packet_size != NULL)
					{
						*packet_size = rx_db->main_db.HLB_packet_list[i].size;
					}
					res = HLB_remove_packet(i, core);
					break;
				}
			}
		}
		else
		{
			if ((rx_db->indication_db.HLB_timestamped_packet_list[i].packet.vendor_header.req_id == req_id) &&
				(rx_db->indication_db.HLB_timestamped_packet_list[i].packet.management_header.msg_id == (HLB_msg_id_t)msg_id))
			{
				*packet = rx_db->indication_db.HLB_timestamped_packet_list[i].packet;
				res = HLB_remove_packet(i, core);
				break;
			}		
		}
	}

	osal_lock_unlock(core->lock);

	return res;
}

RES_result_t HLB_find_and_pop_from_rx_db(HLB_rx_db_t *rx_db, HLB_protocol_msg_id_t msg_id,
										 HLB_req_id_t req_id, HLB_packet_t *packet)
{
	return HLB_find_and_pop_fragment_from_rx_db(rx_db, msg_id, req_id, 0, 0, packet, NULL);
}

size_t HLB_list_rx_db_main_db_of_msg_and_req_id(HLB_rx_db_t *rx_db, HLB_req_id_t *req_id_arr, HLB_msg_id_t *msg_id_arr, size_t size_of_array)
{
	int i;
	size_t count = 0;
	if (rx_db == NULL)
	{
		hlb_log_error("rx_db provided in HLB_list_rx_db_main_db_of_msg_and_req_id is NULL!");
		return RES_RESULT_NULL_PTR;
	}
	if (req_id_arr == NULL)
	{
		hlb_log_error("req_id_arr provided in HLB_list_rx_db_main_db_of_msg_and_req_id is NULL!");
		return RES_RESULT_NULL_PTR;
	}
	if (msg_id_arr == NULL)
	{
		hlb_log_error("msg_id_arr provided in HLB_list_rx_db_main_db_of_msg_and_req_id is NULL!");
		return RES_RESULT_NULL_PTR;
	}
	
	osal_lock_lock(rx_db->main_db.core.lock);
	if (HLB_is_empty_db(&rx_db->main_db.core))
	{
		osal_lock_unlock(rx_db->main_db.core.lock);
		return 0;
	}
	for (i = rx_db->main_db.core.HLB_int_list_first; i != -1;
			i = rx_db->main_db.core.HLB_list_of_taken[i].next)
	{
		req_id_arr[count] = rx_db->main_db.HLB_packet_list[i].packet.vendor_header.req_id;
		msg_id_arr[count] = rx_db->main_db.HLB_packet_list[i].packet.management_header.msg_id;
		count++;
		if (count == size_of_array)
		{
			break;
		}
	}
	osal_lock_unlock(rx_db->main_db.core.lock);
	return count;
}

size_t HLB_list_rx_db_main_db(HLB_rx_db_t *rx_db, HLB_packet_t *arr, size_t size_of_array)
{
	int i;
	size_t count = 0;
	if (rx_db == NULL)
	{
		hlb_log_error("rx_db provided in HLB_list_rx_db_main_db is NULL!");
		return RES_RESULT_NULL_PTR;
	}
	if (arr == NULL)
	{
		hlb_log_error("arr provided in HLB_list_rx_db_main_db_of_msg_and_req_id is NULL!");
		return RES_RESULT_NULL_PTR;
	}
	osal_lock_lock(rx_db->main_db.core.lock);
	if (HLB_is_empty_db(&rx_db->main_db.core))
	{
		osal_lock_unlock(rx_db->main_db.core.lock);
		return 0;
	}
	for (i = rx_db->main_db.core.HLB_int_list_first; i != -1;
			i = rx_db->main_db.core.HLB_list_of_taken[i].next)
	{
		arr[count] = rx_db->main_db.HLB_packet_list[i].packet;
		count++;
		if (count == size_of_array)
		{
			break;
		}
	}
	osal_lock_unlock(rx_db->main_db.core.lock);
	return count;
}

size_t HLB_list_rx_db_indication_db(HLB_rx_db_t *rx_db, HLB_timestamped_packet_t *arr, size_t size_of_array)
{
	int i;
	size_t count = 0;
	if (rx_db == NULL)
	{
		hlb_log_error("rx_db provided in HLB_list_rx_db_indication_db_db is NULL!");
		return RES_RESULT_NULL_PTR;
	}
	osal_lock_lock(rx_db->indication_db.core.lock);
	if (HLB_is_empty_db(&rx_db->indication_db.core))
	{
		osal_lock_unlock(rx_db->indication_db.core.lock);
		return 0;
	}
	for (i = rx_db->indication_db.core.HLB_int_list_first; i != -1;
			i = rx_db->indication_db.core.HLB_list_of_taken[i].next)
	{
		arr[count] = rx_db->indication_db.HLB_timestamped_packet_list[i];
		count++;
		if (count == size_of_array)
		{
			break;
		}
	}
	osal_lock_unlock(rx_db->indication_db.core.lock);
	return count;
}

void HLB_free_rx_db(HLB_rx_db_t *rx_db)
{
	if (rx_db == NULL)
	{
		hlb_log_error("rx_db provided in HLB_free_rx_db is NULL!");
		return;
	}
	osal_cond_lock_lock(rx_db->indication_db.cond_lock);
	rx_db->indication_db.running = false;
	osal_release_condition(rx_db->indication_db.cond_lock);
	osal_cond_lock_unlock(rx_db->indication_db.cond_lock);
	if(rx_db->indication_db.cleaner_thread_handle == NULL)
	{
		hlb_log_error("cleaner_thread_handle provided in HLB_free_rx_db is NULL!");
	}
	else
	{
		osal_thread_join(&rx_db->indication_db.cleaner_thread_handle);
		osal_thread_delete(&rx_db->indication_db.cleaner_thread_handle);
	}
	osal_lock_destroy(rx_db->main_db.core.lock);
	free(rx_db->main_db.core.lock);
	free(rx_db->main_db.core.HLB_list_of_taken);
	HLB_stack_free(rx_db->main_db.core.HLB_packet_free_slots);
	rx_db->main_db.core.HLB_packet_free_slots = NULL;
	free(rx_db->main_db.HLB_packet_list);
	rx_db->main_db.HLB_packet_list = NULL;
	osal_lock_destroy(rx_db->indication_db.core.lock);
	free(rx_db->indication_db.core.lock);
	free(rx_db->indication_db.core.HLB_list_of_taken);
	HLB_stack_free(rx_db->indication_db.core.HLB_packet_free_slots);
	rx_db->indication_db.core.HLB_packet_free_slots = NULL;
	free(rx_db->indication_db.HLB_timestamped_packet_list);
	rx_db->indication_db.HLB_timestamped_packet_list = NULL;
	free(rx_db->indication_db.cond_lock);
	free(rx_db);
}
