/********************************************************************
*
* Module Name: mem pool
* Design:
* Implement mempool
*
********************************************************************/
/*******************************************************************
* IMPORTS
********************************************************************/
#include "mem_pool_api.h"
#include "osal.h"
/*******************************************************************
* CONSTANTS
********************************************************************/
/* align to 64/32 bit systems */
#define ALIGNMENT_SIZE (sizeof(void *))
/*******************************************************************
* MACROS
********************************************************************/
/*******************************************************************
* TYPES
********************************************************************/
typedef struct memory_pool
{
	size_t usage;
	size_t capacity;
	void *start;
	void *current;
	char lock[1];
} memory_pool_t;

memory_pool_t *memory_pool = NULL;
/*******************************************************************
* STATIC and GLOBAL DATA
********************************************************************/
/*******************************************************************
* INTERNAL FUNCTIONS
********************************************************************/
/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/

void memory_pool_init(void *allocated_space, size_t size, memory_pool_t **pool)
{
	size_t lock_size = osal_lock_alloc_size();
	if (allocated_space == NULL)
	{
		hlb_log_error("allocated_space provided in memory_pool_init is NULL!");
		return;
	}
	if (pool == NULL)
	{
		hlb_log_error("pool provided in memory_pool_init is NULL!");
		return;
	}
	*pool = (memory_pool_t *)allocated_space;
	(*pool)->capacity = size;
	(*pool)->usage = sizeof(**pool) + lock_size - sizeof((*pool)->lock);
	(*pool)->start = (char *)allocated_space + (*pool)->usage;
	(*pool)->current = (*pool)->start;
	osal_lock_init(&(*pool)->lock);
}

void memory_pool_set(memory_pool_t *pool)
{
	memory_pool = pool;
}

void *memory_pool_alloc(memory_pool_t *pool, size_t size)
{
	char *ret;
	uint8_t align;
	if (pool == NULL)
	{
		if (memory_pool == NULL)
		{
			return NULL;
		}
		pool = memory_pool;
	}
	osal_lock_lock((void *)pool->lock);
	ret = pool->current;
	align = ALIGNMENT_SIZE - (int)(((uintptr_t)pool->current)%ALIGNMENT_SIZE);
	size += align;
	ret += align;
	if (pool->capacity < (pool->usage + size))
	{
		osal_lock_unlock((void *)pool->lock);
		return NULL;
	}
	pool->current = (char *)pool->current + size;
	pool->usage += size;
	osal_lock_unlock((void *)pool->lock);
	return ret;
}

void memory_pool_free(memory_pool_t *pool, void *pointer)
{
	(void)pool;
	(void)pointer;
}

size_t memory_pool_get_usage(memory_pool_t *pool)
{
	size_t usage = 0;
	if (pool == NULL)
	{
		if (memory_pool == NULL)
		{
			return 0;
		}
		osal_lock_lock((void *)memory_pool->lock);
		usage = memory_pool->usage;
		osal_lock_unlock((void *)memory_pool->lock);
	}
	else
	{
		osal_lock_lock((void *)pool->lock);
		usage = pool->usage;
		osal_lock_unlock((void *)pool->lock);
	}
	return usage;
}

size_t memory_pool_get_capacity(memory_pool_t *pool)
{
	size_t capacity = 0;
	if (pool == NULL)
	{
		if (memory_pool == NULL)
		{
			return 0;
		}
		osal_lock_lock((void *)memory_pool->lock);
		capacity = memory_pool->capacity;
		osal_lock_unlock((void *)memory_pool->lock);
	}
	else
	{
		osal_lock_lock((void *)pool->lock);
		capacity = pool->capacity;
		osal_lock_unlock((void *)pool->lock);
	}
	return capacity;
}

void memory_pool_destroy(memory_pool_t *pool)
{
	if (pool)
	{
		osal_lock_destroy((void *)pool->lock);
	}
	else if (memory_pool)
	{
		osal_lock_destroy((void *)memory_pool->lock);
		memory_pool = NULL;
	}
}
