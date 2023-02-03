/*===========================================================================

                            M P Q M I. H
DESCRIPTION:

    This module contains forward references to the QMI module.

INITIALIZATION AND SEQUENCING REQUIREMENTS:

Copyright (C) 2011 by Qualcomm Technologies, Incorporated.  All Rights Reserved.
===========================================================================*/
/*===========================================================================

                            EDIT HISTORY FOR FILE
  $Header: //depot/QMI/win/qcdrivers/ndis/MPQMI.h#3 $

when        who    what, where, why
--------    ---    ----------------------------------------------------------
11/20/04    hg     Initial version.
===========================================================================*/

#ifndef USBQMI_H
#define USBQMI_H

typedef uint8_t uint8;
typedef int8_t int8;
typedef uint16_t uint16;
typedef int16_t int16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef signed char    CHAR;
typedef unsigned char   UCHAR;
typedef short   SHORT;
typedef unsigned short   USHORT;
typedef int   INT;
typedef unsigned int   UINT;
typedef long    LONG;
typedef unsigned int   ULONG;
typedef unsigned long long   ULONG64;
typedef signed char   *PCHAR;
typedef unsigned char   *PUCHAR;
typedef int   *PINT;
typedef int   BOOL;

#define TRUE (1 == 1)
#define FALSE (1 != 1)

#define QMICTL_SUPPORTED_MAJOR_VERSION 1
#define QMICTL_SUPPORTED_MINOR_VERSION 0

#pragma pack(push, 1)

// ========= USB Control Message ==========

#define USB_CTL_MSG_TYPE_QMI 0x01

// USB Control Message
typedef struct _QCUSB_CTL_MSG_HDR
{
   UCHAR IFType;
} __attribute__ ((packed)) QCUSB_CTL_MSG_HDR, *PQCUSB_CTL_MSG_HDR;

#define QCUSB_CTL_MSG_HDR_SIZE sizeof(QCUSB_CTL_MSG_HDR)

typedef struct _QCUSB_CTL_MSG
{
   UCHAR IFType;
   UCHAR Message;
} __attribute__ ((packed)) QCUSB_CTL_MSG, *PQCUSB_CTL_MSG;

#define QCTLV_TYPE_REQUIRED_PARAMETER 0x01
#define QCTLV_TYPE_RESULT_CODE        0x02

// ================= QMI ==================

// Define QMI Type
typedef enum _QMI_SERVICE_TYPE
{
   QMUX_TYPE_CTL  = 0x00,
   QMUX_TYPE_WDS  = 0x01,
   QMUX_TYPE_DMS  = 0x02,
   QMUX_TYPE_NAS  = 0x03,
   QMUX_TYPE_QOS  = 0x04,
   QMUX_TYPE_WMS  = 0x05,
   QMUX_TYPE_PDS  = 0x06,
   QMUX_TYPE_UIM  = 0x0B,
   QMUX_TYPE_WDS_IPV6  = 0x11,
   QMUX_TYPE_WDS_ADMIN  = 0x1A,
   QMUX_TYPE_COEX  = 0x22,
   QMUX_TYPE_MAX  = 0xFF,
   QMUX_TYPE_ALL  = 0xFF
} QMI_SERVICE_TYPE;

typedef enum _QMI_RESULT_CODE_TYPE
{
   QMI_RESULT_SUCCESS = 0x0000,
   QMI_RESULT_FAILURE = 0x0001
} QMI_RESULT_CODE_TYPE;

typedef enum _QMI_ERROR_CODE_TYPE
{
    QMI_ERR_NONE  = 0x0000
    ,QMI_ERR_MALFORMED_MSG  = 0x0001
    ,QMI_ERR_NO_MEMORY  = 0x0002
    ,QMI_ERR_INTERNAL  = 0x0003
    ,QMI_ERR_ABORTED  = 0x0004
    ,QMI_ERR_CLIENT_IDS_EXHAUSTED  = 0x0005
    ,QMI_ERR_UNABORTABLE_TRANSACTION  = 0x0006
    ,QMI_ERR_INVALID_CLIENT_ID  = 0x0007
    ,QMI_ERR_NO_THRESHOLDS  = 0x0008
    ,QMI_ERR_INVALID_HANDLE  = 0x0009
    ,QMI_ERR_INVALID_PROFILE  = 0x000A
    ,QMI_ERR_INVALID_PINID  = 0x000B
    ,QMI_ERR_INCORRECT_PIN  = 0x000C
    ,QMI_ERR_NO_NETWORK_FOUND  = 0x000D
    ,QMI_ERR_CALL_FAILED  = 0x000E
    ,QMI_ERR_OUT_OF_CALL  = 0x000F
    ,QMI_ERR_NOT_PROVISIONED  = 0x0010
    ,QMI_ERR_MISSING_ARG  = 0x0011
    ,QMI_ERR_ARG_TOO_LONG  = 0x0013
    ,QMI_ERR_INVALID_TX_ID  = 0x0016
    ,QMI_ERR_DEVICE_IN_USE  = 0x0017
    ,QMI_ERR_OP_NETWORK_UNSUPPORTED  = 0x0018
    ,QMI_ERR_OP_DEVICE_UNSUPPORTED  = 0x0019
    ,QMI_ERR_NO_EFFECT  = 0x001A
    ,QMI_ERR_NO_FREE_PROFILE  = 0x001B
    ,QMI_ERR_INVALID_PDP_TYPE  = 0x001C
    ,QMI_ERR_INVALID_TECH_PREF  = 0x001D
    ,QMI_ERR_INVALID_PROFILE_TYPE  = 0x001E
    ,QMI_ERR_INVALID_SERVICE_TYPE  = 0x001F
    ,QMI_ERR_INVALID_REGISTER_ACTION  = 0x0020
    ,QMI_ERR_INVALID_PS_ATTACH_ACTION  = 0x0021
    ,QMI_ERR_AUTHENTICATION_FAILED  = 0x0022
    ,QMI_ERR_PIN_BLOCKED  = 0x0023
    ,QMI_ERR_PIN_PERM_BLOCKED  = 0x0024
    ,QMI_ERR_SIM_NOT_INITIALIZED  = 0x0025
    ,QMI_ERR_MAX_QOS_REQUESTS_IN_USE  = 0x0026
    ,QMI_ERR_INCORRECT_FLOW_FILTER  = 0x0027
    ,QMI_ERR_NETWORK_QOS_UNAWARE  = 0x0028
    ,QMI_ERR_INVALID_QOS_ID  = 0x0029
    ,QMI_ERR_INVALID_ID  = 0x0029
    ,QMI_ERR_REQUESTED_NUM_UNSUPPORTED  = 0x002A
    ,QMI_ERR_INTERFACE_NOT_FOUND  = 0x002B
    ,QMI_ERR_FLOW_SUSPENDED  = 0x002C
    ,QMI_ERR_INVALID_DATA_FORMAT  = 0x002D
    ,QMI_ERR_GENERAL  = 0x002E
    ,QMI_ERR_UNKNOWN  = 0x002F
    ,QMI_ERR_INVALID_ARG  = 0x0030
    ,QMI_ERR_INVALID_INDEX  = 0x0031
    ,QMI_ERR_NO_ENTRY  = 0x0032
    ,QMI_ERR_DEVICE_STORAGE_FULL  = 0x0033
    ,QMI_ERR_DEVICE_NOT_READY  = 0x0034
    ,QMI_ERR_NETWORK_NOT_READY  = 0x0035
    ,QMI_ERR_CAUSE_CODE  = 0x0036
    ,QMI_ERR_MESSAGE_NOT_SENT  = 0x0037
    ,QMI_ERR_MESSAGE_DELIVERY_FAILURE  = 0x0038
    ,QMI_ERR_INVALID_MESSAGE_ID  = 0x0039
    ,QMI_ERR_ENCODING  = 0x003A
    ,QMI_ERR_AUTHENTICATION_LOCK  = 0x003B
    ,QMI_ERR_INVALID_TRANSITION  = 0x003C
    ,QMI_ERR_NOT_A_MCAST_IFACE  = 0x003D
    ,QMI_ERR_MAX_MCAST_REQUESTS_IN_USE  = 0x003E
    ,QMI_ERR_INVALID_MCAST_HANDLE = 0x003F
    ,QMI_ERR_INVALID_IP_FAMILY_PREF  = 0x0040
    ,QMI_ERR_SESSION_INACTIVE  = 0x0041
    ,QMI_ERR_SESSION_INVALID  = 0x0042
    ,QMI_ERR_SESSION_OWNERSHIP  = 0x0043
    ,QMI_ERR_INSUFFICIENT_RESOURCES  = 0x0044
    ,QMI_ERR_DISABLED  = 0x0045
    ,QMI_ERR_INVALID_OPERATION  = 0x0046
    ,QMI_ERR_INVALID_QMI_CMD  = 0x0047
    ,QMI_ERR_TPDU_TYPE  = 0x0048
    ,QMI_ERR_SMSC_ADDR  = 0x0049
    ,QMI_ERR_INFO_UNAVAILABLE  = 0x004A
    ,QMI_ERR_SEGMENT_TOO_LONG  = 0x004B
    ,QMI_ERR_SEGMENT_ORDER  = 0x004C
    ,QMI_ERR_BUNDLING_NOT_SUPPORTED  = 0x004D
    ,QMI_ERR_OP_PARTIAL_FAILURE  = 0x004E
    ,QMI_ERR_POLICY_MISMATCH  = 0x004F
    ,QMI_ERR_SIM_FILE_NOT_FOUND  = 0x0050
    ,QMI_ERR_EXTENDED_INTERNAL  = 0x0051
    ,QMI_ERR_ACCESS_DENIED  = 0x0052
    ,QMI_ERR_HARDWARE_RESTRICTED  = 0x0053
    ,QMI_ERR_ACK_NOT_SENT  = 0x0054
    ,QMI_ERR_INJECT_TIMEOUT  = 0x0055
    ,QMI_ERR_INCOMPATIBLE_STATE = 0x005A
    ,QMI_ERR_FDN_RESTRICT  = 0x005B
    ,QMI_ERR_SUPS_FAILURE_CAUSE  = 0x005C
    ,QMI_ERR_NO_RADIO  = 0x005D
    ,QMI_ERR_NOT_SUPPORTED  = 0x005E
    ,QMI_ERR_NO_SUBSCRIPTION  = 0x005F
    ,QMI_ERR_CARD_CALL_CONTROL_FAILED  = 0x0060
    ,QMI_ERR_NETWORK_ABORTED  = 0x0061
    ,QMI_ERR_MSG_BLOCKED  = 0x0062
    ,QMI_ERR_INVALID_SESSION_TYPE  = 0x0064
    ,QMI_ERR_INVALID_PB_TYPE  = 0x0065
    ,QMI_ERR_NO_SIM  = 0x0066
    ,QMI_ERR_PB_NOT_READY  = 0x0067
    ,QMI_ERR_PIN_RESTRICTION  = 0x0068
    ,QMI_ERR_PIN2_RESTRICTION  = 0x0069
    ,QMI_ERR_PUK_RESTRICTION  = 0x006A
    ,QMI_ERR_PUK2_RESTRICTION  = 0x006B
    ,QMI_ERR_PB_ACCESS_RESTRICTED  = 0x006C
    ,QMI_ERR_PB_DELETE_IN_PROG  = 0x006D
    ,QMI_ERR_PB_TEXT_TOO_LONG  = 0x006E
    ,QMI_ERR_PB_NUMBER_TOO_LONG  = 0x006F
    ,QMI_ERR_PB_HIDDEN_KEY_RESTRICTION  = 0x0070
} QMI_ERROR_CODE_TYPE;

#define QCQMI_CTL_FLAG_SERVICE   0x80
#define QCQMI_CTL_FLAG_CTL_POINT 0x00

typedef struct _QCQMI_HDR
{
   UCHAR  IFType;
   USHORT Length;
   UCHAR  CtlFlags;  // reserved
   UCHAR  QMIType;
   UCHAR  ClientId;
} __attribute__ ((packed)) QCQMI_HDR, *PQCQMI_HDR;

#define QCQMI_HDR_SIZE (sizeof(QCQMI_HDR)-1)

typedef struct _QCQMI
{
   UCHAR  IFType;
   USHORT Length;
   UCHAR  CtlFlags;  // reserved
   UCHAR  QMIType;
   UCHAR  ClientId;
   UCHAR  SDU;
} __attribute__ ((packed)) QCQMI, *PQCQMI;

typedef struct _QMI_SERVICE_VERSION
{
   USHORT Major;
   USHORT Minor;
   USHORT AddendumMajor;
   USHORT AddendumMinor;
} __attribute__ ((packed)) QMI_SERVICE_VERSION, *PQMI_SERVICE_VERSION;

// ================= QMUX ==================

#define QMUX_MSG_OVERHEAD_BYTES 4  // Type(USHORT) Length(USHORT) -- header

#define QMUX_BROADCAST_CID 0xFF

typedef struct _QCQMUX_HDR
{
   UCHAR  CtlFlags;      // 0: single QMUX Msg; 1:
   USHORT TransactionId;
} __attribute__ ((packed)) QCQMUX_HDR, *PQCQMUX_HDR;

typedef struct _QCQMUX
{
   UCHAR  CtlFlags;      // 0: single QMUX Msg; 1:
   USHORT TransactionId;
   UCHAR  Message;  // Type(2), Length(2), Value
} __attribute__ ((packed)) QCQMUX, *PQCQMUX;

#define QCQMUX_HDR_SIZE sizeof(QCQMUX_HDR)

typedef struct _QCQMUX_MSG_HDR
{
   USHORT Type;
   USHORT Length;
} __attribute__ ((packed)) QCQMUX_MSG_HDR, *PQCQMUX_MSG_HDR;

#define QCQMUX_MSG_HDR_SIZE sizeof(QCQMUX_MSG_HDR)

typedef struct _QCQMUX_MSG_HDR_RESP
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;        // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
} __attribute__ ((packed)) QCQMUX_MSG_HDR_RESP, *PQCQMUX_MSG_HDR_RESP;

typedef struct _QCQMUX_TLV
{
   UCHAR Type;
   USHORT Length;
   UCHAR  Value;
} __attribute__ ((packed)) QCQMUX_TLV, *PQCQMUX_TLV;

typedef struct _QMI_TLV_HDR
{
   UCHAR  TLVType;
   USHORT TLVLength;
} __attribute__ ((packed)) QMI_TLV_HDR, *PQMI_TLV_HDR;

typedef struct _QMI_TLV
{
   UCHAR  TLVType;
   USHORT TLVLength;
   union {
        int8_t s8;
        uint8_t u8;
        int16_t s16;
        uint16_t u16;
        int32_t s32;
        uint32_t u32;
        uint64_t u64;
   };
} __attribute__ ((packed)) QMI_TLV, *PQMI_TLV;

// QMUX Message Definitions -- QMI SDU
#define QMUX_CTL_FLAG_SINGLE_MSG    0x00
#define QMUX_CTL_FLAG_COMPOUND_MSG  0x01
#define QMUX_CTL_FLAG_TYPE_CMD      0x00
#define QMUX_CTL_FLAG_TYPE_RSP      0x02
#define QMUX_CTL_FLAG_TYPE_IND      0x04
#define QMUX_CTL_FLAG_MASK_COMPOUND 0x01
#define QMUX_CTL_FLAG_MASK_TYPE     0x06 // 00-cmd, 01-rsp, 10-ind

#pragma pack(pop)

#endif // USBQMI_H
