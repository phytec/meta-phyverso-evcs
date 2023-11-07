#ifndef __HLB_ETH_DRIVER__H__
#define __HLB_ETH_DRIVER__H__

/********************************************************************
* IMPORTS
********************************************************************/
#include "common.h"

/********************************************************************
* EXPORTED CONSTANTS
********************************************************************/
/********************************************************************
* EXPORTED TYPES
********************************************************************/
typedef void (*rx_callback) (const uint8_t *frame, uint16_t len, uint32_t timeout_ms);
typedef bool (*rx_filter_callback) (const uint8_t *frame, uint16_t len);

typedef struct
{
    RES_result_t  (*register_rx_callback)   (rx_callback rx_clb, rx_filter_callback filter_clb);
    RES_result_t  (*unregister_rx_callback) (rx_callback rx_clb);
    RES_result_t  (*send_frame)             (const uint8_t *frame, uint16_t len, uint32_t timeout_ms);
} HLB_eth_drv_t;

/********************************************************************
* EXPORTED MACROS
********************************************************************/
/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/

#endif /* __HLB_ETH_DRIVER__H__ */
