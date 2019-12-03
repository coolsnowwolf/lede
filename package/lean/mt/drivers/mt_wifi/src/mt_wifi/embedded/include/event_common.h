#ifdef VENDOR_FEATURE7_SUPPORT
#ifndef __EVENT_COMMON_API__
#define __EVENT_COMMON_API__

#include <linux/if_ether.h>
#ifdef ARRIS_EVENT_USERSPACE
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#endif


/********************************************************************
 *                              COMMON                              *
 ********************************************************************/
#define ARRIS_EVENT_ETHERTYPE    0x88B6 /* Public use ethertype */
#define ARRIS_EVENT_PROC_ENTRY  "/proc/arris_event"

#define OK                      0
#define ERROR                   -1
#define WIFI_24_RADIO           1
#define WIFI_5_RADIO            9


/* #define DEBUG                   1 */

#define int8                    char
#define uint8                   unsigned char
#define uint                    unsigned int

#define MYSELF                  ATOM_HOST     /* This needs to be defined per system. See enum below. */
/* #define ETH_SRC_DEV           "l2sd0.4093"  This needs to be defined per system (ARM)*/
#define ETH_SRC_DEV             "eth0.4093"			/* This needs to be defined per system (ATOM) */
/* #define ETH_SRC_DEV           "pcie.4093"   This needs to be defined per system (QTN) */
#define ETH_RCV_DEV             "eth0.4093"   /* Default listening interface */

#define MAX_VALUE_LENGTH        1450

/* rev_rpc parameters - copied from rev_rpc_api.h (this enum should stay in sync with rev rpc) */
enum {
	E_WIFI_CONFIGURATION = 0,  /* request ARM to send /tmp/wifi_defaults.dat */
	E_STATUS_EVENT,            /* report current Wi-Fi status to ARM, for LGI project */
	E_STA_ASSOCIATION,         /* E_STA_ASSOCIATE */
	E_STA_DISSOCIATION,        /* E_STA_DISSOCIATION */
	E_WPS_AP_CONFIGURED_EVENT, /* E_WPS_AP_CONFIGURED_EVENT, // report WP AP Configured event */
	E_WIFI_RELOAD                /* request ARM to perform a full WIFI hardware/software reload */
};

/* eventd host */
enum {
	ARM_HOST   = 0, /* ARM */
	ATOM_HOST  = 1, /* ATOM */
	OTHER_HOST  = 2,
	MAIN_HOST  = ATOM_HOST, /* Host where circular NVRAM log file resides */
	MAX_HOST   = OTHER_HOST
};

/* event types */
enum {
	SOLICIT = 0,
	/* WLAN event types */
	WLAN_LOG_CONSOLE = 1,
	WLAN_LOG_SAVE = 2,
	WLAN_LOG_SAVE_CONSOLE = 3,
	WLAN_EVENT = 4,                /* This event type directly calls ARRIS Rev rpc */
	/* Other event types */
	GENERIC,
	SET_DESTINATION_ADDR = 50,
	MAX_TYPE
};

/* event subtypes - solicit */
enum {
	SOLICIT_REQUEST = 0,
	SOLICIT_RESPONSE = 1
};

/* event subtypes - wlan_event */
enum {
	FIRST_WLAN_EVENT_SUBTYPE = 0,
	WLAN_STATUS1,
	STA_ASSOC = E_STA_ASSOCIATION,
	STA_DISSOC = E_STA_DISSOCIATION,
	WPS_AP_CONFIGURED = E_WPS_AP_CONFIGURED_EVENT,
	ASIC_RELOAD_EVENT = E_WIFI_RELOAD,
	WPS_FAIL,
	WPS_OOB,
	MAX_WLAN_EVENT_SUBTYPE
};

typedef struct __attribute__((__packed__))
{
	uint8 bssIdx;
	uint8 bssid[6];
	uint8 sta_mac[6];
} STA_DATA_T;

typedef struct __attribute__((__packed__))
{
	uint8 bssIdx;
	char SSID[32];
	uint8 authMode;
	uint8 encMod;
	char key[64];
} BSS_DATA_T;

/* Event packet data structure */
typedef struct __attribute__((__packed__))
{
	struct ethhdr eth;
	uint8 host;
	uint8 type;
	uint8 subtype;
	int len;
	union {
		int imsg;
		char cmsg[MAX_VALUE_LENGTH]; /* generic msg */
		BSS_DATA_T bss_data;
		STA_DATA_T sta_data;
    } u;
} EVENT_T;


/********************************************************************
 *                            KERNEL                                *
 ********************************************************************/
#ifdef ARRIS_EVENT_KERNEL
#define dump_mac(x)	printk("%02X:%02X:%02X:%02X:%02X:%02X\n", x[0], x[1], x[2], x[3], x[4], x[5])
int k_xmit_event(uint8 host, uint8 *dmac, uint8 type, uint8 subtype, void *data, int len);
int find_listener(uint8 host, uint8 *dmac);
void event_handler(uint8 host, uint8 type, uint8 subtype, void *data, int len);
#endif  /* ARRIS_EVENT_KERNEL */



/********************************************************************
 *                           USERSPACE                              *
 ********************************************************************/
#ifdef ARRIS_EVENT_USERSPACE
#undef dbg_print
#define dbg_print(fmt, args...)     ({if ((access("/tmp/arris_event", F_OK) != -1)) fprintf(stdout, "[ARRIS_EVENT] \e[1;40m%s\e[0m "fmt, __func__, ##args); fflush(stdout); })
#define print_console(fmt, args...) ({fprintf(stdout, fmt, ##args); fflush(stdout); })
#undef print_err
#define print_err(fmt, args...)     ({fprintf(stderr, "[ARRIS_EVENT] \e[1;40m**ERROR**\e[0m %s "fmt, __func__, ##args); fflush(stderr); })
#define dump_mac(x)                 printf("%02X:%02X:%02X:%02X:%02X:%02X\n", x[0], x[1], x[2], x[3], x[4], x[5])
int setup_sock(char *interface);
int sendPacket(int *sock, uint8 host, uint8 *dmac, uint8 type, uint8 subtype, void *data, int len);
int send_arris_event(uint8 host, uint8 type, uint8 subtype, void *data, int len);
int u_xmit_event(uint8 host, uint8 *dmac, uint8 type, uint8 subtype, void *data, int len);
#endif /* ARRIS_EVENT_USERSPACE */

#endif /* __EVENT_COMMON_API__ */
#endif /* VENDOR_FEATURE7_SUPPORT */
