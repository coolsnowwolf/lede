/*===========================================================================

			    M P Q M U X. H
DESCRIPTION:

    This file provides support for QMUX.

INITIALIZATION AND SEQUENCING REQUIREMENTS:

Copyright (C) 2011 by Qualcomm Technologies, Incorporated.  All Rights Reserved.
===========================================================================*/

#ifndef MPQMUX_H
#define MPQMUX_H

#include "MPQMI.h"

#pragma pack(push, 1)

#define QMIWDS_SET_EVENT_REPORT_REQ 0x0001
#define QMIWDS_SET_EVENT_REPORT_RESP 0x0001
#define QMIWDS_EVENT_REPORT_IND 0x0001
#define QMIWDS_START_NETWORK_INTERFACE_REQ 0x0020
#define QMIWDS_START_NETWORK_INTERFACE_RESP 0x0020
#define QMIWDS_STOP_NETWORK_INTERFACE_REQ 0x0021
#define QMIWDS_STOP_NETWORK_INTERFACE_RESP 0x0021
#define QMIWDS_GET_PKT_SRVC_STATUS_REQ 0x0022
#define QMIWDS_GET_PKT_SRVC_STATUS_RESP 0x0022
#define QMIWDS_GET_PKT_SRVC_STATUS_IND 0x0022
#define QMIWDS_GET_CURRENT_CHANNEL_RATE_REQ 0x0023
#define QMIWDS_GET_CURRENT_CHANNEL_RATE_RESP 0x0023
#define QMIWDS_GET_PKT_STATISTICS_REQ 0x0024
#define QMIWDS_GET_PKT_STATISTICS_RESP 0x0024
//begin modified by zhangkaibo add create profile qmi. mantis 0049137,0048741  20200610
#define QMIWDS_CREATE_PROFILE_SETTINGS_REQ 0x0027
#define QMIWDS_CREATE_PROFILE_SETTINGS_RESP 0x0027
//end modified by zhangkaibo add create profile qmi. mantis 0049137,0048741  20200610
#define QMIWDS_MODIFY_PROFILE_SETTINGS_REQ 0x0028
#define QMIWDS_MODIFY_PROFILE_SETTINGS_RESP 0x0028
#define QMIWDS_GET_PROFILE_SETTINGS_REQ 0x002B
#define QMIWDS_GET_PROFILE_SETTINGS_RESP 0x002B
#define QMIWDS_GET_DEFAULT_SETTINGS_REQ 0x002C
#define QMIWDS_GET_DEFAULT_SETTINGS_RESP 0x002C
#define QMIWDS_GET_RUNTIME_SETTINGS_REQ 0x002D
#define QMIWDS_GET_RUNTIME_SETTINGS_RESP 0x002D
#define QMIWDS_GET_MIP_MODE_REQ 0x002F
#define QMIWDS_GET_MIP_MODE_RESP 0x002F
#define QMIWDS_GET_DATA_BEARER_REQ 0x0037
#define QMIWDS_GET_DATA_BEARER_RESP 0x0037
#define QMIWDS_DUN_CALL_INFO_REQ 0x0038
#define QMIWDS_DUN_CALL_INFO_RESP 0x0038
#define QMIWDS_DUN_CALL_INFO_IND 0x0038
#define QMIWDS_SET_CLIENT_IP_FAMILY_PREF_REQ 0x004D
#define QMIWDS_SET_CLIENT_IP_FAMILY_PREF_RESP 0x004D
#define QMIWDS_SET_AUTO_CONNECT_REQ 0x0051
#define QMIWDS_SET_AUTO_CONNECT_RESP 0x0051
#define QMIWDS_BIND_MUX_DATA_PORT_REQ 0x00A2
#define QMIWDS_BIND_MUX_DATA_PORT_RESP 0x00A2

// Stats masks
#define QWDS_STAT_MASK_TX_PKT_OK 0x00000001
#define QWDS_STAT_MASK_RX_PKT_OK 0x00000002
#define QWDS_STAT_MASK_TX_PKT_ER 0x00000004
#define QWDS_STAT_MASK_RX_PKT_ER 0x00000008
#define QWDS_STAT_MASK_TX_PKT_OF 0x00000010
#define QWDS_STAT_MASK_RX_PKT_OF 0x00000020

// TLV Types for xfer statistics
#define TLV_WDS_TX_GOOD_PKTS 0x10
#define TLV_WDS_RX_GOOD_PKTS 0x11
#define TLV_WDS_TX_ERROR 0x12
#define TLV_WDS_RX_ERROR 0x13
#define TLV_WDS_TX_OVERFLOW 0x14
#define TLV_WDS_RX_OVERFLOW 0x15
#define TLV_WDS_CHANNEL_RATE 0x16
#define TLV_WDS_DATA_BEARER 0x17
#define TLV_WDS_DORMANCY_STATUS 0x18

#define QWDS_PKT_DATA_DISCONNECTED 0x01
#define QWDS_PKT_DATA_CONNECTED 0x02
#define QWDS_PKT_DATA_SUSPENDED 0x03
#define QWDS_PKT_DATA_AUTHENTICATING 0x04

#define QMIWDS_ADMIN_SET_DATA_FORMAT_REQ 0x0020
#define QMIWDS_ADMIN_SET_DATA_FORMAT_RESP 0x0020
#define QMIWDS_ADMIN_GET_DATA_FORMAT_REQ 0x0021
#define QMIWDS_ADMIN_GET_DATA_FORMAT_RESP 0x0021
#define QMIWDS_ADMIN_SET_QMAP_SETTINGS_REQ 0x002B
#define QMIWDS_ADMIN_SET_QMAP_SETTINGS_RESP 0x002B
#define QMIWDS_ADMIN_GET_QMAP_SETTINGS_REQ 0x002C
#define QMIWDS_ADMIN_GET_QMAP_SETTINGS_RESP 0x002C

#define NETWORK_DESC_ENCODING_OCTET 0x00
#define NETWORK_DESC_ENCODING_EXTPROTOCOL 0x01
#define NETWORK_DESC_ENCODING_7BITASCII 0x02
#define NETWORK_DESC_ENCODING_IA5 0x03
#define NETWORK_DESC_ENCODING_UNICODE 0x04
#define NETWORK_DESC_ENCODING_SHIFTJIS 0x05
#define NETWORK_DESC_ENCODING_KOREAN 0x06
#define NETWORK_DESC_ENCODING_LATINH 0x07
#define NETWORK_DESC_ENCODING_LATIN 0x08
#define NETWORK_DESC_ENCODING_GSM7BIT 0x09
#define NETWORK_DESC_ENCODING_GSMDATA 0x0A
#define NETWORK_DESC_ENCODING_UNKNOWN 0xFF

typedef struct _QMIWDS_ADMIN_SET_DATA_FORMAT {
    USHORT Type;  // QMUX type 0x0000
    USHORT Length;
} __attribute__((packed)) QMIWDS_ADMIN_SET_DATA_FORMAT,
    *PQMIWDS_ADMIN_SET_DATA_FORMAT;

typedef struct _QMIWDS_ADMIN_SET_DATA_FORMAT_TLV_QOS {
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR QOSSetting;
} __attribute__((packed)) QMIWDS_ADMIN_SET_DATA_FORMAT_TLV_QOS,
    *PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV_QOS;

typedef struct _QMIWDS_ADMIN_SET_DATA_FORMAT_TLV {
    UCHAR TLVType;
    USHORT TLVLength;
    ULONG Value;
} __attribute__((packed)) QMIWDS_ADMIN_SET_DATA_FORMAT_TLV,
    *PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV;

typedef struct _QMIWDS_ENDPOINT_TLV {
    UCHAR TLVType;
    USHORT TLVLength;
    ULONG ep_type;
    ULONG iface_id;
} __attribute__((packed)) QMIWDS_ENDPOINT_TLV, *PQMIWDS_ENDPOINT_TLV;

typedef struct _QMIWDS_ADMIN_SET_DATA_FORMAT_REQ_MSG {
    USHORT Type;
    USHORT Length;
    QMIWDS_ADMIN_SET_DATA_FORMAT_TLV_QOS QosDataFormatTlv;
    QMIWDS_ADMIN_SET_DATA_FORMAT_TLV UnderlyingLinkLayerProtocolTlv;
    QMIWDS_ADMIN_SET_DATA_FORMAT_TLV UplinkDataAggregationProtocolTlv;
    QMIWDS_ADMIN_SET_DATA_FORMAT_TLV DownlinkDataAggregationProtocolTlv;
    QMIWDS_ADMIN_SET_DATA_FORMAT_TLV DownlinkDataAggregationMaxDatagramsTlv;
    QMIWDS_ADMIN_SET_DATA_FORMAT_TLV DownlinkDataAggregationMaxSizeTlv;
#if 0
    QMIWDS_ADMIN_SET_DATA_FORMAT_TLV UplinkDataAggregationMaxDatagramsTlv;
    QMIWDS_ADMIN_SET_DATA_FORMAT_TLV UplinkDataAggregationMaxSizeTlv;
#else
    QMIWDS_ENDPOINT_TLV epTlv;
#endif
} __attribute__((packed)) QMIWDS_ADMIN_SET_DATA_FORMAT_REQ_MSG,
    *PQMIWDS_ADMIN_SET_DATA_FORMAT_REQ_MSG;

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
#endif	// QCUSB_MUX_PROTOCOL

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
#endif	// QCUSB_MUX_PROTOCOL

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

#define QMIWDS_GET_RUNTIME_SETTINGS_MASK_IPV4DNS_ADDR 0x0010
#define QMIWDS_GET_RUNTIME_SETTINGS_MASK_IPV4_ADDR 0x0100
#define QMIWDS_GET_RUNTIME_SETTINGS_MASK_IPV4GATEWAY_ADDR 0x0200
#define QMIWDS_GET_RUNTIME_SETTINGS_MASK_MTU 0x2000

typedef struct _QMIWDS_GET_RUNTIME_SETTINGS_REQ_MSG {
    USHORT Type;  // QMIWDS_GET_RUNTIME_SETTINGS_REQ
    USHORT Length;
    UCHAR TLVType;     // 0x10
    USHORT TLVLength;  // 0x0004
    ULONG Mask;	       // mask, bit 8: IP addr -- 0x0100
} __attribute__((packed)) QMIWDS_GET_RUNTIME_SETTINGS_REQ_MSG,
    *PQMIWDS_GET_RUNTIME_SETTINGS_REQ_MSG;

typedef struct _QMIWDS_BIND_MUX_DATA_PORT_REQ_MSG {
    USHORT Type;
    USHORT Length;
    UCHAR TLVType;
    USHORT TLVLength;
    ULONG ep_type;
    ULONG iface_id;
    UCHAR TLV2Type;
    USHORT TLV2Length;
    UCHAR MuxId;
    UCHAR TLV3Type;
    USHORT TLV3Length;
    ULONG client_type;
} __attribute__((packed)) QMIWDS_BIND_MUX_DATA_PORT_REQ_MSG,
    *PQMIWDS_BIND_MUX_DATA_PORT_REQ_MSG;

#define QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV4PRIMARYDNS 0x15
#define QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV4SECONDARYDNS 0x16
#define QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV4 0x1E
#define QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV4GATEWAY 0x20
#define QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV4SUBNET 0x21

#define QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV6 0x25
#define QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV6GATEWAY 0x26
#define QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV6PRIMARYDNS 0x27
#define QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV6SECONDARYDNS 0x28
#define QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_MTU 0x29

typedef struct _QMIWDS_GET_RUNTIME_SETTINGS_TLV_MTU {
    UCHAR TLVType;     // QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_MTU
    USHORT TLVLength;  // 4
    ULONG Mtu;	       // MTU
} __attribute__((packed)) QMIWDS_GET_RUNTIME_SETTINGS_TLV_MTU,
    *PQMIWDS_GET_RUNTIME_SETTINGS_TLV_MTU;

typedef struct _QMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV4_ADDR {
    UCHAR TLVType;	// QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV4
    USHORT TLVLength;	// 4
    ULONG IPV4Address;	// address
} __attribute__((packed)) QMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV4_ADDR,
    *PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV4_ADDR;

typedef struct _QMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV6_ADDR {
    UCHAR TLVType;	    // QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV6
    USHORT TLVLength;	    // 16
    UCHAR IPV6Address[16];  // address
    UCHAR PrefixLength;	    // prefix length
} __attribute__((packed)) QMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV6_ADDR,
    *PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV6_ADDR;

    typedef struct _QMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV6_DNS_ADDR {
        UCHAR TLVType;      // QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV6
        USHORT TLVLength;       // 16
        UCHAR IPV6Address[16];  // address
    } __attribute__((packed)) QMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV6_DNS_ADDR,
        *PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV6_DNS_ADDR;

typedef struct _QMIWDS_GET_RUNTIME_SETTINGS_RESP_MSG {
    USHORT Type;  // QMIWDS_GET_RUNTIME_SETTINGS_RESP
    USHORT Length;
    UCHAR TLVType;	// QCTLV_TYPE_RESULT_CODE
    USHORT TLVLength;	// 0x0004
    USHORT QMUXResult;	// result code
    USHORT QMUXError;	// error code
} __attribute__((packed)) QMIWDS_GET_RUNTIME_SETTINGS_RESP_MSG,
    *PQMIWDS_GET_RUNTIME_SETTINGS_RESP_MSG;

//#endif // QC_IP_MODE

typedef struct _QMIWDS_IP_FAMILY_TLV {
    UCHAR TLVType;     // 0x12
    USHORT TLVLength;  // 1
    UCHAR IpFamily;    // IPV4-0x04, IPV6-0x06
} __attribute__((packed)) QMIWDS_IP_FAMILY_TLV, *PQMIWDS_IP_FAMILY_TLV;

typedef struct _QMIWDS_PKT_SRVC_TLV {
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR ConnectionStatus;
    UCHAR ReconfigReqd;
} __attribute__((packed)) QMIWDS_PKT_SRVC_TLV, *PQMIWDS_PKT_SRVC_TLV;

typedef struct _QMIWDS_CALL_END_REASON_TLV {
    UCHAR TLVType;
    USHORT TLVLength;
    USHORT CallEndReason;
} __attribute__((packed)) QMIWDS_CALL_END_REASON_TLV,
    *PQMIWDS_CALL_END_REASON_TLV;

typedef struct _QMIWDS_CALL_END_REASON_V_TLV {
    UCHAR TLVType;
    USHORT TLVLength;
    USHORT CallEndReasonType;
    USHORT CallEndReason;
} __attribute__((packed)) QMIWDS_CALL_END_REASON_V_TLV,
    *PQMIWDS_CALL_END_REASON_V_TLV;

typedef struct _QMIWDS_SET_CLIENT_IP_FAMILY_PREF_REQ_MSG {
    USHORT Type;  // QMUX type 0x004D
    USHORT Length;
    UCHAR TLVType;	 // 0x01
    USHORT TLVLength;	 // 1
    UCHAR IpPreference;	 // IPV4-0x04, IPV6-0x06
} __attribute__((packed)) QMIWDS_SET_CLIENT_IP_FAMILY_PREF_REQ_MSG,
    *PQMIWDS_SET_CLIENT_IP_FAMILY_PREF_REQ_MSG;

typedef struct _QMIWDS_SET_CLIENT_IP_FAMILY_PREF_RESP_MSG {
    USHORT Type;  // QMUX type 0x0037
    USHORT Length;
    UCHAR TLVType;	// 0x02
    USHORT TLVLength;	// 4
    USHORT QMUXResult;	// QMI_RESULT_SUCCESS, QMI_RESULT_FAILURE
    USHORT QMUXError;	// QMI_ERR_INTERNAL, QMI_ERR_MALFORMED_MSG,
			// QMI_ERR_INVALID_ARG
} __attribute__((packed)) QMIWDS_SET_CLIENT_IP_FAMILY_PREF_RESP_MSG,
    *PQMIWDS_SET_CLIENT_IP_FAMILY_PREF_RESP_MSG;

typedef struct _QMIWDS_SET_AUTO_CONNECT_REQ_MSG {
    USHORT Type;  // QMUX type 0x0051
    USHORT Length;
    UCHAR TLVType;		// 0x01
    USHORT TLVLength;		// 1
    UCHAR autoconnect_setting;	//  0x00 ?C Disabled, 0x01 ?C Enabled, 0x02 ?C
				//  Paused (resume on power cycle)
} __attribute__((packed)) QMIWDS_SET_AUTO_CONNECT_REQ_MSG,
    *PQMIWDS_SET_AUTO_CONNECT_REQ_MSG;

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

typedef struct _QMIWDS_TECHNOLOGY_PREFERECE {
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR TechPreference;
} __attribute__((packed)) QMIWDS_TECHNOLOGY_PREFERECE,
    *PQMIWDS_TECHNOLOGY_PREFERECE;

typedef struct _QMIWDS_PROFILE_IDENTIFIER {
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR ProfileIndex;
} __attribute__((packed)) QMIWDS_PROFILE_IDENTIFIER,
    *PQMIWDS_PROFILE_IDENTIFIER;

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

typedef struct _QMIWDS_PROFILENAME {
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR ProfileName;
} __attribute__((packed)) QMIWDS_PROFILENAME, *PQMIWDS_PROFILENAME;

typedef struct _QMIWDS_PDPTYPE {
    UCHAR TLVType;
    USHORT TLVLength;
    // 0 ?C PDP-IP (IPv4)
    // 1 ?C PDP-PPP
    // 2 ?C PDP-IPv6
    // 3 ?C PDP-IPv4v6
    UCHAR PdpType;
} __attribute__((packed)) QMIWDS_PDPTYPE, *PQMIWDS_PDPTYPE;

typedef struct _QMIWDS_USERNAME {
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR UserName;
} __attribute__((packed)) QMIWDS_USERNAME, *PQMIWDS_USERNAME;

typedef struct _QMIWDS_PASSWD {
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR Passwd;
} __attribute__((packed)) QMIWDS_PASSWD, *PQMIWDS_PASSWD;

typedef struct _QMIWDS_AUTH_PREFERENCE {
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR AuthPreference;
} __attribute__((packed)) QMIWDS_AUTH_PREFERENCE, *PQMIWDS_AUTH_PREFERENCE;

typedef struct _QMIWDS_APNNAME {
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR ApnName;
} __attribute__((packed)) QMIWDS_APNNAME, *PQMIWDS_APNNAME;

typedef struct _QMIWDS_AUTOCONNECT {
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR AutoConnect;
} __attribute__((packed)) QMIWDS_AUTOCONNECT, *PQMIWDS_AUTOCONNECT;

typedef struct _QMIWDS_START_NETWORK_INTERFACE_REQ_MSG {
    USHORT Type;
    USHORT Length;
} __attribute__((packed)) QMIWDS_START_NETWORK_INTERFACE_REQ_MSG,
    *PQMIWDS_START_NETWORK_INTERFACE_REQ_MSG;

typedef struct _QMIWDS_CALLENDREASON {
    UCHAR TLVType;
    USHORT TLVLength;
    USHORT Reason;
} __attribute__((packed)) QMIWDS_CALLENDREASON, *PQMIWDS_CALLENDREASON;

typedef struct _QMIWDS_START_NETWORK_INTERFACE_RESP_MSG {
    USHORT Type;  // QMUX type 0x0040
    USHORT Length;
    UCHAR TLVType;	// 0x02
    USHORT TLVLength;	// 4
    USHORT QMUXResult;	// QMI_RESULT_SUCCESS
			// QMI_RESULT_FAILURE
    USHORT QMUXError;	// QMI_ERR_INVALID_ARG
			// QMI_ERR_NO_MEMORY
			// QMI_ERR_INTERNAL
			// QMI_ERR_FAULT

    UCHAR TLV2Type;	// 0x01
    USHORT TLV2Length;	// 20
    ULONG Handle;	//
} __attribute__((packed)) QMIWDS_START_NETWORK_INTERFACE_RESP_MSG,
    *PQMIWDS_START_NETWORK_INTERFACE_RESP_MSG;

typedef struct _QMIWDS_STOP_NETWORK_INTERFACE_REQ_MSG {
    USHORT Type;
    USHORT Length;
    UCHAR TLVType;
    USHORT TLVLength;
    ULONG Handle;
} __attribute__((packed)) QMIWDS_STOP_NETWORK_INTERFACE_REQ_MSG,
    *PQMIWDS_STOP_NETWORK_INTERFACE_REQ_MSG;

typedef struct _QMIWDS_STOP_NETWORK_INTERFACE_RESP_MSG {
    USHORT Type;  // QMUX type 0x0040
    USHORT Length;
    UCHAR TLVType;	// 0x02
    USHORT TLVLength;	// 4
    USHORT QMUXResult;	// QMI_RESULT_SUCCESS
			// QMI_RESULT_FAILURE
    USHORT QMUXError;	// QMI_ERR_INVALID_ARG
			// QMI_ERR_NO_MEMORY
			// QMI_ERR_INTERNAL
			// QMI_ERR_FAULT

} __attribute__((packed)) QMIWDS_STOP_NETWORK_INTERFACE_RESP_MSG,
    *PQMIWDS_STOP_NETWORK_INTERFACE_RESP_MSG;

typedef struct _QMIWDS_GET_DEFAULT_SETTINGS_REQ_MSG {
    USHORT Type;
    USHORT Length;
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR ProfileType;
} __attribute__((packed)) QMIWDS_GET_DEFAULT_SETTINGS_REQ_MSG,
    *PQMIWDS_GET_DEFAULT_SETTINGS_REQ_MSG;

typedef struct _QMIWDS_GET_DEFAULT_SETTINGS_RESP_MSG {
    USHORT Type;
    USHORT Length;
    UCHAR TLVType;
    USHORT TLVLength;
    USHORT QMUXResult;
    USHORT QMUXError;
} __attribute__((packed)) QMIWDS_GET_DEFAULT_SETTINGS_RESP_MSG,
    *PQMIWDS_GET_DEFAULT_SETTINGS_RESP_MSG;
//begin modified by zhangkaibo add create profile qmi. mantis 0049137,0048741  20200610
typedef struct _QMIWDS_CREATE_PROFILE_SETTINGS_REQ_MSG {
    USHORT Type;
    USHORT Length;
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR ProfileType;
} __attribute__((packed)) QMIWDS_CREATE_PROFILE_SETTINGS_REQ_MSG,
    *PQMIWDS_CREATE_PROFILE_SETTINGS_REQ_MSG;


typedef struct _QMIWDS_CREATE_PROFILE_SETTINGS_RESP_MSG {
    USHORT Type;
    USHORT Length;
    UCHAR TLVType;
    USHORT TLVLength;
    USHORT QMUXResult;
    USHORT QMUXError;
} __attribute__((packed)) QMIWDS_CREATE_PROFILE_SETTINGS_RESP_MSG,
    *PQMIWDS_CREATE_PROFILE_SETTINGS_RESP_MSG;
//end modified by zhangkaibo add create profile qmi. mantis 0049137,0048741  20200610    
typedef struct _QMIWDS_MODIFY_PROFILE_SETTINGS_REQ_MSG {
    USHORT Type;
    USHORT Length;
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR ProfileType;
    UCHAR ProfileIndex;
} __attribute__((packed)) QMIWDS_MODIFY_PROFILE_SETTINGS_REQ_MSG,
    *PQMIWDS_MODIFY_PROFILE_SETTINGS_REQ_MSG;

typedef struct _QMIWDS_MODIFY_PROFILE_SETTINGS_RESP_MSG {
    USHORT Type;
    USHORT Length;
    UCHAR TLVType;
    USHORT TLVLength;
    USHORT QMUXResult;
    USHORT QMUXError;
} __attribute__((packed)) QMIWDS_MODIFY_PROFILE_SETTINGS_RESP_MSG,
    *PQMIWDS_MODIFY_PROFILE_SETTINGS_RESP_MSG;

typedef struct _QMIWDS_GET_PROFILE_SETTINGS_REQ_MSG {
    USHORT Type;
    USHORT Length;
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR ProfileType;
    UCHAR ProfileIndex;
} __attribute__((packed)) QMIWDS_GET_PROFILE_SETTINGS_REQ_MSG,
    *PQMIWDS_GET_PROFILE_SETTINGS_REQ_MSG;

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
#define QMIDMS_SET_EVENT_REPORT_REQ 0x0001
#define QMIDMS_SET_EVENT_REPORT_RESP 0x0001
#define QMIDMS_EVENT_REPORT_IND 0x0001
#define QMIDMS_GET_DEVICE_CAP_REQ 0x0020
#define QMIDMS_GET_DEVICE_CAP_RESP 0x0020
#define QMIDMS_GET_DEVICE_MFR_REQ 0x0021
#define QMIDMS_GET_DEVICE_MFR_RESP 0x0021
#define QMIDMS_GET_DEVICE_MODEL_ID_REQ 0x0022
#define QMIDMS_GET_DEVICE_MODEL_ID_RESP 0x0022
#define QMIDMS_GET_DEVICE_REV_ID_REQ 0x0023
#define QMIDMS_GET_DEVICE_REV_ID_RESP 0x0023
#define QMIDMS_GET_MSISDN_REQ 0x0024
#define QMIDMS_GET_MSISDN_RESP 0x0024
#define QMIDMS_GET_DEVICE_SERIAL_NUMBERS_REQ 0x0025
#define QMIDMS_GET_DEVICE_SERIAL_NUMBERS_RESP 0x0025
#define QMIDMS_UIM_SET_PIN_PROTECTION_REQ 0x0027
#define QMIDMS_UIM_SET_PIN_PROTECTION_RESP 0x0027
#define QMIDMS_UIM_VERIFY_PIN_REQ 0x0028
#define QMIDMS_UIM_VERIFY_PIN_RESP 0x0028
#define QMIDMS_UIM_UNBLOCK_PIN_REQ 0x0029
#define QMIDMS_UIM_UNBLOCK_PIN_RESP 0x0029
#define QMIDMS_UIM_CHANGE_PIN_REQ 0x002A
#define QMIDMS_UIM_CHANGE_PIN_RESP 0x002A
#define QMIDMS_UIM_GET_PIN_STATUS_REQ 0x002B
#define QMIDMS_UIM_GET_PIN_STATUS_RESP 0x002B
#define QMIDMS_GET_DEVICE_HARDWARE_REV_REQ 0x002C
#define QMIDMS_GET_DEVICE_HARDWARE_REV_RESP 0x002C
#define QMIDMS_GET_OPERATING_MODE_REQ 0x002D
#define QMIDMS_GET_OPERATING_MODE_RESP 0x002D
#define QMIDMS_SET_OPERATING_MODE_REQ 0x002E
#define QMIDMS_SET_OPERATING_MODE_RESP 0x002E
#define QMIDMS_GET_ACTIVATED_STATUS_REQ 0x0031
#define QMIDMS_GET_ACTIVATED_STATUS_RESP 0x0031
#define QMIDMS_ACTIVATE_AUTOMATIC_REQ 0x0032
#define QMIDMS_ACTIVATE_AUTOMATIC_RESP 0x0032
#define QMIDMS_ACTIVATE_MANUAL_REQ 0x0033
#define QMIDMS_ACTIVATE_MANUAL_RESP 0x0033
#define QMIDMS_UIM_GET_ICCID_REQ 0x003C
#define QMIDMS_UIM_GET_ICCID_RESP 0x003C
#define QMIDMS_UIM_GET_CK_STATUS_REQ 0x0040
#define QMIDMS_UIM_GET_CK_STATUS_RESP 0x0040
#define QMIDMS_UIM_SET_CK_PROTECTION_REQ 0x0041
#define QMIDMS_UIM_SET_CK_PROTECTION_RESP 0x0041
#define QMIDMS_UIM_UNBLOCK_CK_REQ 0x0042
#define QMIDMS_UIM_UNBLOCK_CK_RESP 0x0042
#define QMIDMS_UIM_GET_IMSI_REQ 0x0043
#define QMIDMS_UIM_GET_IMSI_RESP 0x0043
#define QMIDMS_UIM_GET_STATE_REQ 0x0044
#define QMIDMS_UIM_GET_STATE_RESP 0x0044
#define QMIDMS_GET_BAND_CAP_REQ 0x0045
#define QMIDMS_GET_BAND_CAP_RESP 0x0045

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

typedef struct _QMIDMS_GET_DEVICE_REV_ID_REQ_MSG {
    USHORT Type;  // QMUX type 0x0005
    USHORT Length;
} __attribute__((packed)) QMIDMS_GET_DEVICE_REV_ID_REQ_MSG,
    *PQMIDMS_GET_DEVICE_REV_ID_REQ_MSG;

typedef struct _DEVICE_REV_ID {
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR RevisionID;
} __attribute__((packed)) DEVICE_REV_ID, *PDEVICE_REV_ID;

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

typedef struct _QMIDMS_UIM_GET_IMSI_REQ_MSG {
    USHORT Type;
    USHORT Length;
} __attribute__((packed)) QMIDMS_UIM_GET_IMSI_REQ_MSG,
    *PQMIDMS_UIM_GET_IMSI_REQ_MSG;

typedef struct _QMIDMS_UIM_GET_IMSI_RESP_MSG {
    USHORT Type;
    USHORT Length;
    UCHAR TLVType;
    USHORT TLVLength;
    USHORT QMUXResult;
    USHORT QMUXError;
    UCHAR TLV2Type;
    USHORT TLV2Length;
    UCHAR IMSI;
} __attribute__((packed)) QMIDMS_UIM_GET_IMSI_RESP_MSG,
    *PQMIDMS_UIM_GET_IMSI_RESP_MSG;

#if 0
typedef struct _QMIDMS_GET_DEVICE_SERIAL_NUMBERS_REQ_MSG
{
   USHORT Type;             // QMUX type 0x0007
   USHORT Length;
} QMIDMS_GET_DEVICE_SERIAL_NUMBERS_REQ_MSG, *PQMIDMS_GET_DEVICE_SERIAL_NUMBERS_REQ_MSG;

#define QCTLV_TYPE_SER_NUM_ESN 0x10
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

typedef struct _QMIDMS_SET_OPERATING_MODE_REQ_MSG {
    USHORT Type;  // QMUX type 0x0002
    USHORT Length;
    UCHAR TLVType;     // 0x02 - result code
    USHORT TLVLength;  // 4
    UCHAR OperatingMode;
} __attribute__((packed)) QMIDMS_SET_OPERATING_MODE_REQ_MSG,
    *PQMIDMS_SET_OPERATING_MODE_REQ_MSG;

typedef struct _QMIDMS_SET_OPERATING_MODE_RESP_MSG {
    USHORT Type;  // QMUX type 0x0002
    USHORT Length;
    UCHAR TLVType;	// 0x02 - result code
    USHORT TLVLength;	// 4
    USHORT QMUXResult;	// QMUX_RESULT_SUCCESS
			// QMUX_RESULT_FAILURE
    USHORT QMUXError;	// QMUX_ERR_INVALID_ARG
			// QMUX_ERR_NO_MEMORY
			// QMUX_ERR_INTERNAL
			// QMUX_ERR_FAULT
} __attribute__((packed)) QMIDMS_SET_OPERATING_MODE_RESP_MSG,
    *PQMIDMS_SET_OPERATING_MODE_RESP_MSG;

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

typedef struct _QMIDMS_UIM_GET_STATE_REQ_MSG {
    USHORT Type;
    USHORT Length;
} __attribute__((packed)) QMIDMS_UIM_GET_STATE_REQ_MSG,
    *PQMIDMS_UIM_GET_STATE_REQ_MSG;

typedef struct _QMIDMS_UIM_GET_STATE_RESP_MSG {
    USHORT Type;
    USHORT Length;
    UCHAR TLVType;
    USHORT TLVLength;
    USHORT QMUXResult;
    USHORT QMUXError;
    UCHAR TLV2Type;
    USHORT TLV2Length;
    UCHAR UIMState;
} __attribute__((packed)) QMIDMS_UIM_GET_STATE_RESP_MSG,
    *PQMIDMS_UIM_GET_STATE_RESP_MSG;

typedef struct _QMIDMS_UIM_GET_PIN_STATUS_REQ_MSG {
    USHORT Type;  // QMUX type 0x0024
    USHORT Length;
} __attribute__((packed)) QMIDMS_UIM_GET_PIN_STATUS_REQ_MSG,
    *PQMIDMS_UIM_GET_PIN_STATUS_REQ_MSG;

typedef struct _QMIDMS_UIM_PIN_STATUS {
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR PINStatus;
    UCHAR PINVerifyRetriesLeft;
    UCHAR PINUnblockRetriesLeft;
} __attribute__((packed)) QMIDMS_UIM_PIN_STATUS, *PQMIDMS_UIM_PIN_STATUS;

#define QMI_PIN_STATUS_NOT_INIT 0
#define QMI_PIN_STATUS_NOT_VERIF 1
#define QMI_PIN_STATUS_VERIFIED 2
#define QMI_PIN_STATUS_DISABLED 3
#define QMI_PIN_STATUS_BLOCKED 4
#define QMI_PIN_STATUS_PERM_BLOCKED 5
#define QMI_PIN_STATUS_UNBLOCKED 6
#define QMI_PIN_STATUS_CHANGED 7

typedef struct _QMIDMS_UIM_GET_PIN_STATUS_RESP_MSG {
    USHORT Type;  // QMUX type 0x0024
    USHORT Length;
    UCHAR TLVType;	// 0x02 - result code
    USHORT TLVLength;	// 4
    USHORT QMUXResult;	// QMI_RESULT_SUCCESS
			// QMI_RESULT_FAILURE
    USHORT QMUXError;	// QMI_ERR_INVALID_ARG
			// QMI_ERR_NO_MEMORY
			// QMI_ERR_INTERNAL
			// QMI_ERR_FAULT
    UCHAR PinStatus;
} __attribute__((packed)) QMIDMS_UIM_GET_PIN_STATUS_RESP_MSG,
    *PQMIDMS_UIM_GET_PIN_STATUS_RESP_MSG;

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

typedef struct _QMIDMS_UIM_VERIFY_PIN_REQ_MSG {
    USHORT Type;  // QMUX type 0x0024
    USHORT Length;
    UCHAR TLVType;     // 0x02 - result code
    USHORT TLVLength;  // 4
    UCHAR PINID;
    UCHAR PINLen;
    UCHAR PINValue;
} __attribute__((packed)) QMIDMS_UIM_VERIFY_PIN_REQ_MSG,
    *PQMIDMS_UIM_VERIFY_PIN_REQ_MSG;

typedef struct _QMIDMS_UIM_VERIFY_PIN_RESP_MSG {
    USHORT Type;  // QMUX type 0x0024
    USHORT Length;
    UCHAR TLVType;	// 0x02 - result code
    USHORT TLVLength;	// 4
    USHORT QMUXResult;	// QMI_RESULT_SUCCESS
			// QMI_RESULT_FAILURE
    USHORT QMUXError;	// QMI_ERR_INVALID_ARG
			// QMI_ERR_NO_MEMORY
			// QMI_ERR_INTERNAL
			// QMI_ERR_FAULT
    UCHAR TLV2Type;
    USHORT TLV2Length;
    UCHAR PINVerifyRetriesLeft;
    UCHAR PINUnblockRetriesLeft;
} __attribute__((packed)) QMIDMS_UIM_VERIFY_PIN_RESP_MSG,
    *PQMIDMS_UIM_VERIFY_PIN_RESP_MSG;

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

#define QMI_QOS_SET_EVENT_REPORT_REQ 0x0001
#define QMI_QOS_SET_EVENT_REPORT_RESP 0x0001
#define QMI_QOS_EVENT_REPORT_IND 0x0001

#if 0
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

typedef struct _QMI_QOS_EVENT_REPORT_IND_MSG
{
   USHORT Type;             // QMUX type 0x0001
   USHORT Length;
   UCHAR  TLVs;
} QMI_QOS_EVENT_REPORT_IND_MSG, *PQMI_QOS_EVENT_REPORT_IND_MSG;

#define QOS_EVENT_RPT_IND_FLOW_ACTIVATED 0x01
#define QOS_EVENT_RPT_IND_FLOW_MODIFIED 0x02
#define QOS_EVENT_RPT_IND_FLOW_DELETED 0x03
#define QOS_EVENT_RPT_IND_FLOW_SUSPENDED 0x04
#define QOS_EVENT_RPT_IND_FLOW_ENABLED 0x05
#define QOS_EVENT_RPT_IND_FLOW_DISABLED 0x06

#define QOS_EVENT_RPT_IND_TLV_PHY_LINK_STATE_TYPE 0x01
#define QOS_EVENT_RPT_IND_TLV_GLOBAL_FL_RPT_STATE 0x10
#define QOS_EVENT_RPT_IND_TLV_GLOBAL_FL_RPT_TYPE 0x10
#define QOS_EVENT_RPT_IND_TLV_TX_FLOW_TYPE 0x11
#define QOS_EVENT_RPT_IND_TLV_RX_FLOW_TYPE 0x12
#define QOS_EVENT_RPT_IND_TLV_TX_FILTER_TYPE 0x13
#define QOS_EVENT_RPT_IND_TLV_RX_FILTER_TYPE 0x14
#define QOS_EVENT_RPT_IND_TLV_FLOW_SPEC 0x10
#define QOS_EVENT_RPT_IND_TLV_FILTER_SPEC 0x10

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

#define QOS_FLOW_TLV_IP_FLOW_IDX_TYPE 0x10
#define QOS_FLOW_TLV_IP_FLOW_TRAFFIC_CLASS_TYPE 0x11
#define QOS_FLOW_TLV_IP_FLOW_DATA_RATE_MIN_MAX_TYPE 0x12
#define QOS_FLOW_TLV_IP_FLOW_DATA_RATE_TOKEN_BUCKET_TYPE 0x13
#define QOS_FLOW_TLV_IP_FLOW_LATENCY_TYPE 0x14
#define QOS_FLOW_TLV_IP_FLOW_JITTER_TYPE 0x15
#define QOS_FLOW_TLV_IP_FLOW_PKT_ERR_RATE_TYPE 0x16
#define QOS_FLOW_TLV_IP_FLOW_MIN_PKT_SIZE_TYPE 0x17
#define QOS_FLOW_TLV_IP_FLOW_MAX_PKT_SIZE_TYPE 0x18
#define QOS_FLOW_TLV_IP_FLOW_3GPP_BIT_ERR_RATE_TYPE 0x19
#define QOS_FLOW_TLV_IP_FLOW_3GPP_TRAF_PRIORITY_TYPE 0x1A
#define QOS_FLOW_TLV_IP_FLOW_3GPP2_PROFILE_ID_TYPE 0x1B

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

#define QOS_FILTER_TLV_IP_FILTER_IDX_TYPE 0x10
#define QOS_FILTER_TLV_IP_VERSION_TYPE 0x11
#define QOS_FILTER_TLV_IPV4_SRC_ADDR_TYPE 0x12
#define QOS_FILTER_TLV_IPV4_DEST_ADDR_TYPE 0x13
#define QOS_FILTER_TLV_NEXT_HDR_PROTOCOL_TYPE 0x14
#define QOS_FILTER_TLV_IPV4_TYPE_OF_SERVICE_TYPE 0x15
#define QOS_FILTER_TLV_TCP_UDP_PORT_SRC_TCP_TYPE 0x1B
#define QOS_FILTER_TLV_TCP_UDP_PORT_DEST_TCP_TYPE 0x1C
#define QOS_FILTER_TLV_TCP_UDP_PORT_SRC_UDP_TYPE 0x1D
#define QOS_FILTER_TLV_TCP_UDP_PORT_DEST_UDP_TYPE 0x1E
#define QOS_FILTER_TLV_ICMP_FILTER_MSG_TYPE_TYPE 0x1F
#define QOS_FILTER_TLV_ICMP_FILTER_MSG_CODE_TYPE 0x20
#define QOS_FILTER_TLV_TCP_UDP_PORT_SRC_TYPE 0x24
#define QOS_FILTER_TLV_TCP_UDP_PORT_DEST_TYPE 0x25

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

#define QOS_FILTER_PRECEDENCE_INVALID 256
#define QOS_FILTER_TLV_PRECEDENCE_TYPE 0x22
#define QOS_FILTER_TLV_ID_TYPE 0x23

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

#define QOS_FILTER_TLV_IPV6_SRC_ADDR_TYPE 0x16
#define QOS_FILTER_TLV_IPV6_DEST_ADDR_TYPE 0x17
#define QOS_FILTER_TLV_IPV6_NEXT_HDR_PROTOCOL_TYPE 0x14	 // same as IPV4
#define QOS_FILTER_TLV_IPV6_TRAFFIC_CLASS_TYPE 0x19
#define QOS_FILTER_TLV_IPV6_FLOW_LABEL_TYPE 0x1A

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

#endif	// QCQOS_IPV6
#endif

// ======================= WMS ==============================
#define QMIWMS_SET_EVENT_REPORT_REQ 0x0001
#define QMIWMS_SET_EVENT_REPORT_RESP 0x0001
#define QMIWMS_EVENT_REPORT_IND 0x0001
#define QMIWMS_RAW_SEND_REQ 0x0020
#define QMIWMS_RAW_SEND_RESP 0x0020
#define QMIWMS_RAW_WRITE_REQ 0x0021
#define QMIWMS_RAW_WRITE_RESP 0x0021
#define QMIWMS_RAW_READ_REQ 0x0022
#define QMIWMS_RAW_READ_RESP 0x0022
#define QMIWMS_MODIFY_TAG_REQ 0x0023
#define QMIWMS_MODIFY_TAG_RESP 0x0023
#define QMIWMS_DELETE_REQ 0x0024
#define QMIWMS_DELETE_RESP 0x0024
#define QMIWMS_GET_MESSAGE_PROTOCOL_REQ 0x0030
#define QMIWMS_GET_MESSAGE_PROTOCOL_RESP 0x0030
#define QMIWMS_LIST_MESSAGES_REQ 0x0031
#define QMIWMS_LIST_MESSAGES_RESP 0x0031
#define QMIWMS_GET_SMSC_ADDRESS_REQ 0x0034
#define QMIWMS_GET_SMSC_ADDRESS_RESP 0x0034
#define QMIWMS_SET_SMSC_ADDRESS_REQ 0x0035
#define QMIWMS_SET_SMSC_ADDRESS_RESP 0x0035
#define QMIWMS_GET_STORE_MAX_SIZE_REQ 0x0036
#define QMIWMS_GET_STORE_MAX_SIZE_RESP 0x0036

#define WMS_MESSAGE_PROTOCOL_CDMA 0x00
#define WMS_MESSAGE_PROTOCOL_WCDMA 0x01

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
#define QMINAS_SET_EVENT_REPORT_REQ 0x0002
#define QMINAS_SET_EVENT_REPORT_RESP 0x0002
#define QMINAS_EVENT_REPORT_IND 0x0002
#define QMINAS_GET_SIGNAL_STRENGTH_REQ 0x0020
#define QMINAS_GET_SIGNAL_STRENGTH_RESP 0x0020
#define QMINAS_PERFORM_NETWORK_SCAN_REQ 0x0021
#define QMINAS_PERFORM_NETWORK_SCAN_RESP 0x0021
#define QMINAS_INITIATE_NW_REGISTER_REQ 0x0022
#define QMINAS_INITIATE_NW_REGISTER_RESP 0x0022
#define QMINAS_INITIATE_ATTACH_REQ 0x0023
#define QMINAS_INITIATE_ATTACH_RESP 0x0023
#define QMINAS_GET_SERVING_SYSTEM_REQ 0x0024
#define QMINAS_GET_SERVING_SYSTEM_RESP 0x0024
#define QMINAS_SERVING_SYSTEM_IND 0x0024
#define QMINAS_GET_HOME_NETWORK_REQ 0x0025
#define QMINAS_GET_HOME_NETWORK_RESP 0x0025
#define QMINAS_GET_PREFERRED_NETWORK_REQ 0x0026
#define QMINAS_GET_PREFERRED_NETWORK_RESP 0x0026
#define QMINAS_SET_PREFERRED_NETWORK_REQ 0x0027
#define QMINAS_SET_PREFERRED_NETWORK_RESP 0x0027
#define QMINAS_GET_FORBIDDEN_NETWORK_REQ 0x0028
#define QMINAS_GET_FORBIDDEN_NETWORK_RESP 0x0028
#define QMINAS_SET_FORBIDDEN_NETWORK_REQ 0x0029
#define QMINAS_SET_FORBIDDEN_NETWORK_RESP 0x0029
#define QMINAS_SET_TECHNOLOGY_PREF_REQ 0x002A
#define QMINAS_SET_TECHNOLOGY_PREF_RESP 0x002A
#define QMINAS_GET_RF_BAND_INFO_REQ 0x0031
#define QMINAS_GET_RF_BAND_INFO_RESP 0x0031
#define QMINAS_GET_PLMN_NAME_REQ 0x0044
#define QMINAS_GET_PLMN_NAME_RESP 0x0044
#define FIBO_PACKET_TRANSFER_START_IND 0X100
#define FIBO_PACKET_TRANSFER_END_IND 0X101
#define QMINAS_GET_SYS_INFO_REQ 0x004D
#define QMINAS_GET_SYS_INFO_RESP 0x004D
#define QMINAS_SYS_INFO_IND 0x004D

typedef struct _QMINAS_GET_HOME_NETWORK_REQ_MSG {
    USHORT Type;  // QMUX type 0x0003
    USHORT Length;
} __attribute__((packed)) QMINAS_GET_HOME_NETWORK_REQ_MSG,
    *PQMINAS_GET_HOME_NETWORK_REQ_MSG;

typedef struct _HOME_NETWORK_SYSTEMID {
    UCHAR TLVType;
    USHORT TLVLength;
    USHORT SystemID;
    USHORT NetworkID;
} __attribute__((packed)) HOME_NETWORK_SYSTEMID, *PHOME_NETWORK_SYSTEMID;

typedef struct _HOME_NETWORK {
    UCHAR TLVType;
    USHORT TLVLength;
    USHORT MobileCountryCode;
    USHORT MobileNetworkCode;
    UCHAR NetworkDesclen;
    UCHAR NetworkDesc;
} __attribute__((packed)) HOME_NETWORK, *PHOME_NETWORK;

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

typedef struct _QMINAS_DATA_CAP {
    UCHAR TLVType;     // 0x01 - required parameter
    USHORT TLVLength;  // length of the mfr string
    UCHAR DataCapListLen;
    UCHAR DataCap;
} __attribute__((packed)) QMINAS_DATA_CAP, *PQMINAS_DATA_CAP;

typedef struct _QMINAS_CURRENT_PLMN_MSG {
    UCHAR TLVType;     // 0x01 - required parameter
    USHORT TLVLength;  // length of the mfr string
    USHORT MobileCountryCode;
    USHORT MobileNetworkCode;
    UCHAR NetworkDesclen;
    UCHAR NetworkDesc;
} __attribute__((packed)) QMINAS_CURRENT_PLMN_MSG, *PQMINAS_CURRENT_PLMN_MSG;

typedef struct _QMINAS_GET_SERVING_SYSTEM_RESP_MSG {
    USHORT Type;
    USHORT Length;
    UCHAR TLVType;
    USHORT TLVLength;
    USHORT QMUXResult;
    USHORT QMUXError;
} __attribute__((packed)) QMINAS_GET_SERVING_SYSTEM_RESP_MSG,
    *PQMINAS_GET_SERVING_SYSTEM_RESP_MSG;

typedef struct _SERVING_SYSTEM {
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR RegistrationState;
    UCHAR CSAttachedState;
    UCHAR PSAttachedState;
    UCHAR RegistredNetwork;
    UCHAR InUseRadioIF;
    UCHAR RadioIF;
} __attribute__((packed)) SERVING_SYSTEM, *PSERVING_SYSTEM;

typedef struct _QMINAS_GET_SYS_INFO_RESP_MSG {
    USHORT Type;
    USHORT Length;
    UCHAR TLVType;
    USHORT TLVLength;
    USHORT QMUXResult;
    USHORT QMUXError;
} __attribute__((packed)) QMINAS_GET_SYS_INFO_RESP_MSG,
    *PQMINAS_GET_SYS_INFO_RESP_MSG;

typedef struct _QMINAS_SYS_INFO_IND_MSG {
    USHORT Type;
    USHORT Length;
} __attribute__((packed)) QMINAS_SYS_INFO_IND_MSG, *PQMINAS_SYS_INFO_IND_MSG;

typedef struct _SERVICE_STATUS_INFO {
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR SrvStatus;
    UCHAR true_srv_status;
    UCHAR IsPrefDataPath;
} __attribute__((packed)) SERVICE_STATUS_INFO, *PSERVICE_STATUS_INFO;

typedef struct _CDMA_SYSTEM_INFO {
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR SrvDomainValid;
    UCHAR SrvDomain;
    UCHAR SrvCapabilityValid;
    UCHAR SrvCapability;
    UCHAR RoamStatusValid;
    UCHAR RoamStatus;
    UCHAR IsSysForbiddenValid;
    UCHAR IsSysForbidden;
    UCHAR IsSysPrlMatchValid;
    UCHAR IsSysPrlMatch;
    UCHAR PRevInUseValid;
    UCHAR PRevInUse;
    UCHAR BSPRevValid;
    UCHAR BSPRev;
    UCHAR CCSSupportedValid;
    UCHAR CCSSupported;
    UCHAR CDMASysIdValid;
    USHORT SID;
    USHORT NID;
    UCHAR BSInfoValid;
    USHORT BaseID;
    ULONG BaseLAT;
    ULONG BaseLONG;
    UCHAR PacketZoneValid;
    USHORT PacketZone;
    UCHAR NetworkIdValid;
    UCHAR MCC[3];
    UCHAR MNC[3];
} __attribute__((packed)) CDMA_SYSTEM_INFO, *PCDMA_SYSTEM_INFO;

typedef struct _HDR_SYSTEM_INFO {
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR SrvDomainValid;
    UCHAR SrvDomain;
    UCHAR SrvCapabilityValid;
    UCHAR SrvCapability;
    UCHAR RoamStatusValid;
    UCHAR RoamStatus;
    UCHAR IsSysForbiddenValid;
    UCHAR IsSysForbidden;
    UCHAR IsSysPrlMatchValid;
    UCHAR IsSysPrlMatch;
    UCHAR HdrPersonalityValid;
    UCHAR HdrPersonality;
    UCHAR HdrActiveProtValid;
    UCHAR HdrActiveProt;
    UCHAR is856SysIdValid;
    UCHAR is856SysId[16];
} __attribute__((packed)) HDR_SYSTEM_INFO, *PHDR_SYSTEM_INFO;

typedef struct _GSM_SYSTEM_INFO {
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR SrvDomainValid;
    UCHAR SrvDomain;
    UCHAR SrvCapabilityValid;
    UCHAR SrvCapability;
    UCHAR RoamStatusValid;
    UCHAR RoamStatus;
    UCHAR IsSysForbiddenValid;
    UCHAR IsSysForbidden;
    UCHAR LacValid;
    USHORT Lac;
    UCHAR CellIdValid;
    ULONG CellId;
    UCHAR RegRejectInfoValid;
    UCHAR RejectSrvDomain;
    UCHAR RejCause;
    UCHAR NetworkIdValid;
    UCHAR MCC[3];
    UCHAR MNC[3];
    UCHAR EgprsSuppValid;
    UCHAR EgprsSupp;
    UCHAR DtmSuppValid;
    UCHAR DtmSupp;
} __attribute__((packed)) GSM_SYSTEM_INFO, *PGSM_SYSTEM_INFO;

typedef struct _WCDMA_SYSTEM_INFO {
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR SrvDomainValid;
    UCHAR SrvDomain;
    UCHAR SrvCapabilityValid;
    UCHAR SrvCapability;
    UCHAR RoamStatusValid;
    UCHAR RoamStatus;
    UCHAR IsSysForbiddenValid;
    UCHAR IsSysForbidden;
    UCHAR LacValid;
    USHORT Lac;
    UCHAR CellIdValid;
    ULONG CellId;
    UCHAR RegRejectInfoValid;
    UCHAR RejectSrvDomain;
    UCHAR RejCause;
    UCHAR NetworkIdValid;
    UCHAR MCC[3];
    UCHAR MNC[3];
    UCHAR HsCallStatusValid;
    UCHAR HsCallStatus;
    UCHAR HsIndValid;
    UCHAR HsInd;
    UCHAR PscValid;
    UCHAR Psc;
} __attribute__((packed)) WCDMA_SYSTEM_INFO, *PWCDMA_SYSTEM_INFO;

typedef struct _LTE_SYSTEM_INFO {
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR SrvDomainValid;
    UCHAR SrvDomain;
    UCHAR SrvCapabilityValid;
    UCHAR SrvCapability;
    UCHAR RoamStatusValid;
    UCHAR RoamStatus;
    UCHAR IsSysForbiddenValid;
    UCHAR IsSysForbidden;
    UCHAR LacValid;
    USHORT Lac;
    UCHAR CellIdValid;
    ULONG CellId;
    UCHAR RegRejectInfoValid;
    UCHAR RejectSrvDomain;
    UCHAR RejCause;
    UCHAR NetworkIdValid;
    UCHAR MCC[3];
    UCHAR MNC[3];
    UCHAR TacValid;
    USHORT Tac;
} __attribute__((packed)) LTE_SYSTEM_INFO, *PLTE_SYSTEM_INFO;
//begin modified by zhangkaibo add 5G network detect feature on x55 platform. 20200605
typedef struct _NR5G_SYSTEM_INFO {
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR SrvDomainValid;
    UCHAR SrvDomain;
    UCHAR SrvCapabilityValid;
    UCHAR SrvCapability;
    UCHAR RoamStatusValid;
    UCHAR RoamStatus;
    UCHAR IsSysForbiddenValid;
    UCHAR IsSysForbidden;
    UCHAR LacValid;
    USHORT Lac;
    UCHAR CellIdValid;
    ULONG CellId;
    UCHAR RegRejectInfoValid;
    UCHAR RejectSrvDomain;
    UCHAR RejCause;
    UCHAR NetworkIdValid;
    UCHAR MCC[3];
    UCHAR MNC[3];
    UCHAR TacValid;
    USHORT Tac;
} __attribute__((packed)) NR5G_SYSTEM_INFO, *PNR5G_SYSTEM_INFO;

//end modified by zhangkaibo add 5G network detect feature on x55 platform. 20200605
typedef struct _TDSCDMA_SYSTEM_INFO {
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR SrvDomainValid;
    UCHAR SrvDomain;
    UCHAR SrvCapabilityValid;
    UCHAR SrvCapability;
    UCHAR RoamStatusValid;
    UCHAR RoamStatus;
    UCHAR IsSysForbiddenValid;
    UCHAR IsSysForbidden;
    UCHAR LacValid;
    USHORT Lac;
    UCHAR CellIdValid;
    ULONG CellId;
    UCHAR RegRejectInfoValid;
    UCHAR RejectSrvDomain;
    UCHAR RejCause;
    UCHAR NetworkIdValid;
    UCHAR MCC[3];
    UCHAR MNC[3];
    UCHAR HsCallStatusValid;
    UCHAR HsCallStatus;
    UCHAR HsIndValid;
    UCHAR HsInd;
    UCHAR CellParameterIdValid;
    USHORT CellParameterId;
    UCHAR CellBroadcastCapValid;
    ULONG CellBroadcastCap;
    UCHAR CsBarStatusValid;
    ULONG CsBarStatus;
    UCHAR PsBarStatusValid;
    ULONG PsBarStatus;
    UCHAR CipherDomainValid;
    UCHAR CipherDomain;
} __attribute__((packed)) TDSCDMA_SYSTEM_INFO, *PTDSCDMA_SYSTEM_INFO;

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
// ======================= End of NAS ==============================

// ======================= UIM ==============================
#define QMIUIM_READ_TRANSPARENT_REQ 0x0020
#define QMIUIM_READ_TRANSPARENT_RESP 0x0020
#define QMIUIM_READ_TRANSPARENT_IND 0x0020
#define QMIUIM_READ_RECORD_REQ 0x0021
#define QMIUIM_READ_RECORD_RESP 0x0021
#define QMIUIM_READ_RECORD_IND 0x0021
#define QMIUIM_WRITE_TRANSPARENT_REQ 0x0022
#define QMIUIM_WRITE_TRANSPARENT_RESP 0x0022
#define QMIUIM_WRITE_TRANSPARENT_IND 0x0022
#define QMIUIM_WRITE_RECORD_REQ 0x0023
#define QMIUIM_WRITE_RECORD_RESP 0x0023
#define QMIUIM_WRITE_RECORD_IND 0x0023
#define QMIUIM_SET_PIN_PROTECTION_REQ 0x0025
#define QMIUIM_SET_PIN_PROTECTION_RESP 0x0025
#define QMIUIM_SET_PIN_PROTECTION_IND 0x0025
#define QMIUIM_VERIFY_PIN_REQ 0x0026
#define QMIUIM_VERIFY_PIN_RESP 0x0026
#define QMIUIM_VERIFY_PIN_IND 0x0026
#define QMIUIM_UNBLOCK_PIN_REQ 0x0027
#define QMIUIM_UNBLOCK_PIN_RESP 0x0027
#define QMIUIM_UNBLOCK_PIN_IND 0x0027
#define QMIUIM_CHANGE_PIN_REQ 0x0028
#define QMIUIM_CHANGE_PIN_RESP 0x0028
#define QMIUIM_CHANGE_PIN_IND 0x0028
#define QMIUIM_DEPERSONALIZATION_REQ 0x0029
#define QMIUIM_DEPERSONALIZATION_RESP 0x0029
#define QMIUIM_EVENT_REG_REQ 0x002E
#define QMIUIM_EVENT_REG_RESP 0x002E
#define QMIUIM_GET_CARD_STATUS_REQ 0x002F
#define QMIUIM_GET_CARD_STATUS_RESP 0x002F
#define QMIUIM_STATUS_CHANGE_IND 0x0032

typedef struct _QMIUIM_GET_CARD_STATUS_RESP_MSG {
    USHORT Type;
    USHORT Length;
    UCHAR TLVType;
    USHORT TLVLength;
    USHORT QMUXResult;
    USHORT QMUXError;
} __attribute__((packed)) QMIUIM_GET_CARD_STATUS_RESP_MSG,
    *PQMIUIM_GET_CARD_STATUS_RESP_MSG;

typedef struct _QMIUIM_PIN_STATE {
    UCHAR UnivPIN;
    UCHAR PIN1State;
    UCHAR PIN1Retries;
    UCHAR PUK1Retries;
    UCHAR PIN2State;
    UCHAR PIN2Retries;
    UCHAR PUK2Retries;
} __attribute__((packed)) QMIUIM_PIN_STATE, *PQMIUIM_PIN_STATE;

typedef struct _QMIUIM_APP_STATUS {
    UCHAR AppType;
    UCHAR AppState;
    UCHAR PersoState;
    UCHAR PersoFeature;
    UCHAR PersoRetries;
    UCHAR PersoUnblockRetries;
    UCHAR AIDLength;
    UCHAR AIDValue[16];
    QMIUIM_PIN_STATE PinState;
} __attribute__((packed)) QMIUIM_APP_STATUS, *PQMIUIM_APP_STATUS;

//2021-03-24 willa.liu@fibocom.com changed begin for support mantis 0071817
typedef struct _instance{
    UCHAR CardState;
    UCHAR UPINState;
    UCHAR UPINRetries;
    UCHAR UPUKRetries;
    UCHAR ErrorCode;
    UCHAR NumApp;
}Instance;

typedef struct _QMIUIM_CARD_STATUS {
    UCHAR TLVType;
    USHORT TLVLength;
    USHORT IndexGWPri;
    USHORT Index1XPri;
    USHORT IndexGWSec;
    USHORT Index1XSec;
    UCHAR NumSlot;
/*
	UCHAR CardState;
    UCHAR UPINState;
    UCHAR UPINRetries;
    UCHAR UPUKRetries;
    UCHAR ErrorCode;
    UCHAR NumApp;
    QMIUIM_APP_STATUS AppStatus[8];
*/
}__attribute__((packed)) QMIUIM_CARD_STATUS, *PQMIUIM_CARD_STATUS;
//2021-03-24 willa.liu@fibocom.com changed end for support mantis 0071817

typedef struct _QMIUIM_VERIFY_PIN_REQ_MSG {
    USHORT Type;
    USHORT Length;
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR Session_Type;
    UCHAR Aid_Len;
    UCHAR TLV2Type;
    USHORT TLV2Length;
    UCHAR PINID;
    UCHAR PINLen;
    UCHAR PINValue;
} __attribute__((packed)) QMIUIM_VERIFY_PIN_REQ_MSG,
    *PQMIUIM_VERIFY_PIN_REQ_MSG;

typedef struct _QMIUIM_VERIFY_PIN_RESP_MSG {
    USHORT Type;
    USHORT Length;
    UCHAR TLVType;
    USHORT TLVLength;
    USHORT QMUXResult;
    USHORT QMUXError;
    UCHAR TLV2Type;
    USHORT TLV2Length;
    UCHAR PINVerifyRetriesLeft;
    UCHAR PINUnblockRetriesLeft;
} __attribute__((packed)) QMIUIM_VERIFY_PIN_RESP_MSG,
    *PQMIUIM_VERIFY_PIN_RESP_MSG;

//2021-03-24 willa.liu@fibocom.com changed begin for support mantis 0071817
typedef struct _QMIUIM_BIND_PIN_REQ_MSG {
    USHORT Type;
    USHORT Length;
    UCHAR TLVType;
    USHORT TLVLength;
    uint8_t Value;
} __attribute__((packed)) QMIUIM_BIND_PIN_REQ_MSG,        *PQMIUIM_BIND_PIN_REQ_MSG;

typedef struct _QMIUIM_BIND_PIN_REQ_MSG_4 {
    USHORT Type;
    USHORT Length;
    UCHAR TLVType;
    USHORT TLVLength;
    UINT Value;
} __attribute__((packed)) QMIUIM_BIND_PIN_REQ_MSG_4;
//2021-03-24 willa.liu@fibocom.com changed end for support mantis 0071817

typedef struct _QMIUIM_READ_TRANSPARENT_REQ_MSG {
    USHORT Type;
    USHORT Length;
    UCHAR TLVType;
    USHORT TLVLength;
    UCHAR Session_Type;
    UCHAR Aid_Len;
    UCHAR TLV2Type;
    USHORT TLV2Length;
    USHORT file_id;
    UCHAR path_len;
    UCHAR path[];
} __attribute__((packed)) QMIUIM_READ_TRANSPARENT_REQ_MSG,
    *PQMIUIM_READ_TRANSPARENT_REQ_MSG;

typedef struct _READ_TRANSPARENT_TLV {
    UCHAR TLVType;
    USHORT TLVLength;
    USHORT Offset;
    USHORT Length;
} __attribute__((packed)) READ_TRANSPARENT_TLV, *PREAD_TRANSPARENT_TLV;

typedef struct _QMIUIM_CONTENT {
    UCHAR TLVType;
    USHORT TLVLength;
    USHORT content_len;
    UCHAR content[];
} __attribute__((packed)) QMIUIM_CONTENT, *PQMIUIM_CONTENT;

typedef struct _QMIUIM_READ_TRANSPARENT_RESP_MSG {
    USHORT Type;
    USHORT Length;
    UCHAR TLVType;
    USHORT TLVLength;
    USHORT QMUXResult;
    USHORT QMUXError;
} __attribute__((packed)) QMIUIM_READ_TRANSPARENT_RESP_MSG,
    *PQMIUIM_READ_TRANSPARENT_RESP_MSG;

typedef struct _QMUX_MSG {
    QCQMUX_HDR QMUXHdr;
    union {
	// Message Header
	QCQMUX_MSG_HDR QMUXMsgHdr;
	QCQMUX_MSG_HDR_RESP QMUXMsgHdrResp;

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
	QMIWDS_GET_RUNTIME_SETTINGS_REQ_MSG GetRuntimeSettingsReq;
	QMIWDS_GET_RUNTIME_SETTINGS_RESP_MSG GetRuntimeSettingsRsp;
	//#endif // QC_IP_MODE
	QMIWDS_SET_CLIENT_IP_FAMILY_PREF_REQ_MSG SetClientIpFamilyPrefReq;
	QMIWDS_SET_CLIENT_IP_FAMILY_PREF_RESP_MSG SetClientIpFamilyPrefResp;
	QMIWDS_SET_AUTO_CONNECT_REQ_MSG SetAutoConnectReq;
#if 0
      QMIWDS_GET_MIP_MODE_REQ_MSG               GetMipModeReq;
      QMIWDS_GET_MIP_MODE_RESP_MSG              GetMipModeResp;
#endif
	QMIWDS_START_NETWORK_INTERFACE_REQ_MSG StartNwInterfaceReq;
	QMIWDS_START_NETWORK_INTERFACE_RESP_MSG StartNwInterfaceResp;
	QMIWDS_STOP_NETWORK_INTERFACE_REQ_MSG StopNwInterfaceReq;
	QMIWDS_STOP_NETWORK_INTERFACE_RESP_MSG StopNwInterfaceResp;
	QMIWDS_GET_DEFAULT_SETTINGS_REQ_MSG GetDefaultSettingsReq;
	QMIWDS_GET_DEFAULT_SETTINGS_RESP_MSG GetDefaultSettingsResp;
//begin modified by zhangkaibo add create profile qmi. mantis 0049137,0048741  20200610
	QMIWDS_CREATE_PROFILE_SETTINGS_REQ_MSG CreateProfileSettingsReq;
	QMIWDS_CREATE_PROFILE_SETTINGS_RESP_MSG CreateProfileSettingsResp;
//end modified by zhangkaibo add create profile qmi. mantis 0049137,0048741  20200610
	QMIWDS_MODIFY_PROFILE_SETTINGS_REQ_MSG ModifyProfileSettingsReq;
	QMIWDS_MODIFY_PROFILE_SETTINGS_RESP_MSG ModifyProfileSettingsResp;
	QMIWDS_GET_PROFILE_SETTINGS_REQ_MSG GetProfileSettingsReq;
#if 0
      QMIWDS_GET_DATA_BEARER_REQ_MSG            GetDataBearerReq;
      QMIWDS_GET_DATA_BEARER_RESP_MSG           GetDataBearerResp;
      QMIWDS_DUN_CALL_INFO_REQ_MSG              DunCallInfoReq;
      QMIWDS_DUN_CALL_INFO_RESP_MSG             DunCallInfoResp;
#endif
	QMIWDS_BIND_MUX_DATA_PORT_REQ_MSG BindMuxDataPortReq;

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
	QMIDMS_SET_OPERATING_MODE_REQ_MSG SetOperatingModeReq;
	QMIDMS_SET_OPERATING_MODE_RESP_MSG SetOperatingModeResp;
#if 0
      QMIDMS_UIM_GET_ICCID_REQ_MSG              GetICCIDReq;
      QMIDMS_UIM_GET_ICCID_RESP_MSG             GetICCIDResp;
      QMIDMS_ACTIVATE_AUTOMATIC_REQ_MSG         ActivateAutomaticReq;
      QMIDMS_ACTIVATE_AUTOMATIC_RESP_MSG        ActivateAutomaticResp;
      QMIDMS_ACTIVATE_MANUAL_REQ_MSG            ActivateManualReq;
      QMIDMS_ACTIVATE_MANUAL_RESP_MSG           ActivateManualResp;
#endif
	QMIDMS_UIM_GET_PIN_STATUS_REQ_MSG UIMGetPinStatusReq;
	QMIDMS_UIM_GET_PIN_STATUS_RESP_MSG UIMGetPinStatusResp;
	QMIDMS_UIM_VERIFY_PIN_REQ_MSG UIMVerifyPinReq;
	QMIDMS_UIM_VERIFY_PIN_RESP_MSG UIMVerifyPinResp;
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
	QMIDMS_UIM_GET_STATE_REQ_MSG UIMGetStateReq;
	QMIDMS_UIM_GET_STATE_RESP_MSG UIMGetStateResp;
	QMIDMS_UIM_GET_IMSI_REQ_MSG UIMGetIMSIReq;
	QMIDMS_UIM_GET_IMSI_RESP_MSG UIMGetIMSIResp;
#if 0
      QMIDMS_UIM_GET_CK_STATUS_REQ_MSG          UIMGetCkStatusReq;
      QMIDMS_UIM_GET_CK_STATUS_RESP_MSG         UIMGetCkStatusResp;
      QMIDMS_UIM_SET_CK_PROTECTION_REQ_MSG      UIMSetCkProtectionReq;
      QMIDMS_UIM_SET_CK_PROTECTION_RESP_MSG     UIMSetCkProtectionResp;
      QMIDMS_UIM_UNBLOCK_CK_REQ_MSG             UIMUnblockCkReq;
      QMIDMS_UIM_UNBLOCK_CK_RESP_MSG            UIMUnblockCkResp;
#endif

	// QMIQOS Messages
#if 0
      QMI_QOS_SET_EVENT_REPORT_REQ_MSG          QosSetEventReportReq;
      QMI_QOS_SET_EVENT_REPORT_RESP_MSG         QosSetEventReportRsp;
      QMI_QOS_EVENT_REPORT_IND_MSG              QosEventReportInd;
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
	QMINAS_GET_SERVING_SYSTEM_RESP_MSG GetServingSystemResp;
	QMINAS_GET_SYS_INFO_RESP_MSG GetSysInfoResp;
	QMINAS_SYS_INFO_IND_MSG NasSysInfoInd;
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
	QMIUIM_GET_CARD_STATUS_RESP_MSG UIMGetCardStatus;
	QMIUIM_VERIFY_PIN_REQ_MSG UIMUIMVerifyPinReq;
	QMIUIM_VERIFY_PIN_RESP_MSG UIMUIMVerifyPinResp;
//2021-03-24 willa.liu@fibocom.com changed begin for support mantis 0071817
    QMIUIM_BIND_PIN_REQ_MSG UIMUIMBindPinReq;
    QMIUIM_BIND_PIN_REQ_MSG_4 UIMUIMBindPinReq_4;
//2021-03-24 willa.liu@fibocom.com changed end for support mantis 0071817
#if 0
      QMIUIM_SET_PIN_PROTECTION_REQ_MSG         UIMUIMSetPinProtectionReq;
      QMIUIM_SET_PIN_PROTECTION_RESP_MSG        UIMUIMSetPinProtectionResp;
      QMIUIM_CHANGE_PIN_REQ_MSG                 UIMUIMChangePinReq;
      QMIUIM_CHANGE_PIN_RESP_MSG                UIMUIMChangePinResp;
      QMIUIM_UNBLOCK_PIN_REQ_MSG                UIMUIMUnblockPinReq;
      QMIUIM_UNBLOCK_PIN_RESP_MSG               UIMUIMUnblockPinResp;
#endif
	QMIUIM_READ_TRANSPARENT_REQ_MSG UIMUIMReadTransparentReq;
	QMIUIM_READ_TRANSPARENT_RESP_MSG UIMUIMReadTransparentResp;

	QMIWDS_ADMIN_SET_DATA_FORMAT_REQ_MSG SetDataFormatReq;
    };
} __attribute__((packed)) QMUX_MSG, *PQMUX_MSG;

//2021-03-24 willa.liu@fibocom.com changed begin for support mantis 0071817
#define QMI_WDS_BIND_SUBSCRIPTION_REQ_V01 0x00AF
#define QMI_WDS_BIND_SUBSCRIPTION_RESP_V01 0x00AF
#define QMI_DMS_BIND_SUBSCRIPTION_REQ_V01 0x0054
#define QMI_DMS_BIND_SUBSCRIPTION_RESP_V01 0x0054
#define QMI_NAS_BIND_SUBSCRIPTION_REQ_V01 0x0045
#define QMI_NAS_BIND_SUBSCRIPTION_RESP_V01 0x0045
#define QMI_QOS_BIND_SUBSCRIPTION_REQ_V01 0x002D
#define QMI_QOS_BIND_SUBSCRIPTION_RESP_V01 0x002D
#define QMI_WMS_BIND_SUBSCRIPTION_REQ_V01 0x004C
#define QMI_WMS_BIND_SUBSCRIPTION_RESP_V01 0x004C

typedef enum {
  QMI_RESULT_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_RESULT_SUCCESS_V01 = 0,
  QMI_RESULT_FAILURE_V01 = 1,
  QMI_RESULT_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_result_type_v01;

typedef enum {
  QMI_ERROR_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_ERR_NONE_V01 = 0x0000,
  QMI_ERR_MALFORMED_MSG_V01 = 0x0001,
  QMI_ERR_NO_MEMORY_V01 = 0x0002,
  QMI_ERR_INTERNAL_V01 = 0x0003,
  QMI_ERR_ABORTED_V01 = 0x0004,
  QMI_ERR_CLIENT_IDS_EXHAUSTED_V01 = 0x0005,
  QMI_ERR_UNABORTABLE_TRANSACTION_V01 = 0x0006,
  QMI_ERR_INVALID_CLIENT_ID_V01 = 0x0007,
  QMI_ERR_NO_THRESHOLDS_V01 = 0x0008,
  QMI_ERR_INVALID_HANDLE_V01 = 0x0009,
  QMI_ERR_INVALID_PROFILE_V01 = 0x000A,
  QMI_ERR_INVALID_PINID_V01 = 0x000B,
  QMI_ERR_INCORRECT_PIN_V01 = 0x000C,
  QMI_ERR_NO_NETWORK_FOUND_V01 = 0x000D,
  QMI_ERR_CALL_FAILED_V01 = 0x000E,
  QMI_ERR_OUT_OF_CALL_V01 = 0x000F,
  QMI_ERR_NOT_PROVISIONED_V01 = 0x0010,
  QMI_ERR_MISSING_ARG_V01 = 0x0011,
  QMI_ERR_ARG_TOO_LONG_V01 = 0x0013,
  QMI_ERR_INVALID_TX_ID_V01 = 0x0016,
  QMI_ERR_DEVICE_IN_USE_V01 = 0x0017,
  QMI_ERR_OP_NETWORK_UNSUPPORTED_V01 = 0x0018,
  QMI_ERR_OP_DEVICE_UNSUPPORTED_V01 = 0x0019,
  QMI_ERR_NO_EFFECT_V01 = 0x001A,
  QMI_ERR_NO_FREE_PROFILE_V01 = 0x001B,
  QMI_ERR_INVALID_PDP_TYPE_V01 = 0x001C,
  QMI_ERR_INVALID_TECH_PREF_V01 = 0x001D,
  QMI_ERR_INVALID_PROFILE_TYPE_V01 = 0x001E,
  QMI_ERR_INVALID_SERVICE_TYPE_V01 = 0x001F,
  QMI_ERR_INVALID_REGISTER_ACTION_V01 = 0x0020,
  QMI_ERR_INVALID_PS_ATTACH_ACTION_V01 = 0x0021,
  QMI_ERR_AUTHENTICATION_FAILED_V01 = 0x0022,
  QMI_ERR_PIN_BLOCKED_V01 = 0x0023,
  QMI_ERR_PIN_PERM_BLOCKED_V01 = 0x0024,
  QMI_ERR_SIM_NOT_INITIALIZED_V01 = 0x0025,
  QMI_ERR_MAX_QOS_REQUESTS_IN_USE_V01 = 0x0026,
  QMI_ERR_INCORRECT_FLOW_FILTER_V01 = 0x0027,
  QMI_ERR_NETWORK_QOS_UNAWARE_V01 = 0x0028,
  QMI_ERR_INVALID_ID_V01 = 0x0029,
  QMI_ERR_INVALID_QOS_ID_V01 = 0x0029,
  QMI_ERR_REQUESTED_NUM_UNSUPPORTED_V01 = 0x002A,
  QMI_ERR_INTERFACE_NOT_FOUND_V01 = 0x002B,
  QMI_ERR_FLOW_SUSPENDED_V01 = 0x002C,
  QMI_ERR_INVALID_DATA_FORMAT_V01 = 0x002D,
  QMI_ERR_GENERAL_V01 = 0x002E,
  QMI_ERR_UNKNOWN_V01 = 0x002F,
  QMI_ERR_INVALID_ARG_V01 = 0x0030,
  QMI_ERR_INVALID_INDEX_V01 = 0x0031,
  QMI_ERR_NO_ENTRY_V01 = 0x0032,
  QMI_ERR_DEVICE_STORAGE_FULL_V01 = 0x0033,
  QMI_ERR_DEVICE_NOT_READY_V01 = 0x0034,
  QMI_ERR_NETWORK_NOT_READY_V01 = 0x0035,
  QMI_ERR_CAUSE_CODE_V01 = 0x0036,
  QMI_ERR_MESSAGE_NOT_SENT_V01 = 0x0037,
  QMI_ERR_MESSAGE_DELIVERY_FAILURE_V01 = 0x0038,
  QMI_ERR_INVALID_MESSAGE_ID_V01 = 0x0039,
  QMI_ERR_ENCODING_V01 = 0x003A,
  QMI_ERR_AUTHENTICATION_LOCK_V01 = 0x003B,
  QMI_ERR_INVALID_TRANSITION_V01 = 0x003C,
  QMI_ERR_NOT_A_MCAST_IFACE_V01 = 0x003D,
  QMI_ERR_MAX_MCAST_REQUESTS_IN_USE_V01 = 0x003E,
  QMI_ERR_INVALID_MCAST_HANDLE_V01 = 0x003F,
  QMI_ERR_INVALID_IP_FAMILY_PREF_V01 = 0x0040,
  QMI_ERR_SESSION_INACTIVE_V01 = 0x0041,
  QMI_ERR_SESSION_INVALID_V01 = 0x0042,
  QMI_ERR_SESSION_OWNERSHIP_V01 = 0x0043,
  QMI_ERR_INSUFFICIENT_RESOURCES_V01 = 0x0044,
  QMI_ERR_DISABLED_V01 = 0x0045,
  QMI_ERR_INVALID_OPERATION_V01 = 0x0046,
  QMI_ERR_INVALID_QMI_CMD_V01 = 0x0047,
  QMI_ERR_TPDU_TYPE_V01 = 0x0048,
  QMI_ERR_SMSC_ADDR_V01 = 0x0049,
  QMI_ERR_INFO_UNAVAILABLE_V01 = 0x004A,
  QMI_ERR_SEGMENT_TOO_LONG_V01 = 0x004B,
  QMI_ERR_SEGMENT_ORDER_V01 = 0x004C,
  QMI_ERR_BUNDLING_NOT_SUPPORTED_V01 = 0x004D,
  QMI_ERR_OP_PARTIAL_FAILURE_V01 = 0x004E,
  QMI_ERR_POLICY_MISMATCH_V01 = 0x004F,
  QMI_ERR_SIM_FILE_NOT_FOUND_V01 = 0x0050,
  QMI_ERR_EXTENDED_INTERNAL_V01 = 0x0051,
  QMI_ERR_ACCESS_DENIED_V01 = 0x0052,
  QMI_ERR_HARDWARE_RESTRICTED_V01 = 0x0053,
  QMI_ERR_ACK_NOT_SENT_V01 = 0x0054,
  QMI_ERR_INJECT_TIMEOUT_V01 = 0x0055,
  QMI_ERR_INCOMPATIBLE_STATE_V01 = 0x005A,
  QMI_ERR_FDN_RESTRICT_V01 = 0x005B,
  QMI_ERR_SUPS_FAILURE_CAUSE_V01 = 0x005C,
  QMI_ERR_NO_RADIO_V01 = 0x005D,
  QMI_ERR_NOT_SUPPORTED_V01 = 0x005E,
  QMI_ERR_NO_SUBSCRIPTION_V01 = 0x005F,
  QMI_ERR_CARD_CALL_CONTROL_FAILED_V01 = 0x0060,
  QMI_ERR_NETWORK_ABORTED_V01 = 0x0061,
  QMI_ERR_MSG_BLOCKED_V01 = 0x0062,
  QMI_ERR_INVALID_SESSION_TYPE_V01 = 0x0064,
  QMI_ERR_INVALID_PB_TYPE_V01 = 0x0065,
  QMI_ERR_NO_SIM_V01 = 0x0066,
  QMI_ERR_PB_NOT_READY_V01 = 0x0067,
  QMI_ERR_PIN_RESTRICTION_V01 = 0x0068,
  QMI_ERR_PIN2_RESTRICTION_V01 = 0x0069,
  QMI_ERR_PUK_RESTRICTION_V01 = 0x006A,
  QMI_ERR_PUK2_RESTRICTION_V01 = 0x006B,
  QMI_ERR_PB_ACCESS_RESTRICTED_V01 = 0x006C,
  QMI_ERR_PB_DELETE_IN_PROG_V01 = 0x006D,
  QMI_ERR_PB_TEXT_TOO_LONG_V01 = 0x006E,
  QMI_ERR_PB_NUMBER_TOO_LONG_V01 = 0x006F,
  QMI_ERR_PB_HIDDEN_KEY_RESTRICTION_V01 = 0x0070,
  QMI_ERR_PB_NOT_AVAILABLE_V01 = 0x0071,
  QMI_ERR_DEVICE_MEMORY_ERROR_V01 = 0x0072,
  QMI_ERR_NO_PERMISSION_V01 = 0x0073,
  QMI_ERR_TOO_SOON_V01 = 0x0074,
  QMI_ERR_TIME_NOT_ACQUIRED_V01 = 0x0075,
  QMI_ERR_OP_IN_PROGRESS_V01 = 0x0076,
  QMI_ERR_EPERM_V01 = 0x101, /**<  Operation not permitted  */
  QMI_ERR_ENOENT_V01 = 0x102, /**<  No such file or directory  */
  QMI_ERR_ESRCH_V01 = 0x103, /**<  No such process  */
  QMI_ERR_EINTR_V01 = 0x104, /**<  Interrupted system call  */
  QMI_ERR_EIO_V01 = 0x105, /**<  I/O error  */
  QMI_ERR_ENXIO_V01 = 0x106, /**<  No such device or address  */
  QMI_ERR_E2BIG_V01 = 0x107, /**<  Argument list too long  */
  QMI_ERR_ENOEXEC_V01 = 0x108, /**<  Exec format error  */
  QMI_ERR_EBADF_V01 = 0x109, /**<  Bad file number  */
  QMI_ERR_ECHILD_V01 = 0x10A, /**<  No child processes  */
  QMI_ERR_EAGAIN_V01 = 0x10B, /**<  Try again  */
  QMI_ERR_ENOMEM_V01 = 0x10C, /**<  Out of memory  */
  QMI_ERR_EACCES_V01 = 0x10D, /**<  Permission denied  */
  QMI_ERR_EFAULT_V01 = 0x10E, /**<  Bad address  */
  QMI_ERR_ENOTBLK_V01 = 0x10F, /**<  Block device required  */
  QMI_ERR_EBUSY_V01 = 0x110, /**<  Device or resource busy  */
  QMI_ERR_EEXIST_V01 = 0x111, /**<  File exists  */
  QMI_ERR_EXDEV_V01 = 0x112, /**<  Cross-device link  */
  QMI_ERR_ENODEV_V01 = 0x113, /**<  No such device  */
  QMI_ERR_ENOTDIR_V01 = 0x114, /**<  Not a directory  */
  QMI_ERR_EISDIR_V01 = 0x115, /**<  Is a directory  */
  QMI_ERR_EINVAL_V01 = 0x116, /**<  Invalid argument  */
  QMI_ERR_ENFILE_V01 = 0x117, /**<  File table overflow  */
  QMI_ERR_EMFILE_V01 = 0x118, /**<  Too many open files  */
  QMI_ERR_ENOTTY_V01 = 0x119, /**<  Not a typewriter  */
  QMI_ERR_ETXTBSY_V01 = 0x11A, /**<  Text file busy  */
  QMI_ERR_EFBIG_V01 = 0x11B, /**<  File too large  */
  QMI_ERR_ENOSPC_V01 = 0x11C, /**<  No space left on device  */
  QMI_ERR_ESPIPE_V01 = 0x11D, /**<  Illegal seek  */
  QMI_ERR_EROFS_V01 = 0x11E, /**<  Read-only file system  */
  QMI_ERR_EMLINK_V01 = 0x11F, /**<  Too many links  */
  QMI_ERR_EPIPE_V01 = 0x120, /**<  Broken pipe  */
  QMI_ERR_EDOM_V01 = 0x121, /**<  Math argument out of domain of func  */
  QMI_ERR_ERANGE_V01 = 0x122, /**<  Math result not representable  */
  QMI_ERR_EDEADLK_V01 = 0x123, /**<  Resource deadlock would occur  */
  QMI_ERR_ENAMETOOLONG_V01 = 0x124, /**<  File name too long  */
  QMI_ERR_ENOLCK_V01 = 0x125, /**<  No record locks available  */
  QMI_ERR_ENOSYS_V01 = 0x126, /**<  Function not implemented  */
  QMI_ERR_ENOTEMPTY_V01 = 0x127, /**<  Directory not empty  */
  QMI_ERR_ELOOP_V01 = 0x128, /**<  Too many symbolic links encountered  */
  QMI_ERR_EWOULDBLOCK_V01 = 0x10B, /**<  Operation would block  */
  QMI_ERR_ENOMSG_V01 = 0x12A, /**<  No message of desired type  */
  QMI_ERR_EIDRM_V01 = 0x12B, /**<  Identifier removed  */
  QMI_ERR_ECHRNG_V01 = 0x12C, /**<  Channel number out of range  */
  QMI_ERR_EL2NSYNC_V01 = 0x12D, /**<  Level 2 not synchronized  */
  QMI_ERR_EL3HLT_V01 = 0x12E, /**<  Level 3 halted  */
  QMI_ERR_EL3RST_V01 = 0x12F, /**<  Level 3 reset  */
  QMI_ERR_ELNRNG_V01 = 0x130, /**<  Link number out of range  */
  QMI_ERR_EUNATCH_V01 = 0x131, /**<  Protocol driver not attached  */
  QMI_ERR_ENOCSI_V01 = 0x132, /**<  No CSI structure available  */
  QMI_ERR_EL2HLT_V01 = 0x133, /**<  Level 2 halted  */
  QMI_ERR_EBADE_V01 = 0x134, /**<  Invalid exchange  */
  QMI_ERR_EBADR_V01 = 0x135, /**<  Invalid request descriptor  */
  QMI_ERR_EXFULL_V01 = 0x136, /**<  Exchange full  */
  QMI_ERR_ENOANO_V01 = 0x137, /**<  No anode  */
  QMI_ERR_EBADRQC_V01 = 0x138, /**<  Invalid request code  */
  QMI_ERR_EBADSLT_V01 = 0x139, /**<  Invalid slot  */
  QMI_ERR_EDEADLOCK_V01 = 0x123, /**<  Resource deadlock would occur  */
  QMI_ERR_EBFONT_V01 = 0x13B, /**<  Bad font file format  */
  QMI_ERR_ENOSTR_V01 = 0x13C, /**<  Device not a stream  */
  QMI_ERR_ENODATA_V01 = 0x13D, /**<  No data available  */
  QMI_ERR_ETIME_V01 = 0x13E, /**<  Timer expired  */
  QMI_ERR_ENOSR_V01 = 0x13F, /**<  Out of streams resources  */
  QMI_ERR_ENONET_V01 = 0x140, /**<  Machine is not on the network  */
  QMI_ERR_ENOPKG_V01 = 0x141, /**<  Package not installed  */
  QMI_ERR_EREMOTE_V01 = 0x142, /**<  Object is remote  */
  QMI_ERR_ENOLINK_V01 = 0x143, /**<  Link has been severed  */
  QMI_ERR_EADV_V01 = 0x144, /**<  Advertise error  */
  QMI_ERR_ESRMNT_V01 = 0x145, /**<  Srmount error  */
  QMI_ERR_ECOMM_V01 = 0x146, /**<  Communication error on send  */
  QMI_ERR_EPROTO_V01 = 0x147, /**<  Protocol error  */
  QMI_ERR_EMULTIHOP_V01 = 0x148, /**<  Multihop attempted  */
  QMI_ERR_EDOTDOT_V01 = 0x149, /**<  RFS specific error  */
  QMI_ERR_EBADMSG_V01 = 0x14A, /**<  Not a data message  */
  QMI_ERR_EOVERFLOW_V01 = 0x14B, /**<  Value too large for defined data type  */
  QMI_ERR_ENOTUNIQ_V01 = 0x14C, /**<  Name not unique on network  */
  QMI_ERR_EBADFD_V01 = 0x14D, /**<  File descriptor in bad state  */
  QMI_ERR_EREMCHG_V01 = 0x14E, /**<  Remote address changed  */
  QMI_ERR_ELIBACC_V01 = 0x14F, /**<  Can not access a needed shared library  */
  QMI_ERR_ELIBBAD_V01 = 0x150, /**<  Accessing a corrupted shared library  */
  QMI_ERR_ELIBSCN_V01 = 0x151, /**<  .lib section in a.out corrupted  */
  QMI_ERR_ELIBMAX_V01 = 0x152, /**<  Attempting to link in too many shared libraries  */
  QMI_ERR_ELIBEXEC_V01 = 0x153, /**<  Cannot exec a shared library directly  */
  QMI_ERR_EILSEQ_V01 = 0x154, /**<  Illegal byte sequence  */
  QMI_ERR_ERESTART_V01 = 0x155, /**<  Interrupted system call should be restarted  */
  QMI_ERR_ESTRPIPE_V01 = 0x156, /**<  Streams pipe error  */
  QMI_ERR_EUSERS_V01 = 0x157, /**<  Too many users  */
  QMI_ERR_ENOTSOCK_V01 = 0x158, /**<  Socket operation on non-socket  */
  QMI_ERR_EDESTADDRREQ_V01 = 0x159, /**<  Destination address required  */
  QMI_ERR_EMSGSIZE_V01 = 0x15A, /**<  Message too long  */
  QMI_ERR_EPROTOTYPE_V01 = 0x15B, /**<  Protocol wrong type for socket  */
  QMI_ERR_ENOPROTOOPT_V01 = 0x15C, /**<  Protocol not available  */
  QMI_ERR_EPROTONOSUPPORT_V01 = 0x15D, /**<  Protocol not supported  */
  QMI_ERR_ESOCKTNOSUPPORT_V01 = 0x15E, /**<  Socket type not supported  */
  QMI_ERR_EOPNOTSUPP_V01 = 0x15F, /**<  Operation not supported on transport endpoint  */
  QMI_ERR_EPFNOSUPPORT_V01 = 0x160, /**<  Protocol family not supported  */
  QMI_ERR_EAFNOSUPPORT_V01 = 0x161, /**<  Address family not supported by protocol  */
  QMI_ERR_EADDRINUSE_V01 = 0x162, /**<  Address already in use  */
  QMI_ERR_EADDRNOTAVAIL_V01 = 0x163, /**<  Cannot assign requested address  */
  QMI_ERR_ENETDOWN_V01 = 0x164, /**<  Network is down  */
  QMI_ERR_ENETUNREACH_V01 = 0x165, /**<  Network is unreachable  */
  QMI_ERR_ENETRESET_V01 = 0x166, /**<  Network dropped connection because of reset  */
  QMI_ERR_ECONNABORTED_V01 = 0x167, /**<  Software caused connection abort  */
  QMI_ERR_ECONNRESET_V01 = 0x168, /**<  Connection reset by peer  */
  QMI_ERR_ENOBUFS_V01 = 0x169, /**<  No buffer space available  */
  QMI_ERR_EISCONN_V01 = 0x16A, /**<  Transport endpoint is already connected  */
  QMI_ERR_ENOTCONN_V01 = 0x16B, /**<  Transport endpoint is not connected  */
  QMI_ERR_ESHUTDOWN_V01 = 0x16C, /**<  Cannot send after transport endpoint shutdown  */
  QMI_ERR_ETOOMANYREFS_V01 = 0x16D, /**<  Too many references: cannot splice  */
  QMI_ERR_ETIMEDOUT_V01 = 0x16E, /**<  Connection timed out  */
  QMI_ERR_ECONNREFUSED_V01 = 0x16F, /**<  Connection refused  */
  QMI_ERR_EHOSTDOWN_V01 = 0x170, /**<  Host is down  */
  QMI_ERR_EHOSTUNREACH_V01 = 0x171, /**<  No route to host  */
  QMI_ERR_EALREADY_V01 = 0x172, /**<  Operation already in progress  */
  QMI_ERR_EINPROGRESS_V01 = 0x173, /**<  Operation now in progress  */
  QMI_ERR_ESTALE_V01 = 0x174, /**<  Stale NFS file handle  */
  QMI_ERR_EUCLEAN_V01 = 0x175, /**<  Structure needs cleaning  */
  QMI_ERR_ENOTNAM_V01 = 0x176, /**<  Not a XENIX named type file  */
  QMI_ERR_ENAVAIL_V01 = 0x177, /**<  No XENIX semaphores available  */
  QMI_ERR_EISNAM_V01 = 0x178, /**<  Is a named type file  */
  QMI_ERR_EREMOTEIO_V01 = 0x179, /**<  Remote I/O error  */
  QMI_ERR_EDQUOT_V01 = 0x17A, /**<  Quota exceeded  */
  QMI_ERR_ENOMEDIUM_V01 = 0x17B, /**<  No medium found  */
  QMI_ERR_EMEDIUMTYPE_V01 = 0x17C, /**<  Wrong medium type  */
  QMI_ERR_ECANCELED_V01 = 0x17D, /**<  Operation Canceled  */
  QMI_ERR_ENOKEY_V01 = 0x17E, /**<  Required key not available  */
  QMI_ERR_EKEYEXPIRED_V01 = 0x17F, /**<  Key has expired  */
  QMI_ERR_EKEYREVOKED_V01 = 0x180, /**<  Key has been revoked  */
  QMI_ERR_EKEYREJECTED_V01 = 0x181, /**<  Key was rejected by service  */
  QMI_ERR_EOWNERDEAD_V01 = 0x182, /**<  Owner died  */
  QMI_ERR_ENOTRECOVERABLE_V01 = 0x183, /**<  State not recoverable  */
  QMI_ERROR_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_error_type_v01;

typedef struct {
  qmi_result_type_v01 result;
  qmi_error_type_v01 error;
}qmi_response_type_v01;  /* Type */

typedef enum {
  NAS_SUBS_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  NAS_PRIMARY_SUBSCRIPTION_V01 = 0x00, /**<  Primary subscription \n  */
  NAS_SECONDARY_SUBSCRIPTION_V01 = 0x01, /**<  Secondary subscription \n  */
  NAS_TERTIARY_SUBSCRIPTION_V01 = 0x02, /**<  Tertiary subscription  */
  NAS_SUBS_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}nas_subs_type_enum_v01;

typedef struct {
  nas_subs_type_enum_v01 subs_type;
}nas_bind_subscription_req_msg_v01;  /* Message */
//2021-03-24 willa.liu@fibocom.com changed end for support mantis 0071817
#pragma pack(pop)

#endif// MPQMUX_H
