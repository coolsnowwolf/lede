/******************************************************************************
  @file    mbim-cm.c
  @brief   MIBIM drivers.

  DESCRIPTION
  Connectivity Management Tool for USB network adapter of Quectel wireless cellular modules.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None.

  ---------------------------------------------------------------------------
  Copyright (c) 2016 - 2023 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <getopt.h>
#include <poll.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <limits.h>
#include <inttypes.h>
#include "QMIThread.h"

#define mbim_debug dbg_time

#define UUID_BASIC_CONNECT "a289cc33-bcbb-8b4f-b6b0-133ec2aae6df"
//https://docs.microsoft.com/en-us/windows-hardware/drivers/network/mb-5g-data-class-support
#define UUID_BASIC_CONNECT_EXT "3d01dcc5-fef5-4d05-0d3a-bef7058e9aaf"
#define UUID_SMS             "533fbeeb-14fe-4467-9f90-33a223e56c3f"
#define UUID_USSD             "e550a0c8-5e82-479e-82f7-10abf4c3351f"
#define UUID_PHONEBOOK     "4bf38476-1e6a-41db-b1d8-bed289c25bdb"
#define UUID_STK             "d8f20131-fcb5-4e17-8602-d6ed3816164c"
#define UUID_AUTH             "1d2b5ff7-0aa1-48b2-aa52-50f15767174e"
#define UUID_DSS             "c08a26dd-7718-4382-8482-6e0d583c4d0e"
#define uuid_ext_qmux "d1a30bc2-f97a-6e43-bf65-c7e24fb0f0d3"
#define uuid_mshsd "883b7c26-985f-43fa-9804-27d7fb80959c"
#define uuid_qmbe "2d0c12c9-0e6a-495a-915c-8d174fe5d63c"
#define UUID_MSFWID "e9f7dea2-feaf-4009-93ce-90a3694103b6"
#define uuid_atds "5967bdcc-7fd2-49a2-9f5c-b2e70e527db3"
#define uuid_qdu "6427015f-579d-48f5-8c54-f43ed1e76f83"
#define UUID_MS_UICC_LOW_LEVEL "c2f6588e-f037-4bc9-8665-f4d44bd09367"
#define UUID_MS_SARControl "68223D04-9F6C-4E0F-822D-28441FB72340"
#define UUID_VOICEEXTENSIONS	"8d8b9eba-37be-449b-8f1e-61cb034a702e"
#define UUID_LIBMBIM_PROXY "838cf7fb-8d0d-4d7f-871e-d71dbefbb39b"

#define UUID_MBIMContextTypeInternet "7E5E2A7E-4E6F-7272-736B-656E7E5E2A7E"

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef unsigned long long UINT64;

#pragma pack(4)
typedef enum {
    MBIM_CID_CMD_TYPE_QUERY = 0,
    MBIM_CID_CMD_TYPE_SET = 1,
} MBIM_CID_CMD_TYPE_E;

typedef enum {
     MBIM_CID_DEVICE_CAPS = 1, 
     MBIM_CID_SUBSCRIBER_READY_STATUS = 2, 
     MBIM_CID_RADIO_STATE = 3, MBIM_CID_PIN = 4,
     MBIM_CID_PIN_LIS = 5,
     MBIM_CID_HOME_PROVIDER = 6,
     MBIM_CID_PREFERRED_PROVIDERS = 7,
     MBIM_CID_VISIBLE_PROVIDERS = 8,
     MBIM_CID_REGISTER_STATE = 9,
     MBIM_CID_PACKET_SERVICE = 10,
     MBIM_CID_SIGNAL_STATE = 11,
     MBIM_CID_CONNECT = 12,
     MBIM_CID_PROVISIONED_CONTEXTS = 13,
     MBIM_CID_SERVICE_ACTIVATION = 14,
     MBIM_CID_IP_CONFIGURATION =       15,
     MBIM_CID_DEVICE_SERVICES = 16,
     MBIM_CID_DEVICE_SERVICE_SUBSCRIBE_LIST = 19, 
     MBIM_CID_PACKET_STATISTICS = 20, 
     MBIM_CID_NETWORK_IDLE_HINT = 21,
     MBIM_CID_EMERGENCY_MODE = 22,
     MBIM_CID_IP_PACKET_FILTERS = 23, 
     MBIM_CID_MULTICARRIER_PROVIDERS = 24,
} UUID_BASIC_CONNECT_CID_E;

typedef enum{
    MBIM_CID_MS_PROVISIONED_CONTEXT_V2 = 1, 
    MBIM_CID_MS_NETWORK_BLACKLIST = 2,
    MBIM_CID_MS_LTE_ATTACH_CONFIG = 3, 
    MBIM_CID_MS_LTE_ATTACH_STATUS = 4, 
    MBIM_CID_MS_SYS_CAPS = 5, 
    MBIM_CID_MS_DEVICE_CAPS_V2 = 6, 
    MBIM_CID_MS_DEVICE_SLOT_MAPPING = 7, 
    MBIM_CID_MS_SLOT_INFO_STATUS = 8, 
    MBIM_CID_MS_PCO = 9, 
    MBIM_CID_MS_DEVICE_RESET = 10, 
    MBIM_CID_MS_BASE_STATIONS_INFO = 11, 
    MBIM_CID_MS_LOCATION_INFO_STATUS = 12, 
    MBIM_CID_NOT_DEFINED = 13, 
    MBIM_CID_MS_PIN_EX = 14, 
    MBIM_CID_MS_VERSION = 15,
} UUID_BASIC_CONNECT_EXT_CID_E;

typedef enum {
    MBIM_CID_SMS_CONFIGURATION = 1, // Y Y Y
    MBIM_CID_SMS_READ = 2, // N Y Y
    MBIM_CID_SMS_SEND = 3, // Y N N
    MBIM_CID_SMS_DELETE = 4, // Y N N
    MBIM_CID_SMS_MESSAGE_STORE_STATUS = 5, // N Y Y
} UUID_SMS_CID_E;

typedef enum {
    MBIM_CID_DSS_CONNECT = 1, // Y N N
} UUID_DSS_CID_E;

typedef enum{
    MBIM_OPEN_MSG = 1, 
    MBIM_CLOSE_MSG = 2, 
    MBIM_COMMAND_MSG = 3, 
    MBIM_HOST_ERROR_MSG = 4,
    MBIM_OPEN_DONE = 0x80000001, 
    MBIM_CLOSE_DONE = 0x80000002, 
    MBIM_COMMAND_DONE = 0x80000003, 
    MBIM_FUNCTION_ERROR_MSG = 0x80000004,       
    MBIM_INDICATE_STATUS_MSG = 0x80000007,
} MBIM_MSG_Type_E;
 
typedef enum { /*< since=1.10 >*/
    MBIM_CID_PROXY_CONTROL_UNKNOWN       = 0,
    MBIM_CID_PROXY_CONTROL_CONFIGURATION = 1
} UUID_LIBMBIM_PROXY_CID_E;

typedef enum {
    MBIM_CID_MS_UICC_ATR = 1,
    MBIM_CID_MS_UICC_OPEN_CHANNEL = 2,
    MBIM_CID_MS_UICC_CLOSE_CHANNEL  = 3,
    MBIM_CID_MS_UICC_APDU = 4,
    MBIM_CID_MS_UICC_TERMINAL_CAPABILITY = 5,
    MBIM_CID_MS_UICC_RESET = 6,
    MBIM_CID_MS_APP_LIST = 7,
} UUID_MS_UICC_CID_E;

typedef enum {
    MBIM_ERROR_TIMEOUT_FRAGMENT = 1,
    MBIM_ERROR_FRAGMENT_OUT_OF_SEQUENCE = 2,
    MBIM_ERROR_LENGTH_MISMATCH = 3,
    MBIM_ERROR_DUPLICATED_TID = 4,
    MBIM_ERROR_NOT_OPENED = 5,
    MBIM_ERROR_UNKNOWN = 6,
    MBIM_ERROR_CANCEL = 7,
    MBIM_ERROR_MAX_TRANSFER = 8,
} MBIM_ERROR_E;

typedef enum {
    MBIM_STATUS_SUCCESS = 0,
    MBIM_STATUS_BUSY =  1,
    MBIM_STATUS_FAILURE = 2,
    MBIM_STATUS_SIM_NOT_INSERTED = 3,
    MBIM_STATUS_BAD_SIM = 4,
    MBIM_STATUS_PIN_REQUIRED = 5,
    MBIM_STATUS_PIN_DISABLED = 6,
    MBIM_STATUS_NOT_REGISTERED = 7,
    MBIM_STATUS_PROVIDERS_NOT_FOUND = 8,
    MBIM_STATUS_NO_DEVICE_SUPPORT = 9,
    MBIM_STATUS_PROVIDER_NOT_VISIBLE = 10,
    MBIM_STATUS_DATA_CLASS_NOT_AVAILABL = 11,
    MBIM_STATUS_PACKET_SERVICE_DETACHED = 12,
}  MBIM_STATUS_CODES_E;

typedef enum {
    MBIMPacketServiceActionAttach = 0,
    MBIMPacketServiceActionDetach = 1,
} MBIM_PACKET_SERVICE_ACTION_E;

typedef enum {
    MBIMPacketServiceStateUnknown = 0,
    MBIMPacketServiceStateAttaching = 1,
    MBIMPacketServiceStateAttached = 2,
    MBIMPacketServiceStateDetaching = 3,
    MBIMPacketServiceStateDetached = 4,
} MBIM_PACKET_SERVICE_STATE_E;

static const char *MBIMPacketServiceStateStr(int _val) {
    struct { int val;char *name;} _enumstr[] = {
        {MBIMPacketServiceStateUnknown, "Unknown"},
        {MBIMPacketServiceStateAttaching, "Attaching"},
        {MBIMPacketServiceStateAttached, "Attached"},
        {MBIMPacketServiceStateDetaching, "Detaching"},
        {MBIMPacketServiceStateDetached, "Detached"},
    };
    int idx;

    for (idx = 0; idx < (int)(sizeof(_enumstr)/sizeof(_enumstr[0])); idx++) {
        if (_val == _enumstr[idx].val)
            return _enumstr[idx].name;
    }

    return "Undefined";
};

typedef enum {
    MBIMDataClassNone = 0x0,
    MBIMDataClassGPRS = 0x1,
    MBIMDataClassEDGE = 0x2,
    MBIMDataClassUMTS = 0x4,
    MBIMDataClassHSDPA = 0x8,
    MBIMDataClassHSUPA = 0x10,
    MBIMDataClassLTE = 0x20,
    MBIMDataClass5G_NSA = 0x40,
    MBIMDataClass5G_SA = 0x80,
    MBIMDataClass1XRTT = 0x10000,
    MBIMDataClass1XEVDO = 0x20000,
    MBIMDataClass1XEVDORevA = 0x40000,
    MBIMDataClass1XEVDV = 0x80000,
    MBIMDataClass3XRTT = 0x100000,
    MBIMDataClass1XEVDORevB = 0x200000,
    MBIMDataClassUMB = 0x400000,
    MBIMDataClassCustom = 0x80000000,
} MBIM_DATA_CLASS_E;

static const char *MBIMDataClassStr(int _val) {
    struct { int val;char *name;} _enumstr[] = {
        {MBIMDataClassNone, "None"},
        {MBIMDataClassGPRS, "GPRS"},
        {MBIMDataClassEDGE, "EDGE"},
        {MBIMDataClassUMTS, "UMTS"},
        {MBIMDataClassHSDPA, "HSDPA"},
        {MBIMDataClassHSUPA, "HSUPA"},
        {MBIMDataClassLTE, "LTE"},
        {MBIMDataClass5G_NSA, "5G_NSA"},
        {MBIMDataClass5G_SA, "5G_SA"},
        {MBIMDataClass1XRTT, "1XRTT"},
        {MBIMDataClass1XEVDO, "1XEVDO"},
        {MBIMDataClass1XEVDORevA, "1XEVDORevA"},
        {MBIMDataClass1XEVDV, "1XEVDV"},
        {MBIMDataClass3XRTT, "3XRTT"},
        {MBIMDataClass1XEVDORevB, "1XEVDORevB"},
        {MBIMDataClassUMB, "UMB"},
        {MBIMDataClassCustom, "Custom"},
    };
    int idx;

    for (idx = 0; idx < (int)(sizeof(_enumstr)/sizeof(_enumstr[0])); idx++) {
        if (_val == _enumstr[idx].val)
            return _enumstr[idx].name;
    }

    return "Unknow";
};

typedef struct {
    UINT32 NwError;
    UINT32 PacketServiceState; //MBIM_PACKET_SERVICE_STATE_E
    UINT32 HighestAvailableDataClass; //MBIM_DATA_CLASS_E
    UINT64 UplinkSpeed;
    UINT64 DownlinkSpeed;
} MBIM_PACKET_SERVICE_INFO_T;

typedef struct {
    UINT32 NwError;
    UINT32 PacketServiceState; //MBIM_PACKET_SERVICE_STATE_E
    UINT32 CurrentDataClass; //MBIM_DATA_CLASS_E
    UINT64 UplinkSpeed;
    UINT64 DownlinkSpeed;
    UINT32 FrequencyRange;
} MBIM_PACKET_SERVICE_INFO_V2_T;

typedef enum {
    MBIMSubscriberReadyStateNotInitialized = 0,
    MBIMSubscriberReadyStateInitialized = 1,
    MBIMSubscriberReadyStateSimNotInserted = 2,
    MBIMSubscriberReadyStateBadSim = 3,
    MBIMSubscriberReadyStateFailure = 4,
    MBIMSubscriberReadyStateNotActivated = 5,
    MBIMSubscriberReadyStateDeviceLocked = 6,
}MBIM_SUBSCRIBER_READY_STATE_E;

static const char *MBIMSubscriberReadyStateStr(int _val) {
    struct { int val;char *name;} _enumstr[] = {
        {MBIMSubscriberReadyStateNotInitialized, "NotInitialized"},
        {MBIMSubscriberReadyStateInitialized, "Initialized"},
        {MBIMSubscriberReadyStateSimNotInserted, "NotInserted"},
        {MBIMSubscriberReadyStateBadSim, "BadSim"},
        {MBIMSubscriberReadyStateFailure, "Failure"},
        {MBIMSubscriberReadyStateNotActivated, "NotActivated"},
        {MBIMSubscriberReadyStateDeviceLocked, "DeviceLocked"},
    };
    int idx;

    for (idx = 0; idx < (int)(sizeof(_enumstr)/sizeof(_enumstr[0])); idx++) {
        if (_val == _enumstr[idx].val)
            return _enumstr[idx].name;
    }

    return "Undefined";
};

typedef struct {
    UINT32 DeviceType; //MBIM_DEVICE_TYPE
    UINT32 CellularClass; //MBIM_CELLULAR_CLASS
    UINT32 VoiceClass; //MBIM_VOICE_CLASS
    UINT32 SimClass; //MBIM_SIM_CLASS
    UINT32 DataClass; //MBIM_DATA_CLASS
    UINT32 SmsCaps; //MBIM_SMS_CAPS
    UINT32 ControlCaps; //MBIM_CTRL_CAPS
    UINT32 MaxSessions;
    UINT32 CustomDataClassOffset;
    UINT32 CustomDataClassSize;
    UINT32 DeviceIdOffset;
    UINT32 DeviceIdSize;
    UINT32 FirmwareInfoOffset;
    UINT32 FirmwareInfoSize;
    UINT32 HardwareInfoOffset;
    UINT32 HardwareInfoSize;
    UINT8 DataBuffer[0]; //DeviceId FirmwareInfo HardwareInfo
} MBIM_DEVICE_CAPS_INFO_T;

typedef enum {
    MBIMRadioOff = 0,
    MBIMRadioOn = 1,
} MBIM_RADIO_SWITCH_STATE_E;

typedef struct {
    MBIM_RADIO_SWITCH_STATE_E RadioState;
} MBIM_SET_RADIO_STATE_T;

typedef struct {
    MBIM_RADIO_SWITCH_STATE_E HwRadioState;
    MBIM_RADIO_SWITCH_STATE_E SwRadioState;
} MBIM_RADIO_STATE_INFO_T;

typedef enum {
    MBIMReadyInfoFlagsNone,
    MBIMReadyInfoFlagsProtectUniqueID,
}MBIM_UNIQUE_ID_FLAGS;

typedef struct {
    UINT32 ReadyState;
    UINT32 SubscriberIdOffset;
    UINT32 SubscriberIdSize;
    UINT32 SimIccIdOffset;
    UINT32 SimIccIdSize;
    UINT32 ReadyInfo;
    UINT32 ElementCount;
    UINT8 *TelephoneNumbersRefList;
    UINT8 *DataBuffer;
} MBIM_SUBSCRIBER_READY_STATUS_T;

typedef enum {
    MBIMRegisterActionAutomatic,
    MBIMRegisterActionManual,
}MBIM_REGISTER_ACTION_E;

typedef enum {
    MBIMRegisterStateUnknown = 0,
    MBIMRegisterStateDeregistered = 1,
    MBIMRegisterStateSearching = 2,
    MBIMRegisterStateHome = 3,
    MBIMRegisterStateRoaming = 4,
    MBIMRegisterStatePartner = 5,
    MBIMRegisterStateDenied = 6,
}MBIM_REGISTER_STATE_E;

typedef enum {
    MBIMRegisterModeUnknown = 0,
    MBIMRegisterModeAutomatic = 1,
    MBIMRegisterModeManual = 2,
}MBIM_REGISTER_MODE_E;

static const char *MBIMRegisterStateStr(int _val) {
    struct { int val;char *name;} _enumstr[] ={
        {MBIMRegisterStateUnknown, "Unknown"},
        {MBIMRegisterStateDeregistered, "Deregistered"},
        {MBIMRegisterStateSearching, "Searching"},
        {MBIMRegisterStateHome, "Home"},
        {MBIMRegisterStateRoaming, "Roaming"},
        {MBIMRegisterStatePartner, "Partner"},
        {MBIMRegisterStateDenied, "Denied"},
    };
    int idx;

    for (idx = 0; idx < (int)(sizeof(_enumstr)/sizeof(_enumstr[0])); idx++) {
        if (_val == _enumstr[idx].val)
            return _enumstr[idx].name;
    }

    return "Undefined";
};

static const char *MBIMRegisterModeStr(int _val) {
    struct { int val;char *name;} _enumstr[] = {
        {MBIMRegisterModeUnknown, "Unknown"},
        {MBIMRegisterModeAutomatic, "Automatic"},
        {MBIMRegisterModeManual, "Manual"},
    };
    int idx;

    for (idx = 0; idx < (int)(sizeof(_enumstr)/sizeof(_enumstr[0])); idx++) {
        if (_val == _enumstr[idx].val)
            return _enumstr[idx].name;
    }

    return "Undefined";
};

typedef enum {
    MBIM_REGISTRATION_NONE,
    MBIM_REGISTRATION_MANUAL_SELECTION_NOT_AVAILABLE,
    MBIM_REGISTRATION_PACKET_SERVICE_AUTOMATIC_ATTACH,
}MBIM_REGISTRATION_FLAGS_E;

typedef struct {
    UINT32 NwError;
    UINT32 RegisterState; //MBIM_REGISTER_STATE_E
    UINT32 RegisterMode;
    UINT32 AvailableDataClasses;
    UINT32 CurrentCellularClass;
    UINT32 ProviderIdOffset;
    UINT32 ProviderIdSize;
    UINT32 ProviderNameOffset;
    UINT32 ProviderNameSize;
    UINT32 RoamingTextOffset;
    UINT32 RoamingTextSize;
    UINT32 RegistrationFlag;
    UINT8 *DataBuffer;
} MBIM_REGISTRATION_STATE_INFO_T;

typedef struct {
    UINT32 NwError;
    UINT32 RegisterState; //MBIM_REGISTER_STATE_E
    UINT32 RegisterMode;
    UINT32 AvailableDataClasses;
    UINT32 CurrentCellularClass;
    UINT32 ProviderIdOffset;
    UINT32 ProviderIdSize;
    UINT32 ProviderNameOffset;
    UINT32 ProviderNameSize;
    UINT32 RoamingTextOffset;
    UINT32 RoamingTextSize;
    UINT32 RegistrationFlag;
    UINT32 PreferredDataClass;
    UINT8 *DataBuffer;
} MBIM_REGISTRATION_STATE_INFO_V2_T;

typedef struct {
    UINT32 MessageType; //Specifies the MBIM message type.
    UINT32 MessageLength; //Specifies the total length of this MBIM message in bytes.
    /* Specifies the MBIM message id value.  This value is used to match host sent messages with function responses.
    This value must be unique among all outstanding transactions.
    For notifications, the TransactionId must be set to 0 by the function */
    UINT32 TransactionId;
} MBIM_MESSAGE_HEADER;

typedef struct {
    UINT32 TotalFragments; //this field indicates how many fragments there are intotal.
    UINT32 CurrentFragment; //This field indicates which fragment this message is.  Values are 0 to TotalFragments?\1
} MBIM_FRAGMENT_HEADER;

typedef struct {
    MBIM_MESSAGE_HEADER MessageHeader;
    UINT32 MaxControlTransfer;
} MBIM_OPEN_MSG_T;

typedef struct {
    MBIM_MESSAGE_HEADER MessageHeader;
    UINT32 Status; //MBIM_STATUS_CODES_E
} MBIM_OPEN_DONE_T;

typedef struct {
    MBIM_MESSAGE_HEADER MessageHeader;
} MBIM_CLOSE_MSG_T;

typedef struct {
    MBIM_MESSAGE_HEADER MessageHeader;
    UINT32 Status;
} MBIM_CLOSE_DONE_T;

typedef struct {
    UINT8 uuid[16];
} UUID_T;

typedef struct {
    MBIM_MESSAGE_HEADER MessageHeader;
    MBIM_FRAGMENT_HEADER FragmentHeader;
    UUID_T DeviceServiceId; //A 16 byte UUID that identifies the device service the following CID value applies.
    UINT32 CID; //Specifies the CID that identifies the parameter being queried for
    UINT32 CommandType; //0 for a query operation, 1 for a Set operation
    UINT32 InformationBufferLength; //Size of the Total InformationBuffer, may be larger than current message if fragmented.
    UINT8 InformationBuffer[0]; //Data supplied to device specific to the CID
} MBIM_COMMAND_MSG_T;

typedef struct {
    MBIM_MESSAGE_HEADER MessageHeader;
    MBIM_FRAGMENT_HEADER FragmentHeader;
    UUID_T DeviceServiceId; //A 16 byte UUID that identifies the device service the following CID value applies.
    UINT32 CID; //Specifies the CID that identifies the parameter being queried for
    UINT32 Status;
    UINT32 InformationBufferLength; //Size of the Total InformationBuffer, may be larger than current message if fragmented.
    UINT8 InformationBuffer[0]; //Data supplied to device specific to the CID
} MBIM_COMMAND_DONE_T;

typedef struct {
    MBIM_MESSAGE_HEADER MessageHeader;
    UINT32 ErrorStatusCode;
} MBIM_HOST_ERROR_MSG_T;

typedef struct {
    MBIM_MESSAGE_HEADER MessageHeader;
    UINT32 ErrorStatusCode;
} MBIM_FUNCTION_ERROR_MSG_T;

typedef struct {
    MBIM_MESSAGE_HEADER MessageHeader;
    MBIM_FRAGMENT_HEADER FragmentHeader;
    UUID_T DeviceServiceId; //A 16 byte UUID that identifies the device service the following CID value applies.
    UINT32 CID; //Specifies the CID that identifies the parameter being queried for
    UINT32 InformationBufferLength; //Size of the Total InformationBuffer, may be larger than current message if fragmented.
    UINT8 InformationBuffer[0]; //Data supplied to device specific to the CID
} MBIM_INDICATE_STATUS_MSG_T;

typedef struct {
    UINT32 offset;
    UINT32 size;
} OL_PAIR_LIST;

typedef struct {
    UUID_T DeviceServiceId;
    UINT32 DssPayload;
    UINT32 MaxDssInstances;
    UINT32 CidCount;
    UINT32 CidList[];
} MBIM_DEVICE_SERVICE_ELEMENT_T;

typedef struct {
    UINT32 DeviceServicesCount;
    UINT32 MaxDssSessions;
    OL_PAIR_LIST DeviceServicesRefList[];
} MBIM_DEVICE_SERVICES_INFO_T;

typedef enum {
    MBIMActivationCommandDeactivate = 0,
    MBIMActivationCommandActivate = 1,
} MBIM_ACTIVATION_COMMAND_E;

typedef enum {
    MBIMCompressionNone =  0,
    MBIMCompressionEnable =  1,
} MBIM_COMPRESSION_E;

typedef enum {
    MBIMAuthProtocolNone = 0,
    MBIMAuthProtocolPap = 1,
    MBIMAuthProtocolChap = 2,
    MBIMAuthProtocolMsChapV2 = 3,
} MBIM_AUTH_PROTOCOL_E;

typedef enum {
    MBIMContextIPTypeDefault = 0, 
    MBIMContextIPTypeIPv4 = 1, 
    MBIMContextIPTypeIPv6 = 2, 
    MBIMContextIPTypeIPv4v6 = 3, 
    MBIMContextIPTypeIPv4AndIPv6 = 4,
} MBIM_CONTEXT_IP_TYPE_E;

typedef enum {
    MBIMActivationStateUnknown = 0,
    MBIMActivationStateActivated = 1,
    MBIMActivationStateActivating = 2,
    MBIMActivationStateDeactivated = 3,
    MBIMActivationStateDeactivating = 4,
} MBIM_ACTIVATION_STATE_E;

typedef enum {
    MBIMVoiceCallStateNone = 0,
    MBIMVoiceCallStateInProgress = 1,
    MBIMVoiceCallStateHangUp = 2,
} MBIM_VOICECALL_STATE_E;

static const char *MBIMMSGTypeStr(int _val) {
    struct { int val;char *name;} _enumstr[] = { 
        {MBIM_OPEN_MSG, "MBIM_OPEN_MSG"}, 
        {MBIM_CLOSE_MSG, "MBIM_CLOSE_MSG"}, 
        {MBIM_COMMAND_MSG, "MBIM_COMMAND_MSG"}, 
        {MBIM_HOST_ERROR_MSG, "MBIM_HOST_ERROR_MSG"}, 
        {MBIM_OPEN_DONE, "MBIM_OPEN_DONE"}, 
        {MBIM_CLOSE_DONE, "MBIM_CLOSE_DONE"}, 
        {MBIM_COMMAND_DONE, "MBIM_COMMAND_DONE"}, 
        {MBIM_FUNCTION_ERROR_MSG, "MBIM_FUNCTION_ERROR_MSG"}, 
        {MBIM_INDICATE_STATUS_MSG, "MBIM_INDICATE_STATUS_MSG"}, 
    }; 
    int idx;

    for (idx = 0; idx < (int)(sizeof(_enumstr)/sizeof(_enumstr[0])); idx++) { 
        if (_val == _enumstr[idx].val) 
            return _enumstr[idx].name;
    } 

    return "MBIMMSGTypeUnknow"; 
};

static const char *MBIMContextIPTypeStr(int _val) { 
    struct { int val;char *name;} _enumstr[] = { 
        {MBIMContextIPTypeDefault, "MBIMContextIPTypeDefault"}, 
        {MBIMContextIPTypeIPv4, "MBIMContextIPTypeIPv4"}, 
        {MBIMContextIPTypeIPv6, "MBIMContextIPTypeIPv6"}, 
        {MBIMContextIPTypeIPv4v6, "MBIMContextIPTypeIPv4v6"}, 
        {MBIMContextIPTypeIPv4AndIPv6, "MBIMContextIPTypeIPv4AndIPv6"}, 
    }; 
    int idx; 

    for (idx = 0; idx < (int)(sizeof(_enumstr)/sizeof(_enumstr[0])); idx++) {
        if (_val == _enumstr[idx].val)
            return _enumstr[idx].name;
    } 

    return "MBIMContextIPTypeUnknow"; 
}

static const char *MBIMActivationStateStr(int _val) {
    struct { int val;char *name;} _enumstr[] = {
        {MBIMActivationStateUnknown, "Unknown"},
        {MBIMActivationStateActivated, "Activated"},
        {MBIMActivationStateActivating, "Activating"},
        {MBIMActivationStateDeactivated, "Deactivated"},
        {MBIMActivationStateDeactivating, "Deactivating"},
    };
    int idx;

    for (idx = 0; idx < (int)(sizeof(_enumstr)/sizeof(_enumstr[0])); idx++) {
        if (_val == _enumstr[idx].val)
            return _enumstr[idx].name;
    }

    return "Undefined";
};

static const char *MBIMVoiceCallStateStr(int _val) {
    struct { int val;char *name;} _enumstr[] = {
        {MBIMVoiceCallStateNone, "None"},
        {MBIMVoiceCallStateInProgress, "InProgress"},
        {MBIMVoiceCallStateHangUp, "HangUp"},
    };
    int idx;

    for (idx = 0; idx < (int)(sizeof(_enumstr)/sizeof(_enumstr[0])); idx++) {
        if (_val == _enumstr[idx].val)
        return _enumstr[idx].name;
    }

    return "Undefined";
};

typedef struct {
    const char *uuid;
    UINT32 cid;
    const char *name;
} UUID_CID_STR;

static const  UUID_CID_STR uuid_cid_string[] = { 
    {UUID_BASIC_CONNECT, MBIM_CID_DEVICE_CAPS, "MBIM_CID_DEVICE_CAPS"}, 
    {UUID_BASIC_CONNECT, MBIM_CID_SUBSCRIBER_READY_STATUS, "MBIM_CID_SUBSCRIBER_READY_STATUS"}, 
    {UUID_BASIC_CONNECT, MBIM_CID_RADIO_STATE, "MBIM_CID_RADIO_STATE"},
    {UUID_BASIC_CONNECT, MBIM_CID_PIN, "MBIM_CID_PIN"}, 
    {UUID_BASIC_CONNECT, MBIM_CID_PIN_LIS, "MBIM_CID_PIN_LIS"}, 
    {UUID_BASIC_CONNECT, MBIM_CID_HOME_PROVIDER, "MBIM_CID_HOME_PROVIDER"}, 
    {UUID_BASIC_CONNECT, MBIM_CID_PREFERRED_PROVIDERS, "MBIM_CID_PREFERRED_PROVIDERS"}, 
    {UUID_BASIC_CONNECT, MBIM_CID_VISIBLE_PROVIDERS, "MBIM_CID_VISIBLE_PROVIDERS"}, 
    {UUID_BASIC_CONNECT, MBIM_CID_REGISTER_STATE, "MBIM_CID_REGISTER_STATE"}, 
    {UUID_BASIC_CONNECT, MBIM_CID_PACKET_SERVICE, "MBIM_CID_PACKET_SERVICE"}, 
    {UUID_BASIC_CONNECT, MBIM_CID_SIGNAL_STATE, "MBIM_CID_SIGNAL_STATE"}, 
    {UUID_BASIC_CONNECT, MBIM_CID_CONNECT, "MBIM_CID_CONNECT"}, 
    {UUID_BASIC_CONNECT, MBIM_CID_PROVISIONED_CONTEXTS, "MBIM_CID_PROVISIONED_CONTEXTS"}, 
    {UUID_BASIC_CONNECT, MBIM_CID_SERVICE_ACTIVATION, "MBIM_CID_SERVICE_ACTIVATION"}, 
    {UUID_BASIC_CONNECT, MBIM_CID_IP_CONFIGURATION, "MBIM_CID_IP_CONFIGURATION"}, 
    {UUID_BASIC_CONNECT, MBIM_CID_DEVICE_SERVICES, "MBIM_CID_DEVICE_SERVICES"}, 
    {UUID_BASIC_CONNECT, MBIM_CID_DEVICE_SERVICE_SUBSCRIBE_LIST, "MBIM_CID_DEVICE_SERVICE_SUBSCRIBE_LIST"}, 
    {UUID_BASIC_CONNECT, MBIM_CID_PACKET_STATISTICS, "MBIM_CID_PACKET_STATISTICS"}, 
    {UUID_BASIC_CONNECT, MBIM_CID_NETWORK_IDLE_HINT, "MBIM_CID_NETWORK_IDLE_HINT"},
    {UUID_BASIC_CONNECT, MBIM_CID_EMERGENCY_MODE, "MBIM_CID_EMERGENCY_MODE"}, 
    {UUID_BASIC_CONNECT, MBIM_CID_IP_PACKET_FILTERS, "MBIM_CID_IP_PACKET_FILTERS"}, 
    {UUID_BASIC_CONNECT, MBIM_CID_MULTICARRIER_PROVIDERS, "MBIM_CID_MULTICARRIER_PROVIDERS"},

    {UUID_BASIC_CONNECT_EXT, MBIM_CID_MS_PROVISIONED_CONTEXT_V2, "MBIM_CID_MS_PROVISIONED_CONTEXT_V2"}, 
    {UUID_BASIC_CONNECT_EXT, MBIM_CID_MS_NETWORK_BLACKLIST, "MBIM_CID_MS_NETWORK_BLACKLIST"}, 
    {UUID_BASIC_CONNECT_EXT, MBIM_CID_MS_LTE_ATTACH_CONFIG, "MBIM_CID_MS_LTE_ATTACH_CONFIG"}, 
    {UUID_BASIC_CONNECT_EXT, MBIM_CID_MS_LTE_ATTACH_STATUS, "MBIM_CID_MS_LTE_ATTACH_STATUS"}, 
    {UUID_BASIC_CONNECT_EXT, MBIM_CID_MS_SYS_CAPS, "MBIM_CID_MS_SYS_CAPS"},
    {UUID_BASIC_CONNECT_EXT, MBIM_CID_MS_DEVICE_CAPS_V2, "MBIM_CID_MS_DEVICE_CAPS_V2"}, 
    {UUID_BASIC_CONNECT_EXT, MBIM_CID_MS_DEVICE_SLOT_MAPPING, "MBIM_CID_MS_DEVICE_SLOT_MAPPING"}, 
    {UUID_BASIC_CONNECT_EXT, MBIM_CID_MS_SLOT_INFO_STATUS, "MBIM_CID_MS_SLOT_INFO_STATUS"}, 
    {UUID_BASIC_CONNECT_EXT, MBIM_CID_MS_PCO, "MBIM_CID_MS_PCO"}, 
    {UUID_BASIC_CONNECT_EXT, MBIM_CID_MS_DEVICE_RESET, "MBIM_CID_MS_DEVICE_RESET"}, 
    {UUID_BASIC_CONNECT_EXT, MBIM_CID_MS_BASE_STATIONS_INFO, "MBIM_CID_MS_BASE_STATIONS_INFO"}, 
    {UUID_BASIC_CONNECT_EXT, MBIM_CID_MS_LOCATION_INFO_STATUS, "MBIM_CID_MS_LOCATION_INFO_STATUS"}, 
    {UUID_BASIC_CONNECT_EXT, MBIM_CID_NOT_DEFINED, "MBIM_CID_NOT_DEFINED"}, 
    {UUID_BASIC_CONNECT_EXT, MBIM_CID_MS_PIN_EX, "MBIM_CID_MS_PIN_EX"}, 
    {UUID_BASIC_CONNECT_EXT, MBIM_CID_MS_VERSION, "MBIM_CID_MS_VERSION"}, 

    {UUID_MS_UICC_LOW_LEVEL, MBIM_CID_MS_UICC_ATR, "MBIM_CID_MS_UICC_ATR"}, 
    {UUID_MS_UICC_LOW_LEVEL, MBIM_CID_MS_UICC_OPEN_CHANNEL, "MBIM_CID_MS_UICC_OPEN_CHANNEL"}, 
    {UUID_MS_UICC_LOW_LEVEL, MBIM_CID_MS_UICC_CLOSE_CHANNEL, "MBIM_CID_MS_UICC_CLOSE_CHANNEL"}, 
    {UUID_MS_UICC_LOW_LEVEL, MBIM_CID_MS_UICC_APDU, "MBIM_CID_MS_UICC_APDU"}, 
    {UUID_MS_UICC_LOW_LEVEL, MBIM_CID_MS_UICC_TERMINAL_CAPABILITY, "MBIM_CID_MS_UICC_TERMINAL_CAPABILITY"}, 
    {UUID_MS_UICC_LOW_LEVEL, MBIM_CID_MS_UICC_RESET, "MBIM_CID_MS_UICC_RESET"}, 
    {UUID_MS_UICC_LOW_LEVEL, MBIM_CID_MS_APP_LIST, "MBIM_CID_MS_APP_LIST"}, 
}; 

typedef struct {
    UINT32 SessionId;
    UINT32 ActivationCommand; //MBIM_ACTIVATION_COMMAND_E
    UINT32 AccessStringOffset;
    UINT32 AccessStringSize;
    UINT32 UserNameOffset;
    UINT32 UserNameSize;
    UINT32 PasswordOffset;
    UINT32 PasswordSize;
    UINT32 Compression; //MBIM_COMPRESSION_E
    UINT32 AuthProtocol; //MBIM_AUTH_PROTOCOL_E
    UINT32 IPType; //MBIM_CONTEXT_IP_TYPE_E
    UUID_T ContextType;
    UINT8 DataBuffer[0];  /* apn, username, password */
} MBIM_SET_CONNECT_T;

typedef struct {
    UINT32 SessionId;
    UINT32 ActivationState; //MBIM_ACTIVATION_STATE_E
    UINT32 VoiceCallState;
    UINT32 IPType; //MBIM_CONTEXT_IP_TYPE_E
    UUID_T ContextType;
    UINT32 NwError;
} MBIM_CONNECT_T;

typedef struct {
    UINT32 OnLinkPrefixLength;
    UINT8 IPv4Address[4];
} MBIM_IPV4_ELEMENT_T;

typedef struct {
    UINT32 OnLinkPrefixLength;
    UINT8 IPv6Address[16];
} MBIM_IPV6_ELEMENT_T;

typedef struct {
    UINT32 SessionId;
    UINT32 IPv4ConfigurationAvailable; //bit0~Address, bit1~gateway, bit2~DNS, bit3~MTU
    UINT32 IPv6ConfigurationAvailable; //bit0~Address, bit1~gateway, bit2~DNS, bit3~MTU
    UINT32 IPv4AddressCount;
    UINT32 IPv4AddressOffset;
    UINT32 IPv6AddressCount;
    UINT32 IPv6AddressOffset;
    UINT32 IPv4GatewayOffset;
    UINT32 IPv6GatewayOffset;
    UINT32 IPv4DnsServerCount;
    UINT32 IPv4DnsServerOffset;
    UINT32 IPv6DnsServerCount;
    UINT32 IPv6DnsServerOffset;
    UINT32 IPv4Mtu;
    UINT32 IPv6Mtu;
    UINT8 DataBuffer[];
} MBIM_IP_CONFIGURATION_INFO_T;

typedef struct {
    UINT32 RSRP;
    UINT32 SNR;
    UINT32 RSRPThreshold;
    UINT32 SNRThreshold;
    UINT32 SystemType;
} MBIM_RSRP_SNR_INFO_T;

typedef struct {
    UINT32 Elementcount;
    MBIM_RSRP_SNR_INFO_T RsrpSnr[0];
} MBIM_RSRP_SNR_T;

typedef struct {
    UINT32 Rssi;
    UINT32 ErrorRate;
    UINT32 SignalStrengthInterval;
    UINT32 RssiThreshold;
    UINT32 ErrorRateThreshold;
} MBIM_SIGNAL_STATE_INFO_T;

typedef struct {
    UINT32 Rssi;
    UINT32 ErrorRate;
    UINT32 SignalStrengthInterval;
    UINT32 RssiThreshold;
    UINT32 ErrorRateThreshold;
    UINT32 RsrpSnrOffset;
    UINT32 RsrpSnrSize;
    UINT8 DataBuffer[];
} MBIM_SIGNAL_STATE_INFO_V2_T;

typedef struct {
    UINT32 SignalStrengthInterval;
    UINT32 RssiThreshold;
    UINT32 ErrorRateThreshold;
} MBIM_SET_SIGNAL_STATE_T;

typedef struct {
    UINT32 DevicePathOffset;
    UINT32 DevicePathSize;
    UINT32 Timeout;
    UINT8 DataBuffer[];
} MBIM_LIBQMI_PROXY_CONFIG_T;

typedef struct {
    UINT32 AtrSize;
    UINT32 AtrOffset;
    UINT8 DataBuffer[];
} MBIM_MS_ATR_INFO_T;

#pragma pack()

static pthread_t s_tid_reader = 0;
static int mbim_verbose = 0;
static UINT32 TransactionId = 1;
static unsigned mbim_default_timeout  = 30000;
static const char *mbim_apn = NULL;
static const char *mbim_user = NULL;
static const char *mbim_passwd = NULL;
static int mbim_iptype = MBIMContextIPTypeDefault;
static int mbim_auth = MBIMAuthProtocolNone;
static int mbim_sessionID = 0;
static int mbim_fd = -1;
static MBIM_MESSAGE_HEADER *mbim_pRequest;
static MBIM_MESSAGE_HEADER *mbim_pResponse;

static unsigned int qmi_over_mbim_support = 0;
static int qmi_over_mbim_sk[2] = {-1, -1};
static pthread_mutex_t mbim_command_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t mbim_command_cond = PTHREAD_COND_INITIALIZER;
static int mbim_ms_version = 1;
static uint8_t qmi_over_mbim_nas = 0;
int qmi_over_mbim_qmidev_send(PQCQMIMSG pQMI);

static const UUID_T * str2uuid(const char *str) {
    static UUID_T uuid;
    UINT32 d[16];
    char tmp[16*2+4+1];
    unsigned i = 0;

    while (str[i]) {
        tmp[i] = tolower(str[i]);
        i++;
    }
    tmp[i] = '\0';

    sscanf(tmp, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        &d[0], &d[1], &d[2], &d[3], &d[4], &d[5], &d[6], &d[7],
        &d[8], &d[9], &d[10], &d[11], &d[12], &d[13], &d[14], &d[15]);

    for (i = 0; i < 16; i++) {
        uuid.uuid[i] = d[i]&0xFF;
    }

    return &uuid;
}

static void wchar2char(const char *src, size_t src_size, char *dst, size_t dst_len) {
    size_t i;
    
    for (i = 0; i < (dst_len-1) && i < (src_size/2); i++)
        dst[i] = src[i*2];
    dst[i] = 0;
}

static size_t char2wchar(const char *src, size_t src_len, uint8_t *dst, size_t dst_len) {
    size_t i;

    if (src_len > (dst_len/2))
        src_len = (dst_len/2);

    for (i = 0; i < src_len; i++) {
        *dst++ = *src++;
        *dst++ = 0;
    }

    return i*2;
}

#define mbim_alloc( _size)  malloc(_size)
#define mbim_free(_mem) do { if (_mem) { free(_mem); _mem = NULL;}} while(0)

static int mbim_open_state = 0;
static MBIM_SUBSCRIBER_READY_STATE_E ReadyState = MBIMSubscriberReadyStateNotInitialized;
static MBIM_REGISTER_STATE_E RegisterState = MBIMRegisterStateUnknown;
static MBIM_PACKET_SERVICE_STATE_E PacketServiceState = MBIMPacketServiceStateUnknown;
static MBIM_ACTIVATION_STATE_E ActivationState = MBIMActivationStateUnknown;
static MBIM_SUBSCRIBER_READY_STATE_E oldReadyState = MBIMSubscriberReadyStateNotInitialized;
static MBIM_REGISTER_STATE_E oldRegisterState = MBIMRegisterStateUnknown;
static MBIM_PACKET_SERVICE_STATE_E oldPacketServiceState = MBIMPacketServiceStateUnknown;
static MBIM_ACTIVATION_STATE_E oldActivationState = MBIMActivationStateUnknown;
static int mbim_update_state(void);

static __inline uint32_t mbim2qmi_ipv4addr(uint32_t addr) {
    return (addr>>24) | (addr>>8&0xff00) | (addr<<8&0xff0000) | (addr<<24);
}

static __inline void mbim2qmi_ipv6addr(const unsigned char *src, unsigned char *dst) {
    int i;

    for (i = 0; i < 16 ; i++) {
        dst[i] = src[i];
    }
}

static MBIM_MESSAGE_HEADER *compose_open_command(UINT32 MaxControlTransfer)
{
    MBIM_OPEN_MSG_T *pRequest = (MBIM_OPEN_MSG_T *)mbim_alloc(sizeof(MBIM_OPEN_MSG_T));

    if(!pRequest)
        return NULL;

    pRequest->MessageHeader.MessageType = htole32(MBIM_OPEN_MSG);
    pRequest->MessageHeader.MessageLength = htole32(sizeof(MBIM_OPEN_MSG_T));
    pRequest->MessageHeader.TransactionId = htole32(TransactionId++);
    pRequest->MaxControlTransfer = htole32(MaxControlTransfer);

    return &pRequest->MessageHeader;
}

static MBIM_MESSAGE_HEADER *compose_close_command(void)
{
    MBIM_CLOSE_MSG_T *pRequest = (MBIM_CLOSE_MSG_T *)mbim_alloc(sizeof(MBIM_CLOSE_MSG_T));

    if(!pRequest)
        return NULL;

    pRequest->MessageHeader.MessageType = htole32(MBIM_CLOSE_MSG);
    pRequest->MessageHeader.MessageLength = htole32(sizeof(MBIM_CLOSE_MSG_T));
    pRequest->MessageHeader.TransactionId = htole32(TransactionId++);

    return &pRequest->MessageHeader;
}

static MBIM_MESSAGE_HEADER *compose_basic_connect_command(UINT32 CID, UINT32 CommandType, void *pInformationBuffer, UINT32 InformationBufferLength)
{
    MBIM_COMMAND_MSG_T *pRequest = (MBIM_COMMAND_MSG_T *)mbim_alloc(sizeof(MBIM_COMMAND_MSG_T) + InformationBufferLength);

    if (!pRequest)
        return NULL;

    pRequest->MessageHeader.MessageType = htole32(MBIM_COMMAND_MSG);
    pRequest->MessageHeader.MessageLength = htole32((sizeof(MBIM_COMMAND_MSG_T) + InformationBufferLength));
    pRequest->MessageHeader.TransactionId = htole32(TransactionId++);

    pRequest->FragmentHeader.TotalFragments = htole32(1);
    pRequest->FragmentHeader.CurrentFragment= htole32(0);

    memcpy(pRequest->DeviceServiceId.uuid, str2uuid(UUID_BASIC_CONNECT), 16);

    pRequest->CID = htole32(CID);
    pRequest->CommandType = htole32(CommandType);
    if (InformationBufferLength && pInformationBuffer) {
        pRequest->InformationBufferLength = htole32(InformationBufferLength);
        memcpy(pRequest->InformationBuffer, pInformationBuffer, InformationBufferLength);
    } else {
        pRequest->InformationBufferLength = htole32(0);
    }

    return &pRequest->MessageHeader;
}

static MBIM_MESSAGE_HEADER *compose_basic_connect_ext_command(UINT32 CID, UINT32 CommandType, void *pInformationBuffer, UINT32 InformationBufferLength)
{
    MBIM_COMMAND_MSG_T *pRequest = (MBIM_COMMAND_MSG_T *)compose_basic_connect_command(CID, CommandType, pInformationBuffer, InformationBufferLength);

    if (!pRequest)
        return NULL;

    memcpy(pRequest->DeviceServiceId.uuid, str2uuid(UUID_BASIC_CONNECT_EXT), 16);

    return &pRequest->MessageHeader;
}

static MBIM_MESSAGE_HEADER *compose_qmi_over_mbim_command(UINT32 CID, UINT32 CommandType, void *pInformationBuffer, UINT32 InformationBufferLength)
{
    MBIM_COMMAND_MSG_T *pRequest = (MBIM_COMMAND_MSG_T *)compose_basic_connect_command(CID, CommandType, pInformationBuffer, InformationBufferLength);

    if (!pRequest)
        return NULL;

    memcpy(pRequest->DeviceServiceId.uuid, str2uuid(uuid_ext_qmux), 16);

    return &pRequest->MessageHeader;
}

static const char * uuid2str(const UUID_T *pUUID) {
    static char str[16*2+4+1];
    const UINT8 *d = pUUID->uuid;

    snprintf(str, sizeof(str), "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7],
        d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15]);

    return str;
}

static const char *DeviceServiceId2str(const UUID_T *pUUID) {
    const char *str = uuid2str(pUUID);

    struct { char *val;char *name;} _enumstr[] = {
        {UUID_BASIC_CONNECT, "UUID_BASIC_CONNECT"},
        {UUID_BASIC_CONNECT_EXT, "UUID_BASIC_CONNECT_EXT"},
        {UUID_SMS, "UUID_SMS"},
        {UUID_USSD, "UUID_USSD"},
        {UUID_PHONEBOOK, "UUID_PHONEBOOK"},
        {UUID_STK, "UUID_STK"},
        {UUID_AUTH, "UUID_AUTH"},
        {UUID_DSS, "UUID_DSS"},
        {uuid_ext_qmux, "uuid_ext_qmux"},
        {uuid_mshsd, "uuid_mshsd"},
        {uuid_qmbe, "uuid_qmbe"},
        {UUID_MSFWID, "UUID_MSFWID"},
        {uuid_atds, "uuid_atds"},
        {uuid_qdu, "uuid_qdu"},
        {UUID_MS_UICC_LOW_LEVEL, "UUID_MS_UICC_LOW_LEVEL"},
        {UUID_MS_SARControl, "UUID_MS_SARControl"},
        {UUID_VOICEEXTENSIONS, "UUID_VOICEEXTENSIONS"},
        {UUID_LIBMBIM_PROXY, "UUID_LIBMBIM_PROXY"},
    };
    int idx;

    for (idx = 0; idx < (int)(sizeof(_enumstr)/sizeof(_enumstr[0])); idx++) {
        if (!strcasecmp(str, _enumstr[idx].val))
            return _enumstr[idx].name;
    }

    return str;
}

static const char *mbim_get_segment(void *_pMsg, UINT32 offset, UINT32 len)
{
    int idx;
    static char buff[256] = {'\0'};
    UINT8 *pMsg = (UINT8*)_pMsg;

    for (idx = 0; idx < (int)(len/2); idx++)
        buff[idx] = pMsg[offset+idx*2];
    buff[idx] = '\0';
    return buff;
}

static void mbim_dump_header(MBIM_MESSAGE_HEADER *pMsg, const char *direction) {
    mbim_debug("%s Header:", direction);
    mbim_debug("%s MessageLength = %u", direction, le32toh(pMsg->MessageLength));
    mbim_debug("%s MessageType =  %s (0x%08x)", direction, MBIMMSGTypeStr(le32toh(pMsg->MessageType)), le32toh(pMsg->MessageType));
    mbim_debug("%s TransactionId = %u", direction, le32toh(pMsg->TransactionId));
    mbim_debug("%s Contents:", direction);
}

static void mbim_dump_uuid_cid(const UUID_T *pUUID, UINT32 CID, const char *direction) {
    size_t idx;
    const char *uuidStr = uuid2str(pUUID);
    const char *cidStr = "unknow";

    for (idx = 0; idx < (sizeof(uuid_cid_string)/sizeof(uuid_cid_string[0])); idx++) {
        if (!strcmp(uuidStr, uuid_cid_string[idx].uuid) && uuid_cid_string[idx].cid == CID) {
            cidStr = uuid_cid_string[idx].name;
        }
    }

    mbim_debug("%s DeviceServiceId = %s (%s)", direction, DeviceServiceId2str(pUUID), uuidStr);
    mbim_debug("%s CID = %s (%u)", direction, cidStr, le32toh(CID));
}


static void mbim_dump_command_msg(MBIM_COMMAND_MSG_T *pCmdMsg, const char *direction) {
    mbim_dump_uuid_cid(&pCmdMsg->DeviceServiceId, le32toh(pCmdMsg->CID), direction);
    mbim_debug("%s CommandType = %s (%u)", direction, le32toh(pCmdMsg->CommandType) ? "set" : "query", le32toh(pCmdMsg->CommandType));
    mbim_debug("%s InformationBufferLength = %u", direction, le32toh(pCmdMsg->InformationBufferLength));
}

static void mbim_dump_command_done(MBIM_COMMAND_DONE_T *pCmdDone, const char *direction) {
    mbim_dump_uuid_cid(&pCmdDone->DeviceServiceId, le32toh(pCmdDone->CID), direction);
    mbim_debug("%s Status = %u", direction, le32toh(pCmdDone->Status));
    mbim_debug("%s InformationBufferLength = %u", direction, le32toh(pCmdDone->InformationBufferLength));
}

static void mbim_dump_indicate_msg(MBIM_INDICATE_STATUS_MSG_T *pIndMsg, const char *direction) {
    mbim_dump_uuid_cid(&pIndMsg->DeviceServiceId, le32toh(pIndMsg->CID), direction);
    mbim_debug("%s InformationBufferLength = %u", direction, le32toh(pIndMsg->InformationBufferLength));
}

static void mbim_dump_connect(MBIM_CONNECT_T *pInfo, const char *direction) {
    mbim_debug("%s SessionId = %u", direction, le32toh(pInfo->SessionId));
    mbim_debug("%s ActivationState = %s (%u)", direction, MBIMActivationStateStr(le32toh(pInfo->ActivationState)), le32toh(pInfo->ActivationState));
    mbim_debug("%s IPType = %s", direction, MBIMContextIPTypeStr(le32toh(pInfo->IPType)));
    mbim_debug("%s VoiceCallState = %s", direction, MBIMVoiceCallStateStr(le32toh(pInfo->VoiceCallState)));
    mbim_debug("%s ContextType = %s", direction, uuid2str(&pInfo->ContextType));
    mbim_debug("%s NwError = %u", direction, le32toh(pInfo->NwError));
}

static void mbim_dump_signal_state(MBIM_SIGNAL_STATE_INFO_T *pInfo, const char *direction)
{
    mbim_debug("%s Rssi = %u", direction, le32toh(pInfo->Rssi));
    mbim_debug("%s ErrorRate = %u", direction, le32toh(pInfo->ErrorRate));
    mbim_debug("%s SignalStrengthInterval = %u", direction, le32toh(pInfo->SignalStrengthInterval));
    mbim_debug("%s RssiThreshold = %u", direction, le32toh(pInfo->RssiThreshold));
    mbim_debug("%s ErrorRateThreshold = %u", direction, le32toh(pInfo->ErrorRateThreshold));
}

static void mbim_dump_packet_service(MBIM_PACKET_SERVICE_INFO_T *pInfo, const char *direction)
{
    mbim_debug("%s NwError = %u", direction, le32toh(pInfo->NwError));
    mbim_debug("%s PacketServiceState = %s", direction, MBIMPacketServiceStateStr(le32toh(pInfo->PacketServiceState)));
    mbim_debug("%s HighestAvailableDataClass = %s", direction, MBIMDataClassStr(le32toh(pInfo->HighestAvailableDataClass)));
    mbim_debug("%s UplinkSpeed = %ld", direction, (long)le64toh(pInfo->UplinkSpeed));
    mbim_debug("%s DownlinkSpeed = %ld", direction, (long)le64toh(pInfo->DownlinkSpeed));
}

static void mbim_dump_subscriber_status(MBIM_SUBSCRIBER_READY_STATUS_T *pInfo, const char *direction)
{
    mbim_debug("%s ReadyState = %s", direction, MBIMSubscriberReadyStateStr(le32toh(pInfo->ReadyState)));
    mbim_debug("%s SIMICCID = %s", direction, mbim_get_segment(pInfo, le32toh(pInfo->SimIccIdOffset), le32toh(pInfo->SimIccIdSize)));
    mbim_debug("%s SubscriberID = %s", direction, mbim_get_segment(pInfo, le32toh(pInfo->SubscriberIdOffset), le32toh(pInfo->SubscriberIdSize)));
    /* maybe more than one number */
    uint32_t idx;
    for (idx = 0; idx < le32toh(pInfo->ElementCount); idx++) {
        UINT32 offset = ((UINT32*)((UINT8*)pInfo+offsetof(MBIM_SUBSCRIBER_READY_STATUS_T, TelephoneNumbersRefList)))[0];
        UINT32 length = ((UINT32*)((UINT8*)pInfo+offsetof(MBIM_SUBSCRIBER_READY_STATUS_T, TelephoneNumbersRefList)))[1];
        mbim_debug("%s Number = %s", direction, mbim_get_segment(pInfo, le32toh(offset), le32toh(length)));
    }
}

static void mbim_dump_regiester_status(MBIM_REGISTRATION_STATE_INFO_T *pInfo, const char *direction)
{
    mbim_debug("%s NwError = %u", direction, le32toh(pInfo->NwError));
    mbim_debug("%s RegisterState = %s", direction, MBIMRegisterStateStr(le32toh(pInfo->RegisterState)));
    mbim_debug("%s RegisterMode = %s", direction, MBIMRegisterModeStr(le32toh(pInfo->RegisterMode)));
}

static void mbim_dump_ipconfig(MBIM_IP_CONFIGURATION_INFO_T *pInfo, const char *direction)
{
    UINT8 prefix = 0, *ipv4=NULL, *ipv6=NULL, *gw=NULL, *dns1=NULL, *dns2=NULL;

    mbim_debug("%s SessionId = %u", direction, le32toh(pInfo->SessionId));
    mbim_debug("%s IPv4ConfigurationAvailable = 0x%x", direction, le32toh(pInfo->IPv4ConfigurationAvailable));
    mbim_debug("%s IPv6ConfigurationAvailable = 0x%x", direction, le32toh(pInfo->IPv6ConfigurationAvailable));
    mbim_debug("%s IPv4AddressCount = 0x%x", direction, le32toh(pInfo->IPv4AddressCount));
    mbim_debug("%s IPv4AddressOffset = 0x%x", direction, le32toh(pInfo->IPv4AddressOffset));
    mbim_debug("%s IPv6AddressCount = 0x%x", direction, le32toh(pInfo->IPv6AddressCount));
    mbim_debug("%s IPv6AddressOffset = 0x%x", direction, le32toh(pInfo->IPv6AddressOffset));

    /* IPv4 */
    if (le32toh(pInfo->IPv4ConfigurationAvailable)&0x1) {
        MBIM_IPV4_ELEMENT_T *pAddress = (MBIM_IPV4_ELEMENT_T *)(&pInfo->DataBuffer[le32toh(pInfo->IPv4AddressOffset)-sizeof(MBIM_IP_CONFIGURATION_INFO_T)]);
        prefix = le32toh(pAddress->OnLinkPrefixLength);
        ipv4 = pAddress->IPv4Address;
        mbim_debug("%s IPv4 = %u.%u.%u.%u/%u", direction, ipv4[0], ipv4[1], ipv4[2], ipv4[3], prefix);
    }
    if (le32toh(pInfo->IPv4ConfigurationAvailable)&0x2) {
        gw = (UINT8 *)(&pInfo->DataBuffer[le32toh(pInfo->IPv4GatewayOffset)-sizeof(MBIM_IP_CONFIGURATION_INFO_T)]);
        mbim_debug("%s gw = %u.%u.%u.%u", direction, gw[0], gw[1], gw[2], gw[3]);
    }
    if (le32toh(pInfo->IPv4ConfigurationAvailable)&0x3) {
        dns1 = (UINT8 *)(&pInfo->DataBuffer[le32toh(pInfo->IPv4DnsServerOffset) -sizeof(MBIM_IP_CONFIGURATION_INFO_T)]);
        mbim_debug("%s dns1 = %u.%u.%u.%u", direction, dns1[0], dns1[1], dns1[2], dns1[3]);
        if (le32toh(pInfo->IPv4DnsServerCount) == 2) {
            dns2 = dns1 + 4;
            mbim_debug("%s dns2 = %u.%u.%u.%u", direction, dns2[0], dns2[1], dns2[2], dns2[3]);
        }
    }
    if (le32toh(pInfo->IPv4Mtu)) mbim_debug("%s ipv4 mtu = %u", direction, le32toh(pInfo->IPv4Mtu));

    /* IPv6 */
    if (le32toh(pInfo->IPv6ConfigurationAvailable)&0x1) {
        MBIM_IPV6_ELEMENT_T *pAddress = (MBIM_IPV6_ELEMENT_T *)(&pInfo->DataBuffer[le32toh(pInfo->IPv6AddressOffset)-sizeof(MBIM_IP_CONFIGURATION_INFO_T)]);
        prefix = le32toh(pAddress->OnLinkPrefixLength);
        ipv6 = pAddress->IPv6Address;
        mbim_debug("%s IPv6 = %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x/%d", \
            direction, ipv6[0], ipv6[1], ipv6[2], ipv6[3], ipv6[4], ipv6[5], ipv6[6], ipv6[7], \
            ipv6[8], ipv6[9], ipv6[10], ipv6[11], ipv6[12], ipv6[13], ipv6[14], ipv6[15], prefix);
    }
    if (le32toh(pInfo->IPv6ConfigurationAvailable)&0x2) {
        gw = (UINT8 *)(&pInfo->DataBuffer[le32toh(pInfo->IPv6GatewayOffset)-sizeof(MBIM_IP_CONFIGURATION_INFO_T)]);
        mbim_debug("%s gw = %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", \
            direction, gw[0], gw[1], gw[2], gw[3], gw[4], gw[5], gw[6], gw[7], \
            gw[8], gw[9], gw[10], gw[11], gw[12], gw[13], gw[14], gw[15]);
    }
    if (le32toh(pInfo->IPv6ConfigurationAvailable)&0x3) {
        dns1 = (UINT8 *)(&pInfo->DataBuffer[le32toh(pInfo->IPv6DnsServerOffset)-sizeof(MBIM_IP_CONFIGURATION_INFO_T)]);
        mbim_debug("%s dns1 = %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", \
            direction, dns1[0], dns1[1], dns1[2], dns1[3], dns1[4], dns1[5], dns1[6], dns1[7], \
            dns1[8], dns1[9], dns1[10], dns1[11], dns1[12], dns1[13], dns1[14], dns1[15]);
        if (le32toh(pInfo->IPv6DnsServerCount) == 2) {
            dns2 = dns1 + 16;
            mbim_debug("%s dns2 = %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", \
                direction, dns2[0], dns2[1], dns2[2], dns2[3], dns1[4], dns1[5], dns1[6], dns1[7],
                dns2[8], dns2[9], dns2[10], dns2[11], dns2[12], dns2[13], dns2[14], dns2[15]);
        }
    }
    if (le32toh(pInfo->IPv6Mtu)) mbim_debug("%s ipv6 mtu = %u", direction, le32toh(pInfo->IPv6Mtu));
}

static void mbim_dump(MBIM_MESSAGE_HEADER *pMsg, int mbim_verbose) {
    unsigned char *data = (unsigned char *)pMsg;
    const char *direction = (le32toh(pMsg->MessageType) & 0x80000000) ? "<" : ">";

    if (!mbim_verbose)
        return;

    if (mbim_verbose) {
        unsigned i;
        static char *_tmp = NULL;

        if (!_tmp)
            _tmp = (char *)malloc(4096);

        if (_tmp) {
            _tmp[0] = (le32toh(pMsg->MessageType) & 0x80000000) ? '<' : '>';
            _tmp[1] = '\0';
            for (i = 0; i < le32toh(pMsg->MessageLength) && i < 4096; i++)
                snprintf(_tmp + strlen(_tmp), 4096 - strlen(_tmp), "%02X:", data[i]);
            mbim_debug("%s", _tmp);
        }
    }

    mbim_dump_header(pMsg, direction);

    switch (le32toh(pMsg->MessageType)) {
    case MBIM_OPEN_MSG: {
        MBIM_OPEN_MSG_T *pOpenMsg = (MBIM_OPEN_MSG_T *)pMsg;
        mbim_debug("%s MaxControlTransfer = %u", direction, le32toh(pOpenMsg->MaxControlTransfer));
    }
    break;
    case MBIM_OPEN_DONE: {
        MBIM_OPEN_DONE_T *pOpenDone = (MBIM_OPEN_DONE_T *)pMsg;
        mbim_debug("%s Status = %u", direction, le32toh(pOpenDone->Status));
    }
    break;
    case MBIM_CLOSE_MSG: {

    }
    break;
    case MBIM_CLOSE_DONE: {
        MBIM_CLOSE_DONE_T *pCloseDone = (MBIM_CLOSE_DONE_T *)pMsg;
        mbim_debug("%s Status = %u", direction, le32toh(pCloseDone->Status));
    }
    break;
    case MBIM_COMMAND_MSG: {
        MBIM_COMMAND_MSG_T *pCmdMsg = (MBIM_COMMAND_MSG_T *)pMsg;

        mbim_dump_command_msg(pCmdMsg, direction);
        if (!memcmp(pCmdMsg->DeviceServiceId.uuid, str2uuid(UUID_BASIC_CONNECT), 16)) {
            switch (le32toh(pCmdMsg->CID)) {
               case  MBIM_CID_CONNECT: {
                    MBIM_SET_CONNECT_T *pInfo = (MBIM_SET_CONNECT_T *)pCmdMsg->InformationBuffer;
                    mbim_debug("%s SessionId = %u", direction, le32toh(pInfo->SessionId));
                }
                break;
                case MBIM_CID_IP_CONFIGURATION: {
                    MBIM_IP_CONFIGURATION_INFO_T *pInfo = (MBIM_IP_CONFIGURATION_INFO_T *)pCmdMsg->InformationBuffer;
                    mbim_debug("%s SessionId = %u", direction, le32toh(pInfo->SessionId));
                }
                break;
                default:
                break;
            }
        }
    }
    break;
    case MBIM_COMMAND_DONE: {
        MBIM_COMMAND_DONE_T *pCmdDone = (MBIM_COMMAND_DONE_T *)pMsg;

        mbim_dump_command_done(pCmdDone, direction);
        if (le32toh(pCmdDone->InformationBufferLength) == 0)
            return;

        if (!memcmp(pCmdDone->DeviceServiceId.uuid, str2uuid(UUID_BASIC_CONNECT), 16)) {
            switch (le32toh(pCmdDone->CID)) {
                case MBIM_CID_CONNECT: {
                MBIM_CONNECT_T *pInfo = (MBIM_CONNECT_T *)pCmdDone->InformationBuffer;
                mbim_dump_connect(pInfo, direction);
                }
                break;
                case MBIM_CID_IP_CONFIGURATION: {
                    //MBIM_IP_CONFIGURATION_INFO_T *pInfo = (MBIM_IP_CONFIGURATION_INFO_T *)pCmdDone->InformationBuffer;
                    //mbim_dump_ipconfig(pInfo, direction);
                }
                break;
                case MBIM_CID_PACKET_SERVICE: {
                    MBIM_PACKET_SERVICE_INFO_T *pInfo = (MBIM_PACKET_SERVICE_INFO_T *)pCmdDone->InformationBuffer;
                    mbim_dump_packet_service(pInfo, direction);
                }
                break;
                case MBIM_CID_SUBSCRIBER_READY_STATUS: {
                    MBIM_SUBSCRIBER_READY_STATUS_T *pInfo = (MBIM_SUBSCRIBER_READY_STATUS_T *)pCmdDone->InformationBuffer;
                    mbim_dump_subscriber_status(pInfo, direction);
                }
                break;
                case MBIM_CID_REGISTER_STATE: {
                    MBIM_REGISTRATION_STATE_INFO_T *pInfo = (MBIM_REGISTRATION_STATE_INFO_T *)pCmdDone->InformationBuffer;
                    mbim_dump_regiester_status(pInfo, direction);
                }
                break;
                default:
                break;
            }
        }
    }
    break;
    case MBIM_INDICATE_STATUS_MSG: {
        MBIM_INDICATE_STATUS_MSG_T *pIndMsg = (MBIM_INDICATE_STATUS_MSG_T *)pMsg;

        mbim_dump_indicate_msg(pIndMsg, direction);
        if (le32toh(pIndMsg->InformationBufferLength) == 0)
            return;

        if (!memcmp(pIndMsg->DeviceServiceId.uuid, str2uuid(UUID_BASIC_CONNECT), 16)) {
            switch (le32toh(pIndMsg->CID)) {
                case MBIM_CID_CONNECT: {
                    MBIM_CONNECT_T *pInfo = (MBIM_CONNECT_T *)pIndMsg->InformationBuffer;
                    mbim_dump_connect(pInfo, direction);
                }
                break;
                case MBIM_CID_SIGNAL_STATE: {
                    MBIM_SIGNAL_STATE_INFO_T *pInfo = (MBIM_SIGNAL_STATE_INFO_T *)pIndMsg->InformationBuffer;
                    mbim_dump_signal_state(pInfo, direction);
                }
                break;
                case MBIM_CID_SUBSCRIBER_READY_STATUS: {
                    MBIM_SUBSCRIBER_READY_STATUS_T *pInfo = (MBIM_SUBSCRIBER_READY_STATUS_T *)pIndMsg->InformationBuffer;
                    mbim_dump_subscriber_status(pInfo, direction);
                }
                break;
                case MBIM_CID_REGISTER_STATE: {
                    MBIM_REGISTRATION_STATE_INFO_T *pInfo = (MBIM_REGISTRATION_STATE_INFO_T *)pIndMsg->InformationBuffer;
                    mbim_dump_regiester_status(pInfo, direction);
                }
                break;
                case MBIM_CID_PACKET_SERVICE: {
                    MBIM_PACKET_SERVICE_INFO_T *pInfo = (MBIM_PACKET_SERVICE_INFO_T *)pIndMsg->InformationBuffer;
                    mbim_dump_packet_service(pInfo, direction);
                }
                break;
                default:
                break;
            }
        }
        else if (!memcmp(pIndMsg->DeviceServiceId.uuid, str2uuid(UUID_BASIC_CONNECT_EXT), 16)) {
        }
    }
    break;
    case MBIM_FUNCTION_ERROR_MSG: {
        MBIM_FUNCTION_ERROR_MSG_T *pErrMsg = (MBIM_FUNCTION_ERROR_MSG_T*)pMsg;
        mbim_debug("%s ErrorStatusCode = %u", direction, le32toh(pErrMsg->ErrorStatusCode));
    }
    break;
    default:
    break;
    }
}

static void mbim_recv_command(MBIM_MESSAGE_HEADER *pResponse, unsigned size)
{
    (void)size;
    pthread_mutex_lock(&mbim_command_mutex);

    if (pResponse)
        mbim_dump(pResponse, mbim_verbose);

    if (pResponse == NULL) {
        pthread_cond_signal(&mbim_command_cond);
    }
    else if (mbim_pRequest && le32toh(mbim_pRequest->TransactionId) == le32toh(pResponse->TransactionId)) {
        mbim_pResponse = mbim_alloc(le32toh(pResponse->MessageLength));
        if (mbim_pResponse)
            memcpy(mbim_pResponse, pResponse, le32toh(pResponse->MessageLength));
        pthread_cond_signal(&mbim_command_cond);
    }
    else if (le32toh(pResponse->MessageType) ==  MBIM_INDICATE_STATUS_MSG) {
        MBIM_INDICATE_STATUS_MSG_T *pIndMsg = (MBIM_INDICATE_STATUS_MSG_T *)pResponse;

        if (!memcmp(pIndMsg->DeviceServiceId.uuid, str2uuid(UUID_BASIC_CONNECT), 16))
        {
            switch (le32toh(pIndMsg->CID)) {
                case MBIM_CID_SUBSCRIBER_READY_STATUS: {
                    MBIM_SUBSCRIBER_READY_STATUS_T *pInfo = (MBIM_SUBSCRIBER_READY_STATUS_T *)pIndMsg->InformationBuffer;
                    if (oldReadyState != le32toh(pInfo->ReadyState))
                        qmidevice_send_event_to_main(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED);
                }
                break;
                case MBIM_CID_REGISTER_STATE: {
                    MBIM_REGISTRATION_STATE_INFO_T *pInfo = (MBIM_REGISTRATION_STATE_INFO_T *)pIndMsg->InformationBuffer;
                    if (oldRegisterState != le32toh(pInfo->RegisterState))
                        qmidevice_send_event_to_main(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED);
                }
                break;
                case MBIM_CID_PACKET_SERVICE: {
                    MBIM_PACKET_SERVICE_INFO_T *pInfo = (MBIM_PACKET_SERVICE_INFO_T *)pIndMsg->InformationBuffer;
                    MBIM_PACKET_SERVICE_STATE_E state = le32toh(pInfo->PacketServiceState);

                    if (oldPacketServiceState != state
                            && (1 || MBIMPacketServiceStateAttached == state || MBIMPacketServiceStateDetached == state))
                        qmidevice_send_event_to_main(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED);
                }
                break;
                case MBIM_CID_CONNECT: {
                    MBIM_CONNECT_T *pInfo = (MBIM_CONNECT_T *)pIndMsg->InformationBuffer;
                    if (pInfo->SessionId == (uint32_t)mbim_sessionID) {
                        MBIM_ACTIVATION_STATE_E state = le32toh(pInfo->ActivationState);

                        if (oldActivationState != state
                            && (1 || MBIMActivationStateActivated == state || MBIMActivationStateDeactivated == state))
                            qmidevice_send_event_to_main(RIL_UNSOL_DATA_CALL_LIST_CHANGED);
                    }
                }
                break;
                default:
                break;
            }
        }
    }

    pthread_mutex_unlock(&mbim_command_mutex);
}

static int mbim_send_command(MBIM_MESSAGE_HEADER *pRequest, MBIM_COMMAND_DONE_T **ppCmdDone, unsigned msecs) {
    int ret;

    if (ppCmdDone)
         *ppCmdDone = NULL;

    if (mbim_fd <= 0)
        return -ENODEV;

   if (s_tid_reader == 0)
      return -EINVAL;

    if (!pRequest)
        return -ENOMEM;

    pthread_mutex_lock(&mbim_command_mutex);

    if (pRequest) {
        if (pRequest->TransactionId == (0xFFFFFF + 1)) { //quectel-mbim-proxy need 0xFF000000 to indicat client
            TransactionId = 1;
            pRequest->TransactionId = htole32(TransactionId++);
        }
        mbim_dump(pRequest, mbim_verbose);
    }

    mbim_pRequest = pRequest;
    mbim_pResponse = NULL;

    ret = write(mbim_fd, pRequest, le32toh(pRequest->MessageLength));

    if (ret > 0 && (uint32_t)ret == le32toh(pRequest->MessageLength)) {
        ret = pthread_cond_timeout_np(&mbim_command_cond, &mbim_command_mutex, msecs);
        if (!ret) {
            if (mbim_pResponse && ppCmdDone) {
                *ppCmdDone = (MBIM_COMMAND_DONE_T *)mbim_pResponse;
            }
        }
    } else {
        mbim_debug("%s pthread_cond_timeout_np=%d", __func__, ret);
    }

    mbim_pRequest = mbim_pResponse = NULL;

    pthread_mutex_unlock(&mbim_command_mutex);

    return ret;
}

static ssize_t mbim_proxy_read (int fd, MBIM_MESSAGE_HEADER *pResponse, size_t size) {
    ssize_t nreads;

    nreads = read(fd, pResponse, sizeof(MBIM_MESSAGE_HEADER));
    if (nreads == sizeof(MBIM_MESSAGE_HEADER) && le32toh(pResponse->MessageLength) <= size) {
        nreads += read(fd, pResponse+1, le32toh(pResponse->MessageLength) - sizeof(MBIM_MESSAGE_HEADER));
    }

    return nreads;
}

static void * mbim_read_thread(void *param) {
    PROFILE_T *profile = (PROFILE_T *)param;
    const char *cdc_wdm = (const char *)profile->qmichannel;
    int wait_for_request_quit = 0;

    mbim_verbose = debug_qmi;
    s_tid_reader = pthread_self();

    if (profile->qmap_mode > 1 && profile->qmapnet_adapter[0]) {
        if (!profile->proxy[0])
             sprintf(profile->proxy, "%s", QUECTEL_MBIM_PROXY);
        mbim_sessionID = profile->pdp;
    }

    if (profile->proxy[0]) {
        mbim_fd = cm_open_proxy(profile->proxy);
    } 
    else {
        mbim_fd = cm_open_dev(cdc_wdm);
    }

    if (mbim_fd <= 0) {
        mbim_debug("fail to open (%s), errno: %d (%s)", cdc_wdm, errno, strerror(errno));
        goto __quit;
    }

    dbg_time("cdc_wdm_fd = %d", mbim_fd);

    qmidevice_send_event_to_main(RIL_INDICATE_DEVICE_CONNECTED);

    while (mbim_fd > 0) {
        struct pollfd pollfds[] = {{mbim_fd, POLLIN, 0}, {qmidevice_control_fd[1], POLLIN, 0}, {qmi_over_mbim_sk[1], POLLIN, 0}};
        int ne, ret, nevents = 2;

        if (pollfds[nevents].fd != -1)
            nevents++;

        ret = poll(pollfds, nevents, wait_for_request_quit ? 1000 : -1);

        if (ret == 0 && wait_for_request_quit) {
            break;
        }

        if (ret < 0) {
            mbim_debug("%s poll=%d, errno: %d (%s)", __func__, ret, errno, strerror(errno));
            break;
        }

        for (ne = 0; ne < nevents; ne++) {
            int fd = pollfds[ne].fd;
            short revents = pollfds[ne].revents;

            if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
                mbim_debug("%s poll err/hup/inval", __func__);
                mbim_debug("epoll fd = %d, events = 0x%04x", fd, revents);
                if (revents & (POLLERR | POLLHUP | POLLNVAL))
                goto __quit;
            }

            if ((revents & POLLIN) == 0)
                continue;

            if (mbim_fd == fd) {
                ssize_t nreads;
                MBIM_MESSAGE_HEADER *pResponse = (MBIM_MESSAGE_HEADER *) cm_recv_buf;

                if (profile->proxy[0])
                    nreads = mbim_proxy_read(fd, pResponse, sizeof(cm_recv_buf));
                else
                    nreads = read(fd, pResponse, sizeof(cm_recv_buf));

                if (nreads <= 0) {
                    mbim_debug("%s read=%d errno: %d (%s)",  __func__, (int)nreads, errno, strerror(errno));
                    break;
                }

                mbim_recv_command(pResponse, nreads);
            }
            else if (fd == qmidevice_control_fd[1]) {
                int triger_event;
                if (read(fd, &triger_event, sizeof(triger_event)) == sizeof(triger_event)) {
                    //mbim_debug("triger_event = 0x%x", triger_event);
                    switch (triger_event) {
                        case RIL_REQUEST_QUIT:
                            goto __quit;
                        break;
                        case SIG_EVENT_STOP:
                            wait_for_request_quit = 1;
                        break;
                        default:
                        break;
                    }
                }
            }
            else if (fd == qmi_over_mbim_sk[1]) {
                    ssize_t nreads = read(fd, cm_recv_buf, sizeof(cm_recv_buf));
                    if (nreads > 0)
                        QmiThreadRecvQMI((PQCQMIMSG)cm_recv_buf);
            }
        }
    }

__quit:
    if (mbim_fd != -1) { close(mbim_fd); mbim_fd = -1; }
    mbim_recv_command(NULL, 0);
    qmidevice_send_event_to_main(RIL_INDICATE_DEVICE_DISCONNECTED);
    mbim_debug("%s exit", __func__);
    s_tid_reader = 0;

    return NULL;
}

static int mbim_status_code(MBIM_MESSAGE_HEADER *pMsgHdr) {
    int status = 0;

    if (!pMsgHdr)
        return 0;

    switch (le32toh(pMsgHdr->MessageType)) {
        case MBIM_OPEN_DONE: {
            MBIM_OPEN_DONE_T *pOpenDone = (MBIM_OPEN_DONE_T *)pMsgHdr;
            status = le32toh(pOpenDone->Status);
        }
        break;
        case MBIM_CLOSE_DONE: {
            MBIM_CLOSE_DONE_T *pCloseDone = (MBIM_CLOSE_DONE_T *)pMsgHdr;
            status = le32toh(pCloseDone->Status);
        }
        break;
        case MBIM_COMMAND_DONE: {
            MBIM_COMMAND_DONE_T *pCmdDone = (MBIM_COMMAND_DONE_T *)pMsgHdr;
            status = le32toh(pCmdDone->Status);
        }
        break;
        case MBIM_FUNCTION_ERROR_MSG: {
            MBIM_FUNCTION_ERROR_MSG_T *pErrMsg = (MBIM_FUNCTION_ERROR_MSG_T *)pMsgHdr;
            status = le32toh(pErrMsg->ErrorStatusCode);
            if (status == MBIM_ERROR_NOT_OPENED)
                mbim_open_state = 0; //EM06ELAR03A05M4G when suspend/resume, may get this error
        }
        break;
        default:
        break;
    }

    return status;
}

#define mbim_check_err(err, pRequest, pCmdDone) do { \
    int _status = mbim_status_code(pCmdDone ? &pCmdDone->MessageHeader : NULL); \
    if (err || _status || !pCmdDone) { \
        if (pCmdDone) { mbim_dump(&pCmdDone->MessageHeader, (mbim_verbose == 0)); } \
        mbim_free(pRequest); mbim_free(pCmdDone); \
        mbim_debug("%s:%d err=%d, Status=%d", __func__, __LINE__, err, _status); \
        if (err) return err; \
        if (_status) return _status; \
        return 8888; \
    } \
} while(0)

/*
 * MBIM device can be open repeatly without error
 * So, we can call the function, no matter it have been opened or not
 */
static int mbim_open_device(uint32_t MaxControlTransfer) {
    MBIM_MESSAGE_HEADER *pRequest = NULL;
    MBIM_OPEN_DONE_T *pOpenDone = NULL;
    int err = 0;

    mbim_debug("%s()", __func__);
    pRequest = compose_open_command(MaxControlTransfer);
    err = mbim_send_command(pRequest, (MBIM_COMMAND_DONE_T **)&pOpenDone, 3*1000); //EM06ELAR03A09M4G take about 2.5 seconds
    mbim_check_err(err, pRequest, pOpenDone);

    err = le32toh(pOpenDone->Status);
    mbim_free(pRequest); mbim_free(pOpenDone);

    return err;
}

static int mbim_close_device(void) {
    MBIM_MESSAGE_HEADER *pRequest = NULL;
    MBIM_CLOSE_DONE_T *pCloseDone = NULL;
    int err = 0;

    mbim_debug("%s()", __func__);
    pRequest = compose_close_command();
    err = mbim_send_command(pRequest, (MBIM_COMMAND_DONE_T **)&pCloseDone, mbim_default_timeout);
    mbim_check_err(err, pRequest, pCloseDone);

    err = le32toh(pCloseDone->Status);
    mbim_free(pRequest); mbim_free(pCloseDone);

    return err;
}

static int mbim_query_connect(int sessionID) {
    MBIM_MESSAGE_HEADER *pRequest = NULL;
    MBIM_COMMAND_DONE_T *pCmdDone = NULL;
    MBIM_SET_CONNECT_T set_connect;
    int err;

    if (ActivationState != MBIMActivationStateActivated  || mbim_verbose)
        mbim_debug("%s(sessionID=%d)", __func__, sessionID); //avoid two many log
    set_connect.SessionId = htole32(sessionID);
    pRequest = compose_basic_connect_command(MBIM_CID_CONNECT, MBIM_CID_CMD_TYPE_QUERY, &set_connect, sizeof(set_connect));
    err = mbim_send_command(pRequest, &pCmdDone, mbim_default_timeout);
    mbim_check_err(err, pRequest, pCmdDone);

    if (le32toh(pCmdDone->InformationBufferLength))
    {
        MBIM_CONNECT_T *pInfo = (MBIM_CONNECT_T *)pCmdDone->InformationBuffer;
        ActivationState = le32toh(pInfo->ActivationState);
        mbim_update_state();
    }
    mbim_free(pRequest); mbim_free(pCmdDone);
    return err;
}

static int mbim_ms_version_query(void) {
    MBIM_MESSAGE_HEADER *pRequest = NULL;
    MBIM_COMMAND_DONE_T *pCmdDone = NULL;
    int err;

    struct _bc_ext_version {
            UINT8 ver_minor;
            UINT8 ver_major;
            UINT8 ext_ver_minor;
            UINT8 ext_ver_major;
    } __attribute__ ((packed)) bc_ext_version;

    bc_ext_version.ver_major = 1;
    bc_ext_version.ver_minor = 0;
    bc_ext_version.ext_ver_major = 2;
    bc_ext_version.ext_ver_minor = 0;

    pRequest = compose_basic_connect_ext_command(MBIM_CID_MS_VERSION, MBIM_CID_CMD_TYPE_QUERY, &bc_ext_version, sizeof(bc_ext_version));
    err = mbim_send_command(pRequest, &pCmdDone, mbim_default_timeout);
    mbim_check_err(err, pRequest, pCmdDone);

    if (le32toh(pCmdDone->InformationBufferLength)) {
        struct _bc_ext_version *pInfo = (struct _bc_ext_version *)pCmdDone->InformationBuffer;
        //mbim_debug("%s ext_rel_ver major=%d, minor=%d", __func__, pInfo->ext_ver_major, pInfo->ext_ver_minor);
        mbim_ms_version = pInfo->ext_ver_major;
    }

    mbim_free(pRequest); mbim_free(pCmdDone);
    return err;
}

static int mbim_device_services_query(void) {
    MBIM_MESSAGE_HEADER *pRequest = NULL;
    MBIM_COMMAND_DONE_T *pCmdDone = NULL;
    int err;
    int mbim_v2_support = 0;

    mbim_debug("%s()", __func__);
    pRequest = compose_basic_connect_command(MBIM_CID_DEVICE_SERVICES, MBIM_CID_CMD_TYPE_QUERY, NULL, 0);
    err = mbim_send_command(pRequest, &pCmdDone, mbim_default_timeout);
    mbim_check_err(err, pRequest, pCmdDone);

    if (pCmdDone->InformationBufferLength) {
         MBIM_DEVICE_SERVICES_INFO_T *pInfo = (MBIM_DEVICE_SERVICES_INFO_T *)pCmdDone->InformationBuffer;
         UINT32 i;

        for (i = 0; i < le32toh(pInfo->DeviceServicesCount) ; i++) {
            //UINT32 size = pInfo->DeviceServicesRefList[i].size;
            UINT32 offset = le32toh(pInfo->DeviceServicesRefList[i].offset);
            MBIM_DEVICE_SERVICE_ELEMENT_T *pSrvEle = (MBIM_DEVICE_SERVICE_ELEMENT_T *)((void *)pInfo + offset);

            //mbim_debug("\t[%2d] %s (%s)", i, DeviceServiceId2str(&pSrvEle->DeviceServiceId), uuid2str(&pSrvEle->DeviceServiceId));
            if (!strcasecmp(UUID_BASIC_CONNECT_EXT, uuid2str(&pSrvEle->DeviceServiceId))) {
                UINT32 cid = 0;

                for (cid = 0; cid < le32toh(pSrvEle->CidCount); cid++) {
                  if (MBIM_CID_MS_VERSION == le32toh(pSrvEle->CidList[cid])) {
                        mbim_v2_support = 1;
                    }
                }
            }
            else if (!strcasecmp(uuid_ext_qmux, uuid2str(&pSrvEle->DeviceServiceId))) {
                qmi_over_mbim_support = 1;
            }
        }
    }
    mbim_free(pRequest); mbim_free(pCmdDone);

    if (mbim_v2_support) {
        mbim_ms_version_query();
    }

    return err;
}

static int mbim_device_caps_query(PROFILE_T *profile) {
    MBIM_MESSAGE_HEADER *pRequest = NULL;
    MBIM_COMMAND_DONE_T *pCmdDone = NULL;
    int err;

    mbim_debug("%s()", __func__);
    pRequest = compose_basic_connect_command(MBIM_CID_DEVICE_CAPS, MBIM_CID_CMD_TYPE_QUERY, NULL, 0);
    err = mbim_send_command(pRequest, &pCmdDone, mbim_default_timeout);
    mbim_check_err(err, pRequest, pCmdDone);

    if (le32toh(pCmdDone->InformationBufferLength)) {
         MBIM_DEVICE_CAPS_INFO_T *pInfo = (MBIM_DEVICE_CAPS_INFO_T *)pCmdDone->InformationBuffer;
         char tmp[32];

         if (le32toh(pInfo->DeviceIdOffset) && le32toh(pInfo->DeviceIdSize)) {
            wchar2char((const char *)pInfo + le32toh(pInfo->DeviceIdOffset), le32toh(pInfo->DeviceIdSize), tmp, sizeof(tmp));
            mbim_debug("DeviceId:     %s", tmp);
         }
         if (le32toh(pInfo->FirmwareInfoOffset) && le32toh(pInfo->FirmwareInfoSize)) {
            wchar2char((const char *)pInfo + le32toh(pInfo->FirmwareInfoOffset), le32toh(pInfo->FirmwareInfoSize), tmp, sizeof(tmp));
            strncpy(profile->BaseBandVersion, tmp, sizeof(profile->BaseBandVersion));
            mbim_debug("FirmwareInfo: %s", tmp);
         }
         if (le32toh(pInfo->HardwareInfoOffset) && le32toh(pInfo->HardwareInfoSize)) {
            wchar2char((const char *)pInfo + le32toh(pInfo->HardwareInfoOffset), le32toh(pInfo->HardwareInfoSize), tmp, sizeof(tmp));
            mbim_debug("HardwareInfo: %s", tmp);
         }
    }
    mbim_free(pRequest); mbim_free(pCmdDone);
    return err;
}

#if 0
static int mbim_radio_state_query(void) {
    MBIM_MESSAGE_HEADER *pRequest = NULL;
    MBIM_COMMAND_DONE_T *pCmdDone = NULL;
    int err;

    mbim_debug("%s()", __func__);
    pRequest = compose_basic_connect_command(MBIM_CID_RADIO_STATE, MBIM_CID_CMD_TYPE_QUERY, NULL, 0);
    err = mbim_send_command(pRequest, &pCmdDone, mbim_default_timeout);
    mbim_check_err(err, pRequest, pCmdDone);

    if (pCmdDone->InformationBufferLength) {
         MBIM_RADIO_STATE_INFO_T *pInfo = (MBIM_RADIO_STATE_INFO_T *)pCmdDone->InformationBuffer;
        mbim_debug("HwRadioState: %d, SwRadioState: %d", pInfo->HwRadioState, pInfo->SwRadioState);
    }
    mbim_free(pRequest); mbim_free(pCmdDone);
    return err;
}
#endif

static int mbim_set_radio_state(MBIM_RADIO_SWITCH_STATE_E RadioState) {
    MBIM_MESSAGE_HEADER *pRequest = NULL;
    MBIM_COMMAND_DONE_T *pCmdDone = NULL;
    UINT32 value = htole32(RadioState);
    int err;

    mbim_debug("%s( %d )", __func__, RadioState);
    pRequest = compose_basic_connect_command(MBIM_CID_RADIO_STATE, MBIM_CID_CMD_TYPE_SET, &value, sizeof(value));
    err = mbim_send_command(pRequest, &pCmdDone, mbim_default_timeout);
    mbim_check_err(err, pRequest, pCmdDone);

    if (le32toh(pCmdDone->InformationBufferLength)) {
         MBIM_RADIO_STATE_INFO_T *pInfo = (MBIM_RADIO_STATE_INFO_T *)pCmdDone->InformationBuffer;
        mbim_debug("HwRadioState: %d, SwRadioState: %d", le32toh(pInfo->HwRadioState), le32toh(pInfo->SwRadioState));
    }
    mbim_free(pRequest); mbim_free(pCmdDone);
    return err;
}

static int mbim_subscriber_status_query(void) {
    MBIM_MESSAGE_HEADER *pRequest = NULL;
    MBIM_COMMAND_DONE_T *pCmdDone = NULL;
    int err;

    mbim_debug("%s()", __func__);
    pRequest = compose_basic_connect_command(MBIM_CID_SUBSCRIBER_READY_STATUS, MBIM_CID_CMD_TYPE_QUERY, NULL, 0);
    err = mbim_send_command(pRequest, &pCmdDone, mbim_default_timeout);
    mbim_check_err(err, pRequest, pCmdDone);

    if (le32toh(pCmdDone->InformationBufferLength)) {
         MBIM_SUBSCRIBER_READY_STATUS_T *pInfo = (MBIM_SUBSCRIBER_READY_STATUS_T *)pCmdDone->InformationBuffer;
         char tmp[32];

        wchar2char((const char *)pInfo + le32toh(pInfo->SubscriberIdOffset), le32toh(pInfo->SubscriberIdSize), tmp, sizeof(tmp));
        mbim_debug("SubscriberId: %s", tmp);
        wchar2char((const char *)pInfo + le32toh(pInfo->SimIccIdOffset), le32toh(pInfo->SimIccIdSize), tmp, sizeof(tmp));
        mbim_debug("SimIccId:     %s", tmp);
        ReadyState = le32toh(pInfo->ReadyState);
        mbim_update_state();
    }
    mbim_free(pRequest); mbim_free(pCmdDone);
    return err;
}

static int mbim_register_state_query(void) {
    MBIM_MESSAGE_HEADER *pRequest = NULL;
    MBIM_COMMAND_DONE_T *pCmdDone = NULL;
    int err;

    mbim_debug("%s()", __func__);
    pRequest = compose_basic_connect_command(MBIM_CID_REGISTER_STATE, MBIM_CID_CMD_TYPE_QUERY, NULL, 0);
    err = mbim_send_command(pRequest, &pCmdDone, mbim_default_timeout);
    mbim_check_err(err, pRequest, pCmdDone);

    if (le32toh(pCmdDone->InformationBufferLength)) {
        MBIM_REGISTRATION_STATE_INFO_T *pInfo = (MBIM_REGISTRATION_STATE_INFO_T *)pCmdDone->InformationBuffer;;
        RegisterState = le32toh(pInfo->RegisterState);
        mbim_update_state();
    }
    mbim_free(pRequest); mbim_free(pCmdDone);
    return err;
}

static int mbim_packet_service_query(void) {
    MBIM_MESSAGE_HEADER *pRequest = NULL;
    MBIM_COMMAND_DONE_T *pCmdDone = NULL;
    int err;

    mbim_debug("%s()", __func__);
    pRequest = compose_basic_connect_command(MBIM_CID_PACKET_SERVICE, MBIM_CID_CMD_TYPE_QUERY, NULL, 0);
    err = mbim_send_command(pRequest, &pCmdDone, mbim_default_timeout);
    mbim_check_err(err, pRequest, pCmdDone);

    if (le32toh(pCmdDone->InformationBufferLength)) {
        MBIM_PACKET_SERVICE_INFO_T *pInfo = (MBIM_PACKET_SERVICE_INFO_T *)pCmdDone->InformationBuffer;
        PacketServiceState = le32toh(pInfo->PacketServiceState);
        mbim_update_state();

        if (le32toh(pCmdDone->InformationBufferLength) == sizeof(MBIM_PACKET_SERVICE_INFO_V2_T)) {
            MBIM_PACKET_SERVICE_INFO_V2_T *pInfo = (MBIM_PACKET_SERVICE_INFO_V2_T *)pCmdDone->InformationBuffer;
            mbim_debug("CurrentDataClass = %s", MBIMDataClassStr(le32toh(pInfo->CurrentDataClass)));
        }
    }
    mbim_free(pRequest); mbim_free(pCmdDone);
    return err;
}

static int mbim_packet_service_set(MBIM_PACKET_SERVICE_ACTION_E action) {
    MBIM_MESSAGE_HEADER *pRequest = NULL;
    MBIM_COMMAND_DONE_T *pCmdDone = NULL;
    UINT32 value = htole32(action);
    int err;

    mbim_debug("%s()", __func__);
    pRequest = compose_basic_connect_command(MBIM_CID_PACKET_SERVICE, MBIM_CID_CMD_TYPE_SET, &value, sizeof(value));
    err = mbim_send_command(pRequest, &pCmdDone, mbim_default_timeout);
    mbim_check_err(err, pRequest, pCmdDone);

    if (le32toh(pCmdDone->InformationBufferLength)) {
        MBIM_PACKET_SERVICE_INFO_T *pInfo = (MBIM_PACKET_SERVICE_INFO_T *)pCmdDone->InformationBuffer;
        PacketServiceState = le32toh(pInfo->PacketServiceState);
        mbim_update_state();
    }
    mbim_free(pRequest); mbim_free(pCmdDone);
    return err;
}

#define _align_32(len) {len += (len % 4) ? (4 - (len % 4)) : 0;}
static int mbim_populate_connect_data(MBIM_SET_CONNECT_T **connect_req_ptr) {
    int offset;
    int buflen = 0;

    if (mbim_apn && strlen(mbim_apn) > 0) buflen += 2*strlen(mbim_apn) ;
    _align_32(buflen);
    if (mbim_user && strlen(mbim_user) > 0) buflen += 2*strlen(mbim_user);
    _align_32(buflen);
    if (mbim_passwd && strlen(mbim_passwd) > 0) buflen += 2*strlen(mbim_passwd);
    _align_32(buflen);

    *connect_req_ptr = (MBIM_SET_CONNECT_T*)malloc(sizeof(MBIM_SET_CONNECT_T) + buflen);
    if (! *connect_req_ptr) {
        mbim_debug("not enough memory\n");
        return -1;
    }
    memset(*connect_req_ptr, 0, sizeof(MBIM_SET_CONNECT_T) + buflen);

    offset = 0;
    if (mbim_apn && strlen(mbim_apn) > 0) {
        (*connect_req_ptr)->AccessStringSize = htole32(2*strlen(mbim_apn));
        (*connect_req_ptr)->AccessStringOffset = htole32(offset + sizeof(MBIM_SET_CONNECT_T));
        offset = char2wchar(mbim_apn, strlen(mbim_apn), &(*connect_req_ptr)->DataBuffer[offset], buflen - offset);
        _align_32(offset);
    }

    if (mbim_user && strlen(mbim_user) > 0) {
        (*connect_req_ptr)->UserNameSize = htole32(2*strlen(mbim_user));
        (*connect_req_ptr)->UserNameOffset = htole32(offset + sizeof(MBIM_SET_CONNECT_T));
        offset = char2wchar(mbim_user, strlen(mbim_user), &(*connect_req_ptr)->DataBuffer[offset], buflen - offset);
        _align_32(offset);
    }

    if (mbim_passwd && strlen(mbim_passwd) > 0) {
        (*connect_req_ptr)->PasswordSize = htole32(2*strlen(mbim_passwd));
        (*connect_req_ptr)->PasswordOffset = htole32(offset + sizeof(MBIM_SET_CONNECT_T));
        offset = char2wchar(mbim_passwd, strlen(mbim_passwd), &(*connect_req_ptr)->DataBuffer[offset], buflen - offset);
    }

    return buflen;
}

static int mbim_set_connect(int onoff, int sessionID) {
    MBIM_MESSAGE_HEADER *pRequest = NULL;
    MBIM_COMMAND_DONE_T *pCmdDone = NULL;
    MBIM_SET_CONNECT_T *set_connect = NULL;
    int err;

    mbim_debug("%s(onoff=%d, sessionID=%d)", __func__, onoff, sessionID);
    /* alloc memory then populate APN USERNAME PASSWORD */
    int buflen = mbim_populate_connect_data(&set_connect);
    if (buflen < 0) {
        return ENOMEM;
     }

    set_connect->SessionId = htole32(sessionID);
    if (onoff == 0)
        set_connect->ActivationCommand = htole32(MBIMActivationCommandDeactivate);
    else
        set_connect->ActivationCommand = htole32(MBIMActivationCommandActivate);

    set_connect->Compression = htole32(MBIMCompressionNone);
    set_connect->AuthProtocol = htole32(mbim_auth);
    set_connect->IPType = htole32(mbim_iptype);
    memcpy(set_connect->ContextType.uuid, str2uuid(UUID_MBIMContextTypeInternet), 16);

    pRequest = compose_basic_connect_command(MBIM_CID_CONNECT, MBIM_CID_CMD_TYPE_SET, set_connect, sizeof(MBIM_SET_CONNECT_T) + buflen);
    mbim_free(set_connect);
    err = mbim_send_command(pRequest, &pCmdDone, mbim_default_timeout*10);
    mbim_check_err(err, pRequest, pCmdDone);

    if (le32toh(pCmdDone->InformationBufferLength)) {
        MBIM_CONNECT_T *pInfo = (MBIM_CONNECT_T *)pCmdDone->InformationBuffer;
        ActivationState = le32toh(pInfo->ActivationState);
        mbim_update_state();
    }

    mbim_free(pRequest); mbim_free(pCmdDone);
    return err;
}

static int mbim_ip_config(PROFILE_T *profile, int sessionID) {
    MBIM_MESSAGE_HEADER *pRequest = NULL;
    MBIM_COMMAND_DONE_T *pCmdDone = NULL;
    MBIM_IP_CONFIGURATION_INFO_T ip_info;
    int err;

    if (profile->ipv4.Address == 0 || mbim_verbose)
        mbim_debug("%s(sessionID=%d)", __func__, sessionID);
    ip_info.SessionId = htole32(sessionID);
    pRequest = compose_basic_connect_command(MBIM_CID_IP_CONFIGURATION, MBIM_CID_CMD_TYPE_QUERY, &ip_info, sizeof(ip_info));
    err = mbim_send_command(pRequest, &pCmdDone, mbim_default_timeout);
    mbim_check_err(err, pRequest, pCmdDone);

    if (le32toh(pCmdDone->InformationBufferLength)) {
        UINT8 prefix, *ipv4=NULL, *ipv6=NULL, *gw=NULL, *dns1=NULL, *dns2=NULL;
        UINT32 mtu = 1500;
        MBIM_IP_CONFIGURATION_INFO_T *pInfo = (MBIM_IP_CONFIGURATION_INFO_T *)pCmdDone->InformationBuffer;

        /* IPv4 network configration */
        if (le32toh(pInfo->IPv4ConfigurationAvailable)&0x1) {
            MBIM_IPV4_ELEMENT_T *pAddress = (MBIM_IPV4_ELEMENT_T *)(&pInfo->DataBuffer[le32toh(pInfo->IPv4AddressOffset)-sizeof(MBIM_IP_CONFIGURATION_INFO_T)]);
            prefix = le32toh(pAddress->OnLinkPrefixLength);
            ipv4 = pAddress->IPv4Address;

            if (le32toh(pInfo->IPv4ConfigurationAvailable)&0x2)
                gw = (UINT8 *)(&pInfo->DataBuffer[le32toh(pInfo->IPv4GatewayOffset)-sizeof(MBIM_IP_CONFIGURATION_INFO_T)]);

            if (le32toh(pInfo->IPv4ConfigurationAvailable)&0x4) {
                dns1 = (UINT8 *)(&pInfo->DataBuffer[le32toh(pInfo->IPv4DnsServerOffset)-sizeof(MBIM_IP_CONFIGURATION_INFO_T)]);
                if (le32toh(pInfo->IPv4DnsServerCount) == 2)
                    dns2 = dns1 + 4;
            }

            if (le32toh(pInfo->IPv4ConfigurationAvailable)&0x8)
                mtu =  le32toh(pInfo->IPv4Mtu);

            if (profile->ipv4.Address != mbim2qmi_ipv4addr(*(uint32_t *)ipv4) || mbim_verbose) {
                mbim_dump_ipconfig(pInfo, "<");
                profile->ipv4.Address = mbim2qmi_ipv4addr(*(uint32_t *)ipv4);
            }

			if(gw != NULL)
            	profile->ipv4.Gateway = mbim2qmi_ipv4addr(*(uint32_t *)gw);
            profile->ipv4.SubnetMask = mbim2qmi_ipv4addr(0xFFFFFFFF>>(32-prefix)<<(32-prefix));
			if(dns1 != NULL)
            	profile->ipv4.DnsPrimary = mbim2qmi_ipv4addr(*(uint32_t *)dns1);
			if(dns2 != NULL)
            	profile->ipv4.DnsSecondary = mbim2qmi_ipv4addr(*(uint32_t *)dns2);
            profile->ipv4.Mtu = mbim2qmi_ipv4addr(mtu);
        }

        /* IPv6 network configration */
        if (le32toh(pInfo->IPv6ConfigurationAvailable)&0x1) {
			gw = NULL; dns1 = NULL; dns2 = NULL;
            MBIM_IPV6_ELEMENT_T *pAddress = (MBIM_IPV6_ELEMENT_T *)(&pInfo->DataBuffer[le32toh(pInfo->IPv6AddressOffset)-sizeof(MBIM_IP_CONFIGURATION_INFO_T)]);
            prefix = le32toh(pAddress->OnLinkPrefixLength);
            ipv6 = pAddress->IPv6Address;

            if (le32toh(pInfo->IPv6ConfigurationAvailable)&0x2)
                gw = (UINT8 *)(&pInfo->DataBuffer[le32toh(pInfo->IPv6GatewayOffset)-sizeof(MBIM_IP_CONFIGURATION_INFO_T)]);

            if (le32toh(pInfo->IPv6ConfigurationAvailable)&0x4) {
                dns1 = (UINT8 *)(&pInfo->DataBuffer[le32toh(pInfo->IPv6DnsServerOffset)-sizeof(MBIM_IP_CONFIGURATION_INFO_T)]);
                if (le32toh(pInfo->IPv6DnsServerCount) == 2)
                    dns2 = dns1 + 16;
            }

            if (le32toh(pInfo->IPv6ConfigurationAvailable)&0x8)
                mtu =  le32toh(pInfo->IPv6Mtu);

            if(ipv6 != NULL)
            	mbim2qmi_ipv6addr(ipv6, profile->ipv6.Address);
			if(gw != NULL)
            	mbim2qmi_ipv6addr(gw, profile->ipv6.Gateway);
			if(dns1 != NULL)
            	mbim2qmi_ipv6addr(dns1, profile->ipv6.DnsPrimary);
			if(dns2 != NULL)
            	mbim2qmi_ipv6addr(dns2, profile->ipv6.DnsSecondary);
            profile->ipv6.PrefixLengthIPAddr = prefix;
            profile->ipv6.PrefixLengthGateway = prefix;
            profile->ipv6.Mtu = mbim2qmi_ipv4addr(mtu);
        }
    }
    return err;
}

int mbim_proxy_configure(const char *dev) {
    MBIM_MESSAGE_HEADER *pRequest = NULL;
    MBIM_COMMAND_DONE_T *pCmdDone = NULL;
    MBIM_LIBQMI_PROXY_CONFIG_T *cfg;
    int err;

    pRequest = compose_basic_connect_command(
                                                    MBIM_CID_PROXY_CONTROL_CONFIGURATION,
                                                    MBIM_CID_CMD_TYPE_SET,
                                                    NULL,
                                                    sizeof(*cfg) + strlen(dev)*2);
    if (pRequest) {
        memcpy(((MBIM_COMMAND_MSG_T *)pRequest)->DeviceServiceId.uuid, str2uuid(UUID_LIBMBIM_PROXY), 16);
        cfg = (MBIM_LIBQMI_PROXY_CONFIG_T *)((MBIM_COMMAND_MSG_T *)pRequest)->InformationBuffer;

        cfg->DevicePathOffset = sizeof(*cfg);
        cfg->DevicePathSize = char2wchar(dev, strlen(dev), cfg->DataBuffer, strlen(dev)*2);
        cfg->Timeout = 15;
    }

    err = mbim_send_command(pRequest, &pCmdDone, mbim_default_timeout);
    mbim_check_err(err, pRequest, pCmdDone);

    mbim_free(pRequest); mbim_free(pCmdDone);
    return err;
}

static int mbim_update_state(void) {
    int chages = 0;

    if (oldReadyState != ReadyState) {
        mbim_debug("SubscriberReadyState %s -> %s ", MBIMSubscriberReadyStateStr(oldReadyState), MBIMSubscriberReadyStateStr(ReadyState));
        oldReadyState = ReadyState; chages++;
    }
    if (oldRegisterState != RegisterState) {
        mbim_debug("RegisterState %s -> %s ", MBIMRegisterStateStr(oldRegisterState), MBIMRegisterStateStr(RegisterState));
        oldRegisterState = RegisterState; chages++;
    }
    if (oldPacketServiceState != PacketServiceState) {
        mbim_debug("PacketServiceState %s -> %s ", MBIMPacketServiceStateStr(oldPacketServiceState), MBIMPacketServiceStateStr(PacketServiceState));
        oldPacketServiceState = PacketServiceState; chages++;
    }
    if (oldActivationState != ActivationState) {
        mbim_debug("ActivationState %s -> %s ", MBIMActivationStateStr(oldActivationState), MBIMActivationStateStr(ActivationState));
        oldActivationState = ActivationState; chages++;
    }

    return chages;
}

static int mbim_init(PROFILE_T *profile) {
    int retval;
    int t = 0;

    if (profile->proxy[0] && !strcmp(profile->proxy, LIBMBIM_PROXY)) {
        retval = mbim_proxy_configure(profile->qmichannel);
        if (retval) goto exit;
    }

    while (t++ < 10) {
        retval = mbim_open_device(4096);
        if (retval != ETIMEDOUT)
            break;
    }
    if (retval) goto exit;
    retval = mbim_device_caps_query(profile);
    if (retval) goto exit;
    mbim_update_state();
    retval = mbim_device_services_query();
    if (retval) goto exit;
    mbim_update_state();
    retval = mbim_set_radio_state(MBIMRadioOn);
    if (retval) goto exit;
    mbim_update_state();

    if (qmi_over_mbim_support) {
        if (!socketpair( AF_LOCAL, SOCK_STREAM, 0, qmi_over_mbim_sk)) {
            qmidev_send = qmi_over_mbim_qmidev_send;
#ifdef CONFIG_CELLINFO //by now, only this function need QMI OVER MBIM
            qmi_over_mbim_nas = qmi_over_mbim_get_client_id(QMUX_TYPE_NAS);
#endif
        }
    }

    return 0;

exit:
    return retval;
}

static int mbim_deinit(void) {
    if (qmi_over_mbim_nas) {
        qmi_over_mbim_release_client_id(QMUX_TYPE_NAS, qmi_over_mbim_nas);
        qmi_over_mbim_nas = 0;
    }
    
    mbim_close_device();

    if (qmi_over_mbim_sk[0] != -1) {
        close(qmi_over_mbim_sk[0]);
        close(qmi_over_mbim_sk[1]);
    }

    return 0;
}

const struct qmi_device_ops mbim_dev_ops = {
    .init = mbim_init,
    .deinit = mbim_deinit,
    .read = mbim_read_thread,
};

static int requestBaseBandVersion(PROFILE_T *profile) {
    (void)profile;
    return 0;
}

static int requestGetSIMStatus(SIM_Status *pSIMStatus)
{
    int retval;

    *pSIMStatus = SIM_ABSENT;
    retval = mbim_subscriber_status_query();
    if (retval)
        goto exit;
    mbim_update_state();

     switch(ReadyState) {
        case MBIMSubscriberReadyStateNotInitialized: *pSIMStatus = SIM_NOT_READY; break;
        case MBIMSubscriberReadyStateInitialized: *pSIMStatus = SIM_READY; break;
        case MBIMSubscriberReadyStateSimNotInserted: *pSIMStatus = SIM_ABSENT; break;
        case MBIMSubscriberReadyStateBadSim: *pSIMStatus = SIM_BAD; break;
        case MBIMSubscriberReadyStateFailure: *pSIMStatus = SIM_ABSENT; break;
        case MBIMSubscriberReadyStateNotActivated: *pSIMStatus = SIM_ABSENT; break;
        case MBIMSubscriberReadyStateDeviceLocked: *pSIMStatus = SIM_PIN; break;
        default: *pSIMStatus = SIM_ABSENT; break;
    }

exit:
    return retval;
}

static int requestRegistrationState(UCHAR *pPSAttachedState) {
    int retval;

    *pPSAttachedState = 0;
    retval = mbim_register_state_query();
    if (retval)
        goto exit;
    mbim_update_state();

    switch (RegisterState) {
        case MBIMRegisterStateUnknown: *pPSAttachedState = 0; break;
        case MBIMRegisterStateDeregistered: *pPSAttachedState = 0; break;
        case MBIMRegisterStateSearching: *pPSAttachedState = 0; break;
        case MBIMRegisterStateHome: *pPSAttachedState = 1; break;
        case MBIMRegisterStateRoaming: *pPSAttachedState = 1; break;
        case MBIMRegisterStatePartner: *pPSAttachedState = 0; break;
        case MBIMRegisterStateDenied: *pPSAttachedState = 0; break;
        default: *pPSAttachedState = 0; break;
    }

    if (*pPSAttachedState == 0)
        goto exit;

    retval =  mbim_packet_service_query();
    if (retval)
        goto exit;

    switch (PacketServiceState) {
        case MBIMPacketServiceStateUnknown: *pPSAttachedState = 0; break;
        case MBIMPacketServiceStateAttaching: *pPSAttachedState = 0; break;
        case MBIMPacketServiceStateAttached: *pPSAttachedState = 1; break;
        case MBIMPacketServiceStateDetaching: *pPSAttachedState = 0; break;
        case MBIMPacketServiceStateDetached: *pPSAttachedState = 0; break;
        default: *pPSAttachedState = 0; break;
    }

    if (*pPSAttachedState == 0)
        mbim_packet_service_set(MBIMPacketServiceActionAttach);

exit:
    return retval;
}

static int requestSetupDataCall(PROFILE_T *profile, int curIpFamily) {
    int retval;

    (void)curIpFamily;

    if (profile->apn)
        mbim_apn = profile->apn;
    if (profile->user)
        mbim_user = profile->user;
    if (profile->password)
        mbim_passwd = profile->password;
    if (profile->auth)
        mbim_auth = profile->auth;
    if (profile->enable_ipv4)
        mbim_iptype = MBIMContextIPTypeIPv4;
    if (profile->enable_ipv6)
        mbim_iptype = MBIMContextIPTypeIPv6;
    if (profile->enable_ipv4 && profile->enable_ipv6)
        mbim_iptype = MBIMContextIPTypeIPv4AndIPv6;

    retval = mbim_set_connect(1, mbim_sessionID);
    if (retval)
        goto exit;

exit:
    return retval;
}

static int requestQueryDataCall(UCHAR  *pConnectionStatus, int curIpFamily) {
    int retval;

    (void)curIpFamily;

    *pConnectionStatus = QWDS_PKT_DATA_DISCONNECTED;

    retval = mbim_query_connect(mbim_sessionID);
    if (retval)
        goto exit;

    switch(ActivationState) {
        case MBIMActivationStateUnknown: *pConnectionStatus = QWDS_PKT_DATA_UNKNOW; break;
        case MBIMActivationStateActivated: *pConnectionStatus = QWDS_PKT_DATA_CONNECTED; break;
        case MBIMActivationStateActivating: *pConnectionStatus = QWDS_PKT_DATA_DISCONNECTED; break;
        case MBIMActivationStateDeactivated: *pConnectionStatus = QWDS_PKT_DATA_DISCONNECTED; break;
        case MBIMActivationStateDeactivating: *pConnectionStatus = QWDS_PKT_DATA_DISCONNECTED; break;
        default: *pConnectionStatus = QWDS_PKT_DATA_DISCONNECTED; break;
    }

exit:
    return retval;
}

static int requestDeactivateDefaultPDP(PROFILE_T *profile, int curIpFamily) {
    int retval;
    
    (void)profile;
    (void)curIpFamily;

    retval = mbim_set_connect(0, mbim_sessionID);
    if (retval)
        goto exit;

exit:
    return retval;
}

static int requestGetIPAddress(PROFILE_T *profile, int curIpFamily) {
    int retval;
    
    (void)curIpFamily;
    retval = mbim_ip_config(profile, mbim_sessionID);
    if (retval)
        goto exit;

exit:
    return retval;
}

#ifdef CONFIG_CELLINFO
static int requestGetCellInfoList(void) {
    if (qmi_over_mbim_nas) {
        if (qmi_request_ops.requestGetCellInfoList)
            return qmi_request_ops.requestGetCellInfoList();
    }

    return 0;
}
#endif

const struct request_ops mbim_request_ops = {
    .requestBaseBandVersion = requestBaseBandVersion,
    .requestGetSIMStatus = requestGetSIMStatus,
    .requestRegistrationState = requestRegistrationState,
    .requestSetupDataCall = requestSetupDataCall,
    .requestQueryDataCall = requestQueryDataCall,
    .requestDeactivateDefaultPDP = requestDeactivateDefaultPDP,
    .requestGetIPAddress = requestGetIPAddress,
#ifdef CONFIG_CELLINFO
    .requestGetCellInfoList = requestGetCellInfoList,
#endif
};

int qmi_over_mbim_qmidev_send(PQCQMIMSG pQMI) {
    MBIM_MESSAGE_HEADER *pRequest = NULL;
    MBIM_COMMAND_DONE_T *pCmdDone = NULL;
    int err;
    size_t len = le16toh(pQMI->QMIHdr.Length) + 1;

    if (pQMI->QMIHdr.QMIType != QMUX_TYPE_CTL) {
        if (pQMI->QMIHdr.QMIType == QMUX_TYPE_NAS)
            pQMI->QMIHdr.ClientId = qmi_over_mbim_nas;

        if (pQMI->QMIHdr.ClientId == 0) {
            dbg_time("QMIType %d has no clientID", pQMI->QMIHdr.QMIType);
            return -ENODEV;
        }
    }

    pRequest = compose_qmi_over_mbim_command(1, MBIM_CID_CMD_TYPE_SET, pQMI, len);
    err = mbim_send_command(pRequest, &pCmdDone, mbim_default_timeout);
    mbim_check_err(err, pRequest, pCmdDone);

    err = -1;
    len = le32toh(pCmdDone->InformationBufferLength);
    if (len) {
        if (write(qmi_over_mbim_sk[0], pCmdDone->InformationBuffer, len) == (long)len) {
            err = 0;
        };
    }

    mbim_free(pRequest); mbim_free(pCmdDone);
    return err;
}
