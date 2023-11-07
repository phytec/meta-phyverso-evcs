/**
 * @file common.h
 * @author Simon Dinkin
 * @date 20 April 2021
 * @brief File containing common stuff for the Library
 *
 *
 *
 * This module will include common stuff for the Library implementation.
 * such asL structs, enums, helper function, etc..
 */

/********************************************************************
*
* Module Name: Library common Header
* Title:Library common Header
* Package title
* Abstract:
*   This module is common header for Lumissil Microsystems GreenPHY CG5317
*
********************************************************************/
#ifndef COMMON_H
#define COMMON_H

#define MAC_LEN 6 /*!< mac address length */

typedef unsigned char mac_address_t[MAC_LEN]; /*!< mac address type */

typedef void *comm_handle_t; /*!< communication handle */

#define ETHER_TYPE 0x88e1 /*!< IEEE-assigned Ethertype */

/*******************************************************************
* MACROS
********************************************************************/
#define MEMCPY(dst, src, size) (void)memcpy((uint8_t*)dst, (const uint8_t*)src, size)
#define MEMSET(x, val, size) (void)memset((uint8_t*)x, val, size)

#define COPY_UNALIGNED_LE_16_BIT_TO_HOST(to, from) \
{ \
	MEMCPY((uint8_t*)&to, (const uint8_t*)&from, sizeof(uint16_t)); \
	to = letohs(to); \
}

#define COPY_UNALIGNED_LE_32_BIT_TO_HOST(to, from) \
{ \
	MEMCPY((uint8_t*)&to, (const uint8_t*)&from, sizeof(uint32_t)); \
	to = letohl(to); \
}

#define COPY_UNALIGNED_BE_16_BIT_TO_HOST(to, from) \
{ \
	MEMCPY((uint8_t*)&to, (const uint8_t*)&from, sizeof(uint16_t)); \
	to = betohs(to); \
}

#define COPY_HOST_16_BIT_TO_LE_UNALIGNED(to, from) \
{ \
	uint16_t tmp_32bit_val = htoles(from); \
	MEMCPY((uint8_t*)&to, (const uint8_t*)&tmp_32bit_val, sizeof(uint16_t)); \
}

#define COPY_HOST_32_BIT_TO_LE_UNALIGNED(to, from) \
{ \
	uint32_t tmp_32bit_val = htolel(from); \
	MEMCPY((uint8_t*)&to, (const uint8_t*)&tmp_32bit_val, sizeof(uint32_t)); \
}

#define COPY_HOST_16_BIT_TO_BE_UNALIGNED(to, from) \
{ \
	uint16_t tmp_32bit_val = htobes(from); \
	MEMCPY((uint8_t*)&to, (const uint8_t*)&tmp_32bit_val, sizeof(uint16_t)); \
}

/*! Return codes used in all API functions */
typedef enum RES_result
{
	RES_RESULT_OK = 0,				  /*!< Enum value used to indicate successful operation. */
	RES_RESULT_GENERAL_ERROR = -1,	  /*!< Enum value used to indicate general failure. */
	RES_RESULT_NULL_PTR = -2,		  /*!< Enum value used to indicate null pointer. */
	RES_RESULT_BAD_ADDRESS = -3,	  /*!< Enum value used to indicate invalid address. */
	RES_RESULT_ACCESS_DENIED = -4,	  /*!< Enum value used to indicate that access is denied. */
	RES_RESULT_BAD_STATE = -5,		  /*!< Enum value used to indicate invalid state. */
	RES_RESULT_BAD_PARAMETER = -6,	  /*!< Enum value used to indicate invalid argument passed as an parameter. */
	RES_RESULT_NOT_SUPPORTED = -7,	  /*!< Enum value used to indicate that the request is not supported. */
	RES_RESULT_NO_MEMORY = -8,		  /*!< Enum value used to indicate that there is no memory left. */
	RES_RESULT_NO_MORE = -9,		  /*!< Enum value used to indicate that the request cannot be completed in the current state. */
	RES_RESULT_TIMEOUT = -10,		  /*!< Enum value used to indicate that the request has timed out before it could be completed. */
	RES_RESULT_NOT_YET = -11,		  /*!< Enum value used to indicate that the module is not ready to accept/complete the request. */
	RES_RESULT_RESOURCE_IN_USE = -12, /*!< Enum value used to indicate that the requested resource is already in use. */
	RES_RESULT_HW_ABORT = -13,		  /*!< Enum value used to indicate that the requested operation is aborted. */
	RES_RESULT_MISSING_FILE = -14,	  /*!< Enum value used to indicate that integral file is missing */
	RES_RESULT_NOT_FOUND = -15,		  /*!< Enum value used to indicate an item was not found */
	RES_RESULT_INVALID_FW_PCKT = -16, /*!< Enum value used to indicate that FW packet is invalid */
} RES_result_t;

#endif // COMMON_H