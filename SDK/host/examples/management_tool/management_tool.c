/********************************************************************
*
* Module Name: management_tool
* Design:
* This application is written above APCM/NSCM APIs, and provides
* a selection of useful features to be used during the evaluation phase.
*
*
********************************************************************/
/*******************************************************************
* IMPORTS
********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>

#include "HLB_protocol.h"
#include "version.h"
#include "HLB_host.h"
#include "HLB_apcm.h"
#include "HLB_nscm.h"
#include "osal.h"
#include "HLB_noise_floor.h"

/*******************************************************************
* CONSTANTS
********************************************************************/
#define ALLOCATED_AREA_SIZE 65536
#define MAX_LINE_LEN 32
#define SNRE_BUF_SIZE SNRE_DATA_LEN * MAX_LINE_LEN
#define LNOE_BUF_SIZE (LNOE_DATA_LEN + 1) * MAX_LINE_LEN
#define CE2_ONE_DATA_BUF_SIZE CE2_DATA_LEN * MAX_LINE_LEN
#define CE2_TOTAL_BUF_SIZE (6*CE2_ONE_DATA_BUF_SIZE) + MAX_LINE_LEN
#define OUT_BUFF_MAX_SIZE (8192 * 8)
#define DEFAULT_TIMEOUT_USEC 1000000 /* TBD: change it according to FW spec */
#define HW_APIS_TIMEOUT_USEC 7000000
#define MAX_ADDRESS_COLONS_NUM 5
#define MAC_ADDR_LEN sizeof(mac_address_t) * 2 + MAX_ADDRESS_COLONS_NUM
#define KB 1024
#define CE2_TOTAL_BUFFER_SIZE 8 * KB
#define MAX_CLI_LEN 500
#define TIME_DOMAIN_NORMALISER (2048)

#define ETH_IFACE_NAME "eth0"
#define SPI_IFACE_NAME "seth0"
/*******************************************************************
* TYPES
********************************************************************/
/*******************************************************************
* STATIC and GLOBAL DATA
********************************************************************/
typedef struct
{
	uint8_t map[256];
	void *lock;
} req_id_map_t;

comm_handle_t handle = NULL;
req_id_map_t rx_cal[HLB_HOST_MSG_ID_LAST] = {0};

char out_buf[OUT_BUFF_MAX_SIZE];
unsigned int cur_loc_out_buffer = 0;

/*******************************************************************
* MACROS
********************************************************************/
#define MAC_ADDR_FMT "%02X:%02X:%02X:%02X:%02X:%02X"
#define NID_FMT		 "%02X%02X%02X%02X%02X%02X%02X"
#define NMK_FMT		 "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X"

#define MAC_ADDR_FMT_ARGS(x)	x[0], x[1], x[2], x[3], x[4], x[5]
#define NID_FMT_ARGS(x)			x[0], x[1], x[2], x[3], x[4], x[5], x[6]
#define NMK_FMT_ARGS(x)			x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7], x[8], x[9], x[10], x[11], x[12], x[13], x[14], x[15]


/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/
void append_to_buffer(const char *message, ...)
{
	va_list args;
	int ret;
	va_start(args, message);
	ret = vsnprintf(out_buf + cur_loc_out_buffer, sizeof(out_buf) - cur_loc_out_buffer, message, args);
	if (ret <= 0)
	{
		printf("can't print, vsnprintf ret=%d\n", ret);
		return;
	}
	cur_loc_out_buffer += ret;
	va_end(args);
	ret = snprintf(out_buf + cur_loc_out_buffer, sizeof(out_buf) - cur_loc_out_buffer, "\n");
	cur_loc_out_buffer += ret;
}

static void str_to_mac(const char *xi_mac, mac_address_t *xo_mac)
{
	char *buffer;
	char *dig;
	int idx;

	/* First copy xi_mac to temp buffer */
	buffer = (char *)malloc(strlen(xi_mac) + 1);
	strcpy(buffer, xi_mac);

	dig = strtok(buffer, ":-");
	for (idx = 0; idx < 6; idx++)
	{
		if (dig)
		{
			(xo_mac[0])[idx] = (unsigned char)strtoul(dig, NULL, 16);
			dig = strtok(NULL, ":-");
		}
	}

	free(buffer);
}

static void inc_map(req_id_map_t *map, int type, int req_id)
{
	if ((type < 0) || (type >= HLB_HOST_MSG_ID_LAST))
	{
		printf("received unsupported msg type '%d'\n", type);
		return;
	}
	osal_lock_lock(map[type].lock);
	map[type].map[req_id]++;
	osal_lock_unlock(map[type].lock);
}

static void dec_map(req_id_map_t *map, int type, int req_id)
{
	if ((type < 0) || (type >= HLB_HOST_MSG_ID_LAST))
	{
		printf("received unsupported msg type '%d'\n", type);
		return;
	}
	osal_lock_lock(map[type].lock);
	map[type].map[req_id]--;
	osal_lock_unlock(map[type].lock);
}

static void init_map_locks(req_id_map_t *map)
{
	int i;
	for (i = 0; i < HLB_HOST_MSG_ID_LAST; i++)
	{
		map[i].lock = malloc(osal_lock_alloc_size());
		osal_lock_init(map[i].lock);
	}
}

static void destroy_map_locks(req_id_map_t *map)
{
	int i;
	for (i = 0; i < HLB_HOST_MSG_ID_LAST; i++)
	{
		osal_lock_destroy(map[i].lock);
		free(map[i].lock);
	}
}

static void rx_callback(HLB_host_msg_id type, HLB_req_id_t req_id)
{
	//printf("received rx ind type %d, req_id=%u\n", (int)type, req_id);
	inc_map(rx_cal,type,req_id);
	
}

static int write_output(char *output_filename, char *output)
{
	FILE *output_file;

	if(output_filename == NULL)
	{
		append_to_buffer(output);
	}
	else
	{
		output_file = fopen(output_filename, "a+");
		if (!output_file)
		{
			printf("Failed to open the output file %s\n", output_filename);
			return RES_RESULT_GENERAL_ERROR;
		}
		fputs(output, output_file);
		fclose(output_file);
	}
	return RES_RESULT_OK;
}

static size_t wait_for(req_id_map_t *map, HLB_req_id_t req_id, HLB_msg_id_t msg_id, size_t timeout, int print)
{
	size_t counter = 0;
	uint64_t start_time;
	struct timeval tv;

	gettimeofday(&tv, NULL);
	start_time = (tv.tv_sec * 1000000ll) + tv.tv_usec;
	while (map[msg_id].map[req_id] == 0)
	{
		gettimeofday(&tv, NULL);
		counter = (size_t)(((tv.tv_sec * 1000000ll) + tv.tv_usec) - start_time);
		if (counter >= timeout)
		{
			printf("received timeout(%zu) in req_id=%d, msg_id=%d\n", timeout, req_id, msg_id);
			return counter;
		}
		usleep(10);
	}

	dec_map(map,msg_id,req_id);
	
	if (print)
	{
		printf("waited %zu usec for req_id=%d, msg_id=%d\n", counter, req_id, msg_id);
	}

	return counter;
}

static inline bool is_hex_char(char c)
{
	return !((c < '0' || c > '9') && (c < 'a' || c > 'f') && (c < 'A' || c > 'F'));
}

static RES_result_t hlb_str_to_binary(char *xi_str, int xi_len, unsigned char *xo_buffer)
{
	int i, j;
	unsigned int val[1];

	for (i = 0, j = 0; j < xi_len; ++i, j += 2)
	{
		if ((xi_str[j] == 0) || (xi_str[j + 1] == 0))
		{
			break;
		}

		if (!is_hex_char(xi_str[j]) ||
			!is_hex_char(xi_str[j + 1]))
		{
			return RES_RESULT_BAD_PARAMETER;
		}

		sscanf(xi_str + j, "%2x", val);
		xo_buffer[i] = val[0];
	}

	return RES_RESULT_OK;
}

static int process_cmd_set_key(char *cmd, HLB_req_id_t req_id, char *output_filename)
{
	char *curr;
	char buf[OUT_BUFF_MAX_SIZE];
	HLB_hpgp_set_key_req_t set_key_req = { 0 };
	HLB_hpgp_result_t result;
	size_t timeout;
	RES_result_t res;

	curr = strstr(cmd, "NMK=");
	if (curr == NULL)
	{
		printf("missing NMK=\n");
		return RES_RESULT_BAD_PARAMETER;
	}

	curr += strlen("NMK=");

	if ((strlen(curr) < sizeof(set_key_req.nmk) * 2) ||
		(strstr(curr, " ") && ((uint32_t)(strstr(curr, " ") - curr)) != sizeof(set_key_req.nmk) * 2))
	{
		printf("NMK too short\n");
		return RES_RESULT_BAD_PARAMETER;
	}

	res = hlb_str_to_binary(curr, sizeof(set_key_req.nmk) * 2, set_key_req.nmk.NMK);
	if (res != RES_RESULT_OK)
	{
		printf("Failed to parse provided NMK string\n");
		return res;
	}

	if (strstr(cmd, "NID=") && strstr(cmd, "sec_level="))
	{
		printf("Only one of NID= and sec_level= can be provided\n");
		return RES_RESULT_BAD_PARAMETER;
	}
	else if ((curr = strstr(cmd, "NID=")))
	{
		curr += strlen("NID=");
		if ((strlen(curr) < sizeof(set_key_req.nid) * 2) ||
			(strstr(curr, " ") && ((uint32_t)(strstr(curr, " ") - curr)) != sizeof(set_key_req.nid) * 2))
		{
			printf("NID too short\n");
			return RES_RESULT_BAD_PARAMETER;
		}

		res = hlb_str_to_binary(curr, sizeof(set_key_req.nid) * 2, set_key_req.nid.NID);
		if (res != RES_RESULT_OK)
		{
			printf("Failed to parse provided NID string\n");
			return res;
		}
	}
	else if ((curr = strstr(cmd, "sec_level=")))
	{
		curr += strlen("sec_level=");
		set_key_req.security_level = atoi(curr);
		if ((set_key_req.security_level != HLB_HPGP_SECURITY_LEVEL_SC) &&
			(set_key_req.security_level != HLB_HPGP_SECURITY_LEVEL_HS))
		{
			printf("invalid security level : %d\n", set_key_req.security_level);
			return RES_RESULT_BAD_PARAMETER;
		}

		/* Set NID to invalid value --> indicates to FW that default NID should be used */
		memset(&set_key_req.nid, 0xFF, sizeof(set_key_req.nid));
	}
	else
	{
		printf("neither NID= nor sec_level= were provided\n");
		return RES_RESULT_BAD_PARAMETER;
	}

	res = HLB_apcm_set_key_req_send(handle, req_id, &set_key_req);
	if (res != RES_RESULT_OK)
	{
		printf("HLB_apcm_set_key_req_send error, res=%d\n", res);
		return res;
	}

	timeout = wait_for(rx_cal, req_id, HLB_HOST_MSG_ID_APCM_SET_KEY_CNF, DEFAULT_TIMEOUT_USEC, 0);
	if (timeout >= DEFAULT_TIMEOUT_USEC)
	{
		printf("Timeout while waiting for APCM_SET_KEY.CNF\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	res = HLB_apcm_set_key_cnf_receive(handle, req_id, &result);
	if (res != RES_RESULT_OK)
	{
		printf("HLB_apcm_set_key_cnf_receive error, res=%d\n", res);
		return res;
	}

	snprintf(buf, sizeof(buf), "set_key result:%u req_id=%u", result, req_id);
	return write_output(output_filename, buf);
}

static int process_cmd_reset_device(char *cmd)
{
	RES_result_t res;
	HLB_hpgp_reset_device_mode_t reset_mode = HLB_HPGP_RESET_DEVICE_MODE_NORMAL;
	char *curr;

	curr = strstr(cmd, "reset_mode=");
	if (curr != NULL)
	{
		curr += strlen("reset_mode=");
		reset_mode = atoi(curr);
	}

	res = HLB_nscm_reset_device_req_send(handle, reset_mode);
	if (res != RES_RESULT_OK)
	{
		printf("HLB_nscm_get_version_req_send error\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	return RES_RESULT_OK;
}

static int process_cmd_get_device_version(char *cmd, HLB_req_id_t req_id, char *output_filename)
{
	RES_result_t res;
	HLB_fw_version_t version;
	char buf[OUT_BUFF_MAX_SIZE];

	(void)cmd;

	res = HLB_nscm_get_fw_version_req_send(handle, req_id);
	if (res != RES_RESULT_OK)
	{
		printf("HLB_nscm_get_version_req_send error\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	res = wait_for(rx_cal, req_id, HLB_HOST_MSG_ID_NSCM_GET_VERSION_CNF, DEFAULT_TIMEOUT_USEC, 0);
	if (res >= DEFAULT_TIMEOUT_USEC)
	{
		printf("Timeout while waiting for NSCM_GET_VERSION.CNF\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	res = HLB_nscm_get_fw_version_cnf_receive(handle, req_id, &version);
	if (res != RES_RESULT_OK)
	{
		printf("HLB_nscm_get_version_cnf_receive error\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	res = snprintf(buf, sizeof(buf),
		"get_device_version fw_version_major=%u fw_version_minor=%u fw_version_sub=%u fw_version_build=%u fw_version_patch=%u"
						 " req_id=%u",
						 version.fw_version_major, version.fw_version_minor,
						 version.fw_version_sub, version.fw_version_build,
						 version.fw_version_patch_version, req_id);

	if(res < 0)
	{
		printf("process_cmd_get_device_version error, failed to parse "
				"get_device_version result\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	return write_output(output_filename, buf);
}

static const char* cco_mode_to_string(HLB_hpgp_cco_mode_t cco_mode)
{
	const char* res = NULL;

	switch(cco_mode)
	{
	case HLB_HPGP_CCO_MODE_AUTO:
		res = "auto";
		break;
	case HLB_HPGP_CCO_MODE_NEVER:
		res = "never";
		break;
	case HLB_HPGP_CCO_MODE_ALWAYS:
		res = "always";
		break;
	default:
		res = "unknown";
		break;
	}

	return res;
}

static const char* host_iface_to_string(HLB_hpgp_host_interface_t host_iface)
{
	const char* res = NULL;

	switch(host_iface)
	{
	case HLB_HPGP_HOST_INTERFACE_SPI:
		res = "SPI";
		break;
	case HLB_HPGP_HOST_INTERFACE_ETH:
		res = "ETH - R/MII";
		break;
	case HLB_HPGP_HOST_INTERFACE_SPI_DBG:
		res = "SPI Debug";
		break;
	default:
		res = "unknown";
		break;
	}

	return res;
}

static const char* security_level_to_string(HLB_hpgp_security_level_t security_level)
{
	const char* res = NULL;

	switch(security_level)
	{
	case HLB_HPGP_SECURITY_LEVEL_SC:
		res = "Simple connect";
		break;
	case HLB_HPGP_SECURITY_LEVEL_HS:
		res = "Secure Security";
		break;
	default:
		res = "unknown";
		break;
	}

	return res;
}

static const char* plc_freq_sel_to_string(HLB_hpgp_plc_freq_sel_t plc_freq_sel)
{
	const char* res = NULL;

	switch(plc_freq_sel)
	{
	case HLB_HPGP_PLC_FREQ_SELECTION_50_HZ:
		res = "50 Hz";
		break;
	case HLB_HPGP_PLC_FREQ_SELECTION_60_HZ:
		res = "60 Hz";
		break;
	case HLB_HPGP_PLC_FREQ_SELECTION_EXTERNAL_SIGNAL:
		res = "External Signal";
		break;
	default:
		res = "unknown";
		break;
	}

	return res;
}

static int process_cmd_device_info(char *cmd, HLB_req_id_t req_id, char *output_filename)
{
	RES_result_t res;
	int ret;
	HLB_device_info_t device_info;
	char buf[OUT_BUFF_MAX_SIZE];

	(void)cmd;

	res = HLB_nscm_device_info_req_send(handle, req_id);
	if (res != RES_RESULT_OK)
	{
		printf("HLB_nscm_device_info_req_send error\n");
	}

	if (res == RES_RESULT_OK)
	{
		ret = wait_for(rx_cal, req_id, HLB_HOST_MSG_ID_NSCM_DEVICE_INFO_CNF, DEFAULT_TIMEOUT_USEC, 0);
		if (ret >= DEFAULT_TIMEOUT_USEC)
		{
			printf("Timeout while waiting for NSCM_DEVICE_INFO.CNF\n");
			res = RES_RESULT_GENERAL_ERROR;
		}
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_device_info_cnf_receive(handle, req_id, &device_info);
		if (res != RES_RESULT_OK)
		{
			printf("HLB_nscm_device_info_cnf_receive error\n");
		}
	}

	if (res == RES_RESULT_OK)
	{
		ret = snprintf(buf, sizeof(buf),
					   "device_info:\n"
					   "\tcco mode                      = %hhu (%s)\n"
					   "\thost iface                    = %hhu (%s)\n"
					   "\tTerminal Equipment Identifier = %hhu\n"
					   "\tMAC address                   = " MAC_ADDR_FMT "\n"
					   "\tManufacturer HFID             = %s\n"
					   "\tUser HFID                     = %s\n"
					   "\tAVLN HFID                     = %s\n"
					   "\tNetwork Membership Key        = " NMK_FMT "\n"
					   "\tNetwork Identifier            = " NID_FMT "\n"
					   "\tsecurity level                = %hhu (%s)\n"
					   "\tSNID                          = 0x%02X\n"
					   "\tMax Receiver sensitivity      = %hhu dB\n"
					   "\tPLC Frequency selection       = %hhu (%s)\n",
					   device_info.cco_mode, cco_mode_to_string(device_info.cco_mode),
					   device_info.host_iface, host_iface_to_string(device_info.host_iface),
					   device_info.terminal_equipment_id,
					   MAC_ADDR_FMT_ARGS(device_info.mac_addr),
					   (const char*)device_info.manufacturer_hfid.HFID,
					   (const char*)device_info.user_hfid.HFID,
					   (const char*)device_info.avln_hfid.HFID,
					   NMK_FMT_ARGS(device_info.nmk.NMK),
					   NID_FMT_ARGS(device_info.nid.NID),
					   device_info.security_level, security_level_to_string(device_info.security_level),
					   device_info.snid,
					   device_info.max_receiver_sensitivity,
					   device_info.plc_freq_sel, plc_freq_sel_to_string(device_info.plc_freq_sel));

		if (ret < 0)
		{
			printf("process_cmd_device_info error, failed to parse device_info result\n");
			res = RES_RESULT_GENERAL_ERROR;
		}
		else
		{
			res = write_output(output_filename, buf);
		}
	}

	return res;
}

static int process_cmd_get_amp_map(char *cmd, HLB_req_id_t req_id, char *output_filename)
{
	RES_result_t res;
	int ret;
	HLB_get_amp_map_t get_amp_map;
	uint16_t *amp_map_curr;
	uint16_t *amp_map_end;
	uint32_t i;
	char buf[48];

	(void)cmd;

	res = HLB_nscm_get_amp_map_req_send(handle, req_id);
	if (res != RES_RESULT_OK)
	{
		printf("HLB_nscm_get_amp_map_req_send error\n");
	}

	if (res == RES_RESULT_OK)
	{
		ret = wait_for(rx_cal, req_id, HLB_HOST_MSG_ID_NSCM_GET_AMP_MAP_CNF, DEFAULT_TIMEOUT_USEC, 0);
		if (ret >= DEFAULT_TIMEOUT_USEC)
		{
			printf("Timeout while waiting for NSCM_GET_AMP_MAP.CNF\n");
			res = RES_RESULT_GENERAL_ERROR;
		}
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_get_amp_map_cnf_receive(handle, req_id, &get_amp_map);
		if (res != RES_RESULT_OK)
		{
			printf("HLB_nscm_get_amp_map_cnf_receive error\n");
		}
	}

	if (res == RES_RESULT_OK)
	{
		amp_map_curr = (uint16_t *)&get_amp_map.amdata;
		amp_map_end = (uint16_t *)(((uint8_t*)amp_map_curr) + sizeof(get_amp_map.amdata));
		i = 0;

		while ((res == RES_RESULT_OK) && (amp_map_curr < amp_map_end))
		{
			ret = snprintf(buf, sizeof(buf), "Amp[%u]=%hu\n", i, letohs(*amp_map_curr));
			if (ret < 0)
			{
				res = RES_RESULT_GENERAL_ERROR;
			}
			else
			{
				res = write_output(output_filename, buf);
			}

			amp_map_curr++;
			i++;
		}
	}

	return res;
}

static int process_cmd_get_channel_estimation(char *cmd, HLB_req_id_t req_id, char *output_filename)
{
	RES_result_t res;
	int written = 0;
	char buf[CE2_TOTAL_BUF_SIZE];
	uint8_t chunk_counter = 0;
	HLB_ce2_info_t ce2_info;
	HLB_ce2_data_req_t data_req;
	HLB_ce2_data_cnf_t ce2_data_cnf;
	int vector_count = 0;

	(void)cmd;
	if(output_filename == NULL)
	{
		printf("output_filename is required for get_channel_estimation\n");
		return RES_RESULT_MISSING_FILE;
	}
	res = HLB_nscm_get_ce2_info_req_send(handle, req_id);
	if (res != RES_RESULT_OK)
	{
		printf("HLB_nscm_get_ce2_info_req_send error\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	res = wait_for(rx_cal, req_id, HLB_HOST_MSG_ID_NSCM_GET_CE2_INFO_CNF, HW_APIS_TIMEOUT_USEC, 0);
	if (res >= HW_APIS_TIMEOUT_USEC)
	{
		printf("Timeout while waiting for NSCM_GET_CE2_INFO.CNF\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	res = HLB_nscm_get_ce2_info_cnf_receive(handle, req_id, &ce2_info);
	if (res != RES_RESULT_OK)
	{
		printf("HLB_nscm_get_ce2_info_cnf_receive error\n");
		return RES_RESULT_GENERAL_ERROR;
	}
	if(ce2_info.status == HLB_HPGP_RESULT_FAILURE)
	{
		printf("process_cmd_get_channel_estimation error, packet received "
				"with status %d\n", ce2_info.status);
		return RES_RESULT_GENERAL_ERROR;
	}
	if(ce2_info.total_buffer_size != CE2_TOTAL_BUFFER_SIZE)
	{
		printf("process_cmd_get_channel_estimation error, packet received "
				"total buffer size %u, should be:%u\n", ce2_info.total_buffer_size,
				CE2_TOTAL_BUFFER_SIZE);
		return RES_RESULT_GENERAL_ERROR;
	}
	written = snprintf(buf, sizeof(buf),
				  "gain=%d\nce2_shift=%d\n",
						 ce2_info.gain, ce2_info.ce2_shift);

	if (written < 0)
	{
		printf("process_cmd_get_channel_estimation error, failed to parse "
				"get_channel_estimation\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	while(chunk_counter <= HLB_MAX_CE2_CHUNK_NUM)
	{
		data_req.chunk_number = chunk_counter;
		res = HLB_nscm_get_ce2_data_req_send(handle, req_id, &data_req);
		if (res != RES_RESULT_OK)
		{
			printf("HLB_nscm_get_ce2_data_req_send error\n");
			return RES_RESULT_GENERAL_ERROR;
		}

		res = wait_for(rx_cal, req_id, HLB_HOST_MSG_ID_NSCM_GET_CE2_DATA_CNF, HW_APIS_TIMEOUT_USEC, 0);
		if (res >= HW_APIS_TIMEOUT_USEC)
		{
			printf("Timeout while waiting for NSCM_GET_CE2_DATA.CNF\n");
			return RES_RESULT_GENERAL_ERROR;
		}

		res = HLB_nscm_get_ce2_data_cnf_receive(handle, req_id, &ce2_data_cnf);

		if (res != RES_RESULT_OK || ce2_data_cnf.size == 0 ||
				ce2_data_cnf.status == HLB_HPGP_RESULT_FAILURE)
		{
			printf("HLB_nscm_get_ce2_data_cnf_receive error\n");
			return RES_RESULT_GENERAL_ERROR;
		}

		for (int i = 0; i < ce2_data_cnf.size; i++)
		{	
			res = snprintf(&buf[written], sizeof(buf) - written, "ce2[%d]=%d\n",
					vector_count, ce2_data_cnf.data[i]);
			if (res < 0)
			{
				printf("process_cmd_get_channel_estimation error, failed to parse ce2[%d] "
						"in chunk: %u\n", i, chunk_counter);
				return RES_RESULT_GENERAL_ERROR;
			}
			written += res;
			vector_count++;
		}
		
		chunk_counter++;
	}

	return write_output(output_filename, buf);
}

static int process_cmd_get_line_noise_estimation(char *cmd, HLB_req_id_t req_id, char *output_filename)
{
	RES_result_t res;
	int written = 0;
	char buf[LNOE_BUF_SIZE];
	HLB_lnoe_info_t lnoe_info;

	(void)cmd;
	if(output_filename == NULL)
	{
		printf("output_filename is required for get_line_noise_estimation\n");
		return RES_RESULT_MISSING_FILE;
	}
	res = HLB_nscm_get_lnoe_req_send(handle, req_id);
	if (res != RES_RESULT_OK)
	{
		printf("HLB_nscm_get_lnoe_req_send error\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	res = wait_for(rx_cal, req_id, HLB_HOST_MSG_ID_NSCM_GET_LNOE_CNF, HW_APIS_TIMEOUT_USEC, 0);
	if (res >= HW_APIS_TIMEOUT_USEC)
	{
		printf("Timeout while waiting for NSCM_GET_LNOE.CNF\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	res = HLB_nscm_get_lnoe_cnf_receive(handle, req_id, &lnoe_info);
	if (res != RES_RESULT_OK)
	{
		printf("HLB_nscm_get_lnoe_cnf_receive error\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	if(lnoe_info.status == HLB_HPGP_RESULT_FAILURE)
	{
		printf("process_cmd_get_line_noise_estimation error, packet received "
				"with status %d\n", lnoe_info.status);
		return RES_RESULT_GENERAL_ERROR;
	}

	written = snprintf(buf, sizeof(buf),
				  "gain=%d\n", lnoe_info.gain);

	if (written < 0)
	{
		printf("process_cmd_get_line_noise_estimation error, failed to parse "
				"get_line_noise_estimation\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	for (int i = 0; i < LNOE_DATA_LEN; i++)
	{	
		res = snprintf(&buf[written], sizeof(buf) - written, "lnoe[%d]=%d\n",
						i, lnoe_info.lnoe[i]);
		if (res < 0)
		{
			printf("process_cmd_get_line_noise_estimation error, failed to parse lnoe[%d]\n", i);
			return RES_RESULT_GENERAL_ERROR;
		}
		written += res;
	}
	return write_output(output_filename, buf);
}

static int process_cmd_get_snr(char *cmd, HLB_req_id_t req_id, char *output_filename)
{
	RES_result_t res;
	int written = 0;
	char buf[SNRE_BUF_SIZE];
	HLB_snre_info_t snre_info;

	(void)cmd;
	if(output_filename == NULL)
	{
		printf("output_filename is required for get_snr\n");
		return RES_RESULT_MISSING_FILE;
	}
	res = HLB_nscm_get_snre_req_send(handle, req_id);
	if (res != RES_RESULT_OK)
	{
		printf("HLB_nscm_get_snre_req_send error\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	res = wait_for(rx_cal, req_id, HLB_HOST_MSG_ID_NSCM_GET_SNRE_CNF, HW_APIS_TIMEOUT_USEC, 0);
	if (res >= HW_APIS_TIMEOUT_USEC)
	{
		printf("Timeout while waiting for NSCM_GET_SNRE.CNF\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	res = HLB_nscm_get_snre_cnf_receive(handle, req_id, &snre_info);
	if (res != RES_RESULT_OK)
	{
		printf("HLB_nscm_get_snre_cnf_receive error\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	if(snre_info.status == HLB_HPGP_RESULT_FAILURE)
	{
		printf("process_cmd_get_snr error, packet received "
				"with status %d\n", snre_info.status);
		return RES_RESULT_GENERAL_ERROR;
	}

	for (uint32_t i = 0; i < SNRE_DATA_LEN; i++)
	{	
		res = snprintf(&buf[written], sizeof(buf) - written, "snre[%d]=%d\n",
						i, snre_info.snre[i]);
		if (res < 0)
		{
			printf("process_cmd_get_snr error, failed to parse snre[%d]\n", i);
			return RES_RESULT_GENERAL_ERROR;
		}
		written += res;
	}
	return write_output(output_filename, buf);
}

static int process_cmd_link_stats(char *cmd, HLB_req_id_t req_id, char *output_filename)
{
	char *curr;
	RES_result_t res;
	HLB_hpgp_link_stats_req_t link_stats_req = {0};
	HLB_hpgp_link_stats_cnf_t link_stats_cnf;
	char buf[OUT_BUFF_MAX_SIZE];
	int written = 0;

	curr = strstr(cmd, "req_type=");
	if (!curr)
	{
		printf("missing req_type=\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	curr += strlen("req_type=");
	link_stats_req.req_type = atoi(curr);

	curr = strstr(cmd, "transmit_link_flag=");
	if (!curr)
	{
		printf("missing transmit_link_flag=\n");
		return RES_RESULT_GENERAL_ERROR;
	}
	
	curr += strlen("transmit_link_flag=");
	link_stats_req.transmit_link_flag = atoi(curr);

	curr = strstr(cmd, "da_sa_mac_addr=");
	if (curr)
	{
		curr += strlen("da_sa_mac_addr=");
		if (strlen(curr) < MAC_ADDR_LEN)
		{
			append_to_buffer("da_sa_mac_addr too short");
			return RES_RESULT_GENERAL_ERROR;
		}
		str_to_mac(curr, &link_stats_req.da_sa_mac_addr);
	}

	res = HLB_nscm_link_stats_req_send(handle, req_id, &link_stats_req);
	if (res != RES_RESULT_OK)
	{
		printf("HLB_apcm_link_stats_req_send error\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	res = wait_for(rx_cal, req_id, HLB_HOST_MSG_ID_NSCM_LINK_STATS_CNF, DEFAULT_TIMEOUT_USEC, 0);
	if (res >= DEFAULT_TIMEOUT_USEC)
	{
		printf("Timeout while waiting for NSCM_LINK_STATS.CNF\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	res = HLB_nscm_link_stats_cnf_receive(handle, req_id, &link_stats_cnf);
	if (res != RES_RESULT_OK)
	{
		printf("HLB_apcm_link_stats_cnf_receive error\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	if(link_stats_cnf.transmit_link_flag == HLB_HPGP_LINK_STATS_RECEIVE_LINK)
	{
		written = snprintf(buf, sizeof(buf),
				  	"link_stats Response Type:%u\nTransmit Link Flag:%u\n"
					"beacon_period_cnt:%u\nMSDUs:%u\noctets:%u\n"
					"segments_received:%u\nsegments missed:%u\n"
					"PBs_handed:%u\n"
					"MPDUs received:%u\n"
					"failed_icv_received_frames: %u",
					link_stats_cnf.res_type,
					link_stats_cnf.transmit_link_flag,
					link_stats_cnf.receive_link.beacon_period_cnt,
					link_stats_cnf.receive_link.MSDUs,
					link_stats_cnf.receive_link.octets,
					link_stats_cnf.receive_link.segments_received,
					link_stats_cnf.receive_link.segments_missed,
					link_stats_cnf.receive_link.PBs_handed,
					link_stats_cnf.receive_link.MPDUs_received,
					link_stats_cnf.receive_link.failed_icv_received_frames);

		if (written < 0)
		{
			printf("failed to parse process_cmd_link_stats\n");
			return RES_RESULT_GENERAL_ERROR;
		}
	}
	else
	{
		written = snprintf(buf, sizeof(buf),
						"link_stats Response Type:%u\nTransmit Link Flag:%u\n"
						"beacon_period_cnt:%u\nMSDUs:%u\noctets:%u\n"
						"segments_generated:%u\nsegments_delivered:%u\nsegments_dropped:%u\n"
						"PBs_handed:%u\nMPDUs_transmitted:%u\n"
						"MPDUs_successfully_acked:%u",link_stats_cnf.res_type,
						link_stats_cnf.transmit_link_flag,
						link_stats_cnf.transmit_link.beacon_period_cnt,
						link_stats_cnf.transmit_link.MSDUs,
						link_stats_cnf.transmit_link.octets,
						link_stats_cnf.transmit_link.segments_generated,
						link_stats_cnf.transmit_link.segments_delivered,
						link_stats_cnf.transmit_link.segments_dropped,
						link_stats_cnf.transmit_link.PBs_handed,
						link_stats_cnf.transmit_link.MPDUs_transmitted,
						link_stats_cnf.transmit_link.MPDUs_successfully_acked);
		
		if (written < 0)
		{
			printf("failed to parse process_cmd_link_stats\n");
			return RES_RESULT_GENERAL_ERROR;
		}
	}

	return write_output(output_filename, buf);
}

static int read_fw_register_value(uint32_t addr, uint32_t *p_value)
{
	RES_result_t res;
	HLB_req_id_t req_id = rand();
	HLB_read_mem_req_t read_mem_req;
	HLB_read_mem_cnf_t read_mem_cnf = { 0 };

	read_mem_req.address = addr;
	read_mem_req.size = sizeof(*p_value);

	res = HLB_nscm_read_mem_req_send(handle, req_id, &read_mem_req);

	if (res == RES_RESULT_OK)
	{
		res = wait_for(rx_cal, req_id, HLB_HOST_MSG_ID_NSCM_READ_MEM_CNF, DEFAULT_TIMEOUT_USEC, 0);
		if (res >= DEFAULT_TIMEOUT_USEC)
		{
			printf("Timeout while waiting for NSCM_READ_MEM.CNF\n");
			res = RES_RESULT_TIMEOUT;
		}
		else
		{
			res = RES_RESULT_OK;
		}
	}

	if (res == RES_RESULT_OK)
	{
		res = HLB_nscm_read_mem_cnf_receive(handle, req_id, &read_mem_cnf);
		if (res == RES_RESULT_OK)
		{
			if (read_mem_cnf.size != sizeof(*p_value))
			{
				res = RES_RESULT_GENERAL_ERROR;
			}
			else
			{
				memcpy(p_value, read_mem_cnf.data, sizeof(*p_value));
				*p_value = letohl(*p_value);
			}
		}
	}

	return res;
}

static int get_fw_slog_buf_size(uint32_t *slog_size)
{
	RES_result_t res;
	uint32_t start_add, last_add;

	res = read_fw_register_value(SLOG_IF_SLOG_FIRST_ADD_REG, &start_add);
	if (res == RES_RESULT_OK)
	{
		res = read_fw_register_value(SLOG_IF_SLOG_LAST_ADD_REG, &last_add);
		if (res == RES_RESULT_OK)
		{
			/* LAST_ADD register value is in fact 16 bytes aligned,
			 * So the last nibble is ignored by the HW
			 */
			last_add &= (~0xF);
			*slog_size = (last_add - start_add) + 16U;
		}
	}

	return res;
}

static uint32_t closest_power_of_two_under_or_equal_to(uint32_t num)
{
	uint32_t res = 0;

	if (num)
	{
		res = 1;
		do
		{
			num >>= 1;
			res <<= 1;
		}
		while (num);
		res >>= 1;
	}

	return res;
}

static int process_cmd_get_noise_floor(char *cmd)
{
	RES_result_t res;
	char *curr;
	uint8_t iterations_num;
	int gain;
	char *slog_ocla_file_name;
	FILE *slog_ocla_file;
	char *slog_ocla_buffer;
	long slog_ocla_size;
	char tmp_slog_str[MAX_CLI_LEN];
	int16_t *time_domain_buffer;
	FILE *time_domain_output_file;
	char tmp_time_domain_str[MAX_CLI_LEN];
	char *time_domain_file_name;
	int time_domain_values_num;
	char tmp_output_buf[MAX_CLI_LEN];
	uint32_t fw_slog_buf_size;
	uint32_t sample_size;

	curr = strstr(cmd, "iterations=");
	if (!curr)
	{
		printf("missing iterations=\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	curr += strlen("iterations=");
	iterations_num = atoi(curr);
	curr = strstr(cmd, "gain=");
	if (!curr)
	{
		printf("missing gain=\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	curr += strlen("gain=");
	gain = atoi(curr);

	curr = strstr(cmd, "slog_ocla_file=");
	if (curr == NULL)
	{
		append_to_buffer("missing slog_ocla_file=");
		return -1;
	}

	curr += strlen("slog_ocla_file=");
	strcpy(tmp_slog_str, curr);
	slog_ocla_file_name = strtok(tmp_slog_str, " ");
	slog_ocla_size = osal_get_file_size(slog_ocla_file_name);
	slog_ocla_buffer = calloc(1, slog_ocla_size + 1);
	if(!slog_ocla_buffer)
	{
		printf("HLB_get_noise_floor: Couldn't allocate slog ocla buffer\n");
		return RES_RESULT_NO_MEMORY;
	}
	slog_ocla_file = fopen(slog_ocla_file_name, "r");
	if(!slog_ocla_file)
	{
		printf("HLB_get_noise_floor: Couldn't open slog_ocla_file\n");
		return RES_RESULT_MISSING_FILE;
	}

	if(!fread(slog_ocla_buffer, 1, slog_ocla_size, slog_ocla_file))
	{
		fclose(slog_ocla_file);
		free(slog_ocla_buffer);
		printf("HLB_get_noise_floor: Failed to read slog file\n");
		return RES_RESULT_GENERAL_ERROR;
	}
	fclose(slog_ocla_file);

	curr = strstr(cmd, "output_file=");
	if (curr == NULL)
	{
		free(slog_ocla_buffer);
		append_to_buffer("missing output_file=");
		return -1;
	}

	curr += strlen("output_file=");
	strcpy(tmp_time_domain_str, curr);
	time_domain_file_name = strtok(tmp_time_domain_str, " ");

	time_domain_output_file = fopen(time_domain_file_name, "w+");
	if(!time_domain_output_file)
	{
		free(slog_ocla_buffer);
		printf("HLB_get_noise_floor: Couldn't open time_domain_output_file\n");
		return RES_RESULT_MISSING_FILE;
	}

	res = get_fw_slog_buf_size(&fw_slog_buf_size);
	if (res != RES_RESULT_OK || fw_slog_buf_size == 0)
	{
		free(slog_ocla_buffer);
		fclose(time_domain_output_file);
		printf("get_fw_slog_buf_size failed, res = %d\n", res);
		return RES_RESULT_GENERAL_ERROR;
	}

	sample_size = closest_power_of_two_under_or_equal_to(fw_slog_buf_size);

	time_domain_values_num = (sample_size * iterations_num) / 2;
	time_domain_buffer = malloc(time_domain_values_num * sizeof(uint16_t));
	if(!time_domain_buffer)
	{
		free(slog_ocla_buffer);
		fclose(time_domain_output_file);
		printf("HLB_get_noise_floor: Failed to allocate time domain buffer\n");
		return RES_RESULT_NO_MEMORY;
	}

	res = HLB_get_noise_floor(handle, iterations_num, gain, slog_ocla_buffer,
							  fw_slog_buf_size, sample_size, time_domain_buffer);
	if (res != RES_RESULT_OK)
	{
		free(slog_ocla_buffer);
		free(time_domain_buffer);
		fclose(time_domain_output_file);
		printf("HLB_get_noise_floor failed\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	res = snprintf(tmp_output_buf, sizeof(tmp_output_buf),
				  "Gain=%d\nIterations=%u\nSample_Size=%u\n",
				  gain, iterations_num, sample_size);
	if (res < 0)
	{
		free(slog_ocla_buffer);
		free(time_domain_buffer);
		fclose(time_domain_output_file);
		printf("process_cmd_get_noise_floor error, failed to parse "
				"Gain and Iterations\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	res = fputs(tmp_output_buf, time_domain_output_file);
	if (res < 0)
	{
		free(slog_ocla_buffer);
		free(time_domain_buffer);
		fclose(time_domain_output_file);
		printf("process_cmd_get_noise_floor error, failed to write to output file\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	for (int i = 0; i < time_domain_values_num; i++)
	{
		res = snprintf(tmp_output_buf, sizeof(tmp_output_buf), "Sample[%d]=%f\t%.3f\n",
						i, (double)time_domain_buffer[i]/TIME_DOMAIN_NORMALISER, (double)i/200);
		if (res < 0)
		{
			free(slog_ocla_buffer);
			free(time_domain_buffer);
			fclose(time_domain_output_file);
			printf("process_cmd_get_noise_floor faile to parse output buffer\n");
			return RES_RESULT_GENERAL_ERROR;
		}
		res = fputs(tmp_output_buf, time_domain_output_file);
		if (res < 0)
		{
			free(slog_ocla_buffer);
			free(time_domain_buffer);
			fclose(time_domain_output_file);
			printf("process_cmd_get_noise_floor error, failed to write to output file\n");
			return RES_RESULT_GENERAL_ERROR;
		}
	}

	fclose(time_domain_output_file);
	free(time_domain_buffer);
	free(slog_ocla_buffer);
	return RES_RESULT_OK;
}

static int process_cmd_get_device_state(char *cmd, HLB_req_id_t req_id, char *output_filename)
{
	RES_result_t res;
	HLB_hpgp_device_state_cnf_t device_state;
	char buf[OUT_BUFF_MAX_SIZE];

	(void)cmd;

	res = HLB_nscm_get_device_state_req_send(handle, req_id);
	if (res != RES_RESULT_OK)
	{
		printf("HLB_nscm_get_device_state_req_send error\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	res = wait_for(rx_cal, req_id, HLB_HOST_MSG_ID_NSCM_GET_DEVICE_STATE_CNF, DEFAULT_TIMEOUT_USEC, 0);
	if (res >= DEFAULT_TIMEOUT_USEC)
	{
		printf("Timeout while waiting for NSCM_GET_DEVICE_STATE.CNF\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	res = HLB_nscm_get_device_state_cnf_receive(handle, req_id, &device_state);
	if (res != RES_RESULT_OK)
	{
		printf("HLB_nscm_get_device_state_cnf_receive error\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	res = snprintf(buf, sizeof(buf),
		"get_device_state device state=%u req_id=%u", device_state.device_status, req_id);

	if(res < 0)
	{
		printf("process_cmd_get_device_state error, failed to parse "
				"get_device_state result\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	return write_output(output_filename, buf);
}

static int process_cmd_get_d_link_status(char *cmd, HLB_req_id_t req_id, char *output_filename)
{
	RES_result_t res;
	HLB_hpgp_d_link_status_cnf_t device_status;
	char buf[OUT_BUFF_MAX_SIZE];

	(void)cmd;

	res = HLB_nscm_get_d_link_status_req_send(handle, req_id);
	if (res != RES_RESULT_OK)
	{
		printf("HLB_nscm_get_d_link_status_req_send error\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	res = wait_for(rx_cal, req_id, HLB_HOST_MSG_ID_NSCM_GET_D_LINK_STATUS_CNF, DEFAULT_TIMEOUT_USEC, 0);
	if (res >= DEFAULT_TIMEOUT_USEC)
	{
		printf("Timeout while waiting for NSCM_GET_D_LINK_STATUS.CNF\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	res = HLB_nscm_get_d_link_status_cnf_receive(handle, req_id, &device_status);
	if (res != RES_RESULT_OK)
	{
		printf("HLB_nscm_get_d_link_status_cnf_receive error\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	res = snprintf(buf, sizeof(buf),
		"get_d_link_status link status=%u req_id=%u", device_status.link_status, req_id);

	if(res < 0)
	{
		printf("process_cmd_get_d_link_status error, failed to parse "
				"get_d_link_status_cnf result\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	return write_output(output_filename, buf);
}

static int process_cmd_link_terminate(char *cmd, HLB_req_id_t req_id)
{
	RES_result_t res;
	(void)cmd;

	res = HLB_nscm_d_link_terminate_req_send(handle, req_id);
	if (res != RES_RESULT_OK)
	{
		printf("HLB_nscm_d_link_terminate_req_send error\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	res = wait_for(rx_cal, req_id, HLB_HOST_MSG_ID_NSCM_D_LINK_TERMINATE_CNF, DEFAULT_TIMEOUT_USEC, 0);
	if (res >= DEFAULT_TIMEOUT_USEC)
	{
		printf("Timeout while waiting for NSCM_D_LINK_TERMINATE.CNF\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	res = HLB_nscm_d_link_terminate_cnf_receive(handle, req_id);
	if (res != RES_RESULT_OK)
	{
		printf("HLB_nscm_d_link_terminate_cnf_receive error\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	printf("link_terminate_success\n");

	return RES_RESULT_OK;
}


static int process_cmd(char *cmd, char *output_filename)
{
	char *curr;
	HLB_req_id_t req_id;
	req_id = rand();
	RES_result_t res = RES_RESULT_OK;

	curr = strstr(cmd, "req_id=");
	if (curr)
	{
		curr += strlen("req_id=");
		req_id = atoi(curr);
	}

	if (strstr(cmd, "set_key"))
	{
		return process_cmd_set_key(cmd, req_id, output_filename);
	}
	else if (strstr(cmd, "reset_device"))
	{
		return process_cmd_reset_device(cmd);
	}
	else if (strstr(cmd, "get_device_version"))
	{
		return process_cmd_get_device_version(cmd, req_id, output_filename);
	}
	else if (strstr(cmd, "device_info"))
	{
		return process_cmd_device_info(cmd, req_id, output_filename);
	}
	else if (strstr(cmd, "get_amp_map"))
	{
		return process_cmd_get_amp_map(cmd, req_id, output_filename);
	}
	else if(strstr(cmd, "get_channel_estimation"))
	{
		return process_cmd_get_channel_estimation(cmd, req_id, output_filename);
	}
	else if(strstr(cmd, "get_line_noise_estimation"))
	{
		return process_cmd_get_line_noise_estimation(cmd, req_id, output_filename);
	}
	else if(strstr(cmd, "get_snr"))
	{
		return process_cmd_get_snr(cmd, req_id, output_filename);
	}
	else if(strstr(cmd, "link_stats"))
	{
		return process_cmd_link_stats(cmd, req_id, output_filename);
	}
	else if(strstr(cmd, "get_noise_floor"))
	{
		return process_cmd_get_noise_floor(cmd);
	}
	else if (strstr(cmd, "get_device_state"))
	{
		return process_cmd_get_device_state(cmd, req_id, output_filename);
	}
	else if (strstr(cmd, "get_d_link_status"))
	{
		return process_cmd_get_d_link_status(cmd, req_id, output_filename);
	}
	else if (strstr(cmd, "link_terminate"))
	{
		return process_cmd_link_terminate(cmd, req_id);
	}
	else
	{
		append_to_buffer("unknown cmd:%s", cmd);
		res = RES_RESULT_BAD_PARAMETER;
	}

	return res;
}

static void print_version(void)
{
	printf("current version=%d.%d.%d.%d\n", MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION, BUILD_VERSION);
}

static void usage(void)
{
	printf("management_tool [-d \"dest mac address\"] [-a \"adapter mac address\"] "
		   "[-s \"adapter interface name\"] "
		   "<-c \"cmd to send\"> [-f \"output_filename\"]\n"
		   "options:\n"
		   "-a = mac address of source interface (eth/spi)\n"
		   "     Default if not provided:: Use SPI interface if exists, o/w use ETH interface\n"
		   "-d = mac address of destination (of CG5317, or broadcast)\n"
		   "     Default if not provided:: broadcast (ff:ff:ff:ff:ff:ff)\n"
		   "-c = the command to execute (use -l to see list of commands)\n"
		   "-l = list all cmds (-c \"cmd\")\n"
		   "-f = stores output of command into file (o/w prints to console)\n"
		   "     Note: output file is required for some commands\n"
		   "-v = print version\n"
		   "-h = help (show this usage text)\n");
}

static void cmds_list(void)
{
	printf("<>=mandatory []=optional (*option*)=option not supported\n"
		   "cmd: set_key NMK=<16 byte hex string> NID=<7 byte hex string> OR sec_level=<0(simple connect),1(Secure Security)>\n"
		   "\texample1: set_key NMK=50D3E4933F855B7040784DF815AA8DB7 NID=B0F2E695666B03\n"
		   "\texample2: set_key NMK=50D3E4933F855B7040784DF815AA8DB7 sec_level=0\n"
		   "cmd: reset_device [reset_mode=<0(Default: normal),1(reset to hostboot)>]\n"
		   "cmd: get_device_version req_id=<0-255>\n"
		   "cmd: device_info req_id=<0-255>\n"
		   "cmd: get_amp_map req_id=<0-255>\n"
		   "cmd: get_channel_estimation req_id=<0-255>\n"
		   "cmd: get_line_noise_estimation req_id=<0-255>\n"
		   "cmd: get_snr req_id=<0-255>\n"
		   "cmd: link_stats"
		   " req_type=<0(reset),1(get),2(get&reset)>"
		   " transmit_link_flag=<0(transmit),1(receive)> req_id=[0-255]\n"
		   "example: link_stats req_type=1 transmit_link_flag=0\n"
		   "cmd: get_noise_floor iterations=<integer> gain=<integer> slog_ocla_file=<SLOG_OCLA_FILE_PATH>"
		   " output_file=<OUTPUT_FILE_PATH> req_id=<0-255>\n"
		   "cmd: get_device_state req_id=<0-255>\n"
		   "cmd: get_d_link_status req_id=<0-255>\n"
		   "cmd: link_terminate req_id=<0-255>\n");
}

int main(int argc, char *argv[])
{
	int res, c;
	char *cmd = NULL;
	char *dest_mac_addr = NULL;
	char *adapter_mac_addr = NULL;
	char *output_filename = NULL;
	void *allocated_area = NULL;
	size_t allocated_area_size;
	mac_address_t dest_addr;
	mac_address_t adapter_addr;
	char *devicename = SPI_IFACE_NAME;

	srand(time(0));
	out_buf[0] = '\0';

	while ((c = getopt(argc, argv, "vhlc:d:a:f:s:")) != -1)
	{
		switch (c)
		{
		case 'c':
			cmd = optarg;
			break;
		case 'h':
			usage();
			return RES_RESULT_OK;
		case 'v':
			print_version();
			return RES_RESULT_OK;
		case 'l':
			cmds_list();
			return RES_RESULT_OK;
		case 'd':
			dest_mac_addr = optarg;
			break;
		case 'a':
			adapter_mac_addr = optarg;
			break;
		case 'f':
			output_filename = optarg;
			break;
		case 's':
			devicename = optarg;
			break;
		default:
			usage();
			return RES_RESULT_GENERAL_ERROR;
		}
	}

	if (!cmd)
	{
		printf("cmd was not provided\n");
		usage();
		return RES_RESULT_GENERAL_ERROR;
	}

	if (adapter_mac_addr)
	{
		str_to_mac(adapter_mac_addr, &adapter_addr);
	}
	else
	{
		bool spi_used = true;

		/* Source adapter not provided -> try to use SPI interface */
		res = ETH_get_mac_address_by_interface_name(devicename, adapter_addr);
		if (res != RES_RESULT_OK)
		{
			/* Try to use Ethernet interface instead */
			spi_used = false;
			res = ETH_get_mac_address_by_interface_name(devicename, adapter_addr);
		}

		if (res == RES_RESULT_OK)
		{
			printf("Adapter (src) mac address not provided (-a) "
				   "-> using %s interface (" MAC_ADDR_FMT ")\n",
				   (spi_used) ? "SPI" : "ETH",
				   MAC_ADDR_FMT_ARGS(adapter_addr));
		}
		else
		{
			printf("Failed to find Adapter (src) (err=%d)\n", res);
			return res;
		}
	}

	if (dest_mac_addr)
	{
		str_to_mac(dest_mac_addr, &dest_addr);
	}
	else
	{
		str_to_mac("ff:ff:ff:ff:ff:ff", &dest_addr);
		printf("Destination mac address not provided (-d)   "
			   "-> using broadcast (" MAC_ADDR_FMT ")\n",
			   MAC_ADDR_FMT_ARGS(dest_addr));
	}

	if ((output_filename != NULL) && (access(output_filename, F_OK) == 0))
	{
		printf("WARNING: file %s already exists -> removing file\n", output_filename);
		if (remove(output_filename))
		{
			printf("failed to remove file\n");
			return RES_RESULT_GENERAL_ERROR;
		}
	}

	allocated_area_size = ALLOCATED_AREA_SIZE;
	allocated_area = malloc(allocated_area_size);
	if (allocated_area == NULL)
	{
		printf("malloc failed\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	init_map_locks(rx_cal);

	handle = HLB_init((uint8_t *)adapter_addr, (uint8_t *)dest_addr, rx_callback, allocated_area, allocated_area_size);
	if (handle == NULL)
	{
		printf("HLB_init failed\n");
		free(allocated_area);
		return RES_RESULT_GENERAL_ERROR;
	}

	res = process_cmd(cmd, output_filename);
	printf("%s", out_buf);
	if (res != RES_RESULT_OK)
	{
		printf("process_cmd %s failed, res=%d\n", cmd, res);
	}
	else
	{
		printf("command: '%s' finished successfully\n", cmd);

		if (output_filename != NULL)
		{
			printf("output written to '%s'\n", output_filename);
		}
	}

	HLB_deinit(handle);
	destroy_map_locks(rx_cal);

	free(allocated_area);

	return res;
}
