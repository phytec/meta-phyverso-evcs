/********************************************************************
*
* Module Name: ind_listener_app
* Design:
* This application is written above HLB_ind_listener APIs, and shows 
* examples of using them.
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
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>

#include "HLB_ind_listener.h"
#include "version.h"
#include "HLB_host.h"
#include "osal.h"

/*******************************************************************
* CONSTANTS
********************************************************************/
/*******************************************************************
* TYPES
********************************************************************/
typedef struct
{
	bool host_message_status;
	bool d_link_ready;
} wanted_indications_t;

/*******************************************************************
* STATIC and GLOBAL DATA
********************************************************************/
int stop_loop = 0;
/*******************************************************************
* MACROS
********************************************************************/
/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/
/* signal handling */
static void hlb_term(int signum)
{
	(void)signum;
	stop_loop = 1;
}

static void init_wanted_indications(char *user_requests, wanted_indications_t *wanted_indications)
{
	char *ret;

	wanted_indications->d_link_ready = false;
	wanted_indications->host_message_status = false;

    ret = strstr(user_requests, "all");
    if (ret)
	{
		wanted_indications->host_message_status = true;
		wanted_indications->d_link_ready = true;
		return;
	}

	ret = strstr(user_requests, "host_message");
	if (ret)
	{
		wanted_indications->host_message_status = true;
	}

	ret = strstr(user_requests, "d_link");
	if (ret)
	{
		wanted_indications->d_link_ready = true;
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

static void print_curr_time()
{
	time_t t = time(NULL);
	struct tm received_time = *localtime(&t);
	printf("|%d-%02d-%02d %02d:%02d:%02d| ",
			received_time.tm_year + 1900,
			received_time.tm_mon + 1, received_time.tm_mday,
			received_time.tm_hour, received_time.tm_min,
			received_time.tm_sec);
}

static void d_link_ready_callback(HLB_hpgp_d_link_ready_status_ind_t *d_link)
{
	print_curr_time();
	if(d_link->d_link_ready_status == 0)
	{
		printf("D link ready status: No link(0)\n");
	}
	else
	{
		printf("D link ready status: Link Established(1)\n");
	}
	
}

static void host_message_status_callback(HLB_hpgp_host_message_status_ind_t *host_message)
{
	print_curr_time();
	if(host_message->host_message_status == 1)
	{
		printf("Host message status: Ready to join AVLN(1)\n");
	}
	else if(host_message->host_message_status == 2)
	{
		printf("Host message status: Joined AVLN(2)\n");
	}
	else
	{
		printf("Host message status: Disconnected from AVLN(3)\n");
	}
}

static void print_version(void)
{
	printf("current version=%d.%d.%d.%d\n", MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION, BUILD_VERSION);
}

static void available_indications(void)
{
	printf("Available wanted indications:\n\n"
			"all - listen to all indications\n"
			"host_message - listen to host message status indications\n"
			"d_link - listen to D Link ready status indications\n");
}

static void usage(void)
{
	printf("Usage:\n"
		   "ind_listener_app [-a<adapter mac address>] [-l<wanted_indications>]\n\n"
		   "Additional options:\n"
		   "-v = print version\n"
		   "-h = help (show this usage text)\n"
		   "-t[<seconds_to_listen>] = seconds to listen\n"
		   "-w = print available wanted indications\n"
		   "-d[<dest_mac_addr>] = receive only broadcast messages sent from dest_mac_addr\n"
		   "\nExample usage of listening to all indication messages:\n"
		   "./ind_listener_app -a MAC_ADDR -l \"all\"\n");
}

int main(int argc, char *argv[])
{
	int res, c;
	int time_to_listen = 0;

	char *adapter_mac_addr = NULL;
	mac_address_t adapter_addr;
	char *dest_mac_addr = NULL;
	mac_address_t dest_addr;
	char *user_wanted_indications = NULL;
    HLB_ind_listener_handler_struct_t ind_listener_handler;
	wanted_indications_t wanted_indications;
	struct sigaction action;

	while ((c = getopt(argc, argv, "vhwa:t:l:")) != -1)
	{
		switch (c)
		{
		case 'h':
			usage();
			return RES_RESULT_OK;
		case 'v':
			print_version();
			return RES_RESULT_OK;
		case 'w':
			available_indications();
			return RES_RESULT_OK;	
		case 'a':
			adapter_mac_addr = optarg;
			break;
		case 'l':
			user_wanted_indications = optarg;
			break;	
		case 't':
			time_to_listen = atoi(optarg);
			break;
		case 'd':
			dest_mac_addr = optarg;
			break;	
		default:
			usage();
			return RES_RESULT_GENERAL_ERROR;
		}
	}

	if (!adapter_mac_addr)
	{
		printf("adapter_mac_addr was not provided\n");
		usage();
		return RES_RESULT_GENERAL_ERROR;
	}

	if (!user_wanted_indications)
	{
		printf("user_wanted_indications was not provided\n");
		usage();
		return RES_RESULT_GENERAL_ERROR;
	}

	init_wanted_indications(user_wanted_indications, &wanted_indications);
	str_to_mac(adapter_mac_addr, &adapter_addr);

	if(dest_mac_addr)
	{
		str_to_mac(dest_mac_addr, &dest_addr);
		res = HLB_init_ind_listener(adapter_addr, dest_addr, &ind_listener_handler);
	}
	else
	{
		res = HLB_init_ind_listener(adapter_addr, NULL, &ind_listener_handler);
	}

    if(res != RES_RESULT_OK)
    {
        printf("%s: Failed to init ind_listener\n", __func__);
        return res;
    }

	/* Register to indications according to the wanted messages from the user */
	if(wanted_indications.d_link_ready)
	{
		res = HLB_register_d_link_ready(&ind_listener_handler, d_link_ready_callback);
    	if(res != RES_RESULT_OK)
    	{
        	printf("%s: Failed to register d_link_ready\n", __func__);
        	return res;
    	}
	}

    if(wanted_indications.host_message_status)
	{
		res = HLB_register_host_message_status(&ind_listener_handler, host_message_status_callback);
    	if(res != RES_RESULT_OK)
    	{
        	printf("%s: Failed to register host_message_status\n", __func__);
        	return res;
    	}
	}

	/* Wait while HLB_ind_listener runs */
	if(time_to_listen != 0)
	{
		osal_sleep(time_to_listen);
	}
	else
	{
		memset(&action, 0, sizeof(action));
		action.sa_handler = hlb_term;
		sigaction(SIGINT, &action, NULL);
		while (!stop_loop)
		{
			usleep(10);
		}
	}

	/* Unregister from indications according to the wanted messages from the user */
	if(wanted_indications.d_link_ready)
	{
		res = HLB_unregister_d_link_ready(&ind_listener_handler);
    	if(res != RES_RESULT_OK)
    	{
        	printf("%s: Failed to unregister d_link_ready\n", __func__);
        	return res;
    	}
	}

    if(wanted_indications.host_message_status)
	{
		res = HLB_unregister_host_message_status(&ind_listener_handler);
    	if(res != RES_RESULT_OK)
    	{
        	printf("%s: Failed to unregister host_message_status\n", __func__);
        	return res;
    	}
	}

    HLB_terminate_ind_listener(&ind_listener_handler);
	return res;
}
