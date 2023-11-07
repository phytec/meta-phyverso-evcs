/**
 * @file PROD_lib.h
 * @author Daniel Varennikov
 * @date 01 August 2021
 * @brief This module APIs of the production library
 *
 *
 *
 */
/********************************************************************
*
* Module Name: PROD_lib.h
* Design:
* Implement Configuration binary management and post processing of hw vectors
*
********************************************************************/
#ifndef PROD_CONFIGURATION_H
#define PROD_CONFIGURATION_H
/********************************************************************
* IMPORTS
********************************************************************/
#include "common.h"
#include "shared.h"
#include <stddef.h>
/********************************************************************
* EXPORTED CONSTANTS
********************************************************************/
/*! The normalisation value used from the conversion
 * of the user configuration amplitude map doubles to
 * the values in the configuration binary*/
#define TXPREEQ_ZERO 16384.0

/*! The special OFF amplitude map value in
 * the user struct that indicates that 0
 * should be written in the binary*/
#define OFF_SPECIAL_CASE 100
#define PROD_CE2_SIZE 2048
#define USER_AMDATA_LEN 1155			 /*!< User configuration amplitude map Length*/
#define PROD_SNRE_DATA_SIZE 512
#define PROD_AMP_MAP_DATA_SIZE 2048
#define FsMhz 200
#define FFTSIZE 2048
#define FREQ_SAMPLES_BUF_SIZE FFTSIZE * sizeof(double)
/********************************************************************
* EXPORTED TYPES
********************************************************************/

/*! Default configuration binary version*/
#define CONF_BIN_VERSION_INIT 1

/*! Default mac address*/
#define MAC_ADDRESS_INIT \
	{                    \
			0x00,        \
			0x16,        \
			0xe8,        \
			0x00,        \
			0x00,        \
			0x00         \
	}

/*! Default nmk*/
#define NMK_INIT                                                                                       \
	{                                                                                                  \
		0xb5, 0x93, 0x19, 0xd7, 0xe8, 0x15, 0x7b, 0xa0, 0x01, 0xb0, 0x18, 0x66, 0x9c, 0xce, 0xe3, 0x0d \
	}

/*! Default nid (invalid NID indicating to use default) */
#define NID_INIT                                 \
	{                                            \
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff \
	}

/*! Default cco role*/
#define CCO_ROLE_INIT HPGP_CCO_MODE_ALWAYS

/*! Default amplitude map*/
#define AMP_MAP_INIT                    \
	{                                   \
		{                               \
			[0 ... 1154] = 0.0			\
		}                               \
	}


#ifndef CG5315

/*! Warning: HOST_INTERFACE_INIT value not final*/
#define HOST_INTERFACE_INIT 1

/*! Warning: MANU_HFID_INIT value not final*/
#define MANU_HFID_INIT "Lumissil"

/*! Warning: USER_HFID_INIT value not final*/
#define USER_HFID_INIT "userHFID"

/*! Warning: AVLN_HFID_INIT value not final*/
#define AVLN_HFID_INIT "AVLNHFID"

/*! Default PLC frequency selection */
#define PLC_FREQ_SELECTION_INIT 0

/*! Default security level */
#define SECURITY_LEVEL_INIT 0

/*! Default Maximal Receiver Sensitivity */
#define MAX_RECEIVER_SENSITIVITY_INIT 43

/*! Default PSD gain */
#define PSD_GAIN_INIT 0

#endif

#ifdef CG5315

/*! Default configuration struct*/
#define PRD_CONFIG_INIT                                        \
	{                                                          \
		.configuration_binary_version = CONF_BIN_VERSION_INIT, \
		.mac_address = MAC_ADDRESS_INIT,                       \
		.nmk = NMK_INIT,                                       \
		.cco_role = CCO_ROLE_INIT,                             \
		.amplitude_map = AMP_MAP_INIT,                         \
	}

#else

/*! Default PRD_CONFIG_INIT value*/
#define PRD_CONFIG_INIT                                        \
	{                                                          \
		.configuration_binary_version = CONF_BIN_VERSION_INIT, \
		.mac_address = MAC_ADDRESS_INIT,                       \
		.nmk = NMK_INIT,                                       \
		.cco_role = CCO_ROLE_INIT,                             \
		.amplitude_map = AMP_MAP_INIT,                         \
		.host_interface = HOST_INTERFACE_INIT,				   \
		.manu_hfid = MANU_HFID_INIT,						   \
	    .user_hfid = USER_HFID_INIT,					       \
		.avln_hfid = AVLN_HFID_INIT, 						   \
		.plc_freq_sel = PLC_FREQ_SELECTION_INIT,			   \
		.nid = NID_INIT,									   \
		.security_level = SECURITY_LEVEL_INIT,				   \
		.receiver_max_sensitivity = MAX_RECEIVER_SENSITIVITY_INIT, \
		.cg_drive_strength_rxd_0_1_2_3_and_rx_dv = CG_PIN_DRIVE_STRENGTH_8_mA, \
		.cg_drive_strength_tx_clk = CG_PIN_DRIVE_STRENGTH_8_mA,\
		.PSD_gain = PSD_GAIN_INIT,							   \
	}

#endif



/*! Structure to hold the NMK (Network Membership Key) */
typedef struct
{
	uint8_t NMK[HPAVKEY_NMK_LEN]; /*!< Network Membership Key */
} hpgp_nmk_t;

#ifndef CG5315
typedef enum
{
	HPGP_SPI = 0,		/*!< Host interface is SPI, ETH interface is disabled*/
	HPGP_ETH = 1,		/*!< Host interface is ETH (R/MII), SPI interface is disabled */
	HPGP_SPI_DBG = 2,	/*!< Host interface is SPI, but ETH interface is still enabled */
} hpgp_host_interface_t;

/*! PLC Frequency Selection */
typedef enum
{
	HPGP_PLC_FREQ_SELECTION_50_HZ = 0,				/*!< PLC Frequency Selection - 50Hz */
	HPGP_PLC_FREQ_SELECTION_60_HZ = 1,				/*!< PLC Frequency Selection - 60Hz */
	HPGP_PLC_FREQ_SELECTION_EXTERNAL_SIGNAL = 2,	/*!< PLC Frequency Selection - External Signal */
} hpgp_plc_freq_sel_t;

/*! Structure to hold the NID (Network Identifier) */
typedef struct
{
	uint8_t NID[HPAVKEY_NID_LEN]; /*!< Network Identifier */
} hpgp_nid_t;

typedef enum
{
	HPGP_SECURITY_LEVEL_SC = 0,	/*!< Simple connect security level */
	HPGP_SECURITY_LEVEL_HS = 1,	/*!< Secure Security level */
} hpgp_security_level_t;
#endif

/*! cco_mode */
typedef enum
{
	HPGP_CCO_MODE_AUTO = 0,		  /*!< Auto CCo. In this case, the station may act either as a CCo or STA of an AVLN */
	HPGP_CCO_MODE_NEVER = 1,  	  /*!< Never a CCo. In this case, the station will never become the CCo of an AVLN */
	HPGP_CCO_MODE_ALWAYS = 2,	  /*!< Always a CCo. In this case, the station will not join other AVLNs as a STA */

} hpgp_cco_mode_t;

/*! Structure to hold amp_map */
typedef struct
{
	double amdata[USER_AMDATA_LEN]; /*!< Amplitude data, 0.0 - 68.0 [dB] or OFF*/
} amplitude_map_t;

#ifndef CG5315
/*! Structure to hold the HFID (Human Friendly Identifier) */
typedef struct
{
	uint8_t HFID[HPAVKEY_HFID_LEN + 1]; /*!< Human Friendly Identifier, See HPGP Clause 7.3.1.2 */
} hpgp_hfid_t;
#endif

/*! CG5317 pin drive strength configuration */
typedef enum
{
	CG_PIN_DRIVE_STRENGTH_8_mA = 0,				/*!< Pin drive strength of 8mA */
	CG_PIN_DRIVE_STRENGTH_4_mA = 1,				/*!< Pin drive strength of 4mA */
	CG_PIN_DRIVE_STRENGTH_8_mA_W_SLEW_RATE = 2,	/*!< Pin drive strength of 8mA with Slew rate */
} cg_pin_drive_strength_t;

typedef struct
{
	uint32_t configuration_binary_version;    /*!< The configuration binary version */
	mac_address_t mac_address;				  /*!< The devices MAC address */
	hpgp_nmk_t nmk;							  /*!< Network Membership Key */
	hpgp_cco_mode_t cco_role;				  /*!< Role of the device */
	amplitude_map_t amplitude_map;			  /*!< Transmission power per tone */

#ifndef CG5315
	hpgp_host_interface_t host_interface;	  /*!< Which Host interface is enabled */
	hpgp_hfid_t manu_hfid; 					  /*!<manufacturer HFID*/
	hpgp_hfid_t user_hfid; 					  /*!<user HFID*/
	hpgp_hfid_t avln_hfid; 					  /*!<AVLN HFID*/
	hpgp_plc_freq_sel_t plc_freq_sel;		  /*!< PLC Frequency Selection */
	hpgp_nid_t nid;							  /*!< Network Identifier (including Security Level) to associate with this NMK, or indicate to use the default NID (all 0xFF bytes) */
	hpgp_security_level_t security_level;	  /*!< Security Level (values = HS or SC) - Only relevant if default NID is used */
	/*! Maximal Receiver Sensitivity configuration.
	 * <br>The receiver sensitivity can be controlled by setting a limitation on the maximum gain of the AGC.
	 * <br>Values range between a minimum of 3dB and a maximum of 43dB. */
	uint8_t receiver_max_sensitivity;
	/*! The drive-strength of HOST1 (RXD_0), HOST2 (RXD_1), HOST3 (RXD_2), HOST4 (RXD_3), HOST5 (RX_DV) pins. */
	cg_pin_drive_strength_t cg_drive_strength_rxd_0_1_2_3_and_rx_dv;
	/*! The drive-strength of HOST6 (TX_CLK) pin. */
	cg_pin_drive_strength_t cg_drive_strength_tx_clk;
	/*! The PSD gain requested by the user. */
	float PSD_gain;
#endif

} PROD_configuration_content_t;

typedef struct
{
	int8_t gain; 						/*!< current gain */
	int16_t ce2_shift; 				/*!< ce2 shift */
	int32_t ce2[PROD_CE2_SIZE];  /*!< data buffer */
} PROD_ce2_info_t;

typedef struct
{
	int8_t gain; 		/*!< current gain */
	int8_t lnoe[LNOE_DATA_LEN];  /*!< data buffer */
} PROD_lnoe_info_t;

typedef struct
{
	int16_t snre[PROD_SNRE_DATA_SIZE]; /*!< data buffer */
} PROD_snre_info_t;

typedef struct
{
	int16_t amp_map[PROD_AMP_MAP_DATA_SIZE]; /*!< FDP HW format amplitude map */
} PROD_hw_amp_map_t;

typedef struct
{
	int gain;						/*!< current gain */
	uint8_t iterations_num;			/*!< number of iterations */
	uint32_t sample_size;			/*!< Size of sample in an iteration */
	int16_t *samples; 				/*!< samples buffer */
} PROD_time_domain_t;
/********************************************************************
* EXPORTED MACROS
********************************************************************/
/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/
/**
 *   Function Name: PROD_configuration_binary_content_create
 *
 * @brief Description: this function creates the binary content
 * from the provided configuration_content native representation struct
 *
 *
 *  @param  input : configuration_content - The configuration_content native struct
 *  @param  output : msg - The configuration_content in binary format
 *  @param  input, output : msg_len - The configuration_content msg len
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t PROD_configuration_binary_content_create(const PROD_configuration_content_t *cnf_content,
													  void *msg, size_t *msg_len);

/**
 *   Function Name: PROD_configuration_binary_content_parse
 *
 * @brief Description: this function parses the binary configuration content
 * and returns it as native representation struct
 *
 *
 *  @param  input : msg - The configuration_content in binary format
 *  @param  input : msg_len - The msg len
 *  @param  output : configuration_content - The configuration_content native struct
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t PROD_configuration_binary_content_parse(const void *msg, size_t msg_len,
													 PROD_configuration_content_t *cnf_content);

/**
 *   Function Name: PROD_channel_estimation_create
 *
 * @brief Description: this function creates the ce2_mag as a list of
 * floating point numbers from the provided PROD_ce2_info_t native
 * representation struct
 *
 *
 *  @param  input : ce2_content - The PROD_ce2_info_t native struct
 *  @param  output : ce2_mag - The manipulated ce2_content
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t PROD_channel_estimation_create(const PROD_ce2_info_t *ce2_content,
										double ce2_mag[PROD_CE2_SIZE]);

/**
 *   Function Name: PROD_lnoe_create
 *
 * @brief Description: this function creates the lnoe_mag as a list of
 * floating point numbers from the provided PROD_lnoe_info_t native
 * representation struct
 *
 *
 *  @param  input : lnoe_content - The PROD_lnoe_info_t native struct
 *  @param  output : lnoe_mag - The manipulated lnoe_content
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t PROD_lnoe_create(const PROD_lnoe_info_t *lnoe_content,
										double lnoe_mag[LNOE_DATA_LEN]);

/**
 *   Function Name: PROD_snr_create
 *
 * @brief Description: this function creates the snr_db as a list of
 * floating point numbers from the provided PROD_snre_info_t native
 * representation struct
 *
 *
 *  @param  input : snr_content - The PROD_snre_info_t native struct
 *  @param  output : snr_db - The manipulated snr_content
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t PROD_snr_create(const PROD_snre_info_t *snr_content,
										double snr_db[PROD_SNRE_DATA_SIZE]);

/**
 *   Function Name: PROD_amplitude_map_parse
 *
 * @brief Description: this function creates the amp_map as a list of
 * floating point numbers (0.0 - 68.0) from the provided PROD_hw_amp_map_t native
 * hw amplitude map representation struct
 *
 *
 *  @param  input : hw_amp_map - The PROD_hw_amp_map_t native hw amplitude map representation
 *  @param  output : amp_map - The parsed amplitude map
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t PROD_amplitude_map_parse(const PROD_hw_amp_map_t *hw_amp_map,
									  amplitude_map_t *amp_map);

/**
 *   Function Name: PROD_noise_floor_time_to_frequency
 *
 * @brief Description: this function converts time domain Noise floor
 * samples into frequency domain samples
 *
 *
 *  @param  input : time_domain - The time domain samples
 *  @param  output : frequency_samples - The frequency samples
 *  @return RES_result_t : result status code
 *
 *
 */
RES_result_t PROD_noise_floor_time_to_frequency(const PROD_time_domain_t *time_domain,
										double frequency_samples[FREQ_SAMPLES_BUF_SIZE]);																																	 
/********************************************************************
* INTERNAL FUNCTIONS DECLARATIONS (FOR UNIT TESTING)
********************************************************************/

#endif // PROD_CONFIGURATION_H
