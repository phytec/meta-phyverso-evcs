/********************************************************************
*
* Module Name: osal
* Design:
* Implement osal api
*
********************************************************************/
/*******************************************************************
* IMPORTS
********************************************************************/
#include "osal.h"
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <syslog.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
/*******************************************************************
* CONSTANTS
********************************************************************/
/*******************************************************************
* TYPES
********************************************************************/
/*
 *	Network Adapter information entry
 */
typedef struct tagAdapterInfo
{
	mac_address_t mac;
	int idx;
} AdapterInfo;

/*
 * ETH network struct
 */
typedef struct
{
	int devfd;
	struct sockaddr_ll sock;
	int pipefd[2];
} eth_desc_t;

/*
 *  Connection data structure
 */
typedef struct
{
	mac_address_t device; /* device MAC address */
	mac_address_t host;	  /* host nic MAC address */
	eth_desc_t adapter;	  /* adapter connection handle */
} connection_t;

typedef struct tagETH_header_t
{
	mac_address_t da;
	mac_address_t sa;
	unsigned short etype;
} ETH_header_t;

typedef struct
{
	pthread_cond_t indication_db_cond;
    pthread_mutex_t indication_db_lock;
} osal_cond_lock_t;
/*******************************************************************
* STATIC and GLOBAL DATA
********************************************************************/
volatile bool hlb_log_on = true;
/*******************************************************************
* MACROS
********************************************************************/

#define MSEC_IN_SEC 1000
#define NSEC_IN_MSEC 1000000
#define NSEC_IN_SEC (MSEC_IN_SEC * NSEC_IN_MSEC)

#define USEC_IN_SEC 1000000
#define USEC_IN_MSEC 1000

#define MAX_ETH_SIZE 1514
#define MIN_ETH_SIZE 64

#define DEF_ARR_SIZE 64
/*******************************************************************
* INTERNAL FUNCTIONS
********************************************************************/
static void hlb_log_format(int level, const char *tag, const char *message, va_list args)
{
	if (hlb_log_on)
	{
		time_t now;
		time(&now);
		char *date = ctime(&now);
		date[strlen(date) - 1] = '\0';
		syslog(level, "%s [%s] ", date, tag);
		vsyslog(level, message, args);
		syslog(level, "\n");
	}
}

/* ETH communication functions */
static RES_result_t ETH_release_adapter(eth_desc_t *xi_adapter)
{
	if (xi_adapter == NULL)
	{
		hlb_log_error("xi_adapter provided in ETH_release_adapter is NULL!");
		return RES_RESULT_NULL_PTR;
	}
	close(xi_adapter->devfd);
	return RES_RESULT_OK;
}

static RES_result_t NIC_get_adapters(AdapterInfo *xio_adapters, int *xo_size)
{
	struct ifreq pifreq[DEF_ARR_SIZE];
	struct ifreq ireq;
	struct ifconf conf;
	RES_result_t status = RES_RESULT_OK;
	int devfd;
	int i;
	int devices = 0;

	if (xio_adapters == NULL)
	{
		hlb_log_error("xio_adapters provided in NIC_get_adapters is NULL!");
		return RES_RESULT_NULL_PTR;
	}
	if (xo_size == NULL)
	{
		hlb_log_error("xo_size provided in NIC_get_adapters is NULL!");
		return RES_RESULT_NULL_PTR;
	}

	devfd = socket(PF_INET, SOCK_STREAM, 0);
	if (devfd < 0)
	{
		hlb_log_error("(!) failed to open socket %d\n", errno);
		return RES_RESULT_GENERAL_ERROR;
	}

	conf.ifc_req = pifreq;
	conf.ifc_len = sizeof(pifreq);

	/* get interfaces list */
	if (ioctl(devfd, SIOCGIFCONF, &conf) < 0)
	{
		hlb_log_error("(!) failed to query interfaces (stat=%d)\n", errno);
		if (devfd)
			close(devfd);
		*xo_size = devices;
		return RES_RESULT_GENERAL_ERROR;
	}

	/* fill adapters array */
	for (i = 0; i < (int)(conf.ifc_len / sizeof(struct ifreq)); i++)
	{
		strcpy(ireq.ifr_name, conf.ifc_req[i].ifr_name);
		/* get interface MAC address */
		if (ioctl(devfd, SIOCGIFHWADDR, &ireq) < 0)
		{
			hlb_log_error("(!)[%d] failed to query MAC for interface %s (stat=%d)\n", i, conf.ifc_req[i].ifr_name, errno);
			if (devfd)
				close(devfd);
			*xo_size = devices;
			return RES_RESULT_GENERAL_ERROR;
		}
		else
		{
			memcpy(xio_adapters[i].mac, ireq.ifr_hwaddr.sa_data, sizeof(mac_address_t));
			devices++;
		}

		/* query interface index */
		if (ioctl(devfd, SIOCGIFINDEX, &ireq) < 0)
		{
			hlb_log_error("(!)[%d] failed to query interface %s index (stat=%d)\n", i, conf.ifc_req[i].ifr_name, errno);
			if (devfd)
				close(devfd);
			*xo_size = devices;
			return RES_RESULT_GENERAL_ERROR;
		}
		else
		{
			xio_adapters[i].idx = ireq.ifr_ifindex;
		}
	}

	if (devfd)
		close(devfd);
	*xo_size = devices;
	return status;
}

static RES_result_t NIC_get_adapter(const uint8_t *xi_adapter_mac_address, AdapterInfo *xo_adapter)
{
	AdapterInfo ifinfo[DEF_ARR_SIZE];
	RES_result_t status;
	int size;
	int i;
	if (xi_adapter_mac_address == NULL)
	{
		hlb_log_error("xi_adapter_mac_address provided in NIC_get_adapter is NULL!");
		return RES_RESULT_NULL_PTR;
	}
	if (xo_adapter == NULL)
	{
		hlb_log_error("xo_adapter provided in NIC_get_adapter is NULL!");
		return RES_RESULT_NULL_PTR;
	}

	status = NIC_get_adapters(ifinfo, &size);
	if (status != RES_RESULT_OK)
	{
		return status;
	}
	else
	{
		status = RES_RESULT_GENERAL_ERROR;
		for (i = 0; i < size; i++)
		{
			if (memcmp(&ifinfo[i].mac[0], xi_adapter_mac_address, sizeof(mac_address_t)) == 0)
			{
				memcpy(xo_adapter, &ifinfo[i], sizeof(AdapterInfo));
				return RES_RESULT_OK;
			}
		}
	}
	return status;
}

static RES_result_t ETH_bind_adapter(const uint8_t *xi_adapter_mac_address, uint16_t xi_ether_type, eth_desc_t *xo_adapter)
{
	eth_desc_t *desc;
	AdapterInfo adapter;
	RES_result_t status;
	desc = xo_adapter;

	if (xi_adapter_mac_address == NULL)
	{
		hlb_log_error("xi_adapter_mac_address provided in ETH_bind_adapter is NULL!");
		return RES_RESULT_NULL_PTR;
	}
	if (xo_adapter == NULL)
	{
		hlb_log_error("xo_adapter provided in ETH_bind_adapter is NULL!");
		return RES_RESULT_NULL_PTR;
	}

	memset(desc, 0, sizeof(eth_desc_t));

	/* open raw packet socket */
	desc->devfd = socket(PF_PACKET, SOCK_RAW, htons(xi_ether_type));
	if (desc->devfd < 0)
	{
		hlb_log_error("(!) failed to open raw socket %d\n", desc->devfd);
		return RES_RESULT_GENERAL_ERROR;
	}

	desc->sock.sll_family = AF_PACKET;
	desc->sock.sll_protocol = htons(xi_ether_type);
	desc->sock.sll_halen = ETH_ALEN;

	/* get interface info */
	if ((status = NIC_get_adapter(xi_adapter_mac_address, &adapter)) != RES_RESULT_OK)
	{
		hlb_log_error("(!) failed to query nic device %d\n", status);
		return status;
	}

	desc->sock.sll_ifindex = adapter.idx;

	/* bind to a specific interface */
	if (bind(desc->devfd, (struct sockaddr *)&desc->sock, sizeof(desc->sock)) < 0)
	{
		hlb_log_error("(!) failed to bind!!!\n");
		return RES_RESULT_GENERAL_ERROR;
	}

	desc->sock.sll_protocol = 0;
	return RES_RESULT_OK;
}

static RES_result_t linux_error_code_to_hlb_error_code(int code)
{
	switch (code)
	{
	case 0:
		return RES_RESULT_OK;
		break;
	case EBUSY:
		return RES_RESULT_RESOURCE_IN_USE;
		break;
	case EINVAL:
		return RES_RESULT_BAD_PARAMETER;
		break;
	case EAGAIN:
		return RES_RESULT_RESOURCE_IN_USE;
		break;
	case ENOMEM:
		return RES_RESULT_NO_MEMORY;
		break;
	case EPERM:
		return RES_RESULT_ACCESS_DENIED;
		break;
	case EDEADLK:
		return RES_RESULT_RESOURCE_IN_USE;
		break;
	default:
		break;
	}
	return RES_RESULT_GENERAL_ERROR;
}

/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/
uint32_t get_msectime()
{
	uint32_t mtime = 0;
	struct timeval t_current_time;
	long seconds, useconds;

	gettimeofday(&t_current_time, NULL);

	seconds = t_current_time.tv_sec;
	useconds = t_current_time.tv_usec;
	mtime = seconds * 1000 + useconds / 1000;

	return mtime;
}

void hlb_log_set(bool on)
{
	hlb_log_on = on;
}

void hlb_log_error(const char *message, ...)
{
	va_list args;
	va_start(args, message);
	hlb_log_format(LOG_ERR, "error", message, args);
	va_end(args);
}

void hlb_log_info(const char *message, ...)
{
	va_list args;
	va_start(args, message);
	hlb_log_format(LOG_INFO, "info", message, args);
	va_end(args);
}

void hlb_log_debug(const char *message, ...)
{
	va_list args;
	va_start(args, message);
	hlb_log_format(LOG_DEBUG, "debug", message, args);
	va_end(args);
}

RES_result_t osal_thread_create(void **handle, void *(*task_func)(void *),
								void *param)
{
	pthread_t thread = 0;
	pthread_attr_t attr;
	if (handle == NULL)
	{
		hlb_log_error("handle provided in osal_thread_create is NULL!");
		return RES_RESULT_NULL_PTR;
	}
	if (pthread_attr_init(&attr) == -1)
	{
		return RES_RESULT_GENERAL_ERROR;
	}
	if (0 == pthread_create(&thread, &attr, task_func, param))
	{
		*handle = (void *)thread;
		pthread_attr_destroy(&attr);
		return RES_RESULT_OK;
	}
	pthread_attr_destroy(&attr);

	return RES_RESULT_GENERAL_ERROR;
}

void osal_thread_delete(void **task_handle)
{
	(void)task_handle;
}

int osal_thread_join(void **task_handle)
{
	if (task_handle == NULL)
	{
		hlb_log_error("task_handle provided in osal_thread_create is NULL!");
		return -1;
	}
	return pthread_join((pthread_t)*task_handle, NULL);
}

size_t osal_lock_alloc_size()
{
	return sizeof(pthread_mutex_t);
}

RES_result_t osal_lock_init(void *lock)
{
	int res;
	memset(lock, 0, sizeof(pthread_mutex_t));
	res = pthread_mutex_init((pthread_mutex_t *)lock, NULL);
	return linux_error_code_to_hlb_error_code(res);
}

RES_result_t osal_lock_lock(void *lock)
{
	int res;
	res = pthread_mutex_lock((pthread_mutex_t *)lock);
	return linux_error_code_to_hlb_error_code(res);
}

RES_result_t osal_lock_unlock(void *lock)
{
	int res;
	res = pthread_mutex_unlock((pthread_mutex_t *)lock);
	return linux_error_code_to_hlb_error_code(res);
}

RES_result_t osal_lock_destroy(void *lock)
{
	int res;
	res = pthread_mutex_destroy((pthread_mutex_t *)lock);
	return linux_error_code_to_hlb_error_code(res);
}

/* ETH communication functions */

size_t ETH_handle_alloc_size()
{
	return sizeof(connection_t);
}

RES_result_t ETH_get_mac_address_by_interface_name(const char *iface_name, mac_address_t addr)
{
	RES_result_t res = RES_RESULT_OK;
	struct ifaddrs *ifaddr;
	struct ifaddrs *ifa;
	struct ifreq ireq;
	int devfd;
	int i;
	int devices = 0;

	devfd = socket(PF_INET, SOCK_STREAM, 0);
	if (devfd < 0)
	{
		hlb_log_error("(!) failed to open socket %d\n", errno);
		return RES_RESULT_GENERAL_ERROR;
	}

	/* get interfaces list */
	if (getifaddrs(&ifaddr) == -1)
	{
		hlb_log_error("(!) failed to get list of interfaces (stat=%d)\n", errno);
		close(devfd);
		return RES_RESULT_GENERAL_ERROR;
	}

	/* search adapter */
	for (ifa = ifaddr, i = 0; ifa != NULL; ifa = ifa->ifa_next, i++)
	{
		if ((ifa->ifa_addr == NULL) || (ifa->ifa_addr->sa_family != AF_PACKET))
		{
			continue;
		}

		if (!strcmp(iface_name, ifa->ifa_name))
		{
			strcpy(ireq.ifr_name, ifa->ifa_name);
			if (ioctl(devfd, SIOCGIFFLAGS, &ireq) < 0)
			{
				hlb_log_error("(!)[%d] failed to query Flags for interface %s (stat=%d)\n", i, ifa->ifa_name, errno);
				res = RES_RESULT_GENERAL_ERROR;
			}
			else if (!(ireq.ifr_flags & IFF_RUNNING))
			{
				hlb_log_error("Interface %s is not running\n", ifa->ifa_name);
				res = RES_RESULT_NOT_YET;
			}

			/* get interface MAC address */
			if (ioctl(devfd, SIOCGIFHWADDR, &ireq) < 0)
			{
				hlb_log_error("(!)[%d] failed to query MAC for interface %s (stat=%d)\n", i, ifa->ifa_name, errno);
				res = RES_RESULT_GENERAL_ERROR;
			}
			else
			{
				memcpy(addr, ireq.ifr_hwaddr.sa_data, sizeof(mac_address_t));
				devices++;
			}

			break;
		}
	}

	if (ifaddr != NULL)
	{
		freeifaddrs(ifaddr);
	}
	close(devfd);

	if ((res == RES_RESULT_OK) && (!devices))
	{
		res = RES_RESULT_NOT_FOUND;
	}

	return res;
}

RES_result_t ETH_tx(const void *xi_con, const void *xi_pkt, size_t xi_len)
{
	short len;
	eth_desc_t *netif;

	if (xi_con == NULL)
	{
		hlb_log_error("xi_con provided in ETH_tx is NULL!");
		return RES_RESULT_NULL_PTR;
	}
	if (xi_pkt == NULL)
	{
		hlb_log_error("xi_pkt provided in ETH_tx is NULL!");
		return RES_RESULT_NULL_PTR;
	}

	netif = (eth_desc_t *)&(((connection_t *)xi_con)->adapter);

	// If the packet is smaller than "MIN_ETH_SIZE", add padding at the end
	if (xi_len < MIN_ETH_SIZE)
	{
		char Buffer[MIN_ETH_SIZE];

		memset(Buffer, 0x00, MIN_ETH_SIZE);
		memcpy(Buffer, xi_pkt, xi_len);

		len = sendto(netif->devfd, (unsigned char *)Buffer, MIN_ETH_SIZE, MSG_DONTWAIT | MSG_NOSIGNAL, NULL, 0);
		if (len < 0)
		{
			hlb_log_error("(!) error in sendto, errno=%d\n", errno);
			return RES_RESULT_GENERAL_ERROR;
		}
		return RES_RESULT_OK;
	}

	/* Truncate if packet is too long */
	len = (xi_len < MAX_ETH_SIZE) ? xi_len : MAX_ETH_SIZE;

	len = sendto(netif->devfd, xi_pkt, len, MSG_DONTWAIT | MSG_NOSIGNAL, NULL, 0);
	if (len < 0)
	{
		hlb_log_error("(!) error in sendto, errno=%d\n", errno);
		return RES_RESULT_GENERAL_ERROR;
	}
	return RES_RESULT_OK;
}

RES_result_t ETH_rx(const void *xi_con, void *xo_pkt, size_t *xio_len, int xi_timeout)
{
	eth_desc_t *desc;
	ETH_header_t *eth_header = (ETH_header_t *)xo_pkt;
	struct timeval tv;
	mac_address_t baddr = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	int ret;
	fd_set rdfds;
	int max_fd;

	if (xi_con == NULL)
	{
		hlb_log_error("xi_con provided in ETH_rx is NULL!");
		return RES_RESULT_NULL_PTR;
	}
	if (xo_pkt == NULL)
	{
		hlb_log_error("xo_pkt provided in ETH_rx is NULL!");
		return RES_RESULT_NULL_PTR;
	}
	if (xio_len == NULL)
	{
		hlb_log_error("xio_len provided in ETH_rx is NULL!");
		return RES_RESULT_NULL_PTR;
	}

	if (xi_timeout >= 0)
	{
		tv.tv_sec = xi_timeout / MSEC_IN_SEC;
		tv.tv_usec = (xi_timeout * USEC_IN_MSEC) % USEC_IN_SEC;
	}
	desc = (eth_desc_t *)&(((connection_t *)xi_con)->adapter);
	FD_ZERO(&rdfds);
	do
	{
		FD_SET(desc->devfd, &rdfds);
		FD_SET(desc->pipefd[0], &rdfds);
		max_fd = (desc->devfd > desc->pipefd[0]) ? desc->devfd : desc->pipefd[0];
		if (xi_timeout >= 0)
			ret = select(max_fd + 1, &rdfds, NULL, NULL, &tv);
		else /* blocking */
			ret = select(max_fd + 1, &rdfds, NULL, NULL, NULL);

		if (ret < 0)
		{
			if (errno == EINTR)
			{
				// "interrupted system call" - Ignore that error message, may occure when nic is disconnect and connect
				// when the device is resetting
				continue;
			}
			else
			{
				hlb_log_error("(!) error in ETH_rx:Select failed (ret=%d,errno=%d)\n", ret, errno);
			}
			return RES_RESULT_GENERAL_ERROR;
		}
		else if (ret == 0)
		{
			hlb_log_info("timeout in ETH_rx");
			return RES_RESULT_TIMEOUT;
		}
		else
		{
			if (FD_ISSET(desc->pipefd[0], &rdfds))
			{
				return RES_RESULT_HW_ABORT;
			}
			ret = recvfrom(desc->devfd, xo_pkt, *xio_len, MSG_DONTWAIT, NULL, NULL);
		}

		if (ret >= ETH_ALEN)
		{
			/*
			 * Verify that the packet's MAC destination is for us (MAC of NIC)
			 */
			if (memcmp(eth_header->da, ((connection_t *)(xi_con))->host, ETH_ALEN) != 0)
			{
				/* And not a broadcast packet */
				if (memcmp(eth_header->da, baddr, ETH_ALEN) != 0)
				{
					/* ignore packet */
					continue;
				}
			}

			/*
             *	Filter out packets from other
             *  devices on a unicast connection
             */
			if (memcmp(((connection_t *)(xi_con))->device, baddr, sizeof(mac_address_t)) != 0)
			{
				if (memcmp(eth_header->sa, ((connection_t *)(xi_con))->device, ETH_ALEN) == 0)
				{
					/* Received-Packet is OK */
					break;
				}
			}
			else /* Broadcast message */
			{
				/*
                 *	on broadcast connection we accept
                 *  any packet with our ether type
                 */
				/* Received-Packet is OK */
				break;
			}

			/* ignore packet */
			continue;
		}
		hlb_log_error("(!) error in recvfrom ret=%d, errno=%d\n", ret, errno);
		return RES_RESULT_GENERAL_ERROR;
	} while (1);

	*xio_len = ret;
	return RES_RESULT_OK;
}

void ETH_break_rx(void *xi_con)
{
	connection_t *pconn;

	if (xi_con == NULL)
	{
		hlb_log_error("xi_con provided in ETH_break_rx is NULL!");
		return;
	}

	pconn = (connection_t *)xi_con;
	if (write(pconn->adapter.pipefd[1], "1", 1) ==1)
		return;
}

RES_result_t CM_connect(const uint8_t *xi_adapter_mac_address, const uint8_t *xi_dev_mac_address, uint16_t xi_ether_type, void *xo_handle)
{
	RES_result_t status = RES_RESULT_OK;
	connection_t *pconn;
	pconn = (connection_t *)xo_handle;
	if (xi_adapter_mac_address == NULL)
	{
		hlb_log_error("xi_adapter_mac_address provided in CM_connect is NULL!");
		return RES_RESULT_NULL_PTR;
	}
	if (xo_handle == NULL)
	{
		hlb_log_error("xo_handle provided in CM_connect is NULL!");
		return RES_RESULT_NULL_PTR;
	}

	memset(pconn, 0, sizeof(connection_t));
	memcpy(pconn->host, xi_adapter_mac_address, sizeof(mac_address_t));

	if (xi_dev_mac_address)
	{
		memcpy(pconn->device, xi_dev_mac_address, sizeof(mac_address_t));
	}
	else
	{
		/* if destination is NULL, assume broadcast address */
		memset(pconn->device, 0xFF, sizeof(mac_address_t));
	}

	/* bind nic by its mac address */
	if ((status = ETH_bind_adapter(xi_adapter_mac_address, xi_ether_type, &(pconn->adapter))) != RES_RESULT_OK)
		return status;

	if ((status = pipe(pconn->adapter.pipefd)) == -1) {
		hlb_log_error("pipe error in CM_connect");
		return status;
	}

	return status;
}

void CM_disconnect(void *xi_con)
{
	connection_t *pconn;
	RES_result_t status;
	if (xi_con == NULL)
	{
		hlb_log_error("xi_con provided in CM_disconnect is NULL!");
		return;
	}
	pconn = (connection_t *)xi_con;

	close(pconn->adapter.pipefd[0]);
	close(pconn->adapter.pipefd[1]);

	/* release adapter */
	if ((status = ETH_release_adapter(&(pconn->adapter))) != RES_RESULT_OK)
	{
		hlb_log_error("Failed to release adapter %d\n", status);
	}
}

int osal_cond_lock_lock(void *cond_lock)
{
	return pthread_mutex_lock(&((osal_cond_lock_t *)cond_lock)->indication_db_lock);
}

int osal_cond_lock_unlock(void *cond_lock)
{
	return pthread_mutex_unlock(&((osal_cond_lock_t *)cond_lock)->indication_db_lock);
}

int osal_wait_on_condition(void *cond_lock)
{
	return pthread_cond_wait(&((osal_cond_lock_t *)cond_lock)->indication_db_cond,
					&((osal_cond_lock_t *)cond_lock)->indication_db_lock);
}

int osal_wait_on_timed_condition(int num_secs, void *cond_lock)
{
	struct timespec max_wait = {0, 0};
	clock_gettime(CLOCK_REALTIME, &max_wait);
	max_wait.tv_sec += num_secs;
	return pthread_cond_timedwait(&((osal_cond_lock_t *)cond_lock)->indication_db_cond,
					&((osal_cond_lock_t *)cond_lock)->indication_db_lock, &max_wait);
}

int osal_release_condition(void *cond_lock)
{
	return pthread_cond_signal(&((osal_cond_lock_t *)cond_lock)->indication_db_cond);
}

size_t osal_cond_lock_alloc_size()
{
	return sizeof(osal_cond_lock_t);
}

RES_result_t osal_cond_lock_init(void *cond_lock)
{
	int res;

	memset(cond_lock, 0, sizeof(osal_cond_lock_t));
	res = pthread_mutex_init(&((osal_cond_lock_t *)cond_lock)->indication_db_lock, NULL);
	if (res == 0)
	{
		res = pthread_cond_init(&((osal_cond_lock_t *)cond_lock)->indication_db_cond, NULL);
	}

	return linux_error_code_to_hlb_error_code(res);
}

int osal_update_timestamp(uint64_t *timestamp_secs)
{
	int ret;
	struct timeval timestamp;
	ret = gettimeofday(&timestamp, NULL);
	*timestamp_secs = timestamp.tv_sec;
	return ret;
}

void osal_msleep(uint32_t msecs)
{
	usleep(msecs * 1000);
}

void osal_sleep(uint32_t secs)
{
	sleep(secs);
}

long osal_get_file_size(const char *file_name)
{
	FILE *file;
	long size;

	if ((file = fopen(file_name, "r")))
	{
		fseek(file, 0, SEEK_END);
		size = ftell(file);
		fclose(file);
		return size;
	}
    else
    {
        hlb_log_error("osal_get_file_size: Couldn't open file\n");
    }

	return 0;
}
