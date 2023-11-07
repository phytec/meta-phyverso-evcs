/**
 * @file HLB_protocol.h
 * @author Orr Mazor
 * @date 21 April 2021
 * @brief File containing header of protocol module
 *
 *
 *
 * This module will include API to be used by HLB_apcm and others
 * to create and parse hlb msgs from user type msgs to
 * fw type msgs and backwards.


 */
/********************************************************************
*
* Name: HLB protocol
* Title: host library protocol
* Package title: host library
* Abstract: provide create & parse to msg api
*
*
********************************************************************/
#ifndef HLB_PROTOCOL_H
#define HLB_PROTOCOL_H
/********************************************************************
* IMPORTS
********************************************************************/
#include "shared.h"
#include "common.h"
#include <stddef.h>
/********************************************************************
* EXPORTED CONSTANTS
********************************************************************/
#define QMP_BODY_LEN 64									/*!< QoS and MAC parameters length */
#define CLASSIFIER_RULES_SET_LEN 128					/*!< clasddifier rules set length */
#define REJECTING_STA_MAC_ADDRS_LEN ((10) * (MAC_LEN))	/*!< rejecting sta mac address length */
#define HD_DURATION_MAX_SECONDS 30U						/*!< hd duration max allowed seconds */
#define RESET_DEVICE_REQ_ID 0							/*!< reset device command req id is always 0 */
#define HLB_MIN_AMPLITUDE_DATA 0x0
#define HLB_MAX_AMPLITUDE_DATA 0xF
#define CE2_DATA_LEN 365
#define LNOE_DATA_LEN 256
#define SNRE_DATA_LEN 512U
#define HLB_MIN_CE2_CHUNK_NUM 0
#define HLB_MAX_CE2_CHUNK_NUM 5U
#define HLB_READ_MEM_MAX_LEN 1452U					/*!< Read memory max length (For legacy reasons) */
#define HLB_WRITE_MEM_MAX_LEN 1452					/*!< Write memory max length (For legacy reasons) */
/********************************************************************
* EXPORTED TYPES
********************************************************************/

/*! result values */
typedef enum
{
	HLB_HPGP_RESULT_SUCCESS = 0,	/*!< result success */
	HLB_HPGP_RESULT_FAILURE = 1,	/*!< result failure */
} HLB_hpgp_result_t;

/*! Reset mode of the CG5317 modem */
typedef enum
{
	/*! Reset and boot normally. */
	HLB_HPGP_RESET_DEVICE_MODE_NORMAL = 0,
	/*! Reset and stop in host boot (Relevant in case of CG loading from Flash).
	 * <br>When CG5317 is configured to load from host loading, this option is identical to normal reset. */
	HLB_HPGP_RESET_DEVICE_MODE_STOP_IN_HOSTBOOT = 1,
} HLB_hpgp_reset_device_mode_t;

/**
Secure (HS) and Simple
Connect (SC). In all, it is assumed that STAs within the same
AVLN are trustworthy (i.e., they do not perform hostile
actions or divulge keys deliberately). */
typedef enum
{
	HLB_HPGP_SECURITY_LEVEL_SC = 0,	/*!< Simple connect security level */
	HLB_HPGP_SECURITY_LEVEL_HS = 1,	/*!< Secure Security level */
} HLB_hpgp_security_level_t;

/*! CCo Mode */
typedef enum
{
	HLB_HPGP_CCO_MODE_AUTO = 0,		/*!< Auto CCo. Currently not supported */
	HLB_HPGP_CCO_MODE_NEVER = 1,	/*!< Never a CCo. In this case, the station will never become the CCo of an AVLN */
	HLB_HPGP_CCO_MODE_ALWAYS = 2,	/*!< Always a CCo. In this case, the station will not join other AVLNs as a STA */
} HLB_hpgp_cco_mode_t;

/*!< host interface */
typedef enum
{
	HLB_HPGP_HOST_INTERFACE_SPI = 0,		/*!< Host interface is SPI, ETH interface is disabled*/
	HLB_HPGP_HOST_INTERFACE_ETH = 1,		/*!< Host interface is ETH (R/MII), SPI interface is disabled */
	HLB_HPGP_HOST_INTERFACE_SPI_DBG = 2,	/*!< Host interface is SPI, but ETH interface is still enabled */
} HLB_hpgp_host_interface_t;

/*! Connection mod cause */
typedef enum
{
	HLB_HPGP_CONN_MOD_CAUSE_CCO_INITIATED = 0,	/*!< CCo initiated cause */
	HLB_HPGP_CONN_MOD_CAUSE_HLE_INITIATED = 1,	/*!< HLE initiated caues */
	HLB_HPGP_CONN_MOD_CAUSE_OTHERS = 2,			/*!< Others cause */
} HLB_hpgp_conn_mod_cause_t;

/*! Connection release reson code */
typedef enum
{
	HLB_HPGP_CONN_REL_REASON_NORMAL_RELEASE = 0,			/*!< Normal release */
	HLB_HPGP_CONN_REL_REASON_VIOLATION_OF_CSPEC = 1,		/*!< Release due to violation of CSPEC */
	HLB_HPGP_CONN_REL_REASON_INSUFFICIENT_BANDWIDTH = 2,	/*!< Insufficient Bandwidth */
	HLB_HPGP_CONN_REL_REASON_REQ_BY_AOTHER_STA = 3,			/*!< Requested by another station within the AVLN that is not part of the connection. */
} HLB_hpgp_conn_rel_reason_code_t;

/*! authorize result */
typedef enum
{
	HLB_HPGP_AUTH_RES_AUTH_COMPLETE = 0,	/*!< Authorization Complete */
	HLB_HPGP_AUTH_RES_NO_RESPONSE = 1,		/*!< No Response */
	HLB_HPGP_AUTH_RES_PROTOCOL_ABORTED = 2, /*!< Protocol aborted */
} HLB_hpgp_authorize_result_t;

/*! authorize status */
typedef enum
{
	HLB_HPGP_AUTH_STATUS_AUTH_COMPLETE = 0,		/*!< Authorization Complete */
	HLB_HPGP_AUTH_STATUS_PROTOCOL_ABORTED = 1,	/*!< Protocol aborted */
} HLB_hpgp_authorize_status_t;

/*! security mode */
typedef enum
{
	HLB_HPGP_SECURITY_MODE_SECURE = 0,				/*!< Secure */
	HLB_HPGP_SECURITY_MODE_SIMPLE_CONNECTED = 1,	/*!< Simple-Connect */
	HLB_HPGP_SECURITY_MODE_SC_ADD = 2,				/*!< Simple-Connect-Add */
	HLB_HPGP_SECURITY_MODE_SC_JOIN = 3,				/*!< Simple-Connect-Join */
} HLB_hpgp_security_mode_t;

/*! AVLN networks status */
typedef enum
{
	HLB_HPGP_NETWORKS_STATUS_JOINED = 0,				/*!< AVLN Joined */
	HLB_HPGP_NETWORKS_STATUS_NOT_JOINED_HAVE_NMK = 1,	/*!< AVLN Not Joined – have NMK */
	HLB_HPGP_NETWORKS_STATUS_NOT_JOINED_NO_NMK = 2,		/*!< AVLN Not Joined – no NMK */
	HLB_HPGP_NETWORKS_STATUS_BLACKLISTED = 3			/*!< AVLN Blacklisted */
} HLB_hpgp_networks_status_t;

/*! networks request type */
typedef enum
{
	HLB_HPGP_NETWORKS_REQ_TYPE_JOIN_NOW = 0,		/*!< request to Join Now */
	HLB_HPGP_NETWORKS_REQ_TYPE_LEAVE_NOW = 1,		/*!< request to Leave Now */
	HLB_HPGP_NETWORKS_REQ_TYPE_BLACKLIST = 2,		/*!< request to Blacklist */
	HLB_HPGP_NETWORKS_REQ_TYPE_REHABILITATE = 3,	/*!< request to Rehabilitate */
} HLB_hpgp_networks_req_type_t;

/*! sta capability auto connect */
typedef enum
{
	HLB_HPGP_CAP_AUTO_CONNECT_NOT_SUPPORTED = 0,	/*!< Auto Connect Service not supported */
	HLB_HPGP_CAP_AUTO_CONNECT_SUPPORTED = 1,		/*!< Auto Connect Service supported */
} HLB_hpgp_cap_auto_connect_t;

/*! sta capability smoothing */
typedef enum
{
	HLB_HPGP_CAP_SMOOTHING_NOT_SUPPORTED = 0,	/*!< Smoothing Service not supported */
	HLB_HPGP_CAP_SMOOTHING_SUPPORTED = 1,		/*!< Smoothing Service supported */
} HLB_hpgp_cap_smoothing_t;

/*! sta capability cco capability */
typedef enum
{
	HLB_HPGP_CAP_CCO_NOT_SUPPORT_QOS_AND_TDMA = 0,				/*!< CCo Capable – does not support QoS and TDMA */
	HLB_HPGP_CAP_CCO_SUPPORT_QOS_AND_TDMA_UNCOORDINATED = 1,	/*!< CCo Capable – supports QoS and TDMA but only in Uncoordinated Mode */
	HLB_HPGP_CAP_CCO_SUPPORT_QOS_AND_TDMA_COORDINATED = 2,		/*!< CCo Capable – supports QoS and TDMA in Coordinated Mode */
	HLB_HPGP_CAP_CCO_FUTURE = 3,								/*!< CCo Capable – future CCo capabilities */
} HLB_hpgp_cap_cco_capability_t;

/*! sta capability proxy capable */
typedef enum
{
	HLB_HPGP_CAP_PROXY_CAPABLE_NOT_SUPPORTED = 0,	/*!< not capable of being a Proxy Coordinator */
	HLB_HPGP_CAP_PROXY_CAPABLE_SUPPORTED = 1,		/*!< capable of being a Proxy Coordinator */
} HLB_hpgp_cap_proxy_capable_t;

/*! sta capability backup cco capable */
typedef enum
{
	HLB_HPGP_CAP_BACKUP_CCO_CAP_NOT_SUPPORTED = 0,	/*!< Backup CCo capability not supported */
	HLB_HPGP_CAP_BACKUP_CCO_CAP_SUPPORTED = 1,		/*!< Backup CCo capability supported */
} HLB_hpgp_cap_backup_cco_cap_t;

/*! sta capability soft hand over */
typedef enum
{
	HLB_HPGP_CAP_SOFT_HANDOVER_NOT_SUPPORTED = 0,	/*!< Soft Handover not supported */
	HLB_HPGP_CAP_SOFT_HANDOVER_SUPPORTED = 1,		/*!< Soft Hand Over supported */
} HLB_hpgp_cap_soft_handover_t;

/*! sta capability two symbol frame control */
typedef enum
{
	HLB_HPGP_CAP_TWO_SYMBOL_FC_NOT_SUPPORTED = 0,	/*!< Two Symbol Frame Control not supported */
	HLB_HPGP_CAP_TWO_SYMBOL_FC_SUPPORTED = 1,		/*!< Two Symbol Frame Control supported */
} HLB_hpgp_cap_two_symbol_fc_t;

/*! sta capability to support Enhanced coexistence with HomePlug 1.1 */
typedef enum
{
	HLB_HPGP_CAP_HP_1_1_CAP_NOT_SUPPORTED = 0,	/*!< not capable of supporting HomePlug 1.1 coexistence */
	HLB_HPGP_CAP_HP_1_1_CAP_SUPPORTED = 1,		/*!< capable of supporting HomePlug 1.1 coexistence */
} HLB_hpgp_cap_hp_1_1_cap_t;

/*! sta capability HomePlug 1.0.1 interoperability */
typedef enum
{
	HLB_HPGP_CAP_HP_1_0_INTEROP_NOT_SUPPORTED = 0,	/*!< not capable of interoperating with HomePlug 1.0.1 */
	HLB_HPGP_CAP_HP_1_0_INTEROP_SUPPORTED = 1,		/*!< capable of interoperating with HomePlug 1.0.1 */
} HLB_hpgp_cap_hp_1_0_interop_t;

/*! sta capability regulatory cap */
typedef enum
{
	HLB_HPGP_CAP_REGULATORY_CAP_NORTH_AMERICA_ONLY = 0, /*!< North America only */
} HLB_hpgp_cap_regulatory_cap_t;

/*! sta capability bidirectional bursting */
typedef enum
{
	HLB_HPGP_CAP_BIDIRECTIONAL_BURTS_NOT_SUPPORTED = 0,								/*!< not capable of supporting Bidirectional Bursts */
	HLB_HPGP_CAP_BIDIRECTIONAL_BURTS_SUPPORTED_CFP_WITH_SACK = 1,					/*!< capable of supporting Bidirectional Bursting. Only supports CFP Bidirectional Bursts ending with SACK */
	HLB_HPGP_CAP_BIDIRECTIONAL_BURTS_SUPPORTED_CFP_WITH_SACK_OR_REVERSE_SOF = 2,	/*!< capable of supporting Bidirectional Bursting. Supports CFP Bidirectional Bursts that either end with a SACK or a Reverse SOF */
} HLB_hpgp_cap_bidirectional_burst_t;

/*! link stats request type */
typedef enum
{
	HLB_HPGP_LINK_STATS_REQ_TYPE_RESET_STATS = 0,			/*!< reset statistics for the corresponding Link */
	HLB_HPGP_LINK_STATS_REQ_TYPE_GET_STATS = 1,				/*!< get statistics for the corresponding Link */
	HLB_HPGP_LINK_STATS_REQ_TYPE_GET_AND_RESET_STATS = 2,	/*!<  */
} HLB_hpgp_link_stats_req_type_t;

/*! link stats transmit flag */
typedef enum
{
	HLB_HPGP_LINK_STATS_TRANSMIT_LINK = 0,	/*!< Transmit Link Flag - transmit Link */
	HLB_HPGP_LINK_STATS_RECEIVE_LINK = 1,	/*!< Transmit Link Flag - receive Link */
} HLB_hpgp_link_stats_transmit_flag_t;

/*! link stats management flag */
typedef enum
{
	HLB_HPGP_LINK_STATS_NOT_MGMT_LINK = 0,	/*!< not management Link */
	HLB_HPGP_LINK_STATS_MGMT_LINK = 1,		/*!< management Link */
} HLB_hpgp_link_stats_mgmt_flag_t;

/*! hfid req type */
typedef enum
{
	HLB_HPGP_HFID_REQ_TYPE_GET_MANUF_SET_HFID = 0,	/*!< request to provide the manufacturer-set HFID of the STA */
	HLB_HPGP_HFID_REQ_TYPE_GET_USER_SET_HFID = 1,	/*!< request to provide the user-set HFID of the STA */
	HLB_HPGP_HFID_REQ_TYPE_GET_NETWORK_HFID = 2,	/*!< request to provide the HFID of the Network, whose network identifier is contained in the NID field */
	HLB_HPGP_HFID_REQ_TYPE_SET_USER_SET_HFID = 3,	/*!< request to set the user-set HFID of the STA to the value indicated in the HFID field */
	HLB_HPGP_HFID_REQ_TYPE_SET_NETWORK_HFID = 4,	/*!< request to set the HFID of the Network, whose network identifier is contained in the NID field, to the value indicated in the HFID field. */
	HLB_HPGP_HFID_REQ_TYPE_GET_FAILURE = 0xFF,		/*!< request failure */
} HLB_hpgp_hfid_req_type_t;

/*! network info sta role */
typedef enum
{
	HLB_HPGP_NWINFO_STA_ROLE_STA = 0,				/*!< Role of the station in the AVLN - STA */
	HLB_HPGP_NWINFO_STA_ROLE_PROXY_COORDINATOR = 1,	/*!< Role of the station in the AVLN - Proxy Coordinator */
	HLB_HPGP_NWINFO_STA_ROLE_CCO = 2,				/*!< Role of the station in the AVLN - CCo */
} HLB_hpgp_nwinfo_sta_role_t;

/*! network info access */
typedef enum
{
	HLB_HPGP_NWINFO_ACCESS_NETWORK_IN_HOME = 0, /*!< This NID corresponds to an in-home network */
	HLB_HPGP_NWINFO_ACCESS_NETWORK_ACCESS = 1,	/*!< This NID corresponds to an Access Network */
} HLB_hpgp_nwinfo_access_t;

/*! slac configuration */
typedef enum
{
	HLB_HPGP_SLAC_CONF_DISABLE_SLAC = 0,									/*!< Disable SLAC: The Green PHY station will not transmit or receive any SLAC related messages */
	HLB_HPGP_SLAC_CONF_ENABLE_SLAC_TRANSMITTER_AND_DISABLE_RECEIVER = 1,	/*!< Enable SLAC functions associated with the transmitter of M-Sounds and to disable SLAC functions associated with receiver of M-Sounds */
	HLB_HPGP_SLAC_CONF_ENABLE_SLAC_RECEIVER_AND_DISABLE_TRANSMITTER = 2,	/*!< Enable SLAC function associated with the receiver of M-Sounds and to disable SLAC functions associated with transmitter of M-Sounds */
} HLB_hpgp_slac_conf_t;

/*! device state */
typedef enum
{
	HLB_HPGP_DEVICE_STATE_UNASSOCIATED = 1,		/*!< Device state unassociated */
	HLB_HPGP_DEVICE_STATE_WAIT_STA_SYNC = 2,	/*!< Device state waiting for sta sync */
	HLB_HPGP_DEVICE_STATE_ASSOCIATING = 3,		/*!< Device state associating */
	HLB_HPGP_DEVICE_STATE_AUTHENTICATING = 4,	/*!< Device state authenticating */
	HLB_HPGP_DEVICE_STATE_OPERATIONAL_STA = 5,	/*!< Device state operational Sta */
	HLB_HPGP_DEVICE_STATE_OPERATIONAL_CCO = 6,	/*!< Device state operational CCo */
} HLB_hpgb_device_state_t;

/*! link state */
typedef enum
{
	HLB_HPGP_LINK_STATE_NO_LINK = 0,	/*!< No link */
	HLB_HPGP_LINK_STATE_LINK = 1,		/*!< Link active */
} HLB_hpgb_d_link_state_t;

/*! AVLN status */
typedef enum
{
	HLB_HPGP_HOST_MESSAGE_STATUS_READY_TO_JOIN = 1,				/*!< Ready to join AVLN */
	HLB_HPGP_HOST_MESSAGE_STATUS_JOINED_AVLN = 2,				/*!< AVLN joined */
	HLB_HPGP_HOST_MESSAGE_STATUS_DISCONNECTED_FROM_AVLN = 3,	/*!< Disconnected from AVLN */
} HLB_hpgb_host_message_status_t;

/*! D-link ready status */
typedef enum
{
	HLB_HPGP_D_LINK_NO_LINK = 0,			/*!< No link */
	HLB_HPGP_D_LINK_LINK_ESTABLISHED = 1,	/*!< Link established */
} HLB_hpgp_d_link_ready_status_t;

/*! PLC Frequency Selection */
typedef enum
{
	HLB_HPGP_PLC_FREQ_SELECTION_50_HZ = 0,				/*!< PLC Frequency Selection - 50Hz */
	HLB_HPGP_PLC_FREQ_SELECTION_60_HZ = 1,				/*!< PLC Frequency Selection - 60Hz */
	HLB_HPGP_PLC_FREQ_SELECTION_EXTERNAL_SIGNAL = 2,	/*!< PLC Frequency Selection - External Signal */
} HLB_hpgp_plc_freq_sel_t;

/*! Structure to hold the NMK (Network Membership Key) */
typedef struct
{
	uint8_t NMK[HPAVKEY_NMK_LEN]; /*!< Network Membership Key */
} HLB_hpgp_nmk_t;

/*! Structure to hold the NID (Network Identifier) */
typedef struct
{
	uint8_t NID[HPAVKEY_NID_LEN]; /*!< Network Identifier */
} HLB_hpgp_nid_t;

/*! Structure to hold the HFID (Human Friendly Identifier) */
typedef struct
{
	uint8_t HFID[HPAVKEY_HFID_LEN + 1]; /*!< Human Friendly Identifier */
} HLB_hpgp_hfid_t;

/*! Structure to hold the CINFO (Connection Information), part of CSPEC */
typedef struct
{
	uint8_t valid_cinfo;				/*!< Valid CINFO */
	uint8_t mac_service_type;			/*!< MAC Service Type */
	uint8_t user_priority;				/*!< User Priority */
	uint8_t arrival_timestamp_to_hle;	/*!< Arrival TimeStamp to HLE (ATS) */
	uint8_t smoothing;					/*!< Smoothing */
} HLB_hpgp_cinfo_t;

/*! Structure to hold the QoS, and MAC parameter fields (part of CSPEC) */
typedef struct
{
	uint8_t forward_reserve;	/*!< Forward/Reverse */
	uint8_t len;				/*!< Length of the Body Field, in Octets */
	uint8_t field_id;			/*!< Identifier of the QoS and MAC parameter field */
	uint8_t body[QMP_BODY_LEN];	/*!< Data of the QoS and MAC parameter field */
} HLB_hpgp_qmp_t;

/*! Structure to hold the CSPEC (Connection Specification) */
typedef struct
{
	uint16_t cspec_len;				/*!< Length of CSPEC, including the 2-octet CSPEC_LEN field */
	HLB_hpgp_cinfo_t cinfo_forward;	/*!< Forward Connection Information */
	HLB_hpgp_cinfo_t cinfo_reverse;	/*!< Reverse Connection Information */
	HLB_hpgp_qmp_t qmp_forward;		/*!< Forward QoS and MAC Parameters, Only present if Connection requires a Forward Link */
	HLB_hpgp_qmp_t qmp_reverse;		/*!< Reverse QoS and MAC Parameters, Only present if Connection requires a Reverse Link */
} HLB_hpgp_cspec_t;

/*! Structure to hold the nwinfo */
typedef struct
{
	HLB_hpgp_nid_t nid;						/*!< Network Identifier */
	uint8_t short_nid;						/*!< Short Network Identifier */
	uint8_t terminal_equipment_id;			/*!< Terminal Equipment Identifier of the STA in the AVLN */
	HLB_hpgp_nwinfo_sta_role_t sta_role;	/*!< Role of the station in the AVLN */
	mac_address_t cco_mac_addr;				/*!< MAC Address of the CCo of the network. */
	HLB_hpgp_nwinfo_access_t access;		/*!< Access Network */
	uint8_t num_cord_neighbor_networks;		/*!< Number of Neighbor Networks that are coordinating with the AVLN */
} HLB_hpgp_nwinfo_t;

/*! Structure to hold the beacon entry */
typedef struct
{
	uint8_t header;									/*!< beacon entry header */
	uint8_t len;									/*!< beacon entry length */
	uint8_t payload[MAX_BEACON_ENTRY_PAYLOAD_LEN];	/*!< beacon entry payload */
} HLB_hpgp_beacon_entry_t;

/*! Structure to hold the BMI (Beacon Management Information) */
typedef struct
{
	uint8_t num_of_beacon_entries;								/*!< number of Beacon Management Information entries */
	HLB_hpgp_beacon_entry_t beacon_entries[MAX_BEACON_ENTRIES]; /*!< the Beacon Management Information entries */
} HLB_hpgp_beacon_mgmt_info_t;

/*! Structure to hold single network configuration */
typedef struct
{
	HLB_hpgp_nid_t nid;					/*!< NID of AVLN */
	HLB_hpgp_networks_status_t status;	/*!< Status of AVLN */
	mac_address_t mac_addr;				/*!< MAC address of AVLN CCo */
	HLB_hpgp_hfid_t hfid;				/*!< HFID of AVLN */
} HLB_hpgp_network_cnf_t;

/*! Structure to hold single station id */
typedef struct
{
	mac_address_t mmac;				/*!< MAC address of newSTA */
	HLB_hpgp_hfid_t manuf_set_hfid;	/*!< Manufacturer-set HFID of newSTA */
	HLB_hpgp_hfid_t user_set_hfid;	/*!< User-set HFID of newSTA */
} HLB_hpgp_station_t;

/*! Structure to hold link stats transmition */
typedef struct
{
	uint16_t beacon_period_cnt;			/*!< Counter indicating the number of Beacon Periods over which Link statistics are collected */
	uint32_t MSDUs;						/*!< Number of MSDUs Received from HLE */
	uint32_t octets;					/*!< Number of Octets of MSDU Payload Received from HLE */
	uint32_t segments_generated;		/*!< Number of Segments That were Generated */
	uint32_t segments_delivered;		/*!< Number of Segments That were successfully delivered. */
	uint32_t segments_dropped;			/*!< Number of Segments that were Dropped */
	uint32_t PBs_handed;				/*!< Number of PBs Handed Over to the PHY for Transmission */
	uint32_t MPDUs_transmitted;			/*!< Number of MPDUs That were Transmitted */
	uint32_t MPDUs_successfully_acked;	/*!< Number of MPDUs that were successfully acknowledged (i.e., SACK with MFSRsp set to ACK). */
} HLB_hpgp_link_stats_transmit_t;

/*! Structure to hold link stats receive */
typedef struct
{
	uint16_t beacon_period_cnt;				/*!< Counter indicating the number of Beacon Periods over which Link statistics are collected */
	uint32_t MSDUs;							/*!< Number of MSDUs Successfully Received */
	uint32_t octets;						/*!< Number of Octets of MSDU Payload Successfully Received */
	uint32_t segments_received;				/*!< Number of Segments that were successfully received. */
	uint32_t segments_missed;				/*!< Number of Segments that were missed. */
	uint32_t PBs_handed;					/*!< Number of PBs that were handed over from the PHY to the MAC. */
	uint32_t MPDUs_received;				/*!< Number of MPDUs That were Received. */
	uint32_t failed_icv_received_frames;	/*!< Number of Received MAC Frame for which ICV Failed */
} HLB_hpgp_link_stats_receive_t;

/*******************************************************************
* start of messages protocol
********************************************************************/
/*! Structure to hold set_key_req */
typedef struct
{
	HLB_hpgp_nmk_t nmk;							/*!< New Network Membership Key */
	HLB_hpgp_nid_t nid;							/*!< Network Identifier (including Security Level) to associate with this NMK, or indicate to use the default NID (all 0xFF bytes) */
	HLB_hpgp_security_level_t security_level;	/*!< Security Level of New NMK (values = HS or SC) - Only relevant if default NID is used */
} HLB_hpgp_set_key_req_t;

/*! Structure to hold get_key_req */
typedef struct
{
	HLB_hpgp_nid_t nid;	/*!< Network Identifier associated with this NMK (including the Security Level) */
	HLB_hpgp_nmk_t nmk;	/*!< Network Membership Key */
} HLB_hpgp_get_key_cnf_t;

/*! Structure to hold get_security_mode_cnf */
typedef struct
{
	HLB_hpgp_result_t result;				/*!< Success | Fail */
	HLB_hpgp_security_mode_t security_mode;	/*!< Secure | Simple-Connect | SC-Add | SC-Join */
} HLB_hpgp_get_security_mode_cnf_t;

/*! Structure to hold get_networks_cnf */
typedef struct
{
	uint8_t num_of_networks;								/*!< Number of Networks Found */
	HLB_hpgp_network_cnf_t networks[MAX_NUM_OF_NETWORKS];	/*!< Networks Found list */
} HLB_hpgp_get_networks_cnf_t;

/*! Structure to hold set_networks_req */
typedef struct
{
	HLB_hpgp_nid_t nid;						/*!< Network ID */
	HLB_hpgp_networks_req_type_t req_type;	/*!< Join Now | Leave Now | Blacklist | Rehabilitate */
} HLB_hpgp_set_networks_req_t;

/*! Structure to hold get_new_sta_cnf */
typedef struct
{
	uint8_t num_of_new_sta;								/*!< Number of newSTAs */
	HLB_hpgp_station_t stations[MAX_NUM_OF_NEW_STA];	/*!< newSTAs list */
} HLB_hpgp_get_new_sta_cnf_t;

/*! Structure to hold get_new_sta_ind */
typedef struct
{
	uint8_t num_of_new_sta;								/*!< Number of newSTAs */
	HLB_hpgp_station_t stations[MAX_NUM_OF_NEW_STA];	/*!< newSTAs list */
} HLB_hpgp_get_new_sta_ind_t;

/*! Structure to hold sta_cap_cnf */
typedef struct
{
	uint8_t av_version;										/*!< HomePlug AV Version. AV 1.1 and Green PHY stations shall set this field to 0x00. Other values are reserved. */
	mac_address_t mac_addr;									/*!< MAC Address */
	uint8_t oui[OUI_LEN];									/*!< Organizationally Unique Identifier */
	HLB_hpgp_cap_auto_connect_t auto_connect;				/*!< Auto Connect Capability */
	HLB_hpgp_cap_smoothing_t smoothing;						/*!< Smoothing Capability */
	HLB_hpgp_cap_cco_capability_t cco_cap;					/*!< CCo Capability */
	HLB_hpgp_cap_proxy_capable_t proxy_capable;				/*!< Proxy Capability */
	HLB_hpgp_cap_backup_cco_cap_t backup_cco_capable;		/*!< Backup CCo-capable */
	HLB_hpgp_cap_soft_handover_t soft_handover;				/*!< Soft Hand Over Support */
	HLB_hpgp_cap_two_symbol_fc_t two_sym_frame_control;		/*!< Two Symbol Frame Control */
	uint16_t max_fl_av;										/*!< Maximum value of FL_AV that the station is capable of supporting in multiples of 1.28 usec. */
	HLB_hpgp_cap_hp_1_1_cap_t home_plug_1_1_cap;			/*!< Ability to Support Enhanced Coexistence with HomePlug 1.1 */
	HLB_hpgp_cap_hp_1_0_interop_t home_plug_1_0_interop;	/*!< HomePlug 1.0.1 Interoperability */
	HLB_hpgp_cap_regulatory_cap_t regulatory_cap;			/*!< Capability of Operating in Various Regulatory Domains */
	HLB_hpgp_cap_bidirectional_burst_t bidirectional_burst;	/*!< Bidirectional Bursting Capability */
	uint16_t implementation_ver;							/*!< Implementation Version */
} HLB_hpgp_sta_cap_cnf_t;

/*! Structure to hold nw_info_cnf */
typedef struct
{
	uint8_t num_of_nw_info;							/*!< Number of AVLNs that the STA is a member i.e., Associated and Authenticated */
	HLB_hpgp_nwinfo_t nwinfo[MAX_NUM_OF_NETWORKS];	/*!< Network Information of the AVLN's */
} HLB_hpgp_nw_info_cnf_t;

/*! Structure to hold link_stats_req */
typedef struct
{
	HLB_hpgp_link_stats_req_type_t req_type;				/*!< Request Type */
	HLB_hpgp_nid_t nid;										/*!< Network Identifier of the STA(s) whose Connection statistics are being requested. */
	HLB_hpgp_link_stats_transmit_flag_t transmit_link_flag;	/*!< Transmit Link Flag */
	mac_address_t da_sa_mac_addr;							/*!< Indicate the Destination MAC Address when transmit_link_flag is set to 0x00, or Source Mac Address when TLFlag is set to 0x01. */
} HLB_hpgp_link_stats_req_t;

/*! Structure to hold link_stats_cnf */
typedef struct
{
	HLB_hpgp_result_t res_type;								/*!< Response Type */
	HLB_hpgp_link_stats_transmit_flag_t transmit_link_flag;	/*!< Transmit Link Flag */
	HLB_hpgp_link_stats_transmit_t transmit_link;			/*!< Transmit Link (if transmit_link_flag is 0x00) */
	HLB_hpgp_link_stats_receive_t receive_link;				/*!< Receive Link (if transmit_link_flag is 0x01) */
} HLB_hpgp_link_stats_cnf_t;

/*! Structure to hold get_beacon_cnf */
typedef struct
{
	HLB_hpgp_nid_t nid;								/*!< Network Identifier */
	uint8_t hybrid_mode;							/*!< Hybrid Mode */
	uint8_t source_terminal_equipment_id;			/*!< Source Terminal Equipment Identifier */
	uint8_t beacon_type;							/*!< Beacon Type */
	uint8_t non_coord_networks_reported;			/*!< Non-Coordinating Networks Reported */
	uint8_t network_power_save_mode;				/*!< Network Power Saving Mode */
	uint8_t num_of_beacon_slots;					/*!< Number of Beacon Slots */
	uint8_t beacon_slot_usage;						/*!< Beacon Slot Usage */
	uint8_t beacon_slot_id;							/*!< Beacon Slot ID */
	uint8_t ac_line_cycle_sync_status;				/*!< AC Line Cycle Synchronization Status */
	uint8_t handover_in_progress;					/*!< Handover-In-Progress */
	uint8_t rts_broadcast_flag;						/*!< RTS Broadcast Flag */
	uint8_t network_mode;							/*!< Network Mode */
	HLB_hpgp_cap_cco_capability_t cco_cap;			/*!< CCo Capability */
	HLB_hpgp_beacon_mgmt_info_t beacon_mgmt_info;	/*!< Beacon Management Information */
	uint8_t reusable_SNID_flag;						/*!< Reusable SNID Flag */
	uint8_t proxy_level;							/*!< Proxy Level */
} HLB_apcm_get_beacon_cnf_t;

/*! Structure to hold get_hfid_req
	If request type is 0 or 1, only req_type will be regarded
	If request type is 2, only nid will be regarded
	If request type is 3, only hfid will be regarded
	If request type is 4, both nid and hfid will be regarded */
typedef struct
{
	HLB_hpgp_hfid_req_type_t req_type;	/*!< Request Type */
	HLB_hpgp_nid_t nid;					/*!< Network Identifier */
	HLB_hpgp_hfid_t hfid;				/*!< Human Friendly Identifier of the STA or AVLN */
} HLB_hpgp_get_hfid_req_t;

/*! Structure to hold get_hfid_cnf */
typedef struct
{
	HLB_hpgp_hfid_req_type_t req_type;	/*!< Response Type */
	HLB_hpgp_hfid_t hfid;				/*!< Human Friendly Identifier of the STA or AVLN */
} HLB_hpgp_get_hfid_cnf_t;

/*! Structure to hold unassociated_sta_ind */
typedef struct
{
	HLB_hpgp_nid_t nid;						/*!< Network Identifier */
	HLB_hpgp_cap_cco_capability_t cco_cap;	/*!< CCo Capability */
} HLB_hpgp_unassociated_sta_ind_t;

/*! Structure to hold set_ppkeys_req */
typedef struct
{
	uint8_t pp_eks[ENCRYPTION_KEY_LEN];	/*!< Encryption Key Select for Point to Point encryption key select */
	uint8_t ppek[ENCRYPTION_KEY_LEN];	/*!< Point-to-Point Encryption Key */
	mac_address_t other_mac_addr;		/*!< MAC address of other STA  */
} HLB_hpgp_set_ppkeys_req_t;

/* firmware version main.minor.sub.build (patch) */
typedef struct
{
	uint16_t fw_version_major;		   /*!< firmware version major (1st) */
	uint16_t fw_version_minor;		   /*!< firmware version minor (2nd) */
	uint16_t fw_version_sub;		   /*!< firmware version sub version (3rd) */
	uint16_t fw_version_build;		   /*!< firmware version build number (4th) */
	uint16_t fw_version_patch_version; /*!< patch version*/
} HLB_fw_version_t;

typedef struct
{
	uint16_t version_major; /*!< host lib version major (1st) */
	uint16_t version_minor; /*!< host lib version minor (2nd) */
	uint16_t version_patch; /*!< host lib version patch (3rd) */
	uint16_t version_build; /*!< host lib version build (4th) */
} version_t;

/*! Structure contains the size and  payload_data of fw */
typedef struct
{
	uint32_t buffer_size; /*!< Buffer size */
	uint8_t *buffer;      /*!< A pointer to the buffer */
} HLB_fw_bin_buffer_t;

/*! Structure containing information about the ce2 of the device*/
typedef struct
{
	HLB_hpgp_result_t status;	/*!< 0-success, 1-general failure */
	int8_t gain; 				/*!< current gain */
	int16_t ce2_shift;			/*!< ce2 shift */
	uint32_t total_buffer_size;	/*!< The size of the whole CE2 buffer */
} HLB_ce2_info_t;

/*! Structure containing data about the ce2 of the device*/
typedef struct
{
	uint32_t chunk_number;	/*!< from 0-5, must come in order, 5 will cause fw to free buffer after sending */
} HLB_ce2_data_req_t;

/*! Structure containing data about the ce2 of the device*/
typedef struct
{
	HLB_hpgp_result_t status;	/*!<  0-success, 1-general failure */
	uint16_t size; 				/*!< size of buffer in words(32bit) if zero something has failed */
	int32_t data[CE2_DATA_LEN];	/*!< buffer, up to 365 words (of 32bit) */
} HLB_ce2_data_cnf_t;

/*! Structure containing information about the lnoe of the device*/
typedef struct
{
	HLB_hpgp_result_t status;	/*!<  0-success, 1-general failure */
	int8_t gain; 				/*!< current gain */
	int8_t lnoe[LNOE_DATA_LEN];	/*!< data buffer */
} HLB_lnoe_info_t;

/*! Structure containing information about the snre of the device*/
typedef struct
{
	HLB_hpgp_result_t status;		/*!< 0-success, 1-general failure */
	int16_t snre[SNRE_DATA_LEN];	/*!< data buffer */
} HLB_snre_info_t;

/*! Structure containing information about the read memory request */
typedef struct
{
	uint32_t address; 		/*!< The address to read from */
	uint32_t size; 			/*!< The amount to read */
} HLB_read_mem_req_t;

/*! Structure containing data from the devices memory */
typedef struct
{
	uint32_t size; 						/*!< size of buffer in bytes(8bit) if zero something has failed */
	uint8_t data[HLB_READ_MEM_MAX_LEN];	/*!< memory buffer */
} HLB_read_mem_cnf_t;

/*! Structure containing information about the write memory request */
typedef struct
{
	uint32_t address; 						/*!< The address to write to */
	uint32_t size; 							/*!< The amount to write */
	uint8_t data[HLB_WRITE_MEM_MAX_LEN]; 	/*!< The data to write */
} HLB_write_mem_req_t;

/*! Structure containing information about the modem dc callibration values */
typedef struct
{
	uint8_t dc_offset_hi_ch1;	/*!< DC offset high */
    uint8_t dc_offset_lo_ch1;	/*!< DC offset low */
} HLB_dc_calib_cnf_t;

/*! Structure containing information about the modem device state values */
typedef struct
{
    HLB_hpgb_device_state_t device_status;	/*!< Device status */
} HLB_hpgp_device_state_cnf_t;

/*! Structure containing information about the links status */
typedef struct
{
    HLB_hpgb_d_link_state_t link_status;	/*!< Link status */
} HLB_hpgp_d_link_status_cnf_t;

/*! Structure of AVLN status */
typedef struct
{
    /*! 1 - Ready to join AVLN
		2 - Joined AVLN
        3 - Disconnected from AVLN */
	HLB_hpgb_host_message_status_t host_message_status;
} HLB_hpgp_host_message_status_ind_t;

/*! Structure of D link ready indication */
typedef struct
{
	HLB_hpgp_d_link_ready_status_t d_link_ready_status;	/*!< D-link ready status */
} HLB_hpgp_d_link_ready_status_ind_t;

/*! Structure containing device info */
typedef struct
{
	HLB_hpgp_cco_mode_t cco_mode;				/*!< Cco mode */
	HLB_hpgp_host_interface_t host_iface;		/*!< Host interface */
	uint8_t terminal_equipment_id;				/*!< Terminal Equipment Identifier */
	mac_address_t mac_addr;						/*!< MAC address of AVLN */
	HLB_hpgp_nmk_t nmk;							/*!< Network Membership Key */
	HLB_hpgp_nid_t nid;							/*!< Network Identifier */
	HLB_hpgp_security_level_t security_level;	/*!< Security Level (values = HS or SC) - Only relevant if default NID is used */
	/*! Short Network Identifier
		<br>The least-significant 4 bits of this field contains the Short Network Identifier.
		<br>The remaining 4 bits are set to 0x0. */
	uint8_t snid;
	/*! Maximal Receiver Sensitivity configuration.
	 * <br>The receiver sensitivity can be controlled by setting a limitation on the maximum gain of the AGC.
	 * <br>Values range between a minimum of 3dB and a maximum of 43dB. */
	uint8_t max_receiver_sensitivity;
	HLB_hpgp_plc_freq_sel_t plc_freq_sel;		/*!< PLC Frequency Selection */
	HLB_hpgp_hfid_t manufacturer_hfid;			/*!< Manufacturer HFID (Human Friendly Identifier) */
	HLB_hpgp_hfid_t user_hfid;					/*!< User HFID (Human Friendly Identifier) */
	HLB_hpgp_hfid_t avln_hfid;					/*!< AVLN HFID (Human Friendly Identifier) */
} HLB_device_info_t;

/*! Structure containing amplitude map */
typedef struct
{
	uint8_t amdata[AMDATA_LEN * 4];	/*!< Amplitude map data */
} HLB_get_amp_map_t;


/********************************************************************
* EXPORTED MACROS
********************************************************************/
/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/

/**
 *   Function Name: HLB_is_control_path_message
 *
 * @brief Description: this function checkes weather a msg
 * is a control path message we support
 *
 *
 *  @param  input : msg - can be every msg
 *  @return status 0 - NOT a control path message, 1 - control path message
 *
 *
 */
int HLB_is_control_path_message(const void *msg);

/**
 *   Function Name: HLB_get_message_id
 *
 * @brief Description: this function returns the unique message id
 * from the eth message
 *
 *
 *  @param  input : msg - can be every msg
 *  @return status 0 - NOT a control path message, 1 - control path message
 *
 *
 */
HLB_msg_id_t HLB_get_message_id(const void *msg);

/**
 *   Function Name: HLB_apcm_set_key_req_create
 *
 * @brief Description: this function creates the eth msg of
 * set_key_req from the hpgp set_key_req api struct
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : set_key_req - The hpgp set_key_req api struct
 *  @param  input : req_id
 *  @param  output : msg - The set_key_req eth msg
 *  @param  input, output : msg_len - The set_key_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_set_key_req_create(const uint8_t *dest_mac_addr,
										 const uint8_t *src_mac_addr,
										 const HLB_hpgp_set_key_req_t *set_key_req, HLB_req_id_t req_id,
										 void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_apcm_set_key_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * set_key_cnf to the hpgp result
 *
 *
 *  @param  input : msg - The set_key_cnf eth msg
 *  @param  output : result - The result of the set_key_req
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_set_key_cnf_parse(const void *msg,
										HLB_hpgp_result_t *result);

/**
 *   Function Name: HLB_apcm_get_key_req_create
 *
 * @brief Description: this function creates the eth msg of
 * get_key_req
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id - Request id
 *  @param  output : msg - The get_key_req eth msg
 *  @param  input, output : msg_len - The get_key_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_get_key_req_create(const uint8_t *dest_mac_addr,
										 const uint8_t *src_mac_addr, HLB_req_id_t req_id,
										 void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_apcm_get_key_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * get_key_cnf to the hpgp get_key_cnf api struct
 *
 *
 *  @param  input : msg - The get_key_cnf eth msg
 *  @param  output : get_key_cnf - The hpgp get_key_cnf api struct
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_get_key_cnf_parse(const void *msg,
										HLB_hpgp_get_key_cnf_t *get_key_cnf);

/**
 *   Function Name: HLB_apcm_set_cco_req_create
 *
 * @brief Description: this function creates the eth msg of
 * set_cco_req from the hpgp cco mode
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  input : cco_mode - The cco mode
 *  @param  output : msg - The set_cco_req eth msg
 *  @param  input, output : msg_len - The set_cco_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_set_cco_req_create(const uint8_t *dest_mac_addr,
										 const uint8_t *src_mac_addr,
										 HLB_req_id_t req_id, HLB_hpgp_cco_mode_t cco_mode,
										 void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_apcm_set_cco_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * set_cco_cnf to the hpgp result
 *
 *
 *  @param  input : msg - The set_cco_cnf eth msg
 *  @param  output : result
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_set_cco_cnf_parse(const void *msg,
										HLB_hpgp_result_t *result);

/**
 *   Function Name: HLB_apcm_get_ntb_req_create
 *
 * @brief Description: this function creates the eth msg of
 * get_ntb_req
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id - Request id
 *  @param  output : msg - The get_ntb_req eth msg
 *  @param  input, output : msg_len - The get_ntb_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_get_ntb_req_create(const uint8_t *dest_mac_addr,
										 const uint8_t *src_mac_addr, HLB_req_id_t req_id,
										 void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_apcm_get_ntb_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * set_cco_cnf to the hpgp result
 *
 *
 *  @param  input  : msg - The set_cco_cnf eth msg
 *  @param  output : ntb: The Network Time Base
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_get_ntb_cnf_parse(const void *msg,
										uint32_t *ntb);

/**
 *   Function Name: HLB_apcm_get_security_mode_req_create
 *
 * @brief Description: this function creates the eth msg of
 * get_security_mode_req
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id - Request id
 *  @param  output : msg - The get_security_mode_req eth msg
 *  @param  input, output : msg_len - The get_security_mode_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_get_security_mode_req_create(const uint8_t *dest_mac_addr,
												   const uint8_t *src_mac_addr, HLB_req_id_t req_id,
												   void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_apcm_get_security_mode_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * get_security_mode_cnf to the hpgp get_security_mode_cnf api struct
 *
 *
 *  @param  input : msg - The get_security_mode_cnf eth msg
 *  @param  output : security_mode_cnf - The hpgp get_security_mode_cnf api struct
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_get_security_mode_cnf_parse(const void *msg,
												  HLB_hpgp_get_security_mode_cnf_t *security_mode_cnf);

/**
 *   Function Name: HLB_apcm_set_security_mode_req_create
 *
 * @brief Description: this function creates the eth msg of
 * set_security_mode_req from the hpgp security mode
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  input : security_mode - The security_mode
 *  @param  output : msg - The set_security_mode_req eth msg
 *  @param  input, output : msg_len - The set_security_mode_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_set_security_mode_req_create(const uint8_t *dest_mac_addr,
												   const uint8_t *src_mac_addr,
												   HLB_req_id_t req_id, HLB_hpgp_security_mode_t security_mode,
												   void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_apcm_set_security_mode_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * set_security_mode_cnf to the hpgp result
 *
 *
 *  @param  input : msg - The set_security_mode_cnf eth msg
 *  @param  output : result
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_set_security_mode_cnf_parse(const void *msg,
												  HLB_hpgp_result_t *result);

/**
 *   Function Name: HLB_apcm_get_networks_req_create
 *
 * @brief Description: this function creates the eth msg of
 * get_networks_req
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id - Request id
 *  @param  output : msg - The get_networks_req eth msg
 *  @param  input, output : msg_len - The get_networks_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_get_networks_req_create(const uint8_t *dest_mac_addr,
											  const uint8_t *src_mac_addr, HLB_req_id_t req_id,
											  void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_apcm_get_networks_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * get_networks_cnf to the hpgp get_networks_cnf api struct
 *
 *
 *  @param  input : msg - The get_networks_cnf eth msg
 *  @param  output : networks_cnf - The hpgp get_networks_cnf api struct
 *  @return RES_result_t : result status code
 *
 *
 */

RES_result_t HLB_apcm_get_networks_cnf_parse(const void *msg,
											 HLB_hpgp_get_networks_cnf_t *networks_cnf);

/**
 *   Function Name: HLB_apcm_set_networks_req_create
 *
 * @brief Description: this function creates the eth msg of
 * set_networks_req from the hpgp set_networks_req api struct
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  input : networks_req - The hpgp set_networks_req api struct
 *  @param  output : msg - The set_networks_req eth msg
 *  @param  input, output : msg_len - The set_networks_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */

RES_result_t HLB_apcm_set_networks_req_create(const uint8_t *dest_mac_addr,
											  const uint8_t *src_mac_addr, HLB_req_id_t req_id,
											  const HLB_hpgp_set_networks_req_t *networks_req,
											  void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_apcm_set_networks_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * set_networks_cnf to the hpgp result
 *
 *
 *  @param  input : msg - The set_networks_cnf eth msg
 *  @param  output : result
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_set_networks_cnf_parse(const void *msg,
											 HLB_hpgp_result_t *result);

/**
 *   Function Name: HLB_apcm_get_new_sta_req_create
 *
 * @brief Description: this function creates the eth msg of
 * get_new_sta_req
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id - Request id
 *  @param  output : msg - The get_new_sta_req eth msg
 *  @param  input, output : msg_len - The get_new_sta_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_get_new_sta_req_create(const uint8_t *dest_mac_addr,
											 const uint8_t *src_mac_addr, HLB_req_id_t req_id,
											 void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_apcm_get_new_sta_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * get_new_sta_cnf to the hpgp get_new_sta_cnf api struct
 *
 *
 *  @param  input : msg - The get_new_sta_cnf eth msg
 *  @param  output : new_sta_cnf - The hpgp get_new_sta_cnf api struct
 *  @return RES_result_t : result status code
 *
 *
 */

RES_result_t HLB_apcm_get_new_sta_cnf_parse(const void *msg,
											HLB_hpgp_get_new_sta_cnf_t *new_sta_cnf);

/**
 *   Function Name: HLB_apcm_get_new_sta_ind_parse
 *
 * @brief Description: this function parses the eth msg of
 * get_new_sta_ind to the hpgp get_new_sta_ind api struct
 *
 *
 *  @param  input : msg - The get_new_sta_ind eth msg
 *  @param  output : new_sta_ind - The hpgp get_new_sta_ind api struct
 *  @return RES_result_t : result status code
 *
 *
 */

RES_result_t HLB_apcm_get_new_sta_ind_parse(const void *msg,
											HLB_hpgp_get_new_sta_ind_t *new_sta_ind);

/**
 *   Function Name: HLB_apcm_sta_restart_req_create
 *
 * @brief Description: this function creates the eth msg of
 * sta_restart_req
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id - Request id
 *  @param  output : msg - The sta_restart_req eth msg
 *  @param  input, output : msg_len - The sta_restart_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_sta_restart_req_create(const uint8_t *dest_mac_addr,
											 const uint8_t *src_mac_addr, HLB_req_id_t req_id,
											 void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_apcm_sta_restart_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * sta_restart_cnf to the hpgp result
 *
 *
 *  @param  input : msg - The sta_restart_cnf eth msg
 *  @param  output : result
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_sta_restart_cnf_parse(const void *msg,
											HLB_hpgp_result_t *result);

/**
 *   Function Name: HLB_apcm_net_exit_req_create
 *
 * @brief Description: this function creates the eth msg of
 * net_exit_req
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id - Request id
 *  @param  output : msg - The net_exit_req eth msg
 *  @param  input, output : msg_len - The net_exit_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_net_exit_req_create(const uint8_t *dest_mac_addr,
										  const uint8_t *src_mac_addr, HLB_req_id_t req_id,
										  void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_apcm_net_exit_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * net_exit_cnf to the hpgp result
 *
 *
 *  @param  input : msg - The net_exit_cnf eth msg
 *  @param  output : result
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_net_exit_cnf_parse(const void *msg,
										 HLB_hpgp_result_t *result);

/**
 *   Function Name: HLB_apcm_set_tone_mask_req_create
 *
 * @brief Description: this function creates the eth msg of
 * set_tone_mask_req from the hpgp result
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  input : tone_mask -  This parameter indicates masked carriers from carrier
 * 								number 74 through 1228 (1.8 MHz to 30 MHz)
 *  @param  output : msg - The set_tone_mask_req eth msg
 *  @param  input, output : msg_len - The set_tone_mask_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_set_tone_mask_req_create(const uint8_t *dest_mac_addr,
											   const uint8_t *src_mac_addr, HLB_req_id_t req_id,
											   uint16_t tone_mask,
											   void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_apcm_set_tone_mask_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * set_tone_mask_cnf to the hpgp result
 *
 *
 *  @param  input : msg - The set_tone_mask_cnf eth msg
 *  @param  output : result
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_set_tone_mask_cnf_parse(const void *msg,
											  HLB_hpgp_result_t *result);

/**
 *   Function Name: HLB_apcm_sta_cap_req_create
 *
 * @brief Description: this function creates the eth msg of
 * sta_cap_req
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id - Request id
 *  @param  output : msg - The sta_cap_req eth msg
 *  @param  input, output : msg_len - The sta_cap_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_sta_cap_req_create(const uint8_t *dest_mac_addr,
										 const uint8_t *src_mac_addr, HLB_req_id_t req_id,
										 void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_apcm_sta_cap_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * sta_cap_cnf to the hpgp sta_cap_cnf api struct
 *
 *
 *  @param  input : msg - The sta_cap_cnf eth msg
 *  @param  output : sta_cap_cnf - The hpgp sta_cap_cnf api struct
 *  @return RES_result_t : result status code
 *
 *
 */

RES_result_t HLB_apcm_sta_cap_cnf_parse(const void *msg,
										HLB_hpgp_sta_cap_cnf_t *sta_cap_cnf);

/**
 *   Function Name: HLB_apcm_nw_info_req_create
 *
 * @brief Description: this function creates the eth msg of
 * nw_info_req
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id - Request id
 *  @param  output : msg - The nw_info_req eth msg
 *  @param  input, output : msg_len - The nw_info_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_nw_info_req_create(const uint8_t *dest_mac_addr,
										 const uint8_t *src_mac_addr, HLB_req_id_t req_id,
										 void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_apcm_nw_info_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * nw_info_cnf to the hpgp nw_info_cnf api struct
 *
 *
 *  @param  input : msg - The nw_info_cnf eth msg
 *  @param  output : nw_info_cnf - The hpgp nw_info_cnf api struct
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_nw_info_cnf_parse(const void *msg,
										HLB_hpgp_nw_info_cnf_t *nw_info_cnf);

/**
 *   Function Name: HLB_nscm_link_stats_req_create
 *
 * @brief Description: this function creates the eth msg of
 * link_stats_req from the hpgp link_stats_req api struct
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  input : link_stats_req - The hpgp link_stats_req api struct
 *  @param  output : msg - The link_stats_req eth msg
 *  @param  input, output : msg_len - The link_stats_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_link_stats_req_create(const uint8_t *dest_mac_addr,
											const uint8_t *src_mac_addr, HLB_req_id_t req_id,
											const HLB_hpgp_link_stats_req_t *link_stats_req,
											void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_apcm_link_stats_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * link_stats_cnf to the hpgp link_stats_cnf api struct
 *
 *
 *  @param  input : msg - The link_stats_cnf eth msg
 *  @param  output : link_stats_cnf - The hpgp link_stats_cnf api struct
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_link_stats_cnf_parse(const void *msg,
										   HLB_hpgp_link_stats_cnf_t *link_stats_cnf);

/**
 *   Function Name: HLB_apcm_get_beacon_req_create
 *
 * @brief Description: this function creates the eth msg of
 * get_beacon_req from the hpgp nid struct
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  input : nid - The Network Identifier
 *  @param  output : msg - The get_beacon_req eth msg
 *  @param  input, output : msg_len - The get_beacon_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_get_beacon_req_create(const uint8_t *dest_mac_addr,
											const uint8_t *src_mac_addr, HLB_req_id_t req_id,
											const HLB_hpgp_nid_t *nid,
											void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_apcm_get_beacon_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * get_beacon_cnf to the hpgp get_beacon_cnf api struct
 *
 *
 *  @param  input : msg - The get_beacon_cnf eth msg
 *  @param  output : get_beacon_cnf - The hpgp get_beacon_cnf api struct
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_get_beacon_cnf_parse(const void *msg,
										   HLB_apcm_get_beacon_cnf_t *get_beacon_cnf);

/**
 *   Function Name: HLB_apcm_get_hfid_req_create
 *
 * @brief Description: this function creates the eth msg of
 * get_hfid_req from the hpgp get_hfid_req api struct
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  input : hfid_req - The human friendly id request struct
 *  @param  output : msg - The get_hfid_req eth msg
 *  @param  input, output : msg_len - The get_hfid_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_get_hfid_req_create(const uint8_t *dest_mac_addr,
										  const uint8_t *src_mac_addr, HLB_req_id_t req_id,
										  const HLB_hpgp_get_hfid_req_t *hfid_req,
										  void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_apcm_get_hfid_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * get_hfid_cnf to the hpgp result
 *
 *
 *  @param  input : msg - The get_hfid_cnf eth msg
 *  @param  output : hfid_cnf - The human friendly id cnf struct
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_get_hfid_cnf_parse(const void *msg,
										 HLB_hpgp_get_hfid_cnf_t *hfid_cnf);

/**
 *   Function Name: HLB_apcm_set_hfid_req_create
 *
 * @brief Description: this function creates the eth msg of
 * set_hfid_req from the hpgp set_hfid_req api struct
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  input : hfid - The human friendly id struct
 *  @param  output : msg - The set_hfid_req eth msg
 *  @param  input, output : msg_len - The set_hfid_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_set_hfid_req_create(const uint8_t *dest_mac_addr,
										  const uint8_t *src_mac_addr, HLB_req_id_t req_id,
										  const HLB_hpgp_hfid_t *hfid,
										  void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_apcm_set_hfid_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * set_hfid_cnf to the hpgp result
 *
 *
 *  @param  input : msg - The set_hfid_cnf eth msg
 *  @param  output : result
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_set_hfid_cnf_parse(const void *msg,
										 HLB_hpgp_result_t *result);

/**
 *   Function Name: HLB_apcm_set_hd_duration_req_create
 *
 * @brief Description: this function creates the eth msg of
 * set_hd_duration_req from the hpgp set_hd_duration_req api struct
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  input : hd_duration - The hd duration in seconds (0-30)
 *  @param  output : msg - The set_hd_duration_req eth msg
 *  @param  input, output : msg_len - The set_hd_duration_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_set_hd_duration_req_create(const uint8_t *dest_mac_addr,
												 const uint8_t *src_mac_addr, HLB_req_id_t req_id,
												 uint8_t hd_duration,
												 void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_apcm_set_hd_duration_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * set_hd_duration_req to the hpgp result
 *
 *
 *  @param  input : msg - The set_hd_duration_req eth msg
 *  @param  output : result
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_set_hd_duration_cnf_parse(const void *msg,
												HLB_hpgp_result_t *result);

/**
 *   Function Name: HLB_apcm_unassociated_sta_ind_parse
 *
 * @brief Description: this function parses the eth msg of
 * unassociated_sta_ind to the hpgp result
 *
 *
 *  @param  input : msg - The unassociated_sta_ind eth msg
 *  @param  output : sta_ind
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_unassociated_sta_ind_parse(const void *msg,
												 HLB_hpgp_unassociated_sta_ind_t *sta_ind);

/**
 *   Function Name: HLB_apcm_set_ppkeys_req_create
 *
 * @brief Description: this function creates the eth msg of
 * set_ppkeys_req from the hpgp set_ppkeys_req api struct
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  input : ppkeys_req - The ppkeys_req api struct
 *  @param  output : msg - The set_ppkeys_req eth msg
 *  @param  input, output : msg_len - The set_ppkeys_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_set_ppkeys_req_create(const uint8_t *dest_mac_addr,
											const uint8_t *src_mac_addr, HLB_req_id_t req_id,
											const HLB_hpgp_set_ppkeys_req_t *ppkeys_req,
											void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_apcm_set_ppkeys_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * set_ppkeys_cnf to the hpgp result
 *
 *
 *  @param  input : msg - The set_ppkeys_cnf eth msg
 *  @param  output : result
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_set_ppkeys_cnf_parse(const void *msg,
										   HLB_hpgp_result_t *result);

/**
 *   Function Name: HLB_apcm_conf_slac_req_create
 *
 * @brief Description: this function creates the eth msg of
 * conf_slac_req from the hpgp conf_slac_req api struct
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  input : conf_slac - The slac_conf_req api struct
 *  @param  output : msg - The slac_conf_req eth msg
 *  @param  input, output : msg_len - The slac_conf_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_conf_slac_req_create(const uint8_t *dest_mac_addr,
										   const uint8_t *src_mac_addr, HLB_req_id_t req_id,
										   const HLB_hpgp_slac_conf_t conf_slac,
										   void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_apcm_conf_slac_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * conf_slac_cnf to the hpgp result
 *
 *
 *  @param  input : msg - The conf_slac_cnf eth msg
 *  @param  output : result
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_apcm_conf_slac_cnf_parse(const void *msg,
										  HLB_hpgp_result_t *result);

/**
 *   Function Name: HLB_nscm_get_fw_version_req_create
 *
 * @brief Description: this function creates the eth msg of
 * get_fw_version
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  output : msg - The get_fw_version eth msg
 *  @param  input, output : msg_len - The get_fw_version eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_get_fw_version_req_create(const uint8_t *dest_mac_addr,
											 const uint8_t *src_mac_addr, HLB_req_id_t req_id,
											 void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_nscm_get_fw_version_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * get_fw_version_cnf to the nscm get_fw_version api struct
 *
 *
 *  @param  input : msg - The get_fw_version eth msg
 *  @param  output : device_info - The nscm get_fw_version api struct
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_get_fw_version_cnf_parse(const void *msg,
											HLB_fw_version_t *fw_version);

/**
 *   Function Name: HLB_nscm_reset_device_req_create
 *
 * @brief Description: this function creates the eth msg of
 * reset_device
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : reset_mode - The reset mode of the CG5317
 *  @param  output : msg - The reset_device eth msg
 *  @param  input, output : msg_len - The reset_device eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_reset_device_req_create(const uint8_t *dest_mac_addr,
											  const uint8_t *src_mac_addr,
											  HLB_hpgp_reset_device_mode_t reset_mode,
											  void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_nscm_get_ce2_info_req_create
 *
 * @brief Description: this function creates the eth msg of
 * get_ce2 info
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  output : msg - The get_ce2 eth msg
 *  @param  input, output : msg_len - The get_ce2 eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_get_ce2_info_req_create(const uint8_t *dest_mac_addr,
											 const uint8_t *src_mac_addr,
											 HLB_req_id_t req_id,
											 void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_nscm_get_ce2_info_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * get_ce2_info_cnf to the nscm ce2_info api struct
 *
 *
 *  @param  input : msg - The get_ce2 eth msg
 *  @param  output : ce2_info - The nscm ce2_info api struct
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_get_ce2_info_cnf_parse(const void *msg, HLB_ce2_info_t *ce2_info);			

/**
 *   Function Name: HLB_nscm_get_ce2_data_req_create
 *
 * @brief Description: this function creates the eth msg of
 * get_ce2_data_req
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : data_req - The part of the data we want
 *  @param  input : req_id
 *  @param  output : msg - The get_ce2_data eth msg
 *  @param  input, output : msg_len - The get_ce2_data eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_get_ce2_data_req_create(const uint8_t *dest_mac_addr,
										 const uint8_t *src_mac_addr,
										 const HLB_ce2_data_req_t *data_req,
										 HLB_req_id_t req_id, void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_nscm_get_ce2_data_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * get_ce2_data_cnf
 *
 *
 *  @param  input : msg - The get_ce2_data cnf eth msg
 *  @param  output : ce2_data - The nscm ce2_data_cnf api struct
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_get_ce2_data_cnf_parse(const void *msg, HLB_ce2_data_cnf_t *ce2_data);
/**
 *   Function Name: HLB_nscm_get_lnoe_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * get_lnoe_info_cnf to the nscm lnoe_info api struct
 *
 *
 *  @param  input : msg - The get_lnoe eth msg
 *  @param  output : lnoe_info - The nscm lnoe_info api struct
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_get_lnoe_cnf_parse(const void *msg, HLB_lnoe_info_t *lnoe_info);

/**
 *   Function Name: HLB_nscm_get_lnoe_req_create
 *
 * @brief Description: this function creates the eth msg of
 * get_lnoe
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  output : msg - The get_lnoe eth msg
 *  @param  input, output : msg_len - The get_lnoe eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_get_lnoe_req_create(const uint8_t *dest_mac_addr,
											 const uint8_t *src_mac_addr,
											 HLB_req_id_t req_id,
											 void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_nscm_get_snre_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * get_snre_info_cnf to the nscm snre_info api struct
 *
 *
 *  @param  input : msg - The get_snre eth msg
 *  @param  output : snre_info - The nscm snre_info api struct
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_get_snre_cnf_parse(const void *msg, HLB_snre_info_t *snre_info);

/**
 *   Function Name: HLB_nscm_get_snre_req_create
 *
 * @brief Description: this function creates the eth msg of
 * get_snre_req
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  output : msg - The get_snre eth msg
 *  @param  input, output : msg_len - The get_snre eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_get_snre_req_create(const uint8_t *dest_mac_addr,
											  const uint8_t *src_mac_addr,
											  HLB_req_id_t req_id,
											  void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_nscm_get_snre_req_create
 *
 * @brief Description: this function creates the eth msg of
 * abort_dump_action
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  output : msg - The abort_dump_action eth msg
 *  @param  input, output : msg_len - The abort_dump_action eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_abort_dump_action_req_create(const uint8_t *dest_mac_addr,
											  		const uint8_t *src_mac_addr,
											  		HLB_req_id_t req_id,
											  		void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_nscm_abort_dump_action_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * abort_dump_action_cnf to the HLB_hpgp_result_t struct
 *
 *
 *  @param  input : msg - The abort_dump_action eth msg
 *  @param  output : result - The result of abort_dump_action
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_abort_dump_action_cnf_parse(const void *msg,
										 HLB_hpgp_result_t *result);

/**
 *   Function Name: HLB_nscm_enter_phy_mode_req_create
 *
 * @brief Description: this function creates the eth msg of
 * enter_phy_mode
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  output : msg - The enter_phy_mode eth msg
 *  @param  input, output : msg_len - The enter_phy_mode eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_enter_phy_mode_req_create(const uint8_t *dest_mac_addr,
											  		const uint8_t *src_mac_addr,
											  		HLB_req_id_t req_id,
											  		void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_nscm_enter_phy_mode_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * enter_phy_mode_cnf to the HLB_hpgp_result_t struct
 *
 *  @param  input : msg - The enter_phy_mode eth msg
 *  @param  output : result - The result of enter_phy_mode
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_enter_phy_mode_cnf_parse(const void *msg,
										 HLB_hpgp_result_t *result);

/**
 *   Function Name: HLB_nscm_read_mem_req_create
 *
 * @brief Description: this function creates the eth msg of
 * read_mem_req
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : read_mem_req - The part of the memory we want to read
 *  @param  input : req_id
 *  @param  output : msg - The read_mem_req eth msg
 *  @param  input, output : msg_len - The read_mem_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_read_mem_req_create(const uint8_t *dest_mac_addr,
										 const uint8_t *src_mac_addr,
										 const HLB_read_mem_req_t *read_mem_req,
										 HLB_req_id_t req_id, void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_nscm_read_mem_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * read_mem_cnf
 *
 *
 *  @param  input : msg - The read_mem cnf eth msg
 *  @param  output : read_mem_cnf - The nscm read_mem_cnf api struct
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_read_mem_cnf_parse(const void *msg, HLB_read_mem_cnf_t *read_mem_cnf);

/**
 *   Function Name: HLB_nscm_write_mem_req_create
 *
 * @brief Description: this function creates the eth msg of
 * write_mem_req
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : write_mem_req - The memory we want to write
 *  @param  input : req_id
 *  @param  output : msg - The write_mem_req eth msg
 *  @param  input, output : msg_len - The write_mem_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_write_mem_req_create(const uint8_t *dest_mac_addr,
										 	const uint8_t *src_mac_addr,
										 	const HLB_write_mem_req_t *write_mem_req,
										 	HLB_req_id_t req_id, void *msg, size_t *msg_len);
											 
/**
 *   Function Name: HLB_nscm_write_mem_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * write_mem_cnf to the HLB_hpgp_result_t struct
 *
 *  @param  input : msg - The write_mem eth msg
 *  @param  output : result - The result of write_mem
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_write_mem_cnf_parse(const void *msg,
										 HLB_hpgp_result_t *result);

/**
 *   Function Name: HLB_nscm_get_dc_calib_req_create
 *
 * @brief Description: this function creates the eth msg of
 * get_dc_calib_req
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  output : msg - The get_dc_calib_req eth msg
 *  @param  input, output : msg_len - The get_dc_calib_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_get_dc_calib_req_create(const uint8_t *dest_mac_addr,
											  		const uint8_t *src_mac_addr,
											  		HLB_req_id_t req_id,
											  		void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_nscm_get_dc_calib_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * dc_calib_cnf
 *
 *
 *  @param  input : msg - The dc_calib cnf eth msg
 *  @param  output : dc_calib_cnf - The nscm dc_calib_cnf api struct
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_get_dc_calib_cnf_parse(const void *msg, HLB_dc_calib_cnf_t *dc_calib_cnf);

/**
 *   Function Name: HLB_nscm_get_device_state_req_create
 *
 * @brief Description: this function creates the eth msg of
 * get_device_state_req
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  output : msg - The get_device_state_req eth msg
 *  @param  input, output : msg_len - The get_device_state_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_get_device_state_req_create(const uint8_t *dest_mac_addr,
											  		const uint8_t *src_mac_addr,
											  		HLB_req_id_t req_id,
											  		void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_nscm_get_device_state_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * get_device_state_cnf
 *
 *
 *  @param  input : msg - The device_state cnf eth msg
 *  @param  output : device_state - The nscm device_state api struct
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_get_device_state_cnf_parse(const void *msg, HLB_hpgp_device_state_cnf_t *device_state);

/**
 *   Function Name: HLB_nscm_get_d_link_status_req_create
 *
 * @brief Description: this function creates the eth msg of
 * get_d_link_status_req
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  output : msg - The get_d_link_status_req eth msg
 *  @param  input, output : msg_len - The get_d_link_status_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_get_d_link_status_req_create(const uint8_t *dest_mac_addr,
											  		const uint8_t *src_mac_addr,
											  		HLB_req_id_t req_id,
											  		void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_nscm_get_d_link_status_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * get_d_link_status_cnf
 *
 *
 *  @param  input : msg - The d_link_status cnf eth msg
 *  @param  output : d_link_status - The nscm d_link_status api struct
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_get_d_link_status_cnf_parse(const void *msg, HLB_hpgp_d_link_status_cnf_t *d_link_status);

/**
 *   Function Name: HLB_nscm_host_message_status_ind_parse
 *
 * @brief Description: this function parses the eth msg of
 * host_message_status_ind to the hpgp result
 *
 *
 *  @param  input : msg - The host_message_status_ind eth msg
 *  @param  output : host_message_status_ind
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_host_message_status_ind_parse(const void *msg,
												 HLB_hpgp_host_message_status_ind_t *host_message_status_ind);

/**
 *   Function Name: HLB_nscm_d_link_ready_ind_parse
 *
 * @brief Description: this function parses the eth msg of
 * d_link_ready_ind to the hpgp result
 *
 *
 *  @param  input : msg - The d_link_ready_ind eth msg
 *  @param  output : d_link_ready_status_ind
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_d_link_ready_ind_parse(const void *msg,
												HLB_hpgp_d_link_ready_status_ind_t *d_link);

/**
 *   Function Name: HLB_nscm_d_link_terminate_req_create
 *
 * @brief Description: this function creates the eth msg of
 * get_d_link_status_req
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  output : msg - The d_link_terminate_req eth msg
 *  @param  input, output : msg_len - The d_link_terminate_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_d_link_terminate_req_create(const uint8_t *dest_mac_addr,
											  		const uint8_t *src_mac_addr,
											  		HLB_req_id_t req_id,
											  		void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_nscm_device_info_req_create
 *
 * @brief Description: this function creates the eth msg of
 * device_info_req
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  output : msg - The device_info_req eth msg
 *  @param  input, output : msg_len - The device_info_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_device_info_req_create(const uint8_t *dest_mac_addr,
											 const uint8_t *src_mac_addr,
											 HLB_req_id_t req_id,
											 void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_nscm_device_info_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * device_info_cnf
 *
 *
 *  @param  input : msg - The device_info cnf eth msg
 *  @param  output : device_info - The nscm device_info api struct
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_device_info_cnf_parse(const void *msg,
											HLB_device_info_t *device_info);

/**
 *   Function Name: HLB_nscm_get_amp_map_req_create
 *
 * @brief Description: this function creates the eth msg of
 * get_amp_map_req
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : req_id
 *  @param  output : msg - The get_amp_map_req eth msg
 *  @param  input, output : msg_len - The get_amp_map_req eth msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_get_amp_map_req_create(const uint8_t *dest_mac_addr,
											 const uint8_t *src_mac_addr,
											 HLB_req_id_t req_id,
											 void *msg, size_t *msg_len);

/**
 *   Function Name: HLB_nscm_get_amp_map_cnf_parse
 *
 * @brief Description: this function parses the eth msg of
 * amp_map_cnf fragment
 *
 *
 *  @param  input  : msg - The amp_map cnf eth msg
 *  @param  input  : msg_len - The length of msg.
 *  @param  output : amp_map - The nscm get_amp_map api struct
 *  @param  input  : parse_at - Position inside amp_map to continue parsing the fragment into.
 *  @param  output : parsed_amount - The amount parsed into amp_map from msg.
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t HLB_nscm_get_amp_map_cnf_parse(const void *msg, size_t msg_len,
											HLB_get_amp_map_t *amp_map,
											uint32_t parse_at, uint16_t *parsed_amount);

/********************************************************************
* INTERNAL FUNCTIONS DECLARATIONS (FOR UNIT TESTING)
********************************************************************/

#endif // HLB_PROTOCOL_H
