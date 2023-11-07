/********************************************************************
*
* Module Name: PROD_lib.c
* Design:
* Implement Configuration binary management and post processing of hw vectors
*
********************************************************************/
/*******************************************************************
* IMPORTS
********************************************************************/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <complex.h>
#include "version.h"
#include "PROD_lib.h"
#include "FFT.h"

#ifdef __linux__
#include "endian.h"

#define htoles(n) htole16(n)
#define htolel(n) htole32(n)
#define letohs(n) le16toh(n)
#define letohl(n) le32toh(n)
#define htobes(n) htobe16(n)
#define htobel(n) htobe32(n)
#define betohs(n) be16toh(n)
#define betohl(n) be32toh(n)

#else
/********************************************************************************
                        ENDIANESS DEFINITIONS
********************************************************************************/
#define swap16(n) ((uint16_t)((((n) << 8) & 0xFF00) | (((n) >> 8) & 0x00FF)))
#define swap32(n) (((swap16((n)&0xFFFF) << 16) & 0xFFFF0000) | \
                   (swap16(((n) >> 16) & 0xFFFF) & 0x0000FFFF))

/*
 *	Following is a set of Host TO Little Endian
 *  macros to support Big/Little endian machines
 *  If the target machine is big endian, please set
 *  BIG_ENDIAN_MACHINE symbol to 1
 */
#ifdef BIG_ENDIAN_MACHINE
#define htoles(n) swap16(n) /* convert host to little endian short */
#define htolel(n) swap32(n) /* convert host to little endian long */
#define letohs(n) swap16(n) /* convert little endian to host short */
#define letohl(n) swap32(n) /* convert little endian to host long */
#define htobes(n) (n)
#define htobel(n) (n)
#define betohs(n) (n)
#define betohl(n) (n)
#else
#define htoles(n) (n)
#define htolel(n) (n)
#define letohs(n) (n)
#define letohl(n) (n)
#define htobes(n) swap16(n) /* convert host to big endian short */
#define htobel(n) swap32(n) /* convert host to big endian long */
#define betohs(n) swap16(n) /* convert big endian to host short */
#define betohl(n) swap32(n) /* convert big endian to host long */
#endif                      /* BIG_ENDIAN_MACHINE */
#endif

#ifdef _WIN32
#  ifdef _WIN64
#	 include <inttypes.h>
#    define PRI_SIZET PRIu64
#  else
#	 include <inttypes.h>
#    define PRI_SIZET PRIu32
#  endif
#else
#  define PRI_SIZET "zu"
#endif

/*******************************************************************
* CONSTANTS
********************************************************************/
#define NUM_ACTIVE_TONES 917.0
#define LOWEST_ILLEGAL_BIN_AMP_VAL 1
#define HIGHEST_ILLEGAL_BIN_AMP_VAL 5
#define TX_PRE_EQ_OFFSET 74
#define LNOE_SIZE 256
#define SNRE_SIZE 512
#define NBUFF_RX 16384

/*!< Due to a limitation in the formula
 *   we need this to keep precision on the
 *   Max db value */
#define MAX_dB_VALUE 6
#define EPSILON 0.00001

#define MAX_ANALOG_GAIN (18)
#define DEFAULT_AMB_GAIN (11)
#define MIN_ANALOG_GAIN (-6)
#define DEFAULT_DIGITAL_GAIN (9)
#define MAX_DIGITAL_GAIN (40)
#define MIN_DIGITAL_GAIN (-6)
#define DEFAULT_AMB_TX_DAC_SHIFT_TABLE_IDX (13)
#define DEFAULT_AMB_TX_DAC_TABLE_IDX (12)
#define MAX_DAC_MUL_GAIN (5.5)
/*******************************************************************
* TYPES
********************************************************************/
/*******************************************************************
* STATIC and GLOBAL DATA
********************************************************************/
/*******************************************************************
* MACROS
********************************************************************/
#define PROD_is_invalid(val, min, max) (((val > max) || (val < min)))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

/*******************************************************************
* INTERNAL FUNCTIONS
********************************************************************/
unsigned int round_up_to_the_next_highest_power_of_2(unsigned int v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;

	return v;
}

static uint8_t Analog_Gain_to_gain_code(int8_t analog_gain)
{
	uint8_t res = 0;
	int8_t idx;
	uint8_t analog_gain_table[] = {
		0x00, 0x01, 0x02, 0x03,
		0x04, 0x05, 0x06,
		0x14, 0x15, 0x16,
		0x24, 0x25, 0x26,
		0x34, 0x35, 0x36,
		0x44, 0x45, 0x46,
		0x54, 0x55, 0x56,
		0x64, 0x65, 0x66,
	};

	idx = analog_gain - MIN_ANALOG_GAIN;
	if ((idx >= 0) && ((uint8_t)idx < sizeof(analog_gain_table)))
	{
		res = analog_gain_table[idx];
	}

	return res;
}

static uint16_t Amp_Map_to_TxPreEq(double amp)
{
	return MIN(MAX((uint16_t)(TXPREEQ_ZERO * pow(10, -amp / 20.0) + 0.5) , 0) , TXPREEQ_ZERO);
}

static double TxPreEq_to_Amp_Map(uint16_t tx_val)
{
	return (double)(-20.0 * (log10(tx_val / TXPREEQ_ZERO)));
}

static double Amp_Map_to_unquantizedTxPreEq(double amp)
{
	return pow(10, -amp / 20.0);	
}

/* Compute the sum of all the following members in the amplitude map binary struct
 * where X:Y means the range of all tones starting from X and ending at Y (inclusive)
 * [86:139 168:214 226:282 303:409 420:569 592:736 749:856 883:1015 1028:1143] */
static double sum_TxPreEq(const amplitude_map_t* amp_map)
{
	int i;
	double sum = 0;
	for (i = 86; i < 1144; i++)
	{
		switch(i)
		{
			/* Skip from 140 to 168 */
			case 140:
				i = 167;
				break;
			/* Skip from 215 to 226 */
			case 215:
				i = 225;
				break;
			/* Skip from 283 to 303 */	
			case 283:
				i = 302;
				break;
			/* Skip from 410 to 420 */	
			case 410:
				i = 419;
				break;
			/* Skip from 570 to 592 */
			case 570:
				i = 591;
				break;
			/* Skip from 737 to 749 */	
			case 737:
				i = 748;
				break;
			/* Skip from 857 to 883 */	
			case 857:
				i = 882;
				break;
			/* Skip from 1016 to 1028 */	
			case 1016:
				i = 1027;
				break;			
			default:
				sum += Amp_Map_to_unquantizedTxPreEq(amp_map->amdata[i]);

		}
	}
	return sum;

}

/* DigitalLinearAttenuation = sum(TxPreEq) / NUM_ACTIVE_TONES */
static double calc_DigitalLinearAttenuation(const amplitude_map_t* amp_map)
{
	return sum_TxPreEq(amp_map) / NUM_ACTIVE_TONES;
}

/* DigitalGain =  |20⋅log10(DigitalLinearAttenuation)| */
static double calc_DigitalGain(const amplitude_map_t* amp_map)
{	
	return fabs(20 * log10(calc_DigitalLinearAttenuation(amp_map)));
}

/* S = floor(DACscalingGain /6) */
static uint32_t calc_S(double dac_scaling_gain)
{
	return (uint32_t)(dac_scaling_gain / 6.0);
}

/* TDU_DACS_SHIFT = S + 1 */
static uint32_t calc_tdu_dacs_shift(uint32_t S)
{
	return S + 1;
}

/* DAClinearScalingGain = 10^(DACscalingGain/20)
 * B =  DAClinearScalingGain / 2^S 
 * B_Quant = floor(B * 2^4 + 0.5)/2^4									
 * TDU_DACS_B1 = (B_Quant - 1)*2^4  */
static uint32_t clac_tdu_dacs_b1(uint32_t S, double dac_scaling_gain)
{
	double DAClinearScalingGain = pow(10, dac_scaling_gain / 20.0);
	double B = DAClinearScalingGain / (pow(2, S));
	double B_Quant = (((uint32_t)(B * pow(2, 4) + 0.5)) / pow(2, 4));
	return (uint32_t)((B_Quant - 1.0) * pow(2, 4));
}

/* 10*log10((abs(ce2*2^(ce2_shift)/2^12).^2/(2048))) -88.48 - gain */
static RES_result_t calc_update_ce2(double complex ce2[PROD_CE2_SIZE],
									double updated_ce2[PROD_CE2_SIZE],
									int8_t gain, int16_t ce2_shift)
{
	int i;
	for(i = 0;i < PROD_CE2_SIZE;i++)
	{
		updated_ce2[i] = (10 * log10(pow(cabs(ce2[i] * ((double)(pow(2, ce2_shift) / pow(2, 12)))), 2) / 2048)) -
				88.48 - gain;
	}

	return RES_RESULT_OK;
}

/* 10*log10(abs(2.^(lnoe/8)/256/2^1))- 88.48 - gain */
static RES_result_t calc_update_lnoe(const int8_t lnoe[LNOE_SIZE], double updated_lnoe[LNOE_SIZE],
										int16_t gain)
{
	int i;
	for(i = 0;i < LNOE_SIZE;i++)
	{
		updated_lnoe[i] = (10 * log10((double)(pow(2, ((double)lnoe[i] / 8)) / 512))) - 88.48 - gain;
	}

	return RES_RESULT_OK;
}

/* 10*log10(2.^(SNRE/2^10)/2^8) */
static RES_result_t calc_update_snre(const int16_t snre[SNRE_SIZE], double updated_snre[SNRE_SIZE])
{
	int i;
	for(i = 0;i < SNRE_SIZE;i++)
	{
		updated_snre[i] = 10 * log10(pow(2, (snre[i] / pow(2, 10))) / pow(2, 8));
	}

	return RES_RESULT_OK;
}

static void int_to_complex(int32_t ce2, double complex *complex_ce2)
{
	int16_t real, img;
	real = (ce2 >> 16);
	img = (ce2 & 0x0000FFFF);
	*complex_ce2 = real + img * I;
}

static void normalise_freq_samples(double frequency_samples[FREQ_SAMPLES_BUF_SIZE], double gain)
{
	int i;
	for (i = 0;i < FFTSIZE / 2 + 1;i++)
	{
		frequency_samples[i] *= (2.0/(FsMhz*FFTSIZE));
		frequency_samples[i] /= 0.3752;
		frequency_samples[i] *= pow(10,gain/10);
		frequency_samples[i] = 10*log10(frequency_samples[i])-60;
	}
}
/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/
RES_result_t PROD_configuration_binary_content_create(const PROD_configuration_content_t *cnf_content,
													  void *msg, size_t *msg_len)
{
	PROD_configuration_content_packed_t *payload = (PROD_configuration_content_packed_t *)msg;
	int i;
	double dac_scaling_gain;
	uint32_t S;
	uint16_t TxPreEq[AMDATA_LEN * 2];
	if (*msg_len < sizeof(*payload))
	{
		fprintf(stderr, "provided buffer size (%"PRI_SIZET") is too small, should be at least %"PRI_SIZET"\n",
				*msg_len, sizeof(*payload));
		return RES_RESULT_NO_MEMORY;
	}

	if (PROD_is_invalid(cnf_content->cco_role, HPGP_CCO_MODE_AUTO, HPGP_CCO_MODE_ALWAYS))
	{
		fprintf(stderr, "HLB_configuration_binary_content_create cco_role bad param: %d,"
			" should be between %d and %d\n", cnf_content->cco_role, HPGP_CCO_MODE_AUTO,
			HPGP_CCO_MODE_ALWAYS);
		return RES_RESULT_BAD_PARAMETER;
	}

	*msg_len = sizeof(*payload);
	payload->min_conf.config.cco_role = cnf_content->cco_role;
	payload->min_conf.config.configuration_binary_version =
		htolel(cnf_content->configuration_binary_version);
	memcpy(&payload->min_conf.config.mac_address.macAddress, &cnf_content->mac_address,
		   sizeof(cnf_content->mac_address));
	memcpy(&payload->min_conf.config.nmk, &cnf_content->nmk, sizeof(cnf_content->nmk));
	payload->min_conf.config.version.version_major = htoles(MAJOR_VERSION);
	payload->min_conf.config.version.version_minor = htoles(MINOR_VERSION);
	payload->min_conf.config.version.version_patch = htoles(PATCH_VERSION);

	/* Convert amplitude map values between 74 and 1228
	 * according to the TX_PRE_EQ formula, the others get the
	 * default value of 16384 */
	for(i = 0;i < AMDATA_LEN * 2;i++)
	{
		if ((i <  TX_PRE_EQ_OFFSET) || (i >= (TX_PRE_EQ_OFFSET + USER_AMDATA_LEN)))
		{
			TxPreEq[i] = TXPREEQ_ZERO;
		}
		else
		{
			if(cnf_content->amplitude_map.amdata[i - TX_PRE_EQ_OFFSET] == OFF_SPECIAL_CASE)
			{
				/* If the user configuration struct contains an OFF value
				* write 0 in the binary configuration struct */
				TxPreEq[i] = 0;
			}
			else if((cnf_content->amplitude_map.amdata[i - TX_PRE_EQ_OFFSET] > MAX_AMDATA_VAL) ||
					(cnf_content->amplitude_map.amdata[i - TX_PRE_EQ_OFFSET] < MIN_AMDATA_VAL))
			{
				fprintf(stderr, "Illegal aplitude map data at Amplitude [%d]." 
						"Given: %.1f, should be between %.1f and %.1f or OFF\n",
						i - TX_PRE_EQ_OFFSET, cnf_content->amplitude_map.amdata[i - TX_PRE_EQ_OFFSET],
						MIN_AMDATA_VAL, MAX_AMDATA_VAL);
				return RES_RESULT_BAD_PARAMETER;
			}
			else
			{
				TxPreEq[i] = Amp_Map_to_TxPreEq(cnf_content->amplitude_map.amdata[i - TX_PRE_EQ_OFFSET]);
			}
		}
	}

	for (i = 0; i < AMDATA_LEN; i++)
	{
		payload->min_conf.amplitude_map.amdata[i] =
			htolel(((uint32_t)(TxPreEq[2*i])) + (uint32_t)(TxPreEq[2*i+1] << 16));
	}

	double PSD_gain = (double)cnf_content->PSD_gain;
	int8_t AnalogGain;
	double DigitalGain;
	uint8_t AmbTxDacTableIdx;

	if (PSD_gain >= 0.5)
	{
		/* GainCorrection = PSDgain */
		double GainCorrection = PSD_gain;
		/* AnalogGain = min(max(round(defaultAMBgain + GainCorrection)),0) ,MAX_ANALOG_GAIN) ,   defaultAMBgain = 11dB, max available gain 7dB */
		AnalogGain = MIN(MAX((int8_t)round(DEFAULT_AMB_GAIN + GainCorrection), 0), MAX_ANALOG_GAIN);
		/* deltaGain = AnalogGain - defaultAMBgain */
		int8_t deltaGain = AnalogGain - DEFAULT_AMB_GAIN;
		/* digital_amp_1= min(max(GainCorrection -  deltaGain), 0),MAX_DAC_MUL_GAIN) */
		double digital_amp_1 = MIN(MAX((GainCorrection -  deltaGain), 0), MAX_DAC_MUL_GAIN);
		/* Round to the nearest 0.5dB and AmbTxDacTableIdx= find(digital_amp_1== AFETXDECscale_tab) */
		digital_amp_1 = round(digital_amp_1 * 2);
		AmbTxDacTableIdx = DEFAULT_AMB_TX_DAC_TABLE_IDX + ((uint8_t)digital_amp_1);
		digital_amp_1 /= 2;
		/* digital_amp_2 = min(max((GainCorrection - deltaGain) - digital_amp_1) , 0),MAX_DIGITAL_GAIN) */
		double digital_amp_2 = MIN(MAX(((GainCorrection - deltaGain) - digital_amp_1), 0), MAX_DIGITAL_GAIN);
		/* DigitalGain = digital_amp_2 */
		DigitalGain = digital_amp_2;
	}
	else
	{
		double G1 = calc_DigitalGain(&cnf_content->amplitude_map);

		/* defualt value */
		AmbTxDacTableIdx = DEFAULT_AMB_TX_DAC_TABLE_IDX;

		/* GainCorrection =  G1 */
		double GainCorrection = G1;
		/* AnalogGain = min(max(round(defaultAMBgain - GainCorrection)), MIN_ANALOG_GAIN),MAX_ANALOG_GAIN)  // max available attenuation 17dB */
		AnalogGain = MIN(MAX((int8_t)round(DEFAULT_AMB_GAIN - GainCorrection), MIN_ANALOG_GAIN), MAX_ANALOG_GAIN);
		/* deltaGain = AnalogGain - defaultAMBgain */
		int8_t deltaGain = AnalogGain - DEFAULT_AMB_GAIN;
		/* digital_att = max(GainCorrection + deltaGain,0) */
		double digital_att = MAX(GainCorrection + deltaGain, 0);
		/* DigitalGain = G1 - digital_att */
		DigitalGain = G1 - digital_att;
	}

	/* DACscalingGain = min(max((DefaultDigitalGain+ DigitalGain),MIN_DIGITAL_GAIN),MAX_DIGITAL_GAIN)  (DefaultDigitalGain = 9dB) */
	dac_scaling_gain = MIN(MAX((DEFAULT_DIGITAL_GAIN + DigitalGain), MIN_DIGITAL_GAIN), MAX_DIGITAL_GAIN);

	S = calc_S(dac_scaling_gain);
	payload->min_conf.config.tdu_dacs_shift = htolel(calc_tdu_dacs_shift(S));
	payload->min_conf.config.tdu_dacs_b1 = htolel(clac_tdu_dacs_b1(S, dac_scaling_gain));

#ifndef CG5315
	if (PROD_is_invalid(cnf_content->host_interface, HPGP_SPI, HPGP_SPI_DBG))
	{
		fprintf(stderr, "HLB_configuration_binary_content_create host_interface bad param: %d,"
			" should be between %d and %d\n", cnf_content->host_interface, HPGP_SPI, HPGP_SPI_DBG);
		return RES_RESULT_BAD_PARAMETER;
	}

	payload->host_interface = cnf_content->host_interface;
	memcpy(&payload->user_hfid, &cnf_content->user_hfid, sizeof(payload->user_hfid));
	memcpy(&payload->manu_hfid, &cnf_content->manu_hfid, sizeof(payload->manu_hfid));
	memcpy(&payload->avln_hfid, &cnf_content->avln_hfid, sizeof(payload->avln_hfid));
	payload->plc_freq_sel = cnf_content->plc_freq_sel;
	memcpy(&payload->nid, &cnf_content->nid, sizeof(cnf_content->nid));
	payload->security_level = cnf_content->security_level;
	payload->receiver_max_sensitivity = cnf_content->receiver_max_sensitivity;
	payload->cg_drive_strength_rxd_0_1_2_3_and_rx_dv = cnf_content->cg_drive_strength_rxd_0_1_2_3_and_rx_dv;
	payload->cg_drive_strength_tx_clk = cnf_content->cg_drive_strength_tx_clk;
	memcpy((uint8_t*)&payload->host_internal_data_PSD_gain, (uint8_t*)&cnf_content->PSD_gain, sizeof(payload->host_internal_data_PSD_gain));
	payload->amb_tx_dac_table_idx = AmbTxDacTableIdx;
	payload->amb_tx_dac_shift_table_idx = DEFAULT_AMB_TX_DAC_SHIFT_TABLE_IDX;
	payload->analog_gain_code = Analog_Gain_to_gain_code(AnalogGain);
#endif
	return RES_RESULT_OK;
}

RES_result_t PROD_amplitude_map_parse(const PROD_hw_amp_map_t *hw_amp_map,
									  amplitude_map_t *amp_map)
{
	int i;

	for (i = TX_PRE_EQ_OFFSET; i < USER_AMDATA_LEN + TX_PRE_EQ_OFFSET; i++)
	{
		/* The OFF special case, write 100 in the user configuration struct*/
		if(hw_amp_map->amp_map[i] == 0)
		{
			amp_map->amdata[i - TX_PRE_EQ_OFFSET] = OFF_SPECIAL_CASE;
		}
		/* 1-5 are illegal values */
		else if(hw_amp_map->amp_map[i] >= LOWEST_ILLEGAL_BIN_AMP_VAL
				&& hw_amp_map->amp_map[i] <= HIGHEST_ILLEGAL_BIN_AMP_VAL)
		{
			fprintf(stderr, "Illegal Amplitude[%d] value : %d, amplitude values can't be"
					" between %d and %d or 0\n", i, hw_amp_map->amp_map[i],
					LOWEST_ILLEGAL_BIN_AMP_VAL, HIGHEST_ILLEGAL_BIN_AMP_VAL);
			return RES_RESULT_BAD_PARAMETER;
		}
		else if(hw_amp_map->amp_map[i] > (uint16_t)TXPREEQ_ZERO)
		{
			fprintf(stderr, "PROD_amplitude_map_parse amdata[%d] bad param: %d,"
					" should be between %d and %d or 0\n", i, hw_amp_map->amp_map[i],
					HIGHEST_ILLEGAL_BIN_AMP_VAL+1, (uint16_t)TXPREEQ_ZERO);
			return RES_RESULT_BAD_PARAMETER;
		}
		else if(hw_amp_map->amp_map[i] == MAX_dB_VALUE)
		{
			amp_map->amdata[i - TX_PRE_EQ_OFFSET] = MAX_AMDATA_VAL;
		}
		else
		{
			amp_map->amdata[i - TX_PRE_EQ_OFFSET] = TxPreEq_to_Amp_Map(hw_amp_map->amp_map[i]);

			/* Get rid of the '-0' entries that are generated when
			   the TxPreEq_to_Amp_Map gets the TXPREEQ_ZERO value */
			if(fabs(amp_map->amdata[i - TX_PRE_EQ_OFFSET] - 0.0 )< EPSILON)
			{
				amp_map->amdata[i - TX_PRE_EQ_OFFSET] = 0.0;
			}
		}
	}

	return RES_RESULT_OK;
}

RES_result_t PROD_configuration_binary_content_parse(const void *msg, size_t msg_len, PROD_configuration_content_t *cnf_content)
{
	int i;
	PROD_configuration_content_t temp = PRD_CONFIG_INIT;
	RES_result_t res;

	PROD_hw_amp_map_t hw_amp_map = { 0 };
	*cnf_content = temp;

	if((int)(sizeof(PROD_configuration_content_packed_t) - msg_len) < 0)
	{	
		fprintf(stderr, "Failed to parse configuration binary content, the file was created with a newer version of this tool\n");
		return RES_RESULT_BAD_PARAMETER;
	}
	PROD_configuration_content_packed_t *payload = (PROD_configuration_content_packed_t *)msg;

	if (PROD_is_invalid(payload->min_conf.config.cco_role,HPGP_CCO_MODE_AUTO, HPGP_CCO_MODE_ALWAYS))
	{
		fprintf(stderr, "HLB_configuration_binary_content_parse cco_role bad param: %d,"
			" should be between %d and %d\n", 
			cnf_content->cco_role, HPGP_CCO_MODE_ALWAYS, HPGP_CCO_MODE_AUTO);
		return RES_RESULT_BAD_PARAMETER;
	}

	for(i = 0;i < AMDATA_LEN;i++)
	{
		uint32_t curr_amp_val = letohl(payload->min_conf.amplitude_map.amdata[i]);
		hw_amp_map.amp_map[2 * i] = (uint16_t)(curr_amp_val & 0xFFFF);
		hw_amp_map.amp_map[(2*i) + 1] =  (uint16_t) (curr_amp_val >> 16);
	}
	cnf_content->cco_role = payload->min_conf.config.cco_role;
	cnf_content->configuration_binary_version = letohl(payload->min_conf.config.configuration_binary_version);
	memcpy(&cnf_content->mac_address, &payload->min_conf.config.mac_address.macAddress,
		sizeof(payload->min_conf.config.mac_address.macAddress));
	memcpy(&cnf_content->nmk, &payload->min_conf.config.nmk, sizeof(payload->min_conf.config.nmk));
	printf("The binary file was created by production library version: v%d.%d.%d\n", 
		letohs(payload->min_conf.config.version.version_major), letohs(payload->min_conf.config.version.version_minor),
		letohs(payload->min_conf.config.version.version_patch));
	memcpy(&cnf_content->amplitude_map, &payload->min_conf.amplitude_map,
		sizeof(payload->min_conf.amplitude_map));

	res = PROD_amplitude_map_parse(&hw_amp_map, &cnf_content->amplitude_map);
	if (res != RES_RESULT_OK)
	{
		return res;
	}

#ifndef CG5315

#define PAYLOAD_CONTAINS_FIELD(field_name) (msg_len >= sizeof(payload->field_name) + offsetof(PROD_configuration_content_packed_t, field_name))

	if (PAYLOAD_CONTAINS_FIELD(host_interface))
	{
		cnf_content->host_interface = payload->host_interface;

		if (PROD_is_invalid(payload->host_interface, HPGP_SPI, HPGP_SPI_DBG))
		{
			fprintf(stderr, "HLB_configuration_binary_content_parse host_interface bad param: %d,"
					" should be between %d and %d\n", cnf_content->host_interface, HPGP_SPI, HPGP_SPI_DBG);
			return RES_RESULT_BAD_PARAMETER;
		}
	}

	if (PAYLOAD_CONTAINS_FIELD(avln_hfid))
	{
		memcpy(&cnf_content->avln_hfid, &payload->avln_hfid, sizeof(payload->avln_hfid));
		cnf_content->avln_hfid.HFID[sizeof(payload->avln_hfid)] = '\0';
	}
	
	if (PAYLOAD_CONTAINS_FIELD(manu_hfid))
	{
		memcpy(&cnf_content->manu_hfid, &payload->manu_hfid, sizeof(payload->manu_hfid));
		cnf_content->manu_hfid.HFID[sizeof(payload->manu_hfid)] = '\0';
	}

	if (PAYLOAD_CONTAINS_FIELD(user_hfid))
	{
		memcpy(&cnf_content->user_hfid, &payload->user_hfid, sizeof(payload->user_hfid));
		cnf_content->user_hfid.HFID[sizeof(payload->user_hfid)] = '\0';
	}

	if (PAYLOAD_CONTAINS_FIELD(plc_freq_sel))
	{
		cnf_content->plc_freq_sel = payload->plc_freq_sel;
	}

	if (PAYLOAD_CONTAINS_FIELD(nid))
	{
		memcpy(&cnf_content->nid, &payload->nid, sizeof(payload->nid));
	}

	if (PAYLOAD_CONTAINS_FIELD(security_level))
	{
		cnf_content->security_level = payload->security_level;
	}

	if (PAYLOAD_CONTAINS_FIELD(receiver_max_sensitivity))
	{
		cnf_content->receiver_max_sensitivity = payload->receiver_max_sensitivity;
	}

	if (PAYLOAD_CONTAINS_FIELD(cg_drive_strength_rxd_0_1_2_3_and_rx_dv))
	{
		cnf_content->cg_drive_strength_rxd_0_1_2_3_and_rx_dv = payload->cg_drive_strength_rxd_0_1_2_3_and_rx_dv;
	}

	if (PAYLOAD_CONTAINS_FIELD(cg_drive_strength_tx_clk))
	{
		cnf_content->cg_drive_strength_tx_clk = payload->cg_drive_strength_tx_clk;
	}

	if (PAYLOAD_CONTAINS_FIELD(host_internal_data_PSD_gain))
	{
		memcpy((uint8_t*)&cnf_content->PSD_gain, (uint8_t*)&payload->host_internal_data_PSD_gain, sizeof(cnf_content->PSD_gain));
	}
	
#endif

	return RES_RESULT_OK;
}

RES_result_t PROD_channel_estimation_create(const PROD_ce2_info_t *ce2_content,
										double ce2_mag[PROD_CE2_SIZE])
{
	int i;
	double complex complex_ce2[PROD_CE2_SIZE];

	for(i = 0;i < PROD_CE2_SIZE;i++)
	{
		int_to_complex(ce2_content->ce2[i], &complex_ce2[i]);
	}
	
	return calc_update_ce2(complex_ce2, ce2_mag, ce2_content->gain, ce2_content->ce2_shift);
}

RES_result_t PROD_lnoe_create(const PROD_lnoe_info_t *lnoe_content,
										double lnoe_mag[LNOE_DATA_LEN])
{	
	return calc_update_lnoe(lnoe_content->lnoe, lnoe_mag, lnoe_content->gain);
}

RES_result_t PROD_snr_create(const PROD_snre_info_t *snr_content,
										double snr_db[PROD_SNRE_DATA_SIZE])
{
	return calc_update_snre(snr_content->snre, snr_db);
}

RES_result_t PROD_noise_floor_time_to_frequency(const PROD_time_domain_t *time_domain,
										double frequency_samples[FREQ_SAMPLES_BUF_SIZE])
{
	int i,j;
	int extended_buffer_size_PN = round_up_to_the_next_highest_power_of_2(time_domain->sample_size)
									* time_domain->iterations_num / 2;
	double PN_GAIN = - time_domain->gain - 59.2;
	complex_t fft_out_buffer[FFTSIZE];

	/* How many fft windows can start in a single buffer */
	int fft_windows_in_single_buffer = extended_buffer_size_PN/(FFTSIZE/2) - 1;
	
	for (j = 0;j < fft_windows_in_single_buffer;j++)
	{
		for (i = 0;i < FFTSIZE;i++)
		{
			fft_out_buffer[i].real = time_domain->samples[(FFTSIZE/2)*j+i] * hanning[i];
			fft_out_buffer[i].img = 0;
		}

		fft(fft_out_buffer,log2(FFTSIZE));
		for (i = 0;i < FFTSIZE;i++)
		{	
			double tmp;
			complex_sqr_abs(fft_out_buffer[i],&tmp);
			frequency_samples[i] += tmp/fft_windows_in_single_buffer;
		}	
	}

	normalise_freq_samples(frequency_samples, PN_GAIN);
	return RES_RESULT_OK;
}
