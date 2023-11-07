/**
 * @file mem_pool_api.h
 * @author Orr Mazor
 * @date 9 May 2021
 * @brief File containing header for mem pool functions
 *
 */
/********************************************************************
*
* Name: mem pool api
* Title: mem pool api
* Package title: host library
* Abstract: File containing header for mem pool functions
*
*
********************************************************************/
#ifndef MEM_POOL_API_H
#define MEM_POOL_API_H
/********************************************************************
* IMPORTS
********************************************************************/
#include <stddef.h>
/********************************************************************
* EXPORTED CONSTANTS
********************************************************************/
/********************************************************************
* EXPORTED TYPES
********************************************************************/
typedef struct memory_pool memory_pool_t;
/********************************************************************
* EXPORTED MACROS
********************************************************************/
/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/
/**
 *   Function Name: memory_pool_init
 *
 * @brief Description: this function creates a memory pool
 *
 *  @param  input : allocated_space - pre-allocated pointer
 *  @param  input : size - Mempool size
 *  @param  output : pool - the pool pointer
 *
 *
 */
void memory_pool_init(void *allocated_space, size_t size, memory_pool_t **pool);

/**
 *   Function Name: memory_pool_set
 *
 * @brief Description: this function saves a memory pool as an internal
 * var, that way if you alloc and send NULL as the pool, it will
 * use the saved memory pool
 *
 *  @param  input : pool - the pool pointer
 *
 *
 */
void memory_pool_set(memory_pool_t *pool);

/**
 *   Function Name: memory_pool_alloc
 *
 * @brief Description: this function allocate
 * a space in the memory pool and returns
 * the pointer to it (like malloc)
 *
 *  @param  input : pool - the pool pointer
 *  @param  input : size - needed size
 *  @return pointer to space if Ok ,null if error.
 *
 *
 */
void *memory_pool_alloc(memory_pool_t *pool, size_t size);

/**
 *   Function Name: memory_pool_free
 *
 * @brief Description: this function frees
 * a space in the memory pool
 *
 *  @param  input : pool - the pool pointer
 *  @param  input : pointer - pointer to the space
 *
 *
 */
void memory_pool_free(memory_pool_t *pool, void *pointer);

/**
 *   Function Name: memory_pool_destroy
 *
 * @brief Description: this function destroy a memory pool
 *
 *  @param  input : pool - the pool pointer
 *
 *
 */
void memory_pool_destroy(memory_pool_t *pool);

/**
 *   Function Name: memory_pool_get_usage
 *
 * @brief Description: this function returns the current usage of a memory pool
 *
 *  @param  input : pool - the pool pointer
 *  @return usage if Ok ,0 if error.
 *
 */
size_t memory_pool_get_usage(memory_pool_t *pool);

/**
 *   Function Name: memory_pool_get_capacity
 *
 * @brief Description: this function returns the capacity of a memory pool
 *
 *  @param  input : pool - the pool pointer
 *  @return capacity if Ok ,0 if error.
 *
 */
size_t memory_pool_get_capacity(memory_pool_t *pool);

#endif // MEM_POOL_API_H
