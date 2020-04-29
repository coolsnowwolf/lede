#ifndef __QMI_THREAD_H__
#define __QMI_THREAD_H__

#define CONFIG_GOBINET
#define CONFIG_QMIWWAN
#define CONFIG_SIM
#define CONFIG_APN
#define CONFIG_VERSION
#define CONFIG_DEFAULT_PDP 1
//#define CONFIG_IMSI_ICCID
#ifndef ANDROID
#define CONFIG_RESET_RADIO (45) //Reset Radiao(AT+CFUN=4,AT+CFUN=1) when cann not register network or setup data call in 45 seconds
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
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stddef.h>

#include "MPQMI.h"
#include "MPQCTL.h"
#include "MPQMUX.h"

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
    CHAR *str;
};

#pragma pack(push, 1)

typedef struct _QCQMIMSG {
    QCQMI_HDR QMIHdr;
    union {
        QMICTL_MSG CTLMsg;
        QMUX_MSG MUXMsg;
    };
} __attribute__ ((packed)) QCQMIMSG, *PQCQMIMSG;

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
typedef struct __PROFILE {
    char * qmichannel;
    char * usbnet_adapter;
    const char *apn;
    const char *user;
    const char *password;
    const char *pincode;
    int auth;
    int pdp;
    int IsDualIPSupported;
    int curIpFamily;
    int rawIP;
    IPV4_T ipv4;
    IPV6_T ipv6;
} PROFILE_T;

typedef enum {
    SIM_ABSENT = 0,
    SIM_NOT_READY = 1,
    SIM_READY = 2, /* SIM_READY means the radio state is RADIO_STATE_SIM_READY */
    SIM_PIN = 3,
    SIM_PUK = 4,
    SIM_NETWORK_PERSONALIZATION = 5,
    SIM_BAD = 6,
} SIM_Status;

#define WDM_DEFAULT_BUFSIZE	256
#define RIL_REQUEST_QUIT    0x1000
#define RIL_INDICATE_DEVICE_CONNECTED    0x1002
#define RIL_INDICATE_DEVICE_DISCONNECTED    0x1003
#define RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED    0x1004
#define RIL_UNSOL_DATA_CALL_LIST_CHANGED    0x1005

extern int pthread_cond_timeout_np(pthread_cond_t *cond, pthread_mutex_t * mutex, unsigned msecs);
extern int QmiThreadSendQMI(PQCQMIMSG pRequest, PQCQMIMSG *ppResponse);
extern int QmiThreadSendQMITimeout(PQCQMIMSG pRequest, PQCQMIMSG *ppResponse, unsigned msecs);
extern void QmiThreadRecvQMI(PQCQMIMSG pResponse);
extern int QmiWwanInit(PROFILE_T *profile);
extern int QmiWwanDeInit(void);
extern int QmiWwanSendQMI(PQCQMIMSG pRequest);
extern void * QmiWwanThread(void *pData);
extern int GobiNetSendQMI(PQCQMIMSG pRequest);
extern void * GobiNetThread(void *pData);
extern void udhcpc_start(PROFILE_T *profile);
extern void udhcpc_stop(PROFILE_T *profile);
extern void dump_qmi(void *dataBuffer, int dataLen);
extern void qmidevice_send_event_to_main(int triger_event);
extern int requestSetEthMode(PROFILE_T *profile);
extern int requestGetSIMStatus(SIM_Status *pSIMStatus);
extern int requestEnterSimPin(const CHAR *pPinCode);
extern int requestGetICCID(void);
extern int requestGetIMSI(void);
extern int requestRegistrationState(UCHAR *pPSAttachedState);
extern int requestQueryDataCall(UCHAR  *pConnectionStatus, int curIpFamily);
extern int requestSetupDataCall(PROFILE_T *profile, int curIpFamily);
extern int requestDeactivateDefaultPDP(PROFILE_T *profile, int curIpFamily);
extern int requestSetProfile(PROFILE_T *profile);
extern int requestGetProfile(PROFILE_T *profile);
extern int requestBaseBandVersion(const char **pp_reversion);
extern int requestGetIPAddress(PROFILE_T *profile, int curIpFamily);
extern int requestSetOperatingMode(UCHAR OperatingMode);

extern FILE *logfilefp;
extern int debug_qmi;
extern char * qmichannel;
extern int qmidevice_control_fd[2];
extern int qmiclientId[QMUX_TYPE_WDS_ADMIN + 1];
extern int cdc_wdm_fd;
extern void dbg_time (const char *fmt, ...);
extern USHORT le16_to_cpu(USHORT v16);
extern UINT  le32_to_cpu (UINT v32);
extern UINT  ql_swap32(UINT v32);
extern USHORT cpu_to_le16(USHORT v16);
extern UINT cpu_to_le32(UINT v32);
#endif
