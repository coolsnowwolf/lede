/******************************************************************************
  @file    QCQCTL.h

  DESCRIPTION
  This module contains QMI QCTL module.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None.

  ---------------------------------------------------------------------------
  Copyright (c) 2016 - 2023 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


#ifndef QCQCTL_H
#define QCQCTL_H

#include "QCQMI.h"

#pragma pack(push, 1)

// ================= QMICTL ==================

// QMICTL Control Flags
#define QMICTL_CTL_FLAG_CMD     0x00
#define QMICTL_CTL_FLAG_RSP     0x01
#define QMICTL_CTL_FLAG_IND     0x02

#if 0
typedef struct _QMICTL_TRANSACTION_ITEM
{
   LIST_ENTRY  List;
   UCHAR       TransactionId;  // QMICTL transaction id
   PVOID       Context;        // Adapter or IocDev
   PIRP        Irp;
} QMICTL_TRANSACTION_ITEM, *PQMICTL_TRANSACTION_ITEM;
#endif

typedef struct _QCQMICTL_MSG_HDR
{
   UCHAR  CtlFlags;  // 00-cmd, 01-rsp, 10-ind
   UCHAR  TransactionId;
   USHORT QMICTLType;
   USHORT Length;
} __attribute__ ((packed)) QCQMICTL_MSG_HDR, *PQCQMICTL_MSG_HDR;

#define QCQMICTL_MSG_HDR_SIZE sizeof(QCQMICTL_MSG_HDR)

typedef struct _QCQMICTL_MSG_HDR_RESP
{
   UCHAR  CtlFlags;  // 00-cmd, 01-rsp, 10-ind
   UCHAR  TransactionId;
   USHORT QMICTLType;
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;        // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
} __attribute__ ((packed)) QCQMICTL_MSG_HDR_RESP, *PQCQMICTL_MSG_HDR_RESP;

typedef struct _QCQMICTL_MSG
{
   UCHAR  CtlFlags;  // 00-cmd, 01-rsp, 10-ind
   UCHAR  TransactionId;
   USHORT QMICTLType;
   USHORT Length;
   UCHAR  Payload;
} __attribute__ ((packed)) QCQMICTL_MSG, *PQCQMICTL_MSG;

// TLV Header
typedef struct _QCQMICTL_TLV_HDR
{
   UCHAR  TLVType;
   USHORT TLVLength;
} __attribute__ ((packed)) QCQMICTL_TLV_HDR, *PQCQMICTL_TLV_HDR;

#define QCQMICTL_TLV_HDR_SIZE sizeof(QCQMICTL_TLV_HDR)

// QMICTL Type
#define QMICTL_SET_INSTANCE_ID_REQ    0x0020
#define QMICTL_SET_INSTANCE_ID_RESP   0x0020
#define QMICTL_GET_VERSION_REQ        0x0021
#define QMICTL_GET_VERSION_RESP       0x0021
#define QMICTL_GET_CLIENT_ID_REQ      0x0022
#define QMICTL_GET_CLIENT_ID_RESP     0x0022
#define QMICTL_RELEASE_CLIENT_ID_REQ  0x0023
#define QMICTL_RELEASE_CLIENT_ID_RESP 0x0023
#define QMICTL_REVOKE_CLIENT_ID_IND   0x0024
#define QMICTL_INVALID_CLIENT_ID_IND  0x0025
#define QMICTL_SET_DATA_FORMAT_REQ    0x0026
#define QMICTL_SET_DATA_FORMAT_RESP   0x0026
#define QMICTL_SYNC_REQ               0x0027
#define QMICTL_SYNC_RESP              0x0027
#define QMICTL_SYNC_IND               0x0027
#define QMI_MESSAGE_CTL_INTERNAL_PROXY_OPEN 0xFF00

#define QMICTL_FLAG_REQUEST    0x00
#define QMICTL_FLAG_RESPONSE   0x01
#define QMICTL_FLAG_INDICATION 0x02

// QMICTL Message Definitions

typedef struct _QMICTL_SET_INSTANCE_ID_REQ_MSG
{
   UCHAR  CtlFlags;        // QMICTL_FLAG_REQUEST
   UCHAR  TransactionId;
   USHORT QMICTLType;      // QMICTL_SET_INSTANCE_ID_REQ
   USHORT Length;          // 4
   UCHAR  TLVType;         // QCTLV_TYPE_REQUIRED_PARAMETER
   USHORT TLVLength;       // 1
   UCHAR  Value;           // Host-unique QMI instance for this device driver
} __attribute__ ((packed)) QMICTL_SET_INSTANCE_ID_REQ_MSG, *PQMICTL_SET_INSTANCE_ID_REQ_MSG;

typedef struct _QMICTL_SET_INSTANCE_ID_RESP_MSG
{
   UCHAR  CtlFlags;        // QMICTL_FLAG_RESPONSE
   UCHAR  TransactionId;
   USHORT QMICTLType;      // QMICTL_SET_INSTANCE_ID_RESP
   USHORT Length;
   UCHAR  TLVType;         // QCTLV_TYPE_RESULT_CODE
   USHORT TLVLength;       // 0x0004
   USHORT QMIResult;
   USHORT QMIError;
   UCHAR  TLV2Type;        // QCTLV_TYPE_REQUIRED_PARAMETER
   USHORT TLV2Length;      // 0x0002
   USHORT QMI_ID;          // Upper byte is assigned by MSM,
                           // lower assigned by host
} __attribute__ ((packed)) QMICTL_SET_INSTANCE_ID_RESP_MSG, *PQMICTL_SET_INSTANCE_ID_RESP_MSG;

typedef struct _QMICTL_GET_VERSION_REQ_MSG
{
   UCHAR  CtlFlags;        // QMICTL_FLAG_REQUEST
   UCHAR  TransactionId;
   USHORT QMICTLType;      // QMICTL_GET_VERSION_REQ
   USHORT Length;          // 0
   UCHAR  TLVType;         // QCTLV_TYPE_REQUIRED_PARAMETER
   USHORT TLVLength;       // var
   UCHAR  QMUXTypes;       // List of one byte QMUX_TYPE values
                           // 0xFF returns a list of versions for all
                           // QMUX_TYPEs implemented on the device
} __attribute__ ((packed)) QMICTL_GET_VERSION_REQ_MSG, *PQMICTL_GET_VERSION_REQ_MSG;

typedef struct _QMUX_TYPE_VERSION_STRUCT
{
   UCHAR  QMUXType;
   USHORT MajorVersion;
   USHORT MinorVersion;
} __attribute__ ((packed)) QMUX_TYPE_VERSION_STRUCT, *PQMUX_TYPE_VERSION_STRUCT;

typedef struct _ADDENDUM_VERSION_PREAMBLE
{
   UCHAR LabelLength;
   UCHAR Label;
} __attribute__ ((packed)) ADDENDUM_VERSION_PREAMBLE, *PADDENDUM_VERSION_PREAMBLE;

#define QMICTL_GET_VERSION_RSP_TLV_TYPE_VERSION     0x01
#define QMICTL_GET_VERSION_RSP_TLV_TYPE_ADD_VERSION 0x10

typedef struct _QMICTL_GET_VERSION_RESP_MSG
{
   UCHAR  CtlFlags;        // QMICTL_FLAG_RESPONSE
   UCHAR  TransactionId;
   USHORT QMICTLType;      // QMICTL_GET_VERSION_RESP
   USHORT Length;
   UCHAR  TLVType;         // QCTLV_TYPE_RESULT_CODE
   USHORT TLVLength;       // 0x0004
   USHORT QMIResult;
   USHORT QMIError;
   UCHAR  TLV2Type;        // QCTLV_TYPE_REQUIRED_PARAMETER
   USHORT TLV2Length;      // var
   UCHAR  NumElements;     // Num of QMUX_TYPE_VERSION_STRUCT
   QMUX_TYPE_VERSION_STRUCT TypeVersion[0];
} __attribute__ ((packed)) QMICTL_GET_VERSION_RESP_MSG, *PQMICTL_GET_VERSION_RESP_MSG;

typedef struct _QMICTL_GET_CLIENT_ID_REQ_MSG
{
   UCHAR  CtlFlags;        // QMICTL_FLAG_REQUEST
   UCHAR  TransactionId;
   USHORT QMICTLType;      // QMICTL_GET_CLIENT_ID_REQ
   USHORT Length;
   UCHAR  TLVType;         // QCTLV_TYPE_REQUIRED_PARAMETER
   USHORT TLVLength;       // 1
   UCHAR  QMIType;         // QMUX type
} __attribute__ ((packed)) QMICTL_GET_CLIENT_ID_REQ_MSG, *PQMICTL_GET_CLIENT_ID_REQ_MSG;

typedef struct _QMICTL_GET_CLIENT_ID_RESP_MSG
{
   UCHAR  CtlFlags;        // QMICTL_FLAG_RESPONSE
   UCHAR  TransactionId;
   USHORT QMICTLType;      // QMICTL_GET_CLIENT_ID_RESP
   USHORT Length;
   UCHAR  TLVType;         // QCTLV_TYPE_RESULT_CODE
   USHORT TLVLength;       // 0x0004
   USHORT QMIResult;       // result code
   USHORT QMIError;        // error code
   UCHAR  TLV2Type;        // QCTLV_TYPE_REQUIRED_PARAMETER
   USHORT TLV2Length;      // 2
   UCHAR  QMIType;
   UCHAR  ClientId;
} __attribute__ ((packed)) QMICTL_GET_CLIENT_ID_RESP_MSG, *PQMICTL_GET_CLIENT_ID_RESP_MSG;

typedef struct _QMICTL_RELEASE_CLIENT_ID_REQ_MSG
{
   UCHAR  CtlFlags;        // QMICTL_FLAG_REQUEST
   UCHAR  TransactionId;
   USHORT QMICTLType;      // QMICTL_RELEASE_CLIENT_ID_REQ
   USHORT Length;
   UCHAR  TLVType;         // QCTLV_TYPE_REQUIRED_PARAMETER
   USHORT TLVLength;       // 0x0002
   UCHAR  QMIType;
   UCHAR  ClientId;
} __attribute__ ((packed)) QMICTL_RELEASE_CLIENT_ID_REQ_MSG, *PQMICTL_RELEASE_CLIENT_ID_REQ_MSG;

typedef struct _QMICTL_RELEASE_CLIENT_ID_RESP_MSG
{
   UCHAR  CtlFlags;        // QMICTL_FLAG_RESPONSE
   UCHAR  TransactionId;
   USHORT QMICTLType;      // QMICTL_RELEASE_CLIENT_ID_RESP
   USHORT Length;
   UCHAR  TLVType;         // QCTLV_TYPE_RESULT_CODE
   USHORT TLVLength;       // 0x0004
   USHORT QMIResult;       // result code
   USHORT QMIError;        // error code
   UCHAR  TLV2Type;        // QCTLV_TYPE_REQUIRED_PARAMETER
   USHORT TLV2Length;      // 2
   UCHAR  QMIType;
   UCHAR  ClientId;
} __attribute__ ((packed)) QMICTL_RELEASE_CLIENT_ID_RESP_MSG, *PQMICTL_RELEASE_CLIENT_ID_RESP_MSG;

typedef struct _QMICTL_REVOKE_CLIENT_ID_IND_MSG
{
   UCHAR  CtlFlags;        // QMICTL_FLAG_INDICATION
   UCHAR  TransactionId;
   USHORT QMICTLType;      // QMICTL_REVOKE_CLIENT_ID_IND
   USHORT Length;
   UCHAR  TLVType;         // QCTLV_TYPE_REQUIRED_PARAMETER
   USHORT TLVLength;       // 0x0002
   UCHAR  QMIType;
   UCHAR  ClientId;
} __attribute__ ((packed)) QMICTL_REVOKE_CLIENT_ID_IND_MSG, *PQMICTL_REVOKE_CLIENT_ID_IND_MSG;

typedef struct _QMICTL_INVALID_CLIENT_ID_IND_MSG
{
   UCHAR  CtlFlags;        // QMICTL_FLAG_INDICATION
   UCHAR  TransactionId;
   USHORT QMICTLType;      // QMICTL_REVOKE_CLIENT_ID_IND
   USHORT Length;
   UCHAR  TLVType;         // QCTLV_TYPE_REQUIRED_PARAMETER
   USHORT TLVLength;       // 0x0002
   UCHAR  QMIType;
   UCHAR  ClientId;
} __attribute__ ((packed)) QMICTL_INVALID_CLIENT_ID_IND_MSG, *PQMICTL_INVALID_CLIENT_ID_IND_MSG;

typedef struct _QMICTL_SET_DATA_FORMAT_REQ_MSG
{
   UCHAR  CtlFlags;        // QMICTL_FLAG_REQUEST
   UCHAR  TransactionId;
   USHORT QMICTLType;      // QMICTL_SET_DATA_FORMAT_REQ
   USHORT Length;
   UCHAR  TLVType;         // QCTLV_TYPE_REQUIRED_PARAMETER
   USHORT TLVLength;       // 1
   UCHAR  DataFormat;      // 0-default; 1-QoS hdr present
} __attribute__ ((packed)) QMICTL_SET_DATA_FORMAT_REQ_MSG, *PQMICTL_SET_DATA_FORMAT_REQ_MSG;

#ifdef QC_IP_MODE
#define SET_DATA_FORMAT_TLV_TYPE_LINK_PROTO 0x10
#define SET_DATA_FORMAT_LINK_PROTO_ETH      0x0001
#define SET_DATA_FORMAT_LINK_PROTO_IP       0x0002
typedef struct _QMICTL_SET_DATA_FORMAT_TLV_LINK_PROT
{
   UCHAR  TLVType;         // Link-Layer Protocol
   USHORT TLVLength;       // 2
   USHORT LinkProt;        // 0x1: ETH; 0x2: IP
} QMICTL_SET_DATA_FORMAT_TLV_LINK_PROT, *PQMICTL_SET_DATA_FORMAT_TLV_LINK_PROT;

#ifdef QCMP_UL_TLP
#define SET_DATA_FORMAT_TLV_TYPE_UL_TLP 0x11
typedef struct _QMICTL_SET_DATA_FORMAT_TLV_UL_TLP
{
   UCHAR  TLVType;         // 0x11, Uplink TLP Setting
   USHORT TLVLength;       // 1
   UCHAR  UlTlpSetting;    // 0x0: Disable; 0x01: Enable
} QMICTL_SET_DATA_FORMAT_TLV_UL_TLP, *PQMICTL_SET_DATA_FORMAT_TLV_UL_TLP;
#endif // QCMP_UL_TLP

#ifdef QCMP_DL_TLP
#define SET_DATA_FORMAT_TLV_TYPE_DL_TLP 0x13
typedef struct _QMICTL_SET_DATA_FORMAT_TLV_DL_TLP
{
   UCHAR  TLVType;         // 0x11, Uplink TLP Setting
   USHORT TLVLength;       // 1
   UCHAR  DlTlpSetting;    // 0x0: Disable; 0x01: Enable
} QMICTL_SET_DATA_FORMAT_TLV_DL_TLP, *PQMICTL_SET_DATA_FORMAT_TLV_DL_TLP;
#endif // QCMP_DL_TLP

#endif // QC_IP_MODE

#ifdef MP_QCQOS_ENABLED
#define SET_DATA_FORMAT_TLV_TYPE_QOS_SETTING 0x12
typedef struct _QMICTL_SET_DATA_FORMAT_TLV_QOS_SETTING
{
   UCHAR  TLVType;       // 0x12, QoS setting
   USHORT TLVLength;     // 1
   UCHAR  QosSetting;    // 0x0: Disable; 0x01: Enable
} QMICTL_SET_DATA_FORMAT_TLV_QOS_SETTING, *PQMICTL_SET_DATA_FORMAT_TLV_QOS_SETTING;
#endif // MP_QCQOS_ENABLED

typedef struct _QMICTL_SET_DATA_FORMAT_RESP_MSG
{
   UCHAR  CtlFlags;        // QMICTL_FLAG_RESPONSE
   UCHAR  TransactionId;
   USHORT QMICTLType;      // QMICTL_SET_DATA_FORMAT_RESP
   USHORT Length;
   UCHAR  TLVType;         // QCTLV_TYPE_RESULT_CODE
   USHORT TLVLength;       // 0x0004
   USHORT QMIResult;       // result code
   USHORT QMIError;        // error code
} __attribute__ ((packed)) QMICTL_SET_DATA_FORMAT_RESP_MSG, *PQMICTL_SET_DATA_FORMAT_RESP_MSG;

typedef struct _QMICTL_SYNC_REQ_MSG
{
   UCHAR  CtlFlags;        // QMICTL_FLAG_REQUEST
   UCHAR  TransactionId;
   USHORT QMICTLType;      // QMICTL_CTL_SYNC_REQ
   USHORT Length;          // 0
} __attribute__ ((packed)) QMICTL_SYNC_REQ_MSG, *PQMICTL_SYNC_REQ_MSG;

typedef struct _QMICTL_SYNC_RESP_MSG
{
   UCHAR  CtlFlags;        // QMICTL_FLAG_RESPONSE
   UCHAR  TransactionId;
   USHORT QMICTLType;      // QMICTL_CTL_SYNC_RESP
   USHORT Length;
   UCHAR  TLVType;         // QCTLV_TYPE_RESULT_CODE
   USHORT TLVLength;       // 0x0004
   USHORT QMIResult;
   USHORT QMIError;
} __attribute__ ((packed)) QMICTL_SYNC_RESP_MSG, *PQMICTL_SYNC_RESP_MSG;

typedef struct _QMICTL_SYNC_IND_MSG
{
   UCHAR  CtlFlags;        // QMICTL_FLAG_INDICATION
   UCHAR  TransactionId;
   USHORT QMICTLType;      // QMICTL_REVOKE_CLIENT_ID_IND
   USHORT Length;
} __attribute__ ((packed)) QMICTL_SYNC_IND_MSG, *PQMICTL_SYNC_IND_MSG;

typedef struct _QMICTL_LIBQMI_PROXY_OPEN_MSG
{
   UCHAR  CtlFlags;        // QMICTL_FLAG_RESPONSE
   UCHAR  TransactionId;
   USHORT QMICTLType;      // QMICTL_SET_DATA_FORMAT_RESP
   USHORT Length;
   UCHAR  TLVType;         // QCTLV_TYPE_RESULT_CODE
   USHORT TLVLength;       // 0x0004
   char device_path[0];       // result code
} __attribute__ ((packed)) QMICTL_LIBQMI_PROXY_OPEN_MSG, *PQMICTL_LIBQMI_PROXY_OPEN_MSG;

typedef struct _QMICTL_MSG
{
   union
   {
      // Message Header
      QCQMICTL_MSG_HDR                             QMICTLMsgHdr;
      QCQMICTL_MSG_HDR_RESP                             QMICTLMsgHdrRsp;

      // QMICTL Message
      QMICTL_SET_INSTANCE_ID_REQ_MSG               SetInstanceIdReq;
      QMICTL_SET_INSTANCE_ID_RESP_MSG              SetInstanceIdRsp;
      QMICTL_GET_VERSION_REQ_MSG                   GetVersionReq;
      QMICTL_GET_VERSION_RESP_MSG                  GetVersionRsp;
      QMICTL_GET_CLIENT_ID_REQ_MSG                 GetClientIdReq;
      QMICTL_GET_CLIENT_ID_RESP_MSG                GetClientIdRsp;
      QMICTL_RELEASE_CLIENT_ID_REQ_MSG             ReleaseClientIdReq;
      QMICTL_RELEASE_CLIENT_ID_RESP_MSG            ReleaseClientIdRsp;
      QMICTL_REVOKE_CLIENT_ID_IND_MSG              RevokeClientIdInd;
      QMICTL_INVALID_CLIENT_ID_IND_MSG             InvalidClientIdInd;
      QMICTL_SET_DATA_FORMAT_REQ_MSG               SetDataFormatReq;
      QMICTL_SET_DATA_FORMAT_RESP_MSG              SetDataFormatRsp;
      QMICTL_SYNC_REQ_MSG                          SyncReq;
      QMICTL_SYNC_RESP_MSG                         SyncRsp;
      QMICTL_SYNC_IND_MSG                          SyncInd;
      QMICTL_LIBQMI_PROXY_OPEN_MSG          LibQmiProxyOpenReq;
   };
} __attribute__ ((packed)) QMICTL_MSG, *PQMICTL_MSG;
#pragma pack(pop)

#endif //QCQCTL_H