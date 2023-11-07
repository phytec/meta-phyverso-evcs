/**
 * @file HLB_apcm.h
 * @author Ronen Kaminsky
 * @date 31 March 2021
 * @brief File containing headers of HomePlug Green PHY Host Library interfaces
 *
 *
 *
 * This module will include API for all SAP Primitives as defined in HPGP
 * specification [1], chapter 12 "Service Access Point Primitives",
 * Specifically, clause 12.2 specifies H1 SAP primitives,


 */
/********************************************************************
*
* Module Name: Host Library APCM API Header
* Title:Host Library APCM API Header
* Package title
* Abstract:
*   This module is API for Lumissil Microsystems GreenPHY CG5317
*
********************************************************************/
#ifndef HLB_APCM_H
#define HLB_APCM_H

#include "common.h"
#include "HLB_protocol.h"

/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/

/**
 *   Function Name: HLB_apcm_set_key_req_send
 *
 * @brief Description: this function  implements the primitive APCM_SET_KEY.REQ.
 *   The APCM_SET_KEY. REQ primitive is used by the HLE to set the NMK of its STA.
 *   Reception of this primitive function causes the STA to leave its existing AVLN
 *   (if it is part of an AVLN) and restart its power-on network procedure.
 *   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.28.
 *
 *  Security level is set by NID last2 bits
 *
 *  @param  input : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                  can be linked with the confirmation.
 *  @param  input : set_key_req - request key parameters
 *
 *  @return RES_result_t : result status code
 *
 *  Currently not supported
 *
 *
 */
RES_result_t HLB_apcm_set_key_req_send(const comm_handle_t handle, HLB_req_id_t req_id, const HLB_hpgp_set_key_req_t *set_key_req);

/**
 *   Function Name: HLB_apcm_set_key_cnf_receive
 *
 * @brief Description: this function implements the primitive APCM_SET_KEY.CNF.
 *   The APCM_SET_KEY.CNF primitive is used by the HLE to set the NMK of its STA.
 *   Reception of this primitive causes the STA to leave its existing AVLN
 *   (if it is part of an AVLN) and restart its power-on network procedure.
 *   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.29.
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input  : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                  can be linked with the confirmation.
 *  @param  output : result -  message status
 *
 *  @return RES_result_t : result status code
 *
 *  Currently not supported
 *
 *
 */
RES_result_t HLB_apcm_set_key_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_result_t *result);

/**
 *   Function Name: HLB_apcm_get_key_req_send
 *
 * @brief Description: this function  implements the primitive APCM_GET_KEY.REQ.
 *   The APCM_GET_KEY.REQ primitive is used by the HLE to obtain the NMK of its STA.
 *   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.30.
 *
 *  @param  input : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                  can be linked with the confirmation.
 *  @return RES_result_t : result status code
 *
 *  Currently not supported
 *
 *
 */
RES_result_t HLB_apcm_get_key_req_send(const comm_handle_t handle, HLB_req_id_t req_id);

/**
*   Function Name: HLB_apcm_get_key_cnf_receive
*
* @brief Description: this function  implements the primitive APCM_GET_KEY.CNF.
*   The APCM_GET_KEY.CNF primitive is used by the STA to provide its NMK to its HLE.
*   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.31.
*
*  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input  : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                  can be linked with the confirmation.
*  @param  output : get_key_cnf - get_key_cnf parameters
*
*  @return RES_result_t : result status code
*
*  Currently not supported
*
*
*/
RES_result_t HLB_apcm_get_key_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_get_key_cnf_t *get_key_cnf);

/**
*   Function Name: HLB_apcm_set_cco_req_send
*
* @brief Description: this function  implements the primitive APCM_SET_CCo.REQ
*   The APCM_SET_CCo.REQ primitive is used by the HLE to configure a Green PHY station to be
*	always CCo, never a CCo or automatically determine whether it should become a CCo.
*   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.57.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input  : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                  can be linked with the confirmation.
*  @param  input : cco_mode -  enum cco_mode
*
*  @return RES_result_t : result status code
*
*
*/
RES_result_t HLB_apcm_set_cco_req_send(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_cco_mode_t cco_mode);

/**
 *   Function Name: HLB_apcm_set_cco_cnf_receive
 *
 * @brief Description: this function  implements the primitive APCM_SET_CCo.CNF
 *   The APCM_SET_CCo.CNF primitive is generated in response to the corresponding
 *	 APCM_SET_CCo.REQ . This primitive indicates successful configuration of the station based
 *	 on the APCM_SET_CCo.REQ.
 *   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.58.
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input  : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                  can be linked with the confirmation.
 *  @param  output : result -  message status
 *
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_set_cco_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_result_t *result);

/**
*   Function Name: HLB_apcm_get_ntb_req_send
*
* @brief Description: this function  implements the primitive APCM_GET_NTB.REQ
*   The APCM_GET_NTB.REQ primitive is used by the HLE to request the Network Time Base
*   from the CCo or its estimate, NTB_STA, from a non-CCo STA.
*   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.12.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                  can be linked with the confirmation.
*
*  @return RES_result_t : result status code
*
*  Currently not supported
*/
RES_result_t HLB_apcm_get_ntb_req_send(const comm_handle_t handle, HLB_req_id_t req_id);

/**
 *   Function Name: HLB_apcm_conn_rel_cnf_receive
 *
 * @brief Description: this function  implements the primitive APCM_GET_NTB.CNF
 *   The APCM_GET_NTB.CNF primitive is used by the CL to provide the current Network Time
 *	 Base to the HLE.
 *   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.10.
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input  : req_id - request id - a number that links between the request and the confirmation.Each API gets a request id so that the request
 *                   can be linked with the confirmation.
 *  @param  output : ntb -  The Network Time Base
 *
 *  @return RES_result_t : result status code
 *
 *  Currently not supported
 */
RES_result_t HLB_apcm_get_ntb_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, uint32_t *ntb);

/**
*   Function Name: HLB_apcm_get_security_mode_req_send
*
* @brief Description: this function  implements the primitive APCM_GET_SECURITY_MODE.REQ
*   The APCM_GET_SECURITY_MODE.REQ primitive is used by the HLE to request the security mode
*   from the STA.
*   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.17.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                   can be linked with the confirmation.
*
*  @return RES_result_t : result status code
*
*  Currently not supported
*/
RES_result_t HLB_apcm_get_security_mode_req_send(const comm_handle_t handle, HLB_req_id_t req_id);

/**
 *   Function Name: HLB_apcm_get_security_mode_cnf_receive
 *
 * @brief Description: this function  implements the primitive APCM_GET_SECURITY_MODE.CNF
 *   APCM_GET_SECURITY_MODE.CNF is generated by the CM in response to the corresponding
 *   APCM_GET_SECURITY_MODE.REQ . This primitive returns the security mode of the network.
 *   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.18.
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input  : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                   can be linked with the confirmation.
 *  @param  output : security_mode_cnf -  The security_mode_cnf parameters
 *
 *  @return RES_result_t : result status code
 *
 *  Currently not supported
 */
RES_result_t HLB_apcm_get_security_mode_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_get_security_mode_cnf_t *security_mode_cnf);

/**
*   Function Name: HLB_apcm_set_security_mode_req_send
*
* @brief Description: this function  implements the primitive APCM_SET_SECURITY_MODE.REQ
*   The APCM_SET_SECURITY_MODE.REQ primitive is used by the HLE to set the security mode
*   to the STA.
*   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.19.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                   can be linked with the confirmation.
*  @param  input : security_mode -  The security_mode to set
*
*  @return RES_result_t : result status code (success/fail)
*
*  Currently not supported
*/
RES_result_t HLB_apcm_set_security_mode_req_send(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_security_mode_t security_mode);

/**
 *   Function Name: HLB_apcm_set_security_mode_cnf_receive
 *
 * @brief Description: this function  implements the primitive APCM_SET_SECURITY_MODE.CNF
 *   APCM_SET_SECURITY_MODE.CNF is generated by the CM in response to the corresponding
 *   APCM_SET_SECURITY_MODE.REQ . This primitive returns if the set of the security mode was successfull.
 *   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.20.
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input  : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                   can be linked with the confirmation.
 *  @param  output : result -  The cnf result
 *
 *  @return RES_result_t : result status code
 *
 *  Currently not supported
 */
RES_result_t HLB_apcm_set_security_mode_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_result_t *result);

/**
*   Function Name: HLB_apcm_get_networks_req_send
*
* @brief Description: this function  implements the primitive APCM_GET_NETWORKS.REQ
*   The APCM_GET_NETWORKS.REQ primitive is used by the HLE to request the networks
*   from the STA.
*    For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.21.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation.Each API gets a request id so that the request
*                   can be linked with the confirmation.
*  @return RES_result_t : result status code
*
*  Currently not supported
*/
RES_result_t HLB_apcm_get_networks_req_send(const comm_handle_t handle, HLB_req_id_t req_id);

/**
 *   Function Name: HLB_apcm_get_networks_cnf_receive
 *
 * @brief Description: this function  implements the primitive APCM_GET_NETWORKS.CNF
 *   APCM_GET_NETWORKS.CNF is generated by the CM in response to the corresponding
 *   APCM_GET_NETWORKS.REQ . This primitive returns the networks found information.
 *   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.22.
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input  : req_id - request id - a number that links between the request and the confirmation.Each API gets a request id so that the request
 *                   can be linked with the confirmation.
 *  @param  output : networks_cnf -  The networks_cnf parameters
 *
 *  @return RES_result_t : result status code
 *
 *  Currently not supported
 */
RES_result_t HLB_apcm_get_networks_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_get_networks_cnf_t *networks_cnf);

/**
*   Function Name: HLB_apcm_set_networks_req_send
*
* @brief Description: this function  implements the primitive APCM_SET_NETWORKS.REQ
*   The APCM_SET_NETWORKS.REQ primitive is used by the HLE to set the networks
*   to the STA. For a chosen Network ID (NID) the possible action are: Join, Leave, Blacklist or Rehabilitate.
*   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.23.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                   can be linked with the confirmation.
*  @param  input : networks -  The networks to set
*
*  @return RES_result_t : result status code
*
*  Currently not supported
*/
RES_result_t HLB_apcm_set_networks_req_send(const comm_handle_t handle, HLB_req_id_t req_id, const HLB_hpgp_set_networks_req_t *networks_req);

/**
 *   Function Name: HLB_apcm_set_networks_cnf_receive
 *
 * @brief Description: this function  implements the primitive APCM_SET_NETWORKS.CNF
 *   APCM_SET_NETWORKS.CNF is generated by the CM in response to the corresponding
 *   APCM_SET_NETWORKS.REQ . This primitive returns if set was successfull
 *   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.24.
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input  : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                   can be linked with the confirmation.
 *  @param  output : result -  The cnf result
 *
 *  @return RES_result_t : result status code
 *
 *  Currently not supported
 */
RES_result_t HLB_apcm_set_networks_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_result_t *result);

/**
*   Function Name: HLB_apcm_get_new_sta_req_send
*
* @brief Description: this function  implements the primitive APCM_GET_NEWSTA.REQ
*   The APCM_GET_NEWSTA.REQ primitive is used by the HLE to request information about
*   the new stations from the STA. The information retreived for each new station is: Mac address
*   Manufacturer human friendly identifier (HFID-MFG) and User human friendly identifier (HFID-USER).
*   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.25.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                   can be linked with the confirmation.
*
*  @return RES_result_t : result status code
*
*  Currently not supported
*/
RES_result_t HLB_apcm_get_new_sta_req_send(const comm_handle_t handle, HLB_req_id_t req_id);

/**
 *   Function Name: HLB_apcm_get_new_sta_cnf_receive
 *
 * @brief Description: this function  implements the primitive APCM_GET_NEWSTA.CNF
 *   APCM_GET_NEWSTA.CNF is generated by the CM in response to the corresponding
 *   APCM_GET_NEWSTA.REQ . This primitive returns the new stations.
 *   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.26.
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input  : req_id - request id - a number that links between the request and the confirmation.Each API gets a request id so that the request
 *                   can be linked with the confirmation.
 *  @param  output : new_sta_cnf -  The new_sta_cnf parameters
 *
 *  @return RES_result_t : result status code
 *
 *  Currently not supported
 */
RES_result_t HLB_apcm_get_new_sta_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_get_new_sta_cnf_t *new_sta_cnf);

/**
 *   Function Name: HLB_apcm_get_new_sta_ind_receive
 *
 * @brief Description: this function  implements the primitive APCM_GET_NEWSTA.IND
 *	APCM_GET_NEWSTA.IND is an indication from the CM to the HLE of the new station.
 *  For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.27.
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input  : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                   can be linked with the confirmation.
 *  @param  output : new_sta_ind -  The new_sta_ind parameters
 *
 *  @return RES_result_t : result status code
 *
 *  Currently not supported
 */
RES_result_t HLB_apcm_get_new_sta_ind_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_get_new_sta_ind_t *new_sta_ind);

/**
*   Function Name: HLB_apcm_sta_restart_req_send
*
* @brief Description: this function  implements the primitive APCM_STA_RESTART.REQ
*   The APCM_STA_RESTART.REQ primitive is used by the HLE to restart the STA. Upon
*   restarting, the STA initiates the power-on network procedure.
*   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.32.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                   can be linked with the confirmation.
*  @return RES_result_t : result status code
*
*  Currently not supported
*/
RES_result_t HLB_apcm_sta_restart_req_send(const comm_handle_t handle, HLB_req_id_t req_id);

/**
 *   Function Name: HLB_apcm_sta_restart_cnf_receive
 *
 * @brief Description: this function  implements the primitive APCM_STA_RESTART.CNF
 *   APCM_STA_RESTART.CNF is generated by the CM in response to the corresponding
 *   APCM_STA_RESTART.REQ . This primitive returns if the sta restarted successfully.
 *   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.33.
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input  : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                   can be linked with the confirmation.
 *  @param  output : result -  The cnf result
 *
 *  @return RES_result_t : result status code
 *
 *  Currently not supported
 */
RES_result_t HLB_apcm_sta_restart_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_result_t *result);

/**
*   Function Name: HLB_apcm_net_exit_req_send
*
* @brief Description: this function  implements the primitive APCM_NET_EXIT.REQ
*   The APCM_NET_EXIT.REQ primitive is used by the HLE to request the STA to leave the AVLN to
*   which it is belongs (if any). There are no parameters for this primitive.
*   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.34.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input  : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                   can be linked with the confirmation.
*  @return RES_result_t : result status code
*
*  Currently not supported
*/
RES_result_t HLB_apcm_net_exit_req_send(const comm_handle_t handle, HLB_req_id_t req_id);

/**
 *   Function Name: HLB_apcm_net_exit_cnf_receive
 *
 * @brief Description: this function  implements the primitive APCM_NET_EXIT.CNF
 *   APCM_NET_EXIT.CNF is generated by the CM in response to the corresponding
 *   APCM_NET_EXIT.REQ . This primitive returns if the sta exited successfully.
 *  For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.35.
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input  : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                   can be linked with the confirmation.
 *  @param  output : result -  The cnf result
 *
 *  @return RES_result_t : result status code
 *
 *  Currently not supported
 */
RES_result_t HLB_apcm_net_exit_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_result_t *result);

/**
*   Function Name: HLB_apcm_set_tone_mask_req_send
*
* @brief Description: this function  implements the primitive APCM_SET_TONE_MASK.REQ
*   The APCM_SET_TONE_MASK.REQ primitive is used to set the tone mask for the station.
*   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.36.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input  : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                   can be linked with the confirmation.
*  @param  input : tone_mask -  This parameter indicates masked carriers from carrier
* 								number 74 through 1228 (1.8 MHz to 30 MHz)
*
*  @return RES_result_t : result status code
*
*  Currently not supported
*/
RES_result_t HLB_apcm_set_tone_mask_req_send(const comm_handle_t handle, HLB_req_id_t req_id, const uint16_t tone_mask);

/**
 *   Function Name: HLB_apcm_set_tone_mask_cnf_receive
 *
 * @brief Description: this function  implements the primitive AAPCM_SET_TONE_MASK.CNF
 *   APCM_SET_TONE_MASK.CNF is generated by the CM in response to the corresponding
 *   APCM_SET_TONE_MASK.REQ . This primitive returns if the set of the tone mask to was successfull.
 *   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.37.
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input  : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                   can be linked with the confirmation.
 *  @param  output : result -  The cnf result.
 *
 *  @return RES_result_t : result status code
 *
 *  Currently not supported
 */
RES_result_t HLB_apcm_set_tone_mask_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_result_t *result);

/**
*   Function Name: HLB_apcm_sta_cap_req_send
*
* @brief Description: this function  implements the primitive APCM_STA_CAP.REQ
*   The APCM_STA_CAP.REQ primitive is a request from HLE to provide the station
*   capabilities.
*   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.38.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input  : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                   can be linked with the confirmation.
*  @return RES_result_t : result status code
*
*  Currently not supported
*/
RES_result_t HLB_apcm_sta_cap_req_send(const comm_handle_t handle, HLB_req_id_t req_id);

/**
 *   Function Name: HLB_apcm_sta_cap_cnf_receive
 *
 * @brief Description: this function  implements the primitive APCM_STA_CAP.CNF
 *   APCM_STA_CAP.CNF is generated by the CM in response to the corresponding
 *   APCM_STA_CAP.REQ . This primitive returns the sta capabilities
 *   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.39.
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input  : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                   can be linked with the confirmation.
 *  @param  output : sta_cap_cnf -  The sta_cap_cnf parameters
 *
 *  @return RES_result_t : result status code
 *
 *  Currently not supported
 */
RES_result_t HLB_apcm_sta_cap_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_sta_cap_cnf_t *sta_cap_cnf);

/**
*   Function Name: HLB_apcm_nw_info_req_send
*
* @brief Description: this function  implements the primitive APCM_NW_INFO.REQ
*   The APCM_NW_INFO.REQ primitive is a request from HLE to provide the list of AVLNs to which
*   the STA is a member and the relevant information about the AVLN.
*   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.40.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                   can be linked with the confirmation.
*  @return RES_result_t : result status code.
*
*  Currently not supported
*/
RES_result_t HLB_apcm_nw_info_req_send(const comm_handle_t handle, HLB_req_id_t req_id);

/**
 *   Function Name: HLB_apcm_nw_info_cnf_receive
 *
 * @brief Description: this function  implements the primitive APCM_NW_INFO.CNF
 *   APCM_NW_INFO.CNF is generated by the CM in response to the corresponding
 *   APCM_NW_INFO.REQ . This primitive returns the network information of the sta.
 *   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.41.
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input  : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                   can be linked with the confirmation.
 *  @param  output : nw_info_cnf -  The nw_info_cnf parameters
 *
 *  @return RES_result_t : result status code.
 *
 *  Currently not supported
 */
RES_result_t HLB_apcm_nw_info_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_nw_info_cnf_t *nw_info_cnf);

/**
*   Function Name: HLB_apcm_get_beacon_req_send
*
* @brief Description: this function  implements the primitive APCM_GET_BEACON.REQ
*   The APCM_GET_BEACON.REQ primitive is a request to provide the Beacon Payload field of a
*   recently received Central Beacon or Proxy Beacon (if station cannot hear the Central
*   Beacon) of an AVLN to which the STA is a member.
*   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.44.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                   can be linked with the confirmation.
*  @param  input : nid - The nid parameter
*
*  @return RES_result_t : result status code
*
*  Currently not supported
*/
RES_result_t HLB_apcm_get_beacon_req_send(const comm_handle_t handle, HLB_req_id_t req_id, const HLB_hpgp_nid_t *nid);

/**
 *   Function Name: HLB_apcm_get_beacon_cnf_receive
 *
 * @brief Description: this function  implements the primitive APCM_GET_BEACON.CNF
 *   APCM_GET_BEACON.CNF is generated by the CM in response to the corresponding
 *   APCM_GET_BEACON.REQ . This primitive returns the beacon
 *   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.45.
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input  : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                   can be linked with the confirmation.
 *  @param  output : get_beacon_cnf -  The get_beacon_cnf parameters
 *
 *  @return RES_result_t : result status code
 *
 *  Currently not supported
 */
RES_result_t HLB_apcm_get_beacon_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_apcm_get_beacon_cnf_t *get_beacon_cnf);

/**
*   Function Name: HLB_apcm_get_hfid_req_send
*
* @brief Description: this function  implements the primitive APCM_GET_HFID.REQ
*   The APCM_GET_HFID.REQ primitive is a request from HLE to provide the Human Friendly
*   Identifier of a STA or an AVLN.
*   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.46.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                   can be linked with the confirmation.
*  @param  input : hfid_req - The hfid_req parameters
*
*  @return RES_result_t : result status code
*
*  Currently not supported
*/
RES_result_t HLB_apcm_get_hfid_req_send(const comm_handle_t handle, HLB_req_id_t req_id, const HLB_hpgp_get_hfid_req_t *hfid_req);

/**
 *   Function Name: HLB_apcm_get_hfid_cnf_receive
 *
 * @brief Description: this function  implements the primitive APCM_GET_HFID.CNF
 *   APCM_GET_HFID.CNF is generated by the CM in response to the corresponding
 *   APCM_GET_HFID.REQ . This primitive returns the human friendly id.
 *   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.47.
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                   can be linked with the confirmation.
 *  @param  output : hfid_cnf -  The hfid_cnf parameters
 *
 *  @return RES_result_t : result status code.
 *
 *  Currently not supported
 */
RES_result_t HLB_apcm_get_hfid_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_get_hfid_cnf_t *hfid_cnf);

/**
*   Function Name: HLB_apcm_set_hfid_req_send
*
* @brief Description: this function  implements the primitive APCM_SET_HFID.REQ
*   The APCM_SET_HFID.REQ primitive is a request from HLE to set the user-defined Human
*   Friendly Identifier of a STA or an AVLN.
*   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.48.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                   can be linked with the confirmation.
*  @param  input : hfid - The hfid parameter.
*
*  @return RES_result_t : result status code.
*
*  Currently not supported
*/
RES_result_t HLB_apcm_set_hfid_req_send(const comm_handle_t handle, HLB_req_id_t req_id, const HLB_hpgp_hfid_t *hfid);

/**
 *   Function Name: HLB_apcm_set_hfid_cnf_receive
 *
 * @brief Description: this function  implements the primitive APCM_SET_HFID.CNF
 *   APCM_SET_HFID.CNF is generated by the CM in response to the corresponding
 *   APCM_SET_HFID.REQ . This primitive returns if the set of the user-defined Human Friendly Identifier was successfull.
 *   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.49.
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input  : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                   can be linked with the confirmation.
 *  @param  output : result -  The cnf result
 *
 *  @return RES_result_t : result status code
 *
 *  Currently not supported
 */
RES_result_t HLB_apcm_set_hfid_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_result_t *result);

/**
*   Function Name: HLB_apcm_set_hd_duration_req_send
*
* @brief Description: this function  implements the primitive APCM_SET_HD_DURATION.REQ
*   The APCM_SET_HD_DURATION.REQ primitive is used to set the Hold Down Duration
*   (HD_Duration) system parameter. A STA shall be capable of Hold Down Durations from zero
*   to 30 seconds.
*   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.50.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                   can be linked with the confirmation.
*  @param  input : hd_duration - the Hold Down Duration in seconds.
*
*  @return RES_result_t : result status code.
*
*  Currently not supported
*/
RES_result_t HLB_apcm_set_hd_duration_req_send(const comm_handle_t handle, HLB_req_id_t req_id, uint8_t hd_duration);

/**
 *   Function Name: HLB_apcm_set_hd_duration_cnf_receive
 *
 * @brief Description: this function  implements the primitive APCM_SET_HD_DURATION.CNF
 *   APCM_SET_HD_DURATION.CNF is generated by the CM in response to the corresponding
 *   APCM_SET_HD_DURATION.REQ . This primitive returns if the set of the Hold Down Duration was successfull.
 *   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.51.
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input  : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                   can be linked with the confirmation.
 *  @param  output : result -  The cnf result
 *
 *  @return RES_result_t : result status code
 *
 *  Currently not supported
 */
RES_result_t HLB_apcm_set_hd_duration_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_result_t *result);

/**
 *   Function Name: HLB_apcm_unassociated_sta_ind_receive
 *
 * @brief Description: this function  implements the primitive APCM_UNASSOCIATED_STA.IND
 *	 The APCM_UNASSOCIATED_STA.IND primitive is used by a STA to inform the HLE that a
 *   CM_UNASSOCIATED_STA.IND has been received.
 *   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.52.
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                   can be linked with the confirmation.
 *  @param  output : unassociated_sta_ind -  The unassociated_sta_ind parameters.
 *
 *  @return RES_result_t : result status code.
 *
 *  Currently not supported
 */
RES_result_t HLB_apcm_unassociated_sta_ind_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_unassociated_sta_ind_t *unassociated_sta_ind);

/**
*   Function Name: HLB_apcm_set_ppkeys_req_send
*
* @brief Description: this function  implements the primitive APCM_SET_PPKEYS.REQ
*   The APCM_SET_PPKEY.REQ primitive is used by the HLE to set the security parameters for a
*   point-to-point encrypted connection (PP-EKS and PPEK). Reception of this primitive causes
*   the STA to store this information, encrypt all future transmissions to the indicated STA using
*   this information, and use these keys to attempt to decrypt and verify MAC Frames received
*   from the indicated station.
*   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.55.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                   can be linked with the confirmation.
*  @param  input : ppkeys_req - the ppkeys_req parameters.
*
*  @return RES_result_t : result status code.
*
*  Currently not supported
*/
RES_result_t HLB_apcm_set_ppkeys_req_send(const comm_handle_t handle, HLB_req_id_t req_id, const HLB_hpgp_set_ppkeys_req_t *ppkeys_req);

/**
 *   Function Name: HLB_apcm_set_ppkeys_cnf_receive
 *
 * @brief Description: this function  implements the primitive APCM_SET_PPKEYS.CNF
 *   APCM_SET_PPKEYS.CNF is generated by the CM in response to the corresponding
 *   APCM_SET_PPKEYS.REQ . This primitive returns if the set of the the security parameters for a point-to-point encrypted connection was successfull.
 *   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.56.
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input  : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                   can be linked with the confirmation.
 *  @param  output : result -  The cnf result.
 *
 *  @return RES_result_t : result status code
 *
 *  Currently not supported
 */
RES_result_t HLB_apcm_set_ppkeys_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_result_t *result);

/**
*   Function Name: HLB_apcm_conf_slac_req_send
*
* @brief Description: this function  implements the primitive APCM_CONF_SLAC.REQ
*   The APCM_CONF_SLAC.REQ primitive is used by the HLE to configure SLAC Protocol at a
*   Green PHY station.
*   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.59.
*
*  @param  input : handle - communication handle - points to an object that contains identifications and resources.
*  @param  input : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
*                   can be linked with the confirmation.
*  @param  input : slac_conf - the slac_conf enum parameter.
*
*  @return RES_result_t : result status code.
*
*  Currently not supported
*/
RES_result_t HLB_apcm_conf_slac_req_send(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_slac_conf_t slac_conf);

/**
 *   Function Name: HLB_apcm_conf_slac_cnf_receive
 *
 * @brief Description: this function  implements the primitive APCM_CONF_SLAC.CNF
 *   APCM_CONF_SLAC.CNF is generated by the CM in response to the corresponding
 *   APCM_CONF_SLAC.REQ . This primitive returns if the configuration was successfull.
 *   For further information please refer to "HomePlug Green PHY Specification"_ver. 1.1.1, section 12.2.2.60.
 *
 *  @param  input  : handle - communication handle - points to an object that contains identifications and resources.
 *  @param  input  : req_id - request id - a number that links between the request and the confirmation. Each API gets a request id so that the request
 *                   can be linked with the confirmation.
 *  @param  output : result -  The cnf result.
 *
 *  @return RES_result_t : result status code.
 *
 *  Currently not supported
 */
RES_result_t HLB_apcm_conf_slac_cnf_receive(const comm_handle_t handle, HLB_req_id_t req_id, HLB_hpgp_result_t *result);

#endif // HLB_APCM_H