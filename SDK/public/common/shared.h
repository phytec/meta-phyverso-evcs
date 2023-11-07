/**
 * @file shared.h
 * @author Orr Mazor
 * @date 2 May 2021
 * @brief File containing header to be shared with fw
 *
 *
 *
 * This file includes any structs of defines that should be
 * shared with fw


 */
/********************************************************************
*
* Name: shared header
* Title: library shared header
* Package title: host library
* Abstract: shared header with the fw.
* contains the struct definitions and other needed
*
*
********************************************************************/
#ifndef SHARED_HEADER_H
#define SHARED_HEADER_H
/********************************************************************
* IMPORTS
********************************************************************/
#include <stdint.h>
/********************************************************************
* EXPORTED CONSTANTS
********************************************************************/
/********************************************************************
* EXPORTED MACROS
********************************************************************/
#ifdef __GNUC__
#define PACK(__Declaration__) typedef struct __attribute__((__packed__)) { __Declaration__ } 
#else
#ifdef _MSC_VER
#define PACK(__Declaration__) __pragma(pack(push, 1)) typedef struct { __Declaration__ } __pragma(pack(pop))
#endif
#endif

#define MANAGEMENT_MESSAGE_VERSION 1		/*!< version */
#define OUI_LEN 3U							/*!< OUI Length */
#define HLB_LUMUSSIL_OUI_1 0x00				/*!< Lumissil OUI */
#define HLB_LUMUSSIL_OUI_2 0x16				/*!< Lumissil OUI */
#define HLB_LUMUSSIL_OUI_3 0xe8				/*!< Lumissil OUI */
#define HMAC_LEN 6							/*!< Mac Adress Length */
#define HPAVKEY_HFID_LEN 64					/*!< HFID Length */
#define MAX_NUM_OF_NETWORKS 18U				/*!< maximun number of networks in network_cnf struct */
#define MAX_NUM_OF_NEW_STA 10U				/*!< maximun number of new sta's in get_new_sta struct */
#define MAX_NUM_OF_LATENCIES 45				/*!< maximun number of new latencies in link_stats struct */
#define MAX_BEACON_ENTRIES 11U				/*!< maximun number of beacon entries in beacon_mgmt_info struct */
#define ENCRYPTION_KEY_LEN 32				/*!< encryption key length */
#define MAX_BEACON_ENTRY_PAYLOAD_LEN 128	/*!< max payload length of single beacon entry */
#define HLB_PACKET_SIZE 1518				/*!< max size of eth packet */
#define HLB_PACKET_L2_PAYLOAD_SIZE 1500U	/*!< max size of eth packet payload (MTU) */
#define HPAVKEY_NMK_LEN 16					/*!< NMK Length */
#define HPAVKEY_NID_LEN 7U					/*!< NID Length */
#define AMDATA_LEN 1024						/*!< AMDATA Length */
#define MAX_AMDATA_VAL 68.0					/*!< The maximal Amplitude value in dB */
#define MIN_AMDATA_VAL 0.0					/*!< The minimal Amplitude value in dB */
#define MAX_RECEIVER_SENSITIVITY_DB 43		/*!< The minimal Amplitude value in dB */
#define LNOE_DATA_LEN 256					/*!< LNEO data length */
#define CE2_DATA_LEN_SHARED 365 * 4			/*!< CE2 data length */
#define SNRE_DATA_LEN_SHARED 512 * 2	 	/*!< SNRE data length */
#define HLB_READ_MEM_MAX_LEN_SHARED 1452	/*!< Read memory max length (For legacy reasons) */
#define HLB_WRITE_MEM_MAX_LEN_SHARED 1452	/*!< Write memory max length (For legacy reasons) */

#define HLB_BASE_MSG_ID 0xAC00U				/*!< Starting index for Lumissil APCM & NSCM MMTYPEs */
#define HLB_REQUEST 0x00					/*!< Lumissil APCM & NSCM request MMTYPEs */
#define HLB_CONFIRMATION 0x01				/*!< Lumissil APCM & NSCM confirmation MMTYPEs */
#define HLB_INDICATION 0x02					/*!< Lumissil APCM & NSCM indication MMTYPEs */
#define HLB_RESPONSE 0x03					/*!< Lumissil APCM & NSCM response MMTYPEs */

/*! Macro to generate HEX number for MMTYPE of given Lumissil APCM or NSCM message */
#define HLB_GENERATE_MSG(msg_number, msg_type) (HLB_BASE_MSG_ID + (msg_number << 2)) + msg_type

/*! Macro to get MMENTRY fragment number from Greenphy frame */
#define HLB_PACKET_GET_FRAG_IDX(p) ((p)->management_header.fragmetation[0] & 0x0FU)

/*! Macro to get MMENTRY fragments count from Greenphy frame */
#define HLB_PACKET_GET_NUM_FRAGS(p) ((((p)->management_header.fragmetation[0] & 0xF0U) >> 4U) + 1U)

/*! Macro to get MMENTRY FSM from Greenphy frame */
#define HLB_PACKET_GET_FSM(p) ((p)->management_header.fragmetation[1])

/********************************************************************
* EXPORTED TYPES
********************************************************************/

/*! MMTYPE of Lumissil APCM and NSCM messages */
typedef enum
{
	/*! APCM set key request.<br>
		The APCM_SET_KEY.REQ primitive is used by the HLE to set the NMK of its STA.<br>
		Reception of this primitive causes the STA to leave its existing AVLN (if it is part of an AVLN) and restart its power-on network procedure.<br>
		HLB_packet_t.payload = HLB_hpgp_set_key_req_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_SET_KEY_REQ = HLB_GENERATE_MSG(0, HLB_REQUEST),
	/*! APCM set key confirmation.<br>
		The APCM_SET_KEY.CNF indicates whether the APCM_SET_KEY.REQ was succesful or not.<br>
		HLB_packet_t.payload = HLB_8b_status_cnf_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_SET_KEY_CNF = HLB_GENERATE_MSG(0, HLB_CONFIRMATION),
	/*! APCM get key request.<br>
		The APCM_GET_KEY.REQ primitive is used by the HLE to obtain the NMK of its STA.<br>
		HLB_packet_t.payload = N/A
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_GET_KEY_REQ = HLB_GENERATE_MSG(1, HLB_REQUEST),
	/*! APCM get key confirmation.<br>
		The APCM_GET_KEY.CNF primitive is used by the STA to provide its NMK to its HLE.<br>
		HLB_packet_t.payload = HLB_hpgp_get_key_cnf_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_GET_KEY_CNF = HLB_GENERATE_MSG(1, HLB_CONFIRMATION),
	/*! APCM set cco request.<br>
		The APCM_SET_CCo.REQ primitive is used by the HLE to configure a Green PHY station to be
		always CCo, never a CCo or automatically determine whether it should become a CCo.<br>
		HLB_packet_t.payload = HLB_hpgp_set_cco_req_packed_t */
	HLB_PROTOCOL_MSG_ID_APCM_SET_CCO_REQ = HLB_GENERATE_MSG(2, HLB_REQUEST),
	/*! APCM set cco confirmation.<br>
		The APCM_SET_CCo.CNF primitive is generated in response to the corresponding APCM_SET_CCo.REQ .<br>
		This primitive indicates successful configuration of the station based on the APCM_SET_CCo.REQ .<br>
		HLB_packet_t.payload = HLB_8b_status_cnf_packed_t */
	HLB_PROTOCOL_MSG_ID_APCM_SET_CCO_CNF = HLB_GENERATE_MSG(2, HLB_CONFIRMATION),
	/*! APCM connection add request.<br>
		APCM_CONN_ADD.REQ is a request from the HLE to the CM to add a new Connection.
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_CONN_ADD_REQ = HLB_GENERATE_MSG(3, HLB_REQUEST),
	/*! APCM connection add confirmation.<br>
		APCM_CONN_ADD.CNF is generated by the CM in response to the corresponding APCM_CONN_ADD.REQ.<br>
		This primitive indicates whether the Connection request was successful.
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_CONN_ADD_CNF = HLB_GENERATE_MSG(3, HLB_CONFIRMATION),
	/*! APCM connection add indication.<br>
		APCM_CONN_ADD.IND is an indication from the CM to the HLE of the new Connection that is being requested.
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_CONN_ADD_IND = HLB_GENERATE_MSG(3, HLB_INDICATION),
	/*! APCM connection add response.
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_CONN_ADD_RESP = HLB_GENERATE_MSG(3, HLB_RESPONSE),
	/*! APCM connection modify request.<br>
		The APCM_CONN_MOD.REQ primitive is used to change the Connection specification of the STA
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_CONN_MOD_REQ = HLB_GENERATE_MSG(4, HLB_REQUEST),
	/*! APCM connection modify confirmation.<br>
		The APCM_CONN_MOD.CNF indicates whether the APCM_CONN_MOD.REQ was succesful or not.<br>
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_CONN_MOD_CNF = HLB_GENERATE_MSG(4, HLB_CONFIRMATION),
	/*! APCM connection modify indication.
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_CONN_MOD_IND = HLB_GENERATE_MSG(4, HLB_INDICATION),
	/*! APCM connection modify response.
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_CONN_MOD_RESP = HLB_GENERATE_MSG(4, HLB_RESPONSE),
	/*! APCM connection release request.
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_CONN_REL_REQ = HLB_GENERATE_MSG(5, HLB_REQUEST),
	/*! APCM connection release confirmation.
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_CONN_REL_CNF = HLB_GENERATE_MSG(5, HLB_CONFIRMATION),
	/*! APCM connection release indication.
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_CONN_REL_IND = HLB_GENERATE_MSG(5, HLB_INDICATION),
	/*! APCM get NTB request.<br>
		The APCM_GET_NTB.REQ primitive is used by the HLE to request the Network Time Base
		from the CCo or its estimate, NTB_STA, from a non-CCo STA.<br>
		HLB_packet_t.payload = N/A
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_GET_NTB_REQ = HLB_GENERATE_MSG(6, HLB_REQUEST),
	/*! APCM get NTB confirmation.<br>
		The APCM_GET_NTB.CNF primitive is used by the CL to provide the current Network Time Base to the HLE.<br>
		HLB_packet_t.payload = HLB_hpgp_get_nbt_cnf_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_GET_NTB_CNF = HLB_GENERATE_MSG(6, HLB_CONFIRMATION),
	/*! APCM authorize request.<br>
		The APCM_AUTHORIZE.REQ primitive is used to instruct a STA to authorize another STA to join
		its AVLN using DAK-based distribution of the NMK.
		@note Currently not supported by CG5317 FW*/
	HLB_PROTOCOL_MSG_ID_APCM_AUTHORIZE_REQ = HLB_GENERATE_MSG(7, HLB_REQUEST),
	/*! APCM authorize confirmation.<br>
		The APCM_AUTHORIZE.CNF primitive is used by a STA to inform the HLE of the results of its
		request to authorize another STA to join its AVLN using DAK-based distribution of the NMK.
		@note Currently not supported by CG5317 FW*/
	HLB_PROTOCOL_MSG_ID_APCM_AUTHORIZE_CNF = HLB_GENERATE_MSG(7, HLB_CONFIRMATION),
	/*! APCM authorize indication.<br>
		The APCM_AUTHORIZE.IND primitive is used by a STA to inform the HLE that it has been
		authorized by another STA to join its AVLN using DAK-based distribution of the NMK.<br>
		This indication shall only be sent when the protocol terminates due to successful completion, TEK
		lifetime timeout, or abort.
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_AUTHORIZE_IND = HLB_GENERATE_MSG(7, HLB_INDICATION),
	/*! APCM get security mode request.<br>
		The APCM_GET_SECURITY_MODE.REQ primitive is used to retreive the security mode from the STA.<br>
		HLB_packet_t.payload = N/A
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_GET_SECURITY_MODE_REQ = HLB_GENERATE_MSG(8, HLB_REQUEST),
	/*! APCM get security mode confirmation.<br>
		The APCM_GET_SECURITY_MODE.CNF primitive is used by a STA to inform the HLE of its security mode.<br>
		HLB_packet_t.payload = HLB_hpgp_get_security_mode_cnf_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_GET_SECURITY_MODE_CNF = HLB_GENERATE_MSG(8, HLB_CONFIRMATION),
	/*! APCM set security mode request.<br>
		The APCM_SET_SECURITY_MODE.REQ primitive is used by the HLE to set the security mode of its STA.<br>
		HLB_packet_t.payload = HLB_hpgp_set_security_mode_req_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_SET_SECURITY_MODE_REQ = HLB_GENERATE_MSG(9, HLB_REQUEST),
	/*! APCM set security mode confirmation.<br>
		The APCM_SET_SECURITY_MODE.CNF indicates whether the APCM_SET_SECURITY_MODE.REQ was succesful or not.<br>
		HLB_packet_t.payload = HLB_8b_status_cnf_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_SET_SECURITY_MODE_CNF = HLB_GENERATE_MSG(9, HLB_CONFIRMATION),
	/*! APCM get networks request.<br>
		The APCM_GET_NETWORKS.REQ primitive is used to retreive the networks from the STA.<br>
		HLB_packet_t.payload = N/A
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_GET_NETWORKS_REQ = HLB_GENERATE_MSG(10, HLB_REQUEST),
	/*! APCM get networks confirmation.<br>
		The APCM_GET_NETWORKS.CNF primitive is used by the STA to provide its networks to its HLE.<br>
		HLB_packet_t.payload = HLB_hpgp_get_networks_cnf_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_GET_NETWORKS_CNF = HLB_GENERATE_MSG(10, HLB_CONFIRMATION),
	/*! APCM set networks request.<br>
		The APCM_SET_NETWORKS.REQ primitive is used by the HLE to set the networks of its STA.<br>
		For a chosen Network ID (NID) the possible action are: Join, Leave, Blacklist or Rehabilitate.<br>
		HLB_packet_t.payload = HLB_hpgp_set_networks_req_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_SET_NETWORKS_REQ = HLB_GENERATE_MSG(11, HLB_REQUEST),
	/*! APCM set networks confirmation.<br>
		The APCM_SET_NETWORKS.CNF indicates whether the APCM_SET_NETWORKS.REQ was succesful or not.<br>
		HLB_packet_t.payload = HLB_8b_status_cnf_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_SET_NETWORKS_CNF = HLB_GENERATE_MSG(11, HLB_CONFIRMATION),
	/*! APCM get new sta request.<br>
		The information retreived for each new station is: Mac address
		Manufacturer human friendly identifier (HFID-MFG) and User human friendly identifier (HFID-USER).<br>
		HLB_packet_t.payload = N/A
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_GET_NEW_STA_REQ = HLB_GENERATE_MSG(12, HLB_REQUEST),
	/*! APCM get new sta confirmation.<br>
		HLB_packet_t.payload = HLB_hpgp_get_new_sta_cnf_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_GET_NEW_STA_CNF = HLB_GENERATE_MSG(12, HLB_CONFIRMATION),
	/*! APCM get new sta indication.<br>
		HLB_packet_t.payload = HLB_hpgp_get_new_sta_ind_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_GET_NEW_STA_IND = HLB_GENERATE_MSG(12, HLB_INDICATION),
	/*! APCM sta restart request.<br>
		The APCM_STA_RESTART.REQ primitive is used by the HLE to restart the STA.<br>
		Upon restarting, the STA initiates the power-on network procedure.
		HLB_packet_t.payload = N/A
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_STA_RESTART_REQ = HLB_GENERATE_MSG(13, HLB_REQUEST),
	/*! APCM sta restart confirmation.<br>
		The APCM_STA_RESTART.CNF primitive is generated by the STA in response to the corresponding APCM_STA_RESTART.REQ .<br>
		HLB_packet_t.payload = HLB_8b_status_cnf_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_STA_RESTART_CNF = HLB_GENERATE_MSG(13, HLB_CONFIRMATION),
	/*! APCM network exit request.<br>
		The APCM_NET_EXIT.REQ primitive is used by the HLE to request the STA to leave the AVLN to which it is belongs (if any).<br>
		Upon leaving the AVLN, the STA will not rejoin the AVLN until it is powered down and restarted or until it receives an APCM_SET_KEY.REQ or APCM_STA_RESTART.REQ primitive.<br>
		HLB_packet_t.payload = N/A
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_NET_EXIT_REQ = HLB_GENERATE_MSG(14, HLB_REQUEST),
	/*! APCM network exit confirmation.<br>
		The APCM_NET_EXIT.CNF primitive is generated by the STA in response to the corresponding APCM_NET_EXIT.REQ .<br>
		HLB_packet_t.payload = HLB_8b_status_cnf_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_NET_EXIT_CNF = HLB_GENERATE_MSG(14, HLB_CONFIRMATION),
	/*! APCM set tone mask request.<br>
		The APCM_SET_TONE_MASK.REQ primitive is used to set the tone mask for the station.<br>
		HLB_packet_t.payload = HLB_hpgp_set_tone_mask_req_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_SET_TONE_MASK_REQ = HLB_GENERATE_MSG(15, HLB_REQUEST),
	/*! APCM set tone mask confirmation.<br>
		The APCM_SET_TONE_MASK.CNF primitive is generated by the STA in response to the corresponding APCM_SET_TONE_MASK.REQ .<br>
		HLB_packet_t.payload = HLB_8b_status_cnf_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_SET_TONE_MASK_CNF = HLB_GENERATE_MSG(15, HLB_CONFIRMATION),
	/*! APCM sta capabilities request.<br>
		The APCM_STA_CAP.REQ primitive is a request from HLE to provide the station capabilities.<br>
		HLB_packet_t.payload = N/A
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_STA_CAP_REQ = HLB_GENERATE_MSG(16, HLB_REQUEST),
	/*! APCM sta capabilities confirmation.<br>
		The APCM_STA_CAP.CNF primitive is generated in response to the corresponding APCM_STA_CAP.REQ.<br>
		HLB_packet_t.payload = HLB_hpgp_sta_cap_cnf_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_STA_CAP_CNF = HLB_GENERATE_MSG(16, HLB_CONFIRMATION),
	/*! APCM network info request.<br>
		The APCM_NW_INFO.REQ primitive is a request from HLE to provide the list of AVLNs to which the STA is a member and the relevant information about the AVLN.<br>
		HLB_packet_t.payload = N/A
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_NW_INFO_REQ = HLB_GENERATE_MSG(17, HLB_REQUEST),
	/*! APCM network info confirmation.<br>
		The APCM_NW_INFO.CNF primitive is generated in response to the corresponding APCM_NW_INFO.REQ .<br>
		HLB_packet_t.payload = HLB_hpgp_nw_info_cnf_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_NW_INFO_CNF = HLB_GENERATE_MSG(17, HLB_CONFIRMATION),
	/*! NSCM link statistics request.<br>
		The NSCM_LINK_STATS.REQ primitive is a request from HLE to provide statistics for a Link
		that is associated with a Connection.<br>
		HLB_packet_t.payload = HLB_hpgp_link_stats_req_packed_t */
	HLB_PROTOCOL_MSG_ID_NSCM_LINK_STATS_REQ = HLB_GENERATE_MSG(18, HLB_REQUEST),
	/*! NSCM link statistics confirmation.<br>
		The NSCM_LINK_STATS.CNF primitive is generated in response to the corresponding NSCM_LINK_STATS.REQ.<br>
		HLB_packet_t.payload = HLB_hpgp_link_stats_cnf_conf_params_packed_t<br>
		- if transmit_link_flag == 0 then<br>
		    + HLB_packet_t.payload = HLB_hpgp_link_stats_cnf_transmit_packed_t<br>
		- else if transmit_link_flag == 1 then<br>
		    + HLB_packet_t.payload = HLB_hpgp_link_stats_cnf_receive_packed_t */
	HLB_PROTOCOL_MSG_ID_NSCM_LINK_STATS_CNF = HLB_GENERATE_MSG(18, HLB_CONFIRMATION),
	/*! APCM get beacon request.<br>
		The APCM_GET_BEACON.REQ primitive is a request to provide the Beacon Payload field of a
		recently received Central Beacon or Proxy Beacon (if station cannot hear the Central
		Beacon) of an AVLN to which the STA is a member.<br>
		HLB_packet_t.payload = HLB_hpgp_nid_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_GET_BEACON_REQ = HLB_GENERATE_MSG(19, HLB_REQUEST),
	/*! APCM get beacon confirmation.<br>
		The APCM_GET_BEACON.CNF primitive is generated in response to the corresponding APCM_GET_BEACON.REQ .<br>
		HLB_packet_t.payload = HLB_apcm_get_beacon_cnf_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_GET_BEACON_CNF = HLB_GENERATE_MSG(19, HLB_CONFIRMATION),
	/*! APCM get HFID request.<br>
		The APCM_GET_HFID.REQ primitive is a request from HLE to provide the Human Friendly Identifier of a STA or an AVLN.<br>
		HLB_packet_t.payload = HLB_hpgp_get_hfid_req_packed_type1_t
		- if req_type == 2 then<br>
		    + HLB_packet_t.payload = HLB_hpgp_get_hfid_req_packed_type2_t<br>
		- if req_type == 3 then<br>
		    + HLB_packet_t.payload = HLB_hpgp_get_hfid_req_packed_type3_t<br>
		- else<br>
		    + HLB_packet_t.payload = HLB_hpgp_get_hfid_req_packed_type4_t<br>
		@note Currently not supported by CG5317 FW*/
	HLB_PROTOCOL_MSG_ID_APCM_GET_HFID_REQ = HLB_GENERATE_MSG(20, HLB_REQUEST),
	/*! APCM get HFID confirmation.<br>
		The APCM_GET_HFID.CNF primitive is generated in response to the corresponding APCM_GET_HFID.REQ .<br>
		HLB_packet_t.payload = HLB_hpgp_get_hfid_cnf_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_GET_HFID_CNF = HLB_GENERATE_MSG(20, HLB_CONFIRMATION),
	/*! APCM set HFID request.<br>
		HLB_packet_t.payload = HLB_hpgp_hfid_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_SET_HFID_REQ = HLB_GENERATE_MSG(21, HLB_REQUEST),
	/*! APCM set HFID confirmation.<br>
		HLB_packet_t.payload = HLB_8b_status_cnf_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_SET_HFID_CNF = HLB_GENERATE_MSG(21, HLB_CONFIRMATION),
	/*! APCM set HD duration request.<br>
		The APCM_SET_HD_DURATION.REQ primitive is used to set the Hold Down Duration (HD_Duration) system parameter.<br>
		A STA shall be capable of Hold Down Durations from zero to 30 seconds.<br>
		HLB_packet_t.payload = HLB_hpgp_set_hd_duration_req_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_SET_HD_DURATION_REQ = HLB_GENERATE_MSG(22, HLB_REQUEST),
	/*! APCM set HD duration confirmation.<br>
		The APCM_SET_HD_DURATION.CNF primitive confirms setting of the Hold Down Duration (HD_Duration) system parameter.<br>
		HLB_packet_t.payload = HLB_8b_status_cnf_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_SET_HD_DURATION_CNF = HLB_GENERATE_MSG(22, HLB_CONFIRMATION),
	/*! APCM unassociated sta indication.<br>
		The APCM_UNASSOCIATED_STA.IND primitive is used by a STA to inform the HLE that a CM_UNASSOCIATED_STA.IND has been received.<br>
		HLB_packet_t.payload = HLB_hpgp_unassociated_sta_ind_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_UNASSOCIATED_STA_IND = HLB_GENERATE_MSG(23, HLB_INDICATION),
	HLB_PROTOCOL_MSG_ID_APCM_SC_JOIN_REQ = HLB_GENERATE_MSG(24, HLB_REQUEST),
	HLB_PROTOCOL_MSG_ID_APCM_SC_JOIN_CNF = HLB_GENERATE_MSG(24, HLB_CONFIRMATION),
	/*! APCM set PPkeys request.<br>
		The APCM_SET_PPKEY.REQ primitive is used by the HLE to set the security parameters for a point-to-point encrypted connection (PP-EKS and PPEK).<br>
		Reception of this primitive causes the STA to store this information, encrypt all future transmissions to the indicated STA using this information,
		and use these keys to attempt to decrypt and verify MAC Frames received from the indicated station.<br>
		HLB_packet_t.payload = HLB_hpgp_set_ppkeys_req_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_SET_PPKEYS_REQ = HLB_GENERATE_MSG(25, HLB_REQUEST),
	/*! APCM set PPkeys confirmation.<br>
		This primitive is used to inform the HLE of the results of the APCM_SET_PPKEYS.REQ.<br>
		HLB_packet_t.payload = HLB_8b_status_cnf_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_SET_PPKEYS_CNF = HLB_GENERATE_MSG(25, HLB_CONFIRMATION),
	/*! APCM config SLAC request.<br>
		The APCM_CONF_SLAC.REQ primitive is used by the HLE to configure SLAC Protocol at a Green PHY station.<br>
		HLB_packet_t.payload = HLB_hpgp_conf_slac_req_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_CONF_SLAC_REQ = HLB_GENERATE_MSG(26, HLB_REQUEST),
	/*! APCM config SLAC confirmation.<br>
		The APCM_CONF_SLAC.CNF primitive is generated in response to the corresponding APCM_CONF_SLAC.REQ .<br>
		This primitive indicates successful configuration of the station based on the APCM_CONF_SLAC.REQ .<br>
		HLB_packet_t.payload = HLB_8b_status_cnf_packed_t
		@note Currently not supported by CG5317 FW */
	HLB_PROTOCOL_MSG_ID_APCM_CONF_SLAC_CNF = HLB_GENERATE_MSG(26, HLB_CONFIRMATION),
	/*! NSCM get device version request.<br>
		The NSCM_GET_VERSION.REQ primitive is used by the HLE to retrieve Firmware, Host library, and configuration binary versions.<br>
		HLB_packet_t.payload = N/A */
	HLB_PROTOCOL_MSG_ID_NSCM_GET_VERSION_REQ = HLB_GENERATE_MSG(27, HLB_REQUEST),
	/*! NSCM get device version confirmation.<br>
		The NSCM_GET_VERSION.CNF primitive is generated in response to the corresponding NSCM_GET_VERSION.REQ .<br>
		HLB_packet_t.payload = HLB_fw_version_packed_t */
	HLB_PROTOCOL_MSG_ID_NSCM_GET_VERSION_CNF = HLB_GENERATE_MSG(27, HLB_CONFIRMATION),
	/*! NSCM reset device request.<br>
		The NSCM_RESET_DEVICE.REQ primitive is used by the HLE to software reset the CG5317.<br>
		HLB_packet_t.payload = HLB_reset_device_packed_t */
	HLB_PROTOCOL_MSG_ID_NSCM_RESET_DEVICE_REQ = HLB_GENERATE_MSG(28, HLB_REQUEST),
	/*! NSCM get channel estimation 2 info request.<br>
		The NSCM_GET_CE2_INFO.REQ primitive is used by the HLE to retrieve channel estimation information from the device.<br> */
	HLB_PROTOCOL_MSG_ID_NSCM_GET_CE2_INFO_REQ = HLB_GENERATE_MSG(29, HLB_REQUEST),
	/*! NSCM get channel estimation 2 info confirmation.<br>
		The NSCM_GET_CE2_INFO.CNF primitive is generated in response to the corresponding NSCM_GET_CE2_INFO.REQ .<br>
		HLB_packet_t.payload = HLB_ce2_info_packed_t */
	HLB_PROTOCOL_MSG_ID_NSCM_GET_CE2_INFO_CNF = HLB_GENERATE_MSG(29, HLB_CONFIRMATION),
	/*! NSCM get channel estimation 2 data request.<br>
		The NSCM_GET_CE2_DATA.REQ primitive is used by the HLE to retrieve raw channel estimation samples from the device.<br>
		HLB_packet_t.payload = HLB_ce2_data_req_packed_t */
	HLB_PROTOCOL_MSG_ID_NSCM_GET_CE2_DATA_REQ = HLB_GENERATE_MSG(30, HLB_REQUEST),
	/*! NSCM get channel estimation 2 data confirmation.<br>
		The NSCM_GET_CE2_DATA.CNF primitive is generated in response to the corresponding NSCM_GET_CE2_DATA.REQ .<br>
		The raw samples received from CG5317 can be converted into channel estimation data using the production tool.<br>
		HLB_packet_t.payload = HLB_ce2_data_cnf_packed_t */
	HLB_PROTOCOL_MSG_ID_NSCM_GET_CE2_DATA_CNF = HLB_GENERATE_MSG(30, HLB_CONFIRMATION),
	/*! NSCM get LNOE request.<br>
		The NSCM_GET_LNOE.REQ primitive is used by the HLE to retrieve raw line noise estimation samples from the device.<br>
		HLB_packet_t.payload = N/A */
	HLB_PROTOCOL_MSG_ID_NSCM_GET_LNOE_REQ = HLB_GENERATE_MSG(31, HLB_REQUEST),
	/*! NSCM get LNOE confirmation.<br>
		The NSCM_GET_LNOE.CNF primitive is generated in response to the corresponding NSCM_GET_LNOE.REQ .<br>
		The raw samples received from CG5317 can be converted into line noise data using the production tool.<br>
		HLB_packet_t.payload = HLB_lnoe_info_packed_t */
	HLB_PROTOCOL_MSG_ID_NSCM_GET_LNOE_CNF = HLB_GENERATE_MSG(31, HLB_CONFIRMATION),
	/*! NSCM get SNRE request.<br>
		The NSCM_GET_SNRE.REQ primitive is used by the HLE to retrieve raw SNR estimation samples from the device.<br>
		HLB_packet_t.payload = N/A */
	HLB_PROTOCOL_MSG_ID_NSCM_GET_SNRE_REQ = HLB_GENERATE_MSG(32, HLB_REQUEST),
	/*! NSCM get SNRE confirmation.<br>
		The NSCM_GET_SNRE.CNF primitive is generated in response to the corresponding NSCM_GET_SNRE.REQ .<br>
		The raw samples received from CG5317 can be converted into snr estimation data using the production tool.<br>
		HLB_packet_t.payload = HLB_snre_info_packed_t */
	HLB_PROTOCOL_MSG_ID_NSCM_GET_SNRE_CNF = HLB_GENERATE_MSG(32, HLB_CONFIRMATION),
	/*! NSCM abort dump action request.<br>
		The NSCM_ABORT_DUMP_ACTION.REQ primitive is used by the HLE to send a signal to the CG5317 to abort its current action.<br>
		HLB_packet_t.payload = N/A */
	HLB_PROTOCOL_MSG_ID_NSCM_ABORT_DUMP_ACTION_REQ = HLB_GENERATE_MSG(33, HLB_REQUEST),
	/*! NSCM abort dump action confirmation.<br>
		The NSCM_ABORT_DUMP_ACTION.CNF primitive is generated in response to the corresponding NSCM_ABORT_DUMP_ACTION.REQ .<br>
		HLB_packet_t.payload = HLB_32b_status_cnf_packed_t */
	HLB_PROTOCOL_MSG_ID_NSCM_ABORT_DUMP_ACTION_CNF = HLB_GENERATE_MSG(33, HLB_CONFIRMATION),
	/*  NSCM enter phy mode request.<br>
		The NSCM_ABORT_DUMP_ACTION.REQ primitive is used by the HLE to send a signal to the CG5317 to enter PHY mode.<br>
		HLB_packet_t.payload = N/A */
	HLB_PROTOCOL_MSG_ID_NSCM_ENTER_PHY_MODE_REQ = HLB_GENERATE_MSG(34, HLB_REQUEST),
	/*  NSCM enter phy mode confirmation.<br>
		The NSCM_ENTER_PHY_MODE.CNF primitive is generated in response to the corresponding NSCM_ENTER_PHY_MODE.REQ .<br>
		HLB_packet_t.payload = HLB_32b_status_cnf_packed_t */
	HLB_PROTOCOL_MSG_ID_NSCM_ENTER_PHY_MODE_CNF = HLB_GENERATE_MSG(34, HLB_CONFIRMATION),
	/*  NSCM read memory request.<br>
		The NSCM_READ_MEM.REQ primitive is used by the HLE to read the content of CG5317s memory.<br>
		HLB_packet_t.payload = HLB_read_mem_req_packed_t */
	HLB_PROTOCOL_MSG_ID_NSCM_READ_MEM_REQ = HLB_GENERATE_MSG(35, HLB_REQUEST),
	/*  NSCM read memory confirmation.<br>
		The NSCM_READ_MEM.CNF primitive is generated in response to the corresponding NSCM_READ_MEM.REQ .<br>
		It contains the content of the part of CG5317s memory that the HLE asked to read in NSCM_READ_MEM.REQ .<br>
		HLB_packet_t.payload = HLB_read_mem_cnf_packed_t */
	HLB_PROTOCOL_MSG_ID_NSCM_READ_MEM_CNF = HLB_GENERATE_MSG(35, HLB_CONFIRMATION),
	/*  NSCM write memory request.<br>
		The NSCM_WRITE_MEM.REQ primitive is used by the HLE to write to CG5317s memory.<br>
		HLB_packet_t.payload = HLB_write_mem_req_packed_t */
	HLB_PROTOCOL_MSG_ID_NSCM_WRITE_MEM_REQ = HLB_GENERATE_MSG(36, HLB_REQUEST),
	/*  NSCM write memory confirmation.<br>
		The NSCM_WRITE_MEM.CNF primitive is generated in response to the corresponding NSCM_WRITE_MEM.REQ .<br>
		HLB_packet_t.payload = HLB_32b_status_cnf_packed_t */
	HLB_PROTOCOL_MSG_ID_NSCM_WRITE_MEM_CNF = HLB_GENERATE_MSG(36, HLB_CONFIRMATION),
	/*  NSCM get DC calibration request.<br>
		The NSCM_GET_DC_CALIB.REQ primitive is used by the HLE to get the CG5317s DC calibration values.<br>
		HLB_packet_t.payload = N/A */
	HLB_PROTOCOL_MSG_ID_NSCM_GET_DC_CALIB_REQ = HLB_GENERATE_MSG(37, HLB_REQUEST),
	/*  NSCM get DC calibration confirmation.<br>
		The NSCM_GET_DC_CALIB.CNF primitive is generated in response to the corresponding NSCM_GET_DC_CALIB.REQ .<br>
		It contains the CG5317 DC callibration values .<br>
		HLB_packet_t.payload = HLB_dc_calib_cnf_packed_t */
	HLB_PROTOCOL_MSG_ID_NSCM_GET_DC_CALIB_CNF = HLB_GENERATE_MSG(37, HLB_CONFIRMATION),
	/*! NSCM get device state request.<br>
		The NSCM_GET_DEVICE_STATE.REQ requests lower layers to send the device status.<br>
		HLB_packet_t.payload = N/A */
	HLB_PROTOCOL_MSG_ID_NSCM_GET_DEVICE_STATE_REQ = HLB_GENERATE_MSG(38, HLB_REQUEST),
	/*! NSCM get device state confirmation.<br>
		The NSCM_GET_DEVICE_STATE.CNF primitive is generated in response to the corresponding NSCM_GET_DEVICE_STATE.REQ .<br>
		It shall inform higher layers (upon request) what is the current device status.<br>
		HLB_packet_t.payload = HLB_device_state_cnf_packed_t */
	HLB_PROTOCOL_MSG_ID_NSCM_GET_DEVICE_STATE_CNF = HLB_GENERATE_MSG(38, HLB_CONFIRMATION),
	/*! NSCM get D link status request.<br>
		The NSCM_GET_D_LINK_STATUS.REQ requests lower layers to send the link status (established/No link).<br>
		HLB_packet_t.payload = N/A */
	HLB_PROTOCOL_MSG_ID_NSCM_GET_D_LINK_STATUS_REQ = HLB_GENERATE_MSG(39, HLB_REQUEST),
	/*! NSCM get D link status confirmation.<br>
		The NSCM_GET_D_LINK_STATUS.CNF primitive is generated in response to the corresponding NSCM_GET_D_LINK_STATUS.REQ .<br>
		It shall inform higher layers (upon request) what is the current link status.<br>
		HLB_packet_t.payload = HLB_hpgp_d_link_status_packed_cnf_t */
	HLB_PROTOCOL_MSG_ID_NSCM_GET_D_LINK_STATUS_CNF = HLB_GENERATE_MSG(39, HLB_CONFIRMATION),
	/*! NSCM host message indication.<br>
		The NSCM_VS_HOST_MESSAGE_STATUS.IND shall inform higher layers whenever the AVLN status changes.<br>
		HLB_packet_t.payload = HLB_hpgp_host_message_status_ind_packed_t */
	HLB_PROTOCOL_MSG_ID_NSCM_VS_HOST_MESSAGE_STATUS_IND = HLB_GENERATE_MSG(40, HLB_INDICATION),
	/*  NSCM D link ready indication<br>
		To be compliant with Vector, MMTYPE=0xA002 &
		doesn't contain vendor_header<br>
		The NSCM_D_LINK_READY.IND shall inform higher layers about a change of communication link status.<br>
		This indication shall be sent with any change in link status.<br>
		HLB_packet_t.payload = HLB_hpgp_d_link_ready_ind_packed_t */
	HLB_PROTOCOL_MSG_ID_NSCM_D_LINK_READY_IND = 0xA002,
	/*! NSCM D link termintate request.<br>
		The NSCM_D_LINK_TERMINATE.REQ requests lower layers to terminate the data link.<br>
		HLB_packet_t.payload = N/A */
	HLB_PROTOCOL_MSG_ID_NSCM_D_LINK_TERMINATE_REQ = HLB_GENERATE_MSG(42, HLB_REQUEST),
	/*! NSCM D link termintate confirmation.<br>
		The NSCM_D_LINK_TERMINATE.CNF primitive is generated in response to the corresponding NSCM_D_LINK_TERMINATE.REQ .<br>
		HLB_packet_t.payload = N/A */
	HLB_PROTOCOL_MSG_ID_NSCM_D_LINK_TERMINATE_CNF = HLB_GENERATE_MSG(42, HLB_CONFIRMATION),
	/*! NSCM Device info request.<br>
		The NSCM_DEVICE_INFO.REQ requests lower layers to send the device information (TEI, MAC address, NID).<br>
		HLB_packet_t.payload = N/A */
	HLB_PROTOCOL_MSG_ID_NSCM_DEVICE_INFO_REQ = HLB_GENERATE_MSG(43, HLB_REQUEST),
	/*! NSCM Device info confirmation.<br>
		The NSCM_DEVICE_INFO.CNF shall inform higher layers (upon request) with information on the device.<br>
		HLB_packet_t.payload = HLB_hpgp_device_info_packed_t */
	HLB_PROTOCOL_MSG_ID_NSCM_DEVICE_INFO_CNF = HLB_GENERATE_MSG(43, HLB_CONFIRMATION),
	/*! NSCM amplitude map profile request.<br>
		The NSCM_GET_AMP_MAP.REQ requests lower layers to send the device amplitude map.<br>
		HLB_packet_t.payload = N/A */
	HLB_PROTOCOL_MSG_ID_NSCM_GET_AMP_MAP_REQ = HLB_GENERATE_MSG(44, HLB_REQUEST),
	/*! NSCM amplitude map profile confirmation.<br>
		The NSCM_GET_AMP_MAP.CNF shall inform higher layers (upon request) with the values of the configured amplitude map.<br>
		If first fragment:<br>
		HLB_packet_t.payload = HLB_hpgp_get_amp_map_packed_t.<br>
		Or if not first fragment:<br>
		HLB_fragmented_packet_t.payload = HLB_hpgp_get_amp_map_packed_t. */
	HLB_PROTOCOL_MSG_ID_NSCM_GET_AMP_MAP_CNF = HLB_GENERATE_MSG(44, HLB_CONFIRMATION),

	HLB_PROTOCOL_MSG_ID_LAST
} HLB_protocol_msg_id_t;

typedef uint16_t HLB_msg_id_t;	/*!< 16 bit data structure to hold MMTYPE defined by HLB_protocol_msg_id_t */
typedef uint8_t HLB_req_id_t;	/*!< 8 bit data structure to hold vendor request id */

/* general packet structs */

/*! Layer 2 header */
PACK(
	uint8_t dest_mac_addr[HMAC_LEN]; /*!< Destination MAC address */
	uint8_t src_mac_addr[HMAC_LEN];	 /*!< Source MAC address */
	uint16_t eth_type;				 /*!< Ethernet type */
)layer_2_header_t;

/*! Green Phy management header */
PACK(
	uint8_t mmv;				/*!< Management Message Version */
	HLB_msg_id_t msg_id;		/*!< Unique ID of the message */
	uint8_t fragmetation[2];	/*!< fragmetation */
) HLB_management_header_t;

/*! Lumissil vendor header */
PACK(
	uint16_t main_version;		/*!< protocol main version */
	uint16_t version_reserved1; /*!< reserved for future use */
	uint16_t version_reserved2; /*!< reserved for future use */
	HLB_req_id_t req_id;		/*!< request ID used to associate Resp with Req */
	uint32_t reserved1;			/*!< reserved for future use */
	uint32_t reserved2;			/*!< reserved for future use */
	uint32_t reserved3;			/*!< reserved for future use */
) HLB_vendor_header_t;

#define HLB_PACKET_HEADERS_SIZE (sizeof(layer_2_header_t) + sizeof(HLB_management_header_t) + OUI_LEN + sizeof(HLB_vendor_header_t))
#define MSG_PAYLOAD_LIMIT (HLB_PACKET_SIZE - HLB_PACKET_HEADERS_SIZE)

#define HLB_FRAG_PACKET_HEADERS_SIZE (sizeof(layer_2_header_t) + sizeof(HLB_management_header_t))
#define FRAG_MSG_PAYLOAD_LIMIT (HLB_PACKET_SIZE - HLB_FRAG_PACKET_HEADERS_SIZE)

/*! APCM/NSCM message stucture */
PACK(
	layer_2_header_t layer_2_header;		   /*!< Ethernet Layer 2 Header */
	HLB_management_header_t management_header; /*!< managemnt header */
	uint8_t OUI[OUI_LEN];					   /*!< Organizationally Unique Identifier */
	HLB_vendor_header_t vendor_header;		   /*!< vendor header */
	uint8_t payload[MSG_PAYLOAD_LIMIT];		   /*!< Ethernet payload */
) HLB_packet_t;

PACK(
	layer_2_header_t layer_2_header;			/*!< Ethernet Layer 2 Header */
	HLB_management_header_t management_header;	/*!< managemnt header */
	uint8_t payload[FRAG_MSG_PAYLOAD_LIMIT];	/*!< Ethernet payload */
) HLB_fragmented_packet_t;

/* specific packet structs */

/*! simple status payload format */
PACK(
	uint32_t status;	/*!< 0 - success.<br>other - failure. */
) HLB_32b_status_cnf_packed_t;

/*! simple status payload format */
PACK(
	uint8_t status;	/*!< 0 - success.<br>1 - failure. */
) HLB_8b_status_cnf_packed_t;

/*! APCM set key request payload format */
PACK(
	uint8_t NMK[HPAVKEY_NMK_LEN];	/*!< Network Membership Key */
	uint8_t NID[HPAVKEY_NID_LEN];	/*!< Network Identifier (including Security Level) to associate with this NMK, or indicate to use the default NID (all 0xFF bytes) */
	uint8_t security_level;			/*!< Security Level of New NMK (values = HS or SC) - Only relevant if default NID is used */
 ) HLB_hpgp_set_key_req_packed_t;

/*! APCM get key confirmation payload format */
PACK(
	uint8_t NID[HPAVKEY_NID_LEN];	/*!< Network Identifier */
	uint8_t NMK[HPAVKEY_NMK_LEN];	/*!< Network Membership Key */
)HLB_hpgp_get_key_cnf_packed_t;

/*! APCM get security mode confirmation payload format */
PACK(
	uint8_t result;				/*!< (0x0) Success | (0x1) Fail */
	uint8_t security_mode;		/*!< (0x0) Secure | (0x1) Simple-Connect | (0x2) SC-Add | (0x3) SC-Join */
)HLB_hpgp_get_security_mode_cnf_packed_t;

/*! Network Identifier */
PACK(
	uint8_t NID[HPAVKEY_NID_LEN];	/*!< Network Identifier */
) HLB_hpgp_nid_packed_t;

/*! MAC address */
PACK(
	uint8_t macAddress[HMAC_LEN];	/*!< MAC address */
) mac_addr_packed_t;

/*! Human Friendly Identifier */
PACK(
	uint8_t HFID[HPAVKEY_HFID_LEN];	/*!< Human Friendly Identifier */
) HLB_hpgp_hfid_packed_t;

/*! APCM set CCo mode request payload format */
PACK(
	/*! CCo mode:
	 * <br>0x0 - Auto CCo. Currently not supported
	 * <br>0x1 - Never a CCo. In this case, the station will never become the CCo of an AVLN
	 * <br>0x2 - Always a CCo. In this case, the station will not join other AVLNs as a STA */
	uint8_t cco_mode;
) HLB_hpgp_set_cco_req_packed_t;

/*! APCM get network time bsae confirmation payload format */
PACK(
	uint32_t ntb;	/*!< The network time base */
) HLB_hpgp_get_nbt_cnf_packed_t;

/*! APCM set security mode request payload format */
PACK(
	/*! Security mode:
	 * <br>(0x0) Secure | (0x1) Simple-Connect | (0x2) SC-Add | (0x3) SC-Join */
	uint8_t security_mode;
) HLB_hpgp_set_security_mode_req_packed_t;

/*! APCM set tone mask request payload format */
PACK(
	uint16_t tone_mask;	/*!< The tone mask */
) HLB_hpgp_set_tone_mask_req_packed_t;

/*! APCM set hd duration request payload format */
PACK(
	uint8_t hd_duration;	/*!< HD duration */
) HLB_hpgp_set_hd_duration_req_packed_t;

/*! Single network configuration inside payload */
PACK(
	HLB_hpgp_nid_packed_t nid;		/*!< NID of AVLN */
	uint8_t status;					/*!< Status of AVLN: (0x0) Joined | (0x1) Not joined, have NMK | (0x2) Not Joined - no NMK | (0x3) Blacklisted */
	mac_addr_packed_t mac_addr;		/*!< MAC address of AVLN CCo */
	HLB_hpgp_hfid_packed_t hfid;	/*!< HFID of AVLN */
) HLB_hpgp_network_cnf_packed_t;

/*! APCM get networks confirmation payload format */
PACK(
	uint8_t num_of_networks;										/*!< Number of networks found */
	HLB_hpgp_network_cnf_packed_t networks[MAX_NUM_OF_NETWORKS];	/*!< The found networks */
) HLB_hpgp_get_networks_cnf_packed_t;

/*! APCM set networks request payload format */
PACK(
	HLB_hpgp_nid_packed_t nid;	/*!< NID of AVLN */
	uint8_t req_type;			/*!< networks request type: () (0x0) Join now | (0x1) Leave Now | (0x2) Blacklist | (0x3) Rehabilitate */
) HLB_hpgp_set_networks_req_packed_t;

/*! APCM get new sta confirmation payload format */
PACK(
	mac_addr_packed_t mmac;					/*!< MAC address of Sta */
	HLB_hpgp_hfid_packed_t manuf_set_hfid;  /*!< Manufacturer human friendly identifier */
	HLB_hpgp_hfid_packed_t user_set_hfid;   /*!< User human friendly identifier */
) HLB_hpgp_station_packed_t;

/*! APCM get new sta cnf parse payload */
PACK(
	uint8_t num_of_new_sta;										/*!< Number of new stations found */
	HLB_hpgp_station_packed_t stations[MAX_NUM_OF_NEW_STA];		/*!< The found stations */
) HLB_hpgp_get_new_sta_cnf_packed_t;

/*! APCM get new sta indication payload */
PACK(
	uint8_t num_of_new_sta;										/*!< Number of new stations found */
	HLB_hpgp_station_packed_t stations[MAX_NUM_OF_NEW_STA];		/*!< The found stations */
) HLB_hpgp_get_new_sta_ind_packed_t;

/*! APCM sta cap confirmation payload format */
PACK(
	uint8_t av_version;				/*!< HomePlug AV Version */
	mac_addr_packed_t mac_addr;		/*!< MAC address of Sta */
	uint8_t oui[OUI_LEN];			/*!< Organizationally Unique Identifier */
	/*! Auto Connect Capability:
		<br>0x00 = Auto Connect Service not supported
		<br>0x01 = Auto Connect Service supported
		<br>0x02 - 0xFF = reserved */
	uint8_t auto_connect;
	/*! Smoothing Capability:
		<br>0x00 = Smoothing Service not supported
		<br>0x01 = Smoothing Service supported
		<br>0x02 - 0xFF = reserved */
	uint8_t smoothing;
	uint8_t cco_cap;				/*!< CCo Capability - The two LSBs of this field contain the STA’s CCo capability. */
	/*! Proxy Capability:
		<br>0x00 = not capable of being a Proxy Coordinator
		<br>0x01 = capable of being a Proxy Coordinator
		<br>0x02 - 0xFF = reserved */
	uint8_t proxy_capable;
	/*! Backup CCo-capable:
		<br>0x00 = Backup CCo capability not supported
		<br>0x01 = Backup CCo capability supported
		<br>0x02 - 0xFF = reserved */
	uint8_t backup_cco_capable;
	/*! Soft Hand Over Support:
		<br>0x00 = Soft Handover not supported
		<br>0x01 = Soft Hand Over supported
		<br>0x02 - 0xFF = reserved */
	uint8_t soft_handover;
	/*! Two Symbol Frame Control:
		<br>0x00 = not supported
		<br>0x01 = supported
		<br>0x02 - 0xFF = reserved */
	uint8_t two_sym_frame_control;
	/*! Maximum value of FL_AV that the station is capable of supporting in
		multiples of 1.28 * 10^-6 sec.
		<br>0x00 = zero
		<br>0x01 = 1.28 * 10^-6 sec, and so on */
	uint16_t max_fl_av;
	/*! Ability to Support Enhanced Coexistence with HomePlug 1.1
		<br>0x00 = not capable of supporting HomePlug 1.1 coexistence
		<br>0x01 = capable of supporting HomePlug 1.1 coexistence
		<br>0x02 – 0xFF = reserved */
	uint8_t home_plug_1_1_cap;
	/*! HomePlug 1.0.1 Interoperability
		<br>0x00 = not capable of interoperating with HomePlug 1.0.1
		<br>0x01 = capable of interoperating with HomePlug 1.0.1 */
	uint8_t home_plug_1_0_interop;
	/*! Capability of Operating in Various Regulatory Domains
		0x00 = North America only
		0x01 - 0xFF = reserved */
	uint8_t regulatory_cap;
	/*! Bidirectional Bursting Capability
		<br>0x00 = not capable of supporting Bidirectional Bursts
		<br>0x01 = capable of supporting Bidirectional Bursting. Only supports CFP
		Bidirectional Bursts ending with SACK
		<br>0x02 = capable of supporting Bidirectional Bursting. Supports CFP
		Bidirectional Bursts that either end with a SACK or a Reverse SOF.
		<br>0x03-0xFF = reserved */
	uint8_t bidirectional_burst;
	/*! Implementation Version
		This field is defined by the chip and/or product manufacturers. It is
		intended to facilitate interoperability testing. */
	uint16_t implementation_ver;
) HLB_hpgp_sta_cap_cnf_packed_t;

/*! network info format */
PACK(
	/*! Network Identifier
		<br>The least-significant 54 bits of this field contains the NID of the AVLN.
		<br>The remaining 2 bits are set to 0b00. */
	HLB_hpgp_nid_packed_t nid;
	/*! Short Network Identifier
		<br>The least-significant 4 bits of this field contains the Short Network Identifier.
		<br>The remaining 4 bits are set to 0x0. */
	uint8_t short_nid;
	uint8_t terminal_equipment_id; /*!< Terminal Equipment Identifier of the STA in the AVLN */
	/*! Role of the station in the AVLN
		<br>0x00 = STA
		<br>0x01 = Proxy Coordinator
		<br>0x02 = CCo
		<br>0x03 – 0xFF = reserved */
	uint8_t sta_role;
	mac_addr_packed_t cco_mac_addr;		/*!< MAC address of CCo */
	/*! Access Network
		<br>0x00 = This NID corresponds to an in-home network
		<br>0x01 = This NID corresponds to an Access Network
		<br>0x02 - 0xFF = reserved */
	uint8_t access;
	/*! Number of Neighbor Networks that are coordinating with the AVLN
		<br>0x00 = none (Un-Coordinated mode)
		<br>0x01 = one Coordinating network, and so on */
	uint8_t num_cord_neighbor_networks;
) HLB_hpgp_nwinfo_packed_t;

/*! APCM network info cnf payload format */
PACK(
	/*! Number of AVLNs that the STA is a member i.e., Associated and
		Authenticated = N
		<br>0x00 = not a member of any AVLN
		<br>0x01 = member of one AVLN and so on. */
	uint8_t num_of_nw_info;
	HLB_hpgp_nwinfo_packed_t nwinfo[MAX_NUM_OF_NETWORKS];	/*!< Network Information of AVLN */
) HLB_hpgp_nw_info_cnf_packed_t;

/* NSCM link stats req payload */
PACK(
	uint8_t req_type;					/*!< Request Type */
	HLB_hpgp_nid_packed_t nid;			/*!< Network Identifier of the STA(s) whose Connection statistics are being requested. */
	uint8_t transmit_link_flag;			/*!< link stats transmit flag */
	mac_addr_packed_t da_sa_mac_addr;	/*!< MAC address */
) HLB_hpgp_link_stats_req_packed_t;

/*! Link stats transmit format  */
PACK(
	/*! Counter indicating the number of Beacon Periods over which Link
		statistics are collected
		<br>0x00 = zero
		<br>0x01 = one Beacon Period
		<br>0x02 = two Beacon Periods, and so on
		<br>Note: The statistics collection may begin in the middle of a Beacon
		Period. In such cases, the partial Beacon Period is counted as the
		first Beacon Period. */
	uint16_t beacon_period_cnt;
	uint32_t MSDUs;						/*!< Number of MSDUs Received from HLE */
	uint32_t octets;					/*!< Number of Octets of MSDU Payload Received from HLE */
	uint32_t segments_generated;		/*!< Number of Segments that were Generated */
	uint32_t segments_delivered;		/*!< Number of Segments that were successfully delivered */
	uint32_t segments_dropped;			/*!< Number of Segments that were Dropped */
	uint32_t PBs_handed;				/*!< Number of PBs Handed Over to the PHY for Transmission */
	uint32_t MPDUs_transmitted;			/*!< Number of MPDUs That were Transmitted */
	uint32_t MPDUs_successfully_acked;	/*!< Number of MPDUs that were successfully acknowledged */
) HLB_hpgp_link_stats_transmit_packed_t;

/*! Link stats receive format  */
PACK(
	/*! Counter indicating the number of Beacon Periods over which Link
		statistics are collected
		<br>0x00 = zero
		<br>0x01 = one Beacon Period
		<br>0x02 = two Beacon Periods, and so on
		<br>Note: The statistics collection may begin in the middle of a Beacon
		Period. In such cases, the partial Beacon Period is counted as the
		first Beacon Period. */
	uint16_t beacon_period_cnt;
	uint32_t MSDUs;						/*!< Number of MSDUs Received from HLE */
	uint32_t octets;					/*!< Number of Octets of MSDU Payload Received from HLE */
	uint32_t segments_received;			/*!< Number of Segments that were Received */
	uint32_t segments_missed;			/*!< Number of Segments that were Missed */
	uint32_t PBs_handed;				/*!< Number of PBs Handed Over to the PHY for Transmission */
	uint32_t MPDUs_received;			/*!< Number of MPDUs That were Received */
	uint32_t failed_icv_received_frames;/*!< Number of Received MAC Frame for which ICV Failed */
) HLB_hpgp_link_stats_receive_packed_t;

/*! Link stats config params format  */
PACK(
	/*! Result:
	<br>0x00 = Success
	<br>0x01 = Failure */
	uint8_t res_type;
	uint8_t transmit_link_flag;			/*!< link stats transmit flag */
) HLB_hpgp_link_stats_cnf_conf_params_packed_t;

/*! NSCM link stats transmit payload */
PACK(
	HLB_hpgp_link_stats_cnf_conf_params_packed_t params;
	HLB_hpgp_link_stats_transmit_packed_t transmit_link;
) HLB_hpgp_link_stats_cnf_transmit_packed_t;

/*! NSCM link stats receive payload */
PACK(
	HLB_hpgp_link_stats_cnf_conf_params_packed_t params;
	HLB_hpgp_link_stats_receive_packed_t receive_link;
) HLB_hpgp_link_stats_cnf_receive_packed_t;

/*! beacon entry format */
PACK(
	uint8_t header;															/*!< Beacon Entry Header */
	uint8_t len;															/*!< Beacon Entry Length */
	uint8_t payload[MAX_BEACON_ENTRY_PAYLOAD_LEN];							/*!< Beacon Entry */
) HLB_hpgp_beacon_entry_packed_t;

/*! beacon management information format */
PACK(
	uint8_t num_of_beacon_entries;											/*!< Number of Beacon Entries */
	HLB_hpgp_beacon_entry_packed_t beacon_entries[MAX_BEACON_ENTRIES];		/*!< Beacon entries */
) HLB_hpgp_beacon_mgmt_info_packed_t;

/*! CM_GET_BEACON.CNF struct */
PACK(
	HLB_hpgp_nid_packed_t nid_and_hybrid_mode; 	/*!< last 2 msb are hybrid mode */
	uint8_t source_terminal_equipment_id;		/*!< Source Terminal Equipment Identifier */
	/*! Bits 0-2 = Beacon Type
		<br>Bit 3 = Non-Coordinating Networks Reported
		<br>Bit 4 = Network Power Saving Mode
		<br>Bits 5-7 = Number of Beacon Slots */
	uint8_t BT_NCNR_NPSM_NumSlots;
	uint8_t beacon_slot_usage;					/*!< Beacon Slot Usage */
	/*! Bits 0-2 = Beacon Slot ID
		<br>Bits 3-5 = AC Line Cycle Synchronization Status
		<br>Bit 6 = Handover-In-Progress
		<br>Bit 7 = RTS Broadcast Flag */
	uint8_t SlotID_ACLSS_HOIP_RTSBF;
	/*! Bits 0-1 = Network Mode
		<br>Bits 2-3 = CCo Capability
		<br>Bits 4-7 = reserved */
	uint8_t network_mode_cco_RSF_Plevel;
	HLB_hpgp_beacon_mgmt_info_packed_t beacon_mgmt_info;	/*!< Beacon management information */
) HLB_apcm_get_beacon_cnf_packed_t;

/*! APCM Conf slac request payload */
PACK(
	/*! 0x00 = Disable SLAC: The Green PHY station will not transmit or receive any SLAC related messages
		0x01 = Disable Receiver: Enable SLAC functions associated with the transmitter of M-Sounds and to disable SLAC functions associated with receiver of M-Sounds
		0x02 = Enable SLAC Receiver and Disable Transmitter: Enable SLAC function associated with the receiver of M-Sounds and to disable SLAC functions associated with transmitter of M-Sounds */
	uint8_t slac_conf;
) HLB_hpgp_conf_slac_req_packed_t;

/*! APCM unassociated sta indication payload */
PACK(
	uint8_t nid[HPAVKEY_NID_LEN];	/*!< Network Identifier */
	/*! 0x00 = does not support QoS and TDMA	
		0x01 = supports QoS and TDMA but only in Uncoordinated Mode
		0x02 = supports QoS and TDMA in Coordinated Mode
		0x03 = future CCo capabilities */
	uint8_t cco_cap;
) HLB_hpgp_unassociated_sta_ind_packed_t;

/*! APCM set ppkeys req payload */
PACK(
	uint8_t pp_eks[ENCRYPTION_KEY_LEN]; /*!< Encryption Key select for Point to Point encryption key select */
	uint8_t ppek[ENCRYPTION_KEY_LEN];	/*!< Point-to-Point Encryption Key */
	mac_addr_packed_t other_mac_addr;	/*!< MAC address of other STA */
) HLB_hpgp_set_ppkeys_req_packed_t;

/*! APCM get hfid req payload */
PACK(
	/*! Request Type
		<br>0x00 = request to provide the manufacturer-set HFID of the STA
		<br>0x01 = request to provide the user-set HFID of the STA
		<br>0x02 = request to provide the HFID of the Network, whose network identifier is contained in the NID field
		<br>0x03 = request to set the user-set HFID of the STA to the value indicated in the HFID field
		<br>0x04 = request to set the HFID of the Network, whose network identifier is contained in the NID field, to the value indicated in the HFID field.
		<br>0x05 - 0xFF = reserved
		<br>Note: This message must be sent to the CCo of an AVLN to set the HFID of the Network */
	uint8_t req_type;
) HLB_hpgp_get_hfid_req_packed_type1_t;

/*! APCM get hfid req payload */
PACK(
	/*! Request Type
		<br>0x00 = request to provide the manufacturer-set HFID of the STA
		<br>0x01 = request to provide the user-set HFID of the STA
		<br>0x02 = request to provide the HFID of the Network, whose network identifier is contained in the NID field
		<br>0x03 = request to set the user-set HFID of the STA to the value indicated in the HFID field
		<br>0x04 = request to set the HFID of the Network, whose network identifier is contained in the NID field, to the value indicated in the HFID field.
		<br>0x05 - 0xFF = reserved
		<br>Note: This message must be sent to the CCo of an AVLN to set the HFID of the Network */
	uint8_t req_type;
	HLB_hpgp_nid_packed_t nid;			/*!< Network identifier */
) HLB_hpgp_get_hfid_req_packed_type2_t;

/*! APCM get hfid req payload */
PACK(
	/*! Request Type
		<br>0x00 = request to provide the manufacturer-set HFID of the STA
		<br>0x01 = request to provide the user-set HFID of the STA
		<br>0x02 = request to provide the HFID of the Network, whose network identifier is contained in the NID field
		<br>0x03 = request to set the user-set HFID of the STA to the value indicated in the HFID field
		<br>0x04 = request to set the HFID of the Network, whose network identifier is contained in the NID field, to the value indicated in the HFID field.
		<br>0x05 - 0xFF = reserved
		<br>Note: This message must be sent to the CCo of an AVLN to set the HFID of the Network */
	uint8_t req_type;
	HLB_hpgp_hfid_packed_t hfid;		/*!< Human friendly identifier */
) HLB_hpgp_get_hfid_req_packed_type3_t;

/*! APCM get hfid req payload */
PACK(
	/*! Request Type
		<br>0x00 = request to provide the manufacturer-set HFID of the STA
		<br>0x01 = request to provide the user-set HFID of the STA
		<br>0x02 = request to provide the HFID of the Network, whose network identifier is contained in the NID field
		<br>0x03 = request to set the user-set HFID of the STA to the value indicated in the HFID field
		<br>0x04 = request to set the HFID of the Network, whose network identifier is contained in the NID field, to the value indicated in the HFID field.
		<br>0x05 - 0xFF = reserved
		<br>Note: This message must be sent to the CCo of an AVLN to set the HFID of the Network */
	uint8_t req_type;
	HLB_hpgp_nid_packed_t nid;			/*!< Network identifier */
	HLB_hpgp_hfid_packed_t hfid;		/*!< Human friendly identifier */
) HLB_hpgp_get_hfid_req_packed_type4_t;

/*! APCM get hfid cnf payload */
PACK(
	/*! Request Type
		<br>0x00 = request to provide the manufacturer-set HFID of the STA
		<br>0x01 = request to provide the user-set HFID of the STA
		<br>0x02 = request to provide the HFID of the Network, whose network identifier is contained in the NID field
		<br>0x03 = request to set the user-set HFID of the STA to the value indicated in the HFID field
		<br>0x04 = request to set the HFID of the Network, whose network identifier is contained in the NID field, to the value indicated in the HFID field.
		<br>0x05 - 0xFF = reserved
		<br>Note: This message must be sent to the CCo of an AVLN to set the HFID of the Network */
	uint8_t req_type;
	HLB_hpgp_hfid_packed_t hfid;		/*!< Human friendly identifier */
) HLB_hpgp_get_hfid_cnf_packed_t;

/*! NSCM device get version confirmation payload format */
PACK(
	uint16_t fw_version_major;		   /*!< firmware version major (1st) */
	uint16_t fw_version_minor;		   /*!< firmware version minor (2nd) */
	uint16_t fw_version_sub;		   /*!< firmware version sub version (3rd) */
	uint16_t fw_version_build;		   /*!< firmware version build number (4th) */
	uint16_t fw_version_patch_version; /*!< patch version */
) HLB_fw_version_packed_t;

/*! CG Network Membership Key */
PACK(
	uint8_t NMK[HPAVKEY_NMK_LEN]; /*!< Network Membership Key */
) hpgp_nmk_packed_t;

/*! BIN amdata values */
PACK(
	uint32_t amdata[AMDATA_LEN];	/*!< Amplitude map data */
) amplitude_map_packed_t;

/*! HLB version */
PACK(
	uint16_t version_major; /*!< host lib version major (1st) */
	uint16_t version_minor; /*!< host lib version minor (2nd) */
	uint16_t version_patch; /*!< host lib version patch (3rd) */
) version_packed_t;

/*! The minimal CG BIN configuration parameters */
PACK(
	version_packed_t version;					/*!< Current version */
	uint32_t configuration_binary_version;		/*!< Current binary version */
	mac_addr_packed_t mac_address;				/*!< CG mac address */
	hpgp_nmk_packed_t nmk;						/*!< CG Network Membership Key */
	/*! CCo mode:
	 * <br>0x0 - Auto CCo. Currently not supported
	 * <br>0x1 - Never a CCo. In this case, the station will never become the CCo of an AVLN
	 * <br>0x2 - Always a CCo. In this case, the station will not join other AVLNs as a STA */
	uint8_t cco_role;
	uint8_t reserved[3]; 						/*!< padding> */
	uint32_t tdu_dacs_b1;						/*!< TDU_DACS_B1 */
	uint32_t tdu_dacs_shift;					/*!< TDU_DACS_SHIFT */
) PROD_minimal_config_params_packed_t;

/*! The minimal CG BIN configuration */
PACK(
	PROD_minimal_config_params_packed_t config;	/*!< BIN min config params */
	amplitude_map_packed_t amplitude_map;		/*!< BIN amdata values */
) PROD_minimal_configuration_content_packed_t;

#ifdef CG5315
PACK(
	PROD_minimal_configuration_content_packed_t min_conf;
) PROD_configuration_content_packed_t;
#else
PACK(
	PROD_minimal_configuration_content_packed_t min_conf; /*!< The minimal CG BIN configuration */
	uint8_t host_interface;						/*!< The enabled Host interface: <0(SPI), 1(ETH R/MII)> */
	HLB_hpgp_hfid_packed_t manu_hfid;			/*!< The Manufcaturer Human friendly identifier */
	HLB_hpgp_hfid_packed_t user_hfid;			/*!< The Users Human friendly identifier */
	HLB_hpgp_hfid_packed_t avln_hfid;			/*!< The AVLN Human friendly identifier */
	uint8_t plc_freq_sel;						/*!< PLC Frequency Selection: <0(50Hz), 1(60Hz), 2(External Signal)> */
	HLB_hpgp_nid_packed_t nid;					/*!< Network Identifier (all 0xFF bytes indicates use of default NID) */
	uint8_t security_level;						/*!< Security Level of New NMK (values = HS or SC) - Only relevant if default NID is used */
	/*! Maximal Receiver Sensitivity configuration.
	 * <br>The receiver sensitivity can be controlled by setting a limitation on the maximum gain of the AGC.
	 * <br>Values range between a minimum of 3dB and a maximum of 43dB. */
	uint8_t receiver_max_sensitivity;
	/*! The drive-strength of HOST1 (RXD_0), HOST2 (RXD_1), HOST3 (RXD_2), HOST4 (RXD_3), HOST5 (RX_DV) pins. */
	uint8_t cg_drive_strength_rxd_0_1_2_3_and_rx_dv;
	/*! The drive-strength of HOST6 (TX_CLK) pin. */
	uint8_t cg_drive_strength_tx_clk;
	/*! Set value of AMB_DSP_AFETXDACTAB_IDX to control The DAC-Mult Gain. */
	uint8_t amb_tx_dac_table_idx;
	/*! Set value of AMB_DSP_AFETXDAC_SFT to control The DAC-Mult Gain. */
	uint8_t amb_tx_dac_shift_table_idx;
	/*! Set value of AMB_TOP_AMB_PARAMETERS_A to control The The AMB Tx gain. */
	uint8_t analog_gain_code;
	/*! PSD gain used by the host to parse back the PSD gain from a binary user config back to text format.
	 * <br>Field is ignored by CG5317 FW. */
	uint32_t host_internal_data_PSD_gain;
) PROD_configuration_content_packed_t;
#endif

/*! NSCM reset device request payload format */
PACK(
	uint8_t stop_in_hostboot;	/*!< 0 - normal reset, 1 - reset and stop in hostboot */
) HLB_reset_device_packed_t;

/*! NSCM Channel estimation 2 info confirmation payload format */
PACK(
	uint32_t status; 			/*!< 0-success, 1-general failure */
	int32_t gain; 				/*!< current gain */
	int32_t ce2_shift;			/*!< ce2 shift */
	uint32_t total_buffer_size;	/*!< The size of the whole CE2 buffer */
) HLB_ce2_info_packed_t;

/*! NSCM Channel estimation 2 data request payload format */
PACK(
	uint32_t chunk_number;	/*!< from 0-5, must come in order, 5 will cause fw to free buffer after sending */
) HLB_ce2_data_req_packed_t;

/*! NSCM Channel estimation 2 data confirmation payload format */
PACK(
	uint32_t status; 					/*!< 0-success, 1-general failure */
	uint32_t size; 						/*!< size of buffer in words(32bit) if zero something has failed */
	int8_t data[CE2_DATA_LEN_SHARED];	/*!< buffer, up to 365 words (of 32bit) */
) HLB_ce2_data_cnf_packed_t;

/*! NSCM LNOE info confirmation payload format */
PACK(
	uint32_t status; 				/*!< 0-success, 1-general failure */
	int32_t gain; 					/*!< current gain */
	int8_t data[LNOE_DATA_LEN]; 	/*!< data buffer */
) HLB_lnoe_info_packed_t;

/*! NSCM SNRE info confirmation payload format */
PACK(
	uint32_t status; 					/*!< 0-success, 1-general failure */
	int8_t snre[SNRE_DATA_LEN_SHARED]; 	/*!< data buffer */
) HLB_snre_info_packed_t;

/*! NSCM read memory request payload format */
PACK(
	uint32_t address; 		/*!< The address to read from */
	uint32_t size; 			/*!< The amount to read */
) HLB_read_mem_req_packed_t;

/*! NSCM read memory confirmation payload format */
PACK(
	uint32_t size; 								/*!< size of buffer in bytes(8bit) if zero something has failed */
	uint8_t data[HLB_READ_MEM_MAX_LEN_SHARED];	/*!< memory buffer */
) HLB_read_mem_cnf_packed_t;

/*! NSCM write memory request payload format */
PACK(
	uint32_t address; 							/*!< The address to write to */
	uint32_t size; 								/*!< The amount to write */
	uint8_t data[HLB_WRITE_MEM_MAX_LEN_SHARED];	/*!< The data to write */
) HLB_write_mem_req_packed_t;

/*! NSCM get modem dc callibration confirmation payload format */
PACK(
	uint8_t dc_offset_hi_ch1;	/*!< DC offset high */
    uint8_t dc_offset_lo_ch1;	/*!< DC offset low */
) HLB_dc_calib_cnf_packed_t;

/*! NSCM get device state confirmation payload format */
PACK(
	/*! Device state:
		<br>0x1 = unassociated
		<br>0x2 = waiting for sta sync
		<br>0x3 = associating
		<br>0x4 = authenticating
		<br>0x5 = operational Sta
		<br>0x6 = operational CCo */
    uint8_t device_state;
) HLB_device_state_cnf_packed_t;

/*! NSCM D Link status confirmation payload format */
PACK(
	/*! D Link status:
		<br>0 - No link
        <br>1 - Link Established */
    uint8_t d_link_state;
) HLB_hpgp_d_link_status_packed_cnf_t;

/*! NSCM Host message indication payload format */
PACK(
	/*! Host message Status:
		<br>1 - Ready to join AVLN
        <br>2 - Joined AVLN
        <br>3 - Disconnected from AVLN */
    uint8_t host_message_status;
) HLB_hpgp_host_message_status_ind_packed_t;

/*! NSCM D Link indication payload format */
PACK(
	/*! D Link ready status:
		<br>0 - No link
        <br>1 - Link Established */
    uint8_t d_link_ready_status;
) HLB_hpgp_d_link_ready_ind_packed_t;

/*! NSCM Device info confirmation payload format */
PACK(
	/*! Connectivity configurations:<br>
		Bits[1:0] - CCo role:<br>
		0x1 STA | 0x2 CCo<br>
		Bits[3:2] - Host Interface:<br>
		0x0 SPI | 0x1 ETH | 0x2 SPI & ETH (data received on both, device reply only to SPI)<br>
		Bits[7:4] - Reserved */
	uint8_t connectivity_config;
	uint8_t terminal_equipment_id;	/*!< Terminal Equipment Identifier */
	mac_addr_packed_t mac_addr;		/*!< MAC address of AVLN */
	hpgp_nmk_packed_t nmk;			/*!< Network Membership Key */
	HLB_hpgp_nid_packed_t nid;		/*!< Network Identifier (all 0xFF bytes indicates use of default NID) */
	uint8_t security_level;			/*!< Security Level of New NMK (values = HS or SC) - Only relevant if default NID is used */
	uint8_t plc_freq_sel;			/*!< PLC Frequency Selection: <0(50Hz), 1(60Hz), 2(External Signal)> */
	/*! Short Network Identifier
		<br>The least-significant 4 bits of this field contains the Short Network Identifier.
		<br>The remaining 4 bits are set to 0x0. */
	uint8_t snid;
	HLB_hpgp_hfid_packed_t manufacturer_hfid;	/*!< Manufacturer HFID (Human Friendly Identifier) */
	HLB_hpgp_hfid_packed_t user_hfid;			/*!< User HFID (Human Friendly Identifier) */
	HLB_hpgp_hfid_packed_t avln_hfid;			/*!< AVLN HFID (Human Friendly Identifier) */
	/*! Maximal Receiver Sensitivity configuration.
	 * <br>The receiver sensitivity can be controlled by setting a limitation on the maximum gain of the AGC.
	 * <br>Values range between a minimum of 3dB and a maximum of 43dB. */
	uint8_t max_receiver_sensitivity;
) HLB_hpgp_device_info_packed_t;

/*! NSCM Get Amplitude map confirmation payload format */
PACK(
	/*! 4096 Octets - Amplitude MAP** (2048 values of UNIT16, linear representation).<br>
		The message will be fragmented to multiple frames, size of amp_map in each frame varies.<br>
		** Amplitude map - the device will send to the host a vector of 2048 values,<br>
		each 16 bits wide (linear representation).<br>
		It is up to the user to implement at the host the conversion to dB */
	uint8_t amp_map[FRAG_MSG_PAYLOAD_LIMIT];
) HLB_hpgp_get_amp_map_packed_t;

/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/
/********************************************************************
* INTERNAL FUNCTIONS DECLARATIONS (FOR UNIT TESTING)
********************************************************************/

#endif //SHARED_HEADER_H
