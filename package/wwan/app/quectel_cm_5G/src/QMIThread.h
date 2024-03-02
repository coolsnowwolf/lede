#ifndef __QMI_THREAD_H__
#define __QMI_THREAD_H__

#define CONFIG_GOBINET
#define CONFIG_QMIWWAN
#define CONFIG_SIM
#define CONFIG_APN
#define CONFIG_VERSION
//#define CONFIG_SIGNALINFO
//#define CONFIG_CELLINFO
//#define CONFIG_COEX_WWAN_STATE
#define CONFIG_DEFAULT_PDP 1
//#define CONFIG_IMSI_ICCID
#define QUECTEL_UL_DATA_AGG
//#define QUECTEL_QMI_MERGE
//#define REBOOT_SIM_CARD_WHEN_APN_CHANGE
//#define REBOOT_SIM_CARD_WHEN_LONG_TIME_NO_PS 60 //unit is seconds
//#define CONFIG_QRTR
//#define CONFIG_ENABLE_QOS
//#define CONFIG_REG_QOS_IND
//#define CONFIG_GET_QOS_INFO
//#define CONFIG_GET_QOS_DATA_RATE

#if (defined(CONFIG_REG_QOS_IND) || defined(CONFIG_GET_QOS_INFO) || defined(CONFIG_GET_QOS_DATA_RATE))
#ifndef CONFIG_REG_QOS_IND
#define CONFIG_REG_QOS_IND
#endif
#ifndef CONFIG_ENABLE_QOS
#define CONFIG_ENABLE_QOS
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stddef.h>

#include "qendian.h"
#include "QCQMI.h"
#include "QCQCTL.h"
#include "QCQMUX.h"
#include "util.h"

#define DEVICE_CLASS_UNKNOWN           0
#define DEVICE_CLASS_CDMA              1
#define DEVICE_CLASS_GSM               2

#define WWAN_DATA_CLASS_NONE            0x00000000
#define WWAN_DATA_CLASS_GPRS            0x00000001
#define WWAN_DATA_CLASS_EDGE            0x00000002 /* EGPRS */
#define WWAN_DATA_CLASS_UMTS            0x00000004
#define WWAN_DATA_CLASS_HSDPA           0x00000008
#define WWAN_DATA_CLASS_HSUPA           0x00000010
#define WWAN_DATA_CLASS_LTE             0x00000020
#define WWAN_DATA_CLASS_5G_NSA        0x00000040
#define WWAN_DATA_CLASS_5G_SA        0x00000080
#define WWAN_DATA_CLASS_1XRTT           0x00010000
#define WWAN_DATA_CLASS_1XEVDO          0x00020000
#define WWAN_DATA_CLASS_1XEVDO_REVA     0x00040000
#define WWAN_DATA_CLASS_1XEVDV          0x00080000
#define WWAN_DATA_CLASS_3XRTT           0x00100000
#define WWAN_DATA_CLASS_1XEVDO_REVB     0x00200000 /* for future use */
#define WWAN_DATA_CLASS_UMB             0x00400000
#define WWAN_DATA_CLASS_CUSTOM          0x80000000

struct wwan_data_class_str {
    ULONG class;
    const char *str;
};

#pragma pack(push, 1)
typedef struct __IPV4 {
    uint32_t Address;
    uint32_t Gateway;
    uint32_t SubnetMask;
    uint32_t DnsPrimary;
    uint32_t DnsSecondary;
    uint32_t Mtu;
} IPV4_T;

typedef struct __IPV6 {
    UCHAR Address[16];
    UCHAR Gateway[16];
    UCHAR SubnetMask[16];
    UCHAR DnsPrimary[16];
    UCHAR DnsSecondary[16];
    UCHAR PrefixLengthIPAddr;
    UCHAR PrefixLengthGateway;
    ULONG Mtu;
} IPV6_T;

typedef struct {
    UINT size;
    UINT rx_urb_size;
    UINT ep_type;
    UINT iface_id;
    UINT MuxId;
    UINT ul_data_aggregation_max_datagrams; //0x17
    UINT ul_data_aggregation_max_size ;//0x18
    UINT dl_minimum_padding; //0x1A
} QMAP_SETTING;

//Configured downlink data aggregationprotocol
#define WDA_DL_DATA_AGG_DISABLED (0x00) //DL data aggregation is disabled (default)
#define WDA_DL_DATA_AGG_TLP_ENABLED (0x01) // DL TLP is enabled
#define WDA_DL_DATA_AGG_QC_NCM_ENABLED (0x02) // DL QC_NCM isenabled
#define WDA_DL_DATA_AGG_MBIM_ENABLED (0x03) // DL MBIM isenabled
#define WDA_DL_DATA_AGG_RNDIS_ENABLED (0x04) // DL RNDIS is enabled
#define WDA_DL_DATA_AGG_QMAP_ENABLED (0x05) // DL QMAP isenabled
#define WDA_DL_DATA_AGG_QMAP_V2_ENABLED (0x06) // DL QMAP V2 is enabled
#define WDA_DL_DATA_AGG_QMAP_V3_ENABLED (0x07) // DL QMAP V3 is enabled
#define WDA_DL_DATA_AGG_QMAP_V4_ENABLED (0x08) // DL QMAP V4 is enabled
#define WDA_DL_DATA_AGG_QMAP_V5_ENABLED (0x09) // DL QMAP V5 is enabled

typedef struct {
    unsigned int size;
    unsigned int rx_urb_size;
    unsigned int ep_type;
    unsigned int iface_id;
    unsigned int qmap_mode;
    unsigned int qmap_version;
    unsigned int dl_minimum_padding;
    char ifname[8][16];
    unsigned char mux_id[8];
} RMNET_INFO;

#define IpFamilyV4 (0x04)
#define IpFamilyV6 (0x06)

struct __PROFILE;
struct qmi_device_ops {
	int (*init)(struct __PROFILE *profile);
	int (*deinit)(void);
	int (*send)(PQCQMIMSG pRequest);
	void* (*read)(void *pData);
};
#ifdef CONFIG_QRTR
extern const struct qmi_device_ops qrtr_qmidev_ops;
#endif
extern const struct qmi_device_ops gobi_qmidev_ops;
extern const struct qmi_device_ops qmiwwan_qmidev_ops;
extern const struct qmi_device_ops mbim_dev_ops;
extern const struct qmi_device_ops atc_dev_ops;
extern int (*qmidev_send)(PQCQMIMSG pRequest);

struct usb_device_info {
    int idVendor;
    int idProduct;
    int  busnum;
    int devnum;
    int bNumInterfaces;
};

struct usb_interface_info {
    int bNumEndpoints;
    int bInterfaceClass;
    int bInterfaceSubClass;
    int bInterfaceProtocol;
    char driver[32];
};

#define LIBQMI_PROXY "qmi-proxy" //src/libqmi-glib/qmi-proxy.h
#define LIBMBIM_PROXY "mbim-proxy"
#define QUECTEL_QMI_PROXY "quectel-qmi-proxy"
#define QUECTEL_MBIM_PROXY "quectel-mbim-proxy"
#define QUECTEL_ATC_PROXY "quectel-atc-proxy"
#define QUECTEL_QRTR_PROXY "quectel-qrtr-proxy"

#ifndef bool
#define bool uint8_t
#endif
struct request_ops;
typedef struct __PROFILE {
    //user input start
    const char *apn;
    const char *user;
    const char *password;
    int auth;
    int iptype;
    const char *pincode;
    char proxy[32];
    int pdp;//pdp_context
    int profile_index;//profile_index
    int enable_bridge;
    bool enable_ipv4;
    bool enable_ipv6;
    bool no_dhcp;
    const char *logfile;
    const char *usblogfile;
    char expect_adapter[32];
    int kill_pdp;
    int replication_factor;
    //user input end

    char qmichannel[32];
    char usbnet_adapter[32];
    char qmapnet_adapter[32];
    char driver_name[32];
    int qmap_mode;
    int qmap_size;
    int qmap_version;
    int curIpFamily;
    int rawIP;
    int muxid;
#ifdef CONFIG_ENABLE_QOS
    UINT qos_id;
#endif
    int wda_client;
    uint32_t udhcpc_ip;
    IPV4_T ipv4;
    IPV6_T ipv6;
    UINT PCSCFIpv4Addr1;
    UINT PCSCFIpv4Addr2;
    UCHAR PCSCFIpv6Addr1[16];
    UCHAR PCSCFIpv6Addr2[16];
    bool reattach_flag;
    int hardware_interface;
    int software_interface;

    struct usb_device_info usb_dev;
    struct usb_interface_info usb_intf;

    int usbmon_fd;
    FILE  *usbmon_logfile_fp;
    bool loopback_state;

    char BaseBandVersion[64];
    char old_apn[64];
    char old_user[64];
    char old_password[64];
    int old_auth;
    int old_iptype;

    const struct qmi_device_ops *qmi_ops;
    const struct request_ops *request_ops;
    RMNET_INFO rmnet_info;
} PROFILE_T;

#ifdef QUECTEL_QMI_MERGE
#define MERGE_PACKET_IDENTITY 0x2c7c
#define MERGE_PACKET_VERSION 0x0001
#define MERGE_PACKET_MAX_PAYLOAD_SIZE 56
typedef struct __QMI_MSG_HEADER {
    uint16_t idenity;
    uint16_t version;
    uint16_t cur_len;
    uint16_t total_len;
} QMI_MSG_HEADER;

typedef struct __QMI_MSG_PACKET {
    QMI_MSG_HEADER header;
    uint16_t len;
    char buf[4096];
} QMI_MSG_PACKET;
#endif

typedef enum {
    SIM_ABSENT = 0,
    SIM_NOT_READY = 1,
    SIM_READY = 2, /* SIM_READY means the radio state is RADIO_STATE_SIM_READY */
    SIM_PIN = 3,
    SIM_PUK = 4,
    SIM_NETWORK_PERSONALIZATION = 5,
    SIM_BAD = 6,
} SIM_Status;

#pragma pack(pop)

#define WDM_DEFAULT_BUFSIZE	256
#define RIL_REQUEST_QUIT    0x1000
#define RIL_INDICATE_DEVICE_CONNECTED    0x1002
#define RIL_INDICATE_DEVICE_DISCONNECTED    0x1003
#define RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED    0x1004
#define RIL_UNSOL_DATA_CALL_LIST_CHANGED    0x1005
#define MODEM_REPORT_RESET_EVENT 0x1006
#define RIL_UNSOL_LOOPBACK_CONFIG_IND 0x1007
#ifdef CONFIG_REG_QOS_IND
#define RIL_UNSOL_GLOBAL_QOS_FLOW_IND_QOS_ID 0x1008
#endif

extern pthread_mutex_t cm_command_mutex;
extern pthread_cond_t cm_command_cond;
extern unsigned int cm_recv_buf[1024];
extern int cm_open_dev(const char *dev);
extern int cm_open_proxy(const char *name);
extern int pthread_cond_timeout_np(pthread_cond_t *cond, pthread_mutex_t * mutex, unsigned msecs);
extern int QmiThreadSendQMITimeout(PQCQMIMSG pRequest, PQCQMIMSG *ppResponse, unsigned msecs, const char *funcname);
#define QmiThreadSendQMI(pRequest, ppResponse) QmiThreadSendQMITimeout(pRequest, ppResponse, 30 * 1000, __func__)
extern void QmiThreadRecvQMI(PQCQMIMSG pResponse);
extern void udhcpc_start(PROFILE_T *profile);
extern void udhcpc_stop(PROFILE_T *profile);
extern void ql_set_driver_link_state(PROFILE_T *profile, int link_state);
extern void ql_set_driver_qmap_setting(PROFILE_T *profile, QMAP_SETTING *qmap_settings);
extern void ql_get_driver_rmnet_info(PROFILE_T *profile, RMNET_INFO *rmnet_info);
extern void dump_qmi(void *dataBuffer, int dataLen);
extern void qmidevice_send_event_to_main(int triger_event);
extern void qmidevice_send_event_to_main_ext(int triger_event, void *data, unsigned len);
extern uint8_t qmi_over_mbim_get_client_id(uint8_t QMIType);
extern uint8_t qmi_over_mbim_release_client_id(uint8_t QMIType, uint8_t ClientId);
#ifdef CONFIG_REG_QOS_IND
extern UCHAR ql_get_global_qos_flow_ind_qos_id(PQCQMIMSG pResponse, UINT *qos_id);
#endif
#ifdef CONFIG_GET_QOS_DATA_RATE
extern UCHAR ql_get_global_qos_flow_ind_data_rate(PQCQMIMSG pResponse, void *max_data_rate);
#endif

struct request_ops {
    int (*requestBaseBandVersion)(PROFILE_T *profile);
    int (*requestSetEthMode)(PROFILE_T *profile);
    int (*requestSetLoopBackState)(UCHAR loopback_state, ULONG replication_factor);
    int (*requestGetSIMStatus)(SIM_Status *pSIMStatus);
    int (*requestEnterSimPin)(const char *pPinCode);
    int (*requestSetProfile)(PROFILE_T *profile); // 1 ~ success and apn change, 0 ~ success and no apn change, -1 ~ fail
    int (*requestGetProfile)(PROFILE_T *profile);
    int (*requestRegistrationState)(UCHAR *pPSAttachedState);
    int (*requestSetupDataCall)(PROFILE_T *profile, int curIpFamily);
    int (*requestQueryDataCall)(UCHAR  *pConnectionStatus, int curIpFamily);
    int (*requestDeactivateDefaultPDP)(PROFILE_T *profile, int curIpFamily);
    int (*requestGetIPAddress)(PROFILE_T *profile, int curIpFamily);
    int (*requestGetSignalInfo)(void);
    int (*requestGetCellInfoList)(void);
    int (*requestGetICCID)(void);
    int (*requestGetIMSI)(void);
    int (*requestRadioPower)(int state);
    int (*requestRegisterQos)(PROFILE_T *profile);
    int (*requestGetQosInfo)(PROFILE_T *profile);
    int (*requestGetCoexWWANState)(void);
};
extern const struct request_ops qmi_request_ops;
extern const struct request_ops mbim_request_ops;
extern const struct request_ops atc_request_ops;

extern int get_driver_type(PROFILE_T *profile);
extern BOOL qmidevice_detect(char *qmichannel, char *usbnet_adapter, unsigned bufsize, PROFILE_T *profile);
int mhidevice_detect(char *qmichannel, char *usbnet_adapter, PROFILE_T *profile);
int atdevice_detect(char *atchannel, char *usbnet_adapter, PROFILE_T *profile);
extern int ql_bridge_mode_detect(PROFILE_T *profile);
extern int ql_enable_qmi_wwan_rawip_mode(PROFILE_T *profile);
extern int ql_qmap_mode_detect(PROFILE_T *profile);
#ifdef CONFIG_QRTR
extern int rtrmnet_ctl_create_vnd(char *devname, char *vndname, uint8_t muxid,
		       uint32_t qmap_version, uint32_t ul_agg_cnt, uint32_t ul_agg_size);
#endif

#define qmidev_is_gobinet(_qmichannel) (strncmp(_qmichannel, "/dev/qcqmi", strlen("/dev/qcqmi")) == 0)
#define qmidev_is_qmiwwan(_qmichannel) (strncmp(_qmichannel, "/dev/cdc-wdm", strlen("/dev/cdc-wdm")) == 0)
#define qmidev_is_pciemhi(_qmichannel) (strncmp(_qmichannel, "/dev/mhi_", strlen("/dev/mhi_")) == 0)

#define driver_is_qmi(_drv_name) (strncasecmp(_drv_name, "qmi_wwan", strlen("qmi_wwan")) == 0)
#define driver_is_mbim(_drv_name) (strncasecmp(_drv_name, "cdc_mbim", strlen("cdc_mbim")) == 0)

extern FILE *logfilefp;
extern int debug_qmi;
extern int qmidevice_control_fd[2];
extern int g_donot_exit_when_modem_hangup;
extern void update_resolv_conf(int iptype, const char *ifname, const char *dns1, const char *dns2);
void update_ipv4_address(const char *ifname, const char *ip, const char *gw, unsigned prefix);
void update_ipv6_address(const char *ifname, const char *ip, const char *gw, unsigned prefix);
int reattach_driver(PROFILE_T *profile);
extern void no_trunc_strncpy(char *dest, const char *src, size_t dest_size);

enum
{
	DRV_INVALID,
	SOFTWARE_QMI,
	SOFTWARE_MBIM,
	SOFTWARE_ECM_RNDIS_NCM,
	SOFTWARE_QRTR,
	HARDWARE_PCIE,
	HARDWARE_USB,
};

enum
{
    SIG_EVENT_START,
    SIG_EVENT_CHECK,
    SIG_EVENT_STOP,
};

typedef enum
{
    DMS_OP_MODE_ONLINE,
    DMS_OP_MODE_LOW_POWER,
    DMS_OP_MODE_FACTORY_TEST_MODE,
    DMS_OP_MODE_OFFLINE,
    DMS_OP_MODE_RESETTING,
    DMS_OP_MODE_SHUTTING_DOWN,
    DMS_OP_MODE_PERSISTENT_LOW_POWER,
    DMS_OP_MODE_MODE_ONLY_LOW_POWER,
    DMS_OP_MODE_NET_TEST_GW,
}Device_operating_mode;

#ifdef CM_DEBUG
#define dbg_time(fmt, args...) do { \
    fprintf(stdout, "[%15s-%04d: %s] " fmt "\n", __FILE__, __LINE__, get_time(), ##args); \
	fflush(stdout);\
    if (logfilefp) fprintf(logfilefp, "[%s-%04d: %s] " fmt "\n", __FILE__, __LINE__, get_time(), ##args); \
} while(0)
#else
#define dbg_time(fmt, args...) do { \
    fprintf(stdout, "[%s] " fmt "\n", get_time(), ##args); \
	fflush(stdout);\
    if (logfilefp) fprintf(logfilefp, "[%s] " fmt "\n", get_time(), ##args); \
} while(0)
#endif
#endif
