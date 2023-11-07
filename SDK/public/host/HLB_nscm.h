/**
 * @file HLB_nscm.h
 * @author Ronen Kaminsky
 * @date 31 March 2021
 * @brief File containing headers of HomePlug Green PHY Host Library interfaces
 *
 *
 *
 * This module will include API for all NSCM Commands

  */
/********************************************************************
*
* Module Name: Host Library NSCM API Header
* Title:Host Library NSCM API Heade
* Package title
* Abstract:
*   This moudle is API for Lumissil Microsystems GreenPHY CG5317
*
********************************************************************/
#ifndef HLB_NSCM_H
#define HLB_NSCM_H

/********************************************************************
* IMPORTS
********************************************************************/

#include "common.h"
#include "shared.h"
#include "HLB_protocol.h"

/*******************************************************************
* CONSTANTS
********************************************************************/
// Declaration of types, structures, enums etc.
/*******************************************************************
* STATIC and GLOBAL DATA
********************************************************************/

//Declarations of const’s and macros representing constants
/*******************************************************************
* TYPES
********************************************************************/
//Declaration of static data and global data
/*******************************************************************
* MACROS
********************************************************************/
//Declaration of Macros representing functions.
/*******************************************************************
* INTERNAL FUNCTIONS
********************************************************************/
//Implementation of internal (static) functions.
/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/

/**
 *   Function Name : HLB_nscm_reset_device_req_send
 *
 *  @brief Description : this function implements soft-reset to the DEVICE.
 *
 *  @param  input : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input : reset_mode - The reset mode of the CG5317
 *
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_reset_device_req_send(const comm_handle_t handle, HLB_hpgp_reset_device_mode_t reset_mode);

/**
*   Function Name: HLB_nscm_get_fw_version_cnf_receive
*
* @brief Description: this function retrieves firmware and configuration binary versions of the CG5317 device.
*
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*  @param  output : version - The firmware and the configuration versions structure.
*
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_get_fw_version_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_fw_version_t *fw_version);

/**
*   Function Name: HLB_nscm_get_fw_version_req_send
*
* @brief Description: this function requests firmware and configuration binary versions of the CG5317 device.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_get_fw_version_req_send(const comm_handle_t handle, HLB_req_id_t req_id);

/**
*   Function Name: HLB_nscm_get_ce2_info_cnf_receive
*
* @brief Description: this function retrieves ce2 information of the CG5317 device.
*
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*  @param  output : ce2_info - The ce2 information structure.
*
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_get_ce2_info_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_ce2_info_t *ce2_info);

/**
*   Function Name: HLB_nscm_get_ce2_info_req_send
*
* @brief Description: this function requests CE2 information from the CG5317 device.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_get_ce2_info_req_send(const comm_handle_t handle, HLB_req_id_t req_id);

/**
*   Function Name: HLB_nscm_get_ce2_info_cnf_receive
*
* @brief Description: this function retrieves ce2 information of the CG5317 device.
*
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*  @param  output : ce2_info - The ce2 information structure.
*
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_get_ce2_info_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_ce2_info_t *ce2_info);

/**
*   Function Name: HLB_nscm_get_ce2_data_req_send
*
* @brief Description: this function requests raw channel samples (CE2) from the CG5317 device.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*  @param  input : data_req - the part of the data we want.
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_get_ce2_data_req_send(const comm_handle_t handle, HLB_req_id_t req_id, const HLB_ce2_data_req_t *data_req);

/**
*   Function Name: HLB_nscm_get_ce2_data_cnf_receive
*
* @brief Description: this function retrieves raw channel samples (CE2) from the CG5317 device.
*                     The raw samples can be converted into channel estimation data using the production tool.
*
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*  @param  output : ce2_data - The ce2 data structure.
*
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_get_ce2_data_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_ce2_data_cnf_t *ce2_data);
/**
*   Function Name: HLB_nscm_get_lnoe_cnf_receive
*
* @brief Description: this function retrieves raw line noise (LNOE) samples from the CG5317 device.
*                     The raw samples can be converted into line noise estimation data using the production tool.
*
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*  @param  output : lnoe_info - The lnoe information structure.
*
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_get_lnoe_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
                                            HLB_lnoe_info_t *lnoe_info);

/**
*   Function Name: HLB_nscm_get_lnoe_req_send
*
* @brief Description: this function requests raw line noise (LNOE) samples from the CG5317 device.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_get_lnoe_req_send(const comm_handle_t handle, HLB_req_id_t req_id);

/**
*   Function Name: HLB_nscm_get_snre_cnf_receive
*
* @brief Description: this function retrieves raw snr (SNRE) samples from the CG5317 device.
*                     The raw samples can be converted into snr estimation data using the production tool.
*
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*  @param  output : snre_info - The snre information structure.
*
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_get_snre_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											HLB_snre_info_t *snre_info);
/**
*   Function Name: HLB_nscm_get_snre_req_send
*
* @brief Description: this function requests raw snr (SNRE) samples from the CG5317 device.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_get_snre_req_send(const comm_handle_t handle, HLB_req_id_t req_id);

/**
*   Function Name: HLB_nscm_abort_dump_action_req_send
*
* @brief Description: this function aborts current dump action,
* can be used in case we get another request is pending failure.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_abort_dump_action_req_send(const comm_handle_t handle, HLB_req_id_t req_id);

/**
*   Function Name: HLB_nscm_abort_dump_action_cnf_receive
*
* @brief Description: this function returns the status of the abort dump action.
*
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*  @param  output : result - The abort result.
*
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_abort_dump_action_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											                            HLB_hpgp_result_t *result);

RES_result_t HLB_nscm_enter_phy_mode_req_send(const comm_handle_t handle, HLB_req_id_t req_id);

RES_result_t HLB_nscm_enter_phy_mode_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											                            HLB_hpgp_result_t *result);

RES_result_t HLB_nscm_read_mem_req_send(const comm_handle_t handle,
                                          HLB_req_id_t req_id,
                                          const HLB_read_mem_req_t *read_mem_req);

RES_result_t HLB_nscm_read_mem_cnf_receive(const comm_handle_t handle,
                                            HLB_req_id_t req_id,
                                            HLB_read_mem_cnf_t *read_mem_cnf);

RES_result_t HLB_nscm_write_mem_req_send(const comm_handle_t handle,
                                          HLB_req_id_t req_id,
                                          const HLB_write_mem_req_t *write_mem_req);

RES_result_t HLB_nscm_write_mem_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											                            HLB_hpgp_result_t *result);

RES_result_t HLB_nscm_get_dc_calib_req_send(const comm_handle_t handle, HLB_req_id_t req_id);

RES_result_t HLB_nscm_get_dc_calib_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											HLB_dc_calib_cnf_t *dc_calib);

/**
*   Function Name: HLB_nscm_get_device_state_req_send
*
* @brief Description: this function sends a request to the modem
*                     to get the devices state.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_get_device_state_req_send(const comm_handle_t handle, HLB_req_id_t req_id);

/**
*   Function Name: HLB_nscm_get_device_state_cnf_receive
*
* @brief Description: this function retrieves the device state of the CG5317 device.
*
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*  @param  output : device_state - The device state information structure.
*
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_get_device_state_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											                              HLB_hpgp_device_state_cnf_t *device_state);

/**
*   Function Name: HLB_nscm_get_d_link_status_req_send
*
* @brief Description: this function sends a request to the modem
*                     to get the D link status.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_get_d_link_status_req_send(const comm_handle_t handle, HLB_req_id_t req_id);

/**
*   Function Name: HLB_nscm_get_d_link_status_cnf_receive
*
* @brief Description: this function retrieves the d link status of the CG5317 device.
*
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*  @param  output : d_link_status - The d_link_status information structure.
*
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_get_d_link_status_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
											                              HLB_hpgp_d_link_status_cnf_t *d_link_status);

/**
*   Function Name: HLB_nscm_link_stats_req_send
*
* @brief Description: this function provides statistics for a Link
*   that is associated with a Connection.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                   can be linked with the confirmation.
*  @param  input : link_stats_req - The link_stats_req parameters
*
*  @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_link_stats_req_send(const comm_handle_t handle, HLB_req_id_t req_id, const HLB_hpgp_link_stats_req_t *link_stats_req);

/**
 *   Function Name: HLB_nscm_link_stats_cnf_receive
 *
 * @brief Description: this function returns the link statistics of the station.
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                   can be linked with the confirmation.
 *  @param  output : link_stats_cnf -  The link_stats_cnf parameters
 *
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_link_stats_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_link_stats_cnf_t *link_stats_cnf);


/**
 *   Function Name: HLB_nscm_host_message_status_ind_receive
 *
 * @brief Description: this function receives AVLN indication
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                   can be linked with the confirmation.
 *  @param  output : host_message_status_ind -  The host_message_status_ind parameters.
 *
 *  @return RES_result_t : result status code.
 *
 *
 */
RES_result_t HLB_nscm_host_message_status_ind_receive(const comm_handle_t handle, HLB_req_id_t req_id,
                                                HLB_hpgp_host_message_status_ind_t *host_message_status_ind);

/**
 *   Function Name: HLB_nscm_d_link_ready_ind_receive
 *
 * @brief Description: this function receives D-Link ready indication
 *
 *  @param  input : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                   can be linked with the confirmation.
 *  @param  output : d_link -  The d link status parameters.
 *
 *  @return RES_result_t : result status code.
 *
 *
 */
RES_result_t HLB_nscm_d_link_ready_ind_receive(const comm_handle_t handle, HLB_req_id_t req_id,
                                                HLB_hpgp_d_link_ready_status_ind_t *d_link);

/**
*   Function Name: HLB_nscm_d_link_terminate_req_send
*
* @brief Description: this function sends a request to the modem
*                     terminate the d link.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_d_link_terminate_req_send(const comm_handle_t handle, HLB_req_id_t req_id);

/**
*   Function Name: HLB_nscm_d_link_terminate_cnf_receive
*
* @brief Description: this function returns the confirmation of the d-link terminate action.
*
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_d_link_terminate_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id);

/**
*   Function Name: HLB_nscm_device_info_req_send
*
* @brief Description: this function sends a device_info get request to the modem.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_device_info_req_send(const comm_handle_t handle, HLB_req_id_t req_id);

/**
*   Function Name: HLB_nscm_device_info_cnf_receive
*
* @brief Description: this function returns the device info of the modem.
*
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*  @param  output : device_info -  The device info parameters.
*
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_device_info_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
                                              HLB_device_info_t *device_info);

/**
*   Function Name: HLB_nscm_get_amp_map_req_send
*
* @brief Description: this function sends a get amplitude map request to the modem.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_get_amp_map_req_send(const comm_handle_t handle, HLB_req_id_t req_id);

/**
*   Function Name: HLB_nscm_get_amp_map_cnf_receive
*
* @brief Description: this function returns the amplitude map of the modem.
*
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                  can be linked with the confirmation.
*  @param  output : amp_map - The Amplitude map.
*
*   @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_nscm_get_amp_map_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id,
                                              HLB_get_amp_map_t *amp_map);

#endif // HLB_NSCM_H