#ifndef __QMI_THREAD_H__
#define __QMI_THREAD_H__

#define CONFIG_GOBINET
#define CONFIG_QMIWWAN
#define CONFIG_SIM
#define CONFIG_APN
#define CONFIG_VERSION
//2021-03-24 willa.liu@fibocom.com changed begin for support mantis 0071817
#define CONFIG_IMSI_ICCID
//2021-03-24 willa.liu@fibocom.com changed end for support mantis 0071817
#define CONFIG_DEFAULT_PDP 1
#define CONFIG_DEFAULT_PDPINDEX 1
//#define CONFIG_IMSI_ICCID

#define CONFIG_RESET_RADIO \
    (45)  // Reset Radiao(AT+CFUN=4,AT+CFUN=1) when cann not register network or
	  // setup data call in 45 seconds

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "MPQCTL.h"
#include "MPQMI.h"
#include "MPQMUX.h"

#define DEVICE_CLASS_UNKNOWN 0
#define DEVICE_CLASS_CDMA 1
#define DEVICE_CLASS_GSM 2

#define WWAN_DATA_CLASS_NONE 0x00000000
#define WWAN_DATA_CLASS_GPRS 0x00000001
#define WWAN_DATA_CLASS_EDGE 0x00000002 /* EGPRS */
#define WWAN_DATA_CLASS_UMTS 0x00000004
#define WWAN_DATA_CLASS_HSDPA 0x00000008
#define WWAN_DATA_CLASS_HSUPA 0x00000010
#define WWAN_DATA_CLASS_LTE 0x00000020
//begin modified by zhangkaibo add 5G network detect feature on x55 platform. 20200605
#define WWAN_DATA_CLASS_5G 0x00000040
//end modified by zhangkaibo add 5G network detect feature on x55 platform. 20200605
#define WWAN_DATA_CLASS_1XRTT 0x00010000
#define WWAN_DATA_CLASS_1XEVDO 0x00020000
#define WWAN_DATA_CLASS_1XEVDO_REVA 0x00040000
#define WWAN_DATA_CLASS_1XEVDV 0x00080000
#define WWAN_DATA_CLASS_3XRTT 0x00100000
#define WWAN_DATA_CLASS_1XEVDO_REVB 0x00200000 /* for future use */
#define WWAN_DATA_CLASS_UMB 0x00400000
#define WWAN_DATA_CLASS_CUSTOM 0x80000000

struct wwan_data_class_str {
    ULONG class;
    CHAR *str;
};

#pragma pack(push, 1)

typedef struct _QCQMIMSG {
    QCQMI_HDR QMIHdr;
    union {
	QMICTL_MSG CTLMsg;
	QMUX_MSG MUXMsg;
//2021-03-24 willa.liu@fibocom.com changed begin for support mantis 0071817
    QMUX_MSG QMUXMsgHdrResp;
//2021-03-24 willa.liu@fibocom.com changed end for support mantis 0071817
    };
} __attribute__((packed)) QCQMIMSG, *PQCQMIMSG;

#pragma pack(pop)

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

#define IpFamilyV4 (0x04)
#define IpFamilyV6 (0x06)

struct __PROFILE;
struct qmi_device_ops {
    int (*init)(struct __PROFILE *profile);
    int (*deinit)(void);
    int (*send)(PQCQMIMSG pRequest);
    void *(*read)(void *pData);
};
extern int (*qmidev_send)(PQCQMIMSG pRequest);

#ifndef bool
#define bool uint8_t
#endif
//2021-03-15 zhangkaibo@fibocom.com changed begin for oa 20210311037
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
//2021-03-15 zhangkaibo@fibocom.com changed end for oa 20210311037

typedef struct __PROFILE {
    char *qmichannel;
    char *usbnet_adapter;
    char *qmapnet_adapter;
    const char *driver_name;
    int qmap_mode;
    int qmap_size;
    int qmap_version;
    const char *apn;
    const char *user;
    const char *password;
    const char *pincode;
    int auth;
    int pdp;
    int pdpindex;
    int pdpnum;
    int curIpFamily;
    int rawIP;
    int muxid;
    IPV4_T ipv4;
    IPV6_T ipv6;
    int ipv4_flag;
    int ipv6_flag;

//2021-02-25 willa.liu@fibocom.com changed begin for support eipd SN-20210129001
    int ipv6_prigateway_flag;
//2021-02-25 willa.liu@fibocom.com changed begin for support eipd SN-20210129001

    int dual_flag;
    int apntype;
    const struct qmi_device_ops *qmi_ops;
    bool loopback_state;
    int replication_factor;
//2021-02-08 zhangkaibo@fibocom.com changed begin for mantis 0070613
    int interfacenum;
//2021-02-08 zhangkaibo@fibocom.com changed end for mantis 0070613
//2021-03-15 zhangkaibo@fibocom.com changed begin for oa 20210311037
    RMNET_INFO rmnet_info;
//2021-03-15 zhangkaibo@fibocom.com changed end for oa 20210311037
} PROFILE_T;

typedef enum {
    SIM_ABSENT = 0,
    SIM_NOT_READY = 1,
    SIM_READY =
	2, /* SIM_READY means the radio state is RADIO_STATE_SIM_READY */
    SIM_PIN = 3,
    SIM_PUK = 4,
    SIM_NETWORK_PERSONALIZATION = 5,
    SIM_BAD = 6,
} SIM_Status;
//2021-03-24 willa.liu@fibocom.com changed begin for support mantis 0071817
typedef enum {
    SIM_Card0 = 0,
    SIM_Card1 = 1
} SIM_Select;
//2021-03-24 willa.liu@fibocom.com changed end for support mantis 0071817
#define WDM_DEFAULT_BUFSIZE 256
#define RIL_REQUEST_QUIT 0x1000
#define RIL_INDICATE_DEVICE_CONNECTED 0x1002
#define RIL_INDICATE_DEVICE_DISCONNECTED 0x1003
#define RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED 0x1004
#define RIL_UNSOL_DATA_CALL_LIST_CHANGED 0x1005

extern int pthread_cond_timeout_np(pthread_cond_t *cond, pthread_mutex_t *mutex,
				   unsigned msecs);
extern int QmiThreadSendQMI(PQCQMIMSG pRequest, PQCQMIMSG *ppResponse);
extern int QmiThreadSendQMITimeout(PQCQMIMSG pRequest, PQCQMIMSG *ppResponse,
				   unsigned msecs);
extern void QmiThreadRecvQMI(PQCQMIMSG pResponse);
extern int fibo_raw_ip_mode_check(const char *ifname);
extern void udhcpc_start(PROFILE_T *profile);
extern void udhcpc_stop(PROFILE_T *profile);
extern void udhcpc_start_pcie(PROFILE_T *profile);
extern void udhcpc_stop_pcie(PROFILE_T *profile);

extern void dump_qmi(void *dataBuffer, int dataLen);
extern void qmidevice_send_event_to_main(int triger_event);
extern int requestSetEthMode(PROFILE_T *profile);
//2021-03-24 willa.liu@fibocom.com changed begin for support mantis 0071817
//extern int requestGetSIMStatus(SIM_Status *pSIMStatus);
extern int requestGetSIMStatus(SIM_Status *pSIMStatus , const int sim_select);
//2021-03-24 willa.liu@fibocom.com changed end for support mantis 0071817
extern int requestEnterSimPin(const CHAR *pPinCode);
extern int requestGetICCID(void);
extern int requestGetIMSI(void);
extern int requestRegistrationState(UCHAR *pPSAttachedState);
extern int requestQueryDataCall(UCHAR *pConnectionStatus, int curIpFamily);
extern int requestSetupDataCall(PROFILE_T *profile, int curIpFamily);
extern int requestDeactivateDefaultPDP(PROFILE_T *profile, int curIpFamily);
extern int requestSetProfile(PROFILE_T *profile);
extern int requestGetProfile(PROFILE_T *profile);
extern int requestBaseBandVersion(const char **pp_reversion);
extern int requestGetIPAddress(PROFILE_T *profile, int curIpFamily);
extern int requestSetOperatingMode(UCHAR OperatingMode);
int requestRegistrationState2(UCHAR *pPSAttachedState);

extern int fibo_qmap_mode_set(PROFILE_T *profile);
extern int fibo_bridge_mode_detect(PROFILE_T *profile);
extern int fibo_qmap_mode_detect(PROFILE_T *profile);
extern const struct qmi_device_ops qmiwwan_qmidev_ops;

#define qmidev_is_gobinet(_qmichannel) \
    (strncmp(_qmichannel, "/dev/qcqmi", strlen("/dev/qcqmi")) == 0)
#define qmidev_is_qmiwwan(_qmichannel) \
    (strncmp(_qmichannel, "/dev/cdc-wdm", strlen("/dev/cdc-wdm")) == 0)
#define qmidev_is_pciemhi(_qmichannel) \
    (strncmp(_qmichannel, "/dev/mhi_", strlen("/dev/mhi_")) == 0)

#define driver_is_qmi(_drv_name) \
    (strncasecmp(_drv_name, "qmi_wwan", strlen("qmi_wwan")) == 0)
#define driver_is_mbim(_drv_name) \
    (strncasecmp(_drv_name, "cdc_mbim", strlen("cdc_mbim")) == 0)

extern FILE *logfilefp;
extern int debug_qmi;
extern USHORT g_MobileCountryCode;
extern USHORT g_MobileNetworkCode;
extern int qmidevice_control_fd[2];
extern int qmiclientId[QMUX_TYPE_WDS_ADMIN + 1];
extern void dbg_time(const char *fmt, ...);
extern USHORT le16_to_cpu(USHORT v16);
extern UINT le32_to_cpu(UINT v32);
extern USHORT cpu_to_le16(USHORT v16);
extern UINT cpu_to_le32(UINT v32);
#endif
