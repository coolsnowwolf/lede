/******************************************************************************
  @file    QMIThread.c
  @brief   QMI WWAN connectivity manager.

  DESCRIPTION
  Connectivity Management Tool for USB network adapter of Quectel wireless cellular modules.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None.

  ---------------------------------------------------------------------------
  Copyright (c) 2016 - 2023 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
#include "QMIThread.h"
#include <inttypes.h>

#ifndef MIN
#define MIN(a, b)	((a) < (b)? (a): (b))
#endif

#define qmi_rsp_check_and_return() do { \
        if (err < 0 || pResponse == NULL) { \
            dbg_time("%s err = %d", __func__, err); \
            return err; \
        } \
        pMUXMsg = &pResponse->MUXMsg; \
        if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) { \
            USHORT QMUXError = le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError); \
            dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__, \
                le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), QMUXError); \
            free(pResponse); \
            return QMUXError; \
        } \
} while(0)

#define qmi_rsp_check() do { \
        if (err < 0 || pResponse == NULL) { \
            dbg_time("%s err = %d", __func__, err); \
            return err; \
        } \
        pMUXMsg = &pResponse->MUXMsg; \
        if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) { \
            USHORT QMUXError = le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError); \
            dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__, \
                le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), QMUXError); \
        } \
} while(0)

static uint32_t WdsConnectionIPv4Handle = 0;
static uint32_t WdsConnectionIPv6Handle = 0;
static int s_is_cdma = 0;
static int s_5g_type = WWAN_DATA_CLASS_NONE;
static int s_hdr_personality = 0; // 0x01-HRPD, 0x02-eHRPD
static char *qstrcpy(char *to, const char *from) { //no __strcpy_chk
	char *save = to;
	for (; (*to = *from) != '\0'; ++from, ++to);
	return(save);
}

static void uchar2char(char *dst_ptr, size_t dst_len, const UCHAR *src_ptr, size_t src_len) { 
    size_t copy = MIN(dst_len-1, src_len);

    if (copy)
        memcpy(dst_ptr, src_ptr, copy);
    dst_ptr[copy] = 0;
}

static int s_9x07 = 1;

typedef USHORT (*CUSTOMQMUX)(PQMUX_MSG pMUXMsg, void *arg);

// To retrieve the ith (Index) TLV
PQMI_TLV_HDR GetTLV (PQCQMUX_MSG_HDR pQMUXMsgHdr, int TLVType) {
    int TLVFind = 0;
    USHORT Length = le16_to_cpu(pQMUXMsgHdr->Length);
    PQMI_TLV_HDR pTLVHdr = (PQMI_TLV_HDR)(pQMUXMsgHdr + 1);

    while (Length >= sizeof(QMI_TLV_HDR)) {
        TLVFind++;
        if (TLVType > 0x1000) {
            if ((TLVFind + 0x1000) == TLVType)
                return pTLVHdr;
        } else  if (pTLVHdr->TLVType == TLVType) {
            return pTLVHdr;
        }

        Length -= (le16_to_cpu((pTLVHdr->TLVLength)) + sizeof(QMI_TLV_HDR));
        pTLVHdr = (PQMI_TLV_HDR)(((UCHAR *)pTLVHdr) + le16_to_cpu(pTLVHdr->TLVLength) + sizeof(QMI_TLV_HDR));
    }

   return NULL;
}

static USHORT GetQMUXTransactionId(void) {
    static int TransactionId = 0;
    if (++TransactionId > 0xFFFF)
        TransactionId = 1;
    return TransactionId;
}

static PQCQMIMSG ComposeQMUXMsg(UCHAR QMIType, USHORT Type, CUSTOMQMUX customQmuxMsgFunction, void *arg) {
    UCHAR QMIBuf[WDM_DEFAULT_BUFSIZE];
    PQCQMIMSG pRequest = (PQCQMIMSG)QMIBuf;
    int Length;

    memset(QMIBuf, 0x00, sizeof(QMIBuf));
    pRequest->QMIHdr.IFType = USB_CTL_MSG_TYPE_QMI;
    pRequest->QMIHdr.CtlFlags = 0x00;
    pRequest->QMIHdr.QMIType = QMIType;

    pRequest->MUXMsg.QMUXHdr.CtlFlags = QMUX_CTL_FLAG_SINGLE_MSG | QMUX_CTL_FLAG_TYPE_CMD;
    pRequest->MUXMsg.QMUXHdr.TransactionId = cpu_to_le16(GetQMUXTransactionId());
    pRequest->MUXMsg.QMUXMsgHdr.Type = cpu_to_le16(Type);
    if (customQmuxMsgFunction)
        pRequest->MUXMsg.QMUXMsgHdr.Length = cpu_to_le16(customQmuxMsgFunction(&pRequest->MUXMsg, arg) - sizeof(QCQMUX_MSG_HDR));
    else
        pRequest->MUXMsg.QMUXMsgHdr.Length = cpu_to_le16(0x0000);

    pRequest->QMIHdr.Length = cpu_to_le16(le16_to_cpu(pRequest->MUXMsg.QMUXMsgHdr.Length) + sizeof(QCQMUX_MSG_HDR) + sizeof(QCQMUX_HDR)
        + sizeof(QCQMI_HDR) - 1);
    Length = le16_to_cpu(pRequest->QMIHdr.Length) + 1;

    pRequest = (PQCQMIMSG)malloc(Length);
    if (pRequest == NULL) {
        dbg_time("%s fail to malloc", __func__);
    } else {
        memcpy(pRequest, QMIBuf, Length);
    }

    return pRequest;
}

#if 0
static USHORT NasSetEventReportReq(PQMUX_MSG pMUXMsg, void *arg) {
    pMUXMsg->SetEventReportReq.TLVType = 0x10;
    pMUXMsg->SetEventReportReq.TLVLength = 0x04;
    pMUXMsg->SetEventReportReq.ReportSigStrength = 0x00;
    pMUXMsg->SetEventReportReq.NumTresholds = 2;
    pMUXMsg->SetEventReportReq.TresholdList[0] = -113;
    pMUXMsg->SetEventReportReq.TresholdList[1] = -50;
    return sizeof(QMINAS_SET_EVENT_REPORT_REQ_MSG);
}

static USHORT WdsSetEventReportReq(PQMUX_MSG pMUXMsg, void *arg) {
    pMUXMsg->EventReportReq.TLVType = 0x10;          // 0x10 -- current channel rate indicator
    pMUXMsg->EventReportReq.TLVLength = 0x0001;        // 1
    pMUXMsg->EventReportReq.Mode = 0x00;             // 0-do not report; 1-report when rate changes

    pMUXMsg->EventReportReq.TLV2Type = 0x11;         // 0x11
    pMUXMsg->EventReportReq.TLV2Length = 0x0005;       // 5
    pMUXMsg->EventReportReq.StatsPeriod = 0x00;      // seconds between reports; 0-do not report
    pMUXMsg->EventReportReq.StatsMask = 0x000000ff;        //

    pMUXMsg->EventReportReq.TLV3Type = 0x12;          // 0x12 -- current data bearer indicator
    pMUXMsg->EventReportReq.TLV3Length = 0x0001;        // 1
    pMUXMsg->EventReportReq.Mode3 = 0x01;             // 0-do not report; 1-report when changes

    pMUXMsg->EventReportReq.TLV4Type = 0x13;          // 0x13 -- dormancy status indicator
    pMUXMsg->EventReportReq.TLV4Length = 0x0001;        // 1
    pMUXMsg->EventReportReq.DormancyStatus = 0x00;    // 0-do not report; 1-report when changes
    return sizeof(QMIWDS_SET_EVENT_REPORT_REQ_MSG);
}

static USHORT DmsSetEventReportReq(PQMUX_MSG pMUXMsg) {
    PPIN_STATUS pPinState = (PPIN_STATUS)(&pMUXMsg->DmsSetEventReportReq + 1);
    PUIM_STATE pUimState = (PUIM_STATE)(pPinState + 1);
    // Pin State
    pPinState->TLVType = 0x12;
    pPinState->TLVLength = 0x01;
    pPinState->ReportPinState = 0x01;
    // UIM State
    pUimState->TLVType = 0x15;
    pUimState->TLVLength = 0x01;
    pUimState->UIMState = 0x01;
    return sizeof(QMIDMS_SET_EVENT_REPORT_REQ_MSG) + sizeof(PIN_STATUS) + sizeof(UIM_STATE);
}
#endif

static USHORT WdsStartNwInterfaceReq(PQMUX_MSG pMUXMsg, void *arg) {
    PQMIWDS_TECHNOLOGY_PREFERECE pTechPref;
    PQMIWDS_AUTH_PREFERENCE pAuthPref;
    PQMIWDS_USERNAME pUserName;
    PQMIWDS_PASSWD pPasswd;
    PQMIWDS_APNNAME pApnName;
    PQMIWDS_IP_FAMILY_TLV pIpFamily;
    USHORT TLVLength = 0;
    UCHAR *pTLV;
    PROFILE_T *profile = (PROFILE_T *)arg;
    const char *profile_user = profile->user;
    const char *profile_password = profile->password;
    int profile_auth = profile->auth;

    if (s_is_cdma && (profile_user == NULL || profile_user[0] == '\0') && (profile_password == NULL || profile_password[0] == '\0')) {
        profile_user = "ctnet@mycdma.cn";
        profile_password = "vnet.mobi";
        profile_auth = 2; //chap
    }

    pTLV = (UCHAR *)(&pMUXMsg->StartNwInterfaceReq + 1);
    pMUXMsg->StartNwInterfaceReq.Length = 0;

    // Set technology Preferece
    pTechPref = (PQMIWDS_TECHNOLOGY_PREFERECE)(pTLV + TLVLength);
    pTechPref->TLVType = 0x30;
    pTechPref->TLVLength = cpu_to_le16(0x01);
    if (s_is_cdma == 0)
        pTechPref->TechPreference = 0x01;
    else
        pTechPref->TechPreference = 0x02;
    TLVLength +=(le16_to_cpu(pTechPref->TLVLength) + sizeof(QCQMICTL_TLV_HDR));

    // Set APN Name
    if (profile->apn && !s_is_cdma) { //cdma no apn
        pApnName = (PQMIWDS_APNNAME)(pTLV + TLVLength);
        pApnName->TLVType = 0x14;
        pApnName->TLVLength = cpu_to_le16(strlen(profile->apn));
        qstrcpy((char *)&pApnName->ApnName, profile->apn);
        TLVLength +=(le16_to_cpu(pApnName->TLVLength) + sizeof(QCQMICTL_TLV_HDR));
    }

    // Set User Name
    if (profile_user) {
        pUserName = (PQMIWDS_USERNAME)(pTLV + TLVLength);
        pUserName->TLVType = 0x17;
        pUserName->TLVLength = cpu_to_le16(strlen(profile_user));
        qstrcpy((char *)&pUserName->UserName, profile_user);
        TLVLength += (le16_to_cpu(pUserName->TLVLength) + sizeof(QCQMICTL_TLV_HDR));
    }

    // Set Password
    if (profile_password) {
        pPasswd = (PQMIWDS_PASSWD)(pTLV + TLVLength);
        pPasswd->TLVType = 0x18;
        pPasswd->TLVLength = cpu_to_le16(strlen(profile_password));
        qstrcpy((char *)&pPasswd->Passwd, profile_password);
	TLVLength += (le16_to_cpu(pPasswd->TLVLength) + sizeof(QCQMICTL_TLV_HDR));
    }

    // Set Auth Protocol
    if (profile_user && profile_password) {
        pAuthPref = (PQMIWDS_AUTH_PREFERENCE)(pTLV + TLVLength);
        pAuthPref->TLVType = 0x16;
        pAuthPref->TLVLength = cpu_to_le16(0x01);
        pAuthPref->AuthPreference = profile_auth; // 0 ~ None, 1 ~ Pap, 2 ~ Chap, 3 ~ MsChapV2
        TLVLength += (le16_to_cpu(pAuthPref->TLVLength) + sizeof(QCQMICTL_TLV_HDR));
    }

    // Add IP Family Preference
    pIpFamily = (PQMIWDS_IP_FAMILY_TLV)(pTLV + TLVLength);
    pIpFamily->TLVType = 0x19;
    pIpFamily->TLVLength = cpu_to_le16(0x01);
    pIpFamily->IpFamily = profile->curIpFamily;
    TLVLength += (le16_to_cpu(pIpFamily->TLVLength) + sizeof(QCQMICTL_TLV_HDR));

    //Set Profile Index
    if (profile->profile_index && !s_is_cdma) { //cdma only support one pdp, so no need to set profile index
        PQMIWDS_PROFILE_IDENTIFIER pProfileIndex = (PQMIWDS_PROFILE_IDENTIFIER)(pTLV + TLVLength);
        pProfileIndex->TLVLength = cpu_to_le16(0x01);
        pProfileIndex->TLVType = 0x31;
        pProfileIndex->ProfileIndex = profile->profile_index;
        if (s_is_cdma && s_hdr_personality == 0x02) {
            pProfileIndex->TLVType = 0x32; //profile_index_3gpp2
            pProfileIndex->ProfileIndex = 101;
        }
        TLVLength += (le16_to_cpu(pProfileIndex->TLVLength) + sizeof(QCQMICTL_TLV_HDR));
    }

    return sizeof(QMIWDS_START_NETWORK_INTERFACE_REQ_MSG) + TLVLength;
}

static USHORT WdsStopNwInterfaceReq(PQMUX_MSG pMUXMsg, void *arg) {
    pMUXMsg->StopNwInterfaceReq.TLVType = 0x01;
    pMUXMsg->StopNwInterfaceReq.TLVLength = cpu_to_le16(0x04);
    if (*((int *)arg) == IpFamilyV4)
        pMUXMsg->StopNwInterfaceReq.Handle =  cpu_to_le32(WdsConnectionIPv4Handle);
    else
        pMUXMsg->StopNwInterfaceReq.Handle =  cpu_to_le32(WdsConnectionIPv6Handle);
    return sizeof(QMIWDS_STOP_NETWORK_INTERFACE_REQ_MSG);
}

static USHORT WdsSetClientIPFamilyPref(PQMUX_MSG pMUXMsg, void *arg) {
    pMUXMsg->SetClientIpFamilyPrefReq.TLVType = 0x01;
    pMUXMsg->SetClientIpFamilyPrefReq.TLVLength = cpu_to_le16(0x01);
    pMUXMsg->SetClientIpFamilyPrefReq.IpPreference = *((UCHAR *)arg);
    return sizeof(QMIWDS_SET_CLIENT_IP_FAMILY_PREF_REQ_MSG);
}

static USHORT WdsSetAutoConnect(PQMUX_MSG pMUXMsg, void *arg) {
    pMUXMsg->SetAutoConnectReq.TLVType = 0x01;
    pMUXMsg->SetAutoConnectReq.TLVLength = cpu_to_le16(0x01);
    pMUXMsg->SetAutoConnectReq.autoconnect_setting = *((UCHAR *)arg);
    return sizeof(QMIWDS_SET_AUTO_CONNECT_REQ_MSG);
}

enum peripheral_ep_type {
	DATA_EP_TYPE_RESERVED	= 0x0,
	DATA_EP_TYPE_HSIC	= 0x1,
	DATA_EP_TYPE_HSUSB	= 0x2,
	DATA_EP_TYPE_PCIE	= 0x3,
	DATA_EP_TYPE_EMBEDDED	= 0x4,
	DATA_EP_TYPE_BAM_DMUX	= 0x5,
};
        
static USHORT WdsSetQMUXBindMuxDataPort(PQMUX_MSG pMUXMsg, void *arg) {
    QMAP_SETTING *qmap_settings = (QMAP_SETTING *)arg;

    pMUXMsg->BindMuxDataPortReq.TLVType = 0x10;
    pMUXMsg->BindMuxDataPortReq.TLVLength = cpu_to_le16(0x08);
    pMUXMsg->BindMuxDataPortReq.ep_type = cpu_to_le32(qmap_settings->ep_type);
    pMUXMsg->BindMuxDataPortReq.iface_id = cpu_to_le32(qmap_settings->iface_id);
    pMUXMsg->BindMuxDataPortReq.TLV2Type = 0x11;
    pMUXMsg->BindMuxDataPortReq.TLV2Length = cpu_to_le16(0x01);
    pMUXMsg->BindMuxDataPortReq.MuxId = qmap_settings->MuxId;
    pMUXMsg->BindMuxDataPortReq.TLV3Type = 0x13;
    pMUXMsg->BindMuxDataPortReq.TLV3Length = cpu_to_le16(0x04);
    pMUXMsg->BindMuxDataPortReq.client_type = cpu_to_le32(1); //WDS_CLIENT_TYPE_TETHERED
    
    return sizeof(QMIWDS_BIND_MUX_DATA_PORT_REQ_MSG);
}

static int qmap_version = 0x05;
static USHORT WdaSetDataFormat(PQMUX_MSG pMUXMsg, void *arg) {
    QMAP_SETTING *qmap_settings = (QMAP_SETTING *)arg;

    if (qmap_settings->rx_urb_size == 0) {
        PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV_QOS pWdsAdminQosTlv;
        PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV linkProto;
        PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV dlTlp;

        pWdsAdminQosTlv = (PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV_QOS)(&pMUXMsg->QMUXMsgHdr + 1);
        pWdsAdminQosTlv->TLVType = 0x10;
        pWdsAdminQosTlv->TLVLength = cpu_to_le16(0x0001);
        pWdsAdminQosTlv->QOSSetting = 0; /* no-QOS header */

        linkProto = (PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV)(pWdsAdminQosTlv + 1);
        linkProto->TLVType = 0x11;
        linkProto->TLVLength = cpu_to_le16(4);
        linkProto->Value = cpu_to_le32(0x01);     /* Set Ethernet  mode */

        dlTlp = (PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV)(linkProto + 1);;
        dlTlp->TLVType = 0x13;
        dlTlp->TLVLength = cpu_to_le16(4);
        dlTlp->Value = cpu_to_le32(0x00);

        if (sizeof(*linkProto) != 7 )
            dbg_time("%s sizeof(*linkProto) = %zu, is not 7!", __func__, sizeof(*linkProto) );

        return sizeof(QCQMUX_MSG_HDR) + sizeof(*pWdsAdminQosTlv) + sizeof(*linkProto) + sizeof(*dlTlp);
    } 
    else {
    //Indicates whether the Quality of Service(QOS) data format is used by the client.
        pMUXMsg->SetDataFormatReq.QosDataFormatTlv.TLVType = 0x10;
        pMUXMsg->SetDataFormatReq.QosDataFormatTlv.TLVLength = cpu_to_le16(0x0001);
        pMUXMsg->SetDataFormatReq.QosDataFormatTlv.QOSSetting = 0; /* no-QOS header */
		
    //Underlying Link Layer Protocol
        pMUXMsg->SetDataFormatReq.UnderlyingLinkLayerProtocolTlv.TLVType = 0x11; 
        pMUXMsg->SetDataFormatReq.UnderlyingLinkLayerProtocolTlv.TLVLength = cpu_to_le16(4);
        pMUXMsg->SetDataFormatReq.UnderlyingLinkLayerProtocolTlv.Value = cpu_to_le32(0x02);     /* Set IP  mode */

    //Uplink (UL) data aggregation protocol to be used for uplink data transfer.
        pMUXMsg->SetDataFormatReq.UplinkDataAggregationProtocolTlv.TLVType = 0x12; 
        pMUXMsg->SetDataFormatReq.UplinkDataAggregationProtocolTlv.TLVLength = cpu_to_le16(4);
        pMUXMsg->SetDataFormatReq.UplinkDataAggregationProtocolTlv.Value = cpu_to_le32(qmap_version); //UL QMAP is enabled
        
    //Downlink (DL) data aggregation protocol to be used for downlink data transfer
        pMUXMsg->SetDataFormatReq.DownlinkDataAggregationProtocolTlv.TLVType = 0x13; 
        pMUXMsg->SetDataFormatReq.DownlinkDataAggregationProtocolTlv.TLVLength = cpu_to_le16(4);
        pMUXMsg->SetDataFormatReq.DownlinkDataAggregationProtocolTlv.Value = cpu_to_le32(qmap_version); //DL QMAP is enabled
        
    //Maximum number of datagrams in a single aggregated packet on downlink
        pMUXMsg->SetDataFormatReq.DownlinkDataAggregationMaxDatagramsTlv.TLVType = 0x15; 
        pMUXMsg->SetDataFormatReq.DownlinkDataAggregationMaxDatagramsTlv.TLVLength = cpu_to_le16(4);
        pMUXMsg->SetDataFormatReq.DownlinkDataAggregationMaxDatagramsTlv.Value = cpu_to_le32(qmap_settings->rx_urb_size/512);
		
    //Maximum size in bytes of a single aggregated packet allowed on downlink
        pMUXMsg->SetDataFormatReq.DownlinkDataAggregationMaxSizeTlv.TLVType = 0x16; 
        pMUXMsg->SetDataFormatReq.DownlinkDataAggregationMaxSizeTlv.TLVLength = cpu_to_le16(4);
        pMUXMsg->SetDataFormatReq.DownlinkDataAggregationMaxSizeTlv.Value = cpu_to_le32(qmap_settings->rx_urb_size);

    //Peripheral End Point ID
        pMUXMsg->SetDataFormatReq.epTlv.TLVType = 0x17; 
        pMUXMsg->SetDataFormatReq.epTlv.TLVLength = cpu_to_le16(8);
        pMUXMsg->SetDataFormatReq.epTlv.ep_type = cpu_to_le32(qmap_settings->ep_type);
        pMUXMsg->SetDataFormatReq.epTlv.iface_id = cpu_to_le32(qmap_settings->iface_id); 

#ifdef QUECTEL_UL_DATA_AGG
        if (!qmap_settings->ul_data_aggregation_max_datagrams) {
            return ((size_t)&((QMIWDS_ADMIN_SET_DATA_FORMAT_REQ_MSG *)0)->DlMinimumPassingTlv);
        }
        
     //Maximum number of datagrams in a single aggregated packet on uplink
        pMUXMsg->SetDataFormatReq.DlMinimumPassingTlv.TLVType = 0x19; 
        pMUXMsg->SetDataFormatReq.DlMinimumPassingTlv.TLVLength = cpu_to_le16(4);
        pMUXMsg->SetDataFormatReq.DlMinimumPassingTlv.Value = cpu_to_le32(qmap_settings->dl_minimum_padding);
		
     //Maximum number of datagrams in a single aggregated packet on uplink
        pMUXMsg->SetDataFormatReq.UplinkDataAggregationMaxDatagramsTlv.TLVType = 0x1B; 
        pMUXMsg->SetDataFormatReq.UplinkDataAggregationMaxDatagramsTlv.TLVLength = cpu_to_le16(4);
        pMUXMsg->SetDataFormatReq.UplinkDataAggregationMaxDatagramsTlv.Value = cpu_to_le32(qmap_settings->ul_data_aggregation_max_datagrams);
		
    //Maximum size in bytes of a single aggregated packet allowed on downlink
        pMUXMsg->SetDataFormatReq.UplinkDataAggregationMaxSizeTlv.TLVType = 0x1C; 
        pMUXMsg->SetDataFormatReq.UplinkDataAggregationMaxSizeTlv.TLVLength = cpu_to_le16(4);
        pMUXMsg->SetDataFormatReq.UplinkDataAggregationMaxSizeTlv.Value = cpu_to_le32(qmap_settings->ul_data_aggregation_max_size);
#endif

        return sizeof(QMIWDS_ADMIN_SET_DATA_FORMAT_REQ_MSG);
    }
}

#ifdef CONFIG_SIM
static USHORT DmsUIMVerifyPinReqSend(PQMUX_MSG pMUXMsg, void *arg) {
    pMUXMsg->UIMVerifyPinReq.TLVType = 0x01;
    pMUXMsg->UIMVerifyPinReq.PINID = 0x01; //Pin1, not Puk
    pMUXMsg->UIMVerifyPinReq.PINLen = strlen((const char *)arg);
    qstrcpy((char *)&pMUXMsg->UIMVerifyPinReq.PINValue, ((const char *)arg));
    pMUXMsg->UIMVerifyPinReq.TLVLength = cpu_to_le16(2 + strlen((const char *)arg));
    return sizeof(QMIDMS_UIM_VERIFY_PIN_REQ_MSG) + (strlen((const char *)arg) - 1);
}

static USHORT UimVerifyPinReqSend(PQMUX_MSG pMUXMsg, void *arg)
{
    pMUXMsg->UIMUIMVerifyPinReq.TLVType = 0x01;
    pMUXMsg->UIMUIMVerifyPinReq.TLVLength = cpu_to_le16(0x02);
    pMUXMsg->UIMUIMVerifyPinReq.Session_Type = 0x00;
    pMUXMsg->UIMUIMVerifyPinReq.Aid_Len = 0x00;
    pMUXMsg->UIMUIMVerifyPinReq.TLV2Type = 0x02;
    pMUXMsg->UIMUIMVerifyPinReq.TLV2Length = cpu_to_le16(2 + strlen((const char *)arg));
    pMUXMsg->UIMUIMVerifyPinReq.PINID = 0x01;  //Pin1, not Puk
    pMUXMsg->UIMUIMVerifyPinReq.PINLen= strlen((const char *)arg);
    qstrcpy((char *)&pMUXMsg->UIMUIMVerifyPinReq.PINValue, ((const char *)arg));
    return sizeof(QMIUIM_VERIFY_PIN_REQ_MSG) + (strlen((const char *)arg) - 1);
}

#ifdef CONFIG_IMSI_ICCID
static USHORT UimReadTransparentIMSIReqSend(PQMUX_MSG pMUXMsg, void *arg) {
    PREAD_TRANSPARENT_TLV pReadTransparent;

    pMUXMsg->UIMUIMReadTransparentReq.TLVType =  0x01;
    pMUXMsg->UIMUIMReadTransparentReq.TLVLength = cpu_to_le16(0x02);
    if (!strcmp((char *)arg, "EF_ICCID")) {
        pMUXMsg->UIMUIMReadTransparentReq.Session_Type = 0x06;
        pMUXMsg->UIMUIMReadTransparentReq.Aid_Len = 0x00;

        pMUXMsg->UIMUIMReadTransparentReq.TLV2Type = 0x02;
        pMUXMsg->UIMUIMReadTransparentReq.file_id = cpu_to_le16(0x2FE2);
        pMUXMsg->UIMUIMReadTransparentReq.path_len = 0x02;
        pMUXMsg->UIMUIMReadTransparentReq.path[0] = 0x00;
        pMUXMsg->UIMUIMReadTransparentReq.path[1] = 0x3F;
    }
    else if(!strcmp((char *)arg, "EF_IMSI")) {
        pMUXMsg->UIMUIMReadTransparentReq.Session_Type = 0x00;
        pMUXMsg->UIMUIMReadTransparentReq.Aid_Len = 0x00;

        pMUXMsg->UIMUIMReadTransparentReq.TLV2Type = 0x02;
        pMUXMsg->UIMUIMReadTransparentReq.file_id = cpu_to_le16(0x6F07);
        pMUXMsg->UIMUIMReadTransparentReq.path_len = 0x04;
        pMUXMsg->UIMUIMReadTransparentReq.path[0] = 0x00;
        pMUXMsg->UIMUIMReadTransparentReq.path[1] = 0x3F;
        pMUXMsg->UIMUIMReadTransparentReq.path[2] = 0xFF;
        pMUXMsg->UIMUIMReadTransparentReq.path[3] = 0x7F;
    }

    pMUXMsg->UIMUIMReadTransparentReq.TLV2Length = cpu_to_le16(3 +  pMUXMsg->UIMUIMReadTransparentReq.path_len);

    pReadTransparent = (PREAD_TRANSPARENT_TLV)(&pMUXMsg->UIMUIMReadTransparentReq.path[pMUXMsg->UIMUIMReadTransparentReq.path_len]);
    pReadTransparent->TLVType = 0x03;
    pReadTransparent->TLVLength = cpu_to_le16(0x04);
    pReadTransparent->Offset = cpu_to_le16(0x00);
    pReadTransparent->Length = cpu_to_le16(0x00);

    return (sizeof(QMIUIM_READ_TRANSPARENT_REQ_MSG) + pMUXMsg->UIMUIMReadTransparentReq.path_len + sizeof(READ_TRANSPARENT_TLV));
}
#endif
#endif

#ifdef CONFIG_APN

static USHORT WdsGetProfileListReqSend(PQMUX_MSG pMUXMsg, void *arg) {
    (void)(arg);
    pMUXMsg->GetProfileListReq.Length = cpu_to_le16(sizeof(QMIWDS_GET_PROFILE_LIST_REQ_MSG) - 4);
    return sizeof(QMIWDS_GET_PROFILE_LIST_REQ_MSG);
}

static USHORT WdsCreateProfileSettingsReqSend(PQMUX_MSG pMUXMsg, void *arg) {
    PROFILE_T *profile = (PROFILE_T *)arg;
    pMUXMsg->CreatetProfileSettingsReq.Length = cpu_to_le16(sizeof(QMIWDS_CREATE_PROFILE_SETTINGS_REQ_MSG) - 4);
    pMUXMsg->CreatetProfileSettingsReq.TLVType = 0x01;
    pMUXMsg->CreatetProfileSettingsReq.TLVLength = cpu_to_le16(0x01);
    pMUXMsg->CreatetProfileSettingsReq.ProfileType = 0x00; // 0 ~ 3GPP, 1 ~ 3GPP2
    pMUXMsg->CreatetProfileSettingsReq.TLV2Type = 0x25;
    pMUXMsg->CreatetProfileSettingsReq.TLV2Length = cpu_to_le16(0x01);
    pMUXMsg->CreatetProfileSettingsReq.pdp_context = profile->pdp; // 0 ~ 3GPP, 1 ~ 3GPP2
    return sizeof(QMIWDS_CREATE_PROFILE_SETTINGS_REQ_MSG);
}

static USHORT WdsGetProfileSettingsReqSend(PQMUX_MSG pMUXMsg, void *arg) {
    PROFILE_T *profile = (PROFILE_T *)arg;
    pMUXMsg->GetProfileSettingsReq.Length = cpu_to_le16(sizeof(QMIWDS_GET_PROFILE_SETTINGS_REQ_MSG) - 4);
    pMUXMsg->GetProfileSettingsReq.TLVType = 0x01;
    pMUXMsg->GetProfileSettingsReq.TLVLength = cpu_to_le16(0x02);
    pMUXMsg->GetProfileSettingsReq.ProfileType = 0x00; // 0 ~ 3GPP, 1 ~ 3GPP2
    pMUXMsg->GetProfileSettingsReq.ProfileIndex = profile->profile_index;
    return sizeof(QMIWDS_GET_PROFILE_SETTINGS_REQ_MSG);
}

static USHORT WdsModifyProfileSettingsReq(PQMUX_MSG pMUXMsg, void *arg) {
    USHORT TLVLength = 0;
    UCHAR *pTLV;
    PROFILE_T *profile = (PROFILE_T *)arg;
    PQMIWDS_PDPTYPE pPdpType;

    pMUXMsg->ModifyProfileSettingsReq.Length = cpu_to_le16(sizeof(QMIWDS_MODIFY_PROFILE_SETTINGS_REQ_MSG) - 4);
    pMUXMsg->ModifyProfileSettingsReq.TLVType = 0x01;
    pMUXMsg->ModifyProfileSettingsReq.TLVLength = cpu_to_le16(0x02);
    pMUXMsg->ModifyProfileSettingsReq.ProfileType = 0x00; // 0 ~ 3GPP, 1 ~ 3GPP2
    pMUXMsg->ModifyProfileSettingsReq.ProfileIndex = profile->profile_index;

    pTLV = (UCHAR *)(&pMUXMsg->ModifyProfileSettingsReq + 1);

    pPdpType = (PQMIWDS_PDPTYPE)(pTLV + TLVLength);
    pPdpType->TLVType = 0x11;
    pPdpType->TLVLength = cpu_to_le16(0x01);
    pPdpType->PdpType = profile->iptype;
    TLVLength +=(le16_to_cpu(pPdpType->TLVLength) + sizeof(QCQMICTL_TLV_HDR));

    // Set APN Name
    if (profile->apn) {
        PQMIWDS_APNNAME pApnName = (PQMIWDS_APNNAME)(pTLV + TLVLength);
        pApnName->TLVType = 0x14;
        pApnName->TLVLength = cpu_to_le16(strlen(profile->apn));
        qstrcpy((char *)&pApnName->ApnName, profile->apn);
        TLVLength +=(le16_to_cpu(pApnName->TLVLength) + sizeof(QCQMICTL_TLV_HDR));
    }

    // Set User Name
    if (profile->user) {
        PQMIWDS_USERNAME pUserName = (PQMIWDS_USERNAME)(pTLV + TLVLength);
        pUserName->TLVType = 0x1B;
        pUserName->TLVLength = cpu_to_le16(strlen(profile->user));
        qstrcpy((char *)&pUserName->UserName, profile->user);
        TLVLength += (le16_to_cpu(pUserName->TLVLength) + sizeof(QCQMICTL_TLV_HDR));
    }

    // Set Password
    if (profile->password) {
        PQMIWDS_PASSWD pPasswd = (PQMIWDS_PASSWD)(pTLV + TLVLength);
        pPasswd->TLVType = 0x1C;
        pPasswd->TLVLength = cpu_to_le16(strlen(profile->password));
        qstrcpy((char *)&pPasswd->Passwd, profile->password);
        TLVLength +=(le16_to_cpu(pPasswd->TLVLength) + sizeof(QCQMICTL_TLV_HDR));
    }

    // Set Auth Protocol
    if (profile->user && profile->password) {
        PQMIWDS_AUTH_PREFERENCE pAuthPref = (PQMIWDS_AUTH_PREFERENCE)(pTLV + TLVLength);
        pAuthPref->TLVType = 0x1D;
        pAuthPref->TLVLength = cpu_to_le16(0x01);
        pAuthPref->AuthPreference = profile->auth; // 0 ~ None, 1 ~ Pap, 2 ~ Chap, 3 ~ MsChapV2
        TLVLength += (le16_to_cpu(pAuthPref->TLVLength) + sizeof(QCQMICTL_TLV_HDR));
    }

    return sizeof(QMIWDS_MODIFY_PROFILE_SETTINGS_REQ_MSG) + TLVLength;
}
#endif

static USHORT WdsGetRuntimeSettingReq(PQMUX_MSG pMUXMsg, void *arg)
{
    (void)arg;
    pMUXMsg->GetRuntimeSettingsReq.TLVType = 0x10;
    pMUXMsg->GetRuntimeSettingsReq.TLVLength = cpu_to_le16(0x04);
    // the following mask also applies to IPV6
    pMUXMsg->GetRuntimeSettingsReq.Mask = cpu_to_le32(QMIWDS_GET_RUNTIME_SETTINGS_MASK_IPV4DNS_ADDR |
                                                      QMIWDS_GET_RUNTIME_SETTINGS_MASK_IPV4_ADDR |
                                                      QMIWDS_GET_RUNTIME_SETTINGS_MASK_MTU |
                                                      QMIWDS_GET_RUNTIME_SETTINGS_MASK_IPV4GATEWAY_ADDR) |
                                          QMIWDS_GET_RUNTIME_SETTINGS_MASK_PCSCF_SV_ADDR |
                                          QMIWDS_GET_RUNTIME_SETTINGS_MASK_PCSCF_DOM_NAME;

    return sizeof(QMIWDS_GET_RUNTIME_SETTINGS_REQ_MSG);
}

static PQCQMIMSG s_pRequest;
static PQCQMIMSG s_pResponse;

static int is_response(const PQCQMIMSG pRequest, const PQCQMIMSG pResponse) {
    if ((pRequest->QMIHdr.QMIType == pResponse->QMIHdr.QMIType)
        && (pRequest->QMIHdr.ClientId == pResponse->QMIHdr.ClientId)) {
            USHORT requestTID, responseTID;
        if (pRequest->QMIHdr.QMIType == QMUX_TYPE_CTL) {
            requestTID = pRequest->CTLMsg.QMICTLMsgHdr.TransactionId;
            responseTID = pResponse->CTLMsg.QMICTLMsgHdr.TransactionId;
        } else {
            requestTID = le16_to_cpu(pRequest->MUXMsg.QMUXHdr.TransactionId);
            responseTID = le16_to_cpu(pResponse->MUXMsg.QMUXHdr.TransactionId);
        }
        return (requestTID == responseTID);
    }
    return 0;
}

int (*qmidev_send)(PQCQMIMSG pRequest);

int QmiThreadSendQMITimeout(PQCQMIMSG pRequest, PQCQMIMSG *ppResponse, unsigned msecs, const char *funcname) {
    int ret;
    
    if (!pRequest)
        return -EINVAL;

    pthread_mutex_lock(&cm_command_mutex);

    if (ppResponse)
        *ppResponse = NULL;

    dump_qmi(pRequest, le16_to_cpu(pRequest->QMIHdr.Length) + 1);

    s_pRequest = pRequest;
    s_pResponse = NULL;

    ret = qmidev_send(pRequest);

    if (ret == 0) {
        ret = pthread_cond_timeout_np(&cm_command_cond, &cm_command_mutex, msecs);
        if (!ret) {
            if (s_pResponse && ppResponse) {
                *ppResponse = s_pResponse;
            } else {
                if (s_pResponse) {
                    free(s_pResponse);
                    s_pResponse = NULL;
                }
            }
        } else {
            dbg_time("%s message timeout", funcname);
        }
    }

    pthread_mutex_unlock(&cm_command_mutex);

    return ret;
}

void QmiThreadRecvQMI(PQCQMIMSG pResponse) {
    pthread_mutex_lock(&cm_command_mutex);
    if (pResponse == NULL) {
        if (s_pRequest) {
            free(s_pRequest);
            s_pRequest = NULL;
            s_pResponse = NULL;
            pthread_cond_signal(&cm_command_cond);
        }
        pthread_mutex_unlock(&cm_command_mutex);
        return;
    }
    dump_qmi(pResponse, le16_to_cpu(pResponse->QMIHdr.Length) + 1);
    if (s_pRequest && is_response(s_pRequest, pResponse)) {
        free(s_pRequest);
        s_pRequest = NULL;
        s_pResponse = malloc(le16_to_cpu(pResponse->QMIHdr.Length) + 1);
        if (s_pResponse != NULL) {
            memcpy(s_pResponse, pResponse, le16_to_cpu(pResponse->QMIHdr.Length) + 1);
        }
        pthread_cond_signal(&cm_command_cond);
    } else if ((pResponse->QMIHdr.QMIType == QMUX_TYPE_CTL)
                    && (le16_to_cpu(pResponse->CTLMsg.QMICTLMsgHdrRsp.QMICTLType == QMICTL_REVOKE_CLIENT_ID_IND))) {
        qmidevice_send_event_to_main(MODEM_REPORT_RESET_EVENT);
    } else if ((pResponse->QMIHdr.QMIType == QMUX_TYPE_NAS)
                    && (le16_to_cpu(pResponse->MUXMsg.QMUXMsgHdrResp.Type) == QMINAS_SERVING_SYSTEM_IND)) {
        qmidevice_send_event_to_main(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED);
    } else if ((pResponse->QMIHdr.QMIType == QMUX_TYPE_WDS)
                    && (le16_to_cpu(pResponse->MUXMsg.QMUXMsgHdrResp.Type) == QMIWDS_GET_PKT_SRVC_STATUS_IND)) {
        qmidevice_send_event_to_main(RIL_UNSOL_DATA_CALL_LIST_CHANGED);
    } else if ((pResponse->QMIHdr.QMIType == QMUX_TYPE_NAS)
                    && (le16_to_cpu(pResponse->MUXMsg.QMUXMsgHdrResp.Type) == QMINAS_SYS_INFO_IND)) {
        qmidevice_send_event_to_main(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED);
    } else if ((pResponse->QMIHdr.QMIType == QMUX_TYPE_WDS_ADMIN)
                    && (le16_to_cpu(pResponse->MUXMsg.QMUXMsgHdrResp.Type) == QMI_WDA_SET_LOOPBACK_CONFIG_IND)) {
    	qmidevice_send_event_to_main_ext(RIL_UNSOL_LOOPBACK_CONFIG_IND,
            &pResponse->MUXMsg.SetLoopBackInd, sizeof(pResponse->MUXMsg.SetLoopBackInd));
    }
#ifdef CONFIG_REG_QOS_IND
    else if ((pResponse->QMIHdr.QMIType == QMUX_TYPE_QOS)
          && (le16_to_cpu(pResponse->MUXMsg.QMUXMsgHdrResp.Type) == QMI_QOS_GLOBAL_QOS_FLOW_IND)) {
        UINT qos_id = 0;
        UCHAR new_flow = ql_get_global_qos_flow_ind_qos_id(pResponse, &qos_id);
        if (qos_id != 0 && new_flow == 1)
            qmidevice_send_event_to_main_ext(RIL_UNSOL_GLOBAL_QOS_FLOW_IND_QOS_ID, &qos_id, sizeof(qos_id));
#ifdef CONFIG_GET_QOS_DATA_RATE
        if (new_flow) {
            ULONG64 max_data_rate[2] = {0};
            if (ql_get_global_qos_flow_ind_data_rate(pResponse, (void *)max_data_rate) == 0){}
        }
#endif
    }
#endif
    else {
        if (debug_qmi)
            dbg_time("nobody care this qmi msg!!");
    }
    pthread_mutex_unlock(&cm_command_mutex);
}

#ifdef CONFIG_COEX_WWAN_STATE
static int requestGetCoexWWANState(void) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    PQMI_COEX_GET_WWAN_STATE_RESP_MSG_LTE_BAND pLteBand;
    static QMI_COEX_GET_WWAN_STATE_RESP_MSG_LTE_BAND oldLteBand = {-1, -1};
    int err;

    pRequest = ComposeQMUXMsg(QMUX_TYPE_COEX, QMI_COEX_GET_WWAN_STATE_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);

    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__, le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        err = le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);
        free(pResponse);
        return err;
    } 
    pLteBand = (PQMI_COEX_GET_WWAN_STATE_RESP_MSG_LTE_BAND)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x10);

    if (pLteBand && memcmp(pLteBand, &oldLteBand, sizeof(oldLteBand))) {
        oldLteBand = *pLteBand;
        dbg_time("%s ul_freq %d ul_bandwidth %d", __func__, le32_to_cpu(pLteBand->ul_band.freq), le32_to_cpu(pLteBand->ul_band.bandwidth));
        dbg_time("%s dl_freq %d dl_bandwidth %d", __func__, le32_to_cpu(pLteBand->dl_band.freq), le32_to_cpu(pLteBand->dl_band.bandwidth));
    }
    free(pResponse);
    return 0;
}
#endif

static int requestSetEthMode(PROFILE_T *profile) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse = NULL;
    PQMUX_MSG pMUXMsg;
    int err;
    PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV linkProto;
    UCHAR IpPreference;
    UCHAR autoconnect_setting = 0;
    QMAP_SETTING qmap_settings = {0};

    qmap_settings.size = sizeof(qmap_settings);
    
    if (profile->qmap_mode) {
        profile->rawIP = 1;
        s_9x07 = profile->rawIP;
        
        qmap_settings.MuxId = profile->muxid;

        if (profile->hardware_interface == HARDWARE_PCIE) { //SDX20_PCIE
            qmap_settings.rx_urb_size = profile->qmap_size; //SDX24&SDX55 support 32KB 
            qmap_settings.ep_type = DATA_EP_TYPE_PCIE;
            qmap_settings.iface_id = 0x04;
        }
        else { // for MDM9x07&MDM9x40&SDX20 USB
            qmap_settings.rx_urb_size = profile->qmap_size; //SDX24&SDX55 support 32KB
            qmap_settings.ep_type = DATA_EP_TYPE_HSUSB;
            qmap_settings.iface_id = 0x04;
        }

        qmap_settings.ul_data_aggregation_max_datagrams = 11; //by test result, 11 can get best TPUT
        qmap_settings.ul_data_aggregation_max_size = 8*1024;
        qmap_settings.dl_minimum_padding = 0; //no effect when register to real netowrk
        if(profile->qmap_version != 0x09)
            profile->qmap_version = 0x05;

        qmap_version = profile->qmap_version;
        if (profile->rmnet_info.size) {
            qmap_settings.rx_urb_size = profile->rmnet_info.rx_urb_size;
            qmap_settings.ep_type = profile->rmnet_info.ep_type;
            qmap_settings.iface_id = profile->rmnet_info.iface_id;
            qmap_settings.dl_minimum_padding = profile->rmnet_info.dl_minimum_padding;
            qmap_version = profile->rmnet_info.qmap_version;
        }

        if (!profile->wda_client) {
            if (qmidev_is_gobinet(profile->qmichannel)) {
                //when QMAP enabled, set data format in GobiNet driver
            }
            else if (profile->proxy[0]) {
                /* the first running 'quectel-cm' had alloc wda client and set data format,
                   so we can ingore to set data format here. */
            }
            goto skip_WdaSetDataFormat;
        }
    }

    pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS_ADMIN, QMIWDS_ADMIN_SET_DATA_FORMAT_REQ, WdaSetDataFormat, (void *)&qmap_settings);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    linkProto = (PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x11);
    if (linkProto != NULL) {
        profile->rawIP = (le32_to_cpu(linkProto->Value) == 2);
        s_9x07 = profile->rawIP; //MDM90x7 only support RAW IP, do not support Eth
    }

    linkProto = (PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x16);
    if (linkProto != NULL && profile->qmap_mode) {
        qmap_settings.rx_urb_size = le32_to_cpu(linkProto->Value);
        dbg_time("qmap_settings.rx_urb_size = %u", qmap_settings.rx_urb_size); //must same as rx_urb_size defined in GobiNet&qmi_wwan driver
    }

#ifdef QUECTEL_UL_DATA_AGG
    if (qmap_settings.ul_data_aggregation_max_datagrams)
    {
        linkProto = (PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x17);
        if (linkProto != NULL) {
            qmap_settings.ul_data_aggregation_max_datagrams = MIN(qmap_settings.ul_data_aggregation_max_datagrams, le32_to_cpu(linkProto->Value));
            dbg_time("qmap_settings.ul_data_aggregation_max_datagrams  = %u", qmap_settings.ul_data_aggregation_max_datagrams);
        }

        linkProto = (PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x18);
        if (linkProto != NULL) {
            qmap_settings.ul_data_aggregation_max_size = MIN(qmap_settings.ul_data_aggregation_max_size, le32_to_cpu(linkProto->Value));
            dbg_time("qmap_settings.ul_data_aggregation_max_size       = %u", qmap_settings.ul_data_aggregation_max_size);
        }

        linkProto = (PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x1A);
        if (linkProto != NULL) {
            qmap_settings.dl_minimum_padding = le32_to_cpu(linkProto->Value);
            dbg_time("qmap_settings.dl_minimum_padding                 = %u", qmap_settings.dl_minimum_padding);
        }

	if (qmap_settings.ul_data_aggregation_max_datagrams > 1) {
		ql_set_driver_qmap_setting(profile, &qmap_settings);
	}
    }
#endif

    free(pResponse);

skip_WdaSetDataFormat:
    if (profile->enable_ipv4) {
    	if (profile->qmapnet_adapter[0]) {
            // bind wds mux data port
        	pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS, QMIWDS_BIND_MUX_DATA_PORT_REQ , WdsSetQMUXBindMuxDataPort, (void *)&qmap_settings);
        	err = QmiThreadSendQMI(pRequest, &pResponse);
        	qmi_rsp_check_and_return();
        	if (pResponse) free(pResponse);
        }

        // set ipv4
    	IpPreference = IpFamilyV4;
    	pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS, QMIWDS_SET_CLIENT_IP_FAMILY_PREF_REQ, WdsSetClientIPFamilyPref, (void *)&IpPreference);
    	err = QmiThreadSendQMI(pRequest, &pResponse);
    	if (pResponse) free(pResponse);
    }

    if (profile->enable_ipv6) {
        if (profile->qmapnet_adapter[0]) {
            // bind wds ipv6 mux data port
            pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS_IPV6, QMIWDS_BIND_MUX_DATA_PORT_REQ , WdsSetQMUXBindMuxDataPort, (void *)&qmap_settings);
            err = QmiThreadSendQMI(pRequest, &pResponse);
            qmi_rsp_check_and_return();
            if (pResponse) free(pResponse);
        }

        // set ipv6
        IpPreference = IpFamilyV6;
        pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS_IPV6, QMIWDS_SET_CLIENT_IP_FAMILY_PREF_REQ, WdsSetClientIPFamilyPref, (void *)&IpPreference);
        err = QmiThreadSendQMI(pRequest, &pResponse);
        qmi_rsp_check_and_return();
        if (pResponse) free(pResponse);
    }

    pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS, QMIWDS_SET_AUTO_CONNECT_REQ , WdsSetAutoConnect, (void *)&autoconnect_setting);
    QmiThreadSendQMI(pRequest, &pResponse);
    if (pResponse) free(pResponse);

    return 0;
}

#ifdef CONFIG_SIM
static int requestGetPINStatus(SIM_Status *pSIMStatus) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    PQMIDMS_UIM_PIN_STATUS pPin1Status = NULL;
    //PQMIDMS_UIM_PIN_STATUS pPin2Status = NULL;

    if (s_9x07)
        pRequest = ComposeQMUXMsg(QMUX_TYPE_UIM, QMIUIM_GET_CARD_STATUS_REQ, NULL, NULL);
    else
        pRequest = ComposeQMUXMsg(QMUX_TYPE_DMS, QMIDMS_UIM_GET_PIN_STATUS_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    pPin1Status = (PQMIDMS_UIM_PIN_STATUS)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x11);
    //pPin2Status = (PQMIDMS_UIM_PIN_STATUS)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x12);

    if (pPin1Status != NULL) {
        if (pPin1Status->PINStatus == QMI_PIN_STATUS_NOT_VERIF) {
            *pSIMStatus = SIM_PIN;
        } else if (pPin1Status->PINStatus == QMI_PIN_STATUS_BLOCKED) {
            *pSIMStatus = SIM_PUK;
        } else if (pPin1Status->PINStatus == QMI_PIN_STATUS_PERM_BLOCKED) {
            *pSIMStatus = SIM_BAD;
        }
    }

    free(pResponse);
    return 0;
}

static int requestGetSIMStatus(SIM_Status *pSIMStatus) { //RIL_REQUEST_GET_SIM_STATUS
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    const char * SIM_Status_String[] = {
        "SIM_ABSENT",
        "SIM_NOT_READY",
        "SIM_READY", /* SIM_READY means the radio state is RADIO_STATE_SIM_READY */
        "SIM_PIN",
        "SIM_PUK",
        "SIM_NETWORK_PERSONALIZATION"
    };

    if (s_9x07)
        pRequest = ComposeQMUXMsg(QMUX_TYPE_UIM, QMIUIM_GET_CARD_STATUS_REQ, NULL, NULL);
    else
        pRequest = ComposeQMUXMsg(QMUX_TYPE_DMS, QMIDMS_UIM_GET_STATE_REQ, NULL, NULL);

    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    *pSIMStatus = SIM_ABSENT;
    if (s_9x07)
    {
        PQMIUIM_CARD_STATUS pCardStatus = NULL;
        PQMIUIM_PIN_STATE pPINState = NULL;
        UCHAR CardState = 0x01;
        UCHAR PIN1State = QMI_PIN_STATUS_NOT_VERIF;
        //UCHAR PIN1Retries;
        //UCHAR PUK1Retries;
        //UCHAR PIN2State;
        //UCHAR PIN2Retries;
        //UCHAR PUK2Retries;

        pCardStatus = (PQMIUIM_CARD_STATUS)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x10);
        if (pCardStatus != NULL)
        {
            pPINState = (PQMIUIM_PIN_STATE)((PUCHAR)pCardStatus + sizeof(QMIUIM_CARD_STATUS) + pCardStatus->AIDLength);
            CardState  = pCardStatus->CardState;
            if (CardState == UIM_CARD_STATE_PRESENT) {
                if (pPINState->UnivPIN == 1)
                {
                   PIN1State = pCardStatus->UPINState;
                   //PIN1Retries = pCardStatus->UPINRetries;
                   //PUK1Retries = pCardStatus->UPUKRetries;
                }
                else
                {
                   PIN1State = pPINState->PIN1State;
                   //PIN1Retries = pPINState->PIN1Retries;
                   //PUK1Retries = pPINState->PUK1Retries;
                }
                //PIN2State = pPINState->PIN2State;
                //PIN2Retries = pPINState->PIN2Retries;
                //PUK2Retries = pPINState->PUK2Retries;
            }
        }

        *pSIMStatus = SIM_ABSENT;
        if ((CardState == 0x01) &&  ((PIN1State == QMI_PIN_STATUS_VERIFIED)|| (PIN1State == QMI_PIN_STATUS_DISABLED)))
        {
            *pSIMStatus = SIM_READY;
        }
        else if (CardState == 0x01)
        {
            if (PIN1State == QMI_PIN_STATUS_NOT_VERIF)
            {
                *pSIMStatus = SIM_PIN;
            }
            if ( PIN1State == QMI_PIN_STATUS_BLOCKED)
            {
                *pSIMStatus = SIM_PUK;
            }
            else if (PIN1State == QMI_PIN_STATUS_PERM_BLOCKED)
            {
                *pSIMStatus = SIM_BAD;
            }
            else if (PIN1State == QMI_PIN_STATUS_NOT_INIT || PIN1State == QMI_PIN_STATUS_VERIFIED || PIN1State == QMI_PIN_STATUS_DISABLED)
            {
                *pSIMStatus = SIM_READY;
            }
        }
        else if (CardState == 0x00 || CardState == 0x02)
        {
        }
        else
        {
        }
    }
    else
    {
    //UIM state. Values:
    // 0x00  UIM initialization completed
    // 0x01  UIM is locked or the UIM failed
    // 0x02  UIM is not present
    // 0x03  Reserved
    // 0xFF  UIM state is currently
    //unavailable
        if (pResponse->MUXMsg.UIMGetStateResp.UIMState == 0x00) {
            *pSIMStatus = SIM_READY;
        } else if (pResponse->MUXMsg.UIMGetStateResp.UIMState == 0x01) {
            *pSIMStatus = SIM_ABSENT;
            err = requestGetPINStatus(pSIMStatus);
        } else if ((pResponse->MUXMsg.UIMGetStateResp.UIMState == 0x02) || (pResponse->MUXMsg.UIMGetStateResp.UIMState == 0xFF)) {
            *pSIMStatus = SIM_ABSENT;
        } else {
            *pSIMStatus = SIM_ABSENT;
        }
    }
    dbg_time("%s SIMStatus: %s", __func__, SIM_Status_String[*pSIMStatus]);

    free(pResponse);

    return 0;
}

static int requestEnterSimPin(const char *pPinCode) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;

    if (s_9x07)
        pRequest = ComposeQMUXMsg(QMUX_TYPE_UIM, QMIUIM_VERIFY_PIN_REQ, UimVerifyPinReqSend, (void *)pPinCode);
    else
        pRequest = ComposeQMUXMsg(QMUX_TYPE_DMS, QMIDMS_UIM_VERIFY_PIN_REQ, DmsUIMVerifyPinReqSend, (void *)pPinCode);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    free(pResponse);
    return 0;
}
#endif

#ifdef CONFIG_IMSI_ICCID
static int requestGetICCID(void) { //RIL_REQUEST_GET_IMSI
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    PQMIUIM_CONTENT pUimContent;
    int err;

    if (s_9x07) {
        pRequest = ComposeQMUXMsg(QMUX_TYPE_UIM, QMIUIM_READ_TRANSPARENT_REQ, UimReadTransparentIMSIReqSend, (void *)"EF_ICCID");
        err = QmiThreadSendQMI(pRequest, &pResponse);
    } else {
        return 0;
    }
    qmi_rsp_check_and_return();

    pUimContent = (PQMIUIM_CONTENT)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x11);
    if (pUimContent != NULL) {
        static char DeviceICCID[32] = {'\0'};
        int i = 0, j = 0;

        for (i = 0, j = 0; i < le16_to_cpu(pUimContent->content_len); ++i) {
            char charmaps[] = "0123456789ABCDEF";

            DeviceICCID[j++] = charmaps[(pUimContent->content[i] & 0x0F)];
            DeviceICCID[j++] = charmaps[((pUimContent->content[i] & 0xF0) >> 0x04)];
        }
        DeviceICCID[j] = '\0';

        dbg_time("%s DeviceICCID: %s", __func__, DeviceICCID);
    }

    free(pResponse);
    return 0;
}

static int requestGetIMSI(void) { //RIL_REQUEST_GET_IMSI
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    PQMIUIM_CONTENT pUimContent;
    int err;

    if (s_9x07) {
        pRequest = ComposeQMUXMsg(QMUX_TYPE_UIM, QMIUIM_READ_TRANSPARENT_REQ, UimReadTransparentIMSIReqSend, (void *)"EF_IMSI");
        err = QmiThreadSendQMI(pRequest, &pResponse);
    } else {
        return 0;
    }
    qmi_rsp_check_and_return();

    pUimContent = (PQMIUIM_CONTENT)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x11);
    if (pUimContent != NULL) {
        static char DeviceIMSI[32] = {'\0'};
        int i = 0, j = 0;

        for (i = 0, j = 0; i < le16_to_cpu(pUimContent->content[0]); ++i) {
            if (i != 0)
                DeviceIMSI[j++] = (pUimContent->content[i+1] & 0x0F) + '0';
            DeviceIMSI[j++] = ((pUimContent->content[i+1] & 0xF0) >> 0x04) + '0';
        }
        DeviceIMSI[j] = '\0';

        dbg_time("%s DeviceIMSI: %s", __func__, DeviceIMSI);
    }

    free(pResponse);
    return 0;
}
#endif

#if 1
static void quectel_convert_cdma_mcc_2_ascii_mcc( USHORT *p_mcc, USHORT mcc )
{
  unsigned int d1, d2, d3, buf = mcc + 111;

  if ( mcc == 0x3FF ) // wildcard
  {
    *p_mcc = 3;
  }
  else
  {
    d3 = buf % 10;
    buf = ( d3 == 0 ) ? (buf-10)/10 : buf/10;

    d2 = buf % 10;
    buf = ( d2 == 0 ) ? (buf-10)/10 : buf/10;

    d1 = ( buf == 10 ) ? 0 : buf;

//dbg_time("d1:%d, d2:%d,d3:%d",d1,d2,d3);
    if ( d1<10 && d2<10 && d3<10 )
    {
    *p_mcc = d1*100+d2*10+d3;
#if 0
      *(p_mcc+0) = '0' + d1;
      *(p_mcc+1) = '0' + d2;
      *(p_mcc+2) = '0' + d3;
#endif
    }
    else
    {
      //dbg_time( "invalid digits %d %d %d", d1, d2, d3 );
      *p_mcc = 0;
    }
  }
}

static void quectel_convert_cdma_mnc_2_ascii_mnc( USHORT *p_mnc, USHORT imsi_11_12)
{
  unsigned int d1, d2, buf = imsi_11_12 + 11;

  if ( imsi_11_12 == 0x7F ) // wildcard
  {
    *p_mnc = 7;
  }
  else
  {
    d2 = buf % 10;
    buf = ( d2 == 0 ) ? (buf-10)/10 : buf/10;

    d1 = ( buf == 10 ) ? 0 : buf;

    if ( d1<10 && d2<10 )
    {
     *p_mnc = d1*10 + d2;
    }
    else
    {
      //dbg_time( "invalid digits %d %d", d1, d2, 0 );
      *p_mnc = 0;
    }
  }
}

static int requestGetHomeNetwork(USHORT *p_mcc, USHORT *p_mnc, USHORT *p_sid, USHORT *p_nid) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    PHOME_NETWORK pHomeNetwork;
    PHOME_NETWORK_SYSTEMID pHomeNetworkSystemID;

    pRequest = ComposeQMUXMsg(QMUX_TYPE_NAS, QMINAS_GET_HOME_NETWORK_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    pHomeNetwork = (PHOME_NETWORK)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x01);
    if (pHomeNetwork && p_mcc && p_mnc ) {
        *p_mcc = le16_to_cpu(pHomeNetwork->MobileCountryCode);
        *p_mnc = le16_to_cpu(pHomeNetwork->MobileNetworkCode);
        //dbg_time("%s MobileCountryCode: %d, MobileNetworkCode: %d", __func__, *pMobileCountryCode, *pMobileNetworkCode);
    }

    pHomeNetworkSystemID = (PHOME_NETWORK_SYSTEMID)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x10);
    if (pHomeNetworkSystemID && p_sid && p_nid) {
        *p_sid = le16_to_cpu(pHomeNetworkSystemID->SystemID); //china-hefei: sid 14451
        *p_nid = le16_to_cpu(pHomeNetworkSystemID->NetworkID);
        //dbg_time("%s SystemID: %d, NetworkID: %d", __func__, *pSystemID, *pNetworkID);
    }

    free(pResponse);

    return 0;
}
#endif

#if 0
// Lookup table for carriers known to produce SIMs which incorrectly indicate MNC length.
static const char * MCCMNC_CODES_HAVING_3DIGITS_MNC[] = {
    "302370", "302720", "310260",
    "405025", "405026", "405027", "405028", "405029", "405030", "405031", "405032",
    "405033", "405034", "405035", "405036", "405037", "405038", "405039", "405040",
    "405041", "405042", "405043", "405044", "405045", "405046", "405047", "405750",
    "405751", "405752", "405753", "405754", "405755", "405756", "405799", "405800",
    "405801", "405802", "405803", "405804", "405805", "405806", "405807", "405808",
    "405809", "405810", "405811", "405812", "405813", "405814", "405815", "405816",
    "405817", "405818", "405819", "405820", "405821", "405822", "405823", "405824",
    "405825", "405826", "405827", "405828", "405829", "405830", "405831", "405832",
    "405833", "405834", "405835", "405836", "405837", "405838", "405839", "405840",
    "405841", "405842", "405843", "405844", "405845", "405846", "405847", "405848",
    "405849", "405850", "405851", "405852", "405853", "405875", "405876", "405877",
    "405878", "405879", "405880", "405881", "405882", "405883", "405884", "405885",
    "405886", "405908", "405909", "405910", "405911", "405912", "405913", "405914",
    "405915", "405916", "405917", "405918", "405919", "405920", "405921", "405922",
    "405923", "405924", "405925", "405926", "405927", "405928", "405929", "405930",
    "405931", "405932", "502142", "502143", "502145", "502146", "502147", "502148"
};

static const char * MCC_CODES_HAVING_3DIGITS_MNC[] = {
    "302",    //Canada
    "310",    //United States of America
    "311",    //United States of America
    "312",    //United States of America
    "313",    //United States of America
    "314",    //United States of America
    "315",    //United States of America
    "316",    //United States of America
    "334",    //Mexico
    "338",    //Jamaica
    "342", //Barbados
    "344",    //Antigua and Barbuda
    "346",    //Cayman Islands
    "348",    //British Virgin Islands
    "365",    //Anguilla
    "708",    //Honduras (Republic of)
    "722",    //Argentine Republic
    "732"    //Colombia (Republic of)
};

int requestGetIMSI(const char **pp_imsi, USHORT *pMobileCountryCode, USHORT *pMobileNetworkCode) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;

    if (pp_imsi) *pp_imsi = NULL;
    if (pMobileCountryCode) *pMobileCountryCode = 0;
    if (pMobileNetworkCode) *pMobileNetworkCode = 0;

    pRequest = ComposeQMUXMsg(QMUX_TYPE_DMS, QMIDMS_UIM_GET_IMSI_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    if (pMUXMsg->UIMGetIMSIResp.TLV2Type == 0x01 &&  le16_to_cpu(pMUXMsg->UIMGetIMSIResp.TLV2Length) >= 5) {
        int mnc_len = 2;
        unsigned i;
        char tmp[4];

        if (pp_imsi) *pp_imsi = strndup((const char *)(&pMUXMsg->UIMGetIMSIResp.IMSI), le16_to_cpu(pMUXMsg->UIMGetIMSIResp.TLV2Length));

        for (i = 0; i < sizeof(MCCMNC_CODES_HAVING_3DIGITS_MNC)/sizeof(MCCMNC_CODES_HAVING_3DIGITS_MNC[0]); i++) {
            if (!strncmp((const char *)(&pMUXMsg->UIMGetIMSIResp.IMSI), MCCMNC_CODES_HAVING_3DIGITS_MNC[i], 6)) {
                mnc_len = 3;
                break;
            }
        }
        if (mnc_len == 2) {
            for (i = 0; i < sizeof(MCC_CODES_HAVING_3DIGITS_MNC)/sizeof(MCC_CODES_HAVING_3DIGITS_MNC[0]); i++) {
                if (!strncmp((const char *)(&pMUXMsg->UIMGetIMSIResp.IMSI), MCC_CODES_HAVING_3DIGITS_MNC[i], 3)) {
                    mnc_len = 3;
                    break;
                }
            }
        }

        tmp[0] = (&pMUXMsg->UIMGetIMSIResp.IMSI)[0];
        tmp[1] = (&pMUXMsg->UIMGetIMSIResp.IMSI)[1];
        tmp[2] = (&pMUXMsg->UIMGetIMSIResp.IMSI)[2];
        tmp[3] = 0;
        if (pMobileCountryCode) *pMobileCountryCode = atoi(tmp);
        tmp[0] = (&pMUXMsg->UIMGetIMSIResp.IMSI)[3];
        tmp[1] = (&pMUXMsg->UIMGetIMSIResp.IMSI)[4];
        tmp[2] = 0;
        if (mnc_len == 3) {
            tmp[2] = (&pMUXMsg->UIMGetIMSIResp.IMSI)[6];
        }
        if (pMobileNetworkCode) *pMobileNetworkCode = atoi(tmp);
    }

    free(pResponse);

    return 0;
}
#endif

static struct wwan_data_class_str class2str[] = {
    {WWAN_DATA_CLASS_NONE, "UNKNOWN"},
    {WWAN_DATA_CLASS_GPRS, "GPRS"},
    {WWAN_DATA_CLASS_EDGE, "EDGE"},
    {WWAN_DATA_CLASS_UMTS, "UMTS"},
    {WWAN_DATA_CLASS_HSDPA, "HSDPA"},
    {WWAN_DATA_CLASS_HSUPA, "HSUPA"},
    {WWAN_DATA_CLASS_LTE, "LTE"},
    {WWAN_DATA_CLASS_5G_NSA, "5G_NSA"},
    {WWAN_DATA_CLASS_5G_SA, "5G_SA"},
    {WWAN_DATA_CLASS_1XRTT, "1XRTT"},
    {WWAN_DATA_CLASS_1XEVDO, "1XEVDO"},
    {WWAN_DATA_CLASS_1XEVDO_REVA, "1XEVDO_REVA"},
    {WWAN_DATA_CLASS_1XEVDV, "1XEVDV"},
    {WWAN_DATA_CLASS_3XRTT, "3XRTT"},
    {WWAN_DATA_CLASS_1XEVDO_REVB, "1XEVDO_REVB"},
    {WWAN_DATA_CLASS_UMB, "UMB"},
    {WWAN_DATA_CLASS_CUSTOM, "CUSTOM"},
};

static const char *wwan_data_class2str(ULONG class)
{
    unsigned int i = 0;
    for (i = 0; i < sizeof(class2str)/sizeof(class2str[0]); i++) {
        if (class2str[i].class == class) {
            return class2str[i].str;
        }
    }
    return "UNKNOWN";
}

static USHORT char2ushort(UCHAR str[3]) {
    int i;
    char temp[4];
    USHORT ret= 0;

    memcpy(temp, str, 3);
    temp[3] = '\0';

    for (i = 0; i < 4; i++) {
        if ((UCHAR)temp[i] == 0xFF) {
            temp[i] = '\0';
        }
    }
    ret = (USHORT)atoi(temp);

    return ret;
}

static int requestRegistrationState2(UCHAR *pPSAttachedState) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    USHORT MobileCountryCode = 0;
    USHORT MobileNetworkCode = 0;
    const char *pDataCapStr = "UNKNOW";
    LONG remainingLen;
    PSERVICE_STATUS_INFO pServiceStatusInfo;
    int is_lte = 0;
    PCDMA_SYSTEM_INFO pCdmaSystemInfo;
    PHDR_SYSTEM_INFO pHdrSystemInfo;
    PGSM_SYSTEM_INFO pGsmSystemInfo;
    PWCDMA_SYSTEM_INFO pWcdmaSystemInfo;
    PLTE_SYSTEM_INFO pLteSystemInfo;
    PTDSCDMA_SYSTEM_INFO pTdscdmaSystemInfo;
    PNR5G_SYSTEM_INFO pNr5gSystemInfo;
    UCHAR DeviceClass = 0;
    ULONG DataCapList = 0;
    
    /*  Additional LTE System Info - Availability of Dual connectivity of E-UTRA with NR5G */
    uint8_t endc_available_valid = 0;  /**< Must be set to true if endc_available is being passed */
    uint8_t endc_available = 0x00;
    /**<  
       Upper layer indication in LTE SIB2. Values: \n
       - 0x00 -- 5G Not available \n
       - 0x01 -- 5G Available
       
    */
    /*  Additional LTE System Info - DCNR restriction Info */
    uint8_t restrict_dcnr_valid = 0;  /**< Must be set to true if restrict_dcnr is being passed */
    uint8_t restrict_dcnr = 0x01;
    /**<  
       DCNR restriction in NAS attach/TAU accept. Values: \n
       - 0x00 -- Not restricted \n
       - 0x01 -- Restricted
    */

    *pPSAttachedState = 0;
    pRequest = ComposeQMUXMsg(QMUX_TYPE_NAS, QMINAS_GET_SYS_INFO_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    pServiceStatusInfo = (PSERVICE_STATUS_INFO)(((PCHAR)&pMUXMsg->GetSysInfoResp) + QCQMUX_MSG_HDR_SIZE);
    remainingLen = le16_to_cpu(pMUXMsg->GetSysInfoResp.Length);

    s_is_cdma = 0;
    s_5g_type = WWAN_DATA_CLASS_NONE;
    s_hdr_personality = 0;
    while (remainingLen > 0) {
        switch (pServiceStatusInfo->TLVType) {
        case 0x10: // CDMA
            if (pServiceStatusInfo->SrvStatus == 0x02) {
                DataCapList = WWAN_DATA_CLASS_1XRTT|
                              WWAN_DATA_CLASS_1XEVDO|
                              WWAN_DATA_CLASS_1XEVDO_REVA|
                              WWAN_DATA_CLASS_1XEVDV|
                              WWAN_DATA_CLASS_1XEVDO_REVB;
                DeviceClass = DEVICE_CLASS_CDMA;
                s_is_cdma = (0 == is_lte);
            }
            break;
        case 0x11: // HDR
            if (pServiceStatusInfo->SrvStatus == 0x02) {
                DataCapList = WWAN_DATA_CLASS_3XRTT|
                              WWAN_DATA_CLASS_UMB;
                DeviceClass = DEVICE_CLASS_CDMA;
                s_is_cdma = (0 == is_lte);
            }
            break;
        case 0x12: // GSM
            if (pServiceStatusInfo->SrvStatus == 0x02) {
                DataCapList = WWAN_DATA_CLASS_GPRS|
                              WWAN_DATA_CLASS_EDGE;
                DeviceClass = DEVICE_CLASS_GSM;
            }
            break;
        case 0x13: // WCDMA
            if (pServiceStatusInfo->SrvStatus == 0x02) {
                DataCapList = WWAN_DATA_CLASS_UMTS;
                DeviceClass = DEVICE_CLASS_GSM;
            }
            break;
        case 0x14: // LTE
            if (pServiceStatusInfo->SrvStatus == 0x02) {
                DataCapList = WWAN_DATA_CLASS_LTE;
                DeviceClass = DEVICE_CLASS_GSM;
                is_lte = 1;
                s_is_cdma = 0;
            }
            break;
        case 0x4A: // NR5G Service Status Info
            if (pServiceStatusInfo->SrvStatus == NAS_SYS_SRV_STATUS_SRV_V01) {
                DataCapList |= WWAN_DATA_CLASS_5G_SA;
                DeviceClass = DEVICE_CLASS_GSM;
                is_lte = 1;
                s_is_cdma = 0;
            }
            break;
        case 0x4B: // NR5G System Info
            pNr5gSystemInfo = (PNR5G_SYSTEM_INFO)pServiceStatusInfo;
            if (pNr5gSystemInfo->srv_domain_valid == 0x01) {
                if (pNr5gSystemInfo->srv_domain & SYS_SRV_DOMAIN_PS_ONLY_V01) {
                    *pPSAttachedState = 1;
                }
            }

            if (pNr5gSystemInfo->network_id_valid == 0x01) {
                MobileCountryCode = (USHORT)char2ushort(pNr5gSystemInfo->MCC);
                MobileNetworkCode = (USHORT)char2ushort(pNr5gSystemInfo->MNC);
            }
            break;
        case 0x4E: //Additional LTE System Info - Availability of Dual Connectivity of E-UTRA with NR5G
            endc_available_valid = 1;
            endc_available = pServiceStatusInfo->SrvStatus;
            break;
            
        case 0x4F: //Additional LTE System Info - DCNR restriction Info
            restrict_dcnr_valid = 1;
            restrict_dcnr = pServiceStatusInfo->SrvStatus;
            break;

        case 0x24: // TDSCDMA
            if (pServiceStatusInfo->SrvStatus == 0x02) {
                pDataCapStr = "TD-SCDMA";
            }
            break;
        case 0x15: // CDMA
            // CDMA_SYSTEM_INFO
            pCdmaSystemInfo = (PCDMA_SYSTEM_INFO)pServiceStatusInfo;
            if (pCdmaSystemInfo->SrvDomainValid == 0x01) {
                if (pCdmaSystemInfo->SrvDomain & 0x02) {
                    *pPSAttachedState = 1;
                    s_is_cdma = (0 == is_lte);
                }
            }
#if 0
            if (pCdmaSystemInfo->SrvCapabilityValid == 0x01) {
                *pPSAttachedState = 0;
                if (pCdmaSystemInfo->SrvCapability & 0x02) {
                    *pPSAttachedState = 1;
                    s_is_cdma = (0 == is_lte);
                }
            }
#endif
            if (pCdmaSystemInfo->NetworkIdValid == 0x01) {
                MobileCountryCode = (USHORT)char2ushort(pCdmaSystemInfo->MCC);
                MobileNetworkCode = (USHORT)char2ushort(pCdmaSystemInfo->MNC);
            }
            break;
        case 0x16: // HDR
            // HDR_SYSTEM_INFO
            pHdrSystemInfo = (PHDR_SYSTEM_INFO)pServiceStatusInfo;
            if (pHdrSystemInfo->SrvDomainValid == 0x01) {
                if (pHdrSystemInfo->SrvDomain & 0x02) {
                    *pPSAttachedState = 1;
                    s_is_cdma = (0 == is_lte);
                }
            }
#if 0
            if (pHdrSystemInfo->SrvCapabilityValid == 0x01) {
                *pPSAttachedState = 0;
                if (pHdrSystemInfo->SrvCapability & 0x02) {
                    *pPSAttachedState = 1;
                    s_is_cdma = (0 == is_lte);
                }
            }
#endif
            if (*pPSAttachedState && pHdrSystemInfo->HdrPersonalityValid == 0x01) {
                if (pHdrSystemInfo->HdrPersonality == 0x03)
                    s_hdr_personality = 0x02;
                //else if (pHdrSystemInfo->HdrPersonality == 0x02)
                //    s_hdr_personality = 0x01;
            }
            USHORT cmda_mcc = 0, cdma_mnc = 0;
            if(!requestGetHomeNetwork(&cmda_mcc, &cdma_mnc,NULL, NULL) && cmda_mcc) {
                quectel_convert_cdma_mcc_2_ascii_mcc(&MobileCountryCode, cmda_mcc);
                quectel_convert_cdma_mnc_2_ascii_mnc(&MobileNetworkCode, cdma_mnc);
            }
            break;
        case 0x17: // GSM
            // GSM_SYSTEM_INFO
            pGsmSystemInfo = (PGSM_SYSTEM_INFO)pServiceStatusInfo;
            if (pGsmSystemInfo->SrvDomainValid == 0x01) {
                if (pGsmSystemInfo->SrvDomain & 0x02) {
                    *pPSAttachedState = 1;
                }
            }
#if 0
            if (pGsmSystemInfo->SrvCapabilityValid == 0x01) {
                *pPSAttachedState = 0;
                if (pGsmSystemInfo->SrvCapability & 0x02) {
                    *pPSAttachedState = 1;
                }
            }
#endif
            if (pGsmSystemInfo->NetworkIdValid == 0x01) {
                MobileCountryCode = (USHORT)char2ushort(pGsmSystemInfo->MCC);
                MobileNetworkCode = (USHORT)char2ushort(pGsmSystemInfo->MNC);
            }
            break;
        case 0x18: // WCDMA
            // WCDMA_SYSTEM_INFO
            pWcdmaSystemInfo = (PWCDMA_SYSTEM_INFO)pServiceStatusInfo;
            if (pWcdmaSystemInfo->SrvDomainValid == 0x01) {
                if (pWcdmaSystemInfo->SrvDomain & 0x02) {
                    *pPSAttachedState = 1;
                }
            }
#if 0
            if (pWcdmaSystemInfo->SrvCapabilityValid == 0x01) {
                *pPSAttachedState = 0;
                if (pWcdmaSystemInfo->SrvCapability & 0x02) {
                    *pPSAttachedState = 1;
                }
            }
#endif
            if (pWcdmaSystemInfo->NetworkIdValid == 0x01) {
                MobileCountryCode = (USHORT)char2ushort(pWcdmaSystemInfo->MCC);
                MobileNetworkCode = (USHORT)char2ushort(pWcdmaSystemInfo->MNC);
            }
            break;
        case 0x19: // LTE_SYSTEM_INFO
            // LTE_SYSTEM_INFO
            pLteSystemInfo = (PLTE_SYSTEM_INFO)pServiceStatusInfo;
            if (pLteSystemInfo->SrvDomainValid == 0x01) {
                if (pLteSystemInfo->SrvDomain & 0x02) {
                    *pPSAttachedState = 1;
                    is_lte = 1;
                    s_is_cdma = 0;
                }
            }
#if 0
            if (pLteSystemInfo->SrvCapabilityValid == 0x01) {
                *pPSAttachedState = 0;
                if (pLteSystemInfo->SrvCapability & 0x02) {
                    *pPSAttachedState = 1;
                    is_lte = 1;
                    s_is_cdma = 0;
                }
            }
#endif
            if (pLteSystemInfo->NetworkIdValid == 0x01) {
                MobileCountryCode = (USHORT)char2ushort(pLteSystemInfo->MCC);
                MobileNetworkCode = (USHORT)char2ushort(pLteSystemInfo->MNC);
            }
            break;
        case 0x25: // TDSCDMA
            // TDSCDMA_SYSTEM_INFO
            pTdscdmaSystemInfo = (PTDSCDMA_SYSTEM_INFO)pServiceStatusInfo;
            if (pTdscdmaSystemInfo->SrvDomainValid == 0x01) {
                if (pTdscdmaSystemInfo->SrvDomain & 0x02) {
                    *pPSAttachedState = 1;
                }
            }
#if 0
            if (pTdscdmaSystemInfo->SrvCapabilityValid == 0x01) {
                *pPSAttachedState = 0;
                if (pTdscdmaSystemInfo->SrvCapability & 0x02) {
                    *pPSAttachedState = 1;
                }
            }
#endif
            if (pTdscdmaSystemInfo->NetworkIdValid == 0x01) {
                MobileCountryCode = (USHORT)char2ushort(pTdscdmaSystemInfo->MCC);
                MobileNetworkCode = (USHORT)char2ushort(pTdscdmaSystemInfo->MNC);
            }
            break;
        default:
            break;
        } /* switch (pServiceStatusInfo->TLYType) */

        remainingLen -= (le16_to_cpu(pServiceStatusInfo->TLVLength) + 3);
        pServiceStatusInfo = (PSERVICE_STATUS_INFO)((PCHAR)&pServiceStatusInfo->TLVLength + le16_to_cpu(pServiceStatusInfo->TLVLength) + sizeof(USHORT));
    } /* while (remainingLen > 0) */

    if (DataCapList & WWAN_DATA_CLASS_LTE) {
        if (endc_available_valid && restrict_dcnr_valid) {
            if (endc_available && !restrict_dcnr) {
                DataCapList |= WWAN_DATA_CLASS_5G_NSA;
            }
        }
    }

    if (DeviceClass == DEVICE_CLASS_CDMA) {
        if (s_hdr_personality == 2) {
            pDataCapStr = s_hdr_personality == 2 ? "eHRPD" : "HRPD";
        } else if (DataCapList & WWAN_DATA_CLASS_1XEVDO_REVB) {
            pDataCapStr = wwan_data_class2str(WWAN_DATA_CLASS_1XEVDO_REVB);
        } else if (DataCapList & WWAN_DATA_CLASS_1XEVDO_REVA) {
            pDataCapStr = wwan_data_class2str(WWAN_DATA_CLASS_1XEVDO_REVA);
        } else if (DataCapList & WWAN_DATA_CLASS_1XEVDO) {
            pDataCapStr = wwan_data_class2str(WWAN_DATA_CLASS_1XEVDO);
        } else if (DataCapList & WWAN_DATA_CLASS_1XRTT) {
            pDataCapStr = wwan_data_class2str(WWAN_DATA_CLASS_1XRTT);
        } else if (DataCapList & WWAN_DATA_CLASS_3XRTT) {
            pDataCapStr = wwan_data_class2str(WWAN_DATA_CLASS_3XRTT);
        } else if (DataCapList & WWAN_DATA_CLASS_UMB) {
            pDataCapStr = wwan_data_class2str(WWAN_DATA_CLASS_UMB);
        }
    } else {
        if (DataCapList & WWAN_DATA_CLASS_5G_SA) {
            s_5g_type = WWAN_DATA_CLASS_5G_SA;
            pDataCapStr = wwan_data_class2str(WWAN_DATA_CLASS_5G_SA);
        } else if (DataCapList & WWAN_DATA_CLASS_5G_NSA) {
            s_5g_type = WWAN_DATA_CLASS_5G_NSA;
            pDataCapStr = wwan_data_class2str(WWAN_DATA_CLASS_5G_NSA);
        } else if (DataCapList & WWAN_DATA_CLASS_LTE) {
            pDataCapStr = wwan_data_class2str(WWAN_DATA_CLASS_LTE);
        } else if ((DataCapList & WWAN_DATA_CLASS_HSDPA) && (DataCapList & WWAN_DATA_CLASS_HSUPA)) {
            pDataCapStr = "HSDPA_HSUPA";
        } else if (DataCapList & WWAN_DATA_CLASS_HSDPA) {
            pDataCapStr = wwan_data_class2str(WWAN_DATA_CLASS_HSDPA);
        } else if (DataCapList & WWAN_DATA_CLASS_HSUPA) {
            pDataCapStr = wwan_data_class2str(WWAN_DATA_CLASS_HSUPA);
        } else if (DataCapList & WWAN_DATA_CLASS_UMTS) {
            pDataCapStr = wwan_data_class2str(WWAN_DATA_CLASS_UMTS);
        } else if (DataCapList & WWAN_DATA_CLASS_EDGE) {
            pDataCapStr = wwan_data_class2str(WWAN_DATA_CLASS_EDGE);
        } else if (DataCapList & WWAN_DATA_CLASS_GPRS) {
            pDataCapStr = wwan_data_class2str(WWAN_DATA_CLASS_GPRS);
        }
    }

    dbg_time("%s MCC: %d, MNC: %d, PS: %s, DataCap: %s", __func__,
        MobileCountryCode, MobileNetworkCode, (*pPSAttachedState == 1) ? "Attached" : "Detached" , pDataCapStr);

    free(pResponse);

    return 0;
}

static int requestRegistrationState(UCHAR *pPSAttachedState) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    PQMINAS_CURRENT_PLMN_MSG pCurrentPlmn;
    PSERVING_SYSTEM pServingSystem;
    PQMINAS_DATA_CAP pDataCap;
    USHORT MobileCountryCode = 0;
    USHORT MobileNetworkCode = 0;
    const char *pDataCapStr = "UNKNOW";

    if (s_9x07) {
        return requestRegistrationState2(pPSAttachedState);
    }

    pRequest = ComposeQMUXMsg(QMUX_TYPE_NAS, QMINAS_GET_SERVING_SYSTEM_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    pCurrentPlmn = (PQMINAS_CURRENT_PLMN_MSG)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x12);
    if (pCurrentPlmn) {
        MobileCountryCode = le16_to_cpu(pCurrentPlmn->MobileCountryCode);
        MobileNetworkCode = le16_to_cpu(pCurrentPlmn->MobileNetworkCode);
    }

    *pPSAttachedState = 0;
    pServingSystem = (PSERVING_SYSTEM)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x01);
    if (pServingSystem) {
    //Packet-switched domain attach state of the mobile.
    //0x00    PS_UNKNOWN ?Unknown or not applicable
    //0x01    PS_ATTACHED ?Attached
    //0x02    PS_DETACHED ?Detached
        *pPSAttachedState = pServingSystem->RegistrationState;
        if (pServingSystem->RegistrationState == 0x01) //0x01 ?C REGISTERED ?C Registered with a network
            *pPSAttachedState  = pServingSystem->PSAttachedState;
        else {
            //MobileCountryCode = MobileNetworkCode = 0;
            *pPSAttachedState  = 0x02;
        }
    }

    pDataCap = (PQMINAS_DATA_CAP)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x11);
    if (pDataCap && pDataCap->DataCapListLen) {
        UCHAR *DataCap = &pDataCap->DataCap;
        if (pDataCap->DataCapListLen == 2) {
            if ((DataCap[0] == 0x06) && ((DataCap[1] == 0x08) || (DataCap[1] == 0x0A)))
                DataCap[0] = DataCap[1];
        }
        switch (DataCap[0]) {
             case 0x01: pDataCapStr = "GPRS"; break;
             case 0x02: pDataCapStr = "EDGE"; break;
             case 0x03: pDataCapStr = "HSDPA"; break;
             case 0x04: pDataCapStr = "HSUPA"; break;
             case 0x05: pDataCapStr = "UMTS"; break;
             case 0x06: pDataCapStr = "1XRTT"; break;
             case 0x07: pDataCapStr = "1XEVDO"; break;
             case 0x08: pDataCapStr = "1XEVDO_REVA"; break;
             case 0x09: pDataCapStr = "GPRS"; break;
             case 0x0A: pDataCapStr = "1XEVDO_REVB"; break;
             case 0x0B: pDataCapStr = "LTE"; break;
             case 0x0C: pDataCapStr = "HSDPA"; break;
             case 0x0D: pDataCapStr = "HSDPA"; break;
             default: pDataCapStr = "UNKNOW"; break;
        }
    }

    if (pServingSystem && pServingSystem->RegistrationState == 0x01 && pServingSystem->InUseRadioIF && pServingSystem->RadioIF == 0x09) {
        pDataCapStr = "TD-SCDMA";
    }

    s_is_cdma = 0;
    if (pServingSystem && pServingSystem->RegistrationState == 0x01 && pServingSystem->InUseRadioIF && (pServingSystem->RadioIF == 0x01 || pServingSystem->RadioIF == 0x02)) {
        USHORT cmda_mcc = 0, cdma_mnc = 0;
        s_is_cdma = 1;
        if(!requestGetHomeNetwork(&cmda_mcc, &cdma_mnc,NULL, NULL) && cmda_mcc) {
            quectel_convert_cdma_mcc_2_ascii_mcc(&MobileCountryCode, cmda_mcc);
            quectel_convert_cdma_mnc_2_ascii_mnc(&MobileNetworkCode, cdma_mnc);
        }
        if (1) {
            PQCQMUX_TLV pTLV = (PQCQMUX_TLV)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x23);
            if (pTLV)
                s_hdr_personality = pTLV->Value;
            else
                s_hdr_personality = 0;
            if (s_hdr_personality == 2)
                pDataCapStr = "eHRPD";
        }
    }

    dbg_time("%s MCC: %d, MNC: %d, PS: %s, DataCap: %s", __func__,
        MobileCountryCode, MobileNetworkCode, (*pPSAttachedState == 1) ? "Attached" : "Detached" , pDataCapStr);

    free(pResponse);

    return 0;
}

static int requestQueryDataCall(UCHAR  *pConnectionStatus, int curIpFamily) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    PQMIWDS_PKT_SRVC_TLV pPktSrvc;
    UCHAR oldConnectionStatus = *pConnectionStatus;
    UCHAR QMIType = (curIpFamily == IpFamilyV4) ? QMUX_TYPE_WDS : QMUX_TYPE_WDS_IPV6;

    pRequest = ComposeQMUXMsg(QMIType, QMIWDS_GET_PKT_SRVC_STATUS_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    *pConnectionStatus = QWDS_PKT_DATA_DISCONNECTED;
    pPktSrvc = (PQMIWDS_PKT_SRVC_TLV)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x01);
    if (pPktSrvc) {
        *pConnectionStatus = pPktSrvc->ConnectionStatus;
        if ((le16_to_cpu(pPktSrvc->TLVLength) == 2) && (pPktSrvc->ReconfigReqd == 0x01))
            *pConnectionStatus = QWDS_PKT_DATA_DISCONNECTED;
    }

    if (*pConnectionStatus == QWDS_PKT_DATA_DISCONNECTED) {
        if (curIpFamily == IpFamilyV4)
            WdsConnectionIPv4Handle = 0;
        else
            WdsConnectionIPv6Handle = 0;
    }

    if (oldConnectionStatus != *pConnectionStatus || debug_qmi) {
        dbg_time("%s %sConnectionStatus: %s", __func__, (curIpFamily == IpFamilyV4) ? "IPv4" : "IPv6",
            (*pConnectionStatus == QWDS_PKT_DATA_CONNECTED) ? "CONNECTED" : "DISCONNECTED");
    }

    free(pResponse);
    return 0;
}

static int requestSetupDataCall(PROFILE_T *profile, int curIpFamily) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err = 0;
    UCHAR QMIType = (curIpFamily == IpFamilyV4) ? QMUX_TYPE_WDS : QMUX_TYPE_WDS_IPV6;

//DualIPSupported means can get ipv4 & ipv6 address at the same time, one wds for ipv4, the other wds for ipv6
    profile->curIpFamily = curIpFamily;
    pRequest = ComposeQMUXMsg(QMIType, QMIWDS_START_NETWORK_INTERFACE_REQ, WdsStartNwInterfaceReq, profile);
    err = QmiThreadSendQMITimeout(pRequest, &pResponse, 120 * 1000, __func__);
    qmi_rsp_check();

    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        PQMI_TLV_HDR pTLVHdr;

        pTLVHdr = GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x10);
        if (pTLVHdr) {
            uint16_t *data16 = (uint16_t *)(pTLVHdr+1);
            uint16_t call_end_reason = le16_to_cpu(data16[0]);
            dbg_time("call_end_reason is %d", call_end_reason);
        }

        pTLVHdr = GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x11);
        if (pTLVHdr) {
            uint16_t *data16 = (uint16_t *)(pTLVHdr+1);
            uint16_t call_end_reason_type = le16_to_cpu(data16[0]);
            uint16_t verbose_call_end_reason  = le16_to_cpu(data16[1]);

            dbg_time("call_end_reason_type is %d", call_end_reason_type);
            dbg_time("call_end_reason_verbose is %d", verbose_call_end_reason);
        }

        err = le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);
        free(pResponse);
        return err;
    }

    if (curIpFamily == IpFamilyV4) {
        WdsConnectionIPv4Handle = le32_to_cpu(pResponse->MUXMsg.StartNwInterfaceResp.Handle);
        dbg_time("%s WdsConnectionIPv4Handle: 0x%08x", __func__, WdsConnectionIPv4Handle);
    } else {
        WdsConnectionIPv6Handle = le32_to_cpu(pResponse->MUXMsg.StartNwInterfaceResp.Handle);
        dbg_time("%s WdsConnectionIPv6Handle: 0x%08x", __func__, WdsConnectionIPv6Handle);
    }

    free(pResponse);

    return 0;
}

static int requestDeactivateDefaultPDP(PROFILE_T *profile, int curIpFamily) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    UCHAR QMIType = (curIpFamily == 0x04) ? QMUX_TYPE_WDS : QMUX_TYPE_WDS_IPV6;

    (void)profile;
    if (curIpFamily == IpFamilyV4 && WdsConnectionIPv4Handle == 0)
        return 0;
    if (curIpFamily == IpFamilyV6 && WdsConnectionIPv6Handle == 0)
        return 0;

    dbg_time("%s WdsConnectionIPv%dHandle", __func__, curIpFamily == IpFamilyV4 ? 4 : 6);

    pRequest = ComposeQMUXMsg(QMIType, QMIWDS_STOP_NETWORK_INTERFACE_REQ , WdsStopNwInterfaceReq, &curIpFamily);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    if (curIpFamily == IpFamilyV4)
        WdsConnectionIPv4Handle = 0;
    else
         WdsConnectionIPv6Handle = 0;
    free(pResponse);
    return 0;
}

static int requestGetIPAddress(PROFILE_T *profile, int curIpFamily) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV4_ADDR pIpv4Addr;
    PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV6_ADDR pIpv6Addr = NULL;
    PQMIWDS_GET_RUNTIME_SETTINGS_TLV_MTU pMtu;
    IPV4_T *pIpv4 = &profile->ipv4;
    IPV6_T *pIpv6 = &profile->ipv6;
    UCHAR QMIType = (curIpFamily == 0x04) ? QMUX_TYPE_WDS : QMUX_TYPE_WDS_IPV6;
    PQMIWDS_GET_RUNNING_SETTINGS_PCSCF_IPV6_ADDR pPCSCFIpv6Addr;
	PQMIWDS_GET_RUNNING_SETTINGS_PCSCF_IPV4_ADDR pPCSCFIpv4Addr;

    if (curIpFamily == IpFamilyV4) {
        memset(pIpv4, 0x00, sizeof(IPV4_T));
        if (WdsConnectionIPv4Handle == 0)
            return 0;
    } else if (curIpFamily == IpFamilyV6) {
        memset(pIpv6, 0x00, sizeof(IPV6_T));
        if (WdsConnectionIPv6Handle == 0)
            return 0;
    }

    pRequest = ComposeQMUXMsg(QMIType, QMIWDS_GET_RUNTIME_SETTINGS_REQ, WdsGetRuntimeSettingReq, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

	pPCSCFIpv6Addr = (PQMIWDS_GET_RUNNING_SETTINGS_PCSCF_IPV6_ADDR)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x2e);    // 0x2e - pcscf ipv6 address 
	if (pPCSCFIpv6Addr) {
    	if (pPCSCFIpv6Addr->PCSCFNumber == 1) {
        	UCHAR *PCSCFIpv6Addr1 = (UCHAR *)(pPCSCFIpv6Addr + 1);
        	memcpy(profile->PCSCFIpv6Addr1, PCSCFIpv6Addr1, 16);
        }else if (pPCSCFIpv6Addr->PCSCFNumber == 2) {
        	UCHAR *PCSCFIpv6Addr1 = (UCHAR *)(pPCSCFIpv6Addr + 1);
        	UCHAR *PCSCFIpv6Addr2 = PCSCFIpv6Addr1 + 16;
        	memcpy(profile->PCSCFIpv6Addr1, PCSCFIpv6Addr1, 16);
        	memcpy(profile->PCSCFIpv6Addr2, PCSCFIpv6Addr2, 16);
        }
    }
    
	pPCSCFIpv4Addr = (PQMIWDS_GET_RUNNING_SETTINGS_PCSCF_IPV4_ADDR)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x23);    // 0x23 - pcscf ipv4 address 
	if (pPCSCFIpv4Addr) {
    	if (pPCSCFIpv4Addr->PCSCFNumber == 1) {
        	UCHAR *PCSCFIpv4Addr1 = (UCHAR *)(pPCSCFIpv4Addr + 1);
        	memcpy(&profile->PCSCFIpv4Addr1, PCSCFIpv4Addr1, 4);
        }else if (pPCSCFIpv4Addr->PCSCFNumber == 2) {
        	UCHAR *PCSCFIpv4Addr1 = (UCHAR *)(pPCSCFIpv4Addr + 1);
        	UCHAR *PCSCFIpv4Addr2 = PCSCFIpv4Addr1 + 4;
        	memcpy(&profile->PCSCFIpv4Addr1, PCSCFIpv4Addr1, 4);
        	memcpy(&profile->PCSCFIpv4Addr2, PCSCFIpv4Addr2, 4);
        }
    }

    pIpv4Addr = (PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV4_ADDR)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV4PRIMARYDNS);
    if (pIpv4Addr) {
        pIpv4->DnsPrimary = pIpv4Addr->IPV4Address;
    }

    pIpv4Addr = (PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV4_ADDR)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV4SECONDARYDNS);
    if (pIpv4Addr) {
        pIpv4->DnsSecondary = pIpv4Addr->IPV4Address;
    }

    pIpv4Addr = (PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV4_ADDR)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV4GATEWAY);
    if (pIpv4Addr) {
        pIpv4->Gateway = pIpv4Addr->IPV4Address;
    }

    pIpv4Addr = (PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV4_ADDR)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV4SUBNET);
    if (pIpv4Addr) {
        pIpv4->SubnetMask = pIpv4Addr->IPV4Address;
    }

    pIpv4Addr = (PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV4_ADDR)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV4);
    if (pIpv4Addr) {
        pIpv4->Address = pIpv4Addr->IPV4Address;
    }

    pIpv6Addr = (PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV6_ADDR)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV6PRIMARYDNS);
    if (pIpv6Addr) {
        memcpy(pIpv6->DnsPrimary, pIpv6Addr->IPV6Address, 16);
    }

    pIpv6Addr = (PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV6_ADDR)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV6SECONDARYDNS);
    if (pIpv6Addr) {
        memcpy(pIpv6->DnsSecondary, pIpv6Addr->IPV6Address, 16);
    }

    pIpv6Addr = (PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV6_ADDR)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV6GATEWAY);
    if (pIpv6Addr) {
        memcpy(pIpv6->Gateway, pIpv6Addr->IPV6Address, 16);
        pIpv6->PrefixLengthGateway = pIpv6Addr->PrefixLength;
    }

    pIpv6Addr = (PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV6_ADDR)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV6);
    if (pIpv6Addr) {
        memcpy(pIpv6->Address, pIpv6Addr->IPV6Address, 16);
        pIpv6->PrefixLengthIPAddr = pIpv6Addr->PrefixLength;
    }

    pMtu = (PQMIWDS_GET_RUNTIME_SETTINGS_TLV_MTU)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_MTU);
    if (pMtu) {
        if (curIpFamily == IpFamilyV4)
            pIpv4->Mtu =  le32_to_cpu(pMtu->Mtu);
        else
            pIpv6->Mtu =  le32_to_cpu(pMtu->Mtu);
    }

    free(pResponse);
    return 0;
}

#ifdef CONFIG_APN
static int requestSetProfile(PROFILE_T *profile) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    const char *new_apn = profile->apn ? profile->apn : "";
    const char *new_user = profile->user ? profile->user : "";
    const char *new_password = profile->password ? profile->password : "";
    const char *ipStr[] = {"IPV4", "NULL", "IPV6", "IPV4V6"};

    dbg_time("%s[pdp:%d index:%d] %s/%s/%s/%d/%s", __func__, profile->pdp, profile->profile_index, profile->apn, profile->user, profile->password, profile->auth,ipStr[profile->iptype]);
    if (!profile->profile_index)
        return -1;

    if ( !strcmp(profile->old_apn, new_apn) && !strcmp(profile->old_user, new_user)
        && !strcmp(profile->old_password, new_password)
        && profile->old_iptype == profile->iptype
        && profile->old_auth == profile->auth)
    {
        dbg_time("no need to set skip the rest");
        return 0;
    }

    pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS, QMIWDS_MODIFY_PROFILE_SETTINGS_REQ, WdsModifyProfileSettingsReq, profile);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    free(pResponse);
    return 1;
}

static int requestGetProfile(PROFILE_T *profile) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    PQMIWDS_APNNAME pApnName;
    PQMIWDS_USERNAME pUserName;
    PQMIWDS_PASSWD pPassWd;
    PQMIWDS_AUTH_PREFERENCE pAuthPref;
    PQMIWDS_IPTYPE pIpType;
    PQMIWDS_PDPCONTEXT pPdpContext;
    PQMIWDS_PROFILELIST pProfileList;
    
    const char *ipStr[] = {"IPV4", "NULL", "IPV6", "IPV4V6"};

    profile->old_apn[0] = profile->old_user[0] = profile->old_password[0] = '\0';
    profile->old_auth = 0;
    profile->old_iptype = 0;
    if (profile->enable_ipv4 && profile->enable_ipv6)
        profile->iptype = 3;
    else if (profile->enable_ipv6)
        profile->iptype = 2;
    else
        profile->iptype = 0;

    if (!profile->pdp)
        return 0;

_re_check:
    pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS, QMIWDS_GET_PROFILE_LIST_REQ, WdsGetProfileListReqSend, profile);
    err = QmiThreadSendQMI(pRequest, &pResponse);s_pResponse = malloc(le16_to_cpu(pResponse->QMIHdr.Length) + 1);
    qmi_rsp_check_and_return();
    
    pProfileList = (PQMIWDS_PROFILELIST)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x01);
    uint8 profile_indexs[42] = {0};
    uint8 profile_num = pProfileList->ProfileList[0];
    if(profile_num >= 1)
    {
        uint8 j = 0;
        uint8 k = 2;
        for(int i=0; i<profile_num; i++)
        {
            profile_indexs[j++] = pProfileList->ProfileList[k];
            if(pProfileList->ProfileList[++k] == 0)
                k+=2;
            else
                k+=2+pProfileList->ProfileList[k];
        }
    }
    free(pResponse);
    
    for(int i=0; i<profile_num; i++)
    {
        profile->profile_index = profile_indexs[i];
        
        pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS, QMIWDS_GET_PROFILE_SETTINGS_REQ, WdsGetProfileSettingsReqSend, profile);
        err = QmiThreadSendQMI(pRequest, &pResponse);
        qmi_rsp_check_and_return();
        
        pPdpContext = (PQMIWDS_PDPCONTEXT)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x25);
        if(pPdpContext->pdp_context == profile->pdp)
            break;
        else
            free(pResponse);

        if(i == profile_num-1)
        {
            pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS, QMIWDS_CREATE_PROFILE_REQ, WdsCreateProfileSettingsReqSend, profile);
            err = QmiThreadSendQMI(pRequest, &pResponse);
            qmi_rsp_check_and_return();
            free(pResponse);
            goto _re_check;
        }
    }
    

    pApnName = (PQMIWDS_APNNAME)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x14);
    pUserName = (PQMIWDS_USERNAME)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x1B);
    pPassWd = (PQMIWDS_PASSWD)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x1C);
    pAuthPref = (PQMIWDS_AUTH_PREFERENCE)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x1D);
    pIpType = (PQMIWDS_IPTYPE)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x11);

    if (pApnName/* && le16_to_cpu(pApnName->TLVLength)*/)
        uchar2char(profile->old_apn, sizeof(profile->old_apn), &pApnName->ApnName, le16_to_cpu(pApnName->TLVLength));
    if (pUserName/*  && pUserName->UserName*/)
        uchar2char(profile->old_user, sizeof(profile->old_user), &pUserName->UserName, le16_to_cpu(pUserName->TLVLength));
    if (pPassWd/*  && le16_to_cpu(pPassWd->TLVLength)*/)
        uchar2char(profile->old_password, sizeof(profile->old_password), &pPassWd->Passwd, le16_to_cpu(pPassWd->TLVLength));
    if (pAuthPref/*  && le16_to_cpu(pAuthPref->TLVLength)*/) {
        profile->old_auth = pAuthPref->AuthPreference;
    }
    if (pIpType) {
        profile->old_iptype = pIpType->IPType;
    }

    dbg_time("%s[pdp:%d index:%d] %s/%s/%s/%d/%s", __func__, profile->pdp, profile->profile_index, profile->old_apn, profile->old_user, profile->old_password, profile->old_auth, ipStr[profile->old_iptype]);

    free(pResponse);
    return 0;
}
#endif

#ifdef CONFIG_SIGNALINFO
static int requestGetSignalInfo(void)
{
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;

    pRequest = ComposeQMUXMsg(QMUX_TYPE_NAS, QMINAS_GET_SIG_INFO_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    
    // CDMA
    {
        PQMINAS_SIG_INFO_CDMA_TLV_MSG ptlv = (PQMINAS_SIG_INFO_CDMA_TLV_MSG)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x10);
        if (ptlv && ptlv->TLVLength)
        {
            dbg_time("%s CDMA: RSSI %d dBm, ECIO %.1lf dBm", __func__,
                ptlv->rssi, (-0.5) * (double)ptlv->ecio);
        }
    }

    // HDR
    {
        PQMINAS_SIG_INFO_HDR_TLV_MSG ptlv = (PQMINAS_SIG_INFO_HDR_TLV_MSG)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x11);
        if (ptlv && ptlv->TLVLength)
        {
            dbg_time("%s HDR: RSSI %d dBm, ECIO %.1lf dBm, IO %d dBm", __func__,
                ptlv->rssi, (-0.5) * (double)ptlv->ecio, ptlv->io);
        }
    }

    // GSM
    {
        PQMINAS_SIG_INFO_GSM_TLV_MSG ptlv = (PQMINAS_SIG_INFO_GSM_TLV_MSG)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x12);
        if (ptlv && ptlv->TLVLength)
        {
            dbg_time("%s GSM: RSSI %d dBm", __func__, ptlv->rssi);
        }
    }

    // WCDMA
    {
        PQMINAS_SIG_INFO_WCDMA_TLV_MSG ptlv = (PQMINAS_SIG_INFO_WCDMA_TLV_MSG)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x13);
        if (ptlv && ptlv->TLVLength)
        {
            dbg_time("%s WCDMA: RSSI %d dBm, ECIO %.1lf dBm", __func__,
                ptlv->rssi, (-0.5) * (double)ptlv->ecio);
        }
    }

    // LTE
    {
        PQMINAS_SIG_INFO_LTE_TLV_MSG ptlv = (PQMINAS_SIG_INFO_LTE_TLV_MSG)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x14);
        if (ptlv && ptlv->TLVLength)
        {
            dbg_time("%s LTE: RSSI %d dBm, RSRQ %d dB, RSRP %d dBm, SNR %.1lf dB", __func__,
                ptlv->rssi, ptlv->rsrq, ptlv->rsrp, (0.1) * (double)ptlv->snr);
        }
    }

    // TDSCDMA
    {
        PQMINAS_SIG_INFO_TDSCDMA_TLV_MSG ptlv = (PQMINAS_SIG_INFO_TDSCDMA_TLV_MSG)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x15);
        if (ptlv && ptlv->TLVLength)
        {
            dbg_time("%s LTE: RSCP %d dBm", __func__, ptlv->rscp);
        }
    }

    // 5G_NSA
    if (s_5g_type == WWAN_DATA_CLASS_5G_NSA)
    {
        PQMINAS_SIG_INFO_5G_NSA_TLV_MSG ptlv = (PQMINAS_SIG_INFO_5G_NSA_TLV_MSG)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x17);
        if (ptlv && ptlv->TLVLength)
        {
            dbg_time("%s 5G_NSA: RSRP %d dBm, SNR %.1lf dB", __func__, ptlv->rsrp, (0.1) * (double)ptlv->snr);
        }
    }

    // 5G_SA
    if (s_5g_type == WWAN_DATA_CLASS_5G_SA)
    {
        PQMINAS_SIG_INFO_5G_SA_TLV_MSG ptlv = (PQMINAS_SIG_INFO_5G_SA_TLV_MSG)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x18);
        if (ptlv && ptlv->TLVLength)
        {
            dbg_time("%s 5G_SA: NR5G_RSRQ %d dB", __func__, ptlv->nr5g_rsrq);
        }
    }

    free(pResponse);
    return 0;
}
#endif

#ifdef CONFIG_VERSION
static int requestBaseBandVersion(PROFILE_T *profile) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    PDEVICE_REV_ID revId;
    int err;

    pRequest = ComposeQMUXMsg(QMUX_TYPE_DMS, QMIDMS_GET_DEVICE_REV_ID_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    revId = (PDEVICE_REV_ID)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x01);

    if (revId && le16_to_cpu(revId->TLVLength))
    {
        uchar2char(profile->BaseBandVersion, sizeof(profile->BaseBandVersion), &revId->RevisionID, le16_to_cpu(revId->TLVLength));
        dbg_time("%s %s", __func__, profile->BaseBandVersion);
    }

    free(pResponse);
    return 0;
}
#endif

static USHORT DmsSetOperatingModeReq(PQMUX_MSG pMUXMsg, void *arg) {
    pMUXMsg->SetOperatingModeReq.TLVType = 0x01;
    pMUXMsg->SetOperatingModeReq.TLVLength = cpu_to_le16(1);
    pMUXMsg->SetOperatingModeReq.OperatingMode = *((UCHAR *)arg);

    return sizeof(QMIDMS_SET_OPERATING_MODE_REQ_MSG);
}

static USHORT UimSetCardSlotReq(PQMUX_MSG pMUXMsg, void *arg) {
    pMUXMsg->UIMSetCardSlotReq.TLVType = 0x01;
    pMUXMsg->UIMSetCardSlotReq.TLVLength = cpu_to_le16(1);
    pMUXMsg->UIMSetCardSlotReq.slot = *((UCHAR *)arg);

    return sizeof(QMIUIM_SET_CARD_SLOT_REQ_MSG);
}

static int requestRadioPower(int state) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    UCHAR OperatingMode = (!!state) ? DMS_OP_MODE_ONLINE : DMS_OP_MODE_LOW_POWER;
    USHORT SimOp = (!!state) ? QMIUIM_POWER_UP : QMIUIM_POWER_DOWN;
    UCHAR cardSlot = 0x01;

    dbg_time("%s(%d)", __func__, state);

    pRequest = ComposeQMUXMsg(QMUX_TYPE_DMS, QMIDMS_SET_OPERATING_MODE_REQ, DmsSetOperatingModeReq, &OperatingMode);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();
    free(pResponse);

    pRequest = ComposeQMUXMsg(QMUX_TYPE_UIM, SimOp, UimSetCardSlotReq, &cardSlot);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();
    free(pResponse);

    return 0;
}

static USHORT WdaSetLoopBackReq(PQMUX_MSG pMUXMsg, void *arg) {
    (void)arg;
	pMUXMsg->SetLoopBackReq.loopback_state.TLVType  = 0x01;
	pMUXMsg->SetLoopBackReq.loopback_state.TLVLength = cpu_to_le16(1);

	pMUXMsg->SetLoopBackReq.replication_factor.TLVType	= 0x10;
	pMUXMsg->SetLoopBackReq.replication_factor.TLVLength = cpu_to_le16(4);

	return sizeof(QMI_WDA_SET_LOOPBACK_CONFIG_REQ_MSG);
}

static int requestSetLoopBackState(UCHAR loopback_state, ULONG replication_factor) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;

    dbg_time("%s(loopback_state=%d, replication_factor=%u)", __func__, loopback_state, replication_factor);
    
    pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS_ADMIN, QMI_WDA_SET_LOOPBACK_CONFIG_REQ, WdaSetLoopBackReq, NULL);
    pRequest->MUXMsg.SetLoopBackReq.loopback_state.TLVVaule = loopback_state;
    pRequest->MUXMsg.SetLoopBackReq.replication_factor.TLVVaule = cpu_to_le16(replication_factor);

    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    free(pResponse);
    return 0;
}

#ifdef CONFIG_ENABLE_QOS
static USHORT QosSetBindMuxDataPort(PQMUX_MSG pMUXMsg, void *arg) {
    PROFILE_T *profile = (PROFILE_T *)arg;
    pMUXMsg->QosBindDataPortReq.EpIdTlv.TLVType = 0x10;
    pMUXMsg->QosBindDataPortReq.EpIdTlv.TLVLength = cpu_to_le16(8);
    pMUXMsg->QosBindDataPortReq.EpIdTlv.ep_type = cpu_to_le32(profile->rmnet_info.ep_type);
    pMUXMsg->QosBindDataPortReq.EpIdTlv.iface_id = cpu_to_le32(profile->rmnet_info.iface_id);
    pMUXMsg->QosBindDataPortReq.MuxIdTlv.TLVType = 0x11;
    pMUXMsg->QosBindDataPortReq.MuxIdTlv.TLVLength = cpu_to_le16(1);
    pMUXMsg->QosBindDataPortReq.MuxIdTlv.mux_id = profile->muxid;
    return sizeof(QMI_QOS_BIND_DATA_PORT_REQ_MSG);
}

#ifdef CONFIG_REG_QOS_IND
static USHORT QosIndRegReq(PQMUX_MSG pMUXMsg, void *arg) {
    pMUXMsg->QosIndRegReq.ReportGlobalQosFlowTlv.TLVType = 0x10;
    pMUXMsg->QosIndRegReq.ReportGlobalQosFlowTlv.TLVLength = cpu_to_le16(1);
    pMUXMsg->QosIndRegReq.ReportGlobalQosFlowTlv.report_global_qos_flows = 1;
    return sizeof(QMI_QOS_INDICATION_REGISTER_REQ_MSG);
}
#endif

static int requestRegisterQos(PROFILE_T *profile) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse = NULL;
    PQMUX_MSG pMUXMsg;
    int err;

    pRequest = ComposeQMUXMsg(QMUX_TYPE_QOS, QMI_QOS_BIND_DATA_PORT_REQ , QosSetBindMuxDataPort, (void *)profile);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    dbg_time("%s QosSetBindMuxDataPort", __func__);
    qmi_rsp_check_and_return();
    if (pResponse) free(pResponse);
	
#ifdef CONFIG_REG_QOS_IND
    pRequest = ComposeQMUXMsg(QMUX_TYPE_QOS, QMI_QOS_INDICATION_REGISTER_REQ , QosIndRegReq, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    dbg_time("%s QosIndRegReq", __func__);
    qmi_rsp_check_and_return();
    if (pResponse) free(pResponse);
#endif
    return 0;
}

#ifdef CONFIG_GET_QOS_INFO
UCHAR ql_get_qos_info_data_rate(PQCQMIMSG pResponse, void *max_data_rate)
{
    PQMI_QOS_GET_QOS_INFO_TLV_GRANTED_FLOW qos_tx_granted_flow = NULL;
    PQMI_QOS_GET_QOS_INFO_TLV_GRANTED_FLOW qos_rx_granted_flow = NULL;
    qos_tx_granted_flow = (PQMI_QOS_GET_QOS_INFO_TLV_GRANTED_FLOW)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x11);
    if(qos_tx_granted_flow != NULL)
    {
        *(ULONG64 *)(max_data_rate) = le64_to_cpu(qos_tx_granted_flow->data_rate_max);
        dbg_time("GET_QOS_INFO: tx_data_rate_max=%llu", *(ULONG64 *)(max_data_rate+0));
    }
    else
        dbg_time("GET_QOS_INFO: No qos_tx_granted_flow");
    qos_rx_granted_flow = (PQMI_QOS_GET_QOS_INFO_TLV_GRANTED_FLOW)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x12);
    if(qos_rx_granted_flow != NULL)
    {
        *(ULONG64 *)(max_data_rate+sizeof(ULONG64)) = le64_to_cpu(qos_rx_granted_flow->data_rate_max);
        dbg_time("GET_QOS_INFO: rx_data_rate_max=%llu", *(ULONG64 *)(max_data_rate+sizeof(ULONG64)));
    }
    else
        dbg_time("GET_QOS_INFO: No qos_rx_granted_flow");
    if(qos_tx_granted_flow != NULL || qos_rx_granted_flow != NULL)
        return 0;
    else
        return 1;
}

static USHORT QosGetQosInfoReq(PQMUX_MSG pMUXMsg, void *arg) {
    PROFILE_T *profile = (PROFILE_T *)arg;
    pMUXMsg->QosGetQosInfoReq.QosIdTlv.TLVType = 0x01;
    pMUXMsg->QosGetQosInfoReq.QosIdTlv.TLVLength = cpu_to_le16(4);
    pMUXMsg->QosGetQosInfoReq.QosIdTlv.qos_id = cpu_to_le32(profile->qos_id);
    return sizeof(QMI_QOS_GET_QOS_INFO_REQ_MSG);
}

static int requestGetQosInfo(PROFILE_T *profile) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse = NULL;
    PQMUX_MSG pMUXMsg;
    int err;

    if(profile->qos_id == 0)
    {
        dbg_time("%s request not send: invalid qos_id", __func__);
        return 0;
    }
    pRequest = ComposeQMUXMsg(QMUX_TYPE_QOS, QMI_QOS_GET_QOS_INFO_REQ , QosGetQosInfoReq, (void *)profile);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();
    if (pResponse)
    {
#ifdef CONFIG_GET_QOS_DATA_RATE
        ULONG64 max_data_rate[2] = {0};
        if(ql_get_qos_info_data_rate(pResponse, (void *)max_data_rate) == 0){}
#endif
        free(pResponse);
    }
    return 0;
}
#endif //#ifdef CONFIG_GET_QOS_INFO

#ifdef CONFIG_REG_QOS_IND
UCHAR ql_get_global_qos_flow_ind_qos_id(PQCQMIMSG pResponse, UINT *qos_id)
{
    PQMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_STATE qos_flow_state = NULL;
    qos_flow_state = (PQMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_STATE)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x01);
    if(qos_flow_state != NULL)
    {
        if(le32_to_cpu(qos_flow_state->state_change) == QOS_IND_FLOW_STATE_ACTIVATED && qos_flow_state->new_flow == 1)
        {
            *qos_id = le32_to_cpu(qos_flow_state->qos_id);
            dbg_time("QMI_QOS_GLOBAL_QOS_FLOW_IND: qos_id=%u state=QOS_IND_FLOW_STATE_ACTIVATED", *qos_id);
        }
        return (qos_flow_state->new_flow);
    }
    return (0);
}

#ifdef CONFIG_GET_QOS_DATA_RATE
UCHAR ql_get_global_qos_flow_ind_data_rate(PQCQMIMSG pResponse, void *max_data_rate)
{
    PQMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_GRANTED qos_tx_flow_granted = NULL;
    PQMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_GRANTED qos_rx_flow_granted = NULL;
    qos_tx_flow_granted = (PQMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_GRANTED)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x10);
    if(qos_tx_flow_granted != NULL)
    {
        *(ULONG64 *)(max_data_rate) = le64_to_cpu(qos_tx_flow_granted->data_rate_max);
        dbg_time("QMI_QOS_GLOBAL_QOS_FLOW_IND: tx_data_rate_max=%llu", *(ULONG64 *)(max_data_rate+0));
    }
    else
    dbg_time("QMI_QOS_GLOBAL_QOS_FLOW_IND: No qos_tx_flow_granted");
    qos_rx_flow_granted = (PQMI_QOS_GLOBAL_QOS_FLOW_TLV_FLOW_GRANTED)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x11);
    if(qos_rx_flow_granted != NULL)
    {
        *(ULONG64 *)(max_data_rate+sizeof(ULONG64)) = le64_to_cpu(qos_rx_flow_granted->data_rate_max);
        dbg_time("QMI_QOS_GLOBAL_QOS_FLOW_IND: rx_data_rate_max=%llu", *(ULONG64 *)(max_data_rate+sizeof(ULONG64)));
    }
    else
        dbg_time("QMI_QOS_GLOBAL_QOS_FLOW_IND: No qos_rx_flow_granted");
    if(qos_tx_flow_granted != NULL || qos_rx_flow_granted != NULL)
        return 0;
    else
        return 1;
}
#endif
#endif //#ifdef CONFIG_REG_QOS_IND
#endif //#ifdef CONFIG_ENABLE_QOS

#ifdef CONFIG_CELLINFO
/*
    at+qeng="servingcell" and at+qeng="neighbourcell"
    https://gitlab.freedesktop.org/mobile-broadband/libqmi/-/blob/master/src/qmicli/qmicli-nas.c
*/
static int nas_get_cell_location_info(void);
static int nas_get_rf_band_information(void);

static int requestGetCellInfoList(void) {
    dbg_time("%s", __func__);
    nas_get_cell_location_info();
    nas_get_rf_band_information();
    return 0;
}
#endif

const struct request_ops qmi_request_ops = {
#ifdef CONFIG_VERSION
    .requestBaseBandVersion = requestBaseBandVersion,
#endif
    .requestSetEthMode = requestSetEthMode,
#ifdef CONFIG_SIM
    .requestGetSIMStatus = requestGetSIMStatus,
    .requestEnterSimPin = requestEnterSimPin,
#endif
#ifdef CONFIG_IMSI_ICCID
    .requestGetICCID = requestGetICCID,
    .requestGetIMSI = requestGetIMSI,
#endif
#ifdef CONFIG_APN
    .requestSetProfile = requestSetProfile,
    .requestGetProfile = requestGetProfile,
#endif
    .requestRegistrationState = requestRegistrationState,
    .requestSetupDataCall = requestSetupDataCall,
    .requestQueryDataCall = requestQueryDataCall,
    .requestDeactivateDefaultPDP = requestDeactivateDefaultPDP,
    .requestGetIPAddress = requestGetIPAddress,
#ifdef CONFIG_SIGNALINFO
    .requestGetSignalInfo = requestGetSignalInfo,
#endif
#ifdef CONFIG_CELLINFO
    .requestGetCellInfoList = requestGetCellInfoList,
#endif
    .requestSetLoopBackState = requestSetLoopBackState,
    .requestRadioPower = requestRadioPower,
#ifdef CONFIG_ENABLE_QOS
    .requestRegisterQos = requestRegisterQos,
#endif
#ifdef CONFIG_GET_QOS_INFO
    .requestGetQosInfo = requestGetQosInfo,
#endif
#ifdef CONFIG_COEX_WWAN_STATE
    .requestGetCoexWWANState = requestGetCoexWWANState,
#endif
};

#ifdef CONFIG_CELLINFO
static char *str_from_bcd_plmn (uint8 plmn[3])
{
    const char bcd_chars[] = "0123456789*#abc\0\0";
    static char str[12];
    int i;
    int j = 0;

    for (i = 0; i < 3; i++) {
        str[j] = bcd_chars[plmn[i]&0xF];
        if (str[j]) j++;
        str[j] = bcd_chars[plmn[i]>>4];
        if (str[j]) j++;
    }

    str[j++] = 0;

    return str;
}

typedef struct {
    UINT type;
    const char *name;
} ENUM_NAME_T;

#define enum_name(type) {type, #type}
#define N_ELEMENTS(arr)		(sizeof (arr) / sizeof ((arr)[0]))

static const ENUM_NAME_T QMI_NAS_ACTIVE_BAND_NAME[] = {
    enum_name(QMI_NAS_ACTIVE_BAND_BC_0),                  
    enum_name(QMI_NAS_ACTIVE_BAND_BC_1),                  
    enum_name(QMI_NAS_ACTIVE_BAND_BC_2),                  
    enum_name(QMI_NAS_ACTIVE_BAND_BC_3),                  
    enum_name(QMI_NAS_ACTIVE_BAND_BC_4),                  
    enum_name(QMI_NAS_ACTIVE_BAND_BC_5),                  
    enum_name(QMI_NAS_ACTIVE_BAND_BC_6),                  
    enum_name(QMI_NAS_ACTIVE_BAND_BC_7),                  
    enum_name(QMI_NAS_ACTIVE_BAND_BC_8),                  
    enum_name(QMI_NAS_ACTIVE_BAND_BC_9),                  
    enum_name(QMI_NAS_ACTIVE_BAND_BC_10),                 
    enum_name(QMI_NAS_ACTIVE_BAND_BC_11),                 
    enum_name(QMI_NAS_ACTIVE_BAND_BC_12),                 
    enum_name(QMI_NAS_ACTIVE_BAND_BC_13),                 
    enum_name(QMI_NAS_ACTIVE_BAND_BC_14),                 
    enum_name(QMI_NAS_ACTIVE_BAND_BC_15),                 
    enum_name(QMI_NAS_ACTIVE_BAND_BC_16),                 
    enum_name(QMI_NAS_ACTIVE_BAND_BC_17),                 
    enum_name(QMI_NAS_ACTIVE_BAND_BC_18),                 
    enum_name(QMI_NAS_ACTIVE_BAND_BC_19),                 
    enum_name(QMI_NAS_ACTIVE_BAND_GSM_450),               
    enum_name(QMI_NAS_ACTIVE_BAND_GSM_480),               
    enum_name(QMI_NAS_ACTIVE_BAND_GSM_750),               
    enum_name(QMI_NAS_ACTIVE_BAND_GSM_850),               
    enum_name(QMI_NAS_ACTIVE_BAND_GSM_900_EXTENDED),      
    enum_name(QMI_NAS_ACTIVE_BAND_GSM_900_PRIMARY),       
    enum_name(QMI_NAS_ACTIVE_BAND_GSM_900_RAILWAYS),      
    enum_name(QMI_NAS_ACTIVE_BAND_GSM_DCS_1800),          
    enum_name(QMI_NAS_ACTIVE_BAND_GSM_PCS_1900),          
    enum_name(QMI_NAS_ACTIVE_BAND_WCDMA_2100),            
    enum_name(QMI_NAS_ACTIVE_BAND_WCDMA_PCS_1900),        
    enum_name(QMI_NAS_ACTIVE_BAND_WCDMA_DCS_1800),        
    enum_name(QMI_NAS_ACTIVE_BAND_WCDMA_1700_US),         
    enum_name(QMI_NAS_ACTIVE_BAND_WCDMA_850),             
    enum_name(QMI_NAS_ACTIVE_BAND_WCDMA_800),             
    enum_name(QMI_NAS_ACTIVE_BAND_WCDMA_2600),            
    enum_name(QMI_NAS_ACTIVE_BAND_WCDMA_900),             
    enum_name(QMI_NAS_ACTIVE_BAND_WCDMA_1700_JAPAN),      
    enum_name(QMI_NAS_ACTIVE_BAND_WCDMA_1500_JAPAN),      
    enum_name(QMI_NAS_ACTIVE_BAND_WCDMA_850_JAPAN),       
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_1),              
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_2),              
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_3),              
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_4),              
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_5),              
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_6),              
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_7),              
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_8),              
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_9),              
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_10),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_11),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_12),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_13),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_14),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_17),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_18),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_19),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_20),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_21),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_23),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_24),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_25),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_26),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_27),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_28),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_29),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_30),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_31),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_32),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_33),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_34),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_35),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_36),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_37),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_38),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_39),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_40),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_41),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_42),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_43),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_46),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_47),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_48),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_66),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_71),             
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_125),            
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_126),            
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_127),            
    enum_name(QMI_NAS_ACTIVE_BAND_EUTRAN_250),            
    enum_name(QMI_NAS_ACTIVE_BAND_TDSCDMA_A),             
    enum_name(QMI_NAS_ACTIVE_BAND_TDSCDMA_B),             
    enum_name(QMI_NAS_ACTIVE_BAND_TDSCDMA_C),             
    enum_name(QMI_NAS_ACTIVE_BAND_TDSCDMA_D),             
    enum_name(QMI_NAS_ACTIVE_BAND_TDSCDMA_E),             
    enum_name(QMI_NAS_ACTIVE_BAND_TDSCDMA_F),             
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_1 ),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_2 ),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_3 ),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_5 ),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_7 ),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_8 ),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_20),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_28),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_38),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_41),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_50),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_51),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_66),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_70),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_71),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_74),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_75),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_76),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_77),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_78),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_79),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_80),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_81),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_82),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_83),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_84),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_85),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_257),         
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_258),         
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_259),         
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_260),         
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_261),         
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_12),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_25),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_34),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_39),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_40),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_65),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_86),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_48),          
    enum_name(QMI_NAS_ACTIVE_BAND_NR5G_BAND_14),          
};

static const char *qmi_nas_radio_interface_get_string(uint8 radio_if)
{
    const char *str = NULL;

    switch (radio_if) {
        case QMI_NAS_RADIO_INTERFACE_CDMA_1X: str = "cdma-1x"; break;
        case QMI_NAS_RADIO_INTERFACE_CDMA_1XEVDO: str = "cdma-1xevdo"; break;
        case QMI_NAS_RADIO_INTERFACE_AMPS: str = "amps"; break;
        case QMI_NAS_RADIO_INTERFACE_GSM: str = "gsm"; break;
        case QMI_NAS_RADIO_INTERFACE_UMTS: str = "umts"; break;
        case QMI_NAS_RADIO_INTERFACE_LTE: str = "lte"; break;
        case QMI_NAS_RADIO_INTERFACE_TD_SCDMA: str = "td-scdma"; break;
        case QMI_NAS_RADIO_INTERFACE_5GNR: str = "5gnr"; break;
        default: str = NULL; break;
    }

    return str ?  str : "unknown";
}

static const char *qmi_nas_active_band_get_string(uint32 active_band)
{
    size_t i;

    for (i = 0; i < N_ELEMENTS(QMI_NAS_ACTIVE_BAND_NAME); i++) {
        if (active_band == QMI_NAS_ACTIVE_BAND_NAME[i].type)
            return QMI_NAS_ACTIVE_BAND_NAME[i].name + strlen("QMI_NAS_ACTIVE_BAND_");
    }

    return "unknown";
}

typedef struct {
    uint16      min;
    uint16      max;
    const char *name;
} EarfcnRange;

/* http://niviuk.free.fr/lte_band.php */
static const EarfcnRange earfcn_ranges[] = {
    {     0,   599, "E-UTRA band 1: 2100"           },
    {   600,  1199, "E-UTRA band 2: 1900 PCS"       },
    {  1200,  1949, "E-UTRA band 3: 1800+"          },
    {  1950,  2399, "E-UTRA band 4: AWS-1"          },
    {  2400,  2649, "E-UTRA band 5: 850"            },
    {  2650,  2749, "E-UTRA band 6: UMTS only"      },
    {  2750,  3449, "E-UTRA band 7: 2600"           },
    {  3450,  3799, "E-UTRA band 8: 900"            },
    {  3800,  4149, "E-UTRA band 9: 1800"           },
    {  4150,  4749, "E-UTRA band 10: AWS-1+"        },
    {  4750,  4999, "E-UTRA band 11: 1500 Lower"    },
    {  5000,  5179, "E-UTRA band 12: 700 a"         },
    {  5180,  5279, "E-UTRA band 13: 700 c"         },
    {  5280,  5379, "E-UTRA band 14: 700 PS"        },
    {  5730,  5849, "E-UTRA band 17: 700 b"         },
    {  5850,  5999, "E-UTRA band 18: 800 Lower"     },
    {  6000,  6149, "E-UTRA band 19: 800 Upper"     },
    {  6150,  6449, "E-UTRA band 20: 800 DD"        },
    {  6450,  6599, "E-UTRA band 21: 1500 Upper"    },
    {  6600,  7399, "E-UTRA band 22: 3500"          },
    {  7500,  7699, "E-UTRA band 23: 2000 S-band"   },
    {  7700,  8039, "E-UTRA band 24: 1600 L-band"   },
    {  8040,  8689, "E-UTRA band 25: 1900+"         },
    {  8690,  9039, "E-UTRA band 26: 850+"          },
    {  9040,  9209, "E-UTRA band 27: 800 SMR"       },
    {  9210,  9659, "E-UTRA band 28: 700 APT"       },
    {  9660,  9769, "E-UTRA band 29: 700 d"         },
    {  9770,  9869, "E-UTRA band 30: 2300 WCS"      },
    {  9870,  9919, "E-UTRA band 31: 450"           },
    {  9920, 10359, "E-UTRA band 32: 1500 L-band"   },
    { 36000, 36199, "E-UTRA band 33: TD 1900"       },
    { 36200, 36349, "E-UTRA band 34: TD 2000"       },
    { 36350, 36949, "E-UTRA band 35: TD PCS Lower"  },
    { 36950, 37549, "E-UTRA band 36: TD PCS Upper"  },
    { 37550, 37749, "E-UTRA band 37: TD PCS Center" },
    { 37750, 38249, "E-UTRA band 38: TD 2600"       },
    { 38250, 38649, "E-UTRA band 39: TD 1900+"      },
    { 38650, 39649, "E-UTRA band 40: TD 2300"       },
    { 39650, 41589, "E-UTRA band 41: TD 2500"       },
    { 41590, 43589, "E-UTRA band 42: TD 3500"       },
    { 43590, 45589, "E-UTRA band 43: TD 3700"       },
    { 45590, 46589, "E-UTRA band 44: TD 700"        },
};

static const char * earfcn_to_eutra_band_string (uint16 earfcn)
{
    size_t i;

    for (i = 0; i < N_ELEMENTS (earfcn_ranges); i++) {
        if (earfcn <= earfcn_ranges[i].max && earfcn >= earfcn_ranges[i].min)
            return earfcn_ranges[i].name;
    }
    
    return "unknown";
}

static int nas_get_cell_location_info(void)
{
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    PQMI_TLV pV;
    int err;
    int i, j;

    pRequest = ComposeQMUXMsg(QMUX_TYPE_NAS, QMINAS_GET_CELL_LOCATION_INFO_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    pV = (PQMI_TLV)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x2E);
    if (pV && pV->TLVLength) {
        printf ("5GNR ARFCN: '%u'\n", pV->u32);
    }

    {
        NasGetCellLocationNr5gServingCell *ptlv = (NasGetCellLocationNr5gServingCell *)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x2F);
        if (ptlv && ptlv->TLVLength)
        {
            printf ("5GNR cell information:\n"
                     "\tPLMN: '%s'\n"
                     "\tTracking Area Code: '%u'\n"
                     "\tGlobal Cell ID: '%" PRIu64 "'\n"
                     "\tPhysical Cell ID: '%u'\n"
                     "\tRSRQ: '%.1lf dB'\n"
                     "\tRSRP: '%.1lf dBm'\n"
                     "\tSNR: '%.1lf dB'\n",
                     str_from_bcd_plmn(ptlv->plmn),
                     ptlv->tac[0]<<16 | ptlv->tac[1]<<8 | ptlv->tac[2] ,
                     ptlv->global_cell_id,
                     ptlv->physical_cell_id,
                     (0.1) * ((double)ptlv->rsrq),
                     (0.1) * ((double)ptlv->rsrp),
                     (0.1) * ((double)ptlv->snr));
        }
    }

    {
        NasGetCellLocationLteInfoIntrafrequency *ptlv = (NasGetCellLocationLteInfoIntrafrequency *)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x13);
        if (ptlv && ptlv->TLVLength)
        {
            printf ("Intrafrequency LTE Info:\n"
                    "\tUE In Idle: '%s'\n"
                    "\tPLMN: '%s'\n"
                    "\tTracking Area Code: '%u'\n"
                    "\tGlobal Cell ID: '%u'\n"
                    "\tEUTRA Absolute RF Channel Number: '%u' (%s)\n"
                    "\tServing Cell ID: '%u'\n",
                    ptlv->ue_in_idle ? "yes" : "no",
                    str_from_bcd_plmn(ptlv->plmn),
                    ptlv->tracking_area_code,
                    ptlv->global_cell_id,
                    ptlv->absolute_rf_channel_number, earfcn_to_eutra_band_string(ptlv->absolute_rf_channel_number),
                    ptlv->serving_cell_id);

                if (ptlv->ue_in_idle)
                    printf ("\tCell Reselection Priority: '%u'\n"
                             "\tS Non Intra Search Threshold: '%u'\n"
                             "\tServing Cell Low Threshold: '%u'\n"
                             "\tS Intra Search Threshold: '%u'\n",
                             ptlv->cell_reselection_priority,
                             ptlv->s_non_intra_search_threshold,
                             ptlv->serving_cell_low_threshold,
                             ptlv->s_intra_search_threshold);


                for (i = 0; i < ptlv->cells_len; i++) {
                    NasGetCellLocationLteInfoCell *cell = &ptlv->cells_array[i];

                    printf ("\tCell [%u]:\n"
                             "\t\tPhysical Cell ID: '%u'\n"
                             "\t\tRSRQ: '%.1lf' dB\n"
                             "\t\tRSRP: '%.1lf' dBm\n"
                             "\t\tRSSI: '%.1lf' dBm\n",
                             i,
                             cell->physical_cell_id,
                             (double) cell->rsrq * 0.1,
                             (double) cell->rsrp * 0.1,
                             (double) cell->rssi * 0.1);

                    if (ptlv->ue_in_idle)
                        printf ("\t\tCell Selection RX Level: '%d'\n",
                                 cell->cell_selection_rx_level);
                }
        }
    }

    {
        NasGetCellLocationLteInfoInterfrequency *ptlv = (NasGetCellLocationLteInfoInterfrequency *)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x14);
        if (ptlv && ptlv->TLVLength)
        {
            int off = offsetof(NasGetCellLocationLteInfoInterfrequency, freqs[0]);
            printf ("Interfrequency LTE Info:\n"
                     "\tUE In Idle: '%s'\n", ptlv->ue_in_idle ? "yes" : "no");

                for (i = 0; i < ptlv->freqs_len; i++) {
                    NasGetCellLocationLteInfoInterfrequencyFrequencyElement *freq = (((void *)ptlv) + off);

                    off += sizeof(*freq);
                    printf ("\tFrequency [%u]:\n"
                             "\t\tEUTRA Absolute RF Channel Number: '%u' (%s)\n"
                             "\t\tSelection RX Level Low Threshold: '%u'\n"
                             "\t\tCell Selection RX Level High Threshold: '%u'\n",
                             i,
                             freq->eutra_absolute_rf_channel_number, earfcn_to_eutra_band_string(freq->eutra_absolute_rf_channel_number),
                             freq->cell_selection_rx_level_low_threshold,
                             freq->cell_selection_rx_level_high_threshold);
                    if (ptlv->ue_in_idle)
                        printf ("\t\tCell Reselection Priority: '%u'\n",
                                 freq->cell_reselection_priority);


                    for (j = 0; j < freq->cells_len; j++) {
                        NasGetCellLocationLteInfoCell *cell = &freq->cells_array[j];

                        off += sizeof(*cell);
                        printf ("\t\tCell [%u]:\n"
                                 "\t\t\tPhysical Cell ID: '%u'\n"
                                 "\t\t\tRSRQ: '%.1lf' dB\n"
                                 "\t\t\tRSRP: '%.1lf' dBm\n"
                                 "\t\t\tRSSI: '%.1lf' dBm\n"
                                 "\t\t\tCell Selection RX Level: '%u'\n",
                                 j,
                                 cell->physical_cell_id,
                                 (double) cell->rsrq * 0.1,
                                 (double) cell->rsrp * 0.1,
                                 (double) cell->rssi * 0.1,
                                 cell->cell_selection_rx_level);
                    }
                }
        }
    }

    pV = (PQMI_TLV)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x1E);
    if (pV && pV->TLVLength) {
        if (pV->u32 == 0xFFFFFFFF)
            printf ("LTE Timing Advance: 'unavailable'\n");
        else
            printf ("LTE Timing Advance: '%u'\n", pV->u32);
    }
    
    free(pResponse);
    return 0;
}

static int nas_get_rf_band_information(void)
{
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    int i;

    pRequest = ComposeQMUXMsg(QMUX_TYPE_NAS, QMINAS_GET_RF_BAND_INFO_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    {
        NasGetRfBandInfoList *ptlv = (NasGetRfBandInfoList *)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x01);
        if (ptlv && ptlv->TLVLength)
        {
            printf ("Band Information:\n");
            for (i = 0; i < ptlv->num_instances; i++) {
                NasGetRfBandInfo *band = &ptlv->bands_array[i];

                printf ("\tRadio Interface:   '%s'\n"
                         "\tActive Band Class: '%s'\n"
                         "\tActive Channel:    '%u'\n",
                         qmi_nas_radio_interface_get_string (band->radio_if),
                         qmi_nas_active_band_get_string (band->active_band),
                         band->active_channel);
            }
        }
    }

    {
        NasGetRfBandInfoExtendedList *ptlv = (NasGetRfBandInfoExtendedList *)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x11);
        if (ptlv && ptlv->TLVLength)
        {
            printf ("Band Information (Extended):\n");
            for (i = 0; i < ptlv->num_instances; i++) {
                NasGetRfBandInfoExtended *band = &ptlv->bands_array[i];

                printf ("\tRadio Interface:   '%s'\n"
                         "\tActive Band Class: '%s'\n"
                         "\tActive Channel:    '%u'\n",
                         qmi_nas_radio_interface_get_string (band->radio_if),
                         qmi_nas_active_band_get_string (band->active_band),
                         band->active_channel);
            }
        }
    }

    {
        NasGetRfBandInfoBandWidthList *ptlv = (NasGetRfBandInfoBandWidthList *)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x12);
        if (ptlv && ptlv->TLVLength)
        {
            printf ("Bandwidth:\n");
            for (i = 0; i < ptlv->num_instances; i++) {
                NasGetRfBandInfoBandWidth *band = &ptlv->bands_array[i];

                printf ("\tRadio Interface:   '%s'\n"
                         "\tBandwidth:         '%u'\n",
                     qmi_nas_radio_interface_get_string (band->radio_if),
                     (band->bandwidth));
            }
        }
    }

    free(pResponse);
    return 0;
}
#endif
