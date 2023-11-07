/**
 * @file osal.h
 * @author Orr Mazor
 * @date 21 April 2021
 * @brief File containing header for osal functions and defines
 *
 *
 *
 * This module includes logging system, thread handling and communication


 */
/********************************************************************
*
* Name: osal
* Title: host library osal
* Package title: host library
* Abstract: provide generic functions and definitions for host library
*
*
********************************************************************/
#ifndef OSAL_H
#define OSAL_H
/********************************************************************
* IMPORTS
********************************************************************/
#include "common.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
/********************************************************************
* EXPORTED CONSTANTS
********************************************************************/
/********************************************************************
* EXPORTED TYPES
********************************************************************/
/********************************************************************
* EXPORTED MACROS
********************************************************************/
#ifndef NULL
#define NULL (0)
#endif

#ifdef __linux__
#include "endian.h"

#define htoles(n) htole16(n)
#define htolel(n) htole32(n)
#define letohs(n) le16toh(n)
#define letohl(n) le32toh(n)
#define htobes(n) htobe16(n)
#define htobel(n) htobe32(n)
#define betohs(n) be16toh(n)
#define betohl(n) be32toh(n)

#else
/********************************************************************************
                        ENDIANESS DEFINITIONS
********************************************************************************/
#define swap16(n) ((uint16_t)((((n) << 8) & 0xFF00) | (((n) >> 8) & 0x00FF)))
#define swap32(n) (((swap16((n)&0xFFFF) << 16) & 0xFFFF0000) | \
                   (swap16(((n) >> 16) & 0xFFFF) & 0x0000FFFF))

/*
 *	Following is a set of Host TO Little Endian
 *  macros to support Big/Little endian machines
 *  If the target machine is big endian, please set
 *  BIG_ENDIAN_MACHINE symbol to 1
 */
#ifdef BIG_ENDIAN_MACHINE
#define htoles(n) swap16(n) /* convert host to little endian short */
#define htolel(n) swap32(n) /* convert host to little endian long */
#define letohs(n) swap16(n) /* convert little endian to host short */
#define letohl(n) swap32(n) /* convert little endian to host long */
#define htobes(n) (n)
#define htobel(n) (n)
#define betohs(n) (n)
#define betohl(n) (n)
#else
#define htoles(n) (n)
#define htolel(n) (n)
#define letohs(n) (n)
#define letohl(n) (n)
#define htobes(n) swap16(n) /* convert host to big endian short */
#define htobel(n) swap32(n) /* convert host to big endian long */
#define betohs(n) swap16(n) /* convert big endian to host short */
#define betohl(n) swap32(n) /* convert big endian to host long */
#endif                      /* BIG_ENDIAN_MACHINE */
#endif

#define HLB_LOG_ERR(msg, ...) hlb_log_error("%s [%d]: " msg, __func__, __LINE__, ##__VA_ARGS__);

#define LOG_IF_ERR(res, msg, ...) \
if (res != RES_RESULT_OK) \
{ \
    HLB_LOG_ERR(msg ", res=%d", ##__VA_ARGS__, res); \
}

/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/

/**
 *   Function Name: get_msectime
 *
 * @brief Description: Get the current time in msecs
 *
 *  @return uint32_t : current time in msecs
 *
 *
 */
uint32_t get_msectime();

/**
 *   Function Name: osal_thread_create
 *
 * @brief Description: Create a thread
 *
 *  @param output : handle - thread handle
 *  @param input  : task_func - the thread function
 *  @param input  : param - the parameter that is passed to the thread function.
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t osal_thread_create(void **handle, void *(*task_func)(void *),
                                void *param);

/**
 *   Function Name: osal_thread_delete
 *
 * @brief Description: Delete/destroy a thread
 *
 *  @param input : task_handle - thread handle
 *
 *
 */
void osal_thread_delete(void **task_handle);

/**
 *   Function Name: osal_thread_join
 *
 * @brief Description: wait for termination of the thread
 *
 *  @param input : task_handle - thread handle
 *  @return pthread_join eror number, 0 on success.
 *
 */
int osal_thread_join(void **task_handle);

/**
 *   Function Name: hlb_log_error
 *
 * @brief Description: prints to log at error log level
 *
 *  @param input : message...
 *
 *
 */
void hlb_log_error(const char *message, ...);

/**
 *   Function Name: hlb_log_info
 *
 * @brief Description: prints to log at info log level
 *
 *  @param input : message...
 *
 *
 */
void hlb_log_info(const char *message, ...);

/**
 *   Function Name: hlb_log_debug
 *
 * @brief Description: prints to log at debug log level
 *
 *  @param input : message...
 *
 *
 */
void hlb_log_debug(const char *message, ...);

/**
 *   Function Name: hlb_log_set
 *
 * @brief Description: set logging to on or off
 *
 *  @param on : set logging to on or off
 *
 *
 */
void hlb_log_set(bool on);

/**
 *   Function Name: osal_lock_alloc_size
 *
 * @brief Description: this function returns the needed size to allocate
 * for a lock handle
 *
 *  @return the needed size.
 *
 *
 */
size_t osal_lock_alloc_size();

/**
 *   Function Name: osal_lock_init
 *
 * @brief Description: init a lock
 *
 *  @param input : lock - lock handle
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t osal_lock_init(void *lock);

/**
 *   Function Name: osal_lock_lock
 *
 * @brief Description: lock a lock
 *
 *  @param input : lock - lock handle
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t osal_lock_lock(void *lock);

/**
 *   Function Name: osal_lock_unlock
 *
 * @brief Description: unlocks a lock
 *
 *  @param input : lock - lock handle
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t osal_lock_unlock(void *lock);

/**
 *   Function Name: osal_lock_destroy
 *
 * @brief Description: destroy a lock
 *
 *  @param input : lock - lock handle
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t osal_lock_destroy(void *lock);

/* ETH communication functions */

/**
 *   Function Name: ETH_handle_alloc_size
 *
 * @brief Description: this function returns the needed size to allocate
 * for eth handle
 *
 *  @return status the needed size.
 *
 *
 */
size_t ETH_handle_alloc_size();

/**
 *   Function Name: ETH_get_mac_address_by_interface_name
 *
 * @brief Description: get mac address of interface from the interface name
 *
 *  @param  input : iface_name - the interface name
 *  @param  output : addr - the interface mac address
 *  @return RES_result_t : result status code
 *
 */
RES_result_t ETH_get_mac_address_by_interface_name(const char *iface_name, mac_address_t addr);

/**
 *   Function Name: ETH_rx
 *
 * @brief Description: Attempt to recieve one packet from a connection.
 *
 *  @param  input : xi_con - connection handle
 *  @param  output : xo_pkt -  receive buffer
 *  @param  input,output : xio_len - receive buffer length
 *  @param  input : xi_timeout - [msec] xi_timeout to wait for a packet before returning
 *  @return RES_result_t : result status code
 *  @note
 *	 This function will wait for xi_timeout milliseconds for a packet to arrive
 *   from the line blocking the thread. If no packet arrives after xi_timeout msec
 *   it will return with a timeout status.
 *
 */
RES_result_t ETH_rx(const void *xi_con, void *xo_pkt, size_t *xio_len, int xi_timeout);

/**
 *   Function Name: ETH_tx
 *
 * @brief Description: Send raw ethernet packet over a previously opened connection
 *
 *  @param  input : xi_con - connection handle
 *  @param  input : xi_pkt -  packet to transmit, the routine will format the DA and SA
 *                  fields in the packet based on the xi_con
 *  @param  input : xi_len - packet total length, lengths shorter or longer than the
 *                min/max ethernet frame length will be truncated
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t ETH_tx(const void *xi_con, const void *xi_pkt, size_t xi_len);

/**
 *   Function Name: ETH_break_rx
 *
 * @brief Description: this function causes the blocking call to ETH_rx() to return
 *
 *  @param  input  : xi_con - eth handle
 *
 *
 */
void ETH_break_rx(void *xi_con);

/**
 *   Function Name: CM_connect
 *
 * @brief Description: this function starts communication between
 * the adapter and the destination
 *
 *  @param  input  : xi_adapter_mac_address
 *  @param  input  : xi_dev_mac_address
 *  @param  input  : xi_ether_type
 *  @param  output : xo_handle - eth handle
 *  @return status 0 Ok ,<0 error.
 *
 *
 */
RES_result_t CM_connect(const uint8_t *xi_adapter_mac_address, const uint8_t *xi_dev_mac_address, uint16_t xi_ether_type, void *xo_handle);

/**
 *   Function Name: CM_disconnect
 *
 * @brief Description: this function stops the communication between
 * the adapter and the destination
 *
 *  @param  input  : xi_con - eth handle
 *
 *
 */
void CM_disconnect(void *xi_con);

/**
 *   Function Name: osal_wait_on_timed_condition
 *
 * @brief Description: this function shall block on
 * the indication_db_cond condition variable
 *
 *
 *  @param  input  : num_secs - the number of seconds
 * before a timeout occurs and ETIMEDOUT is returned
 *  @param  input  : cond_lock - the struct containing
 * indication_db_cond and indication_db_lock
 *  @return 0 on success, error number less
 * than 0 shall be returned to indicate the error
 *
 */
int osal_wait_on_timed_condition(int num_secs, void *cond_lock);

/**
 *   Function Name: osal_wait_on_condition
 *
 * @brief Description: this function shall block on
 * the indication_db_cond condition variable
 *
 *
 *  @param  input  : cond_lock - the struct containing
 * indication_db_cond and indication_db_lock
 *  @return 0 on success, error number less
 * than 0 shall be returned to indicate the error
 *
 */
int osal_wait_on_condition(void *cond_lock);

/**
 *   Function Name: osal_cond_lock_lock
 *
 * @brief Description: this function lock the mutex of
 * indication_db_lock
 *
 *
 *  @param  input  : cond_lock - the struct containing
 * indication_db_cond and indication_db_lock
 *  @return 0 on success, error number less
 * than 0 shall be returned to indicate the error
 *
 */
int osal_cond_lock_lock(void *cond_lock);

/**
 *   Function Name: osal_cond_lock_unlock
 *
 * @brief Description: this function unlocks the mutex of
 * indication_db_lock
 *
 *
 *  @param  input  : cond_lock - the struct containing
 * indication_db_cond and indication_db_lock
 *  @return 0 on success, error number less
 * than 0 shall be returned to indicate the error
 *
 */
int osal_cond_lock_unlock(void *cond_lock);

/**
 *   Function Name: osal_release_condition
 *
 * @brief Description: This function unblocks
 * threads blocked on the indication_db_cond
 * condition variable.
 * @param  input  : cond_lock - the struct containing
 * indication_db_cond and indication_db_lock
 *  @return 0 on success, error number less
 * than 0 shall be returned to indicate the error
 *
 *
 */
int osal_release_condition(void *cond_lock);

/**
 *   Function Name: osal_cond_lock_alloc_size
 *
 * @brief Description: this function returns the needed size to allocate
 * for a cond_lock handle
 *
 *  @return the needed size.
 *
 *
 */
size_t osal_cond_lock_alloc_size();

/**
 *   Function Name: osal_cond_lock_init
 *
 * @brief Description: init a condition lock
 *
 *  @param input : cond_lock - condition lock handle
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t osal_cond_lock_init(void *cond_lock);

/**
 *   Function Name: osal_update_timestamp
 *
 * @brief Description: This function updates
 * timestamp_secs with the current time
 *
 *  @return the return status of gettimeofday
 *
 *
 */
int osal_update_timestamp(uint64_t *timestamp_secs);

/**
 *   Function Name: osal_sleep
 *
 * @brief Description: This function
 * makes the running process sleep the amount
 * of secs specified in its parameter
 *
 * @param  input  : secs - the amount of seconds to
 * sleep
 *
 *
 */
void osal_sleep(uint32_t secs);

/**
 *   Function Name: osal_msleep
 *
 * @brief Description: This function
 * makes the running process sleep the amount
 * of msecs specified in its parameter
 *
 * @param  input  : msecs - the amount of msecs to
 * sleep
 *
 *
 */
void osal_msleep(uint32_t msecs);

/**
 *   Function Name: osal_get_file_size
 *
 * @brief Description: This function
 * returns the size of a file
 *
 * @param  input  : file_name - the file we want to get
 * the size of
 * @return the files size
 *
 */
long osal_get_file_size(const char *file_name);
/********************************************************************
* INTERNAL FUNCTIONS DECLARATIONS (FOR UNIT TESTING)
********************************************************************/

#endif // OSAL_H
