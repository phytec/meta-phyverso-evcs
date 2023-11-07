/**
 * @file HLB_noise_floor.h
 * @author Daniel Varennikov
 * @date 07 November 2021
 * @brief File containing the API for the modem noise floor
 *
 *
 *
  */
/********************************************************************
*
* Module Name: Modem noise floor API Header
* Title:Noise floor API Header
* Package title
* Abstract:
*   This module is the API for the CG5317 noise floor data extraction
*
********************************************************************/
#ifndef HLB_NOISE_FLOOR_H
#define HLB_NOISE_FLOOR_H

/********************************************************************
* IMPORTS
********************************************************************/

#include "common.h"
#include <stdint.h>
/*******************************************************************
* CONSTANTS
********************************************************************/
#define SLOG_IF_SLOG_FIRST_ADD_REG 0x61010004U /*!< FW Address containing offset of SLOG start address relative to SLOG address */
#define SLOG_IF_SLOG_LAST_ADD_REG  0x61010008U /*!< FW Address containing offset of last SLOG entry relative to SLOG address */

/*******************************************************************
* STATIC and GLOBAL DATA
********************************************************************/
/*******************************************************************
* TYPES
********************************************************************/
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
 *   Function Name: HLB_get_noise_floor
 *
 *  @brief Description: this function implements extracting Time domain data from the CG5317.
 *
 *  @param  input : handle - The communication handler
 *  @param  input : iterations_num - The number of iterations.
 *  @param  input : gain - The Gain value that will be used in the calculations.
 *  @param  input : slog_ocla_buffer - The buffer containing the slog_ocla list, must be '\0' terminated.
 *  @param  input : fw_slog_buf_size - size of SLOG buffer in FW.
 *  @param  input : sample_size - size of sample to use for noise.
 *                                should be power of 2 and less then or equal to fw_slog_buf_size.
 *  @param  output : time_domain_buffer - The result time domain buffer.
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_get_noise_floor(const comm_handle_t handle,
                                  uint8_t iterations_num,
                                  int32_t gain, char* slog_ocla_buffer,
                                  uint32_t fw_slog_buf_size,
                                  uint32_t sample_size,
                                  int16_t *time_domain_buffer);

#endif