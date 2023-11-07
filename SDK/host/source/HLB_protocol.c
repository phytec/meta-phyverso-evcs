/********************************************************************
*
* Module Name: protocol
* Design:
* Implement protocol api
*
********************************************************************/
/*******************************************************************
* IMPORTS
********************************************************************/
#include "HLB_protocol.h"
#include "osal.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "version.h"
/*******************************************************************
* CONSTANTS
********************************************************************/
#define PROTOCOL_VERSION 1U

/*******************************************************************
* TYPES
********************************************************************/
/*******************************************************************
* STATIC and GLOBAL DATA
********************************************************************/
/*******************************************************************
* MACROS
********************************************************************/
#define HLB_is_invalid(val, min, max) (((val > max) || (val < min)))

#define ENUM_AS_U8(x)  ((uint8_t)x)
#define ENUM_AS_U16(x) ((uint16_t)x)
#define ENUM_AS_U32(x) ((uint32_t)x)

#define U8_NOT_BETWEEN(val, min, max) (((val > (uint8_t)max) || (val < (uint8_t)min)))

/*******************************************************************
* INTERNAL FUNCTIONS
********************************************************************/

static void HLB_update_layer_2_header(const uint8_t dest_mac_addr[MAC_LEN],
									  const uint8_t src_mac_addr[MAC_LEN],
									  void *msg)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	const uint16_t ether_type = ETHER_TYPE;

	MEMCPY(hlb_packet->layer_2_header.dest_mac_addr, dest_mac_addr, MAC_LEN);
	MEMCPY(hlb_packet->layer_2_header.src_mac_addr, src_mac_addr, MAC_LEN);
	COPY_HOST_16_BIT_TO_BE_UNALIGNED(hlb_packet->layer_2_header.eth_type, ether_type)
}

static void HLB_update_management_header(HLB_msg_id_t msg_id,
										 uint8_t frag_idx,
										 uint8_t num_frags,
										 uint8_t fmsn,
										 void *msg)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	uint8_t frags_count;

	frags_count = (num_frags != 0U) ? (num_frags) : (num_frags + 1U);

	hlb_packet->management_header.mmv = MANAGEMENT_MESSAGE_VERSION;
	COPY_HOST_16_BIT_TO_LE_UNALIGNED(hlb_packet->management_header.msg_id, msg_id)
	hlb_packet->management_header.fragmetation[1] = fmsn;
	hlb_packet->management_header.fragmetation[0] = ((frags_count - 1U) << 4U) | (frag_idx);
}

static void HLB_update_oui(void *msg)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;

	hlb_packet->OUI[0] = HLB_LUMUSSIL_OUI_1;
	hlb_packet->OUI[1] = HLB_LUMUSSIL_OUI_2;
	hlb_packet->OUI[2] = HLB_LUMUSSIL_OUI_3;
}

static void HLB_update_vendor_header(HLB_req_id_t req_id,
									 void *msg)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	const uint16_t header_version = PROTOCOL_VERSION;

	hlb_packet->vendor_header.req_id = req_id;
	COPY_HOST_16_BIT_TO_LE_UNALIGNED(hlb_packet->vendor_header.main_version, header_version)
}

static size_t HLB_handle_frag_headers(const uint8_t dest_mac_addr[MAC_LEN],
									  const uint8_t src_mac_addr[MAC_LEN],
									  HLB_protocol_msg_id_t msg_id, HLB_req_id_t req_id,
									  uint8_t frag_id, uint8_t num_frags, uint8_t fmsn,
									  void *msg)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	size_t headers_size;

	MEMSET(hlb_packet, 0x00, HLB_PACKET_HEADERS_SIZE);
	HLB_update_layer_2_header(dest_mac_addr, src_mac_addr, hlb_packet);
	HLB_update_management_header((HLB_msg_id_t)msg_id, frag_id, num_frags, fmsn, hlb_packet);

	if (frag_id == 0U)
	{
		HLB_update_oui(hlb_packet);
		HLB_update_vendor_header(req_id, hlb_packet);
		headers_size = HLB_PACKET_HEADERS_SIZE;
	}
	else
	{
		headers_size = HLB_FRAG_PACKET_HEADERS_SIZE;
	}

	return headers_size;
}

/**
 *   Function Name: HLB_handle_headers
 *
 * @brief Description: this function updates the packets headers
 * and return the size of the headers
 *
 *
 *  @param  input : dest_mac_addr - Destination mac address
 *  @param  input : src_mac_addr - Source mac address
 *  @param  input : msg_id - The msg id
 *  @param  input : req_id
 *  @param  output : hlb_packet - The set_key_req eth packet
 *  @return size of headers
 *
 *
 */
static size_t HLB_handle_headers(const uint8_t dest_mac_addr[MAC_LEN],
								 const uint8_t src_mac_addr[MAC_LEN],
								 HLB_protocol_msg_id_t msg_id, HLB_req_id_t req_id,
								 void *msg)
{
	return HLB_handle_frag_headers(dest_mac_addr, src_mac_addr,
								   msg_id, req_id, 0, 0, 0, msg);
}

static RES_result_t HLB_req_validate_and_create_headers(const uint8_t *dest_mac_addr,
														const uint8_t *src_mac_addr,
														HLB_protocol_msg_id_t msg_id,
														HLB_req_id_t req_id,
														size_t payload_size,
														void *msg, size_t *msg_len)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	RES_result_t res = RES_RESULT_OK;

	if ((dest_mac_addr == NULL) || (src_mac_addr == NULL) || (msg == NULL) || (msg_len == NULL))
	{
		res = RES_RESULT_BAD_PARAMETER;
	}
	else if (*msg_len < (HLB_PACKET_HEADERS_SIZE + payload_size))
	{
		res = RES_RESULT_NO_MEMORY;
	}
	else
	{
		*msg_len = HLB_handle_headers(dest_mac_addr, src_mac_addr,
									  (HLB_msg_id_t)msg_id, req_id,
									  hlb_packet);
		*msg_len += payload_size;
	}

	return res;
}

static void HLB_parse_hfid(HLB_hpgp_hfid_t *dst, const HLB_hpgp_hfid_packed_t *src)
{
	if ((sizeof(dst->HFID) > 0) && (sizeof(dst->HFID) >= sizeof(src->HFID)))
	{
		MEMCPY(dst->HFID, src->HFID, sizeof(src->HFID));
		dst->HFID[sizeof(dst->HFID) - 1] = '\0';
	}
}

/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/

int HLB_is_control_path_message(const void *msg)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	HLB_protocol_msg_id_t prot_msg_id;
	int ret;

	if (!hlb_packet)
	{
		ret = 0;
	}
	else
	{
		prot_msg_id = (HLB_protocol_msg_id_t)HLB_get_message_id(msg);

		switch (prot_msg_id)
		{
		case HLB_PROTOCOL_MSG_ID_APCM_SET_KEY_REQ:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_SET_KEY_CNF:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_GET_KEY_REQ:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_GET_KEY_CNF:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_SET_CCO_REQ:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_SET_CCO_CNF:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_CONN_ADD_REQ:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_CONN_ADD_CNF:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_CONN_ADD_IND:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_CONN_ADD_RESP:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_CONN_MOD_REQ:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_CONN_MOD_CNF:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_CONN_MOD_IND:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_CONN_MOD_RESP:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_CONN_REL_REQ:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_CONN_REL_CNF:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_CONN_REL_IND:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_GET_NTB_REQ:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_GET_NTB_CNF:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_AUTHORIZE_REQ:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_AUTHORIZE_CNF:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_AUTHORIZE_IND:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_GET_SECURITY_MODE_REQ:	/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_GET_SECURITY_MODE_CNF:	/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_SET_SECURITY_MODE_REQ:	/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_SET_SECURITY_MODE_CNF:	/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_GET_NETWORKS_REQ:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_GET_NETWORKS_CNF:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_SET_NETWORKS_REQ:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_SET_NETWORKS_CNF:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_GET_NEW_STA_REQ:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_GET_NEW_STA_CNF:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_GET_NEW_STA_IND:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_STA_RESTART_REQ:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_STA_RESTART_CNF:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_NET_EXIT_REQ:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_NET_EXIT_CNF:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_SET_TONE_MASK_REQ:		/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_SET_TONE_MASK_CNF:		/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_STA_CAP_REQ:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_STA_CAP_CNF:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_NW_INFO_REQ:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_NW_INFO_CNF:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_LINK_STATS_REQ:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_LINK_STATS_CNF:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_GET_BEACON_REQ:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_GET_BEACON_CNF:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_GET_HFID_REQ:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_GET_HFID_CNF:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_SET_HFID_REQ:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_SET_HFID_CNF:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_SET_HD_DURATION_REQ:		/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_SET_HD_DURATION_CNF:		/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_UNASSOCIATED_STA_IND:		/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_SC_JOIN_REQ:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_SC_JOIN_CNF:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_SET_PPKEYS_REQ:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_SET_PPKEYS_CNF:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_CONF_SLAC_REQ:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_APCM_CONF_SLAC_CNF:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_GET_VERSION_REQ:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_GET_VERSION_CNF:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_RESET_DEVICE_REQ:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_GET_CE2_INFO_REQ:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_GET_CE2_INFO_CNF:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_GET_CE2_DATA_REQ:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_GET_CE2_DATA_CNF:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_GET_LNOE_REQ:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_GET_LNOE_CNF:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_GET_SNRE_REQ:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_GET_SNRE_CNF:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_ABORT_DUMP_ACTION_REQ:	/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_ABORT_DUMP_ACTION_CNF:	/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_ENTER_PHY_MODE_REQ:		/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_ENTER_PHY_MODE_CNF:		/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_READ_MEM_REQ:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_READ_MEM_CNF:				/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_WRITE_MEM_REQ:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_WRITE_MEM_CNF:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_GET_DC_CALIB_REQ:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_GET_DC_CALIB_CNF:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_GET_DEVICE_STATE_REQ:		/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_GET_DEVICE_STATE_CNF:		/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_GET_D_LINK_STATUS_REQ:	/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_GET_D_LINK_STATUS_CNF:	/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_VS_HOST_MESSAGE_STATUS_IND:	/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_D_LINK_READY_IND:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_D_LINK_TERMINATE_REQ:		/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_D_LINK_TERMINATE_CNF:		/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_DEVICE_INFO_REQ:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_DEVICE_INFO_CNF:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_GET_AMP_MAP_REQ:			/* FALLTHROUGH */
		case HLB_PROTOCOL_MSG_ID_NSCM_GET_AMP_MAP_CNF:			/* FALLTHROUGH */
			ret = 1;
			break;

		default:
			ret = 0;
			break;
		}
	}

	return ret;
}

HLB_msg_id_t HLB_get_message_id(const void *msg)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	HLB_msg_id_t msg_id;

	if (!hlb_packet)
	{
		msg_id = 0;
	}
	else
	{
		COPY_UNALIGNED_LE_16_BIT_TO_HOST(msg_id, hlb_packet->management_header.msg_id);
	}

	return msg_id;
}

RES_result_t HLB_apcm_set_key_req_create(const uint8_t *dest_mac_addr,
										 const uint8_t *src_mac_addr,
										 const HLB_hpgp_set_key_req_t *set_key_req,
										 HLB_req_id_t req_id, void *msg, size_t *msg_len)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	HLB_hpgp_set_key_req_packed_t payload;
	RES_result_t res;

	if ((set_key_req == NULL) ||
		HLB_is_invalid(set_key_req->security_level, HLB_HPGP_SECURITY_LEVEL_SC, HLB_HPGP_SECURITY_LEVEL_HS))
	{
		res = RES_RESULT_BAD_PARAMETER;
	}
	else
	{
		res = HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
												  HLB_PROTOCOL_MSG_ID_APCM_SET_KEY_REQ,
												  req_id, sizeof(payload), msg, msg_len);
	}

	if (res == RES_RESULT_OK)
	{
		MEMCPY(payload.NID, &set_key_req->nid, sizeof(payload.NID));
		MEMCPY(payload.NMK, &set_key_req->nmk, sizeof(payload.NMK));
		payload.security_level = ENUM_AS_U8(set_key_req->security_level);
		MEMCPY(hlb_packet->payload, &payload, sizeof(payload));
	}

	return res;
}

static RES_result_t HLB_cnf_validate_msg(const void *msg,
										 HLB_protocol_msg_id_t expected_msg_id)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	RES_result_t res = RES_RESULT_BAD_PARAMETER;

	if ((hlb_packet != NULL) && (HLB_get_message_id(msg) == (HLB_msg_id_t)expected_msg_id))
	{
		res = RES_RESULT_OK;
	}

	return res;
}

static RES_result_t HLB_apcm_cnf_parse_hpgp_status(const void *msg,
												   HLB_hpgp_result_t *result,
												   HLB_protocol_msg_id_t expected_msg_id)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_8b_status_cnf_packed_t *payload;
	RES_result_t res;

	if (result == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, expected_msg_id);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_8b_status_cnf_packed_t *)hlb_packet->payload;

		if (U8_NOT_BETWEEN(payload->status, HLB_HPGP_RESULT_SUCCESS, HLB_HPGP_RESULT_FAILURE))
		{
			res = RES_RESULT_INVALID_FW_PCKT;
		}
		else
		{
			*result = payload->status;
		}
	}

	return res;
}

RES_result_t HLB_apcm_set_key_cnf_parse(const void *msg,
										HLB_hpgp_result_t *result)
{
	return HLB_apcm_cnf_parse_hpgp_status(msg, result, HLB_PROTOCOL_MSG_ID_APCM_SET_KEY_CNF);
}

RES_result_t HLB_apcm_get_key_req_create(const uint8_t *dest_mac_addr,
										 const uint8_t *src_mac_addr,
										 HLB_req_id_t req_id,
										 void *msg, size_t *msg_len)
{
	return HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											   HLB_PROTOCOL_MSG_ID_APCM_GET_KEY_REQ,
											   req_id, 0, msg, msg_len);
}

RES_result_t HLB_apcm_get_key_cnf_parse(const void *msg,
										HLB_hpgp_get_key_cnf_t *get_key_cnf)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_hpgp_get_key_cnf_packed_t *payload;
	RES_result_t res;

	if (get_key_cnf == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_APCM_GET_KEY_CNF);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_hpgp_get_key_cnf_packed_t *)hlb_packet->payload;

		MEMCPY(&get_key_cnf->nid, &payload->NID, sizeof(get_key_cnf->nid));
		MEMCPY(&get_key_cnf->nmk, &payload->NMK, sizeof(get_key_cnf->nmk));
	}

	return res;
}

RES_result_t HLB_apcm_set_cco_req_create(const uint8_t *dest_mac_addr,
										 const uint8_t *src_mac_addr,
										 HLB_req_id_t req_id, HLB_hpgp_cco_mode_t cco_mode,
										 void *msg, size_t *msg_len)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	HLB_hpgp_set_cco_req_packed_t payload;
	RES_result_t res;

	if (HLB_is_invalid(cco_mode, HLB_HPGP_CCO_MODE_AUTO, HLB_HPGP_CCO_MODE_ALWAYS))
	{
		res = RES_RESULT_BAD_PARAMETER;
	}
	else if (cco_mode == HLB_HPGP_CCO_MODE_AUTO)
	{
		hlb_log_error("CCO Auto is not supported by FW");
		res = RES_RESULT_NOT_SUPPORTED;
	}
	else
	{
		res = HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
												  HLB_PROTOCOL_MSG_ID_APCM_SET_CCO_REQ,
												  req_id, sizeof(payload), msg, msg_len);
	}

	if (res == RES_RESULT_OK)
	{
		payload.cco_mode = ENUM_AS_U8(cco_mode);
		MEMCPY(hlb_packet->payload, &payload, sizeof(payload));
	}

	return res;
}

RES_result_t HLB_apcm_set_cco_cnf_parse(const void *msg,
										HLB_hpgp_result_t *result)
{
	return HLB_apcm_cnf_parse_hpgp_status(msg, result, HLB_PROTOCOL_MSG_ID_APCM_SET_CCO_CNF);
}

RES_result_t HLB_apcm_get_ntb_req_create(const uint8_t *dest_mac_addr,
										 const uint8_t *src_mac_addr, HLB_req_id_t req_id,
										 void *msg, size_t *msg_len)
{
	return HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											   HLB_PROTOCOL_MSG_ID_APCM_GET_NTB_REQ,
											   req_id, 0, msg, msg_len);
}

RES_result_t HLB_apcm_get_ntb_cnf_parse(const void *msg,
										uint32_t *ntb)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_hpgp_get_nbt_cnf_packed_t *payload;
	uint32_t tmp_ntb;
	RES_result_t res;

	if (ntb == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_APCM_GET_NTB_CNF);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_hpgp_get_nbt_cnf_packed_t *)hlb_packet->payload;

		COPY_UNALIGNED_LE_32_BIT_TO_HOST(tmp_ntb, payload->ntb)
		*ntb = tmp_ntb;
	}

	return res;
}

RES_result_t HLB_apcm_get_security_mode_req_create(const uint8_t *dest_mac_addr,
												   const uint8_t *src_mac_addr, HLB_req_id_t req_id,
												   void *msg, size_t *msg_len)
{
	return HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											   HLB_PROTOCOL_MSG_ID_APCM_GET_SECURITY_MODE_REQ,
											   req_id, 0, msg, msg_len);
}

RES_result_t HLB_apcm_get_security_mode_cnf_parse(const void *msg,
												  HLB_hpgp_get_security_mode_cnf_t *security_mode_cnf)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_hpgp_get_security_mode_cnf_packed_t *payload;
	RES_result_t res;

	if (security_mode_cnf == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_APCM_GET_SECURITY_MODE_CNF);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_hpgp_get_security_mode_cnf_packed_t *)hlb_packet->payload;

		if (U8_NOT_BETWEEN(payload->result, HLB_HPGP_RESULT_SUCCESS, HLB_HPGP_RESULT_FAILURE) ||
			U8_NOT_BETWEEN(payload->security_mode, HLB_HPGP_SECURITY_MODE_SECURE, HLB_HPGP_SECURITY_MODE_SC_JOIN))
		{
			res = RES_RESULT_INVALID_FW_PCKT;
		}
		else
		{
			security_mode_cnf->result = payload->result;
			security_mode_cnf->security_mode = payload->security_mode;
		}	
	}

	return res;
}

RES_result_t HLB_apcm_set_security_mode_req_create(const uint8_t *dest_mac_addr,
												   const uint8_t *src_mac_addr,
												   HLB_req_id_t req_id, HLB_hpgp_security_mode_t security_mode,
												   void *msg, size_t *msg_len)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	HLB_hpgp_set_security_mode_req_packed_t payload;
	RES_result_t res;

	if (HLB_is_invalid(security_mode, HLB_HPGP_SECURITY_MODE_SECURE, HLB_HPGP_SECURITY_MODE_SC_JOIN))
	{
		res = RES_RESULT_BAD_PARAMETER;
	}
	else
	{
		res = HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
												  HLB_PROTOCOL_MSG_ID_APCM_SET_SECURITY_MODE_REQ,
												  req_id, sizeof(payload), msg, msg_len);
	}

	if (res == RES_RESULT_OK)
	{
		payload.security_mode = ENUM_AS_U8(security_mode);
		MEMCPY(hlb_packet->payload, &payload, sizeof(payload));
	}

	return res;
}

RES_result_t HLB_apcm_set_security_mode_cnf_parse(const void *msg,
												  HLB_hpgp_result_t *result)
{
	return HLB_apcm_cnf_parse_hpgp_status(msg, result, HLB_PROTOCOL_MSG_ID_APCM_SET_SECURITY_MODE_CNF);
}

RES_result_t HLB_apcm_get_networks_req_create(const uint8_t *dest_mac_addr,
											  const uint8_t *src_mac_addr, HLB_req_id_t req_id,
											  void *msg, size_t *msg_len)
{
	return HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											   HLB_PROTOCOL_MSG_ID_APCM_GET_NETWORKS_REQ,
											   req_id, 0, msg, msg_len);
}

RES_result_t HLB_apcm_get_networks_cnf_parse(const void *msg,
											 HLB_hpgp_get_networks_cnf_t *networks_cnf)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_hpgp_get_networks_cnf_packed_t *payload;
	RES_result_t res;
	uint8_t i;

	if (networks_cnf == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_APCM_GET_NETWORKS_CNF);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_hpgp_get_networks_cnf_packed_t *)hlb_packet->payload;
		networks_cnf->num_of_networks = payload->num_of_networks;
		if (networks_cnf->num_of_networks > MAX_NUM_OF_NETWORKS)
		{
			res = RES_RESULT_INVALID_FW_PCKT;
		}
		else
		{
			for (i = 0; i < networks_cnf->num_of_networks; i++)
			{
				networks_cnf->networks[i].status = payload->networks[i].status;
				if (HLB_is_invalid(networks_cnf->networks[i].status, HLB_HPGP_NETWORKS_STATUS_JOINED, HLB_HPGP_NETWORKS_STATUS_BLACKLISTED))
				{
					res = RES_RESULT_INVALID_FW_PCKT;
					break;
				}

				HLB_parse_hfid(&networks_cnf->networks[i].hfid, &payload->networks[i].hfid);
				MEMCPY(&networks_cnf->networks[i].mac_addr, &payload->networks[i].mac_addr, sizeof(payload->networks[i].mac_addr));
				MEMCPY(&networks_cnf->networks[i].nid, &payload->networks[i].nid, sizeof(payload->networks[i].nid));
			}
		}
	}

	return res;
}

RES_result_t HLB_apcm_set_networks_req_create(const uint8_t *dest_mac_addr,
											  const uint8_t *src_mac_addr, HLB_req_id_t req_id,
											  const HLB_hpgp_set_networks_req_t *networks_req,
											  void *msg, size_t *msg_len)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	HLB_hpgp_set_networks_req_packed_t payload;
	RES_result_t res;

	if ((networks_req == NULL) ||
		HLB_is_invalid(networks_req->req_type, HLB_HPGP_NETWORKS_REQ_TYPE_JOIN_NOW, HLB_HPGP_NETWORKS_REQ_TYPE_REHABILITATE))
	{
		res = RES_RESULT_BAD_PARAMETER;
	}
	else
	{
		res = HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
												  HLB_PROTOCOL_MSG_ID_APCM_SET_NETWORKS_REQ,
												  req_id, sizeof(payload), msg, msg_len);
	}

	if (res == RES_RESULT_OK)
	{
		MEMCPY(&payload.nid, &networks_req->nid, sizeof(payload.nid));
		payload.req_type = ENUM_AS_U8(networks_req->req_type);
		MEMCPY(hlb_packet->payload, &payload, sizeof(payload));
	}

	return res;
}

RES_result_t HLB_apcm_set_networks_cnf_parse(const void *msg,
											 HLB_hpgp_result_t *result)
{
	return HLB_apcm_cnf_parse_hpgp_status(msg, result, HLB_PROTOCOL_MSG_ID_APCM_SET_NETWORKS_CNF);
}

RES_result_t HLB_apcm_get_new_sta_req_create(const uint8_t *dest_mac_addr,
											 const uint8_t *src_mac_addr, HLB_req_id_t req_id,
											 void *msg, size_t *msg_len)
{
	return HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											   HLB_PROTOCOL_MSG_ID_APCM_GET_NEW_STA_REQ,
											   req_id, 0, msg, msg_len);
}

RES_result_t HLB_apcm_get_new_sta_cnf_parse(const void *msg,
											HLB_hpgp_get_new_sta_cnf_t *new_sta_cnf)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_hpgp_get_new_sta_cnf_packed_t *payload;
	RES_result_t res;
	uint8_t i;

	if (new_sta_cnf == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_APCM_GET_NEW_STA_CNF);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_hpgp_get_new_sta_cnf_packed_t *)hlb_packet->payload;

		new_sta_cnf->num_of_new_sta = payload->num_of_new_sta;
		if (new_sta_cnf->num_of_new_sta > MAX_NUM_OF_NEW_STA)
		{
			res = RES_RESULT_INVALID_FW_PCKT;
		}
		else
		{
			for (i = 0; i < new_sta_cnf->num_of_new_sta; i++)
			{
				MEMCPY(&new_sta_cnf->stations[i].user_set_hfid, &payload->stations[i].user_set_hfid, sizeof(payload->stations[i].user_set_hfid));
				MEMCPY(&new_sta_cnf->stations[i].manuf_set_hfid, &payload->stations[i].manuf_set_hfid, sizeof(payload->stations[i].manuf_set_hfid));
				MEMCPY(&new_sta_cnf->stations[i].mmac, &payload->stations[i].mmac, sizeof(payload->stations[i].mmac));
			}
		}
	}

	return res;
}

RES_result_t HLB_apcm_get_new_sta_ind_parse(const void *msg,
											HLB_hpgp_get_new_sta_ind_t *new_sta_ind)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_hpgp_get_new_sta_ind_packed_t *payload;
	uint8_t i;
	RES_result_t res;

	if (new_sta_ind == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_APCM_GET_NEW_STA_IND);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_hpgp_get_new_sta_ind_packed_t *)hlb_packet->payload;

		if (payload->num_of_new_sta > MAX_NUM_OF_NEW_STA)
		{
			res = RES_RESULT_INVALID_FW_PCKT;
		}
		else
		{
			new_sta_ind->num_of_new_sta = payload->num_of_new_sta;
			for (i = 0; i < new_sta_ind->num_of_new_sta; i++)
			{
				MEMCPY(&new_sta_ind->stations[i].user_set_hfid, &payload->stations[i].user_set_hfid, sizeof(payload->stations[i].user_set_hfid));
				MEMCPY(&new_sta_ind->stations[i].manuf_set_hfid, &payload->stations[i].manuf_set_hfid, sizeof(payload->stations[i].manuf_set_hfid));
				MEMCPY(&new_sta_ind->stations[i].mmac, &payload->stations[i].mmac, sizeof(payload->stations[i].mmac));
			}
		}
	}

	return res;
}

RES_result_t HLB_apcm_sta_restart_req_create(const uint8_t *dest_mac_addr,
											 const uint8_t *src_mac_addr, HLB_req_id_t req_id,
											 void *msg, size_t *msg_len)
{
	return HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											   HLB_PROTOCOL_MSG_ID_APCM_STA_RESTART_REQ,
											   req_id, 0, msg, msg_len);
}

RES_result_t HLB_apcm_sta_restart_cnf_parse(const void *msg,
											HLB_hpgp_result_t *result)
{
	return HLB_apcm_cnf_parse_hpgp_status(msg, result, HLB_PROTOCOL_MSG_ID_APCM_STA_RESTART_CNF);
}

RES_result_t HLB_apcm_net_exit_req_create(const uint8_t *dest_mac_addr,
										  const uint8_t *src_mac_addr, HLB_req_id_t req_id,
										  void *msg, size_t *msg_len)
{
	return HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											   HLB_PROTOCOL_MSG_ID_APCM_NET_EXIT_REQ,
											   req_id, 0, msg, msg_len);
}

RES_result_t HLB_apcm_net_exit_cnf_parse(const void *msg,
										 HLB_hpgp_result_t *result)
{
	return HLB_apcm_cnf_parse_hpgp_status(msg, result, HLB_PROTOCOL_MSG_ID_APCM_NET_EXIT_CNF);
}

RES_result_t HLB_apcm_set_tone_mask_req_create(const uint8_t *dest_mac_addr,
											   const uint8_t *src_mac_addr,
											   HLB_req_id_t req_id, uint16_t tone_mask,
											   void *msg, size_t *msg_len)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	HLB_hpgp_set_tone_mask_req_packed_t payload;
	RES_result_t res;

	res = HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											  HLB_PROTOCOL_MSG_ID_APCM_SET_TONE_MASK_REQ,
											  req_id, sizeof(payload), msg, msg_len);

	if (res == RES_RESULT_OK)
	{
		COPY_HOST_16_BIT_TO_LE_UNALIGNED(payload.tone_mask, tone_mask)
		MEMCPY(hlb_packet->payload, &payload, sizeof(payload));
	}

	return res;
}

RES_result_t HLB_apcm_set_tone_mask_cnf_parse(const void *msg,
											  HLB_hpgp_result_t *result)
{
	return HLB_apcm_cnf_parse_hpgp_status(msg, result, HLB_PROTOCOL_MSG_ID_APCM_SET_TONE_MASK_CNF);
}

RES_result_t HLB_apcm_sta_cap_req_create(const uint8_t *dest_mac_addr,
										 const uint8_t *src_mac_addr, HLB_req_id_t req_id,
										 void *msg, size_t *msg_len)
{
	return HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											   HLB_PROTOCOL_MSG_ID_APCM_STA_CAP_REQ,
											   req_id, 0, msg, msg_len);
}

RES_result_t HLB_apcm_sta_cap_cnf_parse(const void *msg,
										HLB_hpgp_sta_cap_cnf_t *sta_cap_cnf)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_hpgp_sta_cap_cnf_packed_t *payload;
	RES_result_t res;

	if (sta_cap_cnf == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_APCM_STA_CAP_CNF);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_hpgp_sta_cap_cnf_packed_t *)hlb_packet->payload;

		if (U8_NOT_BETWEEN(payload->home_plug_1_0_interop, HLB_HPGP_CAP_HP_1_0_INTEROP_NOT_SUPPORTED, HLB_HPGP_CAP_HP_1_0_INTEROP_SUPPORTED))
		{
			res = RES_RESULT_INVALID_FW_PCKT;
		}
		else
		{
			MEMCPY(&sta_cap_cnf->mac_addr, &payload->mac_addr, sizeof(payload->mac_addr));
			sta_cap_cnf->auto_connect = payload->auto_connect;
			sta_cap_cnf->av_version = payload->av_version;
			sta_cap_cnf->backup_cco_capable = payload->backup_cco_capable;
			sta_cap_cnf->bidirectional_burst = payload->bidirectional_burst;
			sta_cap_cnf->cco_cap = payload->cco_cap;
			sta_cap_cnf->home_plug_1_0_interop = payload->home_plug_1_0_interop;
			sta_cap_cnf->home_plug_1_1_cap = payload->home_plug_1_1_cap;
			COPY_UNALIGNED_LE_16_BIT_TO_HOST(sta_cap_cnf->implementation_ver, payload->implementation_ver)
			COPY_UNALIGNED_LE_16_BIT_TO_HOST(sta_cap_cnf->max_fl_av, payload->max_fl_av)
			sta_cap_cnf->auto_connect = payload->auto_connect;
			MEMCPY(sta_cap_cnf->oui, payload->oui, sizeof(payload->oui));
			sta_cap_cnf->proxy_capable = payload->proxy_capable;
			sta_cap_cnf->regulatory_cap = payload->regulatory_cap;
			sta_cap_cnf->smoothing = payload->smoothing;
			sta_cap_cnf->soft_handover = payload->soft_handover;
			sta_cap_cnf->two_sym_frame_control = payload->two_sym_frame_control;
		}
	}

	return res;
}

RES_result_t HLB_apcm_nw_info_req_create(const uint8_t *dest_mac_addr,
										 const uint8_t *src_mac_addr, HLB_req_id_t req_id,
										 void *msg, size_t *msg_len)
{
	return HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											   HLB_PROTOCOL_MSG_ID_APCM_NW_INFO_REQ,
											   req_id, 0, msg, msg_len);
}

RES_result_t HLB_apcm_nw_info_cnf_parse(const void *msg,
										HLB_hpgp_nw_info_cnf_t *nw_info_cnf)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_hpgp_nw_info_cnf_packed_t *payload;
	RES_result_t res;
	uint8_t i;

	if (nw_info_cnf == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_APCM_NW_INFO_CNF);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_hpgp_nw_info_cnf_packed_t *)hlb_packet->payload;

		if (payload->num_of_nw_info > MAX_NUM_OF_NETWORKS)
		{
			res = RES_RESULT_BAD_PARAMETER;
		}
		else
		{
			nw_info_cnf->num_of_nw_info = payload->num_of_nw_info;
			for (i = 0; i < nw_info_cnf->num_of_nw_info; i++)
			{
				MEMCPY(&nw_info_cnf->nwinfo[i].nid, &payload->nwinfo[i].nid, sizeof(payload->nwinfo[i].nid));
				MEMCPY(&nw_info_cnf->nwinfo[i].cco_mac_addr, &payload->nwinfo[i].cco_mac_addr, sizeof(payload->nwinfo[i].cco_mac_addr));
				nw_info_cnf->nwinfo[i].access = payload->nwinfo[i].access;
				nw_info_cnf->nwinfo[i].num_cord_neighbor_networks = payload->nwinfo[i].num_cord_neighbor_networks;
				nw_info_cnf->nwinfo[i].short_nid = payload->nwinfo[i].short_nid;
				nw_info_cnf->nwinfo[i].sta_role = payload->nwinfo[i].sta_role;
				nw_info_cnf->nwinfo[i].terminal_equipment_id = payload->nwinfo[i].terminal_equipment_id;
			}
		}
	}

	return res;
}

RES_result_t HLB_nscm_link_stats_req_create(const uint8_t *dest_mac_addr,
											const uint8_t *src_mac_addr, HLB_req_id_t req_id,
											const HLB_hpgp_link_stats_req_t *link_stats_req,
											void *msg, size_t *msg_len)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	HLB_hpgp_link_stats_req_packed_t payload;
	RES_result_t res;

	if ((link_stats_req == NULL) ||
		HLB_is_invalid(link_stats_req->req_type, HLB_HPGP_LINK_STATS_REQ_TYPE_RESET_STATS, HLB_HPGP_LINK_STATS_REQ_TYPE_GET_AND_RESET_STATS) ||
		HLB_is_invalid(link_stats_req->transmit_link_flag, HLB_HPGP_LINK_STATS_TRANSMIT_LINK, HLB_HPGP_LINK_STATS_RECEIVE_LINK))
	{
		res = RES_RESULT_BAD_PARAMETER;
	}
	else
	{
		res = HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
												  HLB_PROTOCOL_MSG_ID_NSCM_LINK_STATS_REQ,
												  req_id, sizeof(payload), msg, msg_len);
	}

	if (res == RES_RESULT_OK)
	{
		MEMCPY(&payload.da_sa_mac_addr, &link_stats_req->da_sa_mac_addr, sizeof(payload.da_sa_mac_addr));
		MEMCPY(&payload.nid, &link_stats_req->nid, sizeof(payload.nid));
		payload.req_type = ENUM_AS_U8(link_stats_req->req_type);
		payload.transmit_link_flag = ENUM_AS_U8(link_stats_req->transmit_link_flag);
		MEMCPY(hlb_packet->payload, &payload, sizeof(payload));
	}

	return res;
}

RES_result_t HLB_nscm_link_stats_cnf_parse(const void *msg,
										   HLB_hpgp_link_stats_cnf_t *link_stats_cnf)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_hpgp_link_stats_cnf_conf_params_packed_t *payload;
	const HLB_hpgp_link_stats_cnf_transmit_packed_t *payload_transmit;
	const HLB_hpgp_link_stats_cnf_receive_packed_t *payload_receive;
	RES_result_t res;

	if (link_stats_cnf == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_NSCM_LINK_STATS_CNF);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_hpgp_link_stats_cnf_conf_params_packed_t *)hlb_packet->payload;

		if ((payload->res_type > HLB_HPGP_RESULT_FAILURE) ||
			(payload->transmit_link_flag > HLB_HPGP_LINK_STATS_RECEIVE_LINK))
		{
			res = RES_RESULT_INVALID_FW_PCKT;
		}
		else
		{
			link_stats_cnf->res_type = payload->res_type;
			link_stats_cnf->transmit_link_flag = payload->transmit_link_flag;
			if (link_stats_cnf->transmit_link_flag == HLB_HPGP_LINK_STATS_TRANSMIT_LINK)
			{
				payload_transmit = (const HLB_hpgp_link_stats_cnf_transmit_packed_t *)hlb_packet->payload;
				COPY_UNALIGNED_LE_16_BIT_TO_HOST(link_stats_cnf->transmit_link.beacon_period_cnt, payload_transmit->transmit_link.beacon_period_cnt)
				COPY_UNALIGNED_LE_32_BIT_TO_HOST(link_stats_cnf->transmit_link.MSDUs, payload_transmit->transmit_link.MSDUs)
				COPY_UNALIGNED_LE_32_BIT_TO_HOST(link_stats_cnf->transmit_link.octets, payload_transmit->transmit_link.octets)
				COPY_UNALIGNED_LE_32_BIT_TO_HOST(link_stats_cnf->transmit_link.segments_generated, payload_transmit->transmit_link.segments_generated)
				COPY_UNALIGNED_LE_32_BIT_TO_HOST(link_stats_cnf->transmit_link.segments_delivered, payload_transmit->transmit_link.segments_delivered)
				COPY_UNALIGNED_LE_32_BIT_TO_HOST(link_stats_cnf->transmit_link.segments_dropped, payload_transmit->transmit_link.segments_dropped)
				COPY_UNALIGNED_LE_32_BIT_TO_HOST(link_stats_cnf->transmit_link.PBs_handed, payload_transmit->transmit_link.PBs_handed)
				COPY_UNALIGNED_LE_32_BIT_TO_HOST(link_stats_cnf->transmit_link.MPDUs_transmitted, payload_transmit->transmit_link.MPDUs_transmitted)
				COPY_UNALIGNED_LE_32_BIT_TO_HOST(link_stats_cnf->transmit_link.MPDUs_successfully_acked, payload_transmit->transmit_link.MPDUs_successfully_acked)
			}
			else
			{
				payload_receive = (const HLB_hpgp_link_stats_cnf_receive_packed_t *)hlb_packet->payload;
				COPY_UNALIGNED_LE_16_BIT_TO_HOST(link_stats_cnf->receive_link.beacon_period_cnt, payload_receive->receive_link.beacon_period_cnt)
				COPY_UNALIGNED_LE_32_BIT_TO_HOST(link_stats_cnf->receive_link.MSDUs, payload_receive->receive_link.MSDUs)
				COPY_UNALIGNED_LE_32_BIT_TO_HOST(link_stats_cnf->receive_link.octets, payload_receive->receive_link.octets)
				COPY_UNALIGNED_LE_32_BIT_TO_HOST(link_stats_cnf->receive_link.segments_received, payload_receive->receive_link.segments_received)
				COPY_UNALIGNED_LE_32_BIT_TO_HOST(link_stats_cnf->receive_link.segments_missed, payload_receive->receive_link.segments_missed)
				COPY_UNALIGNED_LE_32_BIT_TO_HOST(link_stats_cnf->receive_link.PBs_handed, payload_receive->receive_link.PBs_handed)
				COPY_UNALIGNED_LE_32_BIT_TO_HOST(link_stats_cnf->receive_link.MPDUs_received, payload_receive->receive_link.MPDUs_received)
				COPY_UNALIGNED_LE_32_BIT_TO_HOST(link_stats_cnf->receive_link.failed_icv_received_frames, payload_receive->receive_link.failed_icv_received_frames)
			}
		}
	}

	return res;
}

RES_result_t HLB_apcm_get_beacon_req_create(const uint8_t *dest_mac_addr,
											const uint8_t *src_mac_addr, HLB_req_id_t req_id,
											const HLB_hpgp_nid_t *nid,
											void *msg, size_t *msg_len)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	HLB_hpgp_nid_packed_t payload;
	RES_result_t res;

	if (nid == NULL)
	{
		res = RES_RESULT_BAD_PARAMETER;
	}
	else
	{
		res = HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
												  HLB_PROTOCOL_MSG_ID_APCM_GET_BEACON_REQ,
												  req_id, sizeof(payload), msg, msg_len);
	}

	if (res == RES_RESULT_OK)
	{
		MEMCPY(payload.NID, nid->NID, sizeof(payload.NID));
		MEMCPY(hlb_packet->payload, &payload, sizeof(payload));
	}

	return res;
}

RES_result_t HLB_apcm_get_beacon_cnf_parse(const void *msg,
										   HLB_apcm_get_beacon_cnf_t *get_beacon_cnf)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_apcm_get_beacon_cnf_packed_t *payload;
	uint8_t tmp, num_of_beacon_entries, i;
	RES_result_t res;

	if (get_beacon_cnf == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_APCM_GET_BEACON_CNF);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_apcm_get_beacon_cnf_packed_t *)hlb_packet->payload;

		num_of_beacon_entries = payload->beacon_mgmt_info.num_of_beacon_entries;
		if (num_of_beacon_entries > MAX_BEACON_ENTRIES)
		{
			res = RES_RESULT_INVALID_FW_PCKT;
		}
		else
		{
			MEMCPY(get_beacon_cnf->nid.NID, payload->nid_and_hybrid_mode.NID, sizeof(get_beacon_cnf->nid));
			get_beacon_cnf->nid.NID[HPAVKEY_NID_LEN - 1U] &= (0x3FU);
			tmp = payload->nid_and_hybrid_mode.NID[HPAVKEY_NID_LEN - 1U] - get_beacon_cnf->nid.NID[HPAVKEY_NID_LEN - 1U];
			get_beacon_cnf->hybrid_mode = (tmp >> 6);
			get_beacon_cnf->source_terminal_equipment_id = payload->source_terminal_equipment_id;
			get_beacon_cnf->beacon_type = (payload->BT_NCNR_NPSM_NumSlots & 7U);
			get_beacon_cnf->non_coord_networks_reported = ((payload->BT_NCNR_NPSM_NumSlots >> 3U) & 1U);
			get_beacon_cnf->network_power_save_mode = ((payload->BT_NCNR_NPSM_NumSlots >> 4U) & 1U);
			get_beacon_cnf->num_of_beacon_slots = ((payload->BT_NCNR_NPSM_NumSlots >> 5U) & 7U);
			get_beacon_cnf->beacon_slot_usage = payload->beacon_slot_usage;
			get_beacon_cnf->beacon_slot_id = (payload->SlotID_ACLSS_HOIP_RTSBF & 7U);
			get_beacon_cnf->ac_line_cycle_sync_status = ((payload->SlotID_ACLSS_HOIP_RTSBF >> 3U) & 7U);
			get_beacon_cnf->handover_in_progress = ((payload->SlotID_ACLSS_HOIP_RTSBF >> 6U) & 1U);
			get_beacon_cnf->rts_broadcast_flag = ((payload->SlotID_ACLSS_HOIP_RTSBF >> 7U) & 1U);
			get_beacon_cnf->network_mode = (payload->network_mode_cco_RSF_Plevel & 3U);
			get_beacon_cnf->cco_cap = ((payload->network_mode_cco_RSF_Plevel >> 2U) & 3U);
			get_beacon_cnf->reusable_SNID_flag = ((payload->network_mode_cco_RSF_Plevel >> 4U) & 1U);
			get_beacon_cnf->proxy_level = ((payload->network_mode_cco_RSF_Plevel >> 5U) & 7U);
			get_beacon_cnf->beacon_mgmt_info.num_of_beacon_entries = num_of_beacon_entries;
			for (i = 0; i < num_of_beacon_entries; i++)
			{
				get_beacon_cnf->beacon_mgmt_info.beacon_entries[i].header = payload->beacon_mgmt_info.beacon_entries[i].header;
				get_beacon_cnf->beacon_mgmt_info.beacon_entries[i].len = payload->beacon_mgmt_info.beacon_entries[i].len;
				MEMCPY(get_beacon_cnf->beacon_mgmt_info.beacon_entries[i].payload,
					   payload->beacon_mgmt_info.beacon_entries[i].payload,
					   sizeof(get_beacon_cnf->beacon_mgmt_info.beacon_entries[i].payload));
			}
		}
	}

	return res;
}

static RES_result_t HLB_apcm_get_hfid_type1_req_create(const uint8_t *dest_mac_addr,
													   const uint8_t *src_mac_addr, HLB_req_id_t req_id,
													   const HLB_hpgp_get_hfid_req_t *hfid_req,
													   void *msg, size_t *msg_len)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	HLB_hpgp_get_hfid_req_packed_type1_t payload;
	RES_result_t res;

	res = HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											  HLB_PROTOCOL_MSG_ID_APCM_GET_HFID_REQ,
											  req_id, sizeof(payload), msg, msg_len);
	if (res == RES_RESULT_OK)
	{
		payload.req_type = ENUM_AS_U8(hfid_req->req_type);
		MEMCPY(hlb_packet->payload, &payload, sizeof(payload));
	}

	return res;
}

static RES_result_t HLB_apcm_get_hfid_type2_req_create(const uint8_t *dest_mac_addr,
													   const uint8_t *src_mac_addr, HLB_req_id_t req_id,
													   const HLB_hpgp_get_hfid_req_t *hfid_req,
													   void *msg, size_t *msg_len)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	HLB_hpgp_get_hfid_req_packed_type2_t payload;
	RES_result_t res;

	res = HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											  HLB_PROTOCOL_MSG_ID_APCM_GET_HFID_REQ,
											  req_id, sizeof(payload), msg, msg_len);
	if (res == RES_RESULT_OK)
	{
		payload.req_type = ENUM_AS_U8(hfid_req->req_type);
		MEMCPY(payload.nid.NID, hfid_req->nid.NID, sizeof(payload.nid.NID));
		MEMCPY(hlb_packet->payload, &payload, sizeof(payload));
	}

	return res;
}

static RES_result_t HLB_apcm_get_hfid_type3_req_create(const uint8_t *dest_mac_addr,
													   const uint8_t *src_mac_addr, HLB_req_id_t req_id,
													   const HLB_hpgp_get_hfid_req_t *hfid_req,
													   void *msg, size_t *msg_len)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	HLB_hpgp_get_hfid_req_packed_type3_t payload;
	RES_result_t res;

	res = HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											  HLB_PROTOCOL_MSG_ID_APCM_GET_HFID_REQ,
											  req_id, sizeof(payload), msg, msg_len);
	if (res == RES_RESULT_OK)
	{
		payload.req_type = ENUM_AS_U8(hfid_req->req_type);
		MEMCPY(payload.hfid.HFID, hfid_req->hfid.HFID, sizeof(payload.hfid.HFID));
		MEMCPY(hlb_packet->payload, &payload, sizeof(payload));
	}

	return res;
}

static RES_result_t HLB_apcm_get_hfid_type4_req_create(const uint8_t *dest_mac_addr,
													   const uint8_t *src_mac_addr, HLB_req_id_t req_id,
													   const HLB_hpgp_get_hfid_req_t *hfid_req,
													   void *msg, size_t *msg_len)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	HLB_hpgp_get_hfid_req_packed_type4_t payload;
	RES_result_t res;

	res = HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											  HLB_PROTOCOL_MSG_ID_APCM_GET_HFID_REQ,
											  req_id, sizeof(payload), msg, msg_len);
	if (res == RES_RESULT_OK)
	{
		payload.req_type = ENUM_AS_U8(hfid_req->req_type);
		MEMCPY(payload.nid.NID, hfid_req->nid.NID, sizeof(payload.nid.NID));
		MEMCPY(payload.hfid.HFID, hfid_req->hfid.HFID, sizeof(payload.hfid.HFID));
		MEMCPY(hlb_packet->payload, &payload, sizeof(payload));
	}

	return res;
}

RES_result_t HLB_apcm_get_hfid_req_create(const uint8_t *dest_mac_addr,
										  const uint8_t *src_mac_addr, HLB_req_id_t req_id,
										  const HLB_hpgp_get_hfid_req_t *hfid_req,
										  void *msg, size_t *msg_len)
{
	RES_result_t res;

	if ((hfid_req == NULL) ||
		(HLB_is_invalid(hfid_req->req_type,
						HLB_HPGP_HFID_REQ_TYPE_GET_MANUF_SET_HFID,
						HLB_HPGP_HFID_REQ_TYPE_SET_NETWORK_HFID) &&
		 (hfid_req->req_type != HLB_HPGP_HFID_REQ_TYPE_GET_FAILURE)))
	{
		res = RES_RESULT_BAD_PARAMETER;
	}
	else if ((hfid_req->req_type == HLB_HPGP_HFID_REQ_TYPE_GET_MANUF_SET_HFID) ||
			 (hfid_req->req_type == HLB_HPGP_HFID_REQ_TYPE_GET_USER_SET_HFID))
	{
		res = HLB_apcm_get_hfid_type1_req_create(dest_mac_addr, src_mac_addr,
												 req_id, hfid_req, msg, msg_len);
	}
	else if (hfid_req->req_type == HLB_HPGP_HFID_REQ_TYPE_GET_NETWORK_HFID)
	{
		res = HLB_apcm_get_hfid_type2_req_create(dest_mac_addr, src_mac_addr,
												 req_id, hfid_req, msg, msg_len);
	}
	else if (hfid_req->req_type == HLB_HPGP_HFID_REQ_TYPE_SET_USER_SET_HFID)
	{
		res = HLB_apcm_get_hfid_type3_req_create(dest_mac_addr, src_mac_addr,
												 req_id, hfid_req, msg, msg_len);
	}
	else
	{
		res = HLB_apcm_get_hfid_type4_req_create(dest_mac_addr, src_mac_addr,
												 req_id, hfid_req, msg, msg_len);
	}

	return res;
}

RES_result_t HLB_apcm_get_hfid_cnf_parse(const void *msg,
										 HLB_hpgp_get_hfid_cnf_t *hfid_cnf)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_hpgp_get_hfid_cnf_packed_t *payload;
	RES_result_t res;

	if (hfid_cnf == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_APCM_GET_HFID_CNF);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_hpgp_get_hfid_cnf_packed_t *)hlb_packet->payload;

		if (U8_NOT_BETWEEN(payload->req_type, HLB_HPGP_HFID_REQ_TYPE_GET_MANUF_SET_HFID,
											  HLB_HPGP_HFID_REQ_TYPE_SET_NETWORK_HFID) &&
			(payload->req_type != HLB_HPGP_HFID_REQ_TYPE_GET_FAILURE))
		{
			res = RES_RESULT_INVALID_FW_PCKT;
		}
		else
		{
			hfid_cnf->req_type = payload->req_type;
			HLB_parse_hfid(&hfid_cnf->hfid, &payload->hfid);
		}
	}

	return res;
}

RES_result_t HLB_apcm_set_hfid_req_create(const uint8_t *dest_mac_addr,
										  const uint8_t *src_mac_addr, HLB_req_id_t req_id,
										  const HLB_hpgp_hfid_t *hfid,
										  void *msg, size_t *msg_len)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	HLB_hpgp_hfid_packed_t payload;
	RES_result_t res;

	if (hfid == NULL)
	{
		res = RES_RESULT_BAD_PARAMETER;
	}
	else
	{
		res = HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
												  HLB_PROTOCOL_MSG_ID_APCM_SET_HFID_REQ,
												  req_id, sizeof(payload), msg, msg_len);
	}

	if (res == RES_RESULT_OK)
	{
		MEMCPY(payload.HFID, hfid->HFID, sizeof(payload.HFID));
		MEMCPY(hlb_packet->payload, &payload, sizeof(payload));
	}

	return res;
}

RES_result_t HLB_apcm_set_hfid_cnf_parse(const void *msg,
										 HLB_hpgp_result_t *result)
{
	return HLB_apcm_cnf_parse_hpgp_status(msg, result, HLB_PROTOCOL_MSG_ID_APCM_SET_HFID_CNF);
}

RES_result_t HLB_apcm_set_hd_duration_req_create(const uint8_t *dest_mac_addr,
												 const uint8_t *src_mac_addr, HLB_req_id_t req_id,
												 const uint8_t hd_duration,
												 void *msg, size_t *msg_len)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	HLB_hpgp_set_hd_duration_req_packed_t payload;
	RES_result_t res;

	if (hd_duration > HD_DURATION_MAX_SECONDS)
	{
		res = RES_RESULT_BAD_PARAMETER;
	}
	else
	{
		res = HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
												  HLB_PROTOCOL_MSG_ID_APCM_SET_HD_DURATION_REQ,
												  req_id, sizeof(payload), msg, msg_len);
	}

	if (res == RES_RESULT_OK)
	{
		payload.hd_duration = hd_duration;
		MEMCPY(hlb_packet->payload, &payload, sizeof(payload));
	}

	return res;
}

RES_result_t HLB_apcm_set_hd_duration_cnf_parse(const void *msg,
												HLB_hpgp_result_t *result)
{
	return HLB_apcm_cnf_parse_hpgp_status(msg, result, HLB_PROTOCOL_MSG_ID_APCM_SET_HD_DURATION_CNF);
}

RES_result_t HLB_apcm_unassociated_sta_ind_parse(const void *msg,
												 HLB_hpgp_unassociated_sta_ind_t *sta_ind)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_hpgp_unassociated_sta_ind_packed_t *packed_sta_ind;
	RES_result_t res;

	if (sta_ind == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_APCM_UNASSOCIATED_STA_IND);
	}

	if (res == RES_RESULT_OK)
	{
		packed_sta_ind = (const HLB_hpgp_unassociated_sta_ind_packed_t *)hlb_packet->payload;

		if (U8_NOT_BETWEEN(packed_sta_ind->cco_cap, HLB_HPGP_CAP_CCO_NOT_SUPPORT_QOS_AND_TDMA, HLB_HPGP_CAP_CCO_FUTURE))
		{
			res = RES_RESULT_INVALID_FW_PCKT;
		}
		else
		{
			MEMCPY(sta_ind->nid.NID, packed_sta_ind->nid, sizeof(packed_sta_ind->nid));
			sta_ind->cco_cap = packed_sta_ind->cco_cap;
		}
	}

	return res;
}

RES_result_t HLB_apcm_set_ppkeys_req_create(const uint8_t *dest_mac_addr,
											const uint8_t *src_mac_addr, HLB_req_id_t req_id,
											const HLB_hpgp_set_ppkeys_req_t *ppkeys_req,
											void *msg, size_t *msg_len)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	HLB_hpgp_set_ppkeys_req_packed_t payload;
	RES_result_t res;

	/* TBD - check invalid key */
	if (ppkeys_req == NULL)
	{
		res = RES_RESULT_BAD_PARAMETER;
	}
	else
	{
		res = HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
												  HLB_PROTOCOL_MSG_ID_APCM_SET_PPKEYS_REQ,
												  req_id, sizeof(payload), msg, msg_len);
	}

	if (res == RES_RESULT_OK)
	{
		MEMCPY(payload.pp_eks, ppkeys_req->pp_eks, sizeof(payload.pp_eks));
		MEMCPY(payload.ppek, ppkeys_req->ppek, sizeof(payload.ppek));
		MEMCPY(payload.other_mac_addr.macAddress, ppkeys_req->other_mac_addr, sizeof(payload.other_mac_addr));
		MEMCPY(hlb_packet->payload, &payload, sizeof(payload));
	}

	return res;
}

RES_result_t HLB_apcm_set_ppkeys_cnf_parse(const void *msg,
										   HLB_hpgp_result_t *result)
{
	return HLB_apcm_cnf_parse_hpgp_status(msg, result, HLB_PROTOCOL_MSG_ID_APCM_SET_PPKEYS_CNF);
}

RES_result_t HLB_apcm_conf_slac_req_create(const uint8_t *dest_mac_addr,
										   const uint8_t *src_mac_addr, HLB_req_id_t req_id,
										   const HLB_hpgp_slac_conf_t slac_conf,
										   void *msg, size_t *msg_len)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	HLB_hpgp_conf_slac_req_packed_t payload;
	RES_result_t res;

	if (HLB_is_invalid(slac_conf, HLB_HPGP_SLAC_CONF_DISABLE_SLAC,
					   HLB_HPGP_SLAC_CONF_ENABLE_SLAC_RECEIVER_AND_DISABLE_TRANSMITTER))
	{
		res = RES_RESULT_BAD_PARAMETER;
	}
	else
	{
		res = HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
												  HLB_PROTOCOL_MSG_ID_APCM_CONF_SLAC_REQ,
												  req_id, sizeof(payload), msg, msg_len);
	}

	if (res == RES_RESULT_OK)
	{
		payload.slac_conf = ENUM_AS_U8(slac_conf);
		MEMCPY(hlb_packet->payload, &payload, sizeof(payload));
	}

	return res;
}

RES_result_t HLB_apcm_conf_slac_cnf_parse(const void *msg,
										  HLB_hpgp_result_t *result)
{
	return HLB_apcm_cnf_parse_hpgp_status(msg, result, HLB_PROTOCOL_MSG_ID_APCM_CONF_SLAC_CNF);
}

RES_result_t HLB_nscm_get_fw_version_req_create(const uint8_t *dest_mac_addr,
											 const uint8_t *src_mac_addr, HLB_req_id_t req_id,
											 void *msg, size_t *msg_len)
{
	return HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											   HLB_PROTOCOL_MSG_ID_NSCM_GET_VERSION_REQ,
											   req_id, 0, msg, msg_len);
}

RES_result_t HLB_nscm_get_fw_version_cnf_parse(const void *msg,
											   HLB_fw_version_t *fw_version)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_fw_version_packed_t *payload;
	RES_result_t res;

	if (fw_version == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_NSCM_GET_VERSION_CNF);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_fw_version_packed_t *)hlb_packet->payload;

		COPY_UNALIGNED_LE_16_BIT_TO_HOST(fw_version->fw_version_build, payload->fw_version_build)
		COPY_UNALIGNED_LE_16_BIT_TO_HOST(fw_version->fw_version_major, payload->fw_version_major)
		COPY_UNALIGNED_LE_16_BIT_TO_HOST(fw_version->fw_version_minor, payload->fw_version_minor)
		COPY_UNALIGNED_LE_16_BIT_TO_HOST(fw_version->fw_version_patch_version, payload->fw_version_patch_version)
		COPY_UNALIGNED_LE_16_BIT_TO_HOST(fw_version->fw_version_sub, payload->fw_version_sub)
	}

	return res;
}

RES_result_t HLB_nscm_reset_device_req_create(const uint8_t *dest_mac_addr,
											  const uint8_t *src_mac_addr,
											  HLB_hpgp_reset_device_mode_t reset_mode,
											  void *msg, size_t *msg_len)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	HLB_reset_device_packed_t payload;
	RES_result_t res;

	if (HLB_is_invalid(reset_mode,
					   HLB_HPGP_RESET_DEVICE_MODE_NORMAL,
					   HLB_HPGP_RESET_DEVICE_MODE_STOP_IN_HOSTBOOT))
	{
		res = RES_RESULT_BAD_PARAMETER;
	}
	else
	{
		res = HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
												  HLB_PROTOCOL_MSG_ID_NSCM_RESET_DEVICE_REQ,
												  RESET_DEVICE_REQ_ID, sizeof(payload), msg, msg_len);
	}

	if (res == RES_RESULT_OK)
	{
		payload.stop_in_hostboot = ENUM_AS_U8(reset_mode);
		MEMCPY(hlb_packet->payload, &payload, sizeof(payload));
	}

	return res;
}

RES_result_t HLB_nscm_get_ce2_info_cnf_parse(const void *msg, HLB_ce2_info_t *ce2_info)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_ce2_info_packed_t *payload;
	uint32_t tmp_u32bit;
	int32_t tmp_i32bit;
	RES_result_t res;

	if (ce2_info == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_NSCM_GET_CE2_INFO_CNF);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_ce2_info_packed_t *)hlb_packet->payload;

		COPY_UNALIGNED_LE_32_BIT_TO_HOST(tmp_u32bit, payload->status)
		ce2_info->status = tmp_u32bit;
		if (ce2_info->status > HLB_HPGP_RESULT_FAILURE)
		{
			res = RES_RESULT_INVALID_FW_PCKT;
		}
		else
		{
			COPY_UNALIGNED_LE_32_BIT_TO_HOST(tmp_i32bit, payload->gain)
			ce2_info->gain = (int8_t)tmp_i32bit;
			COPY_UNALIGNED_LE_32_BIT_TO_HOST(tmp_i32bit, payload->ce2_shift)
			ce2_info->ce2_shift = tmp_i32bit;
			COPY_UNALIGNED_LE_32_BIT_TO_HOST(ce2_info->total_buffer_size, payload->total_buffer_size)
		}
	}

	return res;
}

RES_result_t HLB_nscm_get_ce2_info_req_create(const uint8_t *dest_mac_addr,
											 const uint8_t *src_mac_addr,
											 HLB_req_id_t req_id,
											 void *msg, size_t *msg_len)
{
	return HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											   HLB_PROTOCOL_MSG_ID_NSCM_GET_CE2_INFO_REQ,
											   req_id, 0, msg, msg_len);
}

RES_result_t HLB_nscm_get_ce2_data_req_create(const uint8_t *dest_mac_addr,
										 const uint8_t *src_mac_addr,
										 const HLB_ce2_data_req_t *data_req,
										 HLB_req_id_t req_id, void *msg, size_t *msg_len)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	HLB_ce2_data_req_packed_t payload;
	RES_result_t res;

	if ((data_req == NULL) ||
		(data_req->chunk_number > HLB_MAX_CE2_CHUNK_NUM))
	{
		res = RES_RESULT_BAD_PARAMETER;
	}
	else
	{
		res = HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
												  HLB_PROTOCOL_MSG_ID_NSCM_GET_CE2_DATA_REQ,
												  req_id, sizeof(payload), msg, msg_len);
	}

	if (res == RES_RESULT_OK)
	{
		COPY_HOST_32_BIT_TO_LE_UNALIGNED(payload.chunk_number, data_req->chunk_number)
		MEMCPY(hlb_packet->payload, &payload, sizeof(payload));
	}

	return res;
}

RES_result_t HLB_nscm_get_ce2_data_cnf_parse(const void *msg, HLB_ce2_data_cnf_t *ce2_data)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_ce2_data_cnf_packed_t *payload;
	uint32_t size;
	uint32_t tmp_u32bit;
	uint16_t i;
	RES_result_t res;

	if (ce2_data == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_NSCM_GET_CE2_DATA_CNF);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_ce2_data_cnf_packed_t *)hlb_packet->payload;

		COPY_UNALIGNED_LE_32_BIT_TO_HOST(size, payload->size)
		if ((size > sizeof(ce2_data->data)) || ((size % 4U) != 0U))
		{
			res = RES_RESULT_INVALID_FW_PCKT;
		}
		else
		{
			COPY_UNALIGNED_LE_32_BIT_TO_HOST(tmp_u32bit, payload->status)
			ce2_data->status = tmp_u32bit;
			ce2_data->size = (size / 4U);
			for (i = 0; i < ce2_data->size; i++)
			{
				COPY_UNALIGNED_LE_32_BIT_TO_HOST(ce2_data->data[i], payload->data[i * 4U])
			}
		}
	}

	return res;
}

RES_result_t HLB_nscm_get_lnoe_cnf_parse(const void *msg, HLB_lnoe_info_t *lnoe_info)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_lnoe_info_packed_t *payload;
	uint32_t tmp_u32bit;
	RES_result_t res;

	if (lnoe_info == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_NSCM_GET_LNOE_CNF);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_lnoe_info_packed_t *)hlb_packet->payload;

		COPY_UNALIGNED_LE_32_BIT_TO_HOST(tmp_u32bit, payload->status)
		lnoe_info->status = tmp_u32bit;
		if (lnoe_info->status > HLB_HPGP_RESULT_FAILURE)
		{
			res = RES_RESULT_INVALID_FW_PCKT;
		}
		else
		{
			COPY_UNALIGNED_LE_32_BIT_TO_HOST(tmp_u32bit, payload->gain)
			lnoe_info->gain = tmp_u32bit;
			MEMCPY(&lnoe_info->lnoe, &payload->data, sizeof(lnoe_info->lnoe));
		}
	}

	return res;
}

RES_result_t HLB_nscm_get_lnoe_req_create(const uint8_t *dest_mac_addr,
											 const uint8_t *src_mac_addr,
											 HLB_req_id_t req_id,
											 void *msg, size_t *msg_len)
{
	return HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											   HLB_PROTOCOL_MSG_ID_NSCM_GET_LNOE_REQ,
											   req_id, 0, msg, msg_len);
}

RES_result_t HLB_nscm_get_snre_cnf_parse(const void *msg, HLB_snre_info_t *snre_info)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_snre_info_packed_t *payload;
	uint32_t tmp_u32bit, i;
	RES_result_t res;

	if (snre_info == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_NSCM_GET_SNRE_CNF);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_snre_info_packed_t *)hlb_packet->payload;

		COPY_UNALIGNED_LE_32_BIT_TO_HOST(tmp_u32bit, payload->status)
		snre_info->status = tmp_u32bit;
		if (snre_info->status > HLB_HPGP_RESULT_FAILURE)
		{
			res = RES_RESULT_INVALID_FW_PCKT;
		}
		else
		{
			for (i = 0U; i < SNRE_DATA_LEN; i++)
			{
				COPY_UNALIGNED_LE_16_BIT_TO_HOST(snre_info->snre[i], payload->snre[i * 2U])
			}
		}
	}

	return res;
}

RES_result_t HLB_nscm_get_snre_req_create(const uint8_t *dest_mac_addr,
											 const uint8_t *src_mac_addr, HLB_req_id_t req_id,
											 void *msg, size_t *msg_len)
{
	return HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											   HLB_PROTOCOL_MSG_ID_NSCM_GET_SNRE_REQ,
											   req_id, 0, msg, msg_len);
}

RES_result_t HLB_nscm_abort_dump_action_req_create(const uint8_t *dest_mac_addr,
											 		const uint8_t *src_mac_addr, HLB_req_id_t req_id,
											 		void *msg, size_t *msg_len)
{
	return HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											   HLB_PROTOCOL_MSG_ID_NSCM_ABORT_DUMP_ACTION_REQ,
											   req_id, 0, msg, msg_len);
}

static RES_result_t HLB_nscm_cnf_parse_hpgp_status(const void *msg,
												   HLB_hpgp_result_t *result,
												   HLB_protocol_msg_id_t expected_msg_id)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_32b_status_cnf_packed_t *payload;
	uint32_t tmp_u32bit;
	RES_result_t res;

	if (result == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, expected_msg_id);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_32b_status_cnf_packed_t *)hlb_packet->payload;
		COPY_UNALIGNED_LE_32_BIT_TO_HOST(tmp_u32bit, payload->status)
		*result = tmp_u32bit;
	}

	return res;
}

RES_result_t HLB_nscm_abort_dump_action_cnf_parse(const void *msg,
												  HLB_hpgp_result_t *result)
{
	return HLB_nscm_cnf_parse_hpgp_status(msg, result, HLB_PROTOCOL_MSG_ID_NSCM_ABORT_DUMP_ACTION_CNF);
}

RES_result_t HLB_nscm_enter_phy_mode_req_create(const uint8_t *dest_mac_addr,
											  	const uint8_t *src_mac_addr,
											  	HLB_req_id_t req_id,
											  	void *msg, size_t *msg_len)
{
	return HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											   HLB_PROTOCOL_MSG_ID_NSCM_ENTER_PHY_MODE_REQ,
											   req_id, 0, msg, msg_len);
}

RES_result_t HLB_nscm_enter_phy_mode_cnf_parse(const void *msg,
											   HLB_hpgp_result_t *result)
{
	return HLB_nscm_cnf_parse_hpgp_status(msg, result, HLB_PROTOCOL_MSG_ID_NSCM_ENTER_PHY_MODE_CNF);
}

RES_result_t HLB_nscm_read_mem_req_create(const uint8_t *dest_mac_addr,
										 const uint8_t *src_mac_addr,
										 const HLB_read_mem_req_t *read_mem_req,
										 HLB_req_id_t req_id, void *msg, size_t *msg_len)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	HLB_read_mem_req_packed_t payload;
	RES_result_t res;

	if ((read_mem_req == NULL) ||
		(read_mem_req->size > HLB_READ_MEM_MAX_LEN))
	{
		res = RES_RESULT_BAD_PARAMETER;
	}
	else
	{
		res = HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
												  HLB_PROTOCOL_MSG_ID_NSCM_READ_MEM_REQ,
												  req_id, sizeof(payload), msg, msg_len);
	}

	if (res == RES_RESULT_OK)
	{
		COPY_HOST_32_BIT_TO_LE_UNALIGNED(payload.address, read_mem_req->address)
		COPY_HOST_32_BIT_TO_LE_UNALIGNED(payload.size, read_mem_req->size)
		MEMCPY(hlb_packet->payload, &payload, sizeof(payload));
	}

	return res;
}

RES_result_t HLB_nscm_read_mem_cnf_parse(const void *msg, HLB_read_mem_cnf_t *read_mem_cnf)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_read_mem_cnf_packed_t *payload;
	RES_result_t res;

	if (read_mem_cnf == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_NSCM_READ_MEM_CNF);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_read_mem_cnf_packed_t *)hlb_packet->payload;

		COPY_UNALIGNED_LE_32_BIT_TO_HOST(read_mem_cnf->size, payload->size)
		MEMCPY(&read_mem_cnf->data, &payload->data, read_mem_cnf->size);
	}

	return res;
}

RES_result_t HLB_nscm_write_mem_req_create(const uint8_t *dest_mac_addr,
										 	const uint8_t *src_mac_addr,
										 	const HLB_write_mem_req_t *write_mem_req,
										 	HLB_req_id_t req_id, void *msg, size_t *msg_len)
{
	HLB_packet_t *hlb_packet = (HLB_packet_t *)msg;
	HLB_write_mem_req_packed_t *payload = (HLB_write_mem_req_packed_t *)(hlb_packet->payload);
	uint32_t payload_size;
	RES_result_t res;

	if ((write_mem_req == NULL) ||
		(write_mem_req->size > HLB_READ_MEM_MAX_LEN))
	{
		res = RES_RESULT_BAD_PARAMETER;
	}
	else
	{
		payload_size = sizeof(payload->size) + sizeof(payload->address) + write_mem_req->size;
		res = HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
												  HLB_PROTOCOL_MSG_ID_NSCM_WRITE_MEM_REQ,
												  req_id, payload_size, msg, msg_len);
	}

	if (res == RES_RESULT_OK)
	{
		COPY_HOST_32_BIT_TO_LE_UNALIGNED(payload->address, write_mem_req->address)
		COPY_HOST_32_BIT_TO_LE_UNALIGNED(payload->size, write_mem_req->size)
		MEMCPY(payload->data, write_mem_req->data, write_mem_req->size);
	}

	return res;
}

RES_result_t HLB_nscm_write_mem_cnf_parse(const void *msg,
										  HLB_hpgp_result_t *result)
{
	return HLB_nscm_cnf_parse_hpgp_status(msg, result, HLB_PROTOCOL_MSG_ID_NSCM_WRITE_MEM_CNF);
}

RES_result_t HLB_nscm_get_dc_calib_req_create(const uint8_t *dest_mac_addr,
											  	const uint8_t *src_mac_addr,
											  	HLB_req_id_t req_id,
											  	void *msg, size_t *msg_len)
{
	return HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											   HLB_PROTOCOL_MSG_ID_NSCM_GET_DC_CALIB_REQ,
											   req_id, 0, msg, msg_len);
}

RES_result_t HLB_nscm_get_dc_calib_cnf_parse(const void *msg, HLB_dc_calib_cnf_t *dc_calib_cnf)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_dc_calib_cnf_packed_t *payload;
	RES_result_t res;

	if (dc_calib_cnf == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_NSCM_GET_DC_CALIB_CNF);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_dc_calib_cnf_packed_t *)hlb_packet->payload;

		dc_calib_cnf->dc_offset_hi_ch1 = payload->dc_offset_hi_ch1;
		dc_calib_cnf->dc_offset_lo_ch1 = payload->dc_offset_lo_ch1;
	}

	return res;
}

RES_result_t HLB_nscm_get_device_state_req_create(const uint8_t *dest_mac_addr,
											  		const uint8_t *src_mac_addr,
											  		HLB_req_id_t req_id,
											  		void *msg, size_t *msg_len)
{
	return HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											   HLB_PROTOCOL_MSG_ID_NSCM_GET_DEVICE_STATE_REQ,
											   req_id, 0, msg, msg_len);
}

RES_result_t HLB_nscm_get_device_state_cnf_parse(const void *msg, HLB_hpgp_device_state_cnf_t *device_state)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_device_state_cnf_packed_t *payload;
	RES_result_t res;

	if (device_state == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_NSCM_GET_DEVICE_STATE_CNF);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_device_state_cnf_packed_t *)hlb_packet->payload;

		device_state->device_status = payload->device_state;
	}

	return res;
}

RES_result_t HLB_nscm_get_d_link_status_req_create(const uint8_t *dest_mac_addr,
											  		const uint8_t *src_mac_addr,
											  		HLB_req_id_t req_id,
											  		void *msg, size_t *msg_len)
{
	return HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											   HLB_PROTOCOL_MSG_ID_NSCM_GET_D_LINK_STATUS_REQ,
											   req_id, 0, msg, msg_len);
}

RES_result_t HLB_nscm_get_d_link_status_cnf_parse(const void *msg, HLB_hpgp_d_link_status_cnf_t *d_link_status)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_hpgp_d_link_status_packed_cnf_t *payload;
	RES_result_t res;

	if (d_link_status == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_NSCM_GET_D_LINK_STATUS_CNF);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_hpgp_d_link_status_packed_cnf_t *)hlb_packet->payload;

		d_link_status->link_status = payload->d_link_state;
	}

	return res;
}

RES_result_t HLB_nscm_host_message_status_ind_parse(const void *msg,
												 HLB_hpgp_host_message_status_ind_t *host_message_status_ind)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_hpgp_host_message_status_ind_packed_t *packed_host_message_status;
	RES_result_t res;

	if (host_message_status_ind == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_NSCM_VS_HOST_MESSAGE_STATUS_IND);
	}

	if (res == RES_RESULT_OK)
	{
		packed_host_message_status = (const HLB_hpgp_host_message_status_ind_packed_t *)hlb_packet->payload;
		if (U8_NOT_BETWEEN(packed_host_message_status->host_message_status, HLB_HPGP_HOST_MESSAGE_STATUS_READY_TO_JOIN, HLB_HPGP_HOST_MESSAGE_STATUS_DISCONNECTED_FROM_AVLN))
		{
			res = RES_RESULT_INVALID_FW_PCKT;
		}
		else
		{
			host_message_status_ind->host_message_status = packed_host_message_status->host_message_status;
		}
	}

	return res;
}

RES_result_t HLB_nscm_d_link_ready_ind_parse(const void *msg,
												HLB_hpgp_d_link_ready_status_ind_t *d_link)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	HLB_hpgp_d_link_ready_ind_packed_t packed_d_link_ready;
	RES_result_t res;

	if (d_link == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_NSCM_D_LINK_READY_IND);
	}

	if (res == RES_RESULT_OK)
	{
		/* This message doesn't contain the Lumissil header, so its payload starts right after the management header.
		 * This was done to preserve backwards compatibility with vector */
		MEMCPY(&packed_d_link_ready, &hlb_packet->vendor_header, sizeof(HLB_hpgp_d_link_ready_ind_packed_t));
		if (U8_NOT_BETWEEN(packed_d_link_ready.d_link_ready_status, HLB_HPGP_D_LINK_NO_LINK, HLB_HPGP_D_LINK_LINK_ESTABLISHED))
		{
			res = RES_RESULT_INVALID_FW_PCKT;
		}
		else
		{
			d_link->d_link_ready_status = packed_d_link_ready.d_link_ready_status;
		}
	}

	return res;
}

RES_result_t HLB_nscm_d_link_terminate_req_create(const uint8_t *dest_mac_addr,
											  		const uint8_t *src_mac_addr,
											  		HLB_req_id_t req_id,
											  		void *msg, size_t *msg_len)
{
	return HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											   HLB_PROTOCOL_MSG_ID_NSCM_D_LINK_TERMINATE_REQ,
											   req_id, 0, msg, msg_len);
}

RES_result_t HLB_nscm_device_info_req_create(const uint8_t *dest_mac_addr,
											 const uint8_t *src_mac_addr,
											 HLB_req_id_t req_id,
											 void *msg, size_t *msg_len)
{
	return HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											   HLB_PROTOCOL_MSG_ID_NSCM_DEVICE_INFO_REQ,
											   req_id, 0, msg, msg_len);
}

RES_result_t HLB_nscm_device_info_cnf_parse(const void *msg,
											HLB_device_info_t *device_info)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_hpgp_device_info_packed_t *payload;
	RES_result_t res;

	if (device_info == NULL)
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_NSCM_DEVICE_INFO_CNF);
	}

	if (res == RES_RESULT_OK)
	{
		payload = (const HLB_hpgp_device_info_packed_t *)hlb_packet->payload;

		/* Bits[1:0] - CCo role */
		device_info->cco_mode = (payload->connectivity_config & 0x3U);
		/* Bits[3:2] - Host Interface */
		device_info->host_iface = (payload->connectivity_config & 0xCU) >> 2U;
		device_info->terminal_equipment_id = payload->terminal_equipment_id;
		MEMCPY(device_info->mac_addr, &payload->mac_addr, sizeof(device_info->mac_addr));
		MEMCPY(&device_info->nmk, &payload->nmk, sizeof(device_info->nmk));
		MEMCPY(&device_info->nid, &payload->nid, sizeof(device_info->nid));
		device_info->security_level = payload->security_level;
		device_info->snid = payload->snid;
		device_info->max_receiver_sensitivity = payload->max_receiver_sensitivity;
		device_info->plc_freq_sel = payload->plc_freq_sel;

		HLB_parse_hfid(&device_info->manufacturer_hfid, &payload->manufacturer_hfid);
		HLB_parse_hfid(&device_info->user_hfid, &payload->user_hfid);
		HLB_parse_hfid(&device_info->avln_hfid, &payload->avln_hfid);
	}

	return res;
}

RES_result_t HLB_nscm_get_amp_map_req_create(const uint8_t *dest_mac_addr,
											 const uint8_t *src_mac_addr,
											 HLB_req_id_t req_id,
											 void *msg, size_t *msg_len)
{
	return HLB_req_validate_and_create_headers(dest_mac_addr, src_mac_addr,
											   HLB_PROTOCOL_MSG_ID_NSCM_GET_AMP_MAP_REQ,
											   req_id, 0, msg, msg_len);
}

RES_result_t HLB_nscm_get_amp_map_cnf_parse(const void *msg, size_t msg_len,
											HLB_get_amp_map_t *amp_map,
											uint32_t parse_at, uint16_t *parsed_amount)
{
	const HLB_packet_t *hlb_packet = (const HLB_packet_t *)msg;
	const HLB_fragmented_packet_t *hlb_frag_packet = NULL;
	const HLB_hpgp_get_amp_map_packed_t *amp_map_payload;
	uint8_t *amdata_ptr;
	RES_result_t res = RES_RESULT_OK;

	if ((amp_map == NULL) || (parsed_amount == NULL))
	{
		res = RES_RESULT_NULL_PTR;
	}
	else
	{
		res = HLB_cnf_validate_msg(msg, HLB_PROTOCOL_MSG_ID_NSCM_GET_AMP_MAP_CNF);
	}

	if (res == RES_RESULT_OK)
	{
		if (HLB_PACKET_GET_FRAG_IDX(hlb_packet) != 0U)
		{
			msg_len -= HLB_FRAG_PACKET_HEADERS_SIZE;
			hlb_frag_packet = (const HLB_fragmented_packet_t *)msg;
			amp_map_payload = (const HLB_hpgp_get_amp_map_packed_t *)&hlb_frag_packet->payload;
		}
		else
		{
			msg_len -= HLB_PACKET_HEADERS_SIZE;
			amp_map_payload = (const HLB_hpgp_get_amp_map_packed_t *)&hlb_packet->payload;
		}

		if ((sizeof(amp_map->amdata) - parse_at) >= msg_len)
		{
			amdata_ptr = (uint8_t*)&amp_map->amdata;
			MEMCPY(&amdata_ptr[parse_at], amp_map_payload, msg_len);
			*parsed_amount = (uint16_t)msg_len;
		}
		else
		{
			res = RES_RESULT_INVALID_FW_PCKT;
		}
	}

	return res;
}
