/******************************************************************************
  @file    QCQMUX.h

  DESCRIPTION
  This module contains QMI QMUX module.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None.

  ---------------------------------------------------------------------------
  Copyright (c) 2016 - 2023 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


#ifndef QCQMUX_H
#define QCQMUX_H

#include "QCQMI.h"

#pragma pack(push, 1)

#define QMIWDS_SET_EVENT_REPORT_REQ           0x0001
#define QMIWDS_SET_EVENT_REPORT_RESP          0x0001
#define QMIWDS_EVENT_REPORT_IND               0x0001
#define QMIWDS_START_NETWORK_INTERFACE_REQ    0x0020
#define QMIWDS_START_NETWORK_INTERFACE_RESP   0x0020
#define QMIWDS_STOP_NETWORK_INTERFACE_REQ     0x0021
#define QMIWDS_STOP_NETWORK_INTERFACE_RESP    0x0021
#define QMIWDS_GET_PKT_SRVC_STATUS_REQ        0x0022
#define QMIWDS_GET_PKT_SRVC_STATUS_RESP       0x0022
#define QMIWDS_GET_PKT_SRVC_STATUS_IND        0x0022
#define QMIWDS_GET_CURRENT_CHANNEL_RATE_REQ   0x0023
#define QMIWDS_GET_CURRENT_CHANNEL_RATE_RESP  0x0023
#define QMIWDS_GET_PKT_STATISTICS_REQ         0x0024
#define QMIWDS_GET_PKT_STATISTICS_RESP        0x0024
#define QMIWDS_CREATE_PROFILE_REQ             0x0027
#define QMIWDS_CREATE_PROFILE_RESP            0x0027
#define QMIWDS_MODIFY_PROFILE_SETTINGS_REQ    0x0028
#define QMIWDS_MODIFY_PROFILE_SETTINGS_RESP   0x0028
#define QMIWDS_GET_PROFILE_LIST_REQ           0x002A
#define QMIWDS_GET_PROFILE_LIST_RESP          0x002A
#define QMIWDS_GET_PROFILE_SETTINGS_REQ       0x002B
#define QMIWDS_GET_PROFILE_SETTINGS_RESP      0x002B
#define QMIWDS_GET_DEFAULT_SETTINGS_REQ       0x002C
#define QMIWDS_GET_DEFAULT_SETTINGS_RESP      0x002C
#define QMIWDS_GET_RUNTIME_SETTINGS_REQ       0x002D
#define QMIWDS_GET_RUNTIME_SETTINGS_RESP      0x002D
#define QMIWDS_GET_MIP_MODE_REQ               0x002F
#define QMIWDS_GET_MIP_MODE_RESP              0x002F
#define QMIWDS_GET_DATA_BEARER_REQ            0x0037
#define QMIWDS_GET_DATA_BEARER_RESP           0x0037
#define QMIWDS_DUN_CALL_INFO_REQ              0x0038
#define QMIWDS_DUN_CALL_INFO_RESP             0x0038
#define QMIWDS_DUN_CALL_INFO_IND              0x0038
#define QMIWDS_SET_CLIENT_IP_FAMILY_PREF_REQ  0x004D
#define QMIWDS_SET_CLIENT_IP_FAMILY_PREF_RESP 0x004D
#define QMIWDS_SET_AUTO_CONNECT_REQ     0x0051
#define QMIWDS_SET_AUTO_CONNECT_RESP     0x0051
#define QMIWDS_BIND_MUX_DATA_PORT_REQ         0x00A2
#define QMIWDS_BIND_MUX_DATA_PORT_RESP        0x00A2


// Stats masks
#define QWDS_STAT_MASK_TX_PKT_OK 0x00000001
#define QWDS_STAT_MASK_RX_PKT_OK 0x00000002
#define QWDS_STAT_MASK_TX_PKT_ER 0x00000004
#define QWDS_STAT_MASK_RX_PKT_ER 0x00000008
#define QWDS_STAT_MASK_TX_PKT_OF 0x00000010
#define QWDS_STAT_MASK_RX_PKT_OF 0x00000020

// TLV Types for xfer statistics
#define TLV_WDS_TX_GOOD_PKTS      0x10
#define TLV_WDS_RX_GOOD_PKTS      0x11
#define TLV_WDS_TX_ERROR          0x12
#define TLV_WDS_RX_ERROR          0x13
#define TLV_WDS_TX_OVERFLOW       0x14
#define TLV_WDS_RX_OVERFLOW       0x15
#define TLV_WDS_CHANNEL_RATE      0x16
#define TLV_WDS_DATA_BEARER       0x17
#define TLV_WDS_DORMANCY_STATUS   0x18

#define QWDS_PKT_DATA_UNKNOW    0x00
#define QWDS_PKT_DATA_DISCONNECTED    0x01
#define QWDS_PKT_DATA_CONNECTED        0x02
#define QWDS_PKT_DATA_SUSPENDED        0x03
#define QWDS_PKT_DATA_AUTHENTICATING   0x04

#define QMIWDS_ADMIN_SET_DATA_FORMAT_REQ      0x0020
#define QMIWDS_ADMIN_SET_DATA_FORMAT_RESP     0x0020
#define QMIWDS_ADMIN_GET_DATA_FORMAT_REQ      0x0021
#define QMIWDS_ADMIN_GET_DATA_FORMAT_RESP     0x0021
#define QMIWDS_ADMIN_SET_QMAP_SETTINGS_REQ    0x002B
#define QMIWDS_ADMIN_SET_QMAP_SETTINGS_RESP   0x002B
#define QMIWDS_ADMIN_GET_QMAP_SETTINGS_REQ    0x002C
#define QMIWDS_ADMIN_GET_QMAP_SETTINGS_RESP   0x002C
#define QMI_WDA_SET_LOOPBACK_CONFIG_REQ       0x002F
#define QMI_WDA_SET_LOOPBACK_CONFIG_RESP      0x002F
#define QMI_WDA_SET_LOOPBACK_CONFIG_IND       0x002F

#define NETWORK_DESC_ENCODING_OCTET       0x00
#define NETWORK_DESC_ENCODING_EXTPROTOCOL 0x01
#define NETWORK_DESC_ENCODING_7BITASCII   0x02
#define NETWORK_DESC_ENCODING_IA5         0x03
#define NETWORK_DESC_ENCODING_UNICODE     0x04
#define NETWORK_DESC_ENCODING_SHIFTJIS    0x05
#define NETWORK_DESC_ENCODING_KOREAN      0x06
#define NETWORK_DESC_ENCODING_LATINH      0x07
#define NETWORK_DESC_ENCODING_LATIN       0x08
#define NETWORK_DESC_ENCODING_GSM7BIT     0x09
#define NETWORK_DESC_ENCODING_GSMDATA     0x0A
#define NETWORK_DESC_ENCODING_UNKNOWN     0xFF

typedef struct _QMIWDS_ADMIN_SET_DATA_FORMAT
{
   USHORT Type;             // QMUX type 0x0000
   USHORT Length;
} __attribute__ ((packed)) QMIWDS_ADMIN_SET_DATA_FORMAT, *PQMIWDS_ADMIN_SET_DATA_FORMAT;

typedef struct _QMIWDS_ADMIN_SET_DATA_FORMAT_TLV_QOS
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  QOSSetting;
} __attribute__ ((packed)) QMIWDS_ADMIN_SET_DATA_FORMAT_TLV_QOS, *PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV_QOS;

typedef struct _QMIWDS_ADMIN_SET_DATA_FORMAT_TLV
{
   UCHAR  TLVType;
   USHORT TLVLength;
   ULONG  Value;
} __attribute__ ((packed)) QMIWDS_ADMIN_SET_DATA_FORMAT_TLV, *PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV;

typedef struct _QMIWDS_ENDPOINT_TLV
{
   UCHAR  TLVType;
   USHORT TLVLength;
   ULONG  ep_type;
   ULONG  iface_id;
} __attribute__ ((packed)) QMIWDS_ENDPOINT_TLV, *PQMIWDS_ENDPOINT_TLV;

typedef struct _QMIWDS_ADMIN_SET_DATA_FORMAT_REQ_MSG
{
   USHORT Type;
   USHORT Length;
    QMIWDS_ADMIN_SET_DATA_FORMAT_TLV_QOS QosDataFormatTlv;
    QMIWDS_ADMIN_SET_DATA_FORMAT_TLV UnderlyingLinkLayerProtocolTlv;
    QMIWDS_ADMIN_SET_DATA_FORMAT_TLV UplinkDataAggregationProtocolTlv;
    QMIWDS_ADMIN_SET_DATA_FORMAT_TLV DownlinkDataAggregationProtocolTlv;
    QMIWDS_ADMIN_SET_DATA_FORMAT_TLV DownlinkDataAggregationMaxDatagramsTlv;
    QMIWDS_ADMIN_SET_DATA_FORMAT_TLV DownlinkDataAggregationMaxSizeTlv;
    QMIWDS_ENDPOINT_TLV epTlv;
#ifdef QUECTEL_UL_DATA_AGG
    QMIWDS_ADMIN_SET_DATA_FORMAT_TLV DlMinimumPassingTlv;
    QMIWDS_ADMIN_SET_DATA_FORMAT_TLV UplinkDataAggregationMaxDatagramsTlv;
    QMIWDS_ADMIN_SET_DATA_FORMAT_TLV UplinkDataAggregationMaxSizeTlv;
#endif	
} __attribute__ ((packed)) QMIWDS_ADMIN_SET_DATA_FORMAT_REQ_MSG, *PQMIWDS_ADMIN_SET_DATA_FORMAT_REQ_MSG;

typedef struct _QMI_U8_TLV
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  TLVVaule;
} __attribute__ ((packed)) QMI_U8_TLV, *PQMI_U8_TLV;

typedef struct _QMI_U32_TLV
{
   UCHAR  TLVType;
   USHORT TLVLength;
   ULONG  TLVVaule;
} __attribute__ ((packed)) QMI_U32_TLV, *PQMI_U32_TLV;

typedef struct _QMI_WDA_SET_LOOPBACK_CONFIG_REQ_MSG {
	USHORT Type;
	USHORT Length;
	QMI_U8_TLV loopback_state; //0x01
	QMI_U32_TLV replication_factor; //0x10
} __attribute__ ((packed)) QMI_WDA_SET_LOOPBACK_CONFIG_REQ_MSG, *PQMI_WDA_SET_LOOPBACK_CONFIG_REQ_MSG;

typedef struct _QMI_WDA_SET_LOOPBACK_CONFIG_IND_MSG
{
   USHORT Type;
   USHORT Length;
   QMI_U8_TLV loopback_state; //0x01
   QMI_U32_TLV replication_factor; //0x10
} __attribute__ ((packed)) QMI_WDA_SET_LOOPBACK_CONFIG_IND_MSG, *PQMI_WDA_SET_LOOPBACK_CONFIG_IND_MSG;

#if 0
typedef enum _QMI_RETURN_CODES {
   QMI_SUCCESS = 0,
   QMI_SUCCESS_NOT_COMPLETE,
   QMI_FAILURE
}QMI_RETURN_CODES;

typedef struct _QMIWDS_GET_PKT_SRVC_STATUS_REQ_MSG
{
   USHORT Type;    // 0x0022
   USHORT Length;  // 0x0000
} QMIWDS_GET_PKT_SRVC_STATUS_REQ_MSG, *PQMIWDS_GET_PKT_SRVC_STATUS_REQ_MSG;

typedef struct _QMIWDS_GET_PKT_SRVC_STATUS_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
   UCHAR  TLVType2;
   USHORT TLVLength2;
   UCHAR  ConnectionStatus; // 0x01: QWDS_PKT_DATAC_DISCONNECTED
                            // 0x02: QWDS_PKT_DATA_CONNECTED
                            // 0x03: QWDS_PKT_DATA_SUSPENDED
                            // 0x04: QWDS_PKT_DATA_AUTHENTICATING
} QMIWDS_GET_PKT_SRVC_STATUS_RESP_MSG, *PQMIWDS_GET_PKT_SRVC_STATUS_RESP_MSG;

typedef struct _QMIWDS_GET_PKT_SRVC_STATUS_IND_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  ConnectionStatus; // 0x01: QWDS_PKT_DATAC_DISCONNECTED
                            // 0x02: QWDS_PKT_DATA_CONNECTED
                            // 0x03: QWDS_PKT_DATA_SUSPENDED
   UCHAR  ReconfigRequired; // 0x00: No need to reconfigure
                            // 0x01: Reconfiguration required
} QMIWDS_GET_PKT_SRVC_STATUS_IND_MSG, *PQMIWDS_GET_PKT_SRVC_STATUS_IND_MSG;

typedef struct _WDS_PKT_SRVC_IP_FAMILY_TLV
{
   UCHAR  TLVType;     // 0x12
   USHORT TLVLength;   // 1
   UCHAR  IpFamily;    // IPV4-0x04, IPV6-0x06
} WDS_PKT_SRVC_IP_FAMILY_TLV, *PWDS_PKT_SRVC_IP_FAMILY_TLV;

typedef struct _QMIWDS_DUN_CALL_INFO_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   ULONG  Mask;
   UCHAR  TLV2Type;
   USHORT TLV2Length;
   UCHAR  ReportConnectionStatus;
} QMIWDS_DUN_CALL_INFO_REQ_MSG, *PQMIWDS_DUN_CALL_INFO_REQ_MSG;

typedef struct _QMIWDS_DUN_CALL_INFO_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
} QMIWDS_DUN_CALL_INFO_RESP_MSG, *PQMIWDS_DUN_CALL_INFO_RESP_MSG;

typedef struct _QMIWDS_DUN_CALL_INFO_IND_MSG
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  ConnectionStatus;
} QMIWDS_DUN_CALL_INFO_IND_MSG, *PQMIWDS_DUN_CALL_INFO_IND_MSG;

typedef struct _QMIWDS_GET_CURRENT_CHANNEL_RATE_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0040
   USHORT Length;
} QMIWDS_GET_CURRENT_CHANNEL_RATE_REQ_MSG, *PQMIWDS_GET_CURRENT_CHANNEL_RATE_REQ_MSG;

typedef struct _QMIWDS_GET_CURRENT_CHANNEL_RATE_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0040
   USHORT Length;
   UCHAR  TLVType;          // 0x02
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;        // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT

   UCHAR  TLV2Type;         // 0x01
   USHORT TLV2Length;       // 16
   //ULONG  CallHandle;       // Context corresponding to reported channel
   ULONG  CurrentTxRate;       // bps
   ULONG  CurrentRxRate;       // bps
   ULONG  ServingSystemTxRate; // bps
   ULONG  ServingSystemRxRate; // bps

} QMIWDS_GET_CURRENT_CHANNEL_RATE_RESP_MSG, *PQMIWDS_GET_CURRENT_CHANNEL_RATE_RESP;

#define QWDS_EVENT_REPORT_MASK_RATES 0x01
#define QWDS_EVENT_REPORT_MASK_STATS 0x02

#ifdef QCUSB_MUX_PROTOCOL
#error code not present
#endif // QCUSB_MUX_PROTOCOL

typedef struct _QMIWDS_SET_EVENT_REPORT_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0042
   USHORT Length;

   UCHAR  TLVType;          // 0x10 -- current channel rate indicator
   USHORT TLVLength;        // 1
   UCHAR  Mode;             // 0-do not report; 1-report when rate changes

   UCHAR  TLV2Type;         // 0x11
   USHORT TLV2Length;       // 5
   UCHAR  StatsPeriod;      // seconds between reports; 0-do not report
   ULONG  StatsMask;        //

   UCHAR  TLV3Type;          // 0x12 -- current data bearer indicator
   USHORT TLV3Length;        // 1
   UCHAR  Mode3;             // 0-do not report; 1-report when changes

   UCHAR  TLV4Type;          // 0x13 -- dormancy status indicator
   USHORT TLV4Length;        // 1
   UCHAR  DormancyStatus;    // 0-do not report; 1-report when changes
} QMIWDS_SET_EVENT_REPORT_REQ_MSG, *PQMIWDS_SET_EVENT_REPORT_REQ_MSG;

typedef struct _QMIWDS_SET_EVENT_REPORT_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0042
   USHORT Length;

   UCHAR  TLVType;          // 0x02 result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;        // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_NO_BATTERY
                            // QMI_ERR_FAULT
} QMIWDS_SET_EVENT_REPORT_RESP_MSG, *PQMIWDS_SET_EVENT_REPORT_RESP_MSG;

typedef struct _QMIWDS_EVENT_REPORT_IND_MSG
{
   USHORT Type;             // QMUX type 0x0001
   USHORT Length;
} QMIWDS_EVENT_REPORT_IND_MSG, *PQMIWDS_EVENT_REPORT_IND_MSG;

// PQCTLV_PKT_STATISTICS

typedef struct _QMIWDS_EVENT_REPORT_IND_CHAN_RATE_TLV
{
   UCHAR  Type;
   USHORT Length;  // 8
   ULONG  TxRate;
   ULONG  RxRate;
} QMIWDS_EVENT_REPORT_IND_CHAN_RATE_TLV, *PQMIWDS_EVENT_REPORT_IND_CHAN_RATE_TLV;

#ifdef QCUSB_MUX_PROTOCOL
#error code not present
#endif // QCUSB_MUX_PROTOCOL

typedef struct _QMIWDS_GET_PKT_STATISTICS_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0041
   USHORT Length;
   UCHAR  TLVType;          // 0x01
   USHORT TLVLength;        // 4
   ULONG  StateMask;        // 0x00000001  tx success packets
                            // 0x00000002  rx success packets
                            // 0x00000004  rx packet errors (checksum)
                            // 0x00000008  rx packets dropped (memory)

} QMIWDS_GET_PKT_STATISTICS_REQ_MSG, *PQMIWDS_GET_PKT_STATISTICS_REQ_MSG;

typedef struct _QMIWDS_GET_PKT_STATISTICS_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0041
   USHORT Length;
   UCHAR  TLVType;          // 0x02
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;        // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
} QMIWDS_GET_PKT_STATISTICS_RESP_MSG, *PQMIWDS_GET_PKT_STATISTICS_RESP_MSG;

// optional TLV for stats
typedef struct _QCTLV_PKT_STATISTICS
{
   UCHAR  TLVType;          // see above definitions for TLV types
   USHORT TLVLength;        // 4
   ULONG  Count;
} QCTLV_PKT_STATISTICS, *PQCTLV_PKT_STATISTICS;
#endif

//#ifdef QC_IP_MODE

/*
    â€?Bit 0 â€?Profile identifier
    â€?Bit 1 â€?Profile name
    â€?Bit 2 â€?PDP type
    â€?Bit 3 â€?APN name
    â€?Bit 4 â€?DNS address
    â€?Bit 5 â€?UMTS/GPRS granted QoS
    â€?Bit 6 â€?Username
    â€?Bit 7 â€?Authentication Protocol
    â€?Bit 8 â€?IP address
    â€?Bit 9 â€?Gateway information (address and subnet mask)
    â€?Bit 10 â€?PCSCF address using a PCO flag
    â€?Bit 11 â€?PCSCF server address list
    â€?Bit 12 â€?PCSCF domain name list
    â€?Bit 13 â€?MTU
    â€?Bit 14 â€?Domain name list
    â€?Bit 15 â€?IP family
    â€?Bit 16 â€?IM_CM flag
    â€?Bit 17 â€?Technology name
    â€?Bit 18 â€?Operator reserved PCO
*/
#define QMIWDS_GET_RUNTIME_SETTINGS_MASK_IPV4DNS_ADDR       (1 << 4)
#define QMIWDS_GET_RUNTIME_SETTINGS_MASK_IPV4_ADDR          (1 << 8)
#define QMIWDS_GET_RUNTIME_SETTINGS_MASK_IPV4GATEWAY_ADDR   (1 << 9)
#define QMIWDS_GET_RUNTIME_SETTINGS_MASK_MTU                (1 << 13)
#define QMIWDS_GET_RUNTIME_SETTINGS_MASK_PCSCF_SV_ADDR      (1 << 11)
#define QMIWDS_GET_RUNTIME_SETTINGS_MASK_PCSCF_DOM_NAME     (1 << 14)

typedef struct _QMIWDS_GET_RUNTIME_SETTINGS_REQ_MSG
{
   USHORT Type;            // QMIWDS_GET_RUNTIME_SETTINGS_REQ
   USHORT Length;
   UCHAR  TLVType;         // 0x10
   USHORT TLVLength;       // 0x0004
   ULONG  Mask;            // mask, bit 8: IP addr -- 0x0100
} __attribute__ ((packed)) QMIWDS_GET_RUNTIME_SETTINGS_REQ_MSG, *PQMIWDS_GET_RUNTIME_SETTINGS_REQ_MSG;

typedef struct _QMIWDS_BIND_MUX_DATA_PORT_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   ULONG  ep_type;
   ULONG  iface_id;
   UCHAR  TLV2Type;
   USHORT TLV2Length;
   UCHAR  MuxId;
   UCHAR  TLV3Type;
   USHORT TLV3Length;
   ULONG  client_type;
} __attribute__ ((packed)) QMIWDS_BIND_MUX_DATA_PORT_REQ_MSG, *PQMIWDS_BIND_MUX_DATA_PORT_REQ_MSG;

#define QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV4PRIMARYDNS 0x15
#define QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV4SECONDARYDNS 0x16
#define QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV4 0x1E
#define QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV4GATEWAY 0x20
#define QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV4SUBNET 0x21

#define QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV6             0x25
#define QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV6GATEWAY      0x26
#define QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV6PRIMARYDNS   0x27
#define QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV6SECONDARYDNS 0x28
#define QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_MTU              0x29

typedef struct _QMIWDS_GET_RUNTIME_SETTINGS_TLV_MTU
{
   UCHAR  TLVType;         // QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_MTU
   USHORT TLVLength;       // 4
   ULONG  Mtu;             // MTU
} __attribute__ ((packed)) QMIWDS_GET_RUNTIME_SETTINGS_TLV_MTU, *PQMIWDS_GET_RUNTIME_SETTINGS_TLV_MTU;

typedef struct _QMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV4_ADDR
{
   UCHAR  TLVType;         // QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV4
   USHORT TLVLength;       // 4
   ULONG  IPV4Address;     // address
} __attribute__ ((packed)) QMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV4_ADDR, *PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV4_ADDR;

typedef struct _QMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV6_ADDR
{
   UCHAR  TLVType;         // QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV6
   USHORT TLVLength;       // 16
   UCHAR  IPV6Address[16]; // address
   UCHAR  PrefixLength;    // prefix length
} __attribute__ ((packed)) QMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV6_ADDR, *PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV6_ADDR;

typedef struct _QMIWDS_GET_RUNNING_SETTINGS_PCSCF_IPV6_ADDR
{
	UCHAR TLVType;
	USHORT TLVLength;
	UCHAR PCSCFNumber;
} __attribute__ ((packed)) QMIWDS_GET_RUNNING_SETTINGS_PCSCF_IPV6_ADDR, *PQMIWDS_GET_RUNNING_SETTINGS_PCSCF_IPV6_ADDR;

typedef struct _QMIWDS_GET_RUNNING_SETTINGS_PCSCF_IPV4_ADDR
{
	UCHAR TLVType;
	USHORT TLVLength;
	UCHAR PCSCFNumber;
} __attribute__ ((packed)) QMIWDS_GET_RUNNING_SETTINGS_PCSCF_IPV4_ADDR, *PQMIWDS_GET_RUNNING_SETTINGS_PCSCF_IPV4_ADDR;

typedef struct _QMIWDS_GET_RUNTIME_SETTINGS_RESP_MSG
{
   USHORT Type;            // QMIWDS_GET_RUNTIME_SETTINGS_RESP
   USHORT Length;
   UCHAR  TLVType;         // QCTLV_TYPE_RESULT_CODE
   USHORT TLVLength;       // 0x0004
   USHORT QMUXResult;      // result code
   USHORT QMUXError;       // error code
} __attribute__ ((packed)) QMIWDS_GET_RUNTIME_SETTINGS_RESP_MSG, *PQMIWDS_GET_RUNTIME_SETTINGS_RESP_MSG;

//#endif // QC_IP_MODE

typedef struct _QMIWDS_IP_FAMILY_TLV
{
   UCHAR  TLVType;          // 0x12
   USHORT TLVLength;        // 1
   UCHAR  IpFamily;         // IPV4-0x04, IPV6-0x06
} __attribute__ ((packed)) QMIWDS_IP_FAMILY_TLV, *PQMIWDS_IP_FAMILY_TLV;

typedef struct _QMIWDS_PKT_SRVC_TLV
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  ConnectionStatus;
   UCHAR  ReconfigReqd;
} __attribute__ ((packed)) QMIWDS_PKT_SRVC_TLV, *PQMIWDS_PKT_SRVC_TLV;

typedef struct _QMIWDS_CALL_END_REASON_TLV
{
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT CallEndReason;
} __attribute__ ((packed)) QMIWDS_CALL_END_REASON_TLV, *PQMIWDS_CALL_END_REASON_TLV;

typedef struct _QMIWDS_CALL_END_REASON_V_TLV
{
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT CallEndReasonType;
   USHORT CallEndReason;
} __attribute__ ((packed)) QMIWDS_CALL_END_REASON_V_TLV, *PQMIWDS_CALL_END_REASON_V_TLV;

typedef struct _QMIWDS_SET_CLIENT_IP_FAMILY_PREF_REQ_MSG
{
   USHORT Type;             // QMUX type 0x004D
   USHORT Length;
   UCHAR  TLVType;          // 0x01
   USHORT TLVLength;        // 1
   UCHAR  IpPreference;     // IPV4-0x04, IPV6-0x06
} __attribute__ ((packed)) QMIWDS_SET_CLIENT_IP_FAMILY_PREF_REQ_MSG, *PQMIWDS_SET_CLIENT_IP_FAMILY_PREF_REQ_MSG;

typedef struct _QMIWDS_SET_CLIENT_IP_FAMILY_PREF_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0037
   USHORT Length;
   UCHAR  TLVType;          // 0x02
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMI_RESULT_SUCCESS, QMI_RESULT_FAILURE
   USHORT QMUXError;        // QMI_ERR_INTERNAL, QMI_ERR_MALFORMED_MSG, QMI_ERR_INVALID_ARG
} __attribute__ ((packed)) QMIWDS_SET_CLIENT_IP_FAMILY_PREF_RESP_MSG, *PQMIWDS_SET_CLIENT_IP_FAMILY_PREF_RESP_MSG;

typedef struct _QMIWDS_SET_AUTO_CONNECT_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0051
   USHORT Length;
   UCHAR  TLVType;          // 0x01
   USHORT TLVLength;        // 1
   UCHAR  autoconnect_setting;     //  0x00 ?C Disabled, 0x01 ?C Enabled, 0x02 ?C Paused (resume on power cycle)
} __attribute__ ((packed)) QMIWDS_SET_AUTO_CONNECT_REQ_MSG, *PQMIWDS_SET_AUTO_CONNECT_REQ_MSG;

#if 0
typedef struct _QMIWDS_GET_MIP_MODE_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0040
   USHORT Length;
} QMIWDS_GET_MIP_MODE_REQ_MSG, *PQMIWDS_GET_MIP_MODE_REQ_MSG;

typedef struct _QMIWDS_GET_MIP_MODE_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0040
   USHORT Length;
   UCHAR  TLVType;          // 0x02
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;        // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT

   UCHAR  TLV2Type;         // 0x01
   USHORT TLV2Length;       // 20
   UCHAR  MipMode;          //
} QMIWDS_GET_MIP_MODE_RESP_MSG, *PQMIWDS_GET_MIP_MODE_RESP_MSG;
#endif

typedef struct _QMIWDS_TECHNOLOGY_PREFERECE
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  TechPreference;
} __attribute__ ((packed)) QMIWDS_TECHNOLOGY_PREFERECE, *PQMIWDS_TECHNOLOGY_PREFERECE;

typedef struct _QMIWDS_PROFILE_IDENTIFIER
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  ProfileIndex;
} __attribute__ ((packed)) QMIWDS_PROFILE_IDENTIFIER, *PQMIWDS_PROFILE_IDENTIFIER;

#if 0
typedef struct _QMIWDS_IPADDRESS
{
   UCHAR  TLVType;
   USHORT TLVLength;
   ULONG  IPv4Address;
}QMIWDS_IPADDRESS, *PQMIWDS_IPADDRESS;

/*
typedef struct _QMIWDS_UMTS_QOS
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  TrafficClass;
   ULONG  MaxUplinkBitRate;
   ULONG  MaxDownlinkBitRate;
   ULONG  GuarUplinkBitRate;
   ULONG  GuarDownlinkBitRate;
   UCHAR  QOSDevOrder;
   ULONG  MAXSDUSize;
   UCHAR  SDUErrorRatio;
   UCHAR  ResidualBerRatio;
   UCHAR  DeliveryErrorSDUs;
   ULONG  TransferDelay;
   ULONG  TrafficHndPri;
}QMIWDS_UMTS_QOS, *PQMIWDS_UMTS_QOS;

typedef struct _QMIWDS_GPRS_QOS
{
   UCHAR  TLVType;
   USHORT TLVLength;
   ULONG  PrecedenceClass;
   ULONG  DelayClass;
   ULONG  ReliabilityClass;
   ULONG  PeekThroClass;
   ULONG  MeanThroClass;
}QMIWDS_GPRS_QOS, *PQMIWDS_GPRS_QOS;
*/
#endif

typedef struct _QMIWDS_PDPCONTEXT
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  pdp_context;
} __attribute__ ((packed)) QMIWDS_PDPCONTEXT, *PQMIWDS_PDPCONTEXT;

typedef struct _QMIWDS_PROFILELIST
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  ProfileList[1024];
} __attribute__ ((packed)) QMIWDS_PROFILELIST, *PQMIWDS_PROFILELIST;

typedef struct _QMIWDS_PROFILENAME
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  ProfileName;
} __attribute__ ((packed)) QMIWDS_PROFILENAME, *PQMIWDS_PROFILENAME;

typedef struct _QMIWDS_PDPTYPE
{
   UCHAR  TLVType;
   USHORT TLVLength;
// 0 ?C PDP-IP (IPv4)
// 1 ?C PDP-PPP
// 2 ?C PDP-IPv6
// 3 ?C PDP-IPv4v6
    UCHAR  PdpType;
} __attribute__ ((packed)) QMIWDS_PDPTYPE, *PQMIWDS_PDPTYPE;

typedef struct _QMIWDS_USERNAME
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  UserName;
} __attribute__ ((packed)) QMIWDS_USERNAME, *PQMIWDS_USERNAME;

typedef struct _QMIWDS_PASSWD
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  Passwd;
} __attribute__ ((packed)) QMIWDS_PASSWD, *PQMIWDS_PASSWD;

typedef struct _QMIWDS_AUTH_PREFERENCE
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  AuthPreference;
} __attribute__ ((packed)) QMIWDS_AUTH_PREFERENCE, *PQMIWDS_AUTH_PREFERENCE;

typedef struct _QMIWDS_IPTYPE
{
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR IPType;
} __attribute__ ((packed)) QMIWDS_IPTYPE, *PQMIWDS_IPTYPE;

typedef struct _QMIWDS_APNNAME
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  ApnName;
} __attribute__ ((packed)) QMIWDS_APNNAME, *PQMIWDS_APNNAME;

typedef struct _QMIWDS_AUTOCONNECT
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  AutoConnect;
} __attribute__ ((packed)) QMIWDS_AUTOCONNECT, *PQMIWDS_AUTOCONNECT;

typedef struct _QMIWDS_START_NETWORK_INTERFACE_REQ_MSG
{
   USHORT Type;
   USHORT Length;
} __attribute__ ((packed)) QMIWDS_START_NETWORK_INTERFACE_REQ_MSG, *PQMIWDS_START_NETWORK_INTERFACE_REQ_MSG;

typedef struct _QMIWDS_CALLENDREASON
{
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT Reason;
}__attribute__ ((packed)) QMIWDS_CALLENDREASON, *PQMIWDS_CALLENDREASON;

typedef struct _QMIWDS_START_NETWORK_INTERFACE_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0040
   USHORT Length;
   UCHAR  TLVType;          // 0x02
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;        // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT

   UCHAR  TLV2Type;         // 0x01
   USHORT TLV2Length;       // 20
   ULONG  Handle;          //
} __attribute__ ((packed)) QMIWDS_START_NETWORK_INTERFACE_RESP_MSG, *PQMIWDS_START_NETWORK_INTERFACE_RESP_MSG;

typedef struct _QMIWDS_STOP_NETWORK_INTERFACE_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   ULONG  Handle;
} __attribute__ ((packed)) QMIWDS_STOP_NETWORK_INTERFACE_REQ_MSG, *PQMIWDS_STOP_NETWORK_INTERFACE_REQ_MSG;

typedef struct _QMIWDS_STOP_NETWORK_INTERFACE_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0040
   USHORT Length;
   UCHAR  TLVType;          // 0x02
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;        // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT

} __attribute__ ((packed)) QMIWDS_STOP_NETWORK_INTERFACE_RESP_MSG, *PQMIWDS_STOP_NETWORK_INTERFACE_RESP_MSG;

typedef struct _QMIWDS_GET_DEFAULT_SETTINGS_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  ProfileType;
} __attribute__ ((packed)) QMIWDS_GET_DEFAULT_SETTINGS_REQ_MSG, *PQMIWDS_GET_DEFAULT_SETTINGS_REQ_MSG;

typedef struct _QMIWDS_GET_DEFAULT_SETTINGS_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
} __attribute__ ((packed)) QMIWDS_GET_DEFAULT_SETTINGS_RESP_MSG, *PQMIWDS_GET_DEFAULT_SETTINGS_RESP_MSG;

typedef struct _QMIWDS_MODIFY_PROFILE_SETTINGS_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  ProfileType;
   UCHAR  ProfileIndex;
} __attribute__ ((packed)) QMIWDS_MODIFY_PROFILE_SETTINGS_REQ_MSG, *PQMIWDS_MODIFY_PROFILE_SETTINGS_REQ_MSG;

typedef struct _QMIWDS_MODIFY_PROFILE_SETTINGS_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
} __attribute__ ((packed)) QMIWDS_MODIFY_PROFILE_SETTINGS_RESP_MSG, *PQMIWDS_MODIFY_PROFILE_SETTINGS_RESP_MSG;

typedef struct _QMIWDS_GET_PROFILE_SETTINGS_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  ProfileType;
   UCHAR  ProfileIndex;
} __attribute__ ((packed)) QMIWDS_GET_PROFILE_SETTINGS_REQ_MSG, *PQMIWDS_GET_PROFILE_SETTINGS_REQ_MSG;

typedef struct _QMIWDS_CREATE_PROFILE_SETTINGS_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  ProfileType;
   UCHAR  TLV2Type; //0x25
   USHORT TLV2Length;
   UCHAR  pdp_context;
} __attribute__ ((packed)) QMIWDS_CREATE_PROFILE_SETTINGS_REQ_MSG, *PQMIWDS_CREATE_PROFILE_SETTINGS_REQ_MSG;

typedef struct _QMIWDS_GET_PROFILE_LIST_REQ_MSG
{
   USHORT Type;
   USHORT Length;
} __attribute__ ((packed)) QMIWDS_GET_PROFILE_LIST_REQ_MSG, *PQMIWDS_GET_PROFILE_LIST_REQ_MSG;

typedef struct _QMIWDS_GET_PROFILE_LIST_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  ProfileList[1024];
} __attribute__ ((packed)) QMIWDS_GET_PROFILE_LIST_RESP_MSG, *PQMIWDS_GET_PROFILE_LIST_RESP_MSG;

#if 0
typedef struct _QMIWDS_EVENT_REPORT_IND_DATA_BEARER_TLV
{
   UCHAR  Type;
   USHORT Length;
   UCHAR  DataBearer;
} QMIWDS_EVENT_REPORT_IND_DATA_BEARER_TLV, *PQMIWDS_EVENT_REPORT_IND_DATA_BEARER_TLV;

typedef struct _QMIWDS_EVENT_REPORT_IND_DORMANCY_STATUS_TLV
{
   UCHAR  Type;
   USHORT Length;
   UCHAR  DormancyStatus;
} QMIWDS_EVENT_REPORT_IND_DORMANCY_STATUS_TLV, *PQMIWDS_EVENT_REPORT_IND_DORMANCY_STATUS_TLV;


typedef struct _QMIWDS_GET_DATA_BEARER_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0037
   USHORT Length;
} QMIWDS_GET_DATA_BEARER_REQ_MSG, *PQMIWDS_GET_DATA_BEARER_REQ_MSG;

typedef struct _QMIWDS_GET_DATA_BEARER_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0037
   USHORT Length;
   UCHAR  TLVType;          // 0x02
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;        // QMI_ERR_INTERNAL
                            // QMI_ERR_MALFORMED_MSG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_OUT_OF_CALL
                            // QMI_ERR_INFO_UNAVAILABLE
   UCHAR  TLV2Type;         // 0x01
   USHORT TLV2Length;       //
   UCHAR  Technology;       //
} QMIWDS_GET_DATA_BEARER_RESP_MSG, *PQMIWDS_GET_DATA_BEARER_RESP_MSG;
#endif

// ======================= DMS ==============================
#define QMIDMS_SET_EVENT_REPORT_REQ           0x0001
#define QMIDMS_SET_EVENT_REPORT_RESP          0x0001
#define QMIDMS_EVENT_REPORT_IND               0x0001
#define QMIDMS_GET_DEVICE_CAP_REQ             0x0020
#define QMIDMS_GET_DEVICE_CAP_RESP            0x0020
#define QMIDMS_GET_DEVICE_MFR_REQ             0x0021
#define QMIDMS_GET_DEVICE_MFR_RESP            0x0021
#define QMIDMS_GET_DEVICE_MODEL_ID_REQ        0x0022
#define QMIDMS_GET_DEVICE_MODEL_ID_RESP       0x0022
#define QMIDMS_GET_DEVICE_REV_ID_REQ          0x0023
#define QMIDMS_GET_DEVICE_REV_ID_RESP         0x0023
#define QMIDMS_GET_MSISDN_REQ                 0x0024
#define QMIDMS_GET_MSISDN_RESP                0x0024
#define QMIDMS_GET_DEVICE_SERIAL_NUMBERS_REQ  0x0025
#define QMIDMS_GET_DEVICE_SERIAL_NUMBERS_RESP 0x0025
#define QMIDMS_UIM_SET_PIN_PROTECTION_REQ     0x0027
#define QMIDMS_UIM_SET_PIN_PROTECTION_RESP    0x0027
#define QMIDMS_UIM_VERIFY_PIN_REQ             0x0028
#define QMIDMS_UIM_VERIFY_PIN_RESP            0x0028
#define QMIDMS_UIM_UNBLOCK_PIN_REQ            0x0029
#define QMIDMS_UIM_UNBLOCK_PIN_RESP           0x0029
#define QMIDMS_UIM_CHANGE_PIN_REQ             0x002A
#define QMIDMS_UIM_CHANGE_PIN_RESP            0x002A
#define QMIDMS_UIM_GET_PIN_STATUS_REQ         0x002B
#define QMIDMS_UIM_GET_PIN_STATUS_RESP        0x002B
#define QMIDMS_GET_DEVICE_HARDWARE_REV_REQ    0x002C
#define QMIDMS_GET_DEVICE_HARDWARE_REV_RESP   0x002C
#define QMIDMS_GET_OPERATING_MODE_REQ         0x002D
#define QMIDMS_GET_OPERATING_MODE_RESP        0x002D
#define QMIDMS_SET_OPERATING_MODE_REQ         0x002E
#define QMIDMS_SET_OPERATING_MODE_RESP        0x002E
#define QMIDMS_GET_ACTIVATED_STATUS_REQ       0x0031
#define QMIDMS_GET_ACTIVATED_STATUS_RESP      0x0031
#define QMIDMS_ACTIVATE_AUTOMATIC_REQ         0x0032
#define QMIDMS_ACTIVATE_AUTOMATIC_RESP        0x0032
#define QMIDMS_ACTIVATE_MANUAL_REQ            0x0033
#define QMIDMS_ACTIVATE_MANUAL_RESP           0x0033
#define QMIDMS_UIM_GET_ICCID_REQ              0x003C
#define QMIDMS_UIM_GET_ICCID_RESP             0x003C
#define QMIDMS_UIM_GET_CK_STATUS_REQ          0x0040
#define QMIDMS_UIM_GET_CK_STATUS_RESP         0x0040
#define QMIDMS_UIM_SET_CK_PROTECTION_REQ      0x0041
#define QMIDMS_UIM_SET_CK_PROTECTION_RESP     0x0041
#define QMIDMS_UIM_UNBLOCK_CK_REQ             0x0042
#define QMIDMS_UIM_UNBLOCK_CK_RESP            0x0042
#define QMIDMS_UIM_GET_IMSI_REQ               0x0043
#define QMIDMS_UIM_GET_IMSI_RESP              0x0043
#define QMIDMS_UIM_GET_STATE_REQ              0x0044
#define QMIDMS_UIM_GET_STATE_RESP             0x0044
#define QMIDMS_GET_BAND_CAP_REQ               0x0045
#define QMIDMS_GET_BAND_CAP_RESP              0x0045

#if 0
typedef struct _QMIDMS_GET_DEVICE_MFR_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
} QMIDMS_GET_DEVICE_MFR_REQ_MSG, *PQMIDMS_GET_DEVICE_MFR_REQ_MSG;

typedef struct _QMIDMS_GET_DEVICE_MFR_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;      // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;       // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
   UCHAR  TLV2Type;         // 0x01 - required parameter
   USHORT TLV2Length;       // length of the mfr string
   UCHAR  DeviceManufacturer; // first byte of string
} QMIDMS_GET_DEVICE_MFR_RESP_MSG, *PQMIDMS_GET_DEVICE_MFR_RESP_MSG;

typedef struct _QMIDMS_GET_DEVICE_MODEL_ID_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0004
   USHORT Length;
} QMIDMS_GET_DEVICE_MODEL_ID_REQ_MSG, *PQMIDMS_GET_DEVICE_MODEL_ID_REQ_MSG;

typedef struct _QMIDMS_GET_DEVICE_MODEL_ID_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0004
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;      // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;       // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
   UCHAR  TLV2Type;         // 0x01 - required parameter
   USHORT TLV2Length;       // length of the modem id string
   UCHAR  DeviceModelID;    // device model id
} QMIDMS_GET_DEVICE_MODEL_ID_RESP_MSG, *PQMIDMS_GET_DEVICE_MODEL_ID_RESP_MSG;
#endif

typedef struct _QMIDMS_GET_DEVICE_REV_ID_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0005
   USHORT Length;
} __attribute__ ((packed)) QMIDMS_GET_DEVICE_REV_ID_REQ_MSG, *PQMIDMS_GET_DEVICE_REV_ID_REQ_MSG;

typedef struct _DEVICE_REV_ID
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  RevisionID;
} __attribute__ ((packed)) DEVICE_REV_ID, *PDEVICE_REV_ID;

#if 0
typedef struct _QMIDMS_GET_DEVICE_REV_ID_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0023
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;      // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;       // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
} QMIDMS_GET_DEVICE_REV_ID_RESP_MSG, *PQMIDMS_GET_DEVICE_REV_ID_RESP_MSG;

typedef struct _QMIDMS_GET_MSISDN_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0024
   USHORT Length;
} QMIDMS_GET_MSISDN_REQ_MSG, *PQMIDMS_GET_MSISDN_REQ_MSG;

typedef struct _QCTLV_DEVICE_VOICE_NUMBERS
{
   UCHAR  TLVType;            // as defined above
   USHORT TLVLength;          // 4/7/7
   UCHAR  VoideNumberString; // ESN, IMEI, or MEID

} QCTLV_DEVICE_VOICE_NUMBERS, *PQCTLV_DEVICE_VOICE_NUMBERS;


typedef struct _QMIDMS_GET_MSISDN_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0024
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;        // QMI_ERR_INVALID_ARG
} QMIDMS_GET_MSISDN_RESP_MSG, *PQMIDMS_GET_MSISDN_RESP_MSG;
#endif

typedef struct _QMIDMS_UIM_GET_IMSI_REQ_MSG
{
   USHORT Type;
   USHORT Length;
} __attribute__ ((packed)) QMIDMS_UIM_GET_IMSI_REQ_MSG, *PQMIDMS_UIM_GET_IMSI_REQ_MSG;

typedef struct _QMIDMS_UIM_GET_IMSI_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
   UCHAR  TLV2Type;
   USHORT TLV2Length;
   UCHAR IMSI;
} __attribute__ ((packed)) QMIDMS_UIM_GET_IMSI_RESP_MSG, *PQMIDMS_UIM_GET_IMSI_RESP_MSG;

#if 0
typedef struct _QMIDMS_GET_DEVICE_SERIAL_NUMBERS_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0007
   USHORT Length;
} QMIDMS_GET_DEVICE_SERIAL_NUMBERS_REQ_MSG, *PQMIDMS_GET_DEVICE_SERIAL_NUMBERS_REQ_MSG;

#define QCTLV_TYPE_SER_NUM_ESN  0x10
#define QCTLV_TYPE_SER_NUM_IMEI 0x11
#define QCTLV_TYPE_SER_NUM_MEID 0x12

typedef struct _QCTLV_DEVICE_SERIAL_NUMBER
{
   UCHAR  TLVType;            // as defined above
   USHORT TLVLength;          // 4/7/7
   UCHAR  SerialNumberString; // ESN, IMEI, or MEID

} QCTLV_DEVICE_SERIAL_NUMBER, *PQCTLV_DEVICE_SERIAL_NUMBER;

typedef struct _QMIDMS_GET_DEVICE_SERIAL_NUMBERS_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0007
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;      // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;       // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
  // followed by optional TLV
} QMIDMS_GET_DEVICE_SERIAL_NUMBERS_RESP_MSG, *PQMIDMS_GET_DEVICE_SERIAL_NUMBERS_RESP;

typedef struct _QMIDMS_GET_DMS_BAND_CAP
{
   USHORT  Type;
   USHORT  Length;
} QMIDMS_GET_BAND_CAP_REQ_MSG, *PQMIDMS_GET_BAND_CAP_REQ_MSG;

typedef struct _QMIDMS_GET_BAND_CAP_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;        // QMI_ERR_NONE
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_MALFORMED_MSG
                            // QMI_ERR_NO_MEMORY

   UCHAR  TLV2Type;         // 0x01
   USHORT TLV2Length;       // 2
   ULONG64 BandCap;
} QMIDMS_GET_BAND_CAP_RESP_MSG, *PQMIDMS_GET_BAND_CAP_RESP;

typedef struct _QMIDMS_GET_DEVICE_CAP_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0002
   USHORT Length;
} QMIDMS_GET_DEVICE_CAP_REQ_MSG, *PQMIDMS_GET_DEVICE_CAP_REQ_MSG;

typedef struct _QMIDMS_GET_DEVICE_CAP_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0002
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMUX_RESULT_SUCCESS
                            // QMUX_RESULT_FAILURE
   USHORT QMUXError;        // QMUX_ERR_INVALID_ARG
                            // QMUX_ERR_NO_MEMORY
                            // QMUX_ERR_INTERNAL
                            // QMUX_ERR_FAULT
   UCHAR  TLV2Type;         // 0x01
   USHORT TLV2Length;       // 2

   ULONG  MaxTxChannelRate;
   ULONG  MaxRxChannelRate;
   UCHAR  VoiceCap;
   UCHAR  SimCap;

   UCHAR  RadioIfListCnt;   // #elements in radio interface list
   UCHAR  RadioIfList;      // N 1-byte elements
} QMIDMS_GET_DEVICE_CAP_RESP_MSG, *PQMIDMS_GET_DEVICE_CAP_RESP_MSG;

typedef struct _QMIDMS_GET_ACTIVATED_STATUS_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0002
   USHORT Length;
} QMIDMS_GET_ACTIVATED_STATUS_REQ_MSG, *PQMIDMS_GET_ACTIVATES_STATUD_REQ_MSG;

typedef struct _QMIDMS_GET_ACTIVATED_STATUS_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0002
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMUX_RESULT_SUCCESS
                            // QMUX_RESULT_FAILURE
   USHORT QMUXError;        // QMUX_ERR_INVALID_ARG
                            // QMUX_ERR_NO_MEMORY
                            // QMUX_ERR_INTERNAL
                            // QMUX_ERR_FAULT
   UCHAR  TLV2Type;         // 0x01
   USHORT TLV2Length;       // 2

   USHORT ActivatedStatus;
} QMIDMS_GET_ACTIVATED_STATUS_RESP_MSG, *PQMIDMS_GET_ACTIVATED_STATUS_RESP_MSG;

typedef struct _QMIDMS_GET_OPERATING_MODE_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0002
   USHORT Length;
} QMIDMS_GET_OPERATING_MODE_REQ_MSG, *PQMIDMS_GET_OPERATING_MODE_REQ_MSG;

typedef struct _OFFLINE_REASON
{
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT OfflineReason;
} OFFLINE_REASON, *POFFLINE_REASON;

typedef struct _HARDWARE_RESTRICTED_MODE
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  HardwareControlledMode;
} HARDWARE_RESTRICTED_MODE, *PHARDWARE_RESTRICTED_MODE;

typedef struct _QMIDMS_GET_OPERATING_MODE_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0002
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMUX_RESULT_SUCCESS
                            // QMUX_RESULT_FAILURE
   USHORT QMUXError;        // QMUX_ERR_INVALID_ARG
                            // QMUX_ERR_NO_MEMORY
                            // QMUX_ERR_INTERNAL
                            // QMUX_ERR_FAULT
   UCHAR  TLV2Type;         // 0x01
   USHORT TLV2Length;       // 2

   UCHAR  OperatingMode;
} QMIDMS_GET_OPERATING_MODE_RESP_MSG, *PQMIDMS_GET_OPERATING_MODE_RESP_MSG;

typedef struct _QMIDMS_UIM_GET_ICCID_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0024
   USHORT Length;
} QMIDMS_UIM_GET_ICCID_REQ_MSG, *PQMIDMS_UIM_GET_ICCID_REQ_MSG;

typedef struct _QMIDMS_UIM_GET_ICCID_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0024
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;        // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
   UCHAR  TLV2Type;         // 0x01 - required parameter
   USHORT TLV2Length;       // var
   UCHAR  ICCID;      // String of voice number
} QMIDMS_UIM_GET_ICCID_RESP_MSG, *PQMIDMS_UIM_GET_ICCID_RESP_MSG;
#endif

typedef struct _QMIDMS_SET_OPERATING_MODE_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0002
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   UCHAR  OperatingMode;
} __attribute__ ((packed)) QMIDMS_SET_OPERATING_MODE_REQ_MSG, *PQMIDMS_SET_OPERATING_MODE_REQ_MSG;

typedef struct _QMIDMS_SET_OPERATING_MODE_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0002
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMUX_RESULT_SUCCESS
                            // QMUX_RESULT_FAILURE
   USHORT QMUXError;        // QMUX_ERR_INVALID_ARG
                            // QMUX_ERR_NO_MEMORY
                            // QMUX_ERR_INTERNAL
                            // QMUX_ERR_FAULT
} __attribute__ ((packed)) QMIDMS_SET_OPERATING_MODE_RESP_MSG, *PQMIDMS_SET_OPERATING_MODE_RESP_MSG;

#if 0
typedef struct _QMIDMS_ACTIVATE_AUTOMATIC_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0024
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        //
   UCHAR  ActivateCodelen;
   UCHAR  ActivateCode;
} QMIDMS_ACTIVATE_AUTOMATIC_REQ_MSG, *PQMIDMS_ACTIVATE_AUTOMATIC_REQ_MSG;

typedef struct _QMIDMS_ACTIVATE_AUTOMATIC_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0024
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;        // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
} QMIDMS_ACTIVATE_AUTOMATIC_RESP_MSG, *PQMIDMS_ACTIVATE_AUTOMATIC_RESP_MSG;


typedef struct _SPC_MSG
{
   UCHAR SPC[6];
   USHORT SID;
} SPC_MSG, *PSPC_MSG;

typedef struct _MDN_MSG
{
   UCHAR MDNLEN;
   UCHAR MDN;
} MDN_MSG, *PMDN_MSG;

typedef struct _MIN_MSG
{
   UCHAR MINLEN;
   UCHAR MIN;
} MIN_MSG, *PMIN_MSG;

typedef struct _PRL_MSG
{
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        //
   USHORT PRLLEN;
   UCHAR PRL;
} PRL_MSG, *PPRL_MSG;

typedef struct _MN_HA_KEY_MSG
{
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        //
   UCHAR MN_HA_KEY_LEN;
   UCHAR MN_HA_KEY;
} MN_HA_KEY_MSG, *PMN_HA_KEY_MSG;

typedef struct _MN_AAA_KEY_MSG
{
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        //
   UCHAR MN_AAA_KEY_LEN;
   UCHAR MN_AAA_KEY;
} MN_AAA_KEY_MSG, *PMN_AAA_KEY_MSG;

typedef struct _QMIDMS_ACTIVATE_MANUAL_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0024
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        //
   UCHAR  Value;
} QMIDMS_ACTIVATE_MANUAL_REQ_MSG, *PQMIDMS_ACTIVATE_MANUAL_REQ_MSG;

typedef struct _QMIDMS_ACTIVATE_MANUAL_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0024
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;        // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
} QMIDMS_ACTIVATE_MANUAL_RESP_MSG, *PQMIDMS_ACTIVATE_MANUAL_RESP_MSG;
#endif

typedef struct _QMIDMS_UIM_GET_STATE_REQ_MSG
{
   USHORT Type;
   USHORT Length;
} __attribute__ ((packed)) QMIDMS_UIM_GET_STATE_REQ_MSG, *PQMIDMS_UIM_GET_STATE_REQ_MSG;

typedef struct _QMIDMS_UIM_GET_STATE_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
   UCHAR  TLV2Type;
   USHORT TLV2Length;
   UCHAR  UIMState;
} __attribute__ ((packed)) QMIDMS_UIM_GET_STATE_RESP_MSG, *PQMIDMS_UIM_GET_STATE_RESP_MSG;

typedef struct _QMIDMS_UIM_GET_PIN_STATUS_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0024
   USHORT Length;
} __attribute__ ((packed)) QMIDMS_UIM_GET_PIN_STATUS_REQ_MSG, *PQMIDMS_UIM_GET_PIN_STATUS_REQ_MSG;

typedef struct _QMIDMS_UIM_PIN_STATUS
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  PINStatus;
   UCHAR  PINVerifyRetriesLeft;
   UCHAR  PINUnblockRetriesLeft;
} __attribute__ ((packed)) QMIDMS_UIM_PIN_STATUS, *PQMIDMS_UIM_PIN_STATUS;

#define QMI_PIN_STATUS_NOT_INIT      0
#define QMI_PIN_STATUS_NOT_VERIF     1
#define QMI_PIN_STATUS_VERIFIED      2
#define QMI_PIN_STATUS_DISABLED      3
#define QMI_PIN_STATUS_BLOCKED       4
#define QMI_PIN_STATUS_PERM_BLOCKED  5
#define QMI_PIN_STATUS_UNBLOCKED     6
#define QMI_PIN_STATUS_CHANGED       7


typedef struct _QMIDMS_UIM_GET_PIN_STATUS_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0024
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;        // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
   UCHAR PinStatus;
} __attribute__ ((packed)) QMIDMS_UIM_GET_PIN_STATUS_RESP_MSG, *PQMIDMS_UIM_GET_PIN_STATUS_RESP_MSG;

#if 0
typedef struct _QMIDMS_UIM_GET_CK_STATUS_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  Facility;
} QMIDMS_UIM_GET_CK_STATUS_REQ_MSG, *PQMIDMS_UIM_GET_CK_STATUS_REQ_MSG;


typedef struct _QMIDMS_UIM_CK_STATUS
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  FacilityStatus;
   UCHAR  FacilityVerifyRetriesLeft;
   UCHAR  FacilityUnblockRetriesLeft;
} QMIDMS_UIM_CK_STATUS, *PQMIDMS_UIM_CK_STATUS;

typedef struct _QMIDMS_UIM_CK_OPERATION_STATUS
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  OperationBlocking;
} QMIDMS_UIM_CK_OPERATION_STATUS, *PQMIDMS_UIM_CK_OPERATION_STATUS;

typedef struct _QMIDMS_UIM_GET_CK_STATUS_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
   UCHAR  CkStatus;
} QMIDMS_UIM_GET_CK_STATUS_RESP_MSG, *PQMIDMS_UIM_GET_CK_STATUS_RESP_MSG;
#endif

typedef struct _QMIDMS_UIM_VERIFY_PIN_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0024
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   UCHAR  PINID;
   UCHAR  PINLen;
   UCHAR  PINValue;
} __attribute__ ((packed)) QMIDMS_UIM_VERIFY_PIN_REQ_MSG, *PQMIDMS_UIM_VERIFY_PIN_REQ_MSG;

typedef struct _QMIDMS_UIM_VERIFY_PIN_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0024
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;        // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
   UCHAR  TLV2Type;
   USHORT TLV2Length;
   UCHAR  PINVerifyRetriesLeft;
   UCHAR  PINUnblockRetriesLeft;
} __attribute__ ((packed)) QMIDMS_UIM_VERIFY_PIN_RESP_MSG, *PQMIDMS_UIM_VERIFY_PIN_RESP_MSG;

#if 0
typedef struct _QMIDMS_UIM_SET_PIN_PROTECTION_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0024
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   UCHAR  PINID;
   UCHAR  ProtectionSetting;
   UCHAR  PINLen;
   UCHAR  PINValue;
} QMIDMS_UIM_SET_PIN_PROTECTION_REQ_MSG, *PQMIDMS_UIM_SET_PIN_PROTECTION_REQ_MSG;

typedef struct _QMIDMS_UIM_SET_PIN_PROTECTION_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0024
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;        // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
   UCHAR  TLV2Type;
   USHORT TLV2Length;
   UCHAR  PINVerifyRetriesLeft;
   UCHAR  PINUnblockRetriesLeft;
} QMIDMS_UIM_SET_PIN_PROTECTION_RESP_MSG, *PQMIDMS_UIM_SET_PIN_PROTECTION_RESP_MSG;

typedef struct _QMIDMS_UIM_SET_CK_PROTECTION_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  Facility;
   UCHAR  FacilityState;
   UCHAR  FacliltyLen;
   UCHAR  FacliltyValue;
} QMIDMS_UIM_SET_CK_PROTECTION_REQ_MSG, *PQMIDMS_UIM_SET_CK_PROTECTION_REQ_MSG;

typedef struct _QMIDMS_UIM_SET_CK_PROTECTION_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
   UCHAR  TLV2Type;
   USHORT TLV2Length;
   UCHAR  FacilityRetriesLeft;
} QMIDMS_UIM_SET_CK_PROTECTION_RESP_MSG, *PQMIDMS_UIM_SET_CK_PROTECTION_RESP_MSG;


typedef struct _UIM_PIN
{
   UCHAR  PinLength;
   UCHAR  PinValue;
} UIM_PIN, *PUIM_PIN;

typedef struct _QMIDMS_UIM_CHANGE_PIN_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0024
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   UCHAR  PINID;
   UCHAR  PinDetails;
} QMIDMS_UIM_CHANGE_PIN_REQ_MSG, *PQMIDMS_UIM_CHANGE_PIN_REQ_MSG;

typedef struct QMIDMS_UIM_CHANGE_PIN_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0024
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;        // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
   UCHAR  TLV2Type;
   USHORT TLV2Length;
   UCHAR  PINVerifyRetriesLeft;
   UCHAR  PINUnblockRetriesLeft;
} QMIDMS_UIM_CHANGE_PIN_RESP_MSG, *PQMIDMS_UIM_CHANGE_PIN_RESP_MSG;

typedef struct _UIM_PUK
{
   UCHAR  PukLength;
   UCHAR  PukValue;
} UIM_PUK, *PUIM_PUK;

typedef struct _QMIDMS_UIM_UNBLOCK_PIN_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0024
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   UCHAR  PINID;
   UCHAR  PinDetails;
} QMIDMS_UIM_UNBLOCK_PIN_REQ_MSG, *PQMIDMS_UIM_BLOCK_PIN_REQ_MSG;

typedef struct QMIDMS_UIM_UNBLOCK_PIN_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0024
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;        // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
   UCHAR  TLV2Type;
   USHORT TLV2Length;
   UCHAR  PINVerifyRetriesLeft;
   UCHAR  PINUnblockRetriesLeft;
} QMIDMS_UIM_UNBLOCK_PIN_RESP_MSG, *PQMIDMS_UIM_UNBLOCK_PIN_RESP_MSG;

typedef struct _QMIDMS_UIM_UNBLOCK_CK_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  Facility;
   UCHAR  FacliltyUnblockLen;
   UCHAR  FacliltyUnblockValue;
} QMIDMS_UIM_UNBLOCK_CK_REQ_MSG, *PQMIDMS_UIM_BLOCK_CK_REQ_MSG;

typedef struct QMIDMS_UIM_UNBLOCK_CK_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
   UCHAR  TLV2Type;
   USHORT TLV2Length;
   UCHAR  FacilityUnblockRetriesLeft;
} QMIDMS_UIM_UNBLOCK_CK_RESP_MSG, *PQMIDMS_UIM_UNBLOCK_CK_RESP_MSG;

typedef struct _QMIDMS_SET_EVENT_REPORT_REQ_MSG
{
   USHORT Type;
   USHORT Length;
} QMIDMS_SET_EVENT_REPORT_REQ_MSG, *PQMIDMS_SET_EVENT_REPORT_REQ_MSG;

typedef struct _QMIDMS_SET_EVENT_REPORT_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;      // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;       // QMI_ERR_INVALID_ARG
} QMIDMS_SET_EVENT_REPORT_RESP_MSG, *PQMIDMS_SET_EVENT_REPORT_RESP_MSG;

typedef struct _PIN_STATUS
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  ReportPinState;
} PIN_STATUS, *PPIN_STATUS;

typedef struct _POWER_STATUS
{
   UCHAR TLVType;
   USHORT TLVLength;
   UCHAR PowerStatus;
   UCHAR BatteryLvl;
} POWER_STATUS, *PPOWER_STATUS;

typedef struct _ACTIVATION_STATE
{
   UCHAR TLVType;
   USHORT TLVLength;
   USHORT ActivationState;
} ACTIVATION_STATE, *PACTIVATION_STATE;

typedef struct _ACTIVATION_STATE_REQ
{
   UCHAR TLVType;
   USHORT TLVLength;
   UCHAR ActivationState;
} ACTIVATION_STATE_REQ, *PACTIVATION_STATE_REQ;

typedef struct _OPERATING_MODE
{
   UCHAR TLVType;
   USHORT TLVLength;
   UCHAR OperatingMode;
} OPERATING_MODE, *POPERATING_MODE;

typedef struct _UIM_STATE
{
   UCHAR TLVType;
   USHORT TLVLength;
   UCHAR UIMState;
} UIM_STATE, *PUIM_STATE;

typedef struct _WIRELESS_DISABLE_STATE
{
   UCHAR TLVType;
   USHORT TLVLength;
   UCHAR WirelessDisableState;
} WIRELESS_DISABLE_STATE, *PWIRELESS_DISABLE_STATE;

typedef struct _QMIDMS_EVENT_REPORT_IND_MSG
{
   USHORT Type;
   USHORT Length;
} QMIDMS_EVENT_REPORT_IND_MSG, *PQMIDMS_EVENT_REPORT_IND_MSG;
#endif

// ============================ END OF DMS ===============================

// ======================= QOS ==============================
typedef struct _MPIOC_DEV_INFO MPIOC_DEV_INFO, *PMPIOC_DEV_INFO;

#define QMI_QOS_SET_EVENT_REPORT_REQ        0x0001
#define QMI_QOS_SET_EVENT_REPORT_RESP       0x0001
#define QMI_QOS_SET_EVENT_REPORT_IND        0x0001
#define QMI_QOS_BIND_DATA_PORT_REQ          0x002B
#define QMI_QOS_BIND_DATA_PORT_RESP         0x002B
#define QMI_QOS_INDICATION_REGISTER_REQ	    0x002F
#define QMI_QOS_INDICATION_REGISTER_RESP    0x002F
#define QMI_QOS_GLOBAL_QOS_FLOW_IND	        0x0031
#define QMI_QOS_GET_QOS_INFO_REQ            0x0033
#define QMI_QOS_GET_QOS_INFO_RESP           0x0033


#if 1
typedef struct _QMI_QOS_SET_EVENT_REPORT_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0001
   USHORT Length;
   // UCHAR  TLVType;          // 0x01 - physical link state
   // USHORT TLVLength;        // 1
   // UCHAR  PhyLinkStatusRpt; // 0-enable; 1-disable
   UCHAR  TLVType2;         // 0x02 = global flow reporting
   USHORT TLVLength2;       // 1
   UCHAR  GlobalFlowRpt;    // 1-enable; 0-disable
} QMI_QOS_SET_EVENT_REPORT_REQ_MSG, *PQMI_QOS_SET_EVENT_REPORT_REQ_MSG;

typedef struct _QMI_QOS_SET_EVENT_REPORT_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0010
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;       // QMUX_RESULT_SUCCESS
                            // QMUX_RESULT_FAILURE
   USHORT QMUXError;        // QMUX_ERR_INVALID_ARG
                            // QMUX_ERR_NO_MEMORY
                            // QMUX_ERR_INTERNAL
                            // QMUX_ERR_FAULT
} QMI_QOS_SET_EVENT_REPORT_RESP_MSG, *PQMI_QOS_SET_EVENT_REPORT_RESP_MSG;

typedef struct _QMI_QOS_SET_EVENT_REPORT_IND_MSG
{
   USHORT Type;             // QMUX type 0x0001
   USHORT Length;
   UCHAR  TLVs;
} QMI_QOS_SET_EVENT_REPORT_IND_MSG, *PQMI_QOS_SET_EVENT_REPORT_IND_MSG;


typedef struct _QMI_QOS_BIND_DATA_PORT_TLV_EP_ID
{
	UCHAR	TLVType;		//0x10
	USHORT	TLVLength;
	ULONG	ep_type;
	ULONG	iface_id;
} __attribute__ ((packed)) QMI_QOS_BIND_DATA_PORT_TLV_EP_ID, *PQMI_QOS_BIND_DATA_PORT_TLV_EP_ID;

typedef struct _QMI_QOS_BIND_DATA_PORT_TLV_MUX_ID
{
	UCHAR	TLVType;		//0x11
	USHORT	TLVLength;
	UCHAR	mux_id;
} __attribute__ ((packed)) QMI_QOS_BIND_DATA_PORT_TLV_MUX_ID, *PQMI_QOS_BIND_DATA_PORT_TLV_MUX_ID;

typedef struct _QMI_QOS_BIND_DATA_PORT_TLV_DATA_PORT
{
	UCHAR	TLVType;		//0x12
	USHORT	TLVLength;
	USHORT	data_port;
} __attribute__ ((packed)) QMI_QOS_BIND_DATA_PORT_TLV_DATA_PORT, *PQMI_QOS_BIND_DATA_PORT_TLV_DATA_PORT;

typedef struct _QMI_QOS_BIND_DATA_PORT_REQ_MSG
{
	USHORT	Type;
	USHORT	Length;
	QMI_QOS_BIND_DATA_PORT_TLV_EP_ID		EpIdTlv;
	QMI_QOS_BIND_DATA_PORT_TLV_MUX_ID		MuxIdTlv;
	//QMI_QOS_BIND_DATA_PORT_TLV_DATA_PORT	DataPortTlv;
} __attribute__ ((packed)) QMI_QOS_BIND_DATA_PORT_REQ_MSG, *PQMI_QOS_BIND_DATA_PORT_REQ_MSG;

typedef struct _QMI_QOS_BIND_DATA_PORT_RESP_MSG
{
	USHORT	Type;
	USHORT	Length;
	UCHAR	TLVType;		//0x02
	USHORT	TLVLength;
	USHORT	QMUXResult;
	USHORT	QMUXError;
} __attribute__ ((packed)) QMI_QOS_BIND_DATA_PORT_RESP_MSG, *PQMI_QOS_BIND_DATA_PORT_RESP_MSG;

typedef struct _QMI_QOS_INDICATION_REGISTER_TLV_REPORT_GLOBAL_QOS_FLOW
{
	UCHAR	TLVType;		//0x10
	USHORT	TLVLength;
	UCHAR	report_global_qos_flows;
} __attribute__ ((packed)) QMI_QOS_INDICATION_REGISTER_TLV_REPORT_GLOBAL_QOS_FLOW, *PQMI_QOS_INDICATION_REGISTER_TLV_REPORT_GLOBAL_QOS_FLOW;

typedef struct _QMI_QOS_INDICATION_REGISTER_TLV_SUPPRESS_REPORT_FLOW_CTL
{
	UCHAR	TLVType;		//0x11
	USHORT	TLVLength;
	UCHAR	suppress_report_flow_control;
} __attribute__ ((packed)) QMI_QOS_INDICATION_REGISTER_TLV_SUPPRESS_REPORT_FLOW_CTL, *PQMI_QOS_INDICATION_REGISTER_TLV_SUPPRESS_REPORT_FLOW_CTL;

typedef struct _QMI_QOS_INDICATION_REGISTER_TLV_SUPPRESS_NW_STATUS_IND
{
	UCHAR	TLVType;		//0x12
	USHORT	TLVLength;
	UCHAR	suppress_network_status_ind;
} __attribute__ ((packed)) QMI_QOS_INDICATION_REGISTER_TLV_SUPPRESS_NW_STATUS_IND, *PQMI_QOS_INDICATION_REGISTER_TLV_SUPPRESS_NW_STATUS_IND;

typedef struct _QMI_QOS_INDICATION_REGISTER_REQ_MSG
{
	USHORT	Type;
	USHORT	Length;
	QMI_QOS_INDICATION_REGISTER_TLV_REPORT_GLOBAL_QOS_FLOW		ReportGlobalQosFlowTlv;
 	//QMI_QOS_INDICATION_REGISTER_TLV_SUPPRESS_REPORT_FLOW_CTL	SuppressReportFlowCtlTlv;
	//QMI_QOS_INDICATION_REGISTER_TLV_SUPPRESS_NW_STATUS_IND		SuppressNWStatusIndTlv;
} __attribute__ ((packed)) QMI_QOS_INDICATION_REGISTER_REQ_MSG, *PQMI_QOS_INDICATION_REGISTER_REQ_MSG;

typedef struct _QMI_QOS_INDICATION_REGISTER_RESP_MSG
{
	USHORT	Type;
	USHORT	Length;
	UCHAR	TLVType;		//0x02
	USHORT	TLVLength;
	USHORT	QMUXResult;
	USHORT	QMUXError;
} __attribute__ ((packed)) QMI_QOS_INDICATION_REGISTER_RESP_MSG, *PQMI_QOS_INDICATION_REGISTER_RESP_MSG;

typedef struct _QMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_STATE
{
	UCHAR	TLVType;		//0x01
	USHORT	TLVLength;
	ULONG	qos_id;
	UCHAR	new_flow;
	ULONG	state_change;
} __attribute__ ((packed)) QMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_STATE, *PQMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_STATE;

typedef struct _QMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_GRANTED
{
	UCHAR	TLVType;		//0x10 0x11
	USHORT	TLVLength;
	ULONG64	flow_valid_params;
	ULONG	ip_flow_trf_cls;
	ULONG64 data_rate_max;
	ULONG64	guaranteed_rate;
	ULONG	peak_rate;
	ULONG	token_rate;
	ULONG	bucket_size;
	ULONG	ip_flow_latency;
	ULONG	ip_flow_jitter;
	USHORT	ip_flow_pkt_error_rate_multiplier;
	USHORT	ip_flow_pkt_error_rate_exponent;
	ULONG	ip_flow_min_policed_packet_size;
	ULONG	ip_flow_max_allowed_packet_size;
	ULONG	ip_flow_3gpp_residual_bit_error_rate;
	ULONG	ip_flow_3gpp_traffic_handling_priority;
	USHORT	ip_flow_3gpp2_profile_id;
	UCHAR	ip_flow_3gpp2_flow_priority;
	UCHAR	ip_flow_3gpp_im_cn_flag;
	UCHAR	ip_flow_3gpp_sig_ind;
	ULONG	ip_flow_lte_qci;
} __attribute__ ((packed)) QMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_GRANTED, *PQMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_GRANTED;

typedef struct _QMI_QOS_GLOBAL_QOS_FLOW_TLV_FILTER
{
	UCHAR	TLVType;		//0x12 0x13
	USHORT	TLVLength;
	UCHAR	tx_rx_qos_filter_len;
	UCHAR	ip_version;
	ULONG64	valid_params0;
	ULONG	ipv4_addr0;
	ULONG	subnet_mask0;
	ULONG	ipv4_addr1;
	ULONG	subnet_mask1;
	UCHAR	val4;
	UCHAR	mask4;
	ULONG64	valid_params01;
	UCHAR	ipv6_address00;
	UCHAR	ipv6_address01;
	UCHAR	ipv6_address02;
	UCHAR	ipv6_address03;
	UCHAR	ipv6_address04;
	UCHAR	ipv6_address05;
	UCHAR	ipv6_address06;
	UCHAR	ipv6_address07;
	UCHAR	ipv6_address08;
	UCHAR	ipv6_address09;
	UCHAR	ipv6_address010;
	UCHAR	ipv6_address011;
	UCHAR	ipv6_address012;
	UCHAR	ipv6_address013;
	UCHAR	ipv6_address014;
	ULONG	ipv6_address015;
	UCHAR	prefix_len0;
	UCHAR	ipv6_address10;
	UCHAR	ipv6_address11;
	UCHAR	ipv6_address12;
	UCHAR	ipv6_address13;
	UCHAR	ipv6_address14;
	UCHAR	ipv6_address15;
	UCHAR	ipv6_address16;
	UCHAR	ipv6_address17;
	UCHAR	ipv6_address18;
	UCHAR	ipv6_address19;
	UCHAR	ipv6_address110;
	UCHAR	ipv6_address111;
	UCHAR	ipv6_address112;
	UCHAR	ipv6_address113;
	UCHAR	ipv6_address114;
	ULONG	ipv6_address115;
	UCHAR	prefix_len1;
	UCHAR	val6;
	UCHAR	mask6;
	ULONG	flow_label;
	ULONG	xport_protocol;
	ULONG64	valid_params2;
	USHORT	port0;
	USHORT	range0;
	USHORT	port1;
	USHORT	range1;
	ULONG64	valid_params3;
	USHORT	port2;
	USHORT	range2;
	USHORT	port3;
	USHORT	range3;
	ULONG64	valid_params4;
	UCHAR	type;
	UCHAR	code;
	ULONG64	valid_params5;
	ULONG	spi0;
	ULONG64	valid_params6;
	ULONG	spi1;
	USHORT	filter_id;
	USHORT	filter_precedence;
} __attribute__ ((packed)) QMI_QOS_GLOBAL_QOS_FLOW_TLV_FILTER, *PQMI_QOS_GLOBAL_QOS_FLOW_TLV_FILTER;

typedef struct _QMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_TYPE
{
	UCHAR	TLVType;		//0x14
	USHORT	TLVLength;
	ULONG	flow_type;
} __attribute__ ((packed)) QMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_TYPE, *PQMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_TYPE;

typedef struct _QMI_QOS_GLOBAL_QOS_FLOW_TLV_BEARER_ID
{
	UCHAR	TLVType;		//0x15
	USHORT	TLVLength;
	UCHAR	bearer_id;
} __attribute__ ((packed)) QMI_QOS_GLOBAL_QOS_FLOW_TLV_BEARER_ID, *PQMI_QOS_GLOBAL_QOS_FLOW_TLV_BEARER_ID;

typedef struct _QMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_CTL_SEQ_NUM
{
	UCHAR	TLVType;		//0x16
	USHORT	TLVLength;
	USHORT	fc_seq_num;
} __attribute__ ((packed)) QMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_CTL_SEQ_NUM, *PQMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_CTL_SEQ_NUM;

typedef struct _QMI_QOS_GLOBAL_QOS_FLOW_TLV_5G_QCI
{
	UCHAR	TLVType;		//0x17 0x18
	USHORT	TLVLength;
	ULONG	tx_rx_5g_qci;
} __attribute__ ((packed)) QMI_QOS_GLOBAL_QOS_FLOW_TLV_5G_QCI, *PQMI_QOS_GLOBAL_QOS_FLOW_TLV_5G_QCI;

typedef struct _QMI_QOS_GLOBAL_QOS_FLOW_TLV_AVG_WINDOW
{
	UCHAR	TLVType;		//0x19 0x1A
	USHORT	TLVLength;
	USHORT	tx_rx_avg_window;
} __attribute__ ((packed)) QMI_QOS_GLOBAL_QOS_FLOW_TLV_AVG_WINDOW, *PQMI_QOS_GLOBAL_QOS_FLOW_TLV_AVG_WINDOW;

typedef struct _QMI_QOS_GLOBAL_QOS_FLOW_TLV_TX_FILTER_MATCH_ALL
{
	UCHAR	TLVType;		//0x1B
	USHORT	TLVLength;
	UCHAR	tx_filter_match_all_len;
	USHORT	filter_id;
} __attribute__ ((packed)) QMI_QOS_GLOBAL_QOS_FLOW_TLV_TX_FILTER_MATCH_ALL, *PQMI_QOS_GLOBAL_QOS_FLOW_TLV_TX_FILTER_MATCH_ALL;

typedef struct _QMI_QOS_GLOBAL_QOS_FLOW_IND_MSG
{
	USHORT	Type;
	USHORT	Length;
	QMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_STATE			FlowStateTlv;
	//QMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_GRANTED		TxFlowGrantedTlv;
	//QMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_GRANTED		RxFlowGrantedTlv;
	//QMI_QOS_GLOBAL_QOS_FLOW_TLV_FILTER				TxFilterTlv;
	//QMI_QOS_GLOBAL_QOS_FLOW_TLV_FILTER				RxFilterTlv;
	//QMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_TYPE			FlowTypeTlv;
	//QMI_QOS_GLOBAL_QOS_FLOW_TLV_BEARER_ID			BearerIdTlv;
	//QMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_CTL_SEQ_NUM	FlowCtlSeqNumTlv;
	//QMI_QOS_GLOBAL_QOS_FLOW_TLV_5G_QCI				Tx5GQciTlv;
	//QMI_QOS_GLOBAL_QOS_FLOW_TLV_5G_QCI				Rx5GQciTlv;
	//QMI_QOS_GLOBAL_QOS_FLOW_TLV_AVG_WINDOW			AvgWindowTlv;
	//QMI_QOS_GLOBAL_QOS_FLOW_TLV_TX_FILTER_MATCH_ALL	TxFilterMatchAllTlv;
} __attribute__ ((packed)) QMI_QOS_GLOBAL_QOS_FLOW_IND_MSG, *PQMI_QOS_GLOBAL_QOS_FLOW_IND_MSG;

typedef struct _QMI_QOS_GET_QOS_INFO_TLV_QOS_ID
{
	UCHAR	TLVType;		//0x01
	USHORT	TLVLength;
	ULONG	qos_id;
} __attribute__ ((packed)) QMI_QOS_GET_QOS_INFO_TLV_QOS_ID, *PQMI_QOS_GET_QOS_INFO_TLV_QOS_ID;

typedef struct _QMI_QOS_GET_QOS_INFO_TLV_FLOW_STATUS
{
	UCHAR	TLVType;		//0x10
	USHORT	TLVLength;
	UCHAR	flow_status;
} __attribute__ ((packed)) QMI_QOS_GET_QOS_INFO_TLV_FLOW_STATUS, *PQMI_QOS_GET_QOS_INFO_TLV_FLOW_STATUS;

typedef struct _QMI_QOS_GET_QOS_INFO_TLV_GRANTED_FLOW
{
	UCHAR	TLVType;		//0x11 0x12
	USHORT	TLVLength;
	ULONG64	flow_valid_params;
	ULONG	ip_flow_trf_cls;
	ULONG64 data_rate_max;
	ULONG64	guaranteed_rate;
	ULONG	peak_rate;
	ULONG	token_rate;
	ULONG	bucket_size;
	ULONG	ip_flow_latency;
	ULONG	ip_flow_jitter;
	USHORT	ip_flow_pkt_error_rate_multiplier;
	USHORT	ip_flow_pkt_error_rate_exponent;
	ULONG	ip_flow_min_policed_packet_size;
	ULONG	ip_flow_max_allowed_packet_size;
	ULONG	ip_flow_3gpp_residual_bit_error_rate;
	ULONG	ip_flow_3gpp_traffic_handling_priority;
	USHORT	ip_flow_3gpp2_profile_id;
	UCHAR	ip_flow_3gpp2_flow_priority;
	UCHAR	ip_flow_3gpp_im_cn_flag;
	UCHAR	ip_flow_3gpp_sig_ind;
	ULONG	ip_flow_lte_qci;
} __attribute__ ((packed)) QMI_QOS_GET_QOS_INFO_TLV_GRANTED_FLOW, *PQMI_QOS_GET_QOS_INFO_TLV_GRANTED_FLOW;

typedef struct _QMI_QOS_GET_QOS_INFO_TLV_FILTER_SPECS
{
	UCHAR	TLVType;		//0x13 0x14
	USHORT	TLVLength;
	UCHAR	tx_rx_qos_filter_len;
	UCHAR	ip_version;
	ULONG64	valid_params0;
	ULONG	ipv4_addr0;
	ULONG	subnet_mask0;
	ULONG	ipv4_addr1;
	ULONG	subnet_mask1;
	UCHAR	val4;
	UCHAR	mask4;
	ULONG64	valid_params01;
	UCHAR	ipv6_address00;
	UCHAR	ipv6_address01;
	UCHAR	ipv6_address02;
	UCHAR	ipv6_address03;
	UCHAR	ipv6_address04;
	UCHAR	ipv6_address05;
	UCHAR	ipv6_address06;
	UCHAR	ipv6_address07;
	UCHAR	ipv6_address08;
	UCHAR	ipv6_address09;
	UCHAR	ipv6_address010;
	UCHAR	ipv6_address011;
	UCHAR	ipv6_address012;
	UCHAR	ipv6_address013;
	UCHAR	ipv6_address014;
	ULONG	ipv6_address015;
	UCHAR	prefix_len0;
	UCHAR	ipv6_address10;
	UCHAR	ipv6_address11;
	UCHAR	ipv6_address12;
	UCHAR	ipv6_address13;
	UCHAR	ipv6_address14;
	UCHAR	ipv6_address15;
	UCHAR	ipv6_address16;
	UCHAR	ipv6_address17;
	UCHAR	ipv6_address18;
	UCHAR	ipv6_address19;
	UCHAR	ipv6_address110;
	UCHAR	ipv6_address111;
	UCHAR	ipv6_address112;
	UCHAR	ipv6_address113;
	UCHAR	ipv6_address114;
	ULONG	ipv6_address115;
	UCHAR	prefix_len1;
	UCHAR	val6;
	UCHAR	mask6;
	ULONG	flow_label;
	ULONG	xport_protocol;
	ULONG64	valid_params2;
	USHORT	port0;
	USHORT	range0;
	USHORT	port1;
	USHORT	range1;
	ULONG64	valid_params3;
	USHORT	port2;
	USHORT	range2;
	USHORT	port3;
	USHORT	range3;
	ULONG64	valid_params4;
	UCHAR	type;
	UCHAR	code;
	ULONG64	valid_params5;
	ULONG	spi0;
	ULONG64	valid_params6;
	ULONG	spi1;
	USHORT	filter_id;
	USHORT	filter_precedence;
} __attribute__ ((packed)) QMI_QOS_GET_QOS_INFO_TLV_FILTER_SPECS, *PQMI_QOS_GET_QOS_INFO_TLV_FILTER_SPECS;

typedef struct _QMI_QOS_GET_QOS_INFO_TLV_EXT_ERROR_INFO
{
	UCHAR	TLVType;		//0x15
	USHORT	TLVLength;
	USHORT	ext_error_info;
} __attribute__ ((packed)) QMI_QOS_GET_QOS_INFO_TLV_EXT_ERROR_INFO, *PQMI_QOS_GET_QOS_INFO_TLV_EXT_ERROR_INFO;

typedef struct _QMI_QOS_GET_QOS_INFO_TLV_5G_QCI
{
	UCHAR	TLVType;		//0x16 0x17
	USHORT	TLVLength;
	ULONG	tx_rx_5g_qci;
} __attribute__ ((packed)) QMI_QOS_GET_QOS_INFO_TLV_5G_QCI, *PQMI_QOS_GET_QOS_INFO_TLV_5G_QCI;

typedef struct _QMI_QOS_GET_QOS_INFO_TLV_AVG_WINDOW
{
	UCHAR	TLVType;		//0x18 0x19
	USHORT	TLVLength;
	USHORT	tx_rx_averaging_window;
} __attribute__ ((packed)) QMI_QOS_GET_QOS_INFO_TLV_AVG_WINDOW, *PQMI_QOS_GET_QOS_INFO_TLV_AVG_WINDOW;

typedef struct _QMI_QOS_GET_QOS_INFO_TLV_TX_FILTER_MATCH_ALL
{
	UCHAR	TLVType;		//0x1A
	USHORT	TLVLength;
	UCHAR	tx_filter_match_all_len;
	USHORT	filter_id;
} __attribute__ ((packed)) QMI_QOS_GET_QOS_INFO_TLV_TX_FILTER_MATCH_ALL, *PQMI_QOS_GET_QOS_INFO_TLV_TX_FILTER_MATCH_ALL;

typedef struct _QMI_QOS_GET_QOS_INFO_REQ_MSG
{
	USHORT	Type;
	USHORT	Length;
	QMI_QOS_GET_QOS_INFO_TLV_QOS_ID		QosIdTlv;
} __attribute__ ((packed)) QMI_QOS_GET_QOS_INFO_REQ_MSG, *PQMI_QOS_GET_QOS_INFO_REQ_MSG;

typedef struct _QMI_QOS_GET_QOS_INFO_RESP_MSG
{
	USHORT	Type;
	USHORT	Length;
	UCHAR	TLVType;		//0x02
	USHORT	TLVLength;
	USHORT	QMUXResult;
	USHORT	QMUXError;
	//QMI_QOS_GET_QOS_INFO_TLV_FLOW_STATUS			FlowStatusTlv;
	//QMI_QOS_GET_QOS_INFO_TLV_GRANTED_FLOW			TxGrantedFlowTlv;
	//QMI_QOS_GET_QOS_INFO_TLV_GRANTED_FLOW			RxGrantedFlowTlv;
	//QMI_QOS_GET_QOS_INFO_TLV_FILTER_SPECS			TxFilterSpecsTlv;
	//QMI_QOS_GET_QOS_INFO_TLV_FILTER_SPECS			RxFilterSpecsTlv;
	//QMI_QOS_GET_QOS_INFO_TLV_EXT_ERROR_INFO			ExtErrorInfoTlv;
	//QMI_QOS_GET_QOS_INFO_TLV_5G_QCI					Tx5GQciTlv;
	//QMI_QOS_GET_QOS_INFO_TLV_5G_QCI					Rx5GQciTlv;
	//QMI_QOS_GET_QOS_INFO_TLV_AVG_WINDOW				TxAvgWindowTlv;
	//QMI_QOS_GET_QOS_INFO_TLV_AVG_WINDOW				RxAvgWindowTlv;
	//QMI_QOS_GET_QOS_INFO_TLV_TX_FILTER_MATCH_ALL	TxFilterMatchAllTlv;
} __attribute__ ((packed)) QMI_QOS_GET_QOS_INFO_RESP_MSG, *PQMI_QOS_GET_QOS_INFO_RESP_MSG;

#define QOS_IND_FLOW_STATE_ACTIVATED 0x00
#define QOS_IND_FLOW_STATE_MODIFIED  0x01
#define QOS_IND_FLOW_STATE_DELETED   0x02
#define QOS_IND_FLOW_STATE_SUSPENDED 0x03
#define QOS_IND_FLOW_STATE_ENABLED   0x04
#define QOS_IND_FLOW_STATE_DISABLED  0x05
#define QOS_IND_FLOW_STATE_INVALID	 0x06

#define QOS_EVENT_RPT_IND_FLOW_ACTIVATED 0x01
#define QOS_EVENT_RPT_IND_FLOW_MODIFIED  0x02
#define QOS_EVENT_RPT_IND_FLOW_DELETED   0x03
#define QOS_EVENT_RPT_IND_FLOW_SUSPENDED 0x04
#define QOS_EVENT_RPT_IND_FLOW_ENABLED   0x05
#define QOS_EVENT_RPT_IND_FLOW_DISABLED  0x06

#define QOS_EVENT_RPT_IND_TLV_PHY_LINK_STATE_TYPE 0x01
#define QOS_EVENT_RPT_IND_TLV_GLOBAL_FL_RPT_STATE 0x10
#define QOS_EVENT_RPT_IND_TLV_GLOBAL_FL_RPT_TYPE  0x10
#define QOS_EVENT_RPT_IND_TLV_TX_FLOW_TYPE        0x11
#define QOS_EVENT_RPT_IND_TLV_RX_FLOW_TYPE        0x12
#define QOS_EVENT_RPT_IND_TLV_TX_FILTER_TYPE      0x13
#define QOS_EVENT_RPT_IND_TLV_RX_FILTER_TYPE      0x14
#define QOS_EVENT_RPT_IND_TLV_FLOW_SPEC           0x10
#define QOS_EVENT_RPT_IND_TLV_FILTER_SPEC         0x10

typedef struct _QOS_EVENT_RPT_IND_TLV_PHY_LINK_STATE
{
   UCHAR  TLVType;       // 0x01
   USHORT TLVLength;     // 1
   UCHAR  PhyLinkState;  // 0-dormant, 1-active
} QOS_EVENT_RPT_IND_TLV_PHY_LINK_STATE, *PQOS_EVENT_RPT_IND_TLV_PHY_LINK_STATE;

typedef struct _QOS_EVENT_RPT_IND_TLV_GLOBAL_FL_RPT
{
   UCHAR  TLVType;       // 0x10
   USHORT TLVLength;     // 6
   ULONG  QosId;
   UCHAR  NewFlow;       // 1: newly added flow; 0: existing flow
   UCHAR  StateChange;   // 1: activated; 2: modified; 3: deleted;
                         // 4: suspended(delete); 5: enabled; 6: disabled
} QOS_EVENT_RPT_IND_TLV_GLOBAL_FL_RPT, *PQOS_EVENT_RPT_IND_TLV_GLOBAL_FL_RPT;

// QOS Flow

typedef struct _QOS_EVENT_RPT_IND_TLV_FLOW
{
   UCHAR  TLVType;       // 0x10-TX flow; 0x11-RX flow
   USHORT TLVLength;     // var
   // embedded TLV's
} QOS_EVENT_RPT_IND_TLV_TX_FLOW, *PQOS_EVENT_RPT_IND_TLV_TX_FLOW;

#define QOS_FLOW_TLV_IP_FLOW_IDX_TYPE                    0x10
#define QOS_FLOW_TLV_IP_FLOW_TRAFFIC_CLASS_TYPE          0x11
#define QOS_FLOW_TLV_IP_FLOW_DATA_RATE_MIN_MAX_TYPE      0x12
#define QOS_FLOW_TLV_IP_FLOW_DATA_RATE_TOKEN_BUCKET_TYPE 0x13
#define QOS_FLOW_TLV_IP_FLOW_LATENCY_TYPE                0x14
#define QOS_FLOW_TLV_IP_FLOW_JITTER_TYPE                 0x15
#define QOS_FLOW_TLV_IP_FLOW_PKT_ERR_RATE_TYPE           0x16
#define QOS_FLOW_TLV_IP_FLOW_MIN_PKT_SIZE_TYPE           0x17
#define QOS_FLOW_TLV_IP_FLOW_MAX_PKT_SIZE_TYPE           0x18
#define QOS_FLOW_TLV_IP_FLOW_3GPP_BIT_ERR_RATE_TYPE      0x19
#define QOS_FLOW_TLV_IP_FLOW_3GPP_TRAF_PRIORITY_TYPE     0x1A
#define QOS_FLOW_TLV_IP_FLOW_3GPP2_PROFILE_ID_TYPE       0x1B

typedef struct _QOS_FLOW_TLV_IP_FLOW_IDX
{
   UCHAR  TLVType;       // 0x10
   USHORT TLVLength;     // 1
   UCHAR  IpFlowIndex;
}  QOS_FLOW_TLV_IP_FLOW_IDX, *PQOS_FLOW_TLV_IP_FLOW_IDX;

typedef struct _QOS_FLOW_TLV_IP_FLOW_TRAFFIC_CLASS
{
   UCHAR  TLVType;       // 0x11
   USHORT TLVLength;     // 1
   UCHAR  TrafficClass;
}  QOS_FLOW_TLV_IP_FLOW_TRAFFIC_CLASS, *PQOS_FLOW_TLV_IP_FLOW_TRAFFIC_CLASS;

typedef struct _QOS_FLOW_TLV_IP_FLOW_DATA_RATE_MIN_MAX
{
   UCHAR  TLVType;       // 0x12
   USHORT TLVLength;     // 8
   ULONG  DataRateMax;
   ULONG  GuaranteedRate;
}  QOS_FLOW_TLV_IP_FLOW_DATA_RATE_MIN_MAX, *PQOS_FLOW_TLV_IP_FLOW_DATA_RATE_MIN_MAX;

typedef struct _QOS_FLOW_TLV_IP_FLOW_DATA_RATE_TOKEN_BUCKET
{
   UCHAR  TLVType;       // 0x13
   USHORT TLVLength;     // 12
   ULONG  PeakRate;
   ULONG  TokenRate;
   ULONG  BucketSize;
}  QOS_FLOW_TLV_IP_FLOW_DATA_RATE_TOKEN_BUCKET, *PQOS_FLOW_TLV_IP_FLOW_DATA_RATE_TOKEN_BUCKET;

typedef struct _QOS_FLOW_TLV_IP_FLOW_LATENCY
{
   UCHAR  TLVType;       // 0x14
   USHORT TLVLength;     // 4
   ULONG  IpFlowLatency;
}  QOS_FLOW_TLV_IP_FLOW_LATENCY, *PQOS_FLOW_TLV_IP_FLOW_LATENCY;

typedef struct _QOS_FLOW_TLV_IP_FLOW_JITTER
{
   UCHAR  TLVType;       // 0x15
   USHORT TLVLength;     // 4
   ULONG  IpFlowJitter;
}  QOS_FLOW_TLV_IP_FLOW_JITTER, *PQOS_FLOW_TLV_IP_FLOW_JITTER;

typedef struct _QOS_FLOW_TLV_IP_FLOW_PKT_ERR_RATE
{
   UCHAR  TLVType;       // 0x16
   USHORT TLVLength;     // 4
   USHORT ErrRateMultiplier;
   USHORT ErrRateExponent;
}  QOS_FLOW_TLV_IP_FLOW_PKT_ERR_RATE, *PQOS_FLOW_TLV_IP_FLOW_PKT_ERR_RATE;

typedef struct _QOS_FLOW_TLV_IP_FLOW_MIN_PKT_SIZE
{
   UCHAR  TLVType;       // 0x17
   USHORT TLVLength;     // 4
   ULONG  MinPolicedPktSize;
}  QOS_FLOW_TLV_IP_FLOW_MIN_PKT_SIZE, *PQOS_FLOW_TLV_IP_FLOW_MIN_PKT_SIZE;

typedef struct _QOS_FLOW_TLV_IP_FLOW_MAX_PKT_SIZE
{
   UCHAR  TLVType;       // 0x18
   USHORT TLVLength;     // 4
   ULONG  MaxAllowedPktSize;
}  QOS_FLOW_TLV_IP_FLOW_MAX_PKT_SIZE, *PQOS_FLOW_TLV_IP_FLOW_MAX_PKT_SIZE;

typedef struct _QOS_FLOW_TLV_IP_FLOW_3GPP_BIT_ERR_RATE
{
   UCHAR  TLVType;       // 0x19
   USHORT TLVLength;     // 1
   UCHAR  ResidualBitErrorRate;
}  QOS_FLOW_TLV_IP_FLOW_3GPP_BIT_ERR_RATE, *PQOS_FLOW_TLV_IP_FLOW_3GPP_BIT_ERR_RATE;

typedef struct _QOS_FLOW_TLV_IP_FLOW_3GPP_TRAF_PRIORITY
{
   UCHAR  TLVType;       // 0x1A
   USHORT TLVLength;     // 1
   UCHAR  TrafficHandlingPriority;
}  QOS_FLOW_TLV_IP_FLOW_3GPP_TRAF_PRIORITY, *PQOS_FLOW_TLV_IP_FLOW_3GPP_TRAF_PRIORITY;

typedef struct _QOS_FLOW_TLV_IP_FLOW_3GPP2_PROFILE_ID
{
   UCHAR  TLVType;       // 0x1B
   USHORT TLVLength;     // 2
   USHORT ProfileId;
}  QOS_FLOW_TLV_IP_FLOW_3GPP2_PROFILE_ID, *PQOS_FLOW_TLV_IP_FLOW_3GPP2_PROFILE_ID;

// QOS Filter

#define QOS_FILTER_TLV_IP_FILTER_IDX_TYPE          0x10
#define QOS_FILTER_TLV_IP_VERSION_TYPE             0x11
#define QOS_FILTER_TLV_IPV4_SRC_ADDR_TYPE          0x12
#define QOS_FILTER_TLV_IPV4_DEST_ADDR_TYPE         0x13
#define QOS_FILTER_TLV_NEXT_HDR_PROTOCOL_TYPE      0x14
#define QOS_FILTER_TLV_IPV4_TYPE_OF_SERVICE_TYPE   0x15
#define QOS_FILTER_TLV_TCP_UDP_PORT_SRC_TCP_TYPE   0x1B
#define QOS_FILTER_TLV_TCP_UDP_PORT_DEST_TCP_TYPE  0x1C
#define QOS_FILTER_TLV_TCP_UDP_PORT_SRC_UDP_TYPE   0x1D
#define QOS_FILTER_TLV_TCP_UDP_PORT_DEST_UDP_TYPE  0x1E
#define QOS_FILTER_TLV_ICMP_FILTER_MSG_TYPE_TYPE   0x1F
#define QOS_FILTER_TLV_ICMP_FILTER_MSG_CODE_TYPE   0x20
#define QOS_FILTER_TLV_TCP_UDP_PORT_SRC_TYPE       0x24
#define QOS_FILTER_TLV_TCP_UDP_PORT_DEST_TYPE      0x25

typedef struct _QOS_EVENT_RPT_IND_TLV_FILTER
{
   UCHAR  TLVType;       // 0x12-TX filter; 0x13-RX filter
   USHORT TLVLength;     // var
   // embedded TLV's
} QOS_EVENT_RPT_IND_TLV_RX_FILTER, *PQOS_EVENT_RPT_IND_TLV_RX_FILTER;

typedef struct _QOS_FILTER_TLV_IP_FILTER_IDX
{
   UCHAR  TLVType;       // 0x10
   USHORT TLVLength;     // 1
   UCHAR  IpFilterIndex;
}  QOS_FILTER_TLV_IP_FILTER_IDX, *PQOS_FILTER_TLV_IP_FILTER_IDX;

typedef struct _QOS_FILTER_TLV_IP_VERSION
{
   UCHAR  TLVType;       // 0x11
   USHORT TLVLength;     // 1
   UCHAR  IpVersion;
}  QOS_FILTER_TLV_IP_VERSION, *PQOS_FILTER_TLV_IP_VERSION;

typedef struct _QOS_FILTER_TLV_IPV4_SRC_ADDR
{
   UCHAR  TLVType;       // 0x12
   USHORT TLVLength;     // 8
   ULONG  IpSrcAddr;
   ULONG  IpSrcSubnetMask;
}  QOS_FILTER_TLV_IPV4_SRC_ADDR, *PQOS_FILTER_TLV_IPV4_SRC_ADDR;

typedef struct _QOS_FILTER_TLV_IPV4_DEST_ADDR
{
   UCHAR  TLVType;       // 0x13
   USHORT TLVLength;     // 8
   ULONG  IpDestAddr;
   ULONG  IpDestSubnetMask;
}  QOS_FILTER_TLV_IPV4_DEST_ADDR, *PQOS_FILTER_TLV_IPV4_DEST_ADDR;

typedef struct _QOS_FILTER_TLV_NEXT_HDR_PROTOCOL
{
   UCHAR  TLVType;       // 0x14
   USHORT TLVLength;     // 1
   UCHAR  NextHdrProtocol;
}  QOS_FILTER_TLV_NEXT_HDR_PROTOCOL, *PQOS_FILTER_TLV_NEXT_HDR_PROTOCOL;

typedef struct _QOS_FILTER_TLV_IPV4_TYPE_OF_SERVICE
{
   UCHAR  TLVType;       // 0x15
   USHORT TLVLength;     // 2
   UCHAR  Ipv4TypeOfService;
   UCHAR  Ipv4TypeOfServiceMask;
}  QOS_FILTER_TLV_IPV4_TYPE_OF_SERVICE, *PQOS_FILTER_TLV_IPV4_TYPE_OF_SERVICE;

typedef struct _QOS_FILTER_TLV_TCP_UDP_PORT
{
   UCHAR  TLVType;       // source port: 0x1B-TCP; 0x1D-UDP
                         // dest port:   0x1C-TCP; 0x1E-UDP
   USHORT TLVLength;     // 4
   USHORT FilterPort;
   USHORT FilterPortRange;
}  QOS_FILTER_TLV_TCP_UDP_PORT, *PQOS_FILTER_TLV_TCP_UDP_PORT;

typedef struct _QOS_FILTER_TLV_ICMP_FILTER_MSG_TYPE
{
   UCHAR  TLVType;       // 0x1F
   USHORT TLVLength;     // 1
   UCHAR  IcmpFilterMsgType;
}  QOS_FILTER_TLV_ICMP_FILTER_MSG_TYPE, *PQOS_FILTER_TLV_ICMP_FILTER_MSG_TYPE;

typedef struct _QOS_FILTER_TLV_ICMP_FILTER_MSG_CODE
{
   UCHAR  TLVType;       // 0x20
   USHORT TLVLength;     // 1
   UCHAR  IcmpFilterMsgCode;
}  QOS_FILTER_TLV_ICMP_FILTER_MSG_CODE, *PQOS_FILTER_TLV_ICMP_FILTER_MSG_CODE;

#define QOS_FILTER_PRECEDENCE_INVALID  256
#define QOS_FILTER_TLV_PRECEDENCE_TYPE 0x22
#define QOS_FILTER_TLV_ID_TYPE         0x23

typedef struct _QOS_FILTER_TLV_PRECEDENCE
{
   UCHAR  TLVType;    // 0x22
   USHORT TLVLength;  // 2
   USHORT Precedence; // precedence of the filter
}  QOS_FILTER_TLV_PRECEDENCE, *PQOS_FILTER_TLV_PRECEDENCE;

typedef struct _QOS_FILTER_TLV_ID
{
   UCHAR  TLVType;    // 0x23
   USHORT TLVLength;  // 2
   USHORT FilterId;   // filter ID
}  QOS_FILTER_TLV_ID, *PQOS_FILTER_TLV_ID;

#ifdef QCQOS_IPV6

#define QOS_FILTER_TLV_IPV6_SRC_ADDR_TYPE          0x16
#define QOS_FILTER_TLV_IPV6_DEST_ADDR_TYPE         0x17
#define QOS_FILTER_TLV_IPV6_NEXT_HDR_PROTOCOL_TYPE 0x14  // same as IPV4
#define QOS_FILTER_TLV_IPV6_TRAFFIC_CLASS_TYPE     0x19
#define QOS_FILTER_TLV_IPV6_FLOW_LABEL_TYPE        0x1A

typedef struct _QOS_FILTER_TLV_IPV6_SRC_ADDR
{
   UCHAR  TLVType;       // 0x16
   USHORT TLVLength;     // 17
   UCHAR  IpSrcAddr[16];
   UCHAR  IpSrcAddrPrefixLen;  // [0..128]
}  QOS_FILTER_TLV_IPV6_SRC_ADDR, *PQOS_FILTER_TLV_IPV6_SRC_ADDR;

typedef struct _QOS_FILTER_TLV_IPV6_DEST_ADDR
{
   UCHAR  TLVType;       // 0x17
   USHORT TLVLength;     // 17
   UCHAR  IpDestAddr[16];
   UCHAR  IpDestAddrPrefixLen;  // [0..128]
}  QOS_FILTER_TLV_IPV6_DEST_ADDR, *PQOS_FILTER_TLV_IPV6_DEST_ADDR;

#define QOS_FILTER_IPV6_NEXT_HDR_PROTOCOL_TCP 0x06
#define QOS_FILTER_IPV6_NEXT_HDR_PROTOCOL_UDP 0x11

typedef struct _QOS_FILTER_TLV_IPV6_TRAFFIC_CLASS
{
   UCHAR  TLVType;       // 0x19
   USHORT TLVLength;     // 2
   UCHAR  TrafficClass;
   UCHAR  TrafficClassMask; // compare the first 6 bits only
}  QOS_FILTER_TLV_IPV6_TRAFFIC_CLASS, *PQOS_FILTER_TLV_IPV6_TRAFFIC_CLASS;

typedef struct _QOS_FILTER_TLV_IPV6_FLOW_LABEL
{
   UCHAR  TLVType;       // 0x1A
   USHORT TLVLength;     // 4
   ULONG  FlowLabel;
}  QOS_FILTER_TLV_IPV6_FLOW_LABEL, *PQOS_FILTER_TLV_IPV6_FLOW_LABEL;

#endif // QCQOS_IPV6
#endif

// ======================= WMS ==============================
#define QMIWMS_SET_EVENT_REPORT_REQ           0x0001
#define QMIWMS_SET_EVENT_REPORT_RESP          0x0001
#define QMIWMS_EVENT_REPORT_IND               0x0001
#define QMIWMS_RAW_SEND_REQ                   0x0020
#define QMIWMS_RAW_SEND_RESP                  0x0020
#define QMIWMS_RAW_WRITE_REQ                  0x0021
#define QMIWMS_RAW_WRITE_RESP                 0x0021
#define QMIWMS_RAW_READ_REQ                   0x0022
#define QMIWMS_RAW_READ_RESP                  0x0022
#define QMIWMS_MODIFY_TAG_REQ                 0x0023
#define QMIWMS_MODIFY_TAG_RESP                0x0023
#define QMIWMS_DELETE_REQ                     0x0024
#define QMIWMS_DELETE_RESP                    0x0024
#define QMIWMS_GET_MESSAGE_PROTOCOL_REQ       0x0030
#define QMIWMS_GET_MESSAGE_PROTOCOL_RESP      0x0030
#define QMIWMS_LIST_MESSAGES_REQ              0x0031
#define QMIWMS_LIST_MESSAGES_RESP             0x0031
#define QMIWMS_GET_SMSC_ADDRESS_REQ           0x0034
#define QMIWMS_GET_SMSC_ADDRESS_RESP          0x0034
#define QMIWMS_SET_SMSC_ADDRESS_REQ           0x0035
#define QMIWMS_SET_SMSC_ADDRESS_RESP          0x0035
#define QMIWMS_GET_STORE_MAX_SIZE_REQ         0x0036
#define QMIWMS_GET_STORE_MAX_SIZE_RESP        0x0036


#define WMS_MESSAGE_PROTOCOL_CDMA             0x00
#define WMS_MESSAGE_PROTOCOL_WCDMA            0x01

#if 0
typedef struct _QMIWMS_GET_MESSAGE_PROTOCOL_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
} QMIWMS_GET_MESSAGE_PROTOCOL_REQ_MSG, *PQMIWMS_GET_MESSAGE_PROTOCOL_REQ_MSG;

typedef struct _QMIWMS_GET_MESSAGE_PROTOCOL_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
   UCHAR  TLV2Type;
   USHORT TLV2Length;
   UCHAR  MessageProtocol;
} QMIWMS_GET_MESSAGE_PROTOCOL_RESP_MSG, *PQMIWMS_GET_MESSAGE_PROTOCOL_RESP_MSG;

typedef struct _QMIWMS_GET_STORE_MAX_SIZE_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  StorageType;
} QMIWMS_GET_STORE_MAX_SIZE_REQ_MSG, *PQMIWMS_GET_STORE_MAX_SIZE_REQ_MSG;

typedef struct _QMIWMS_GET_STORE_MAX_SIZE_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
   UCHAR  TLV2Type;
   USHORT TLV2Length;
   ULONG  MemStoreMaxSize;
} QMIWMS_GET_STORE_MAX_SIZE_RESP_MSG, *PQMIWMS_GET_STORE_MAX_SIZE_RESP_MSG;

typedef struct _REQUEST_TAG
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  TagType;
} REQUEST_TAG, *PREQUEST_TAG;

typedef struct _QMIWMS_LIST_MESSAGES_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  StorageType;
} QMIWMS_LIST_MESSAGES_REQ_MSG, *PQMIWMS_LIST_MESSAGES_REQ_MSG;

typedef struct _QMIWMS_MESSAGE
{
   ULONG  MessageIndex;
   UCHAR  TagType;
} QMIWMS_MESSAGE, *PQMIWMS_MESSAGE;

typedef struct _QMIWMS_LIST_MESSAGES_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
   UCHAR  TLV2Type;
   USHORT TLV2Length;
   ULONG  NumMessages;
} QMIWMS_LIST_MESSAGES_RESP_MSG, *PQMIWMS_LIST_MESSAGES_RESP_MSG;

typedef struct _QMIWMS_RAW_READ_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  StorageType;
   ULONG  MemoryIndex;
} QMIWMS_RAW_READ_REQ_MSG, *PQMIWMS_RAW_READ_REQ_MSG;

typedef struct _QMIWMS_RAW_READ_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
   UCHAR  TLV2Type;
   USHORT TLV2Length;
   UCHAR  TagType;
   UCHAR  Format;
   USHORT MessageLength;
   UCHAR  Message;
} QMIWMS_RAW_READ_RESP_MSG, *PQMIWMS_RAW_READ_RESP_MSG;

typedef struct _QMIWMS_MODIFY_TAG_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  StorageType;
   ULONG  MemoryIndex;
   UCHAR  TagType;
} QMIWMS_MODIFY_TAG_REQ_MSG, *PQMIWMS_MODIFY_TAG_REQ_MSG;

typedef struct _QMIWMS_MODIFY_TAG_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
} QMIWMS_MODIFY_TAG_RESP_MSG, *PQMIWMS_MODIFY_TAG_RESP_MSG;

typedef struct _QMIWMS_RAW_SEND_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  SmsFormat;
   USHORT SmsLength;
   UCHAR  SmsMessage;
} QMIWMS_RAW_SEND_REQ_MSG, *PQMIWMS_RAW_SEND_REQ_MSG;

typedef struct _RAW_SEND_CAUSE_CODE
{
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT CauseCode;
} RAW_SEND_CAUSE_CODE, *PRAW_SEND_CAUSE_CODE;


typedef struct _QMIWMS_RAW_SEND_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
} QMIWMS_RAW_SEND_RESP_MSG, *PQMIWMS_RAW_SEND_RESP_MSG;


typedef struct _WMS_DELETE_MESSAGE_INDEX
{
   UCHAR  TLVType;
   USHORT TLVLength;
   ULONG  MemoryIndex;
} WMS_DELETE_MESSAGE_INDEX, *PWMS_DELETE_MESSAGE_INDEX;

typedef struct _WMS_DELETE_MESSAGE_TAG
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  MessageTag;
} WMS_DELETE_MESSAGE_TAG, *PWMS_DELETE_MESSAGE_TAG;

typedef struct _QMIWMS_DELETE_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  StorageType;
} QMIWMS_DELETE_REQ_MSG, *PQMIWMS_DELETE_REQ_MSG;

typedef struct _QMIWMS_DELETE_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
} QMIWMS_DELETE_RESP_MSG, *PQMIWMS_DELETE_RESP_MSG;


typedef struct _QMIWMS_GET_SMSC_ADDRESS_REQ_MSG
{
   USHORT Type;
   USHORT Length;
} QMIWMS_GET_SMSC_ADDRESS_REQ_MSG, *PQMIWMS_GET_SMSC_ADDRESS_REQ_MSG;

typedef struct _QMIWMS_SMSC_ADDRESS
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  SMSCAddressType[3];
   UCHAR  SMSCAddressLength;
   UCHAR  SMSCAddressDigits;
} QMIWMS_SMSC_ADDRESS, *PQMIWMS_SMSC_ADDRESS;


typedef struct _QMIWMS_GET_SMSC_ADDRESS_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
   UCHAR  SMSCAddress;
} QMIWMS_GET_SMSC_ADDRESS_RESP_MSG, *PQMIWMS_GET_SMSC_ADDRESS_RESP_MSG;

typedef struct _QMIWMS_SET_SMSC_ADDRESS_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  SMSCAddress;
} QMIWMS_SET_SMSC_ADDRESS_REQ_MSG, *PQMIWMS_SET_SMSC_ADDRESS_REQ_MSG;

typedef struct _QMIWMS_SET_SMSC_ADDRESS_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
} QMIWMS_SET_SMSC_ADDRESS_RESP_MSG, *PQMIWMS_SET_SMSC_ADDRESS_RESP_MSG;

typedef struct _QMIWMS_SET_EVENT_REPORT_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  ReportNewMessage;
} QMIWMS_SET_EVENT_REPORT_REQ_MSG, *PQMIWMS_SET_EVENT_REPORT_REQ_MSG;

typedef struct _QMIWMS_SET_EVENT_REPORT_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
} QMIWMS_SET_EVENT_REPORT_RESP_MSG, *PQMIWMS_SET_EVENT_REPORT_RESP_MSG;

typedef struct _QMIWMS_EVENT_REPORT_IND_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  StorageType;
   ULONG  StorageIndex;
} QMIWMS_EVENT_REPORT_IND_MSG, *PQMIWMS_EVENT_REPORT_IND_MSG;
#endif

// ======================= End of WMS ==============================


// ======================= NAS ==============================
#define QMINAS_SET_EVENT_REPORT_REQ             0x0002
#define QMINAS_SET_EVENT_REPORT_RESP            0x0002
#define QMINAS_EVENT_REPORT_IND                 0x0002
#define QMINAS_GET_SIGNAL_STRENGTH_REQ          0x0020
#define QMINAS_GET_SIGNAL_STRENGTH_RESP         0x0020
#define QMINAS_PERFORM_NETWORK_SCAN_REQ         0x0021
#define QMINAS_PERFORM_NETWORK_SCAN_RESP        0x0021
#define QMINAS_INITIATE_NW_REGISTER_REQ         0x0022
#define QMINAS_INITIATE_NW_REGISTER_RESP        0x0022
#define QMINAS_INITIATE_ATTACH_REQ              0x0023
#define QMINAS_INITIATE_ATTACH_RESP             0x0023
#define QMINAS_GET_SERVING_SYSTEM_REQ           0x0024
#define QMINAS_GET_SERVING_SYSTEM_RESP          0x0024
#define QMINAS_SERVING_SYSTEM_IND               0x0024
#define QMINAS_GET_HOME_NETWORK_REQ             0x0025
#define QMINAS_GET_HOME_NETWORK_RESP            0x0025
#define QMINAS_GET_PREFERRED_NETWORK_REQ        0x0026
#define QMINAS_GET_PREFERRED_NETWORK_RESP       0x0026
#define QMINAS_SET_PREFERRED_NETWORK_REQ        0x0027
#define QMINAS_SET_PREFERRED_NETWORK_RESP       0x0027
#define QMINAS_GET_FORBIDDEN_NETWORK_REQ        0x0028
#define QMINAS_GET_FORBIDDEN_NETWORK_RESP       0x0028
#define QMINAS_SET_FORBIDDEN_NETWORK_REQ        0x0029
#define QMINAS_SET_FORBIDDEN_NETWORK_RESP       0x0029
#define QMINAS_SET_TECHNOLOGY_PREF_REQ          0x002A
#define QMINAS_SET_TECHNOLOGY_PREF_RESP         0x002A
#define QMINAS_GET_RF_BAND_INFO_REQ             0x0031
#define QMINAS_GET_RF_BAND_INFO_RESP            0x0031
#define QMINAS_GET_CELL_LOCATION_INFO_REQ  0x0043
#define QMINAS_GET_CELL_LOCATION_INFO_RESP 0x0043
#define QMINAS_GET_PLMN_NAME_REQ                0x0044
#define QMINAS_GET_PLMN_NAME_RESP               0x0044
#define QUECTEL_PACKET_TRANSFER_START_IND 0X100
#define QUECTEL_PACKET_TRANSFER_END_IND 0X101
#define QMINAS_GET_SYS_INFO_REQ                 0x004D
#define QMINAS_GET_SYS_INFO_RESP                0x004D
#define QMINAS_SYS_INFO_IND                     0x004E
#define QMINAS_GET_SIG_INFO_REQ                 0x004F
#define QMINAS_GET_SIG_INFO_RESP                0x004F

typedef struct _QMINAS_GET_HOME_NETWORK_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
} __attribute__ ((packed)) QMINAS_GET_HOME_NETWORK_REQ_MSG, *PQMINAS_GET_HOME_NETWORK_REQ_MSG;

typedef struct _HOME_NETWORK_SYSTEMID
{
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT SystemID;
   USHORT NetworkID;
} __attribute__ ((packed)) HOME_NETWORK_SYSTEMID, *PHOME_NETWORK_SYSTEMID;

typedef struct _HOME_NETWORK
{
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT MobileCountryCode;
   USHORT MobileNetworkCode;
   UCHAR  NetworkDesclen;
   UCHAR  NetworkDesc;
} __attribute__ ((packed)) HOME_NETWORK, *PHOME_NETWORK;

#if 0
typedef struct _HOME_NETWORK_EXT
{
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT MobileCountryCode;
   USHORT MobileNetworkCode;
   UCHAR  NetworkDescDisp;
   UCHAR  NetworkDescEncoding;
   UCHAR  NetworkDesclen;
   UCHAR  NetworkDesc;
} HOME_NETWORK_EXT, *PHOME_NETWORK_EXT;

typedef struct _QMINAS_GET_HOME_NETWORK_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
} QMINAS_GET_HOME_NETWORK_RESP_MSG, *PQMINAS_GET_HOME_NETWORK_RESP_MSG;

typedef struct _QMINAS_GET_PREFERRED_NETWORK_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
} QMINAS_GET_PREFERRED_NETWORK_REQ_MSG, *PQMINAS_GET_PREFERRED_NETWORK_REQ_MSG;


typedef struct _PREFERRED_NETWORK
{
   USHORT MobileCountryCode;
   USHORT MobileNetworkCode;
   USHORT RadioAccess;
} PREFERRED_NETWORK, *PPREFERRED_NETWORK;

typedef struct _QMINAS_GET_PREFERRED_NETWORK_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;      // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;       // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
   UCHAR  TLV2Type;         // 0x01 - required parameter
   USHORT TLV2Length;       // length of the mfr string
   USHORT NumPreferredNetwork;
} QMINAS_GET_PREFERRED_NETWORK_RESP_MSG, *PQMINAS_GET_PREFERRED_NETWORK_RESP_MSG;

typedef struct _QMINAS_GET_FORBIDDEN_NETWORK_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
} QMINAS_GET_FORBIDDEN_NETWORK_REQ_MSG, *PQMINAS_GET_FORBIDDEN_NETWORK_REQ_MSG;

typedef struct _FORBIDDEN_NETWORK
{
   USHORT MobileCountryCode;
   USHORT MobileNetworkCode;
} FORBIDDEN_NETWORK, *PFORBIDDEN_NETWORK;

typedef struct _QMINAS_GET_FORBIDDEN_NETWORK_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;      // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;       // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
   UCHAR  TLV2Type;         // 0x01 - required parameter
   USHORT TLV2Length;       // length of the mfr string
   USHORT NumForbiddenNetwork;
} QMINAS_GET_FORBIDDEN_NETWORK_RESP_MSG, *PQMINAS_GET_FORBIDDEN_NETWORK_RESP_MSG;

typedef struct _QMINAS_GET_SERVING_SYSTEM_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
} QMINAS_GET_SERVING_SYSTEM_REQ_MSG, *PQMINAS_GET_SERVING_SYSTEM_REQ_MSG;

typedef struct _QMINAS_ROAMING_INDICATOR_MSG
{
   UCHAR  TLVType;         // 0x01 - required parameter
   USHORT TLVLength;       // length of the mfr string
   UCHAR  RoamingIndicator;
} QMINAS_ROAMING_INDICATOR_MSG, *PQMINAS_ROAMING_INDICATOR_MSG;
#endif

typedef struct _QMINAS_DATA_CAP
{
   UCHAR  TLVType;         // 0x01 - required parameter
   USHORT TLVLength;       // length of the mfr string
   UCHAR  DataCapListLen;
   UCHAR  DataCap;
} __attribute__ ((packed)) QMINAS_DATA_CAP, *PQMINAS_DATA_CAP;

typedef struct _QMINAS_CURRENT_PLMN_MSG
{
   UCHAR  TLVType;         // 0x01 - required parameter
   USHORT TLVLength;       // length of the mfr string
   USHORT MobileCountryCode;
   USHORT MobileNetworkCode;
   UCHAR  NetworkDesclen;
   UCHAR  NetworkDesc;
} __attribute__ ((packed)) QMINAS_CURRENT_PLMN_MSG, *PQMINAS_CURRENT_PLMN_MSG;

typedef struct _QMINAS_GET_SERVING_SYSTEM_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
} __attribute__ ((packed)) QMINAS_GET_SERVING_SYSTEM_RESP_MSG, *PQMINAS_GET_SERVING_SYSTEM_RESP_MSG;

typedef struct _SERVING_SYSTEM
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  RegistrationState;
   UCHAR  CSAttachedState;
   UCHAR  PSAttachedState;
   UCHAR  RegistredNetwork;
   UCHAR  InUseRadioIF;
   UCHAR  RadioIF;
} __attribute__ ((packed)) SERVING_SYSTEM, *PSERVING_SYSTEM;

typedef struct _QMINAS_GET_SYS_INFO_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
} __attribute__ ((packed)) QMINAS_GET_SYS_INFO_RESP_MSG, *PQMINAS_GET_SYS_INFO_RESP_MSG;

typedef struct _QMINAS_SYS_INFO_IND_MSG
{
   USHORT Type;
   USHORT Length;
} __attribute__ ((packed)) QMINAS_SYS_INFO_IND_MSG, *PQMINAS_SYS_INFO_IND_MSG;

typedef struct _SERVICE_STATUS_INFO
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  SrvStatus;
   UCHAR  true_srv_status;
   UCHAR  IsPrefDataPath;
} __attribute__ ((packed)) SERVICE_STATUS_INFO, *PSERVICE_STATUS_INFO;

typedef struct _CDMA_SYSTEM_INFO
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  SrvDomainValid;
   UCHAR  SrvDomain;
   UCHAR  SrvCapabilityValid;
   UCHAR  SrvCapability;
   UCHAR  RoamStatusValid;
   UCHAR  RoamStatus;
   UCHAR  IsSysForbiddenValid;
   UCHAR  IsSysForbidden;
   UCHAR  IsSysPrlMatchValid;
   UCHAR  IsSysPrlMatch;
   UCHAR  PRevInUseValid;
   UCHAR  PRevInUse;
   UCHAR  BSPRevValid;
   UCHAR  BSPRev;
   UCHAR  CCSSupportedValid;
   UCHAR  CCSSupported;
   UCHAR  CDMASysIdValid;
   USHORT SID;
   USHORT NID;
   UCHAR  BSInfoValid;
   USHORT BaseID;
   ULONG  BaseLAT;
   ULONG  BaseLONG;
   UCHAR  PacketZoneValid;
   USHORT PacketZone;
   UCHAR  NetworkIdValid;
   UCHAR  MCC[3];
   UCHAR  MNC[3];
} __attribute__ ((packed)) CDMA_SYSTEM_INFO, *PCDMA_SYSTEM_INFO;

typedef struct _HDR_SYSTEM_INFO
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  SrvDomainValid;
   UCHAR  SrvDomain;
   UCHAR  SrvCapabilityValid;
   UCHAR  SrvCapability;
   UCHAR  RoamStatusValid;
   UCHAR  RoamStatus;
   UCHAR  IsSysForbiddenValid;
   UCHAR  IsSysForbidden;
   UCHAR  IsSysPrlMatchValid;
   UCHAR  IsSysPrlMatch;
   UCHAR  HdrPersonalityValid;
   UCHAR  HdrPersonality;
   UCHAR  HdrActiveProtValid;
   UCHAR  HdrActiveProt;
   UCHAR  is856SysIdValid;
   UCHAR  is856SysId[16];
} __attribute__ ((packed)) HDR_SYSTEM_INFO, *PHDR_SYSTEM_INFO;

typedef struct _GSM_SYSTEM_INFO
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  SrvDomainValid;
   UCHAR  SrvDomain;
   UCHAR  SrvCapabilityValid;
   UCHAR  SrvCapability;
   UCHAR  RoamStatusValid;
   UCHAR  RoamStatus;
   UCHAR  IsSysForbiddenValid;
   UCHAR  IsSysForbidden;
   UCHAR  LacValid;
   USHORT Lac;
   UCHAR  CellIdValid;
   ULONG  CellId;
   UCHAR  RegRejectInfoValid;
   UCHAR  RejectSrvDomain;
   UCHAR  RejCause;
   UCHAR  NetworkIdValid;
   UCHAR  MCC[3];
   UCHAR  MNC[3];
   UCHAR  EgprsSuppValid;
   UCHAR  EgprsSupp;
   UCHAR  DtmSuppValid;
   UCHAR  DtmSupp;
} __attribute__ ((packed)) GSM_SYSTEM_INFO, *PGSM_SYSTEM_INFO;

typedef struct _WCDMA_SYSTEM_INFO
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  SrvDomainValid;
   UCHAR  SrvDomain;
   UCHAR  SrvCapabilityValid;
   UCHAR  SrvCapability;
   UCHAR  RoamStatusValid;
   UCHAR  RoamStatus;
   UCHAR  IsSysForbiddenValid;
   UCHAR  IsSysForbidden;
   UCHAR  LacValid;
   USHORT Lac;
   UCHAR  CellIdValid;
   ULONG  CellId;
   UCHAR  RegRejectInfoValid;
   UCHAR  RejectSrvDomain;
   UCHAR  RejCause;
   UCHAR  NetworkIdValid;
   UCHAR  MCC[3];
   UCHAR  MNC[3];
   UCHAR  HsCallStatusValid;
   UCHAR  HsCallStatus;
   UCHAR  HsIndValid;
   UCHAR  HsInd;
   UCHAR  PscValid;
   UCHAR  Psc;
} __attribute__ ((packed)) WCDMA_SYSTEM_INFO, *PWCDMA_SYSTEM_INFO;

typedef struct _LTE_SYSTEM_INFO
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  SrvDomainValid;
   UCHAR  SrvDomain;
   UCHAR  SrvCapabilityValid;
   UCHAR  SrvCapability;
   UCHAR  RoamStatusValid;
   UCHAR  RoamStatus;
   UCHAR  IsSysForbiddenValid;
   UCHAR  IsSysForbidden;
   UCHAR  LacValid;
   USHORT Lac;
   UCHAR  CellIdValid;
   ULONG  CellId;
   UCHAR  RegRejectInfoValid;
   UCHAR  RejectSrvDomain;
   UCHAR  RejCause;
   UCHAR  NetworkIdValid;
   UCHAR  MCC[3];
   UCHAR  MNC[3];
   UCHAR  TacValid;
   USHORT Tac;
} __attribute__ ((packed)) LTE_SYSTEM_INFO, *PLTE_SYSTEM_INFO;

typedef struct _TDSCDMA_SYSTEM_INFO
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  SrvDomainValid;
   UCHAR  SrvDomain;
   UCHAR  SrvCapabilityValid;
   UCHAR  SrvCapability;
   UCHAR  RoamStatusValid;
   UCHAR  RoamStatus;
   UCHAR  IsSysForbiddenValid;
   UCHAR  IsSysForbidden;
   UCHAR  LacValid;
   USHORT Lac;
   UCHAR  CellIdValid;
   ULONG  CellId;
   UCHAR  RegRejectInfoValid;
   UCHAR  RejectSrvDomain;
   UCHAR  RejCause;
   UCHAR  NetworkIdValid;
   UCHAR  MCC[3];
   UCHAR  MNC[3];
   UCHAR  HsCallStatusValid;
   UCHAR  HsCallStatus;
   UCHAR  HsIndValid;
   UCHAR  HsInd;
   UCHAR  CellParameterIdValid;
   USHORT CellParameterId;
   UCHAR  CellBroadcastCapValid;
   ULONG  CellBroadcastCap;
   UCHAR  CsBarStatusValid;
   ULONG  CsBarStatus;
   UCHAR  PsBarStatusValid;
   ULONG  PsBarStatus;
   UCHAR  CipherDomainValid;
   UCHAR  CipherDomain;
} __attribute__ ((packed)) TDSCDMA_SYSTEM_INFO, *PTDSCDMA_SYSTEM_INFO;

typedef enum {
  NAS_SYS_SRV_STATUS_NO_SRV_V01 = 0, 
  NAS_SYS_SRV_STATUS_LIMITED_V01 = 1, 
  NAS_SYS_SRV_STATUS_SRV_V01 = 2, 
  NAS_SYS_SRV_STATUS_LIMITED_REGIONAL_V01 = 3, 
  NAS_SYS_SRV_STATUS_PWR_SAVE_V01 = 4, 
}nas_service_status_enum_type_v01;

typedef enum {
  SYS_SRV_DOMAIN_NO_SRV_V01 = 0, 
  SYS_SRV_DOMAIN_CS_ONLY_V01 = 1, 
  SYS_SRV_DOMAIN_PS_ONLY_V01 = 2, 
  SYS_SRV_DOMAIN_CS_PS_V01 = 3, 
  SYS_SRV_DOMAIN_CAMPED_V01 = 4, 
}nas_service_domain_enum_type_v01;

typedef enum {
    QMI_NAS_RADIO_INTERFACE_UNKNOWN     = -1,
    QMI_NAS_RADIO_INTERFACE_NONE        = 0x00,
    QMI_NAS_RADIO_INTERFACE_CDMA_1X     = 0x01,
    QMI_NAS_RADIO_INTERFACE_CDMA_1XEVDO = 0x02,
    QMI_NAS_RADIO_INTERFACE_AMPS        = 0x03,
    QMI_NAS_RADIO_INTERFACE_GSM         = 0x04,
    QMI_NAS_RADIO_INTERFACE_UMTS        = 0x05,
    QMI_NAS_RADIO_INTERFACE_LTE         = 0x08,
    QMI_NAS_RADIO_INTERFACE_TD_SCDMA    = 0x09,
    QMI_NAS_RADIO_INTERFACE_5GNR        = 0x0C,
} QMI_NAS_RADIO_INTERFACE_E;

typedef enum {
    QMI_NAS_ACTIVE_BAND_BC_0 = 0,
    QMI_NAS_ACTIVE_BAND_BC_1 = 1,
    QMI_NAS_ACTIVE_BAND_BC_2 = 2,
    QMI_NAS_ACTIVE_BAND_BC_3 = 3,
    QMI_NAS_ACTIVE_BAND_BC_4 = 4,
    QMI_NAS_ACTIVE_BAND_BC_5 = 5,
    QMI_NAS_ACTIVE_BAND_BC_6 = 6,
    QMI_NAS_ACTIVE_BAND_BC_7 = 7,
    QMI_NAS_ACTIVE_BAND_BC_8 = 8,
    QMI_NAS_ACTIVE_BAND_BC_9 = 9,
    QMI_NAS_ACTIVE_BAND_BC_10 = 10,
    QMI_NAS_ACTIVE_BAND_BC_11 = 11,
    QMI_NAS_ACTIVE_BAND_BC_12 = 12,
    QMI_NAS_ACTIVE_BAND_BC_13 = 13,
    QMI_NAS_ACTIVE_BAND_BC_14 = 14,
    QMI_NAS_ACTIVE_BAND_BC_15 = 15,
    QMI_NAS_ACTIVE_BAND_BC_16 = 16,
    QMI_NAS_ACTIVE_BAND_BC_17 = 17,
    QMI_NAS_ACTIVE_BAND_BC_18 = 18,
    QMI_NAS_ACTIVE_BAND_BC_19 = 19,
    QMI_NAS_ACTIVE_BAND_GSM_450 = 40,
    QMI_NAS_ACTIVE_BAND_GSM_480 = 41,
    QMI_NAS_ACTIVE_BAND_GSM_750 = 42,
    QMI_NAS_ACTIVE_BAND_GSM_850 = 43,
    QMI_NAS_ACTIVE_BAND_GSM_900_EXTENDED = 44,
    QMI_NAS_ACTIVE_BAND_GSM_900_PRIMARY = 45,
    QMI_NAS_ACTIVE_BAND_GSM_900_RAILWAYS = 46,
    QMI_NAS_ACTIVE_BAND_GSM_DCS_1800 = 47,
    QMI_NAS_ACTIVE_BAND_GSM_PCS_1900 = 48,
    QMI_NAS_ACTIVE_BAND_WCDMA_2100 = 80,
    QMI_NAS_ACTIVE_BAND_WCDMA_PCS_1900 = 81,
    QMI_NAS_ACTIVE_BAND_WCDMA_DCS_1800 = 82,
    QMI_NAS_ACTIVE_BAND_WCDMA_1700_US = 83,
    QMI_NAS_ACTIVE_BAND_WCDMA_850 = 84,
    QMI_NAS_ACTIVE_BAND_WCDMA_800 = 85,
    QMI_NAS_ACTIVE_BAND_WCDMA_2600 = 86,
    QMI_NAS_ACTIVE_BAND_WCDMA_900 = 87,
    QMI_NAS_ACTIVE_BAND_WCDMA_1700_JAPAN = 88,
    QMI_NAS_ACTIVE_BAND_WCDMA_1500_JAPAN = 90,
    QMI_NAS_ACTIVE_BAND_WCDMA_850_JAPAN = 91,
    QMI_NAS_ACTIVE_BAND_EUTRAN_1 = 120,
    QMI_NAS_ACTIVE_BAND_EUTRAN_2 = 121,
    QMI_NAS_ACTIVE_BAND_EUTRAN_3 = 122,
    QMI_NAS_ACTIVE_BAND_EUTRAN_4 = 123,
    QMI_NAS_ACTIVE_BAND_EUTRAN_5 = 124,
    QMI_NAS_ACTIVE_BAND_EUTRAN_6 = 125,
    QMI_NAS_ACTIVE_BAND_EUTRAN_7 = 126,
    QMI_NAS_ACTIVE_BAND_EUTRAN_8 = 127,
    QMI_NAS_ACTIVE_BAND_EUTRAN_9 = 128,
    QMI_NAS_ACTIVE_BAND_EUTRAN_10 = 129,
    QMI_NAS_ACTIVE_BAND_EUTRAN_11 = 130,
    QMI_NAS_ACTIVE_BAND_EUTRAN_12 = 131,
    QMI_NAS_ACTIVE_BAND_EUTRAN_13 = 132,
    QMI_NAS_ACTIVE_BAND_EUTRAN_14 = 133,
    QMI_NAS_ACTIVE_BAND_EUTRAN_17 = 134,
    QMI_NAS_ACTIVE_BAND_EUTRAN_18 = 143,
    QMI_NAS_ACTIVE_BAND_EUTRAN_19 = 144,
    QMI_NAS_ACTIVE_BAND_EUTRAN_20 = 145,
    QMI_NAS_ACTIVE_BAND_EUTRAN_21 = 146,
    QMI_NAS_ACTIVE_BAND_EUTRAN_23 = 152,
    QMI_NAS_ACTIVE_BAND_EUTRAN_24 = 147,
    QMI_NAS_ACTIVE_BAND_EUTRAN_25 = 148,
    QMI_NAS_ACTIVE_BAND_EUTRAN_26 = 153,
    QMI_NAS_ACTIVE_BAND_EUTRAN_27 = 164,
    QMI_NAS_ACTIVE_BAND_EUTRAN_28 = 158,
    QMI_NAS_ACTIVE_BAND_EUTRAN_29 = 159,
    QMI_NAS_ACTIVE_BAND_EUTRAN_30 = 160,
    QMI_NAS_ACTIVE_BAND_EUTRAN_31 = 165,
    QMI_NAS_ACTIVE_BAND_EUTRAN_32 = 154,
    QMI_NAS_ACTIVE_BAND_EUTRAN_33 = 135,
    QMI_NAS_ACTIVE_BAND_EUTRAN_34 = 136,
    QMI_NAS_ACTIVE_BAND_EUTRAN_35 = 137,
    QMI_NAS_ACTIVE_BAND_EUTRAN_36 = 138,
    QMI_NAS_ACTIVE_BAND_EUTRAN_37 = 139,
    QMI_NAS_ACTIVE_BAND_EUTRAN_38 = 140,
    QMI_NAS_ACTIVE_BAND_EUTRAN_39 = 141,
    QMI_NAS_ACTIVE_BAND_EUTRAN_40 = 142,
    QMI_NAS_ACTIVE_BAND_EUTRAN_41 = 149,
    QMI_NAS_ACTIVE_BAND_EUTRAN_42 = 150,
    QMI_NAS_ACTIVE_BAND_EUTRAN_43 = 151,
    QMI_NAS_ACTIVE_BAND_EUTRAN_46 = 163,
    QMI_NAS_ACTIVE_BAND_EUTRAN_47 = 166,
    QMI_NAS_ACTIVE_BAND_EUTRAN_48 = 167,
    QMI_NAS_ACTIVE_BAND_EUTRAN_66 = 161,
    QMI_NAS_ACTIVE_BAND_EUTRAN_71 = 168,
    QMI_NAS_ACTIVE_BAND_EUTRAN_125 = 155,
    QMI_NAS_ACTIVE_BAND_EUTRAN_126 = 156,
    QMI_NAS_ACTIVE_BAND_EUTRAN_127 = 157,
    QMI_NAS_ACTIVE_BAND_EUTRAN_250 = 162,
    QMI_NAS_ACTIVE_BAND_TDSCDMA_A = 200,
    QMI_NAS_ACTIVE_BAND_TDSCDMA_B = 201,
    QMI_NAS_ACTIVE_BAND_TDSCDMA_C = 202,
    QMI_NAS_ACTIVE_BAND_TDSCDMA_D = 203,
    QMI_NAS_ACTIVE_BAND_TDSCDMA_E = 204,
    QMI_NAS_ACTIVE_BAND_TDSCDMA_F = 205,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_1  = 250,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_2  = 251,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_3  = 252,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_5  = 253,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_7  = 254,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_8  = 255,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_20 = 256,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_28 = 257,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_38 = 258,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_41 = 259,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_50 = 260,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_51 = 261,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_66 = 262,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_70 = 263,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_71 = 264,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_74 = 265,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_75 = 266,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_76 = 267,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_77 = 268,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_78 = 269,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_79 = 270,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_80 = 271,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_81 = 272,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_82 = 273,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_83 = 274,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_84 = 275,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_85 = 276,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_257= 277,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_258= 278,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_259= 279,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_260= 280,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_261= 281,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_12 = 282,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_25 = 283,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_34 = 284,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_39 = 285,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_40 = 286,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_65 = 287,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_86 = 288,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_48 = 289,
    QMI_NAS_ACTIVE_BAND_NR5G_BAND_14 = 290
} QMI_NAS_ACTIVE_BAND_E;

typedef struct {
  UCHAR  TLVType;
  USHORT TLVLength;

  uint8_t srv_domain_valid;
  uint8_t srv_domain;
  uint8_t srv_capability_valid;
  uint8_t srv_capability;
  uint8_t roam_status_valid;
  uint8_t roam_status;
  uint8_t is_sys_forbidden_valid;
  uint8_t is_sys_forbidden;

  uint8_t lac_valid;
  uint16_t lac;
  uint8_t cell_id_valid;
  uint32_t cell_id;
  uint8_t reg_reject_info_valid;
  uint8_t reject_srv_domain;
  uint8_t rej_cause;
  uint8_t network_id_valid;
  UCHAR MCC[3];
  UCHAR MNC[3];

  uint8_t tac_valid;
  uint16_t tac;
} __attribute__ ((packed)) NR5G_SYSTEM_INFO, *PNR5G_SYSTEM_INFO;

#if 0
typedef struct _QMINAS_SERVING_SYSTEM_IND_MSG
{
   USHORT Type;
   USHORT Length;
} QMINAS_SERVING_SYSTEM_IND_MSG, *PQMINAS_SERVING_SYSTEM_IND_MSG;

typedef struct _QMINAS_SET_PREFERRED_NETWORK_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT NumPreferredNetwork;
   USHORT MobileCountryCode;
   USHORT MobileNetworkCode;
   USHORT RadioAccess;
} QMINAS_SET_PREFERRED_NETWORK_REQ_MSG, *PQMINAS_SET_PREFERRED_NETWORK_REQ_MSG;

typedef struct _QMINAS_SET_PREFERRED_NETWORK_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;      // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;       // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
} QMINAS_SET_PREFERRED_NETWORK_RESP_MSG, *PQMINAS_SET_PREFERRED_NETWORK_RESP_MSG;

typedef struct _QMINAS_SET_FORBIDDEN_NETWORK_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT NumForbiddenNetwork;
   USHORT MobileCountryCode;
   USHORT MobileNetworkCode;
} QMINAS_SET_FORBIDDEN_NETWORK_REQ_MSG, *PQMINAS_SET_FORBIDDEN_NETWORK_REQ_MSG;

typedef struct _QMINAS_SET_FORBIDDEN_NETWORK_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;      // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;       // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
} QMINAS_SET_FORBIDDEN_NETWORK_RESP_MSG, *PQMINAS_SET_FORBIDDEN_NETWORK_RESP_MSG;

typedef struct _QMINAS_PERFORM_NETWORK_SCAN_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
} QMINAS_PERFORM_NETWORK_SCAN_REQ_MSG, *PQMINAS_PERFORM_NETWORK_SCAN_REQ_MSG;

typedef struct _VISIBLE_NETWORK
{
   USHORT MobileCountryCode;
   USHORT MobileNetworkCode;
   UCHAR  NetworkStatus;
   UCHAR  NetworkDesclen;
} VISIBLE_NETWORK, *PVISIBLE_NETWORK;

typedef struct _QMINAS_PERFORM_NETWORK_SCAN_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;      // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;       // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
} QMINAS_PERFORM_NETWORK_SCAN_RESP_MSG, *PQMINAS_PERFORM_NETWORK_SCAN_RESP_MSG;

typedef struct _QMINAS_PERFORM_NETWORK_SCAN_NETWORK_INFO
{
   UCHAR  TLVType;         // 0x010 - required parameter
   USHORT TLVLength;       // length
   USHORT NumNetworkInstances;
} QMINAS_PERFORM_NETWORK_SCAN_NETWORK_INFO, *PQMINAS_PERFORM_NETWORK_SCAN_NETWORK_INFO;

typedef struct _QMINAS_PERFORM_NETWORK_SCAN_RAT_INFO
{
   UCHAR  TLVType;         // 0x011 - required parameter
   USHORT TLVLength;       // length
   USHORT NumInst;
} QMINAS_PERFORM_NETWORK_SCAN_RAT_INFO, *PQMINAS_PERFORM_NETWORK_SCAN_RAT_INFO;

typedef struct _QMINAS_PERFORM_NETWORK_SCAN_RAT
{
   USHORT MCC;
   USHORT MNC;
   UCHAR  RAT;
} QMINAS_PERFORM_NETWORK_SCAN_RAT, *PQMINAS_PERFORM_NETWORK_SCAN_RAT;


typedef struct _QMINAS_MANUAL_NW_REGISTER
{
   UCHAR  TLV2Type;          // 0x02 - result code
   USHORT TLV2Length;        // 4
   USHORT MobileCountryCode;
   USHORT MobileNetworkCode;
   UCHAR  RadioAccess;
} QMINAS_MANUAL_NW_REGISTER, *PQMINAS_MANUAL_NW_REGISTER;

typedef struct _QMINAS_INITIATE_NW_REGISTER_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   UCHAR  RegisterAction;
} QMINAS_INITIATE_NW_REGISTER_REQ_MSG, *PQMINAS_INITIATE_NW_REGISTER_REQ_MSG;

typedef struct _QMINAS_INITIATE_NW_REGISTER_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;      // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;       // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
} QMINAS_INITIATE_NW_REGISTER_RESP_MSG, *PQMINAS_INITIATE_NW_REGISTER_RESP_MSG;

typedef struct _QMINAS_SET_TECHNOLOGY_PREF_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT TechPref;
   UCHAR  Duration;
} QMINAS_SET_TECHNOLOGY_PREF_REQ_MSG, *PQMINAS_SET_TECHNOLOGY_PREF_REQ_MSG;

typedef struct _QMINAS_SET_TECHNOLOGY_PREF_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;      // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;       // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
} QMINAS_SET_TECHNOLOGY_PREF_RESP_MSG, *PQMINAS_SET_TECHNOLOGY_PREF_RESP_MSG;

typedef struct _QMINAS_GET_SIGNAL_STRENGTH_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
} QMINAS_GET_SIGNAL_STRENGTH_REQ_MSG, *PQMINAS_GET_SIGNAL_STRENGTH_REQ_MSG;

typedef struct _QMINAS_SIGNAL_STRENGTH
{
   CHAR   SigStrength;
   UCHAR  RadioIf;
} QMINAS_SIGNAL_STRENGTH, *PQMINAS_SIGNAL_STRENGTH;

typedef struct _QMINAS_SIGNAL_STRENGTH_LIST
{
   UCHAR  TLV3Type;
   USHORT TLV3Length;
   USHORT NumInstance;
} QMINAS_SIGNAL_STRENGTH_LIST, *PQMINAS_SIGNAL_STRENGTH_LIST;


typedef struct _QMINAS_GET_SIGNAL_STRENGTH_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;      // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;       // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
   UCHAR  TLV2Type;
   USHORT TLV2Length;
   CHAR   SignalStrength;
   UCHAR  RadioIf;
} QMINAS_GET_SIGNAL_STRENGTH_RESP_MSG, *PQMINAS_GET_SIGNAL_STRENGTH_RESP_MSG;


typedef struct _QMINAS_SET_EVENT_REPORT_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  ReportSigStrength;
   UCHAR  NumTresholds;
   CHAR   TresholdList[2];
} QMINAS_SET_EVENT_REPORT_REQ_MSG, *PQMINAS_SET_EVENT_REPORT_REQ_MSG;

typedef struct _QMINAS_SET_EVENT_REPORT_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;      // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;       // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
} QMINAS_SET_EVENT_REPORT_RESP_MSG, *PQMINAS_SET_EVENT_REPORT_RESP_MSG;

typedef struct _QMINAS_SIGNAL_STRENGTH_TLV
{
   UCHAR  TLVType;
   USHORT TLVLength;
   CHAR   SigStrength;
   UCHAR  RadioIf;
} QMINAS_SIGNAL_STRENGTH_TLV, *PQMINAS_SIGNAL_STRENGTH_TLV;

typedef struct _QMINAS_REJECT_CAUSE_TLV
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  ServiceDomain;
   USHORT RejectCause;
} QMINAS_REJECT_CAUSE_TLV, *PQMINAS_REJECT_CAUSE_TLV;

typedef struct _QMINAS_EVENT_REPORT_IND_MSG
{
   USHORT Type;
   USHORT Length;
} QMINAS_EVENT_REPORT_IND_MSG, *PQMINAS_EVENT_REPORT_IND_MSG;

typedef struct _QMINAS_GET_RF_BAND_INFO_REQ_MSG
{
   USHORT Type;
   USHORT Length;
} QMINAS_GET_RF_BAND_INFO_REQ_MSG, *PQMINAS_GET_RF_BAND_INFO_REQ_MSG;

typedef struct _QMINASRF_BAND_INFO
{
   UCHAR  RadioIf;
   USHORT ActiveBand;
   USHORT ActiveChannel;
} QMINASRF_BAND_INFO, *PQMINASRF_BAND_INFO;

typedef struct _QMINAS_GET_RF_BAND_INFO_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;      // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;       // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
   UCHAR  TLV2Type;
   USHORT TLV2Length;
   UCHAR  NumInstances;
} QMINAS_GET_RF_BAND_INFO_RESP_MSG, *PQMINAS_GET_RF_BAND_INFO_RESP_MSG;


typedef struct _QMINAS_GET_PLMN_NAME_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT MCC;
   USHORT MNC;
} QMINAS_GET_PLMN_NAME_REQ_MSG, *PQMINAS_GET_PLMN_NAME_REQ_MSG;

typedef struct _QMINAS_GET_PLMN_NAME_RESP_MSG
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
} QMINAS_GET_PLMN_NAME_RESP_MSG, *PQMINAS_GET_PLMN_NAME_RESP_MSG;

typedef struct _QMINAS_GET_PLMN_NAME_SPN
{
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  SPN_Enc;
   UCHAR  SPN_Len;
} QMINAS_GET_PLMN_NAME_SPN, *PQMINAS_GET_PLMN_NAME_SPN;

typedef struct _QMINAS_GET_PLMN_NAME_PLMN
{
   UCHAR  PLMN_Enc;
   UCHAR  PLMN_Ci;
   UCHAR  PLMN_SpareBits;
   UCHAR  PLMN_Len;
} QMINAS_GET_PLMN_NAME_PLMN, *PQMINAS_GET_PLMN_NAME_PLMN;

typedef struct _QMINAS_INITIATE_ATTACH_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  PsAttachAction;
} QMINAS_INITIATE_ATTACH_REQ_MSG, *PQMINAS_INITIATE_ATTACH_REQ_MSG;

typedef struct _QMINAS_INITIATE_ATTACH_RESP_MSG
{
   USHORT Type;             // QMUX type 0x0003
   USHORT Length;
   UCHAR  TLVType;          // 0x02 - result code
   USHORT TLVLength;        // 4
   USHORT QMUXResult;      // QMI_RESULT_SUCCESS
                            // QMI_RESULT_FAILURE
   USHORT QMUXError;       // QMI_ERR_INVALID_ARG
                            // QMI_ERR_NO_MEMORY
                            // QMI_ERR_INTERNAL
                            // QMI_ERR_FAULT
} QMINAS_INITIATE_ATTACH_RESP_MSG, *PQMINAS_INITIATE_ATTACH_RESP_MSG;
#endif
typedef struct {
    UCHAR  TLVType;
    USHORT TLVLength;
    CHAR rssi;
    SHORT ecio;
} __attribute__ ((packed)) QMINAS_SIG_INFO_CDMA_TLV_MSG, *PQMINAS_SIG_INFO_CDMA_TLV_MSG;

typedef struct {
    UCHAR  TLVType;
    USHORT TLVLength;
    CHAR rssi;
    SHORT ecio;
    CHAR sinr;
    INT io;
} __attribute__ ((packed)) QMINAS_SIG_INFO_HDR_TLV_MSG, *PQMINAS_SIG_INFO_HDR_TLV_MSG;

typedef struct {
    UCHAR  TLVType;
    USHORT TLVLength;
    CHAR rssi;
} __attribute__ ((packed)) QMINAS_SIG_INFO_GSM_TLV_MSG, *PQMINAS_SIG_INFO_GSM_TLV_MSG;

typedef struct {
    UCHAR  TLVType;
    USHORT TLVLength;
    CHAR rssi;
    SHORT ecio;
} __attribute__ ((packed)) QMINAS_SIG_INFO_WCDMA_TLV_MSG, *PQMINAS_SIG_INFO_WCDMA_TLV_MSG;

typedef struct {
    UCHAR  TLVType;
    USHORT TLVLength;
    CHAR rssi;
    CHAR rsrq;
    SHORT rsrp;
    SHORT snr;
} __attribute__ ((packed)) QMINAS_SIG_INFO_LTE_TLV_MSG, *PQMINAS_SIG_INFO_LTE_TLV_MSG;

typedef struct {
    UCHAR  TLVType;
    USHORT TLVLength;
    CHAR rscp;
} __attribute__ ((packed)) QMINAS_SIG_INFO_TDSCDMA_TLV_MSG, *PQMINAS_SIG_INFO_TDSCDMA_TLV_MSG;

typedef struct {
    UCHAR  TLVType;
    USHORT TLVLength;
    SHORT rsrp;
    SHORT snr;
} __attribute__ ((packed)) QMINAS_SIG_INFO_5G_NSA_TLV_MSG, *PQMINAS_SIG_INFO_5G_NSA_TLV_MSG;

typedef struct {
    UCHAR  TLVType;
    USHORT TLVLength;
    SHORT nr5g_rsrq;
} __attribute__ ((packed)) QMINAS_SIG_INFO_5G_SA_TLV_MSG, *PQMINAS_SIG_INFO_5G_SA_TLV_MSG;

typedef struct {
    uint8 radio_if;
    uint16 active_band;
    uint16 active_channel;
} __attribute__ ((packed)) NasGetRfBandInfo;

typedef struct {
    UCHAR  TLVType;
    USHORT TLVLength;

    uint8 num_instances;
    NasGetRfBandInfo bands_array[0];
} __attribute__ ((packed)) NasGetRfBandInfoList;

typedef struct {
    uint8 radio_if;
    uint16 dedicated_band;
} __attribute__ ((packed)) NasGetRfBandInfoDedicated;

typedef struct {
    UCHAR  TLVType;
    USHORT TLVLength;

    uint8 num_instances;
    NasGetRfBandInfoDedicated bands_array[0];
} __attribute__ ((packed)) NasGetRfBandInfoDedicatedList;

typedef struct {
    uint8 radio_if;
    uint16 active_band;
    uint32 active_channel;
} __attribute__ ((packed)) NasGetRfBandInfoExtended;

typedef struct {
    UCHAR  TLVType;
    USHORT TLVLength;

    uint8 num_instances;
    NasGetRfBandInfoExtended bands_array[0];
} __attribute__ ((packed)) NasGetRfBandInfoExtendedList;

typedef struct {
    uint8 radio_if;
    uint32 bandwidth;
} __attribute__ ((packed)) NasGetRfBandInfoBandWidth;

typedef struct {
    UCHAR  TLVType;
    USHORT TLVLength;

    uint8 num_instances;
    NasGetRfBandInfoBandWidth bands_array[0];
} __attribute__ ((packed)) NasGetRfBandInfoBandWidthList;

typedef struct {
    UCHAR  TLVType;
    USHORT TLVLength;

    uint8 plmn[3];
    uint8 tac[3];
    uint64 global_cell_id;
    uint16 physical_cell_id;
    int16 rsrq;
    int16 rsrp;
    int16 snr;
} __attribute__ ((packed)) NasGetCellLocationNr5gServingCell;

typedef struct {
    uint16 physical_cell_id;
    int16 rsrq;
    int16 rsrp;
    int16 rssi;
    int16 cell_selection_rx_level;
} __attribute__ ((packed)) NasGetCellLocationLteInfoCell;

typedef struct {
    UCHAR  TLVType;
    USHORT TLVLength;

    uint8 ue_in_idle;
    uint8 plmn[3];
    uint16 tracking_area_code;
    uint32 global_cell_id;
    uint16 absolute_rf_channel_number;
    uint16 serving_cell_id;
    uint8 cell_reselection_priority;
    uint8 s_non_intra_search_threshold;
    uint8 serving_cell_low_threshold;
    uint8 s_intra_search_threshold;
    uint8 cells_len;
    NasGetCellLocationLteInfoCell cells_array[0];
} __attribute__ ((packed)) NasGetCellLocationLteInfoIntrafrequency;

typedef struct _QmiMessageNasGetCellLocationInfoOutputInterfrequencyLteInfoFrequencyElement {
    uint16 eutra_absolute_rf_channel_number;
    uint8 cell_selection_rx_level_low_threshold;
    uint8 cell_selection_rx_level_high_threshold;
    uint8 cell_reselection_priority;
    uint8 cells_len;
    NasGetCellLocationLteInfoCell cells_array[0];
} __attribute__ ((packed)) NasGetCellLocationLteInfoInterfrequencyFrequencyElement;

typedef struct {
    UCHAR  TLVType;
    USHORT TLVLength;

    uint8 ue_in_idle;
    uint8 freqs_len;
    NasGetCellLocationLteInfoInterfrequencyFrequencyElement freqs[0];
} __attribute__ ((packed)) NasGetCellLocationLteInfoInterfrequency;

// ======================= End of NAS ==============================

// ======================= UIM ==============================
#define QMIUIM_READ_TRANSPARENT_REQ      0x0020
#define QMIUIM_READ_TRANSPARENT_RESP     0x0020
#define QMIUIM_READ_TRANSPARENT_IND      0x0020
#define QMIUIM_READ_RECORD_REQ           0x0021
#define QMIUIM_READ_RECORD_RESP          0x0021
#define QMIUIM_READ_RECORD_IND           0x0021
#define QMIUIM_WRITE_TRANSPARENT_REQ     0x0022
#define QMIUIM_WRITE_TRANSPARENT_RESP    0x0022
#define QMIUIM_WRITE_TRANSPARENT_IND     0x0022
#define QMIUIM_WRITE_RECORD_REQ          0x0023
#define QMIUIM_WRITE_RECORD_RESP         0x0023
#define QMIUIM_WRITE_RECORD_IND          0x0023
#define QMIUIM_SET_PIN_PROTECTION_REQ    0x0025
#define QMIUIM_SET_PIN_PROTECTION_RESP   0x0025
#define QMIUIM_SET_PIN_PROTECTION_IND    0x0025
#define QMIUIM_VERIFY_PIN_REQ            0x0026
#define QMIUIM_VERIFY_PIN_RESP           0x0026
#define QMIUIM_VERIFY_PIN_IND            0x0026
#define QMIUIM_UNBLOCK_PIN_REQ           0x0027
#define QMIUIM_UNBLOCK_PIN_RESP          0x0027
#define QMIUIM_UNBLOCK_PIN_IND           0x0027
#define QMIUIM_CHANGE_PIN_REQ            0x0028
#define QMIUIM_CHANGE_PIN_RESP           0x0028
#define QMIUIM_CHANGE_PIN_IND            0x0028
#define QMIUIM_DEPERSONALIZATION_REQ     0x0029
#define QMIUIM_DEPERSONALIZATION_RESP    0x0029
#define QMIUIM_EVENT_REG_REQ             0x002E
#define QMIUIM_EVENT_REG_RESP            0x002E
#define QMIUIM_GET_CARD_STATUS_REQ       0x002F
#define QMIUIM_GET_CARD_STATUS_RESP      0x002F
#define QMIUIM_STATUS_CHANGE_IND         0x0032
#define QMIUIM_POWER_DOWN                0x0030
#define QMIUIM_POWER_UP                  0x0031


typedef struct _QMIUIM_GET_CARD_STATUS_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
} __attribute__ ((packed)) QMIUIM_GET_CARD_STATUS_RESP_MSG, *PQMIUIM_GET_CARD_STATUS_RESP_MSG;

#define UIM_CARD_STATE_ABSENT     0x00
#define UIM_CARD_STATE_PRESENT    0x01
#define UIM_CARD_STATE_ERROR      0x02

typedef struct _QMIUIM_CARD_STATUS
{
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT IndexGWPri;
   USHORT Index1XPri;
   USHORT IndexGWSec;
   USHORT Index1XSec;
   UCHAR  NumSlot;
   UCHAR  CardState;
   UCHAR  UPINState;
   UCHAR  UPINRetries;
   UCHAR  UPUKRetries;
   UCHAR  ErrorCode;
   UCHAR  NumApp;
   UCHAR  AppType;
   UCHAR  AppState;
   UCHAR  PersoState;
   UCHAR  PersoFeature;
   UCHAR  PersoRetries;
   UCHAR  PersoUnblockRetries;
   UCHAR  AIDLength;
} __attribute__ ((packed)) QMIUIM_CARD_STATUS, *PQMIUIM_CARD_STATUS;

typedef struct _QMIUIM_PIN_STATE
{
   UCHAR  UnivPIN;
   UCHAR  PIN1State;
   UCHAR  PIN1Retries;
   UCHAR  PUK1Retries;
   UCHAR  PIN2State;
   UCHAR  PIN2Retries;
   UCHAR  PUK2Retries;
} __attribute__ ((packed)) QMIUIM_PIN_STATE, *PQMIUIM_PIN_STATE;

typedef struct _QMIUIM_VERIFY_PIN_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  Session_Type;
   UCHAR  Aid_Len;
   UCHAR  TLV2Type;
   USHORT TLV2Length;
   UCHAR  PINID;
   UCHAR  PINLen;
   UCHAR  PINValue;
} __attribute__ ((packed)) QMIUIM_VERIFY_PIN_REQ_MSG, *PQMIUIM_VERIFY_PIN_REQ_MSG;

typedef struct _QMIUIM_VERIFY_PIN_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
   UCHAR  TLV2Type;
   USHORT TLV2Length;
   UCHAR  PINVerifyRetriesLeft;
   UCHAR  PINUnblockRetriesLeft;
} __attribute__ ((packed)) QMIUIM_VERIFY_PIN_RESP_MSG, *PQMIUIM_VERIFY_PIN_RESP_MSG;

typedef struct _QMIUIM_READ_TRANSPARENT_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR  Session_Type;
   UCHAR  Aid_Len;
   UCHAR  TLV2Type;
   USHORT TLV2Length;
   USHORT file_id;
   UCHAR  path_len;
   UCHAR  path[];
} __attribute__ ((packed)) QMIUIM_READ_TRANSPARENT_REQ_MSG, *PQMIUIM_READ_TRANSPARENT_REQ_MSG;

typedef struct _READ_TRANSPARENT_TLV
{
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT Offset;
   USHORT Length;
} __attribute__ ((packed)) READ_TRANSPARENT_TLV, *PREAD_TRANSPARENT_TLV;

typedef struct _QMIUIM_CONTENT
{
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT content_len;
   UCHAR  content[];
} __attribute__ ((packed)) QMIUIM_CONTENT, *PQMIUIM_CONTENT;

typedef struct _QMIUIM_READ_TRANSPARENT_RESP_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   USHORT QMUXResult;
   USHORT QMUXError;
} __attribute__ ((packed)) QMIUIM_READ_TRANSPARENT_RESP_MSG, *PQMIUIM_READ_TRANSPARENT_RESP_MSG;

typedef struct _QMIUIM_SET_CARD_SLOT_REQ_MSG
{
   USHORT Type;
   USHORT Length;
   UCHAR  TLVType;
   USHORT TLVLength;
   UCHAR slot;
} __attribute__ ((packed)) QMIUIM_SET_CARD_SLOT_REQ_MSG, *PQMIUIM_SET_CARD_SLOT_REQ_MSG;

// ======================= COEX ==============================
#define QMI_COEX_GET_WWAN_STATE_REQ 0x22
#define QMI_COEX_GET_WWAN_STATE_RESP 0x22

typedef struct {

  uint32_t freq;
  /**<   Band center frequency in MHz. */

  uint32_t bandwidth;
  /**<   Bandwidth in MHz. */
}coex_band_type_v01;  /* Type */

typedef struct _QMI_COEX_GET_WWAN_STATE_RESP_MSG_LTE_BAND
{
   UCHAR  TLVType;
   USHORT TLVLength;
   coex_band_type_v01 ul_band;
   coex_band_type_v01  dl_band;
} __attribute__ ((packed)) QMI_COEX_GET_WWAN_STATE_RESP_MSG_LTE_BAND, *PQMI_COEX_GET_WWAN_STATE_RESP_MSG_LTE_BAND;


typedef struct _QMUX_MSG
{
   QCQMUX_HDR QMUXHdr;
   union
   {
      // Message Header
      QCQMUX_MSG_HDR                           QMUXMsgHdr;
      QCQMUX_MSG_HDR_RESP                      QMUXMsgHdrResp;

      // QMIWDS Message
#if 0
      QMIWDS_GET_PKT_SRVC_STATUS_REQ_MSG        PacketServiceStatusReq;
      QMIWDS_GET_PKT_SRVC_STATUS_RESP_MSG       PacketServiceStatusRsp;
      QMIWDS_GET_PKT_SRVC_STATUS_IND_MSG        PacketServiceStatusInd;
      QMIWDS_EVENT_REPORT_IND_MSG               EventReportInd;
      QMIWDS_GET_CURRENT_CHANNEL_RATE_REQ_MSG   GetCurrChannelRateReq;
      QMIWDS_GET_CURRENT_CHANNEL_RATE_RESP_MSG  GetCurrChannelRateRsp;
      QMIWDS_GET_PKT_STATISTICS_REQ_MSG         GetPktStatsReq;
      QMIWDS_GET_PKT_STATISTICS_RESP_MSG        GetPktStatsRsp;
      QMIWDS_SET_EVENT_REPORT_REQ_MSG           EventReportReq;
      QMIWDS_SET_EVENT_REPORT_RESP_MSG          EventReportRsp;
#endif
      //#ifdef QC_IP_MODE
      QMIWDS_GET_RUNTIME_SETTINGS_REQ_MSG       GetRuntimeSettingsReq;
      QMIWDS_GET_RUNTIME_SETTINGS_RESP_MSG      GetRuntimeSettingsRsp;
      //#endif // QC_IP_MODE
      QMIWDS_SET_CLIENT_IP_FAMILY_PREF_REQ_MSG  SetClientIpFamilyPrefReq;
      QMIWDS_SET_CLIENT_IP_FAMILY_PREF_RESP_MSG SetClientIpFamilyPrefResp;
      QMIWDS_SET_AUTO_CONNECT_REQ_MSG SetAutoConnectReq;
#if 0
      QMIWDS_GET_MIP_MODE_REQ_MSG               GetMipModeReq;
      QMIWDS_GET_MIP_MODE_RESP_MSG              GetMipModeResp;
#endif
      QMIWDS_START_NETWORK_INTERFACE_REQ_MSG    StartNwInterfaceReq;
      QMIWDS_START_NETWORK_INTERFACE_RESP_MSG   StartNwInterfaceResp;
      QMIWDS_STOP_NETWORK_INTERFACE_REQ_MSG     StopNwInterfaceReq;
      QMIWDS_STOP_NETWORK_INTERFACE_RESP_MSG    StopNwInterfaceResp;
      QMIWDS_GET_DEFAULT_SETTINGS_REQ_MSG       GetDefaultSettingsReq;
      QMIWDS_GET_DEFAULT_SETTINGS_RESP_MSG      GetDefaultSettingsResp;
      QMIWDS_MODIFY_PROFILE_SETTINGS_REQ_MSG    ModifyProfileSettingsReq;
      QMIWDS_MODIFY_PROFILE_SETTINGS_RESP_MSG   ModifyProfileSettingsResp;
      QMIWDS_GET_PROFILE_SETTINGS_REQ_MSG       GetProfileSettingsReq;
      QMIWDS_CREATE_PROFILE_SETTINGS_REQ_MSG    CreatetProfileSettingsReq;
      QMIWDS_GET_PROFILE_LIST_REQ_MSG           GetProfileListReq;
      QMIWDS_GET_PROFILE_LIST_RESP_MSG          GetProfileListResp;
#if 0
      QMIWDS_GET_DATA_BEARER_REQ_MSG            GetDataBearerReq;
      QMIWDS_GET_DATA_BEARER_RESP_MSG           GetDataBearerResp;
      QMIWDS_DUN_CALL_INFO_REQ_MSG              DunCallInfoReq;
      QMIWDS_DUN_CALL_INFO_RESP_MSG             DunCallInfoResp;
#endif
      QMIWDS_BIND_MUX_DATA_PORT_REQ_MSG         BindMuxDataPortReq;

      // QMIDMS Messages
#if 0
      QMIDMS_GET_DEVICE_MFR_REQ_MSG             GetDeviceMfrReq;
      QMIDMS_GET_DEVICE_MFR_RESP_MSG            GetDeviceMfrRsp;
      QMIDMS_GET_DEVICE_MODEL_ID_REQ_MSG        GetDeviceModeIdReq;
      QMIDMS_GET_DEVICE_MODEL_ID_RESP_MSG       GetDeviceModeIdRsp;
      QMIDMS_GET_DEVICE_REV_ID_REQ_MSG          GetDeviceRevIdReq;
      QMIDMS_GET_DEVICE_REV_ID_RESP_MSG         GetDeviceRevIdRsp;
      QMIDMS_GET_MSISDN_REQ_MSG                 GetMsisdnReq;
      QMIDMS_GET_MSISDN_RESP_MSG                GetMsisdnRsp;
      QMIDMS_GET_DEVICE_SERIAL_NUMBERS_REQ_MSG  GetDeviceSerialNumReq;
      QMIDMS_GET_DEVICE_SERIAL_NUMBERS_RESP_MSG GetDeviceSerialNumRsp;
      QMIDMS_GET_DEVICE_CAP_REQ_MSG             GetDeviceCapReq;
      QMIDMS_GET_DEVICE_CAP_RESP_MSG            GetDeviceCapResp;
      QMIDMS_GET_BAND_CAP_REQ_MSG               GetBandCapReq;
      QMIDMS_GET_BAND_CAP_RESP_MSG              GetBandCapRsp;
      QMIDMS_GET_ACTIVATED_STATUS_REQ_MSG       GetActivatedStatusReq;
      QMIDMS_GET_ACTIVATED_STATUS_RESP_MSG      GetActivatedStatusResp;
      QMIDMS_GET_OPERATING_MODE_REQ_MSG         GetOperatingModeReq;
      QMIDMS_GET_OPERATING_MODE_RESP_MSG        GetOperatingModeResp;
#endif
      QMIDMS_SET_OPERATING_MODE_REQ_MSG         SetOperatingModeReq;
      QMIDMS_SET_OPERATING_MODE_RESP_MSG        SetOperatingModeResp;
#if 0
      QMIDMS_UIM_GET_ICCID_REQ_MSG              GetICCIDReq;
      QMIDMS_UIM_GET_ICCID_RESP_MSG             GetICCIDResp;
      QMIDMS_ACTIVATE_AUTOMATIC_REQ_MSG         ActivateAutomaticReq;
      QMIDMS_ACTIVATE_AUTOMATIC_RESP_MSG        ActivateAutomaticResp;
      QMIDMS_ACTIVATE_MANUAL_REQ_MSG            ActivateManualReq;
      QMIDMS_ACTIVATE_MANUAL_RESP_MSG           ActivateManualResp;
#endif
      QMIDMS_UIM_GET_PIN_STATUS_REQ_MSG         UIMGetPinStatusReq;
      QMIDMS_UIM_GET_PIN_STATUS_RESP_MSG        UIMGetPinStatusResp;
      QMIDMS_UIM_VERIFY_PIN_REQ_MSG             UIMVerifyPinReq;
      QMIDMS_UIM_VERIFY_PIN_RESP_MSG            UIMVerifyPinResp;
#if 0
      QMIDMS_UIM_SET_PIN_PROTECTION_REQ_MSG     UIMSetPinProtectionReq;
      QMIDMS_UIM_SET_PIN_PROTECTION_RESP_MSG    UIMSetPinProtectionResp;
      QMIDMS_UIM_CHANGE_PIN_REQ_MSG             UIMChangePinReq;
      QMIDMS_UIM_CHANGE_PIN_RESP_MSG            UIMChangePinResp;
      QMIDMS_UIM_UNBLOCK_PIN_REQ_MSG            UIMUnblockPinReq;
      QMIDMS_UIM_UNBLOCK_PIN_RESP_MSG           UIMUnblockPinResp;
      QMIDMS_SET_EVENT_REPORT_REQ_MSG           DmsSetEventReportReq;
      QMIDMS_SET_EVENT_REPORT_RESP_MSG          DmsSetEventReportResp;
      QMIDMS_EVENT_REPORT_IND_MSG               DmsEventReportInd;
#endif
      QMIDMS_UIM_GET_STATE_REQ_MSG              UIMGetStateReq;
      QMIDMS_UIM_GET_STATE_RESP_MSG             UIMGetStateResp;
      QMIDMS_UIM_GET_IMSI_REQ_MSG               UIMGetIMSIReq;
      QMIDMS_UIM_GET_IMSI_RESP_MSG              UIMGetIMSIResp;
#if 0
      QMIDMS_UIM_GET_CK_STATUS_REQ_MSG          UIMGetCkStatusReq;
      QMIDMS_UIM_GET_CK_STATUS_RESP_MSG         UIMGetCkStatusResp;
      QMIDMS_UIM_SET_CK_PROTECTION_REQ_MSG      UIMSetCkProtectionReq;
      QMIDMS_UIM_SET_CK_PROTECTION_RESP_MSG     UIMSetCkProtectionResp;
      QMIDMS_UIM_UNBLOCK_CK_REQ_MSG             UIMUnblockCkReq;
      QMIDMS_UIM_UNBLOCK_CK_RESP_MSG            UIMUnblockCkResp;
#endif

      // QMIQOS Messages
#if 1
      QMI_QOS_SET_EVENT_REPORT_REQ_MSG          QosSetEventReportReq;
      QMI_QOS_SET_EVENT_REPORT_RESP_MSG         QosSetEventReportRsp;
      QMI_QOS_SET_EVENT_REPORT_IND_MSG          QosSetEventReportInd;
	  QMI_QOS_BIND_DATA_PORT_REQ_MSG			QosBindDataPortReq;
	  QMI_QOS_BIND_DATA_PORT_RESP_MSG			QosBindDataPortRsp;
	  QMI_QOS_INDICATION_REGISTER_REQ_MSG		QosIndRegReq;
	  QMI_QOS_INDICATION_REGISTER_RESP_MSG		QosIndRegRsp;
	  QMI_QOS_GLOBAL_QOS_FLOW_IND_MSG			QosGlobalQosFlowInd;
	  QMI_QOS_GET_QOS_INFO_REQ_MSG				QosGetQosInfoReq;
	  QMI_QOS_GET_QOS_INFO_RESP_MSG				QosGetQosInfoRsp;
#endif

      // QMIWMS Messages
#if 0
      QMIWMS_GET_MESSAGE_PROTOCOL_REQ_MSG       GetMessageProtocolReq;
      QMIWMS_GET_MESSAGE_PROTOCOL_RESP_MSG      GetMessageProtocolResp;
      QMIWMS_GET_SMSC_ADDRESS_REQ_MSG           GetSMSCAddressReq;
      QMIWMS_GET_SMSC_ADDRESS_RESP_MSG          GetSMSCAddressResp;
      QMIWMS_SET_SMSC_ADDRESS_REQ_MSG           SetSMSCAddressReq;
      QMIWMS_SET_SMSC_ADDRESS_RESP_MSG          SetSMSCAddressResp;
      QMIWMS_GET_STORE_MAX_SIZE_REQ_MSG         GetStoreMaxSizeReq;
      QMIWMS_GET_STORE_MAX_SIZE_RESP_MSG        GetStoreMaxSizeResp;
      QMIWMS_LIST_MESSAGES_REQ_MSG              ListMessagesReq;
      QMIWMS_LIST_MESSAGES_RESP_MSG             ListMessagesResp;
      QMIWMS_RAW_READ_REQ_MSG                   RawReadMessagesReq;
      QMIWMS_RAW_READ_RESP_MSG                  RawReadMessagesResp;
      QMIWMS_SET_EVENT_REPORT_REQ_MSG           WmsSetEventReportReq;
      QMIWMS_SET_EVENT_REPORT_RESP_MSG          WmsSetEventReportResp;
      QMIWMS_EVENT_REPORT_IND_MSG               WmsEventReportInd;
      QMIWMS_DELETE_REQ_MSG                     WmsDeleteReq;
      QMIWMS_DELETE_RESP_MSG                    WmsDeleteResp;
      QMIWMS_RAW_SEND_REQ_MSG                   RawSendMessagesReq;
      QMIWMS_RAW_SEND_RESP_MSG                  RawSendMessagesResp;
      QMIWMS_MODIFY_TAG_REQ_MSG                 WmsModifyTagReq;
      QMIWMS_MODIFY_TAG_RESP_MSG                WmsModifyTagResp;
#endif

      // QMINAS Messages
#if 0
      QMINAS_GET_HOME_NETWORK_REQ_MSG           GetHomeNetworkReq;
      QMINAS_GET_HOME_NETWORK_RESP_MSG          GetHomeNetworkResp;
      QMINAS_GET_PREFERRED_NETWORK_REQ_MSG      GetPreferredNetworkReq;
      QMINAS_GET_PREFERRED_NETWORK_RESP_MSG     GetPreferredNetworkResp;
      QMINAS_GET_FORBIDDEN_NETWORK_REQ_MSG      GetForbiddenNetworkReq;
      QMINAS_GET_FORBIDDEN_NETWORK_RESP_MSG     GetForbiddenNetworkResp;
      QMINAS_GET_SERVING_SYSTEM_REQ_MSG         GetServingSystemReq;
#endif
      QMINAS_GET_SERVING_SYSTEM_RESP_MSG        GetServingSystemResp;
      QMINAS_GET_SYS_INFO_RESP_MSG              GetSysInfoResp;
      QMINAS_SYS_INFO_IND_MSG                   NasSysInfoInd;
#if 0
      QMINAS_SERVING_SYSTEM_IND_MSG             NasServingSystemInd;
      QMINAS_SET_PREFERRED_NETWORK_REQ_MSG      SetPreferredNetworkReq;
      QMINAS_SET_PREFERRED_NETWORK_RESP_MSG     SetPreferredNetworkResp;
      QMINAS_SET_FORBIDDEN_NETWORK_REQ_MSG      SetForbiddenNetworkReq;
      QMINAS_SET_FORBIDDEN_NETWORK_RESP_MSG     SetForbiddenNetworkResp;
      QMINAS_PERFORM_NETWORK_SCAN_REQ_MSG       PerformNetworkScanReq;
      QMINAS_PERFORM_NETWORK_SCAN_RESP_MSG      PerformNetworkScanResp;
      QMINAS_INITIATE_NW_REGISTER_REQ_MSG       InitiateNwRegisterReq;
      QMINAS_INITIATE_NW_REGISTER_RESP_MSG      InitiateNwRegisterResp;
      QMINAS_SET_TECHNOLOGY_PREF_REQ_MSG        SetTechnologyPrefReq;
      QMINAS_SET_TECHNOLOGY_PREF_RESP_MSG       SetTechnologyPrefResp;
      QMINAS_GET_SIGNAL_STRENGTH_REQ_MSG        GetSignalStrengthReq;
      QMINAS_GET_SIGNAL_STRENGTH_RESP_MSG       GetSignalStrengthResp;
      QMINAS_SET_EVENT_REPORT_REQ_MSG           SetEventReportReq;
      QMINAS_SET_EVENT_REPORT_RESP_MSG          SetEventReportResp;
      QMINAS_EVENT_REPORT_IND_MSG               NasEventReportInd;
      QMINAS_GET_RF_BAND_INFO_REQ_MSG           GetRFBandInfoReq;
      QMINAS_GET_RF_BAND_INFO_RESP_MSG          GetRFBandInfoResp;
      QMINAS_INITIATE_ATTACH_REQ_MSG            InitiateAttachReq;
      QMINAS_INITIATE_ATTACH_RESP_MSG           InitiateAttachResp;
      QMINAS_GET_PLMN_NAME_REQ_MSG              GetPLMNNameReq;
      QMINAS_GET_PLMN_NAME_RESP_MSG             GetPLMNNameResp;
#endif

      // QMIUIM Messages
      QMIUIM_GET_CARD_STATUS_RESP_MSG           UIMGetCardStatus;
      QMIUIM_VERIFY_PIN_REQ_MSG                 UIMUIMVerifyPinReq;
      QMIUIM_VERIFY_PIN_RESP_MSG                UIMUIMVerifyPinResp;
#if 0
      QMIUIM_SET_PIN_PROTECTION_REQ_MSG         UIMUIMSetPinProtectionReq;
      QMIUIM_SET_PIN_PROTECTION_RESP_MSG        UIMUIMSetPinProtectionResp;
      QMIUIM_CHANGE_PIN_REQ_MSG                 UIMUIMChangePinReq;
      QMIUIM_CHANGE_PIN_RESP_MSG                UIMUIMChangePinResp;
      QMIUIM_UNBLOCK_PIN_REQ_MSG                UIMUIMUnblockPinReq;
      QMIUIM_UNBLOCK_PIN_RESP_MSG               UIMUIMUnblockPinResp;
#endif
      QMIUIM_READ_TRANSPARENT_REQ_MSG           UIMUIMReadTransparentReq;
      QMIUIM_READ_TRANSPARENT_RESP_MSG          UIMUIMReadTransparentResp;
      QMIUIM_SET_CARD_SLOT_REQ_MSG                 UIMSetCardSlotReq;

      QMIWDS_ADMIN_SET_DATA_FORMAT_REQ_MSG      SetDataFormatReq;
      QMI_WDA_SET_LOOPBACK_CONFIG_REQ_MSG       SetLoopBackReq;
      QMI_WDA_SET_LOOPBACK_CONFIG_IND_MSG       SetLoopBackInd;	  
   };
} __attribute__ ((packed)) QMUX_MSG, *PQMUX_MSG;

typedef struct _QCQMIMSG {
    QCQMI_HDR QMIHdr;
    union {
        QMICTL_MSG CTLMsg;
        QMUX_MSG MUXMsg;
    };
} __attribute__ ((packed)) QCQMIMSG, *PQCQMIMSG;

#pragma pack(pop)

#endif // QCQMUX_H

