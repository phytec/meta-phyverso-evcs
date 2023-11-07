/********************************************************************
*
* Module Name: production_tool
* Design:
* the production tool capabilities can be divided into two categories:
* 1. Configuration binary management - see dedicated section Configuration
* 2. Post-processing of various HW samples - Noise Floor, SNR, and potentially others
*
********************************************************************/
/*******************************************************************
* IMPORTS
********************************************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include "version.h"
#include "PROD_lib.h"

/*******************************************************************
* CONSTANTS
********************************************************************/
#define CONFIGURATION_MAX_SIZE 1024
#define MAX_ARG_LEN 15
#define MAX_CONFIG_LINE_LEN 200
#define MIN_CONFIG_LINE_LEN 5
#define MAX_ADDRESS_COLONS_NUM 5
#define MAC_ADDR_LEN sizeof(mac_address_t) * 2 + MAX_ADDRESS_COLONS_NUM
#define SIZE_AMP_MAP_ARG 8
#define SUCCESS 0
#define FAILURE -1
#define DEFAULT_AMP_MAP_VALUE 0
#define TIME_DOMAIN_NORMALISER (2048)

#define CG_PIN_DRIVE_STRENGTH_8_mA_STRING "8"
#define CG_PIN_DRIVE_STRENGTH_4_mA_STRING "4"
#define CG_PIN_DRIVE_STRENGTH_8_mA_W_SLEW_RATE_STRING "8_S"
/*******************************************************************
* TYPES
********************************************************************/
typedef struct
{
	bool mac_addr_filled;
	bool conf_version_filled;
	bool nmk_filled;
	bool cco_role_filled;
	uint16_t amp_vals_filled;
} filled_config_params_flags;

typedef struct
{
	bool gain_filled;
	bool ce2_shift_filled;
	uint16_t ce2_vals_filled;
	uint16_t lnoe_vals_filled;
	uint16_t snre_vals_filled;
	uint16_t amp_map_vals_filled;
} filled_hw_tools_params_flags;

typedef struct
{
	bool gain_filled;
	bool iterations_filled;
	bool sample_size_filled;
	uint32_t sample_vals_filled;
} filled_noise_floor_params_flags;

typedef enum
{
	CE2 = 0,
	LNOE = 1,
	SNRE = 2,
	AMP_MAP = 3,
} hw_tool_type;

/*******************************************************************
* STATIC and GLOBAL DATA
********************************************************************/
/*******************************************************************
* MACROS
********************************************************************/

#define PARSE_CONFIG_TEXT(param, var)                                    \
	var = atoi(curr_argument);                                           \
	if (var == 0 && curr_argument[0] != '0')                             \
	{                                                                    \
		fprintf(stderr, "Illegal value for " #param " was provided!\n"); \
		return FAILURE;                                                  \
	}

/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/
static bool is_valid_mac_addr(const char* mac)
{
    int i = 0;
    int s = 0;

    while (*mac)
	{
       if (isxdigit(*mac))
	   {
          i++;
       }
       else if (*mac == ':' || *mac == '-')
	   {

          if (i == 0 || i / 2 - 1 != s)
		  {
			  break;
		  }

          ++s;
       }
       else
	   {
           s = -1;
       }

       ++mac;
    }
	return (i == 12 && s == 5);
}

static bool is_empty_line(const char *s)
{
  while (*s != '\0' && *s != '\n')
  {
    if (!isspace((unsigned char)*s))
	{
		return false;
	}
    s++;
  }
  return true;
}

static void str_to_binary(char *xi_str, int xi_len, unsigned char *xo_buffer)
{
	int i;
	int j = 0;
	unsigned int val[1];
	if (xi_str[0] == '0' && xi_str[1] == 'x')
	{
		j = 2;
		xi_len += 2;
	}
	for (i = 0; j < xi_len; ++i, j += 2)
	{

		if ((xi_str[j] == 0) || (xi_str[j + 1] == 0))
		{
			break;
		}
		sscanf(xi_str + j, "%2x", val);
		xo_buffer[i] = val[0];
	}
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

static const char* pin_drive_strength_to_str(cg_pin_drive_strength_t drive_strength)
{
	const char *ret;

	switch (drive_strength)
	{
		case CG_PIN_DRIVE_STRENGTH_8_mA:
			ret = CG_PIN_DRIVE_STRENGTH_8_mA_STRING;
			break;
		case CG_PIN_DRIVE_STRENGTH_4_mA:
			ret = CG_PIN_DRIVE_STRENGTH_4_mA_STRING;
			break;
		case CG_PIN_DRIVE_STRENGTH_8_mA_W_SLEW_RATE:
			ret = CG_PIN_DRIVE_STRENGTH_8_mA_W_SLEW_RATE_STRING;
			break;
		default:
			ret = "Invalid Value";
			break;
	}

	return ret;
}

static int pin_drive_strength_str_to_enum(const char *str, cg_pin_drive_strength_t *out_drive_strength)
{
	int ret = 0;

	if (!strcmp(str, CG_PIN_DRIVE_STRENGTH_8_mA_STRING))
	{
		*out_drive_strength = CG_PIN_DRIVE_STRENGTH_8_mA;
	}
	else if (!strcmp(str, CG_PIN_DRIVE_STRENGTH_4_mA_STRING))
	{
		*out_drive_strength = CG_PIN_DRIVE_STRENGTH_4_mA;
	}
	else if (!strcmp(str, CG_PIN_DRIVE_STRENGTH_8_mA_W_SLEW_RATE_STRING))
	{
		*out_drive_strength = CG_PIN_DRIVE_STRENGTH_8_mA_W_SLEW_RATE;
	}
	else
	{
		ret = 1;
	}

	return ret;
}

static int PROD_write_to_file(PROD_configuration_content_t* cnf_content, FILE *dst)
{
	int res;
	char buf[MAX_CONFIG_LINE_LEN];
	char *curr_string;
	char temp[MAX_ARG_LEN];
	char result[MAX_CONFIG_LINE_LEN];


	res = snprintf(buf, sizeof(buf), 
				  "# The configuration binary version:"
				  "A number that describes the version\n"
				  "Configuration version=%d",
			      cnf_content->configuration_binary_version);
	if (res < 0)
	{
		fprintf(stderr, "Failed to parse configuration_binary_version!\n");
		return FAILURE;
	}
	fputs(buf, dst);

	res = snprintf(buf, sizeof(buf),
				   "\n\n# The mac adress of the modem\n"
				   "MAC address=%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
				    cnf_content->mac_address[0], cnf_content->mac_address[1],
					cnf_content->mac_address[2], cnf_content->mac_address[3],
					cnf_content->mac_address[4], cnf_content->mac_address[5]);
	if (res < 0)
	{
		fprintf(stderr, "Failed to parse nmac_address!\n");
		return FAILURE;
	}
	fputs(buf, dst);

	curr_string = "\n\n# Network Membership Key: NMK=<32 hex>\nNMK=";
	strcpy(result, curr_string);
	for (size_t i = 0; i < HPAVKEY_NMK_LEN; i++)
	{
		sprintf(&temp[0], "%.2X", cnf_content->nmk.NMK[i]);
		strcat(result, temp);
	}
	fputs(result, dst);

	res = snprintf(buf, sizeof(buf), "\n\n# CCo Role=<0(auto), 1(never a CCo), 2(always a CCo)>\nCCo Role=%d", cnf_content->cco_role);
	if (res < 0)
	{
		fprintf(stderr, "Failed to parse cco_role!\n");
		return FAILURE;
	}
	fputs(buf, dst);

#ifndef CG5315
	curr_string = "\n\n# The Manufacturers Human friendly identifier"
				  "\n# A string of up to 64 ASCII characters chosen from the range ASCII[32] to ASCII[127]"
				  "\nManufacturer HFID=";
	strcpy(result, curr_string);
	strcat(result, cnf_content->manu_hfid.HFID);
	fputs(result, dst);

	curr_string = "\n\n# The Users Human friendly identifier"
				  "\n# A string of up to 64 ASCII characters chosen from the range ASCII[32] to ASCII[127]"
				  "\nUser HFID=";
	strcpy(result, curr_string);
	strcat(result, cnf_content->user_hfid.HFID);
	fputs(result, dst);

	curr_string = "\n\n# The AVLN Human friendly identifier"
				  "\n# A string of up to 64 ASCII characters chosen from the range ASCII[32] to ASCII[127]"
				  "\nAVLN HFID=";
	strcpy(result, curr_string);
	strcat(result, cnf_content->avln_hfid.HFID);
	fputs(result, dst);

	fputs("\n\n# The enabled Host interface: <0(SPI), 1(ETH - R/MII), 2(SPI Debug)>", dst);
	fputs("\n# 0 - SPI interface is enabled, ETH(R/MII) interface is disabled - only SPI interface will be used", dst);
	fputs("\n# 1 - ETH(R/MII) interface is enabled, SPI interface is disabled - only ETH(R/MII) interface will be used", dst);
	fputs("\n# 2 - SPI Debug mode. Both interfaces are enabled - This is not an operational mode!", dst);
	res = snprintf(buf, sizeof(buf), "\nHost Interface=%d", cnf_content->host_interface);
	if (res < 0)
	{
		fprintf(stderr, "Failed to parse host_interface!\n");
		return FAILURE;
	}
	fputs(buf, dst);

	res = snprintf(buf, sizeof(buf), "\n\n# The enabled PLC Frequency Selection: <0(50Hz), 1(60Hz), 2(External Signal)>"
									 "\nPLC Frequency Selection=%d", cnf_content->plc_freq_sel);
	if (res < 0)
	{
		fprintf(stderr, "Failed to parse plc_freq_sel!\n");
		return FAILURE;
	}
	fputs(buf, dst);

	fputs("\n\n# Network Identifier: NID=<14 hex>", dst);
	fputs("\n# Value of all 0xFF equals default NID, which means that CG5317 will generate the NID", dst);
	fputs("\n# from the NMK and the security level.", dst);
	res = snprintf(buf, sizeof(buf), "\nNID=");
	if (res < 0)
	{
		fprintf(stderr, "Failed to parse nid!\n");
		return FAILURE;
	}
	fputs(buf, dst);
	for (size_t i = 0; i < HPAVKEY_NID_LEN; i++)
	{
		snprintf(buf, sizeof(buf), "%.2X", cnf_content->nid.NID[i]);
		fputs(buf, dst);
	}

	fputs("\n\n# The Security Level: <0(simple connect), 1(secure connect)>", dst);
	fputs("\n# Relevant only when default NID is used (NID=FFFFFFFFFFFFFF)", dst);
	res = snprintf(buf, sizeof(buf), "\nSecurity Level=%d", cnf_content->security_level);
	if (res < 0)
	{
		fprintf(stderr, "Failed to parse security_level!\n");
		return FAILURE;
	}
	fputs(buf, dst);

	fputs("\n\n# Maximal Receiver Sensitivity is decreased by dB amount, with a range between 0 and 40,", dst);
	fputs("\n# where 0 is the maximal sensitivity and 40 is the minimal", dst);
	res = snprintf(buf, sizeof(buf), "\nDecrease Maximal Receiver Sensitivity=%hhu",
				   MAX_RECEIVER_SENSITIVITY_DB - cnf_content->receiver_max_sensitivity);
	if (res < 0)
	{
		return FAILURE;
	}
	fputs(buf, dst);

	fputs("\n\n# Configure drive-strength of HOST1 (RXD_0), HOST2 (RXD_1), HOST3 (RXD_2), HOST4 (RXD_3), HOST5 (RX_DV) pins (Select one of the following 4[mA] / 8[mA] / 8_S[mA])", dst);
	fputs("\n# (8_S refers to 8_Slew_rate, please refer to user configuration document for detailed information on slew_rate)", dst);
	res = snprintf(buf, sizeof(buf), "\nRXD_0/1/2/3 RX_DV Drive Strength=%s",
				   pin_drive_strength_to_str(cnf_content->cg_drive_strength_rxd_0_1_2_3_and_rx_dv));
	if (res < 0)
	{
		return FAILURE;
	}
	fputs(buf, dst);

	fputs("\n\n# Configure drive-strength of HOST6 (TX_CLK) pin (Select one of the following 4[mA] / 8[mA] / 8_S[mA])", dst);
	fputs("\n# (8_S refers to 8_Slew_rate, please refer to user configuration document for detailed information on slew_rate)", dst);
	res = snprintf(buf, sizeof(buf), "\nTX_CLK Drive Strength=%s",
				   pin_drive_strength_to_str(cnf_content->cg_drive_strength_tx_clk));
	if (res < 0)
	{
		return FAILURE;
	}
	fputs(buf, dst);

	fputs("\n\n# Configure PSD gain additional gain in dB, to calibrate the PSD to -75dBm/Hz  (value between 0 to 15dB)", dst);
	fputs("\n# (Please refer to user configuration document for detailed information on the calibration process and the PSD gain parameter)", dst);
	res = snprintf(buf, sizeof(buf), "\nPSD gain=%.1f", cnf_content->PSD_gain);
	if (res < 0)
	{
		return FAILURE;
	}
	fputs(buf, dst);

#endif

	fputs("\n\n# Amplitude[0-1154]: Range 0.0 - 68.0 [dB] or OFF", dst);
	for (size_t i = 0; i < USER_AMDATA_LEN; i++)
	{	
		if(cnf_content->amplitude_map.amdata[i] == OFF_SPECIAL_CASE)
		{
			res = snprintf(buf, sizeof(buf), "\nAmplitude [%ld]=OFF", i);
			if (res < 0)
			{
				fprintf(stderr, "Failed to parse Amplitude [%zu]!\n", i);
				return FAILURE;
			}
		}
		else
		{
			res = snprintf(buf, sizeof(buf), "\nAmplitude [%ld]=%.1f",
					   i, cnf_content->amplitude_map.amdata[i]);
			if (res < 0)
			{
				fprintf(stderr, "Failed to parse Amplitude [%zu]!\n", i);
				return FAILURE;
			}
			if (buf[res-1] == '0')
			{
				/* we prefer XX over XX.0 in the file*/
				buf[res-2] = 0;
			}
		}
		
		fputs(buf, dst);
	}

	return SUCCESS;
}

static int PROD_parse_config(FILE *src, FILE *dst)
{
	/* Read the data from the bin file*/
	PROD_configuration_content_packed_t packed_cnf_content = {0};
	PROD_configuration_content_t cnf_content;
	int res;
	fread(&packed_cnf_content, 1, sizeof(PROD_configuration_content_packed_t), src);
	res = PROD_configuration_binary_content_parse(&packed_cnf_content,
											sizeof(PROD_configuration_content_packed_t),
											&cnf_content);
	if(res != SUCCESS)
	{
		return res;
	}
	return PROD_write_to_file(&cnf_content, dst);
}

static int PROD_parse_default_config(FILE *dst)
{
	PROD_configuration_content_t cnf_content = PRD_CONFIG_INIT;

	return PROD_write_to_file(&cnf_content, dst);
}

static int PROD_extract_command(char *line, char *command, char *argument)
{
	char curr_command[MAX_CONFIG_LINE_LEN];
	bool left_side = true;
	int command_index = 0;
	int argument_index = 0;
	int i;
	for (i = 0; i < MAX_CONFIG_LINE_LEN; i++)
	{

		if (isspace(line[i]) && (left_side == true))
		{
			continue;
		}
		else if (line[i] == '=')
		{
			left_side = false;
		}
		else if ((line[i] == '\n') || (line[i] == '\r'))
		{
			break;
		}
		else if (left_side)
		{
			command[command_index] = line[i];
			command_index++;
		}
		else
		{
			argument[argument_index] = line[i];
			argument_index++;
		}
	}
	command[command_index] = '\0';
	argument[argument_index] = '\0';
	return SUCCESS;
}

static int PROD_parse_hfid_argument(hpgp_hfid_t *hfid, const char *argument, const char *param_name)
{
	size_t i, len = strlen(argument);
	int ret = SUCCESS;

	if (len <= HPAVKEY_HFID_LEN && len > 0)
	{
		for (i = 0; i < len; i++)
		{
			if ((argument[i] < 32) || (argument[i] > 127))
			{
				fprintf(stderr, "%s argument contains invalid character, please use alphabetic text string ((ASCII[32] to ASCII[127])\n", param_name);
				ret = FAILURE;
				break;
			}
		}
	}
	else
	{
		fprintf(stderr, "%s too long\n", param_name);
		ret = FAILURE;
	}

	if (ret == SUCCESS)
	{
		memset(hfid->HFID, 0, sizeof(hfid->HFID));
		strcpy(hfid->HFID, argument);
	}

	return ret;
}

static int PROD_process_config_line(char *line, PROD_configuration_content_t *cnf_content, int line_number,
										filled_config_params_flags *params)
{
	char curr_command[MAX_CONFIG_LINE_LEN];
	char curr_argument[MAX_CONFIG_LINE_LEN];

	if(is_empty_line(line))
	{
		/* An empty line - skip it */
		return SUCCESS;
	}

	PROD_extract_command(line, curr_command, curr_argument);
	size_t len = strlen(curr_argument);
	size_t args_len = 0;
	char *curr;
	char *eptr;
	int i;

	if (strcmp(curr_command, "Configurationversion") == 0)
	{
		if(params->conf_version_filled)
		{
			fprintf(stderr, "Configuration version filled multiple times\n");
			return FAILURE;
		}

		for(i = 0;;i++)
		{

			if(curr_argument[i] == '\n' || curr_argument[i] == 0)
			{
				break;
			}
			if(curr_argument[i] < '0' || curr_argument[i] > '9')
			{
				fprintf(stderr, "Line number %d - illegal Configuration version, "
								"Provided: %s, should be a number\n", line_number, curr_argument);
				return FAILURE;
			}
		}
		PARSE_CONFIG_TEXT(configuration_binary_version, cnf_content->configuration_binary_version)
		params->conf_version_filled = true;
	}
	else if (strcmp(curr_command, "MACaddress") == 0)
	{

		if(params->mac_addr_filled)
		{
			fprintf(stderr, "mac address filled multiple times\n");
			return FAILURE;
		}

		if (len < MAC_ADDR_LEN)
		{
			fprintf(stderr, "Line number %d - mac_address too short\n", line_number);
			return FAILURE;
		}

		if (!is_valid_mac_addr(curr_argument))
		{
			fprintf(stderr, "Line number %d - illegal mac address\n", line_number);
			return FAILURE;
		}
		str_to_mac(curr_argument, &cnf_content->mac_address);
		params->mac_addr_filled = true;
	}
	else if (strcmp(curr_command, "NMK") == 0)
	{
		if(params->nmk_filled)
		{
			fprintf(stderr, "NMK filled multiple times\n");
			return FAILURE;
		}

		if (len < sizeof(hpgp_nmk_t) * 2)
		{

			fprintf(stderr, "Line number %d - nmk too short\n", line_number);
			return FAILURE;
		}
		str_to_binary(curr_argument, sizeof(hpgp_nmk_t) * 2, cnf_content->nmk.NMK);
		params->nmk_filled = true;
	}
	else if (strcmp(curr_command, "CCoRole") == 0)
	{

		if(params->cco_role_filled)
		{
			fprintf(stderr, "CCo role filled multiple times\n");
			return FAILURE;
		}

		if(curr_argument[0] != '0' && curr_argument[0] != '1' && curr_argument[0] != '2')
		{
			fprintf(stderr, "Line number %d - Illegal CCo role value, should be 0, 1 or 2"
								", given: %s\n", line_number, curr_argument);
			return FAILURE;
		}
		if(curr_argument[1] != '\0' && curr_argument[1] != '\n')
		{
			fprintf(stderr, "Line number %d - Illegal CCo role value, should be 0, 1 or 2"
								", given: %s\n", line_number, curr_argument);
			return FAILURE;
		}
		PARSE_CONFIG_TEXT(cco_role, cnf_content->cco_role)
		params->cco_role_filled = true;
	}
	else if(curr_command[0] == '#')
	{
		/* A comment line - skip it */
		return SUCCESS;
	}
#ifndef CG5315
	else if (strcmp(curr_command, "ManufacturerHFID") == 0)
	{
		if (PROD_parse_hfid_argument(&cnf_content->manu_hfid, curr_argument, "Manufacturer HFID"))
		{
			return FAILURE;
		}
	}
	else if (strcmp(curr_command, "UserHFID") == 0)
	{
		if (PROD_parse_hfid_argument(&cnf_content->user_hfid, curr_argument, "User HFID"))
		{
			return FAILURE;
		}
	}
	else if (strcmp(curr_command, "AVLNHFID") == 0)
	{
		if (PROD_parse_hfid_argument(&cnf_content->avln_hfid, curr_argument, "AVLN HFID"))
		{
			return FAILURE;
		}
	}
	else if (strcmp(curr_command, "HostInterface") == 0)
	{
		PARSE_CONFIG_TEXT(host_interface, cnf_content->host_interface)
	}
	else if (strcmp(curr_command, "PLCFrequencySelection") == 0)
	{
		PARSE_CONFIG_TEXT(plc_freq_sel, cnf_content->plc_freq_sel)
		if (cnf_content->plc_freq_sel != HPGP_PLC_FREQ_SELECTION_50_HZ &&
			cnf_content->plc_freq_sel != HPGP_PLC_FREQ_SELECTION_60_HZ &&
			cnf_content->plc_freq_sel != HPGP_PLC_FREQ_SELECTION_EXTERNAL_SIGNAL)
		{
			fprintf(stderr, "Line number %d - Illegal PLC Frequency Selection value, should be 0,1 or 2"
							", given: %s\n", line_number, curr_argument);
			return FAILURE;
		}
	}
	else if (strcmp(curr_command, "NID") == 0)
	{
		if (len < sizeof(hpgp_nid_t) * 2)
		{

			fprintf(stderr, "Line number %d - nid too short\n", line_number);
			return FAILURE;
		}
		str_to_binary(curr_argument, sizeof(hpgp_nid_t) * 2, cnf_content->nid.NID);
	}
	else if (strcmp(curr_command, "SecurityLevel") == 0)
	{
		PARSE_CONFIG_TEXT(security_level, cnf_content->security_level)
		if (cnf_content->security_level != HPGP_SECURITY_LEVEL_SC &&
			cnf_content->security_level != HPGP_SECURITY_LEVEL_HS)
		{
			fprintf(stderr, "Line number %d - Illegal Security Level value, should be 0,1"
							", given: %s\n", line_number, curr_argument);
			return FAILURE;
		}
	}
	else if (strcmp(curr_command, "DecreaseMaximalReceiverSensitivity") == 0)
	{
		uint8_t receiver_max_sensitivity_dec;

		PARSE_CONFIG_TEXT(receiver_max_sensitivity_dec, receiver_max_sensitivity_dec)
		if (receiver_max_sensitivity_dec > 40)
		{
			fprintf(stderr, "Line number %d - Illegal Decrease Maximal Receiver Sensitivity value, should be [0,40]"
							", given: %s\n", line_number, curr_argument);
			return FAILURE;
		}

		cnf_content->receiver_max_sensitivity = MAX_RECEIVER_SENSITIVITY_DB - receiver_max_sensitivity_dec;
	}
	else if (strcmp(curr_command, "RXD_0/1/2/3RX_DVDriveStrength") == 0)
	{
		if (pin_drive_strength_str_to_enum(curr_argument, &cnf_content->cg_drive_strength_rxd_0_1_2_3_and_rx_dv))
		{
			fprintf(stderr, "Line number %d - Illegal RXD_0/1/2/3 RX_DV Drive Strength value, should be "
							CG_PIN_DRIVE_STRENGTH_8_mA_STRING ", "
							CG_PIN_DRIVE_STRENGTH_4_mA_STRING " or "
							CG_PIN_DRIVE_STRENGTH_8_mA_W_SLEW_RATE_STRING
							", given: %s\n", line_number, curr_argument);
			return FAILURE;
		}
	}
	else if (strcmp(curr_command, "TX_CLKDriveStrength") == 0)
	{
		if (pin_drive_strength_str_to_enum(curr_argument, &cnf_content->cg_drive_strength_tx_clk))
		{
			fprintf(stderr, "Line number %d - Illegal TX_CLK Drive Strength value, should be "
							CG_PIN_DRIVE_STRENGTH_8_mA_STRING ", "
							CG_PIN_DRIVE_STRENGTH_4_mA_STRING " or "
							CG_PIN_DRIVE_STRENGTH_8_mA_W_SLEW_RATE_STRING
							", given: %s\n", line_number, curr_argument);
			return FAILURE;
		}
	}
	else if (strcmp(curr_command, "PSDgain") == 0)
	{
		cnf_content->PSD_gain = strtod(curr_argument, &eptr);
		if (((cnf_content->PSD_gain == 0.0) && (curr_argument[0] != '0')) ||
			(((cnf_content->PSD_gain - (int)cnf_content->PSD_gain) != 0) && ((cnf_content->PSD_gain - (int)cnf_content->PSD_gain) != 0.5)) ||
			((cnf_content->PSD_gain < 0) || (cnf_content->PSD_gain > 15)))
		{
			/* strtod returned error */
			fprintf(stderr, "Line number %d - Illegal PSD gain value, should be [0, 15] with 0.5 increments"
							", given: %s\n", line_number, curr_argument);
			return FAILURE;
		}
	}
#endif
	else
	{
		curr = strstr(curr_command, "Amplitude[");
		if (curr)
		{
			curr += strlen("Amplitude[");

			/* Check for OFF special case */
			if((curr_argument[0] == 'O' || curr_argument[0] == 'o')
				&& (curr_argument[1] == 'F' || curr_argument[1] =='f')
				&& (curr_argument[2] == 'F' || curr_argument[2] == 'f'))
			{
				cnf_content->amplitude_map.amdata[atoi(curr)] = OFF_SPECIAL_CASE;
			}
			else
			{
				cnf_content->amplitude_map.amdata[atoi(curr)] = strtod(curr_argument, &eptr);
				/* strtod returned error */
				if (cnf_content->amplitude_map.amdata[atoi(curr)] == 0.0 && curr_argument[0] != '0')
				{
					return FAILURE;
				}
				params->amp_vals_filled++;
			}	
			
		}
		else
		{
			printf("Line number %d - unrecognised line in text configuration file, line:%s\n", line_number,
					line);
			return FAILURE;
		}
	}
	return SUCCESS;
}

static void init_config_params_flags(filled_config_params_flags *params)
{
	params->cco_role_filled = false;
	params->conf_version_filled = false;
	params->mac_addr_filled = false;
	params->nmk_filled = false;
	params->amp_vals_filled = 0;
}

static int check_config_params_flags(filled_config_params_flags *params)
{
	int res = SUCCESS;
	if(!params->cco_role_filled)
	{
		fprintf(stderr, "No CCo role was provided\n");
		res = FAILURE;
	}
	if(!params->conf_version_filled)
	{
		fprintf(stderr, "No configuration binary version was provided\n");
		res = FAILURE;
	}
	if(!params->mac_addr_filled)
	{
		fprintf(stderr, "No mac address was provided\n");
		res = FAILURE;
	}
	if(!params->nmk_filled)
	{
		fprintf(stderr, "No NMK was provided\n");
		res = FAILURE;
	}
	if(USER_AMDATA_LEN - params->amp_vals_filled > 0)
	{
		if(params->amp_vals_filled == 0)
		{
			fprintf(stderr, "Using default value %d for all Amplitude values\n",
								DEFAULT_AMP_MAP_VALUE);
		}
		else
		{
			fprintf(stderr, "Using default value %d for %d Amplitude fields\n",
						DEFAULT_AMP_MAP_VALUE,
						USER_AMDATA_LEN - params->amp_vals_filled);
		}
	}
	else if(params->amp_vals_filled > USER_AMDATA_LEN)
	{
		fprintf(stderr, "%d extra unneeded Amplitude fields were filled\n",
						params->amp_vals_filled - USER_AMDATA_LEN);
		res = FAILURE;
	}
	return res;
}

static int PROD_parse_text_config(FILE *src, PROD_configuration_content_t *cnf_content)
{
	int res;
	char str[MAX_CONFIG_LINE_LEN];
	int counter = 0;
	filled_config_params_flags params;
	init_config_params_flags(&params);

	while (fgets(str, MAX_CONFIG_LINE_LEN, src) != NULL)
	{
		counter++;
		if(strlen(str) > MIN_CONFIG_LINE_LEN)
		{
			res = PROD_process_config_line(str, cnf_content, counter, &params);
			if (res == FAILURE)
			{
				return FAILURE;
			}
		}
	}

	return check_config_params_flags(&params);
}

static int PROD_create_config(FILE *src, FILE *dst)
{
	size_t size;
	PROD_configuration_content_t cnf_content = PRD_CONFIG_INIT;

	PROD_configuration_content_packed_t cnf_content_packed = {0};
	int res;
	if (src)
	{
		res = PROD_parse_text_config(src, &cnf_content);
		if(res != SUCCESS)
		{
			return res;
		}
	}
	size = sizeof(cnf_content_packed);
	res = PROD_configuration_binary_content_create(&cnf_content, &cnf_content_packed, &size);
	if(res != SUCCESS)
	{
		return res;
	}
	fwrite(&cnf_content_packed, 1, sizeof(cnf_content_packed), dst);

	return res;
}

static double calc_hw_frequency(size_t index, hw_tool_type type)
{
	switch(type)
	{
		case CE2:
			return ((double)index/PROD_CE2_SIZE) * 50;
		case LNOE:
			return ((double)index/LNOE_DATA_LEN) * 50;
		case SNRE:
			return ((double)index/PROD_SNRE_DATA_SIZE) * 50;
	}
}

static void init_hw_tools_params_flags(filled_hw_tools_params_flags *params)
{
	params->gain_filled = false;
	params->ce2_shift_filled = false;
	params->ce2_vals_filled = 0;
	params->lnoe_vals_filled = 0;
	params->snre_vals_filled = 0;
}

static int check_hw_tools_params_flags(filled_hw_tools_params_flags *params, hw_tool_type type)
{
	int res = SUCCESS;
	if((type == CE2 || type == LNOE) && !params->gain_filled)
	{
		fprintf(stderr, "No gain was provided\n");
		res = FAILURE;
	}

	if(type == CE2 && !params->ce2_shift_filled)
	{
		fprintf(stderr, "No ce2_shift was provided\n");
		res = FAILURE;
	}

	if(type == CE2 && params->ce2_vals_filled != PROD_CE2_SIZE)
	{
		fprintf(stderr, "%d were filled, %d are needed\n",
						params->ce2_vals_filled, PROD_CE2_SIZE);
		res = FAILURE;
	}

	if(type == LNOE && params->lnoe_vals_filled != LNOE_DATA_LEN)
	{
		fprintf(stderr, "%d were filled, %d are needed\n",
						params->lnoe_vals_filled, LNOE_DATA_LEN);
		res = FAILURE;
	}

	if(type == SNRE && params->snre_vals_filled != PROD_SNRE_DATA_SIZE)
	{
		fprintf(stderr, "%d were filled, %d are needed\n",
						params->snre_vals_filled, PROD_SNRE_DATA_SIZE);
		res = FAILURE;
	}

	if (type == AMP_MAP && params->amp_map_vals_filled != PROD_AMP_MAP_DATA_SIZE)
	{
		fprintf(stderr, "%d were filled, %d are needed\n",
						params->amp_map_vals_filled, PROD_AMP_MAP_DATA_SIZE);
		res = FAILURE;
	}

	return res;
}

static int PROD_process_estimation_line(char *line, PROD_ce2_info_t *ce2_info, int line_number,
										filled_hw_tools_params_flags *params)
{
	char curr_command[MAX_CONFIG_LINE_LEN];
	char curr_argument[MAX_CONFIG_LINE_LEN];

	if(is_empty_line(line))
	{
		/* An empty line - skip it */
		return SUCCESS;
	}

	PROD_extract_command(line, curr_command, curr_argument);
	size_t len = strlen(curr_argument);
	size_t args_len = 0;
	char *curr;
	char *eptr;
	int i;

	if (strcmp(curr_command, "gain") == 0)
	{
		if(params->gain_filled)
		{
			fprintf(stderr, "Gain filled multiple times\n");
			return FAILURE;
		}

		for(i = 0;;i++)
		{

			if(curr_argument[i] == '\n' || curr_argument[i] == 0)
			{
				break;
			}
			if((curr_argument[i] < '0' || curr_argument[i] > '9') && curr_argument[i] != '-')
			{
				fprintf(stderr, "Line number %d - illegal Gain, "
								"Provided: %s, should be a number\n", line_number, curr_argument);
				return FAILURE;
			}
		}
		PARSE_CONFIG_TEXT(gain, ce2_info->gain)
		params->gain_filled = true;
	}
	else if (strcmp(curr_command, "ce2_shift") == 0)
	{

		if(params->ce2_shift_filled)
		{
			fprintf(stderr, "ce2_shift filled multiple times\n");
			return FAILURE;
		}

		for(i = 0;;i++)
		{

			if(curr_argument[i] == '\n' || curr_argument[i] == 0)
			{
				break;
			}
			if((curr_argument[i] < '0' || curr_argument[i] > '9') && curr_argument[i] != '-')
			{
				fprintf(stderr, "Line number %d - illegal ce2_shift, "
								"Provided: %s, should be a number\n", line_number, curr_argument);
				return FAILURE;
			}
		}
		PARSE_CONFIG_TEXT(ce2_shift, ce2_info->ce2_shift)
		params->ce2_shift_filled = true;
	}
	else if(curr_command[0] == '#')
	{
		/* A comment line - skip it */
		return SUCCESS;
	}
	else
	{
		curr = strstr(curr_command, "ce2[");
		if (curr)
		{
			curr += strlen("ce2[");
			ce2_info->ce2[atoi(curr)] = atoi(curr_argument);
			/* strtod returned error */
			if (ce2_info->ce2[atoi(curr)] == 0 && curr_argument[0] != '0')
			{
				return FAILURE;
			}
			params->ce2_vals_filled++;	
		}
		else
		{
			printf("Line number %d - unrecognised line in text configuration file, line:%s\n", line_number,
					line);
			return FAILURE;
		}
	}
	return SUCCESS;
}

static int PROD_process_lnoe_line(char *line, PROD_lnoe_info_t *lnoe_content, int line_number,
									filled_hw_tools_params_flags *params)
{
	char curr_command[MAX_CONFIG_LINE_LEN];
	char curr_argument[MAX_CONFIG_LINE_LEN];

	if(is_empty_line(line))
	{
		/* An empty line - skip it */
		return SUCCESS;
	}

	PROD_extract_command(line, curr_command, curr_argument);
	size_t len = strlen(curr_argument);
	size_t args_len = 0;
	char *curr;
	char *eptr;
	int i;

	if (strcmp(curr_command, "gain") == 0)
	{
		if(params->gain_filled)
		{
			fprintf(stderr, "Gain filled multiple times\n");
			return FAILURE;
		}

		for(i = 0;;i++)
		{

			if(curr_argument[i] == '\n' || curr_argument[i] == 0)
			{
				break;
			}
			if((curr_argument[i] < '0' || curr_argument[i] > '9') && curr_argument[i] != '-')
			{
				fprintf(stderr, "Line number %d - illegal Gain, "
								"Provided: %s, should be a number\n", line_number, curr_argument);
				return FAILURE;
			}
		}
		PARSE_CONFIG_TEXT(gain, lnoe_content->gain)
		params->gain_filled = true;
	}
	else if(curr_command[0] == '#')
	{
		/* A comment line - skip it */
		return SUCCESS;
	}
	else
	{
		curr = strstr(curr_command, "lnoe[");
		if (curr)
		{
			curr += strlen("lnoe[");
			lnoe_content->lnoe[atoi(curr)] = atoi(curr_argument);
			/* strtod returned error */
			if (lnoe_content->lnoe[atoi(curr)] == 0 && curr_argument[0] != '0')
			{
				return FAILURE;
			}
			params->lnoe_vals_filled++;	
		}
		else
		{
			printf("Line number %d - unrecognised line in text configuration file, line:%s\n", line_number,
					line);
			return FAILURE;
		}
	}
	return SUCCESS;
}

static int PROD_process_snr_line(char *line, PROD_snre_info_t *snr_info, int line_number,
									filled_hw_tools_params_flags *params)
{
	char curr_command[MAX_CONFIG_LINE_LEN];
	char curr_argument[MAX_CONFIG_LINE_LEN];

	if(is_empty_line(line))
	{
		/* An empty line - skip it */
		return SUCCESS;
	}

	PROD_extract_command(line, curr_command, curr_argument);
	size_t len = strlen(curr_argument);
	size_t args_len = 0;
	char *curr;
	char *eptr;
	int i;

	if(curr_command[0] == '#')
	{
		/* A comment line - skip it */
		return SUCCESS;
	}
	else
	{
		curr = strstr(curr_command, "snre[");
		if (curr)
		{
			curr += strlen("snre[");
			snr_info->snre[atoi(curr)] = atoi(curr_argument);
			/* strtod returned error */
			if (snr_info->snre[atoi(curr)] == 0 && curr_argument[0] != '0')
			{
				return FAILURE;
			}
			params->snre_vals_filled++;	
		}
		else
		{
			printf("Line number %d - unrecognised line in text configuration file, line:%s\n", line_number,
					line);
			return FAILURE;
		}
	}
	return SUCCESS;
}

static int PROD_process_amp_map_line(char *line, PROD_hw_amp_map_t *amp_map, int line_number,
									 filled_hw_tools_params_flags *params)
{
	char curr_command[MAX_CONFIG_LINE_LEN];
	char curr_argument[MAX_CONFIG_LINE_LEN];

	if(is_empty_line(line))
	{
		/* An empty line - skip it */
		return SUCCESS;
	}

	PROD_extract_command(line, curr_command, curr_argument);
	size_t len = strlen(curr_argument);
	size_t args_len = 0;
	char *curr;
	char *eptr;
	int i;

	if(curr_command[0] == '#')
	{
		/* A comment line - skip it */
		return SUCCESS;
	}
	else
	{
		curr = strstr(curr_command, "Amp[");
		if (curr)
		{
			curr += strlen("Amp[");
			amp_map->amp_map[atoi(curr)] = atoi(curr_argument);
			/* strtod returned error */
			if (amp_map->amp_map[atoi(curr)] == 0 && curr_argument[0] != '0')
			{
				return FAILURE;
			}
			params->amp_map_vals_filled++;
		}
		else
		{
			printf("Line number %d - unrecognised line in text configuration file, line:%s\n", line_number,
					line);
			return FAILURE;
		}
	}

	return SUCCESS;
}

static int PROD_parse_text_hw_tools(FILE *src, void *tool_info, hw_tool_type type)
{
	int res;
	char str[MAX_CONFIG_LINE_LEN];
	int counter = 0;
	filled_hw_tools_params_flags params;
	init_hw_tools_params_flags(&params);

	while (fgets(str, MAX_CONFIG_LINE_LEN, src) != NULL)
	{
		counter++;
		if(strlen(str) > MIN_CONFIG_LINE_LEN)
		{
			if(type == CE2)
			{
				res = PROD_process_estimation_line(str, (PROD_ce2_info_t *)tool_info,
												counter, &params);
			}
			else if(type == LNOE)
			{
				res = PROD_process_lnoe_line(str, (PROD_lnoe_info_t *)tool_info,
												counter, &params);
			}
			else if (type == AMP_MAP)
			{
				res = PROD_process_amp_map_line(str, (PROD_hw_amp_map_t *)tool_info,
												counter, &params);
			}
			else
			{
				res = PROD_process_snr_line(str, (PROD_snre_info_t *)tool_info,
												counter, &params);
			}
			
			if (res == FAILURE)
			{
				return FAILURE;
			}
		}
	}

	return check_hw_tools_params_flags(&params, type);
}

static int PROD_parse_channel_estimation(FILE *src, FILE *dst)
{
	PROD_ce2_info_t ce2_info;
	double ce2_mag[PROD_CE2_SIZE];
	char buf[MAX_CONFIG_LINE_LEN];
	int res;
	if (src)
	{
		res = PROD_parse_text_hw_tools(src, &ce2_info, CE2);
		if(res != SUCCESS)
		{
			return res;
		}
	}

	res = PROD_channel_estimation_create(&ce2_info, ce2_mag);
	if(res != SUCCESS)
	{
		return res;
	}

	for (size_t i = 0; i < PROD_CE2_SIZE; i++)
	{	
		res = snprintf(buf, sizeof(buf), "%ld %.2f %.4f\n",
					   i, calc_hw_frequency(i, CE2), ce2_mag[i]);
		if (res < 0)
		{
			fprintf(stderr, "Failed to parse ce2_mag [%zu]!\n", i);
			return FAILURE;
		}
		if (buf[res-1] == '0')
		{
			/* we prefer XX over XX.0 in the file*/
			buf[res-2] = 0;
		}
		fputs(buf, dst);
	}

	return SUCCESS;
}

static int PROD_parse_line_noise_estimation(FILE *src, FILE *dst)
{
	PROD_lnoe_info_t lnoe_info;
	double lnoe_mag[LNOE_DATA_LEN];
	char buf[MAX_CONFIG_LINE_LEN];
	int res;
	if (src)
	{
		res = PROD_parse_text_hw_tools(src, &lnoe_info, LNOE);
		if(res != SUCCESS)
		{
			return res;
		}
	}

	res = PROD_lnoe_create(&lnoe_info, lnoe_mag);
	if(res != SUCCESS)
	{
		return res;
	}

	for (size_t i = 0; i < LNOE_DATA_LEN; i++)
	{	
		res = snprintf(buf, sizeof(buf), "%ld %.2f %.4f\n",
					   i, calc_hw_frequency(i, LNOE), lnoe_mag[i]);
		if (res < 0)
		{
			fprintf(stderr, "Failed to parse lnoe_mag [%zu]!\n", i);
			return FAILURE;
		}
		if (buf[res-1] == '0')
		{
			/* we prefer XX over XX.0 in the file*/
			buf[res-2] = 0;
		}
		fputs(buf, dst);
	}

	return SUCCESS;
}

static int PROD_parse_amplitude_map(FILE *src, FILE *dst)
{
	PROD_hw_amp_map_t hw_amp_map = { 0 };
	char buf[MAX_CONFIG_LINE_LEN];
	amplitude_map_t amp_map = { 0 };
	int res;

	res = PROD_parse_text_hw_tools(src, &hw_amp_map, AMP_MAP);
	if(res != SUCCESS)
	{
		return res;
	}

	res = PROD_amplitude_map_parse(&hw_amp_map, &amp_map);
	if(res != SUCCESS)
	{
		return res;
	}

	fputs("#Amplitude[0-1154]: Range 0.0 - 68.0 [dB] or OFF\n", dst);
	for (size_t i = 0; i < USER_AMDATA_LEN; i++)
	{
		if(amp_map.amdata[i] == OFF_SPECIAL_CASE)
		{
			res = snprintf(buf, sizeof(buf), "Amplitude [%ld]=OFF\n", i);
			if (res < 0)
			{
				return FAILURE;
			}
		}
		else
		{
			res = snprintf(buf, sizeof(buf), "Amplitude [%ld]=%.1f\n",
						   i, amp_map.amdata[i]);
			if (res < 0)
			{
				return FAILURE;
			}
			if (buf[res-2] == '0')
			{
				/* we prefer XX over XX.0 in the file*/
				buf[res-3] = '\n';
				buf[res-2] = '\0';
			}
		}
		fputs(buf, dst);
	}

	return SUCCESS;
}

static int PROD_parse_snr(FILE *src, FILE *dst)
{
	PROD_snre_info_t snr_info;
	double snr_db[PROD_SNRE_DATA_SIZE];
	char buf[MAX_CONFIG_LINE_LEN];
	int res;
	if (src)
	{
		res = PROD_parse_text_hw_tools(src, &snr_info, SNRE);
		if(res != SUCCESS)
		{
			return res;
		}
	}

	res = PROD_snr_create(&snr_info, snr_db);
	if(res != SUCCESS)
	{
		return res;
	}

	for (size_t i = 0; i < PROD_SNRE_DATA_SIZE; i++)
	{	
		res = snprintf(buf, sizeof(buf), "%ld %.2f %.4f\n",
					   i, calc_hw_frequency(i, SNRE), snr_db[i]);
		if (res < 0)
		{
			fprintf(stderr, "Failed to parse snr_db [%zu]!\n", i);
			return FAILURE;
		}
		if (buf[res-1] == '0')
		{
			/* we prefer XX over XX.0 in the file*/
			buf[res-2] = 0;
		}
		fputs(buf, dst);
	}

	return SUCCESS;
}

static void init_noise_floor_params_flags(filled_noise_floor_params_flags *params)
{
	params->gain_filled = false;
	params->iterations_filled = false;
	params->sample_size_filled = false;
	params->sample_vals_filled = 0;
}

static int PROD_process_noise_floor_gain_iter_line(char *line, PROD_time_domain_t *time_domain,
													int line_number,
													filled_noise_floor_params_flags *params)
{
	char curr_command[MAX_CONFIG_LINE_LEN];
	char curr_argument[MAX_CONFIG_LINE_LEN];

	PROD_extract_command(line, curr_command, curr_argument);
	size_t len = strlen(curr_argument);
	size_t args_len = 0;
	char *curr;
	char *eptr;
	int i;

	if (strcmp(curr_command, "Gain") == 0)
	{
		for(i = 0;;i++)
		{

			if(curr_argument[i] == '\n' || curr_argument[i] == 0)
			{
				break;
			}
			if((curr_argument[i] < '0' || curr_argument[i] > '9') && curr_argument[i] != '-')
			{
				fprintf(stderr, "Line number %d - illegal Gain, "
								"Provided: %s, should be a number\n", line_number, curr_argument);
				return FAILURE;
			}
		}
		PARSE_CONFIG_TEXT(Gain, time_domain->gain)
		params->gain_filled = true;
	}
	else if (strcmp(curr_command, "Iterations") == 0)
	{
		for(i = 0;;i++)
		{

			if(curr_argument[i] == '\n' || curr_argument[i] == 0)
			{
				break;
			}
			if(curr_argument[i] < '0' || curr_argument[i] > '9')
			{
				fprintf(stderr, "Line number %d - illegal Iterations, "
								"Provided: %s, should be a number\n", line_number, curr_argument);
				return FAILURE;
			}
		}
		PARSE_CONFIG_TEXT(Iterations, time_domain->iterations_num)
		params->iterations_filled = true;
	}
	else if (strcmp(curr_command, "Sample_Size") == 0)
	{
		for(i = 0;;i++)
		{

			if(curr_argument[i] == '\n' || curr_argument[i] == 0)
			{
				break;
			}
			if(curr_argument[i] < '0' || curr_argument[i] > '9')
			{
				fprintf(stderr, "Line number %d - illegal Sample_Size, "
								"Provided: %s, should be a number\n", line_number, curr_argument);
				return FAILURE;
			}
		}
		PARSE_CONFIG_TEXT(Sample_Size, time_domain->sample_size)
		params->sample_size_filled = true;
	}

	return SUCCESS;
}

static int PROD_parse_noise_floor_gain_iter(char *src, PROD_time_domain_t *time_domain,
											filled_noise_floor_params_flags *params)
{
	
	int res;
	char str[MAX_CONFIG_LINE_LEN];
	FILE *fp_src = fopen(src, "r");
	int counter = 0;
	if (!fp_src)
	{
		fprintf(stderr, "failed to open source file for reading\n");
		return FAILURE;
	}

	while (fgets(str, MAX_CONFIG_LINE_LEN, fp_src) != NULL)
	{
		counter++;
		if(strlen(str) > MIN_CONFIG_LINE_LEN)
		{
			res = PROD_process_noise_floor_gain_iter_line(str, time_domain, counter, params);
			if (res == FAILURE)
			{
				fclose(fp_src);
				return FAILURE;
			}
		}

		if(params->gain_filled && params->iterations_filled && params->sample_size_filled)
		{
			fclose(fp_src);
			return SUCCESS;
		}
	}

	fclose(fp_src);
	return FAILURE;
}

static int PROD_process_samples_line(char *line, PROD_time_domain_t *time_domain,
										int line_number, filled_noise_floor_params_flags *params)
{
	char curr_command[MAX_CONFIG_LINE_LEN];
	char curr_argument[MAX_CONFIG_LINE_LEN];

	PROD_extract_command(line, curr_command, curr_argument);
	size_t len = strlen(curr_argument);
	size_t args_len = 0;
	char *curr;

	curr = strstr(curr_command, "Sample[");
	if (curr)
	{
		curr += strlen("Sample[");
		time_domain->samples[atoi(curr)] = (int16_t)(atof(curr_argument) * TIME_DOMAIN_NORMALISER);
		params->sample_vals_filled++;	
	}

	return SUCCESS;
}

static int PROD_parse_noise_floor_samples(char *src, PROD_time_domain_t *time_domain,
											filled_noise_floor_params_flags *params,
											int samples_size)
{
	int res;
	char str[MAX_CONFIG_LINE_LEN];
	FILE *fp_src = fopen(src, "r");
	int counter = 2;
	if (!fp_src)
	{
		fprintf(stderr, "failed to open source file for reading\n");
		return FAILURE;
	}

	while (fgets(str, MAX_CONFIG_LINE_LEN, fp_src) != NULL)
	{
		counter++;
		if(strlen(str) > MIN_CONFIG_LINE_LEN)
		{
			res = PROD_process_samples_line(str, time_domain, counter, params);
			if (res == FAILURE)
			{
				fclose(fp_src);
				return FAILURE;
			}
		}

		if(params->sample_vals_filled == samples_size)
		{
			fclose(fp_src);
			return SUCCESS;
		}
	}

	fprintf(stderr, "Noise floor calculation needs %d samples, provided: %d\n",
			samples_size, params->sample_vals_filled);
	fclose(fp_src);
	return FAILURE;
}

static int PROD_parse_noise_floor_src(char *src, PROD_time_domain_t *time_domain)
{
	int res;
	filled_noise_floor_params_flags params;
	int samples_size;
	init_noise_floor_params_flags(&params);
	res = PROD_parse_noise_floor_gain_iter(src, time_domain, &params);
	if(res != SUCCESS)
	{
		fprintf(stderr, "Failed to parse Gain and Iterations!\n");
		return FAILURE;
	}

	samples_size = time_domain->sample_size * time_domain->iterations_num / 2;
	time_domain->samples = malloc(samples_size * sizeof(int16_t));
	
	res = PROD_parse_noise_floor_samples(src, time_domain, &params, samples_size);
	if(res != SUCCESS)
	{
		free(time_domain->samples);
		fprintf(stderr, "Failed to parse Noise Floor Samples!\n");
		return FAILURE;
	}

	return SUCCESS;
}

static int PROD_calc_noise_floor(char *src, FILE *dst)
{
	int res;
	PROD_time_domain_t time_domain;
	double frequency_samples[FREQ_SAMPLES_BUF_SIZE] = {0};
	char buf[MAX_CONFIG_LINE_LEN];

	/* Extract the time domain noise floor samples from the input file */
	res = PROD_parse_noise_floor_src(src, &time_domain);
	if(res != SUCCESS)
	{
		fprintf(stderr, "Failed to parse Noise Floor Source!\n");
		return FAILURE;
	}

	/* Convert the time domain samples to frequency domain samples */
	res = PROD_noise_floor_time_to_frequency(&time_domain, frequency_samples);
	if(res != SUCCESS)
	{
		free(time_domain.samples);
		fprintf(stderr, "Failed to convert time samples to frequency samples!\n");
		return FAILURE;
	}

	/* Write the frequency domain samples to the output file */
	for (size_t i = 0; i < FFTSIZE / 2 + 1; i++)
	{	
		res = snprintf(buf, sizeof(buf), "%f\t%f\n",
					   frequency_samples[i], ((double)i*FsMhz)/(double)FFTSIZE);
		if (res < 0)
		{
			free(time_domain.samples);
			fprintf(stderr, "Failed to parse frequency_samples [%zu]!\n", i);
			return FAILURE;
		}
		if (buf[res-1] == '0')
		{
			/* we prefer XX over XX.0 in the file*/
			buf[res-2] = 0;
		}
		fputs(buf, dst);
	}

	free(time_domain.samples);
	return RES_RESULT_OK;
}

static void print_version(void)
{
	printf("current version=%d.%d.%d.%d\n", MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION, BUILD_VERSION);
}

static void usage(void)
{
	printf("Usage: production_tool [Options]\n"
			"-a\n"
			"\tActions: \n"
			"\t\tcreate_config - take a user config text file and transform it into a BIN config file\n"
			"\t\tparse_config - take BIN config file and transform it into a user config text file\n"
			"\t\tparse_amp_map - take file containing FDP HW format amplitude map values and transform it to regular attenuation values (0dB - 68dB)\n"
			"\t\tparse_channel_estimation - take a raw channel samples file and transform it into a channel estimation file\n"
			"\t\tparse_line_noise_estimation - take a raw line noise samples file and transform it into a line noise estimation file\n"
			"\t\tparse_snr - take a raw snr samples file and transform it into a snr estimation file\n"
			"\t\tcalc_noise_floor - take a time domain noise floor samples file and transform it into a frequency domain noise floor file\n"
			"-s\n"
			"\tSource file, if not provided using default configuration\n"
			"-d\n"
			"\tDestination file\n"
			"-h, --help\n"
			"\tDisplay this help and exit\n"
			"-v, --version\n"
			"\tDisplay production library version and legal info and exit\n"
			"-f\n"
			"\tShow the format of the configuration text file\n"
			"Standard usage format:\n"
			"\tproduction_tool -a <action:create_config,parse_config,parse_channel_estimation,"
		   		"parse_line_noise_estimation,parse_snr> -s <source file> -d <dest file>\n"
			"Examples:\n"
			"create_config:\n"
			"\tproduction_tool -a create_config -s config.txt -d config.bin\n"
			"parse_config:\n"
			"\tproduction_tool -a parse_config -s config.bin -d config.txt\n"
			"Create default file usage format:\n"
			"\tproduction_tool -a <action:create_config,parse_config> -d <dest file>\n"
			"Default binary creation example:\n"
			"\tproduction_tool -a create_config -d config.bin\n"
			"Default text configuration creation example:\n"
			"\tproduction_tool -a parse_config -d config.txt\n"
			"parse_amp_map:\n"
			"\tproduction_tool -a parse_amp_map -s hw_amp_map.txt -d amp_map.txt\n"
			"parse_channel_estimation:\n"
			"\tproduction_tool -a parse_channel_estimation -s raw_ce2_samples.txt -d channel_estimation_output.txt\n"
			"parse_line_noise_estimation:\n"
			"\tproduction_tool -a parse_line_noise_estimation -s raw_lnoe_samples.txt -d line_noise_output.txt\n"
			"parse_snr:\n"
			"\tproduction_tool -a parse_snr -s raw_snr_samples.txt -d snr_output.txt\n"
			"calc_noise_floor:\n"
			"\tproduction_tool -a calc_noise_floor -s time_domain_samples.txt -d frequency_domain_output.txt\n");
}

static void show_format(void)
{
	printf("create_config:\n"
			"\tConfiguration text file format:\n\n"
			"\tConfiguration version=<non negative integer>\n"
			"\tMAC address=<XX:XX:XX:XX:XX:XX>\n"
			"\tNMK=<16 bytes in hex as AABBCC...>\n"
			"\tCCo Role=<0(Auto),1(never),2(always)>\n"
			"\tAmplitude [<index:0-1154>]=<0.0-68.0|OFF>\n\n"
			"parse_channel_estimation:\n"
			"\tChannel raw samples text file format:\n\n"
			"\tgain=<integer>\n"
			"\tce2_shift=<integer>\n"
			"\tce2[<index:0-2047>]=<integer>\n\n"
			"parse_line_noise_estimation:\n"
			"\tLine noise samples text file format:\n\n"
			"\tgain=<integer>\n"
			"\tlnoe[<index:0-255>]=<integer>\n\n"
			"parse_snr:\n"
			"\tSNR samples text file format:\n\n"
			"\tsnre[<index:0-511>]=<integer>\n\n"
			"calc_noise_floor:\n"
			"\tNoise floor time domain example file format:\n\n"
			"\tGain=<integer>\n"
			"\tIterations=<integer>\n"
			"\tSample[<index:0-65536*iterations/2>]=<integer>\n\n"
			);
}

int main(int argc, char *argv[])
{
	FILE *fp_src;
	FILE *fp_dst;
	char *src = NULL;
	char *dest = NULL;
	char *action = NULL;
	int res, c;
	char cnf_binary_content[CONFIGURATION_MAX_SIZE];
	size_t size = CONFIGURATION_MAX_SIZE;
	int create_empty_cnf_file = 0;

	if(argc < 2)
	{
		usage();
		return FAILURE;
	}

	if (strcmp(argv[1], "--version") == 0)
	{
		print_version();
		return SUCCESS;
	}

	if (strcmp(argv[1], "--help") == 0)
	{
		usage();
		return SUCCESS;
	}

	while ((c = getopt(argc, argv, "a:hfvs:d:")) != -1)
	{
		switch (c)
		{
		case 'a':
			action = optarg;
			break;
		case 'h':
			usage();
			return SUCCESS;
		case 'f':
			show_format();
			return SUCCESS;
		case 'v':
			print_version();
			return SUCCESS;
		case 's':
			src = optarg;
			break;
		case 'd':
			dest = optarg;
			break;
		default:
			usage();
			return FAILURE;
		}
	}

	if (!dest)
	{
		fprintf(stderr, "destination was not provided\n");
		return FAILURE;
	}

	if (!action)
	{
		fprintf(stderr, "action was not provided\n");
		return FAILURE;
	}

	if (strcmp(action, "parse_config") == 0)
	{
		fp_dst = fopen(dest, "w+");
		if (!fp_dst)
		{
			fprintf(stderr, "failed to open destination file for writing\n");
			return FAILURE;
		}

		if (!src)
		{
			printf("Using default configuration\n");
			res = PROD_parse_default_config(fp_dst);
			fclose(fp_dst);
			if (res == SUCCESS)
			{
				printf("Parse config succeeded!\n");
				printf("Text configuration created at::%s\n", dest);
				return SUCCESS;
			}
			else
			{
				fprintf(stderr, "Parse config failed!\n");
				return FAILURE;
			}
		}
		else
		{
			fp_src = fopen(src, "rb");
			if (!fp_src)
			{
				fclose(fp_dst);
				fprintf(stderr, "failed to open source file for reading\n");
				return FAILURE;
			}

			res = PROD_parse_config(fp_src, fp_dst);
			fclose(fp_src);
			fclose(fp_dst);
			if (res == SUCCESS)
			{
				printf("Parse config succeeded!\n");
				printf("Text configuration created at::%s\n", dest);
				return SUCCESS;
			}
			else
			{
				fprintf(stderr, "Parse config failed!\n");
				return FAILURE;
			}
		}
	}
	else if (strcmp(action, "create_config") == 0)
	{
		fp_dst = fopen(dest, "wb+");
		if (!fp_dst)
		{
			fprintf(stderr, "failed to open destination file for writing\n");
			return FAILURE;
		}

		if (!src)
		{
			printf("Using default configuration\n");
			res = PROD_create_config(NULL, fp_dst);
			fclose(fp_dst);
			if (res == SUCCESS)
			{
				printf("Create config succeeded!\n");
				printf("Configuration binary created at: %s\n", dest);
				return SUCCESS;
			}
			else
			{
				fprintf(stderr, "Create config failed!\n");
				return FAILURE;
			}
		}
		else
		{
			fp_src = fopen(src, "r");
			if (!fp_src)
			{
				fprintf(stderr, "failed to open source file for reading\n");
				return FAILURE;
			}
			res = PROD_create_config(fp_src, fp_dst);
			fclose(fp_src);
			fclose(fp_dst);
			if (res == SUCCESS)
			{
				printf("Create config succeeded!\n");
				printf("Configuration binary created at: %s\n", dest);
				return SUCCESS;
			}
			else
			{
				fprintf(stderr, "Create config failed!\n");
				return FAILURE;
			}
		}
	}
	else if(strcmp(action, "parse_channel_estimation") == 0 ||
			strcmp(action, "parse_line_noise_estimation") == 0 ||
			strcmp(action, "parse_snr") == 0 ||
			strcmp(action, "parse_amp_map") == 0)
	{
		char* hw_tool_type;
		fp_dst = fopen(dest, "w+");
		if (!fp_dst)
		{
			fprintf(stderr, "failed to open destination file for writing\n");
			return FAILURE;
		}

		if (!src)
		{
			fprintf(stderr, "no source file for reading provided\n");
			return FAILURE;
		}
		
		fp_src = fopen(src, "r");
		if (!fp_src)
		{
			fprintf(stderr, "failed to open source file for reading\n");
			return FAILURE;
		}

		if(strcmp(action, "parse_channel_estimation") == 0)
		{
			res = PROD_parse_channel_estimation(fp_src, fp_dst);
			hw_tool_type = "channel estimation";
		}
		else if(strcmp(action, "parse_line_noise_estimation") == 0)
		{
			res = PROD_parse_line_noise_estimation(fp_src, fp_dst);
			hw_tool_type = "line noise";
		}
		else if (strcmp(action, "parse_amp_map") == 0)
		{
			res = PROD_parse_amplitude_map(fp_src, fp_dst);
			hw_tool_type = "amplitude map";
		}
		else
		{
			res = PROD_parse_snr(fp_src, fp_dst);
			hw_tool_type = "SNR";
		}
		
		fclose(fp_src);
		fclose(fp_dst);
		if (res == SUCCESS)
		{
			printf("Parse %s succeeded!\n", hw_tool_type);
			printf("Output file at: %s\n", dest);
			return SUCCESS;
		}
		else
		{
			fprintf(stderr, "Parse %s failed!\n", hw_tool_type);
			return FAILURE;
		}
		
	}
	else if(strcmp(action, "calc_noise_floor") == 0)
	{
		fp_dst = fopen(dest, "w+");
		if (!fp_dst)
		{
			fprintf(stderr, "failed to open destination file for writing\n");
			return FAILURE;
		}

		res = PROD_calc_noise_floor(src, fp_dst);
		fclose(fp_dst);
		if (res == SUCCESS)
		{
			printf("Calculate noise floor succeeded!\n");
			printf("Text frequency domain file created at:%s\n", dest);
			return SUCCESS;
		}
		else
		{
			fprintf(stderr, "Calculate noise floor failed!\n");
			return FAILURE;
		}
	}
	else
	{
		fprintf(stderr, "Illegal action, action should be create_config / parse_config / parse_amp_map"
					" parse_channel_estimation / parse_line_noise_estimation / parse_snr ,"
					" instead given: %s\n", action);
		return FAILURE;
	}

	return SUCCESS;
}
