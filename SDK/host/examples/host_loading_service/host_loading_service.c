/********************************************************************
*
* Module Name: host_loading_service
* Design:
* This service monitor the modem and call fw_load api when needed
*
*
********************************************************************/
/*******************************************************************
* IMPORTS
********************************************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <syslog.h>
#include <gpiod.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include "version.h"
#include "comm_mgr_lib.h"
#include "osal.h"
#include "shared.h"
#include "HLB_fwload.h"
#include "HLB_nscm.h"
#include "PROD_lib.h"

/*******************************************************************
* CONSTANTS
********************************************************************/
/*******************************************************************
* TYPES
********************************************************************/
/*******************************************************************
* STATIC and GLOBAL DATA
********************************************************************/
volatile bool modem_notification = false;
volatile bool program_run_flag = true;
volatile bool got_user_signal = false;
volatile bool keepalive_run_flag = true;
volatile bool waiting_for_signal = false;
int listen_pipefd[2];

static char *firmware_binary_file = NULL;
static char *configuration_binary_file = NULL;
static char *devicename = NULL;

/*******************************************************************
* MACROS
********************************************************************/
#define DEFAULT_FW_BINARY_PATH "/lib/firmware/cg5317-sdk.bin"
#define DEFAULT_CONF_BINARY_PATH "/lib/firmware/cg5317-user-conf.bin"
#define MODEM_NOTIFICATION_LOCATION "/sys/devices//modem_status"
#define ETH_IFACE_NAME "eth0"
#define SPI_IFACE_NAME "seth0"
#define SUCCESS 0
#define FAILURE -1
#define MAX_FAILURE_QUERY 2
#define QUERY_TIMEOUT 500
#define READ_STAT_TIME 50
#define MAX_FAILURE_OPEN 2
#define DEFAULT_TIMEOUT_KEEPALIVE_MS 700
#define DEFAULT_TIMEOUT_ROM_UP_MICROSEC 1000000
#define MICROSEC_TO_MILLISEC_DIV 1000
#define GET_ADAPTER_RETRY_DELAY_US (250 * MICROSEC_TO_MILLISEC_DIV)
#define MAX_TRIES_RESET_ROM 10
#define MAX_TRIES_LOAD_FW 10
#define MAX_USER_CONF_FILE_SIZE 1024*100
#define NAME_OF_PROGRAM "host_loading_daemon"

#define RESET_MODEM_GPIO "gpiochip1"
#define RESET_MODEM_GPIO_OFFSET 30



static void *wait_for_modem_notification(void *param)
{
	FILE *fp;
	int counter = 0;
	char buffer[1024] = { 0 };
	(void)(param);
	char modem_location[1000] = "/sys/devices/";

	strcat(modem_location, devicename);
	strcat(modem_location, "/modem_status");

	while ((counter < MAX_FAILURE_OPEN) && program_run_flag)
	{
		usleep(READ_STAT_TIME * 1000);
		fp = fopen(modem_location, "r");
		if (!fp)
		{
			hlb_log_error("failed to open %s for reading", modem_location);
			counter++;
		}
		counter = 0;
		if (read(fp->_fileno, buffer, sizeof(buffer)) == -1) {
			fclose(fp);
			return NULL;
		}
		if (!strcmp(buffer, "1"))
		{
			hlb_log_info("got modem_notification");
			modem_notification = true;
			if (write(listen_pipefd[1], "2", 1) == -1) {
				fclose(fp);
				return NULL;
			}
			break;
		}
		fclose(fp);
	}
	return NULL;
}

static int start_wait_for_modem_notification(void **thread_handle)
{
	RES_result_t res;
	modem_notification = false;
	res = osal_thread_create(thread_handle, wait_for_modem_notification, NULL);
	if (res != SUCCESS)
	{
		hlb_log_error("osal_thread_create failed, res=%d", res);
	}
	return res;
}

static void stop_wait_for_modem_notification(void *thread_handle)
{
	osal_thread_join(&thread_handle);
}

static int busy_wait_for_flag(volatile bool *flag, uint64_t timeout, uint32_t resolution)
{
	uint64_t counter = 0;
	uint64_t start_time;
	struct timeval tv;

	gettimeofday(&tv, NULL);
	start_time = (tv.tv_sec * 1000000ll) + tv.tv_usec;
	while ((!(*flag)) && program_run_flag)
	{
		gettimeofday(&tv, NULL);
		counter = (((tv.tv_sec * 1000000ll) + tv.tv_usec) - start_time);
		if (counter >= timeout)
		{
			return FAILURE;
		}
		usleep(resolution);
	}

	return SUCCESS;
}

static RES_result_t reset_modem_using_gpio()
{
	int res;
	res = gpiod_ctxless_set_value(RESET_MODEM_GPIO, RESET_MODEM_GPIO_OFFSET, 0, false, NAME_OF_PROGRAM, NULL, NULL);
	if (res == -1)
	{
		hlb_log_error("could not set gpio "RESET_MODEM_GPIO" %d to 0", RESET_MODEM_GPIO_OFFSET);
		return FAILURE;
	}
	usleep(200000);
	res = gpiod_ctxless_set_value(RESET_MODEM_GPIO, RESET_MODEM_GPIO_OFFSET, 1, false, NAME_OF_PROGRAM, NULL, NULL);
	if (res == -1)
	{
		hlb_log_error("could not set gpio "RESET_MODEM_GPIO" %d to 1", RESET_MODEM_GPIO_OFFSET);
		return FAILURE;
	}
	usleep(200000);
	return RES_RESULT_OK;
}

static hpgp_host_interface_t get_iface_from_user_configuration(char *conf_bin_file)
{
	FILE *fp = fopen(conf_bin_file, "rb");
	PROD_configuration_content_packed_t packed_cnf_content = {0};
	size_t ret;

	if (!fp)
	{
		hlb_log_error("failed to open %s for reading", conf_bin_file);
		return FAILURE;
	}
	ret = fread(&packed_cnf_content, 1, sizeof(PROD_configuration_content_packed_t), fp);
	if (ret != sizeof(PROD_configuration_content_packed_t)) {
		fclose(fp);
		return FAILURE;
	}
	fclose(fp);
	return packed_cnf_content.host_interface;
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

/* signal handling */
static void host_loading_ser_reload_fw(int signum)
{
	(void)signum;
	got_user_signal = true;
	if (!waiting_for_signal)
	{
		reset_modem_using_gpio();
	}
}

static void host_loading_ser_term(int signum)
{
	(void)signum;
	program_run_flag = false;
	modem_notification = false;
	if (write(listen_pipefd[1], "3", 1) == -1)
		return;
}

static void host_loading_register_syslog()
{
	openlog("host_loading_service", LOG_PID | LOG_NDELAY, LOG_DAEMON);
	syslog(LOG_INFO, "Start logging");
}

static void host_loading_unregister_syslog()
{
	syslog(LOG_INFO, "Stop logging");
	closelog();
}

static void print_version(void)
{
	printf("current version=%d.%d.%d.%d\n", MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION, BUILD_VERSION);
}

static void usage(void)
{
	printf("host_loading_service [-f<firmware binary>] [-c<configuration binary>]"
		   "[-a<adapter mac address>] [-i<0=spi,1=eth>]"
		   "[-s<adapter name>\n"
		   "additional options:\n"
		   "-v = print version\n"
		   "-h = help (show this usage text)\n"
		   "-t = test mode, waits for SIGUSR1 to be sent before performing host loading\n"
		   "-r = CG5317 is configured to work in RMII mode\n"
		   "all options are optional\n");
}

static RES_result_t IO_access_read(const char *file_path, uint8_t *buffer, uint32_t offset, uint32_t count)
{
	int fd;

	fd = open(file_path, O_RDONLY);
	if (fd == -1)
		return RES_RESULT_MISSING_FILE;

	if ((off_t)offset != lseek(fd, offset, SEEK_SET))
	{
		close(fd);
		return RES_RESULT_ACCESS_DENIED;
	}

	if ((ssize_t)count != read(fd, buffer, count))
	{
		close(fd);
		return RES_RESULT_ACCESS_DENIED;
	}

	close(fd);

	return RES_RESULT_OK;
}

static RES_result_t fw_bin_IO_access_read(void *priv, uint8_t *buffer, uint32_t offset, uint32_t count)
{
	(void)priv;

	return IO_access_read(firmware_binary_file, buffer, offset, count);
}

static RES_result_t user_config_IO_access_read(void *priv, uint8_t *buffer, uint32_t offset, uint32_t count)
{
	(void)priv;

	return IO_access_read(configuration_binary_file, buffer, offset, count);
}

int main(int argc, char *argv[])
{
	int res, c;
	char *adapter_mac_addr = NULL;
	bool user_defined_iface = false;
	bool test_mode = false;
	bool is_cg_rmii = false;
	char buffer[1024];
	mac_address_t adapter_addr;
	hpgp_host_interface_t iface;
	void *notification_thread;
	struct sigaction usr_action;
	struct sigaction term_action;
	int failed_counter = 0;

	configuration_binary_file = DEFAULT_CONF_BINARY_PATH;
	iface = get_iface_from_user_configuration(configuration_binary_file);
	devicename = SPI_IFACE_NAME;

	while ((c = getopt(argc, argv, "rtvhc:f:a:i:s:")) != -1)
	{
		switch (c)
		{
		case 'c':
			configuration_binary_file = optarg;
			break;
		case 'h':
			usage();
			return RES_RESULT_OK;
		case 'v':
			print_version();
			return RES_RESULT_OK;
		case 'i':
			iface = atoi(optarg);
			user_defined_iface = true;
			break;
		case 'f':
			firmware_binary_file = optarg;
			break;
		case 'a':
			adapter_mac_addr = optarg;
			break;
		case 't':
			test_mode = true;
			break;
		case 'r':
			is_cg_rmii = true;
			break;
		case 's':
			devicename = optarg;
			break;
		default:
			usage();
			return RES_RESULT_GENERAL_ERROR;
		}
	}

	host_loading_register_syslog();

	if (!firmware_binary_file)
	{
		firmware_binary_file = DEFAULT_FW_BINARY_PATH;
	}

	if (!configuration_binary_file)
	{
		configuration_binary_file = DEFAULT_CONF_BINARY_PATH;
	}

	if (pipe(listen_pipefd) == -1) {
		hlb_log_error("listen pipe error");
		host_loading_unregister_syslog();
		return FAILURE;
	}

	if (daemon(0, 0) == -1)
		return FAILURE;

	res = start_wait_for_modem_notification(&notification_thread);
	if (res != SUCCESS)
	{
		hlb_log_error("failed to start notification thread (%d)", res);
		host_loading_unregister_syslog();
		return FAILURE;
	}

	memset(&usr_action, 0, sizeof(usr_action));
	usr_action.sa_handler = host_loading_ser_reload_fw;
	sigaction(SIGUSR1, &usr_action, NULL);

	memset(&term_action, 0, sizeof(term_action));
	term_action.sa_handler = host_loading_ser_term;
	sigaction(SIGTERM, &term_action, NULL);

	while (program_run_flag)
	{
		if (!modem_notification)
		{
			fd_set rdfds;
			FD_ZERO(&rdfds);
			FD_SET(listen_pipefd[0], &rdfds);
			res = select(listen_pipefd[0] + 1, &rdfds, NULL, NULL, NULL);
			if (res < 0)
			{
				hlb_log_error("error on select (%d) at host loading service", res);
				continue;
			}
			if (read(listen_pipefd[0], buffer, sizeof(buffer)) == -1)
				return FAILURE;
				
		}
		else /* modem_notification = true */
		{
			if (test_mode)
			{
				/* ** DEBUG feature **
				 * When this service is executed with -t flag, it will wait for SIGUSR1 signal
				 * before performing the fw loading when modem notification is received.
				 * This allows QA to control when exactly the fw load happens after Modem reset.
				 */
				hlb_log_error("Test mode - waiting for signal");
				waiting_for_signal = true;
				busy_wait_for_flag(&got_user_signal, UINT64_MAX, 100000);
				waiting_for_signal = false;
			}

			got_user_signal = false;

			if (adapter_mac_addr == NULL)
			{
				const char *iface_name;
				int attempts_left = 8;

				if (!user_defined_iface)
				{
					iface = get_iface_from_user_configuration(configuration_binary_file);
				}

				iface_name = (iface == HPGP_ETH) ? ETH_IFACE_NAME : devicename;
				while (attempts_left > 0)
				{
					res = ETH_get_mac_address_by_interface_name(iface_name, adapter_addr);
					if (res != RES_RESULT_NOT_YET)
					{
						/* success or error */
						break;
					}
					else
					{
						attempts_left--;
						if (attempts_left > 0)
						{
							/* The interface may not be ready yet after CG reset because it was not
							 * running before the reset.
							 * For example, when the CG5317 is running in ETH only host interface mode
							 * the SPI interface will not be active.
							 * If the reset happened inorder to switch from ETH host interface to SPI
							 * host interface, it may take some time for the SPI interface to be fully
							 * running again after detecting SPI HW activity from Bootrom.
							 */
							usleep(GET_ADAPTER_RETRY_DELAY_US);
						}
					}
				}

				if (res != RES_RESULT_OK)
				{
					hlb_log_error("failed to get mac address for interface %s", iface_name);
					host_loading_unregister_syslog();
					return FAILURE;
				}
			}
			else
			{
				str_to_mac(adapter_mac_addr, &adapter_addr);
			}

			//load fw
			res = HLB_load_fw(adapter_addr,
							  fw_bin_IO_access_read,
							  user_config_IO_access_read,
							  NULL,
							  is_cg_rmii);
			if ((test_mode) && (res != SUCCESS))
			{
				hlb_log_error("failed to load firmware (%d)", res);
				hlb_log_error("Test mode - not reseting");
			}
			else if (res != SUCCESS)
			{
				hlb_log_error("failed to load firmware (%d)", res);
				hlb_log_error("firmware file %s", firmware_binary_file);
				hlb_log_error("configuration file %s", configuration_binary_file);
				failed_counter++;
				if ((res == RES_RESULT_TIMEOUT) || (failed_counter < MAX_TRIES_LOAD_FW))
				{
					hlb_log_error("load firmware failed, reset and try again");
					reset_modem_using_gpio();
				}
				else
				{
					hlb_log_error("load firmware failed due to unknown reason"
									", maybe file corrupted");
					hlb_log_error("waiting for signal to notify us fw files has changed");
					busy_wait_for_flag(&got_user_signal, UINT64_MAX, 100000);
				}
			}
			else
			{
				failed_counter = 0;
			}

			stop_wait_for_modem_notification(notification_thread);
			start_wait_for_modem_notification(&notification_thread);
		}
	}
	stop_wait_for_modem_notification(notification_thread);
	close(listen_pipefd[0]);
	close(listen_pipefd[1]);

	hlb_log_error("exiting host loading service");
	sleep(1);
	host_loading_unregister_syslog();

	return res;
}
