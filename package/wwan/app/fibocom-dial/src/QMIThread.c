#include "QMIThread.h"
#include "query_pcie_mode.h"
extern char *strndup(const char *__string, size_t __n);

//2021-03-24 willa.liu@fibocom.com changed begin for support mantis 0071817
//extern int *speed_arr;
//extern int *name_arr;

PQCQMIMSG pResponse;
PQMUX_MSG pMUXMsg;
//2021-03-24 willa.liu@fibocom.com changed end for support mantis 0071817

#define qmi_rsp_check_and_return()\
    do {                                                                       \
	if (err < 0 || pResponse == NULL) {                                    \
	    dbg_time("%s err = %d", __func__, err);                            \
	    return err;                                                        \
	}                                                                      \
	pMUXMsg = &pResponse->MUXMsg;                                          \
	if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) ||                 \
	    le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {                  \
	    USHORT QMUXError = le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError); \
	    dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,       \
		     le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult),          \
		     QMUXError);                                               \
	    free(pResponse);                                                   \
	    return QMUXError;                                                  \
	}                                                                      \
    } while (0)

#define qmi_rsp_check()                                                        \
    do {                                                                       \
	if (err < 0 || pResponse == NULL) {                                    \
	    dbg_time("%s err = %d", __func__, err);                            \
	    return err;                                                        \
	}                                                                      \
	pMUXMsg = &pResponse->MUXMsg;                                          \
	if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) ||                 \
	    le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {                  \
	    USHORT QMUXError = le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError); \
	    dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,       \
		     le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult),          \
		     QMUXError);                                               \
	}                                                                      \
    } while (0)

int qmiclientId[QMUX_TYPE_WDS_ADMIN +
		1];  // GobiNet use fd to indicate client ID, so type of
		     // qmiclientId must be int
static uint32_t WdsConnectionIPv4Handle = 0;
static uint32_t WdsConnectionIPv6Handle = 0;
static int s_is_cdma = 0;
static int s_hdr_personality = 0;  // 0x01-HRPD, 0x02-eHRPD
static char *qstrcpy(char *to, const char *from)
{  // no __strcpy_chk
    char *save = to;
    for (; (*to = *from) != '\0'; ++from, ++to)
	;
    return (save);
}



typedef USHORT (*CUSTOMQMUX)(PQMUX_MSG pMUXMsg, void *arg);

// To retrieve the ith (Index) TLV
PQMI_TLV_HDR GetTLV(PQCQMUX_MSG_HDR pQMUXMsgHdr, int TLVType)
{
    int TLVFind = 0;
    USHORT Length = le16_to_cpu(pQMUXMsgHdr->Length);
    PQMI_TLV_HDR pTLVHdr = (PQMI_TLV_HDR)(pQMUXMsgHdr + 1);

    while (Length >= sizeof(QMI_TLV_HDR)) {
	TLVFind++;
	if (TLVType > 0x1000) {
	    if ((TLVFind + 0x1000) == TLVType)
		return pTLVHdr;
	} else if (pTLVHdr->TLVType == TLVType) {
	    return pTLVHdr;
	}

	Length -= (le16_to_cpu((pTLVHdr->TLVLength)) + sizeof(QMI_TLV_HDR));
	pTLVHdr = (PQMI_TLV_HDR)(((UCHAR *)pTLVHdr) +
				 le16_to_cpu(pTLVHdr->TLVLength) +
				 sizeof(QMI_TLV_HDR));
    }

    return NULL;
}

static USHORT GetQMUXTransactionId(void)
{
    static int TransactionId = 0;
    if (++TransactionId > 0xFFFF)
	TransactionId = 1;
    return TransactionId;
}

static PQCQMIMSG ComposeQMUXMsg(UCHAR QMIType, USHORT Type,
				CUSTOMQMUX customQmuxMsgFunction, void *arg)
{
    UCHAR QMIBuf[WDM_DEFAULT_BUFSIZE];
    PQCQMIMSG pRequest = (PQCQMIMSG)QMIBuf;
    int Length;

    memset(QMIBuf, 0x00, sizeof(QMIBuf));
    pRequest->QMIHdr.IFType = USB_CTL_MSG_TYPE_QMI;
    pRequest->QMIHdr.CtlFlags = 0x00;
    pRequest->QMIHdr.QMIType = QMIType;
    pRequest->QMIHdr.ClientId = qmiclientId[QMIType] & 0xFF;

    if (qmiclientId[QMIType] == 0) {
	dbg_time("QMIType %d has no clientID", QMIType);
	return NULL;
    }

    pRequest->MUXMsg.QMUXHdr.CtlFlags =
	QMUX_CTL_FLAG_SINGLE_MSG | QMUX_CTL_FLAG_TYPE_CMD;
    pRequest->MUXMsg.QMUXHdr.TransactionId =
	cpu_to_le16(GetQMUXTransactionId());
    pRequest->MUXMsg.QMUXMsgHdr.Type = cpu_to_le16(Type);
    if (customQmuxMsgFunction)
	pRequest->MUXMsg.QMUXMsgHdr.Length =
	    cpu_to_le16(customQmuxMsgFunction(&pRequest->MUXMsg, arg) -
			sizeof(QCQMUX_MSG_HDR));
    else
	pRequest->MUXMsg.QMUXMsgHdr.Length = cpu_to_le16(0x0000);

    pRequest->QMIHdr.Length = cpu_to_le16(
	le16_to_cpu(pRequest->MUXMsg.QMUXMsgHdr.Length) +
	sizeof(QCQMUX_MSG_HDR) + sizeof(QCQMUX_HDR) + sizeof(QCQMI_HDR) - 1);
    Length = le16_to_cpu(pRequest->QMIHdr.Length) + 1;

    pRequest = (PQCQMIMSG)malloc(Length);
    if (pRequest == NULL) {
	dbg_time("%s fail to malloc", __func__);
    } else {
	memcpy(pRequest, QMIBuf, Length);
    }

    return pRequest;
}

static USHORT WdsStartNwInterfaceReq(PQMUX_MSG pMUXMsg, void *arg)
{
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

    if (s_is_cdma && (profile_user == NULL || profile_user[0] == '\0') &&
	(profile_password == NULL || profile_password[0] == '\0')) {
	profile_user = "ctnet@mycdma.cn";
	profile_password = "vnet.mobi";
	profile_auth = 2;  // chap
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
    TLVLength += (le16_to_cpu(pTechPref->TLVLength) + sizeof(QCQMICTL_TLV_HDR));

    // Set APN Name
    if (profile->apn && !s_is_cdma) {  // cdma no apn
	pApnName = (PQMIWDS_APNNAME)(pTLV + TLVLength);
	pApnName->TLVType = 0x14;
	pApnName->TLVLength = cpu_to_le16(strlen(profile->apn));
	qstrcpy((char *)&pApnName->ApnName, profile->apn);
	TLVLength +=
	    (le16_to_cpu(pApnName->TLVLength) + sizeof(QCQMICTL_TLV_HDR));
    }

    // Set User Name
    if (profile_user) {
	pUserName = (PQMIWDS_USERNAME)(pTLV + TLVLength);
	pUserName->TLVType = 0x17;
	pUserName->TLVLength = cpu_to_le16(strlen(profile_user));
	qstrcpy((char *)&pUserName->UserName, profile_user);
	TLVLength +=
	    (le16_to_cpu(pUserName->TLVLength) + sizeof(QCQMICTL_TLV_HDR));
    }

    // Set Password
    if (profile_password) {
	pPasswd = (PQMIWDS_PASSWD)(pTLV + TLVLength);
	pPasswd->TLVType = 0x18;
	pPasswd->TLVLength = cpu_to_le16(strlen(profile_password));
	qstrcpy((char *)&pPasswd->Passwd, profile_password);
	TLVLength +=
	    (le16_to_cpu(pPasswd->TLVLength) + sizeof(QCQMICTL_TLV_HDR));
    }

    // Set Auth Protocol
    if (profile_user && profile_password) {
	pAuthPref = (PQMIWDS_AUTH_PREFERENCE)(pTLV + TLVLength);
	pAuthPref->TLVType = 0x16;
	pAuthPref->TLVLength = cpu_to_le16(0x01);
	pAuthPref->AuthPreference =
	    profile_auth;  // 0 ~ None, 1 ~ Pap, 2 ~ Chap, 3 ~ MsChapV2
	TLVLength +=
	    (le16_to_cpu(pAuthPref->TLVLength) + sizeof(QCQMICTL_TLV_HDR));
    }

    // Add IP Family Preference
    pIpFamily = (PQMIWDS_IP_FAMILY_TLV)(pTLV + TLVLength);
    pIpFamily->TLVType = 0x19;
    pIpFamily->TLVLength = cpu_to_le16(0x01);
    pIpFamily->IpFamily = profile->curIpFamily;
    TLVLength += (le16_to_cpu(pIpFamily->TLVLength) + sizeof(QCQMICTL_TLV_HDR));

    // Set Profile Index
    if (profile->pdp && !s_is_cdma) {  // cdma only support one pdp, so no need
				       // to set profile index
	PQMIWDS_PROFILE_IDENTIFIER pProfileIndex =
	    (PQMIWDS_PROFILE_IDENTIFIER)(pTLV + TLVLength);
	pProfileIndex->TLVLength = cpu_to_le16(0x01);
	pProfileIndex->TLVType = 0x31;
	pProfileIndex->ProfileIndex = profile->pdpindex;
	if (s_is_cdma && s_hdr_personality == 0x02) {
	    pProfileIndex->TLVType = 0x32;  // profile_index_3gpp2
	    pProfileIndex->ProfileIndex = 101;
	}
	TLVLength +=
	    (le16_to_cpu(pProfileIndex->TLVLength) + sizeof(QCQMICTL_TLV_HDR));
    }

    return sizeof(QMIWDS_START_NETWORK_INTERFACE_REQ_MSG) + TLVLength;
}

static USHORT WdsStopNwInterfaceReq(PQMUX_MSG pMUXMsg, void *arg)
{
    pMUXMsg->StopNwInterfaceReq.TLVType = 0x01;
    pMUXMsg->StopNwInterfaceReq.TLVLength = cpu_to_le16(0x04);
    if (*((int *)arg) == IpFamilyV4)
	pMUXMsg->StopNwInterfaceReq.Handle =
	    cpu_to_le32(WdsConnectionIPv4Handle);
    else
	pMUXMsg->StopNwInterfaceReq.Handle =
	    cpu_to_le32(WdsConnectionIPv6Handle);
    return sizeof(QMIWDS_STOP_NETWORK_INTERFACE_REQ_MSG);
}

static USHORT WdsSetClientIPFamilyPref(PQMUX_MSG pMUXMsg, void *arg)
{
    pMUXMsg->SetClientIpFamilyPrefReq.TLVType = 0x01;
    pMUXMsg->SetClientIpFamilyPrefReq.TLVLength = cpu_to_le16(0x01);
    pMUXMsg->SetClientIpFamilyPrefReq.IpPreference = *((UCHAR *)arg);
    return sizeof(QMIWDS_SET_CLIENT_IP_FAMILY_PREF_REQ_MSG);
}

static USHORT WdsSetAutoConnect(PQMUX_MSG pMUXMsg, void *arg)
{
    pMUXMsg->SetAutoConnectReq.TLVType = 0x01;
    pMUXMsg->SetAutoConnectReq.TLVLength = cpu_to_le16(0x01);
    pMUXMsg->SetAutoConnectReq.autoconnect_setting = *((UCHAR *)arg);
    return sizeof(QMIWDS_SET_AUTO_CONNECT_REQ_MSG);
}

enum peripheral_ep_type {
    DATA_EP_TYPE_RESERVED = 0x0,
    DATA_EP_TYPE_HSIC = 0x1,
    DATA_EP_TYPE_HSUSB = 0x2,
    DATA_EP_TYPE_PCIE = 0x3,
    DATA_EP_TYPE_EMBEDDED = 0x4,
    DATA_EP_TYPE_BAM_DMUX = 0x5,
};

typedef struct {
    UINT rx_urb_size;
    enum peripheral_ep_type ep_type;
    UINT iface_id;
    UCHAR MuxId;
} QMAP_SETTING;
static USHORT WdsSetQMUXBindMuxDataPort(PQMUX_MSG pMUXMsg, void *arg)
{
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
    pMUXMsg->BindMuxDataPortReq.client_type =
	cpu_to_le32(1);	 // WDS_CLIENT_TYPE_TETHERED

    return sizeof(QMIWDS_BIND_MUX_DATA_PORT_REQ_MSG);
}

static USHORT WdaSetDataFormat(PQMUX_MSG pMUXMsg, void *arg)
{
    QMAP_SETTING *qmap_settings = (QMAP_SETTING *)arg;

    if (qmap_settings->rx_urb_size == 0) {
	PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV_QOS pWdsAdminQosTlv;
	PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV linkProto;
	PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV dlTlp;

	pWdsAdminQosTlv =
	    (PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV_QOS)(&pMUXMsg->QMUXMsgHdr + 1);
	pWdsAdminQosTlv->TLVType = 0x10;
	pWdsAdminQosTlv->TLVLength = cpu_to_le16(0x0001);
	pWdsAdminQosTlv->QOSSetting = 0; /* no-QOS header */

	linkProto = (PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV)(pWdsAdminQosTlv + 1);
	linkProto->TLVType = 0x11;
	linkProto->TLVLength = cpu_to_le16(4);
	linkProto->Value = cpu_to_le32(0x01); /* Set Ethernet  mode */

	dlTlp = (PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV)(linkProto + 1);
	;
	dlTlp->TLVType = 0x13;
	dlTlp->TLVLength = cpu_to_le16(4);
	dlTlp->Value = cpu_to_le32(0x00);

	if (sizeof(*linkProto) != 7)
	    dbg_time("%s sizeof(*linkProto) = %d, is not 7!", __func__,
		     sizeof(*linkProto));

	return sizeof(QCQMUX_MSG_HDR) + sizeof(*pWdsAdminQosTlv) +
	       sizeof(*linkProto) + sizeof(*dlTlp);
    } else {
	// Indicates whether the Quality of Service(QOS) data format is used by
	// the client.
	pMUXMsg->SetDataFormatReq.QosDataFormatTlv.TLVType = 0x10;
	pMUXMsg->SetDataFormatReq.QosDataFormatTlv.TLVLength =
	    cpu_to_le16(0x0001);
	pMUXMsg->SetDataFormatReq.QosDataFormatTlv.QOSSetting =
	    0; /* no-QOS header */
	       // Underlying Link Layer Protocol
	pMUXMsg->SetDataFormatReq.UnderlyingLinkLayerProtocolTlv.TLVType = 0x11;
	pMUXMsg->SetDataFormatReq.UnderlyingLinkLayerProtocolTlv.TLVLength =
	    cpu_to_le16(4);
	pMUXMsg->SetDataFormatReq.UnderlyingLinkLayerProtocolTlv.Value =
	    cpu_to_le32(0x02); /* Set IP  mode */
	// Uplink (UL) data aggregation protocol to be used for uplink data
	// transfer.
	pMUXMsg->SetDataFormatReq.UplinkDataAggregationProtocolTlv.TLVType =
	    0x12;
	pMUXMsg->SetDataFormatReq.UplinkDataAggregationProtocolTlv.TLVLength =
	    cpu_to_le16(4);
	pMUXMsg->SetDataFormatReq.UplinkDataAggregationProtocolTlv.Value =
	    cpu_to_le32(0x05);	// UL QMAP is enabled
	// Downlink (DL) data aggregation protocol to be used for downlink data
	// transfer
	pMUXMsg->SetDataFormatReq.DownlinkDataAggregationProtocolTlv.TLVType =
	    0x13;
	pMUXMsg->SetDataFormatReq.DownlinkDataAggregationProtocolTlv.TLVLength =
	    cpu_to_le16(4);
	pMUXMsg->SetDataFormatReq.DownlinkDataAggregationProtocolTlv.Value =
	    cpu_to_le32(0x05);	// UL QMAP is enabled
	// Maximum number of datagrams in a single aggregated packet on downlink
	pMUXMsg->SetDataFormatReq.DownlinkDataAggregationMaxDatagramsTlv
	    .TLVType = 0x15;
	pMUXMsg->SetDataFormatReq.DownlinkDataAggregationMaxDatagramsTlv
	    .TLVLength = cpu_to_le16(4);
	pMUXMsg->SetDataFormatReq.DownlinkDataAggregationMaxDatagramsTlv.Value =
	    cpu_to_le32(qmap_settings->rx_urb_size / 512);
	// Maximum size in bytes of a single aggregated packet allowed on
	// downlink
	pMUXMsg->SetDataFormatReq.DownlinkDataAggregationMaxSizeTlv.TLVType =
	    0x16;
	pMUXMsg->SetDataFormatReq.DownlinkDataAggregationMaxSizeTlv.TLVLength =
	    cpu_to_le16(4);
	pMUXMsg->SetDataFormatReq.DownlinkDataAggregationMaxSizeTlv.Value =
	    cpu_to_le32(qmap_settings->rx_urb_size);
	// Peripheral End Point ID
	pMUXMsg->SetDataFormatReq.epTlv.TLVType = 0x17;
	pMUXMsg->SetDataFormatReq.epTlv.TLVLength = cpu_to_le16(8);
	pMUXMsg->SetDataFormatReq.epTlv.ep_type =
	    cpu_to_le32(qmap_settings->ep_type);
	pMUXMsg->SetDataFormatReq.epTlv.iface_id =
	    cpu_to_le32(qmap_settings->iface_id);

	return sizeof(QMIWDS_ADMIN_SET_DATA_FORMAT_REQ_MSG);
    }
}

#ifdef CONFIG_SIM

static USHORT UimVerifyPinReqSend(PQMUX_MSG pMUXMsg, void *arg)
{
    pMUXMsg->UIMUIMVerifyPinReq.TLVType = 0x01;
    pMUXMsg->UIMUIMVerifyPinReq.TLVLength = cpu_to_le16(0x02);
    pMUXMsg->UIMUIMVerifyPinReq.Session_Type = 0x00;
    pMUXMsg->UIMUIMVerifyPinReq.Aid_Len = 0x00;
    pMUXMsg->UIMUIMVerifyPinReq.TLV2Type = 0x02;
    pMUXMsg->UIMUIMVerifyPinReq.TLV2Length =
	cpu_to_le16(2 + strlen((const char *)arg));
    pMUXMsg->UIMUIMVerifyPinReq.PINID = 0x01;  // Pin1, not Puk
    pMUXMsg->UIMUIMVerifyPinReq.PINLen = strlen((const char *)arg);
    qstrcpy((PCHAR)&pMUXMsg->UIMUIMVerifyPinReq.PINValue, ((const char *)arg));
    return sizeof(QMIUIM_VERIFY_PIN_REQ_MSG) + (strlen((const char *)arg) - 1);
}
//2021-03-24 willa.liu@fibocom.com changed begin for support mantis 0071817
static USHORT UimBindPinReqSend_NAS_WMS(PQMUX_MSG pMUXMsg, void *arg)
{
    pMUXMsg->UIMUIMBindPinReq.TLVType = 0x01;
    pMUXMsg->UIMUIMBindPinReq.TLVLength = cpu_to_le16(0x01);
    pMUXMsg->UIMUIMBindPinReq.Value = 0x01;
    return sizeof(QMIUIM_BIND_PIN_REQ_MSG);
}

static USHORT UimBindPinReqSend_WDS_DMS_QOS(PQMUX_MSG pMUXMsg, void *arg)
{
    pMUXMsg->UIMUIMBindPinReq_4.TLVType = 0x01;
    pMUXMsg->UIMUIMBindPinReq_4.TLVLength = cpu_to_le16(0x04);
    pMUXMsg->UIMUIMBindPinReq_4.Value = 0x0002;
    return sizeof(QMIUIM_BIND_PIN_REQ_MSG_4);
}
//2021-03-24 willa.liu@fibocom.com changed end for support mantis 0071817
#ifdef CONFIG_IMSI_ICCID
static USHORT UimReadTransparentIMSIReqSend(PQMUX_MSG pMUXMsg, void *arg)
{
    PREAD_TRANSPARENT_TLV pReadTransparent;

    pMUXMsg->UIMUIMReadTransparentReq.TLVType = 0x01;
    pMUXMsg->UIMUIMReadTransparentReq.TLVLength = cpu_to_le16(0x02);
    if (!strcmp((char *)arg, "EF_ICCID")) {
	pMUXMsg->UIMUIMReadTransparentReq.Session_Type = 0x06;
	pMUXMsg->UIMUIMReadTransparentReq.Aid_Len = 0x00;

	pMUXMsg->UIMUIMReadTransparentReq.TLV2Type = 0x02;
	pMUXMsg->UIMUIMReadTransparentReq.file_id = cpu_to_le16(0x2FE2);
	pMUXMsg->UIMUIMReadTransparentReq.path_len = 0x02;
	pMUXMsg->UIMUIMReadTransparentReq.path[0] = 0x00;
	pMUXMsg->UIMUIMReadTransparentReq.path[1] = 0x3F;
    } else if (!strcmp((char *)arg, "EF_IMSI")) {
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

    pMUXMsg->UIMUIMReadTransparentReq.TLV2Length =
	cpu_to_le16(3 + pMUXMsg->UIMUIMReadTransparentReq.path_len);

    pReadTransparent = (PREAD_TRANSPARENT_TLV)(
	&pMUXMsg->UIMUIMReadTransparentReq
	     .path[pMUXMsg->UIMUIMReadTransparentReq.path_len]);
    pReadTransparent->TLVType = 0x03;
    pReadTransparent->TLVLength = cpu_to_le16(0x04);
    pReadTransparent->Offset = cpu_to_le16(0x00);
    pReadTransparent->Length = cpu_to_le16(0x00);

    return (sizeof(QMIUIM_READ_TRANSPARENT_REQ_MSG) +
	    pMUXMsg->UIMUIMReadTransparentReq.path_len +
	    sizeof(READ_TRANSPARENT_TLV));
}
#endif
#endif

#ifdef CONFIG_APN
static USHORT WdsGetProfileSettingsReqSend(PQMUX_MSG pMUXMsg, void *arg)
{
    PROFILE_T *profile = (PROFILE_T *)arg;
    pMUXMsg->GetProfileSettingsReq.Length =
	cpu_to_le16(sizeof(QMIWDS_GET_PROFILE_SETTINGS_REQ_MSG) - 4);
    pMUXMsg->GetProfileSettingsReq.TLVType = 0x01;
    pMUXMsg->GetProfileSettingsReq.TLVLength = cpu_to_le16(0x02);
    pMUXMsg->GetProfileSettingsReq.ProfileType = 0x00;	// 0 ~ 3GPP, 1 ~ 3GPP2
    pMUXMsg->GetProfileSettingsReq.ProfileIndex = profile->pdpindex;
    return sizeof(QMIWDS_GET_PROFILE_SETTINGS_REQ_MSG);
}
//begin modified by zhangkaibo add create profile qmi. mantis 0049137,0048741  20200610 
static USHORT WdsCreateProfileSettingsReq(PQMUX_MSG pMUXMsg, void *arg)
{
    USHORT TLVLength = 0;
    UCHAR *pTLV;
    PROFILE_T *profile = (PROFILE_T *)arg;
    PQMIWDS_PDPTYPE pPdpType;

    pMUXMsg->CreateProfileSettingsReq.Length =
	cpu_to_le16(sizeof(QMIWDS_CREATE_PROFILE_SETTINGS_REQ_MSG) - 4);
    pMUXMsg->CreateProfileSettingsReq.TLVType = 0x01;
    pMUXMsg->CreateProfileSettingsReq.TLVLength = cpu_to_le16(0x01);
    pMUXMsg->CreateProfileSettingsReq.ProfileType =
	0x00;  // 0 ~ 3GPP, 1 ~ 3GPP2

    pTLV = (UCHAR *)(&pMUXMsg->CreateProfileSettingsReq + 1);

    pPdpType = (PQMIWDS_PDPTYPE)(pTLV + TLVLength);
    pPdpType->TLVType = 0x11;
    pPdpType->TLVLength = cpu_to_le16(0x01);
    // 0 ?C PDP-IP (IPv4)
    // 1 ?C PDP-PPP
    // 2 ?C PDP-IPv6
    // 3 ?C PDP-IPv4v6
    if (profile->ipv4_flag && profile->ipv6_flag)
	pPdpType->PdpType = 3;
    else if (profile->ipv6_flag)
	pPdpType->PdpType = 2;
    else
	pPdpType->PdpType = 0;
    TLVLength += (le16_to_cpu(pPdpType->TLVLength) + sizeof(QCQMICTL_TLV_HDR));

    // Set APN Name
    if (profile->apn) {
	PQMIWDS_APNNAME pApnName = (PQMIWDS_APNNAME)(pTLV + TLVLength);
	pApnName->TLVType = 0x14;
	pApnName->TLVLength = cpu_to_le16(strlen(profile->apn));
	qstrcpy((char *)&pApnName->ApnName, profile->apn);
	TLVLength +=
	    (le16_to_cpu(pApnName->TLVLength) + sizeof(QCQMICTL_TLV_HDR));
    }

    // Set User Name
    if (profile->user) {
	PQMIWDS_USERNAME pUserName = (PQMIWDS_USERNAME)(pTLV + TLVLength);
	pUserName->TLVType = 0x1B;

	pUserName->TLVLength = cpu_to_le16(strlen(profile->user));
	qstrcpy((char *)&pUserName->UserName, profile->user);
	TLVLength +=
	    (le16_to_cpu(pUserName->TLVLength) + sizeof(QCQMICTL_TLV_HDR));
    }

    // Set Password
    if (profile->password) {
	PQMIWDS_PASSWD pPasswd = (PQMIWDS_PASSWD)(pTLV + TLVLength);
	pPasswd->TLVType = 0x1C;
	pPasswd->TLVLength = cpu_to_le16(strlen(profile->password));
	qstrcpy((char *)&pPasswd->Passwd, profile->password);
	TLVLength +=
	    (le16_to_cpu(pPasswd->TLVLength) + sizeof(QCQMICTL_TLV_HDR));
    }

    // Set Auth Protocol
    if (profile->user && profile->password) {
	PQMIWDS_AUTH_PREFERENCE pAuthPref =
	    (PQMIWDS_AUTH_PREFERENCE)(pTLV + TLVLength);
	pAuthPref->TLVType = 0x1D;
	pAuthPref->TLVLength = cpu_to_le16(0x01);
	pAuthPref->AuthPreference =
	    profile->auth;  // 0 ~ None, 1 ~ Pap, 2 ~ Chap, 3 ~ MsChapV2
	TLVLength +=
	    (le16_to_cpu(pAuthPref->TLVLength) + sizeof(QCQMICTL_TLV_HDR));
    }

    return sizeof(QMIWDS_CREATE_PROFILE_SETTINGS_REQ_MSG) + TLVLength;
}
//end modified by zhangkaibo add create profile qmi. mantis 0049137,0048741  20200610
static USHORT WdsModifyProfileSettingsReq(PQMUX_MSG pMUXMsg, void *arg)
{
    USHORT TLVLength = 0;
    UCHAR *pTLV;
    PROFILE_T *profile = (PROFILE_T *)arg;
    PQMIWDS_PDPTYPE pPdpType;

    pMUXMsg->ModifyProfileSettingsReq.Length =
	cpu_to_le16(sizeof(QMIWDS_MODIFY_PROFILE_SETTINGS_REQ_MSG) - 4);
    pMUXMsg->ModifyProfileSettingsReq.TLVType = 0x01;
    pMUXMsg->ModifyProfileSettingsReq.TLVLength = cpu_to_le16(0x02);
    pMUXMsg->ModifyProfileSettingsReq.ProfileType =
	0x00;  // 0 ~ 3GPP, 1 ~ 3GPP2
    pMUXMsg->ModifyProfileSettingsReq.ProfileIndex = profile->pdpindex;

    pTLV = (UCHAR *)(&pMUXMsg->ModifyProfileSettingsReq + 1);

    pPdpType = (PQMIWDS_PDPTYPE)(pTLV + TLVLength);
    pPdpType->TLVType = 0x11;
    pPdpType->TLVLength = cpu_to_le16(0x01);
    // 0 ?C PDP-IP (IPv4)
    // 1 ?C PDP-PPP
    // 2 ?C PDP-IPv6
    // 3 ?C PDP-IPv4v6
    if (profile->ipv6_flag && profile->ipv4_flag)
	pPdpType->PdpType = 3;
    else if (profile->ipv6_flag)
	pPdpType->PdpType = 2;
    else
	pPdpType->PdpType = 0;
    TLVLength += (le16_to_cpu(pPdpType->TLVLength) + sizeof(QCQMICTL_TLV_HDR));

    // Set APN Name
    if (profile->apn) {
	PQMIWDS_APNNAME pApnName = (PQMIWDS_APNNAME)(pTLV + TLVLength);
	pApnName->TLVType = 0x14;
	pApnName->TLVLength = cpu_to_le16(strlen(profile->apn));
	qstrcpy((char *)&pApnName->ApnName, profile->apn);
	TLVLength +=
	    (le16_to_cpu(pApnName->TLVLength) + sizeof(QCQMICTL_TLV_HDR));
    }

    // Set User Name
    if (profile->user) {
	PQMIWDS_USERNAME pUserName = (PQMIWDS_USERNAME)(pTLV + TLVLength);
	pUserName->TLVType = 0x1B;
	pUserName->TLVLength = cpu_to_le16(strlen(profile->user));
	qstrcpy((char *)&pUserName->UserName, profile->user);
	TLVLength +=
	    (le16_to_cpu(pUserName->TLVLength) + sizeof(QCQMICTL_TLV_HDR));
    }

    // Set Password
    if (profile->password) {
	PQMIWDS_PASSWD pPasswd = (PQMIWDS_PASSWD)(pTLV + TLVLength);
	pPasswd->TLVType = 0x1C;
	pPasswd->TLVLength = cpu_to_le16(strlen(profile->password));
	qstrcpy((char *)&pPasswd->Passwd, profile->password);
	TLVLength +=
	    (le16_to_cpu(pPasswd->TLVLength) + sizeof(QCQMICTL_TLV_HDR));
    }

    // Set Auth Protocol
    if (profile->user && profile->password) {
	PQMIWDS_AUTH_PREFERENCE pAuthPref =
	    (PQMIWDS_AUTH_PREFERENCE)(pTLV + TLVLength);
	pAuthPref->TLVType = 0x1D;
	pAuthPref->TLVLength = cpu_to_le16(0x01);
	pAuthPref->AuthPreference =
	    profile->auth;  // 0 ~ None, 1 ~ Pap, 2 ~ Chap, 3 ~ MsChapV2
	TLVLength +=
	    (le16_to_cpu(pAuthPref->TLVLength) + sizeof(QCQMICTL_TLV_HDR));
    }

    return sizeof(QMIWDS_MODIFY_PROFILE_SETTINGS_REQ_MSG) + TLVLength;
}
#endif

static USHORT WdsGetRuntimeSettingReq(PQMUX_MSG pMUXMsg, void *arg)
{
    pMUXMsg->GetRuntimeSettingsReq.TLVType = 0x10;
    pMUXMsg->GetRuntimeSettingsReq.TLVLength = cpu_to_le16(0x04);
    // the following mask also applies to IPV6
    pMUXMsg->GetRuntimeSettingsReq.Mask =
	cpu_to_le32(QMIWDS_GET_RUNTIME_SETTINGS_MASK_IPV4DNS_ADDR |
		    QMIWDS_GET_RUNTIME_SETTINGS_MASK_IPV4_ADDR |
		    QMIWDS_GET_RUNTIME_SETTINGS_MASK_MTU |
		    QMIWDS_GET_RUNTIME_SETTINGS_MASK_IPV4GATEWAY_ADDR);	 // |
    // QMIWDS_GET_RUNTIME_SETTINGS_MASK_PCSCF_SV_ADDR |
    // QMIWDS_GET_RUNTIME_SETTINGS_MASK_PCSCF_DOM_NAME;

    return sizeof(QMIWDS_GET_RUNTIME_SETTINGS_REQ_MSG);
}

static PQCQMIMSG s_pRequest;
static PQCQMIMSG s_pResponse;
static pthread_mutex_t s_commandmutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_commandcond = PTHREAD_COND_INITIALIZER;

static int is_response(const PQCQMIMSG pRequest, const PQCQMIMSG pResponse)
{
    if ((pRequest->QMIHdr.QMIType == pResponse->QMIHdr.QMIType) &&
	(pRequest->QMIHdr.ClientId == pResponse->QMIHdr.ClientId)) {
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

int QmiThreadSendQMITimeout(PQCQMIMSG pRequest, PQCQMIMSG *ppResponse,
			    unsigned msecs)
{
    int ret;

    if (!pRequest) {
	return -EINVAL;
    }

    pthread_mutex_lock(&s_commandmutex);

    if (ppResponse)
	*ppResponse = NULL;

    dump_qmi(pRequest, le16_to_cpu(pRequest->QMIHdr.Length) + 1);

    s_pRequest = pRequest;
    s_pResponse = NULL;

    ret = qmidev_send(pRequest);

    if (ret == 0) {
	ret = pthread_cond_timeout_np(&s_commandcond, &s_commandmutex, msecs);
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
	    dbg_time("%s pthread_cond_timeout_np=%d, errno: %d (%s)", __func__,
		     ret, errno, strerror(errno));
	}
    }

    pthread_mutex_unlock(&s_commandmutex);

    return ret;
}

int QmiThreadSendQMI(PQCQMIMSG pRequest, PQCQMIMSG *ppResponse)
{
    return QmiThreadSendQMITimeout(pRequest, ppResponse, 30 * 1000);
}

void QmiThreadRecvQMI(PQCQMIMSG pResponse)
{
    pthread_mutex_lock(&s_commandmutex);
    if (pResponse == NULL) {
	if (s_pRequest) {
	    free(s_pRequest);
	    s_pRequest = NULL;
	    s_pResponse = NULL;
	    pthread_cond_signal(&s_commandcond);
	}
	pthread_mutex_unlock(&s_commandmutex);
	return;
    }
    dump_qmi(pResponse, le16_to_cpu(pResponse->QMIHdr.Length) + 1);
    if (s_pRequest && is_response(s_pRequest, pResponse)) {
	free(s_pRequest);
	s_pRequest = NULL;
	s_pResponse = malloc(le16_to_cpu(pResponse->QMIHdr.Length) + 1);
	if (s_pResponse != NULL) {
	    memcpy(s_pResponse, pResponse,
		   le16_to_cpu(pResponse->QMIHdr.Length) + 1);
	}
	pthread_cond_signal(&s_commandcond);
    } else if ((pResponse->QMIHdr.QMIType == QMUX_TYPE_NAS) &&
	       (le16_to_cpu(pResponse->MUXMsg.QMUXMsgHdrResp.Type) ==
		QMINAS_SERVING_SYSTEM_IND)) {
	qmidevice_send_event_to_main(
	    RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED);
    } else if ((pResponse->QMIHdr.QMIType == QMUX_TYPE_WDS) &&
	       (le16_to_cpu(pResponse->MUXMsg.QMUXMsgHdrResp.Type) ==
		QMIWDS_GET_PKT_SRVC_STATUS_IND)) {
	qmidevice_send_event_to_main(RIL_UNSOL_DATA_CALL_LIST_CHANGED);
    } else if ((pResponse->QMIHdr.QMIType == QMUX_TYPE_WDS_IPV6) &&
	       (le16_to_cpu(pResponse->MUXMsg.QMUXMsgHdrResp.Type) ==
		QMIWDS_GET_PKT_SRVC_STATUS_IND)) {
	qmidevice_send_event_to_main(RIL_UNSOL_DATA_CALL_LIST_CHANGED);
    } else if ((pResponse->QMIHdr.QMIType == QMUX_TYPE_NAS) &&
	       (le16_to_cpu(pResponse->MUXMsg.QMUXMsgHdrResp.Type) ==
		QMINAS_SYS_INFO_IND)) {
	qmidevice_send_event_to_main(
	    RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED);
    } else {
	if (debug_qmi)
	    dbg_time("nobody care this qmi msg!!");
    }
    pthread_mutex_unlock(&s_commandmutex);
}

int requestSetEthMode(PROFILE_T *profile)
{
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse = NULL;
    PQMUX_MSG pMUXMsg;
    int err;
    PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV linkProto;
    UCHAR IpPreference;
    UCHAR autoconnect_setting = 0;
    QMAP_SETTING qmap_settings = {0, 0, 0, 0};

    if (profile->qmap_mode) {
	profile->rawIP = 1;
	

	qmap_settings.MuxId = profile->muxid;

	if (qmidev_is_pciemhi(profile->qmichannel)) {  // SDX20_PCIE
            qmap_settings.rx_urb_size = 32*1024; //SDX24&SDX55 support 32KB 
	    qmap_settings.ep_type = DATA_EP_TYPE_PCIE;
	    qmap_settings.iface_id = 0x04;
	} else {  // for MDM9x07&MDM9x40&SDX20 USB
	    qmap_settings.rx_urb_size = profile->qmap_size;
		//profile->qmap_size;  // SDX24&SDX55 support 32KB
	    qmap_settings.ep_type = DATA_EP_TYPE_HSUSB;
//2021-02-08 zhangkaibo@fibocom.com changed begin for mantis 0070613
	    qmap_settings.iface_id = profile->interfacenum;
//2021-02-08 zhangkaibo@fibocom.com changed end for mantis 0070613
	}

	if (qmidev_is_gobinet(
		profile->qmichannel)) {	 // GobiNet set data format
					 // in GobiNet driver
	    goto skip_WdaSetDataFormat;
	} else if (profile->qmap_mode >
		   1) {	 // QMAP MUX enabled, set data format in fibo-qmi-proxy

//2021-06-03 willa.liu@fibocom.com changed begin for support mantis 0079541
//2021-01-27 willa.liu@fibocom.com changed begin for support mantis 0068849
        //goto skip_WdaSetDataFormat;
		goto skip_WdaSetDataFormat;
//2021-01-27 willa.liu@fibocom.com changed end for support mantis 0068849
//2021-06-03 willa.liu@fibocom.com changed end for support mantis 0079541

	}
    }

    pRequest =
	ComposeQMUXMsg(QMUX_TYPE_WDS_ADMIN, QMIWDS_ADMIN_SET_DATA_FORMAT_REQ,
		       WdaSetDataFormat, (void *)&qmap_settings);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    linkProto = (PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV)GetTLV(
	&pResponse->MUXMsg.QMUXMsgHdr, 0x11);
    if (linkProto != NULL) {
	profile->rawIP = (le32_to_cpu(linkProto->Value) == 2);
     }

    linkProto = (PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV)GetTLV(
	&pResponse->MUXMsg.QMUXMsgHdr, 0x16);
    if (linkProto != NULL && profile->qmap_mode) {
	qmap_settings.rx_urb_size = le32_to_cpu(linkProto->Value);
	dbg_time(
	    "qmap_settings.rx_urb_size = %d",
	    qmap_settings.rx_urb_size);	 // must same as rx_urb_size defined
					 // in GobiNet&qmi_wwan driver
    }

    free(pResponse);

skip_WdaSetDataFormat:
//bind v4 client
	// set ipv4
	if(profile->ipv4_flag)
	{
/*		IpPreference = IpFamilyV4;
		pRequest =
			ComposeQMUXMsg(QMUX_TYPE_WDS, QMIWDS_SET_CLIENT_IP_FAMILY_PREF_REQ,
				   WdsSetClientIPFamilyPref, (void *)&IpPreference);
		err = QmiThreadSendQMI(pRequest, &pResponse);
		if (pResponse)
			free(pResponse);
*/
		if (profile->qmapnet_adapter) {
			// bind wds mux data port
			pRequest = ComposeQMUXMsg(
			QMUX_TYPE_WDS, QMIWDS_BIND_MUX_DATA_PORT_REQ,
			WdsSetQMUXBindMuxDataPort, (void *)&qmap_settings);
			err = QmiThreadSendQMI(pRequest, &pResponse);
			qmi_rsp_check_and_return();
			if (pResponse)
			free(pResponse);
		}
// set auto connect
		pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS, QMIWDS_SET_AUTO_CONNECT_REQ,
					  WdsSetAutoConnect, (void *)&autoconnect_setting);
		QmiThreadSendQMI(pRequest, &pResponse);
		if (pResponse)
		free(pResponse);	
	}
    
//bind v6 client
	if(profile->ipv6_flag)
	{
		IpPreference = IpFamilyV6;
		pRequest = ComposeQMUXMsg(
			QMUX_TYPE_WDS_IPV6, QMIWDS_SET_CLIENT_IP_FAMILY_PREF_REQ,
			WdsSetClientIPFamilyPref, (void *)&IpPreference);
		err = QmiThreadSendQMI(pRequest, &pResponse);
		qmi_rsp_check_and_return();
		if (pResponse)
			free(pResponse);

		if (profile->qmapnet_adapter) {
			// bind wds mux data port
			pRequest = ComposeQMUXMsg(
			QMUX_TYPE_WDS_IPV6, QMIWDS_BIND_MUX_DATA_PORT_REQ,
			WdsSetQMUXBindMuxDataPort, (void *)&qmap_settings);
			err = QmiThreadSendQMI(pRequest, &pResponse);
			qmi_rsp_check_and_return();
			if (pResponse)
			free(pResponse);
		}
		IpPreference = IpFamilyV6;
		pRequest = ComposeQMUXMsg(
			QMUX_TYPE_WDS_IPV6, QMIWDS_SET_CLIENT_IP_FAMILY_PREF_REQ,
			WdsSetClientIPFamilyPref, (void *)&IpPreference);
		err = QmiThreadSendQMI(pRequest, &pResponse);
		qmi_rsp_check_and_return();
		if (pResponse)
			free(pResponse);

// set auto connect
		pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS_IPV6, QMIWDS_SET_AUTO_CONNECT_REQ,
					  WdsSetAutoConnect, (void *)&autoconnect_setting);
		QmiThreadSendQMI(pRequest, &pResponse);
		if (pResponse)
		free(pResponse);
	}
    return 0;
}

#ifdef CONFIG_SIM
int requestGetPINStatus(SIM_Status *pSIMStatus)
{
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    PQMIDMS_UIM_PIN_STATUS pPin1Status = NULL;
    // PQMIDMS_UIM_PIN_STATUS pPin2Status = NULL;
	

	pRequest = ComposeQMUXMsg(QMUX_TYPE_UIM, QMIUIM_GET_CARD_STATUS_REQ,
				  NULL, NULL);

    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    pPin1Status =
	(PQMIDMS_UIM_PIN_STATUS)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x11);
    // pPin2Status =
    // (PQMIDMS_UIM_PIN_STATUS)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x12);

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

static int requestGetSIMMode(PROFILE_T *profile)
{
    int i;
    int fd;
    int ret;
    int len;
    char buffer[1024] = {0};
    char *sendbuffer = "AT+GTDUALSIMEN?";
    int totallen = 0;
    fd_set readfds;
    struct timeval timeout;
    struct termios tiosfd, tio;
    int timeoutVal = 5;
    char *dev = NULL;
    if(qmidev_is_gobinet(profile->qmichannel))
    {
        dev = "/dev/ttyUSB1"; //The port under Linux is operated by opening the device file
    }
	else
    {
        dev = "/dev/mhi_DUN"; //The port under Linux is operated by opening the device file
    }

    fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if(fd < 0)
        goto ERR;
    fcntl(fd, F_SETFL, O_RDWR);
    xget1(fd, &tio, &tiosfd);

    if(xset1(fd, &tio, dev))
        goto ERR;

    tcflush(fd, TCIOFLUSH);

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    sprintf(buffer, "%s\r", sendbuffer);
    ret = write (fd, buffer, strlen(buffer));
    if(ret < 0)
    {
        dbg_time("write failed\n");
        goto ERR;
    }
    dbg_time("dev: %s\nsendbuffer:%s\n", dev, sendbuffer);
    sleep(3);

    while(1)
    {
        timeout.tv_sec = timeoutVal;
        timeout.tv_usec = 0;

        ret = select(fd+1, &readfds, (fd_set *)0, (fd_set *)0, &timeout);
        if(ret > 0)
        {
            ret = read(fd, buffer+totallen, sizeof(buffer)-totallen-1);
            if(ret < 0)
            {
                dbg_time("read failed\n");
                goto ERR;
            }

            if(ret == 0)
            {
                goto ERR;
            }

            totallen += ret;
            buffer[totallen] = '\0';
            dbg_time("read %d %s", ret, &buffer[totallen-ret]);

//current buffer is return value
            if(strstr(buffer, "+GTDUALSIMEN: 1"))
            {
                close(fd);
                dbg_time("read 1\n");
                return 1;
            }
            else
            {
                close(fd);
                dbg_time("read 0/2/3\n");
                return 0;
            }
        }
        else
        {
            dbg_time("select timeout\n");
            goto ERR;
        }

    }

    close(fd);

ERR:
    if(fd > 0)
    close(fd);
    return -1;
}


//2021-03-24 willa.liu@fibocom.com changed begin for support mantis 0071817
int requestGetSIMCardNumber(PROFILE_T *profile)
{
    int i;
    int fd;
    int ret;
    int len;
    char buffer[1024] = {0};
    int rate;
    char *sendbuffer;
    int totallen = 0;
    fd_set readfds;
    struct timeval timeout;
    struct termios tiosfd, tio;
    int timeoutVal = 5;
    char *dev = NULL;
    if(qmidev_is_gobinet(profile->qmichannel))
    {
        dev = "/dev/ttyUSB1"; //The port under Linux is operated by opening the device file
    }
	else
    {
        dev = "/dev/mhi_DUN"; //The port under Linux is operated by opening the device file
    }
    rate = 115200;
    sendbuffer = "AT+GTDUALSIM?";
    if(requestGetSIMMode(profile) != 1)
    {
        return 0;
    }
    fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if(fd < 0)
        goto ERR;
    fcntl(fd, F_SETFL, O_RDWR);
    xget1(fd, &tio, &tiosfd);

    if(xset1(fd, &tio, dev))
        goto ERR;

    tcflush(fd, TCIOFLUSH);

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    sprintf(buffer, "%s\r", sendbuffer);
    ret = write (fd, buffer, strlen(buffer));
    if(ret < 0)
    {
        dbg_time("write failed\n");
        goto ERR;
    }
    dbg_time("dev: %s\nrate:%d\nsendbuffer:%s\n", dev, rate, sendbuffer);
    sleep(3);
    goto read_result;
    while(1)
    {
        timeout.tv_sec = timeoutVal;
        timeout.tv_usec = 0;

        //ret = select(fd+1, &readfds, (fd_set *)0, (fd_set *)0, &timeout);
        //if(ret > 0)
        {
            read_result:
            ret = read(fd, buffer+totallen, sizeof(buffer)-totallen-1);
            if(ret < 0)
            {
                dbg_time("read failed\n");
                goto ERR;
            }

            if(ret == 0)
            {
                goto ERR;
            }

            totallen += ret;
            buffer[totallen] = '\0';
            dbg_time("read %d %s", ret, &buffer[totallen-ret]);

//current buffer is return value
            if(strstr(buffer, "+GTDUALSIM: 0"))
            {
                close(fd);
                dbg_time("read 0\n");
                return 0;
            }
            else if(strstr(buffer, "+GTDUALSIM: 1"))
            {
                close(fd);
                dbg_time("read 1\n");
                return 1;
            }
            else
                continue;
        }
        //else
        {
            //dbg_time("select timeout\n");
            //goto ERR;
        }

    }

    close(fd);

ERR:
    if(fd > 0)
    close(fd);
    return -1;
}

int requestSimBindSubscription_NAS_WMS()
{
    PQCQMIMSG pRequest_nas;
    PQCQMIMSG pResponse_nas;
    int err;

    pRequest_nas = ComposeQMUXMsg(QMUX_TYPE_NAS, QMI_NAS_BIND_SUBSCRIPTION_REQ_V01,
                UimBindPinReqSend_NAS_WMS, NULL);
    err = QmiThreadSendQMI(pRequest_nas, &pResponse_nas);
    free(pResponse_nas);

    return 0;
}

int requestSimBindSubscription_WDS_DMS_QOS()
{
    PQCQMIMSG pRequest_wds;
    PQCQMIMSG pRequest_dms;

    PQCQMIMSG pResponse_wds;
    PQCQMIMSG pResponse_dms;
    int err;

    pRequest_wds = ComposeQMUXMsg(QMUX_TYPE_WDS, QMI_WDS_BIND_SUBSCRIPTION_REQ_V01,
                UimBindPinReqSend_WDS_DMS_QOS, NULL);
    err = QmiThreadSendQMI(pRequest_wds, &pResponse_wds);

    pRequest_dms = ComposeQMUXMsg(QMUX_TYPE_DMS, QMI_DMS_BIND_SUBSCRIPTION_REQ_V01,
                UimBindPinReqSend_WDS_DMS_QOS, NULL);
    err = QmiThreadSendQMI(pRequest_dms, &pResponse_dms);

    free(pResponse_wds);
    free(pResponse_dms);

    return 0;
}

//int requestGetSIMStatus(SIM_Status *pSIMStatus)
int requestGetSIMStatus(SIM_Status *pSIMStatus, const int sim_select)
{  // RIL_REQUEST_GET_SIM_STATUS
    int i;
    dbg_time("sim_select = %d\n", sim_select);
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    const char *SIM_Status_String[] = {
	"SIM_ABSENT",
	"SIM_NOT_READY",
	"SIM_READY", /* SIM_READY means the radio state is RADIO_STATE_SIM_READY
		      */
	"SIM_PIN",
	"SIM_PUK",
	"SIM_NETWORK_PERSONALIZATION"};

 	pRequest = ComposeQMUXMsg(QMUX_TYPE_UIM, QMIUIM_GET_CARD_STATUS_REQ,
				  NULL, NULL); 

    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    *pSIMStatus = SIM_ABSENT; 
	PQMIUIM_CARD_STATUS pCardStatus = NULL;
	PQMIUIM_PIN_STATE pPINState = NULL;
	UCHAR CardState = 0x01;
	UCHAR PIN1State = QMI_PIN_STATUS_NOT_VERIF;
	// UCHAR PIN1Retries;
	// UCHAR PUK1Retries;
	// UCHAR PIN2State;
	// UCHAR PIN2Retries;
	// UCHAR PUK2Retries;

    void *temp = NULL;
    QMIUIM_APP_STATUS * curr_app = NULL;
    Instance * curr_ints = NULL;

	pCardStatus =
	    (PQMIUIM_CARD_STATUS)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x10);
    if(sim_select == 0)
    {
        temp =(void*)pCardStatus + sizeof(QMIUIM_CARD_STATUS);
        curr_ints = temp;
        dbg_time("curr_ints_sim1 %s", curr_ints);
        temp = temp + sizeof(Instance);
        curr_app = temp;
    }
    else
    {
        temp = (void*)pCardStatus + sizeof(QMIUIM_CARD_STATUS);
        temp = temp + sizeof(Instance) + (sizeof(QMIUIM_APP_STATUS)*(((Instance *)temp)->NumApp)); 
        curr_ints = temp;
	dbg_time("curr_ints_sim2 %s", curr_ints);
        curr_app = temp + sizeof(Instance);
    }
/*
	if (pCardStatus != NULL) {

        for (i = 0; i < pCardStatus->NumApp; i++) {
            if (pCardStatus->AppStatus[i].AppType == 2) {
                pPINState = &(pCardStatus->AppStatus[i].PinState);
                break;
            }
        }
        if (i == pCardStatus->NumApp) {
            dbg_time("no USIM Card info");
            return -1;
        }

	    CardState = pCardStatus->CardState;
	    if (pPINState->UnivPIN == 1) {
		PIN1State = pCardStatus->UPINState;
*/
	if (curr_ints != NULL) {
        dbg_time("curr_ints->CardState is %d", curr_ints->CardState);
        dbg_time("curr_ints->NumApp is %d", curr_ints->NumApp);
        for (i = 0; i < curr_ints->NumApp; i++) {
            dbg_time("AppType = %d\n", curr_app[i].AppType);
            if (curr_app[i].AppType == 2) {
                pPINState = &(curr_app[i].PinState);
                break;
            }
        }
        if (i == curr_ints->NumApp) {
            dbg_time("no USIM Card info");
            return -1;
        }

	    //CardState = pCardStatus->CardState;
	    CardState = curr_ints->CardState;
	    if (pPINState->UnivPIN == 1) {
		PIN1State = curr_ints->UPINState;
		// PIN1Retries = pCardStatus->UPINRetries;
		// PUK1Retries = pCardStatus->UPUKRetries;
	    } else {
		PIN1State = pPINState->PIN1State;
		// PIN1Retries = pPINState->PIN1Retries;
		// PUK1Retries = pPINState->PUK1Retries;
	    }
	    // PIN2State = pPINState->PIN2State;
	    // PIN2Retries = pPINState->PIN2Retries;
	    // PUK2Retries = pPINState->PUK2Retries;
	}
//2021-03-24 willa.liu@fibocom.com changed end for support mantis 0071817
	*pSIMStatus = SIM_ABSENT;
	if ((CardState == 0x01) && ((PIN1State == QMI_PIN_STATUS_VERIFIED) ||
				    (PIN1State == QMI_PIN_STATUS_DISABLED))) {
	    *pSIMStatus = SIM_READY;
	} else if (CardState == 0x01) {
	    if (PIN1State == QMI_PIN_STATUS_NOT_VERIF) {
		*pSIMStatus = SIM_PIN;
	    }
	    if (PIN1State == QMI_PIN_STATUS_BLOCKED) {
		*pSIMStatus = SIM_PUK;
	    } else if (PIN1State == QMI_PIN_STATUS_PERM_BLOCKED) {
		*pSIMStatus = SIM_BAD;
	    } else if (PIN1State == QMI_PIN_STATUS_NOT_INIT ||
		       PIN1State == QMI_PIN_STATUS_VERIFIED ||
		       PIN1State == QMI_PIN_STATUS_DISABLED) {
		*pSIMStatus = SIM_READY;
	    }
	} else if (CardState == 0x00 || CardState == 0x02) {
	} else {
	}

    dbg_time("%s SIMStatus: %s", __func__, SIM_Status_String[*pSIMStatus]);

    free(pResponse);

    return 0;
}

int requestEnterSimPin(const CHAR *pPinCode)
{
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;

 	pRequest = ComposeQMUXMsg(QMUX_TYPE_UIM, QMIUIM_VERIFY_PIN_REQ,
				  UimVerifyPinReqSend, (void *)pPinCode);

    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    free(pResponse);
    return 0;
}

#ifdef CONFIG_IMSI_ICCID
int requestGetICCID(void)
{  // RIL_REQUEST_GET_IMSI
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    PQMIUIM_CONTENT pUimContent;
    int err;
	
	pRequest =
	    ComposeQMUXMsg(QMUX_TYPE_UIM, QMIUIM_READ_TRANSPARENT_REQ,
			   UimReadTransparentIMSIReqSend, (void *)"EF_ICCID");
	err = QmiThreadSendQMI(pRequest, &pResponse);
	

    qmi_rsp_check_and_return();

    pUimContent = (PQMIUIM_CONTENT)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x11);
    if (pUimContent != NULL) {
	static char DeviceICCID[32] = {'\0'};
	int i = 0, j = 0;

	for (i = 0, j = 0; i < le16_to_cpu(pUimContent->content_len); ++i) {
	    char charmaps[] = "0123456789ABCDEF";

	    DeviceICCID[j++] = charmaps[(pUimContent->content[i] & 0x0F)];
	    DeviceICCID[j++] =
		charmaps[((pUimContent->content[i] & 0xF0) >> 0x04)];
	}
	DeviceICCID[j] = '\0';

	dbg_time("%s DeviceICCID: %s", __func__, DeviceICCID);
    }

    free(pResponse);
    return 0;
}

int requestGetIMSI(void)
{  // RIL_REQUEST_GET_IMSI
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    PQMIUIM_CONTENT pUimContent;
    int err;

	pRequest =
	    ComposeQMUXMsg(QMUX_TYPE_UIM, QMIUIM_READ_TRANSPARENT_REQ,
			   UimReadTransparentIMSIReqSend, (void *)"EF_IMSI");
	err = QmiThreadSendQMI(pRequest, &pResponse);
	

    qmi_rsp_check_and_return();

    pUimContent = (PQMIUIM_CONTENT)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x11);
    if (pUimContent != NULL) {
	static char DeviceIMSI[32] = {'\0'};
	int i = 0, j = 0;

	for (i = 0, j = 0; i < le16_to_cpu(pUimContent->content[0]); ++i) {
	    if (i != 0)
		DeviceIMSI[j++] = (pUimContent->content[i + 1] & 0x0F) + '0';
	    DeviceIMSI[j++] =
		((pUimContent->content[i + 1] & 0xF0) >> 0x04) + '0';
	}
	DeviceIMSI[j] = '\0';

	dbg_time("%s DeviceIMSI: %s", __func__, DeviceIMSI);
    }

    free(pResponse);
    return 0;
}
#endif
#endif

static void fibo_convert_cdma_mcc_2_ascii_mcc(USHORT *p_mcc, USHORT mcc)
{
    unsigned int d1, d2, d3, buf = mcc + 111;

    if (mcc == 0x3FF)  // wildcard
    {
	*p_mcc = 3;
    } else {
	d3 = buf % 10;
	buf = (d3 == 0) ? (buf - 10) / 10 : buf / 10;

	d2 = buf % 10;
	buf = (d2 == 0) ? (buf - 10) / 10 : buf / 10;

	d1 = (buf == 10) ? 0 : buf;

	// dbg_time("d1:%d, d2:%d,d3:%d",d1,d2,d3);
	if (d1 < 10 && d2 < 10 && d3 < 10) {
	    *p_mcc = d1 * 100 + d2 * 10 + d3;
	} else {
	    // dbg_time( "invalid digits %d %d %d", d1, d2, d3 );
	    *p_mcc = 0;
	}
    }
}

static void fibo_convert_cdma_mnc_2_ascii_mnc(USHORT *p_mnc, USHORT imsi_11_12)
{
    unsigned int d1, d2, buf = imsi_11_12 + 11;

    if (imsi_11_12 == 0x7F)  // wildcard
    {
	*p_mnc = 7;
    } else {
	d2 = buf % 10;
	buf = (d2 == 0) ? (buf - 10) / 10 : buf / 10;

	d1 = (buf == 10) ? 0 : buf;

	if (d1 < 10 && d2 < 10) {
	    *p_mnc = d1 * 10 + d2;
	} else {
	    // dbg_time( "invalid digits %d %d", d1, d2, 0 );
	    *p_mnc = 0;
	}
    }
}

int requestGetHomeNetwork(USHORT *p_mcc, USHORT *p_mnc, USHORT *p_sid,
			  USHORT *p_nid)
{
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    PHOME_NETWORK pHomeNetwork;
    PHOME_NETWORK_SYSTEMID pHomeNetworkSystemID;

    pRequest =
	ComposeQMUXMsg(QMUX_TYPE_NAS, QMINAS_GET_HOME_NETWORK_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    pHomeNetwork = (PHOME_NETWORK)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x01);
    if (pHomeNetwork && p_mcc && p_mnc) {
	*p_mcc = le16_to_cpu(pHomeNetwork->MobileCountryCode);
	*p_mnc = le16_to_cpu(pHomeNetwork->MobileNetworkCode);
	// dbg_time("%s MobileCountryCode: %d, MobileNetworkCode: %d", __func__,
	// *pMobileCountryCode, *pMobileNetworkCode);
    }

    pHomeNetworkSystemID =
	(PHOME_NETWORK_SYSTEMID)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x10);
    if (pHomeNetworkSystemID && p_sid && p_nid) {
	*p_sid = le16_to_cpu(
	    pHomeNetworkSystemID->SystemID);  // china-hefei: sid 14451
	*p_nid = le16_to_cpu(pHomeNetworkSystemID->NetworkID);
	// dbg_time("%s SystemID: %d, NetworkID: %d", __func__, *pSystemID,
	// *pNetworkID);
    }

    free(pResponse);

    return 0;
}

struct wwan_data_class_str class2str[] = {
    {WWAN_DATA_CLASS_NONE, "UNKNOWN"},
    {WWAN_DATA_CLASS_GPRS, "GPRS"},
    {WWAN_DATA_CLASS_EDGE, "EDGE"},
    {WWAN_DATA_CLASS_UMTS, "UMTS"},
    {WWAN_DATA_CLASS_HSDPA, "HSDPA"},
    {WWAN_DATA_CLASS_HSUPA, "HSUPA"},
    {WWAN_DATA_CLASS_LTE, "LTE"},
    {WWAN_DATA_CLASS_1XRTT, "1XRTT"},
    {WWAN_DATA_CLASS_1XEVDO, "1XEVDO"},
    {WWAN_DATA_CLASS_1XEVDO_REVA, "1XEVDO_REVA"},
    {WWAN_DATA_CLASS_1XEVDV, "1XEVDV"},
    {WWAN_DATA_CLASS_3XRTT, "3XRTT"},
    {WWAN_DATA_CLASS_1XEVDO_REVB, "1XEVDO_REVB"},
    {WWAN_DATA_CLASS_UMB, "UMB"},
    {WWAN_DATA_CLASS_CUSTOM, "CUSTOM"},
//begin modified by zhangkaibo add 5G network detect feature on x55 platform. 20200605
    {WWAN_DATA_CLASS_5G, "NR5G"},
//end modified by zhangkaibo add 5G network detect feature on x55 platform. 20200605
};

CHAR *wwan_data_class2str(ULONG class)
{
    unsigned int i = 0;
    for (i = 0; i < sizeof(class2str) / sizeof(class2str[0]); i++) {
	if (class2str[i].class == class) {
	    return class2str[i].str;
	}
    }
    return "UNKNOWN";
}

int requestRegistrationState2(UCHAR *pPSAttachedState)
{
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
//begin modified by zhangkaibo add 5G network detect feature on x55 platform. 20200605	
    PNR5G_SYSTEM_INFO pNr5gSystemInfo;
//end modified by zhangkaibo add 5G network detect feature on x55 platform. 20200605
    PTDSCDMA_SYSTEM_INFO pTdscdmaSystemInfo;
    UCHAR DeviceClass = 0;
    ULONG DataCapList = 0;

    *pPSAttachedState = 0;
    pRequest =
	ComposeQMUXMsg(QMUX_TYPE_NAS, QMINAS_GET_SYS_INFO_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    pServiceStatusInfo = (PSERVICE_STATUS_INFO)(
	((PCHAR)&pMUXMsg->GetSysInfoResp) + QCQMUX_MSG_HDR_SIZE);
    remainingLen = le16_to_cpu(pMUXMsg->GetSysInfoResp.Length);

    s_is_cdma = 0;
    s_hdr_personality = 0;
    while (remainingLen > 0) {
	switch (pServiceStatusInfo->TLVType) {
	case 0x10:  // CDMA
	    if (pServiceStatusInfo->SrvStatus == 0x02) {
		DataCapList = WWAN_DATA_CLASS_1XRTT | WWAN_DATA_CLASS_1XEVDO |
			      WWAN_DATA_CLASS_1XEVDO_REVA |
			      WWAN_DATA_CLASS_1XEVDV |
			      WWAN_DATA_CLASS_1XEVDO_REVB;
		DeviceClass = DEVICE_CLASS_CDMA;
		s_is_cdma = (0 == is_lte);
	    }
	    break;
	case 0x11:  // HDR
	    if (pServiceStatusInfo->SrvStatus == 0x02) {
		DataCapList = WWAN_DATA_CLASS_3XRTT | WWAN_DATA_CLASS_UMB;
		DeviceClass = DEVICE_CLASS_CDMA;
		s_is_cdma = (0 == is_lte);
	    }
	    break;
	case 0x12:  // GSM
	    if (pServiceStatusInfo->SrvStatus == 0x02) {
		DataCapList = WWAN_DATA_CLASS_GPRS | WWAN_DATA_CLASS_EDGE;
		DeviceClass = DEVICE_CLASS_GSM;
	    }
	    break;
	case 0x13:  // WCDMA
	    if (pServiceStatusInfo->SrvStatus == 0x02) {
		DataCapList = WWAN_DATA_CLASS_UMTS;
		DeviceClass = DEVICE_CLASS_GSM;
	    }
	    break;
//begin modified by zhangkaibo add 5G network detect feature on x55 platform. 20200605
	case 0x4A:  // NR5G
	    if (pServiceStatusInfo->SrvStatus == 0x02) {
		DataCapList = WWAN_DATA_CLASS_5G;
		DeviceClass = DEVICE_CLASS_GSM;
		is_lte = 1;
		s_is_cdma = 0;
	    }
	    break;
//end modified by zhangkaibo add 5G network detect feature on x55 platform. 20200605
	case 0x14:  // LTE
	    if (pServiceStatusInfo->SrvStatus == 0x02) {
		DataCapList = WWAN_DATA_CLASS_LTE;
		DeviceClass = DEVICE_CLASS_GSM;
		is_lte = 1;
		s_is_cdma = 0;
	    }
	    break;
	case 0x24:  // TDSCDMA
	    if (pServiceStatusInfo->SrvStatus == 0x02) {
		pDataCapStr = "TD-SCDMA";
	    }
	    break;
	case 0x15:  // CDMA
	    // CDMA_SYSTEM_INFO
	    pCdmaSystemInfo = (PCDMA_SYSTEM_INFO)pServiceStatusInfo;
	    if (pCdmaSystemInfo->SrvDomainValid == 0x01) {
		*pPSAttachedState = 0;
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
		int i;
		CHAR temp[10];
		strncpy(temp, (CHAR *)pCdmaSystemInfo->MCC, 3);
		temp[3] = '\0';
		for (i = 0; i < 4; i++) {
		    if ((UCHAR)temp[i] == 0xFF) {
			temp[i] = '\0';
		    }
		}
		MobileCountryCode = (USHORT)atoi(temp);

		strncpy(temp, (CHAR *)pCdmaSystemInfo->MNC, 3);
		temp[3] = '\0';
		for (i = 0; i < 4; i++) {
		    if ((UCHAR)temp[i] == 0xFF) {
			temp[i] = '\0';
		    }
		}
		MobileNetworkCode = (USHORT)atoi(temp);
	    }
	    break;
	case 0x16:  // HDR
	    // HDR_SYSTEM_INFO
	    pHdrSystemInfo = (PHDR_SYSTEM_INFO)pServiceStatusInfo;
	    if (pHdrSystemInfo->SrvDomainValid == 0x01) {
		*pPSAttachedState = 0;
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
	    if (*pPSAttachedState &&
		pHdrSystemInfo->HdrPersonalityValid == 0x01) {
		if (pHdrSystemInfo->HdrPersonality == 0x03)
		    s_hdr_personality = 0x02;
		// else if (pHdrSystemInfo->HdrPersonality == 0x02)
		//    s_hdr_personality = 0x01;
	    }
	    USHORT cmda_mcc = 0, cdma_mnc = 0;
	    if (!requestGetHomeNetwork(&cmda_mcc, &cdma_mnc, NULL, NULL) &&
		cmda_mcc) {
		fibo_convert_cdma_mcc_2_ascii_mcc(&MobileCountryCode, cmda_mcc);
		fibo_convert_cdma_mnc_2_ascii_mnc(&MobileNetworkCode, cdma_mnc);
	    }
	    break;
	case 0x17:  // GSM
	    // GSM_SYSTEM_INFO
	    pGsmSystemInfo = (PGSM_SYSTEM_INFO)pServiceStatusInfo;
	    if (pGsmSystemInfo->SrvDomainValid == 0x01) {
		*pPSAttachedState = 0;
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
		int i;
		CHAR temp[10];
		strncpy(temp, (CHAR *)pGsmSystemInfo->MCC, 3);
		temp[3] = '\0';
		for (i = 0; i < 4; i++) {
		    if ((UCHAR)temp[i] == 0xFF) {
			temp[i] = '\0';
		    }
		}
		MobileCountryCode = (USHORT)atoi(temp);

		strncpy(temp, (CHAR *)pGsmSystemInfo->MNC, 3);
		temp[3] = '\0';
		for (i = 0; i < 4; i++) {
		    if ((UCHAR)temp[i] == 0xFF) {
			temp[i] = '\0';
		    }
		}
		MobileNetworkCode = (USHORT)atoi(temp);
	    }
	    break;
	case 0x18:  // WCDMA
	    // WCDMA_SYSTEM_INFO
	    pWcdmaSystemInfo = (PWCDMA_SYSTEM_INFO)pServiceStatusInfo;
	    if (pWcdmaSystemInfo->SrvDomainValid == 0x01) {
		*pPSAttachedState = 0;
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
		int i;
		CHAR temp[10];
		strncpy(temp, (CHAR *)pWcdmaSystemInfo->MCC, 3);
		temp[3] = '\0';
		for (i = 0; i < 4; i++) {
		    if ((UCHAR)temp[i] == 0xFF) {
			temp[i] = '\0';
		    }
		}
		MobileCountryCode = (USHORT)atoi(temp);

		strncpy(temp, (CHAR *)pWcdmaSystemInfo->MNC, 3);
		temp[3] = '\0';
		for (i = 0; i < 4; i++) {
		    if ((UCHAR)temp[i] == 0xFF) {
			temp[i] = '\0';
		    }
		}
		MobileNetworkCode = (USHORT)atoi(temp);
	    }
	    break;
	case 0x19:  // LTE_SYSTEM_INFO
	    // LTE_SYSTEM_INFO
	    pLteSystemInfo = (PLTE_SYSTEM_INFO)pServiceStatusInfo;
	    if (pLteSystemInfo->SrvDomainValid == 0x01) {
		*pPSAttachedState = 0;
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
		int i;
		CHAR temp[10];
		strncpy(temp, (CHAR *)pLteSystemInfo->MCC, 3);
		temp[3] = '\0';
		for (i = 0; i < 4; i++) {
		    if ((UCHAR)temp[i] == 0xFF) {
			temp[i] = '\0';
		    }
		}
		MobileCountryCode = (USHORT)atoi(temp);

		strncpy(temp, (CHAR *)pLteSystemInfo->MNC, 3);
		temp[3] = '\0';
		for (i = 0; i < 4; i++) {
		    if ((UCHAR)temp[i] == 0xFF) {
			temp[i] = '\0';
		    }
		}
		MobileNetworkCode = (USHORT)atoi(temp);
	    }
	    break;
//begin modified by zhangkaibo add 5G network detect feature on x55 platform. 20200605
	case 0x4b:  // NR5G_SYSTEM_INFO
	    // NR5G_SYSTEM_INFO
	    pNr5gSystemInfo = (PNR5G_SYSTEM_INFO)pServiceStatusInfo;
	    if (pNr5gSystemInfo->SrvDomainValid == 0x01) {
		*pPSAttachedState = 0;
		if (pNr5gSystemInfo->SrvDomain & 0x02) {
		    *pPSAttachedState = 1;
		    is_lte = 1;
		    s_is_cdma = 0;
		}
	    }

	    if (pNr5gSystemInfo->NetworkIdValid == 0x01) {
		int i;
		CHAR temp[10];
		strncpy(temp, (CHAR *)pNr5gSystemInfo->MCC, 3);
		temp[3] = '\0';
		for (i = 0; i < 4; i++) {
		    if ((UCHAR)temp[i] == 0xFF) {
			temp[i] = '\0';
		    }
		}
		MobileCountryCode = (USHORT)atoi(temp);

		strncpy(temp, (CHAR *)pNr5gSystemInfo->MNC, 3);
		temp[3] = '\0';
		for (i = 0; i < 4; i++) {
		    if ((UCHAR)temp[i] == 0xFF) {
			temp[i] = '\0';
		    }
		}
		MobileNetworkCode = (USHORT)atoi(temp);
	    }
	    break;
//end modified by zhangkaibo add 5G network detect feature on x55 platform. 20200605
	case 0x25:  // TDSCDMA
	    // TDSCDMA_SYSTEM_INFO
	    pTdscdmaSystemInfo = (PTDSCDMA_SYSTEM_INFO)pServiceStatusInfo;
	    if (pTdscdmaSystemInfo->SrvDomainValid == 0x01) {
		*pPSAttachedState = 0;
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
		int i;
		CHAR temp[10];
		strncpy(temp, (CHAR *)pTdscdmaSystemInfo->MCC, 3);
		temp[3] = '\0';
		for (i = 0; i < 4; i++) {
		    if ((UCHAR)temp[i] == 0xFF) {
			temp[i] = '\0';
		    }
		}
		MobileCountryCode = (USHORT)atoi(temp);

		strncpy(temp, (CHAR *)pTdscdmaSystemInfo->MNC, 3);
		temp[3] = '\0';
		for (i = 0; i < 4; i++) {
		    if ((UCHAR)temp[i] == 0xFF) {
			temp[i] = '\0';
		    }
		}
		MobileNetworkCode = (USHORT)atoi(temp);
	    }
	    break;
	default:
	    break;
	} /* switch (pServiceStatusInfo->TLYType) */
	remainingLen -= (le16_to_cpu(pServiceStatusInfo->TLVLength) + 3);
	pServiceStatusInfo = (PSERVICE_STATUS_INFO)(
	    (PCHAR)&pServiceStatusInfo->TLVLength +
	    le16_to_cpu(pServiceStatusInfo->TLVLength) + sizeof(USHORT));
    } /* while (remainingLen > 0) */

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
	if (DataCapList & WWAN_DATA_CLASS_LTE) {
	    pDataCapStr = wwan_data_class2str(WWAN_DATA_CLASS_LTE);
	} else if ((DataCapList & WWAN_DATA_CLASS_HSDPA) &&
		   (DataCapList & WWAN_DATA_CLASS_HSUPA)) {
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
//begin modified by zhangkaibo add 5G network detect feature on x55 platform. 20200605
	} else if (DataCapList & WWAN_DATA_CLASS_5G) {
	    pDataCapStr = wwan_data_class2str(WWAN_DATA_CLASS_5G);
//end modified by zhangkaibo add 5G network detect feature on x55 platform. 20200605
	}
    }
	g_MobileCountryCode = MobileCountryCode;
	g_MobileNetworkCode = MobileNetworkCode;
    dbg_time("%s MCC: %d, MNC: %d, PS: %s, DataCap: %s", __func__,
	     MobileCountryCode, MobileNetworkCode,
	     (*pPSAttachedState == 1) ? "Attached" : "Detached", pDataCapStr);

    free(pResponse);

    return 0;
}

int requestRegistrationState(UCHAR *pPSAttachedState)
{
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
//begin modified by zhangkaibo add 5G network detect feature on x55 platform. 20200605
	/*
	for 9x07 later, QMINAS_GET_SERVING_SYSTEM_REQ has been declara as Deprecated
	so sdx55 NR5G register info can not get by QMINAS_GET_SERVING_SYSTEM_REQ.
	coninue use QMINAS_GET_SERVING_SYSTEM_REQ, fibocom-dial will show network as UNKNOWN
	*/
    //begin modified by zhaofei delete incorrect registration interface on x55/x24 platform. 20200603
//    #if 0
// if (s_9x07) {
//eturn requestRegistrationState2(pPSAttachedState);
//  }
//    #endif
    //end modified by zhaofei delete incorrect registration interface on x55/x24 platform.20200603
//begin modified by zhangkaibo add 5G network detect feature on x55 platform. 20200605
    pRequest = ComposeQMUXMsg(QMUX_TYPE_NAS, QMINAS_GET_SERVING_SYSTEM_REQ,
			      NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    pCurrentPlmn =
	(PQMINAS_CURRENT_PLMN_MSG)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x12);
    if (pCurrentPlmn) {
	MobileCountryCode = le16_to_cpu(pCurrentPlmn->MobileCountryCode);
	MobileNetworkCode = le16_to_cpu(pCurrentPlmn->MobileNetworkCode);
    }

    *pPSAttachedState = 0;
    pServingSystem =
	(PSERVING_SYSTEM)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x01);
    if (pServingSystem) {
	// Packet-switched domain attach state of the mobile.
	// 0x00    PS_UNKNOWN ?Unknown or not applicable
	// 0x01    PS_ATTACHED ?Attached
	// 0x02    PS_DETACHED ?Detached
	*pPSAttachedState = pServingSystem->RegistrationState;
	if (pServingSystem->RegistrationState ==
	    0x01)  // 0x01 ?C REGISTERED ?C Registered with a network
	    *pPSAttachedState = pServingSystem->PSAttachedState;
	else {
	    // MobileCountryCode = MobileNetworkCode = 0;
	    *pPSAttachedState = 0x02;
	}
    }

    pDataCap = (PQMINAS_DATA_CAP)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x11);
    if (pDataCap && pDataCap->DataCapListLen) {
	UCHAR *DataCap = &pDataCap->DataCap;
	if (pDataCap->DataCapListLen == 2) {
	    if ((DataCap[0] == 0x06) &&
		((DataCap[1] == 0x08) || (DataCap[1] == 0x0A)))
		DataCap[0] = DataCap[1];
	}
	switch (DataCap[0]) {
	case 0x01:
	    pDataCapStr = "GPRS";
	    break;
	case 0x02:
	    pDataCapStr = "EDGE";
	    break;
	case 0x03:
	    pDataCapStr = "HSDPA";
	    break;
	case 0x04:
	    pDataCapStr = "HSUPA";
	    break;
	case 0x05:
	    pDataCapStr = "UMTS";
	    break;
	case 0x06:
	    pDataCapStr = "1XRTT";
	    break;
	case 0x07:
	    pDataCapStr = "1XEVDO";
	    break;
	case 0x08:
	    pDataCapStr = "1XEVDO_REVA";
	    break;
	case 0x09:
	    pDataCapStr = "GPRS";
	    break;
	case 0x0A:
	    pDataCapStr = "1XEVDO_REVB";
	    break;
	case 0x0B:
	    pDataCapStr = "LTE";
	    break;
	case 0x0C:
	    pDataCapStr = "HSDPA";
	    break;
	case 0x0D:
	    pDataCapStr = "HSDPA";
	    break;
	default:
	    pDataCapStr = "UNKNOW";
	    break;
	}
    }

    if (pServingSystem && pServingSystem->RegistrationState == 0x01 &&
	pServingSystem->InUseRadioIF && pServingSystem->RadioIF == 0x09) {
	pDataCapStr = "TD-SCDMA";
    }

    s_is_cdma = 0;
    if (pServingSystem && pServingSystem->RegistrationState == 0x01 &&
	pServingSystem->InUseRadioIF &&
	(pServingSystem->RadioIF == 0x01 || pServingSystem->RadioIF == 0x02)) {
	USHORT cmda_mcc = 0, cdma_mnc = 0;
	s_is_cdma = 1;
	if (!requestGetHomeNetwork(&cmda_mcc, &cdma_mnc, NULL, NULL) &&
	    cmda_mcc) {
	    fibo_convert_cdma_mcc_2_ascii_mcc(&MobileCountryCode, cmda_mcc);
	    fibo_convert_cdma_mnc_2_ascii_mnc(&MobileNetworkCode, cdma_mnc);
	}
	if (1) {
	    PQCQMUX_TLV pTLV =
		(PQCQMUX_TLV)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x23);
	    if (pTLV)
		s_hdr_personality = pTLV->Value;
	    else
		s_hdr_personality = 0;
	    if (s_hdr_personality == 2)
		pDataCapStr = "eHRPD";
	}
    }

    dbg_time("%s MCC: %d, MNC: %d, PS: %s, DataCap: %s", __func__,
	     MobileCountryCode, MobileNetworkCode,
	     (*pPSAttachedState == 1) ? "Attached" : "Detached", pDataCapStr);

    free(pResponse);

    return 0;
}

int requestQueryDataCall(UCHAR *pConnectionStatus, int curIpFamily)
{
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    PQMIWDS_PKT_SRVC_TLV pPktSrvc;
    UCHAR oldConnectionStatus = *pConnectionStatus;
    UCHAR QMIType =
	(curIpFamily == IpFamilyV4) ? QMUX_TYPE_WDS : QMUX_TYPE_WDS_IPV6;

    pRequest =
	ComposeQMUXMsg(QMIType, QMIWDS_GET_PKT_SRVC_STATUS_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    *pConnectionStatus = QWDS_PKT_DATA_DISCONNECTED;
    pPktSrvc =
	(PQMIWDS_PKT_SRVC_TLV)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x01);
    if (pPktSrvc) {
	*pConnectionStatus = pPktSrvc->ConnectionStatus;
	if ((le16_to_cpu(pPktSrvc->TLVLength) == 2) &&
	    (pPktSrvc->ReconfigReqd == 0x01))
	    *pConnectionStatus = QWDS_PKT_DATA_DISCONNECTED;
    }

    if (*pConnectionStatus == QWDS_PKT_DATA_DISCONNECTED) {
	if (curIpFamily == IpFamilyV4)
	    WdsConnectionIPv4Handle = 0;
	else
	    WdsConnectionIPv6Handle = 0;
    }

    if (oldConnectionStatus != *pConnectionStatus || debug_qmi) {
	dbg_time("%s %sConnectionStatus: %s", __func__,
		 (curIpFamily == IpFamilyV4) ? "IPv4" : "IPv6",
		 (*pConnectionStatus == QWDS_PKT_DATA_CONNECTED)
		     ? "CONNECTED"
		     : "DISCONNECTED");
    }

    free(pResponse);
    return 0;
}

int requestSetupDataCall(PROFILE_T *profile, int curIpFamily)
{
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err = 0;
    UCHAR QMIType =
	(curIpFamily == IpFamilyV4) ? QMUX_TYPE_WDS : QMUX_TYPE_WDS_IPV6;

    // DualIPSupported means can get ipv4 & ipv6 address at the same time, one
    // wds for ipv4, the other wds for ipv6
    profile->curIpFamily = curIpFamily;
    pRequest = ComposeQMUXMsg(QMIType, QMIWDS_START_NETWORK_INTERFACE_REQ,
			      WdsStartNwInterfaceReq, profile);
    err = QmiThreadSendQMITimeout(pRequest, &pResponse, 120 * 1000);
    qmi_rsp_check();

    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) ||
	le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
	PQMI_TLV_HDR pTLVHdr;

	pTLVHdr = GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x10);
	if (pTLVHdr) {
	    uint16_t *data16 = (uint16_t *)(pTLVHdr + 1);
	    uint16_t call_end_reason = le16_to_cpu(data16[0]);
	    dbg_time("call_end_reason is %d", call_end_reason);
	}

	pTLVHdr = GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x11);
	if (pTLVHdr) {
	    uint16_t *data16 = (uint16_t *)(pTLVHdr + 1);
	    uint16_t call_end_reason_type = le16_to_cpu(data16[0]);
	    uint16_t verbose_call_end_reason = le16_to_cpu(data16[1]);

	    dbg_time("call_end_reason_type is %d", call_end_reason_type);
	    dbg_time("call_end_reason_verbose is %d", verbose_call_end_reason);
	}

	free(pResponse);
	return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);
    }

    if (curIpFamily == IpFamilyV4) {
	WdsConnectionIPv4Handle =
	    le32_to_cpu(pResponse->MUXMsg.StartNwInterfaceResp.Handle);
	dbg_time("%s WdsConnectionIPv4Handle: 0x%08x", __func__,
		 WdsConnectionIPv4Handle);
    } else {
	WdsConnectionIPv6Handle =
	    le32_to_cpu(pResponse->MUXMsg.StartNwInterfaceResp.Handle);
	dbg_time("%s WdsConnectionIPv6Handle: 0x%08x", __func__,
		 WdsConnectionIPv6Handle);
    }

    free(pResponse);

    return 0;
}

int requestDeactivateDefaultPDP(PROFILE_T *profile, int curIpFamily)
{
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    UCHAR QMIType = (curIpFamily == 0x04) ? QMUX_TYPE_WDS : QMUX_TYPE_WDS_IPV6;

    if (curIpFamily == IpFamilyV4 && WdsConnectionIPv4Handle == 0)
	return 0;
    if (curIpFamily == IpFamilyV6 && WdsConnectionIPv6Handle == 0)
	return 0;

    pRequest = ComposeQMUXMsg(QMIType, QMIWDS_STOP_NETWORK_INTERFACE_REQ,
			      WdsStopNwInterfaceReq, &curIpFamily);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    if (curIpFamily == IpFamilyV4)
	WdsConnectionIPv4Handle = 0;
    else
	WdsConnectionIPv6Handle = 0;
    free(pResponse);
    return 0;
}

int requestGetIPAddress(PROFILE_T *profile, int curIpFamily)
{
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV4_ADDR pIpv4Addr;
    PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV6_ADDR pIpv6Addr = NULL;
    PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV6_DNS_ADDR pIpv6DNSAddr = NULL;
    PQMIWDS_GET_RUNTIME_SETTINGS_TLV_MTU pMtu;
    IPV4_T *pIpv4 = &profile->ipv4;
    IPV6_T *pIpv6 = &profile->ipv6;
    UCHAR QMIType = (curIpFamily == 0x04) ? QMUX_TYPE_WDS : QMUX_TYPE_WDS_IPV6;

    if (curIpFamily == IpFamilyV4) {
	memset(pIpv4, 0x00, sizeof(IPV4_T));
	if (WdsConnectionIPv4Handle == 0)
	    return 0;
    } else if (curIpFamily == IpFamilyV6) {
	memset(pIpv6, 0x00, sizeof(IPV6_T));
	if (WdsConnectionIPv6Handle == 0)
	    return 0;
    }

    pRequest = ComposeQMUXMsg(QMIType, QMIWDS_GET_RUNTIME_SETTINGS_REQ,
			      WdsGetRuntimeSettingReq, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    pIpv4Addr = (PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV4_ADDR)GetTLV(
	&pResponse->MUXMsg.QMUXMsgHdr,
	QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV4PRIMARYDNS);
    if (pIpv4Addr) {
	pIpv4->DnsPrimary = pIpv4Addr->IPV4Address;
    }

    pIpv4Addr = (PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV4_ADDR)GetTLV(
	&pResponse->MUXMsg.QMUXMsgHdr,
	QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV4SECONDARYDNS);
    if (pIpv4Addr) {
	pIpv4->DnsSecondary = pIpv4Addr->IPV4Address;
    }

    pIpv4Addr = (PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV4_ADDR)GetTLV(
	&pResponse->MUXMsg.QMUXMsgHdr,
	QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV4GATEWAY);
    if (pIpv4Addr) {
	pIpv4->Gateway = pIpv4Addr->IPV4Address;
    }

    pIpv4Addr = (PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV4_ADDR)GetTLV(
	&pResponse->MUXMsg.QMUXMsgHdr,
	QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV4SUBNET);
    if (pIpv4Addr) {
	pIpv4->SubnetMask = pIpv4Addr->IPV4Address;
    }

    pIpv4Addr = (PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV4_ADDR)GetTLV(
	&pResponse->MUXMsg.QMUXMsgHdr,
	QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV4);
    if (pIpv4Addr) {
	pIpv4->Address = pIpv4Addr->IPV4Address;
    }

    pIpv6DNSAddr = (PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV6_DNS_ADDR)GetTLV(
	&pResponse->MUXMsg.QMUXMsgHdr,
	QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV6PRIMARYDNS);
    if (pIpv6DNSAddr) {
	memcpy(pIpv6->DnsPrimary, pIpv6DNSAddr->IPV6Address, 16);
    }

    pIpv6DNSAddr = (PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV6_DNS_ADDR)GetTLV(
	&pResponse->MUXMsg.QMUXMsgHdr,
	QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV6SECONDARYDNS);
    if (pIpv6DNSAddr) {
	memcpy(pIpv6->DnsSecondary, pIpv6DNSAddr->IPV6Address, 16);
    }

//2021-02-25 willa.liu@fibocom.com changed begin for support eipd SN-20210129001

/*
    pIpv6Addr = (PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV6_ADDR)GetTLV(
    &pResponse->MUXMsg.QMUXMsgHdr,
    QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV6GATEWAY);
    if(pIpv6Addr) {
    memcpy(pIpv6->Gateway, pIpv6Addr->IPV6Address, 16);
    pIpv6->PrefixLengthGateway = pIpv6Addr->PrefixLength;
    }
*/

    if(profile->ipv6_prigateway_flag == 1)
    {
        char localip6gateway[1024] = {0};
        get_private_gateway(localip6gateway);
        int length = sizeof(localip6gateway)/sizeof(localip6gateway[0]);
        if (pIpv6Addr)
        {
            int i = 0;
            char *token = strtok(localip6gateway, ".");
            while(token != NULL)
            {
                pIpv6->Gateway[i++] = atoi(token);
                //printf("token:%s, pIpv6->Gateway[%d]:%d\n", token, i-1, pIpv6->Gateway[i-1]);
                token = strtok(NULL, ".");
            }
        pIpv6->PrefixLengthGateway = 16;
        }
    }
    else
    {
        pIpv6Addr = (PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV6_ADDR)GetTLV(
        &pResponse->MUXMsg.QMUXMsgHdr,
        QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV6GATEWAY);
        if(pIpv6Addr)
        {
            memcpy(pIpv6->Gateway, pIpv6Addr->IPV6Address, 16);
            pIpv6->PrefixLengthGateway = pIpv6Addr->PrefixLength;
        }
    }
//2021-02-25 willa.liu@fibocom.com changed end for support eipd SN-20210129001


    pIpv6Addr = (PQMIWDS_GET_RUNTIME_SETTINGS_TLV_IPV6_ADDR)GetTLV(
	&pResponse->MUXMsg.QMUXMsgHdr,
	QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_IPV6);
    if (pIpv6Addr) {
	memcpy(pIpv6->Address, pIpv6Addr->IPV6Address, 16);
	pIpv6->PrefixLengthIPAddr = pIpv6Addr->PrefixLength;
    }

    pMtu = (PQMIWDS_GET_RUNTIME_SETTINGS_TLV_MTU)GetTLV(
	&pResponse->MUXMsg.QMUXMsgHdr,
	QMIWDS_GET_RUNTIME_SETTINGS_TLV_TYPE_MTU);
    if (pMtu) {
	pIpv4->Mtu = pIpv6->Mtu = le32_to_cpu(pMtu->Mtu);
    }

    free(pResponse);
    return 0;
}

#ifdef CONFIG_APN
int requestSetProfile(PROFILE_T *profile)
{
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;

    if (!profile->pdp)
	return 0;

    dbg_time("%s[%d] %s/%s/%s/%d", __func__, profile->pdpindex, profile->apn,
	     profile->user, profile->password, profile->auth);
	     
    UCHAR QMIType = (profile->ipv4_flag) ? QMUX_TYPE_WDS : QMUX_TYPE_WDS_IPV6;
    pRequest = ComposeQMUXMsg(QMIType, QMIWDS_MODIFY_PROFILE_SETTINGS_REQ,
			      WdsModifyProfileSettingsReq, profile);
    err = QmiThreadSendQMI(pRequest, &pResponse);
//begin modified by zhangkaibo add create profile qmi. mantis 0049137,0048741  20200610
    //modify profile no check return value, try create
    qmi_rsp_check();
    if(le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError))
    {    	
		pRequest = ComposeQMUXMsg(QMIType, QMIWDS_CREATE_PROFILE_SETTINGS_REQ,
					  WdsCreateProfileSettingsReq, profile);
		err = QmiThreadSendQMI(pRequest, &pResponse);
		qmi_rsp_check();		
		if(le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)){    
			free(pResponse);
			return -1;
		}
    	dbg_time("WdsCreateProfileSettingsReq[%d] %s/%s/%s/%d", profile->pdpindex, profile->apn,
	     profile->user, profile->password, profile->auth);
    }
//end modified by zhangkaibo add create profile qmi. mantis 0049137,0048741  20200610
    free(pResponse);
    return 0;
}

int requestGetProfile(PROFILE_T *profile)
{
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    char *apn = NULL;
    char *user = NULL;
    char *password = NULL;
    int auth = 0;
    PQMIWDS_APNNAME pApnName;
    PQMIWDS_USERNAME pUserName;
    PQMIWDS_PASSWD pPassWd;
    PQMIWDS_AUTH_PREFERENCE pAuthPref;

    if (!profile->pdp)
	return 0;

    UCHAR QMIType = (profile->ipv4_flag) ? QMUX_TYPE_WDS : QMUX_TYPE_WDS_IPV6;
    pRequest = ComposeQMUXMsg(QMIType, QMIWDS_GET_PROFILE_SETTINGS_REQ,
			      WdsGetProfileSettingsReqSend, profile);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    pApnName = (PQMIWDS_APNNAME)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x14);
    pUserName = (PQMIWDS_USERNAME)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x1B);
    pPassWd = (PQMIWDS_PASSWD)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x1C);
    pAuthPref =
	(PQMIWDS_AUTH_PREFERENCE)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x1D);

    if (pApnName /* && le16_to_cpu(pApnName->TLVLength)*/)
	apn = strndup((const char *)(&pApnName->ApnName),
		      le16_to_cpu(pApnName->TLVLength));
    if (pUserName /*  && pUserName->UserName*/)
	user = strndup((const char *)(&pUserName->UserName),
		       le16_to_cpu(pUserName->TLVLength));
    if (pPassWd /*  && le16_to_cpu(pPassWd->TLVLength)*/)
	password = strndup((const char *)(&pPassWd->Passwd),
			   le16_to_cpu(pPassWd->TLVLength));
    if (pAuthPref /*  && le16_to_cpu(pAuthPref->TLVLength)*/) {
	auth = pAuthPref->AuthPreference;
    }

#if 0
    if (profile) {
        profile->apn = apn;
        profile->user = user;
        profile->password = password;
        profile->auth = auth;
    }
#endif

    dbg_time("%s[%d] %s/%s/%s/%d", __func__, profile->pdpindex, apn, user, password,
	     auth);

    free(pResponse);
    return 0;
}
#endif

#ifdef CONFIG_VERSION
int requestBaseBandVersion(const char **pp_reversion)
{
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    PDEVICE_REV_ID revId;
    int err;

    if (pp_reversion)
	*pp_reversion = NULL;

    pRequest =
	ComposeQMUXMsg(QMUX_TYPE_DMS, QMIDMS_GET_DEVICE_REV_ID_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    revId = (PDEVICE_REV_ID)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x01);

    if (revId && le16_to_cpu(revId->TLVLength)) {
	char *DeviceRevisionID = strndup((const char *)(&revId->RevisionID),
					 le16_to_cpu(revId->TLVLength));
	dbg_time("%s %s", __func__, DeviceRevisionID);

	if (pp_reversion)
	    *pp_reversion = DeviceRevisionID;
    }

    free(pResponse);
    return 0;
}
#endif

#ifdef CONFIG_RESET_RADIO
static USHORT DmsSetOperatingModeReq(PQMUX_MSG pMUXMsg, void *arg)
{
    pMUXMsg->SetOperatingModeReq.TLVType = 0x01;
    pMUXMsg->SetOperatingModeReq.TLVLength = cpu_to_le16(1);
    pMUXMsg->SetOperatingModeReq.OperatingMode = *((UCHAR *)arg);

    return sizeof(QMIDMS_SET_OPERATING_MODE_REQ_MSG);
}

int requestSetOperatingMode(UCHAR OperatingMode)
{
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;

    dbg_time("%s(%d)", __func__, OperatingMode);

    pRequest = ComposeQMUXMsg(QMUX_TYPE_DMS, QMIDMS_SET_OPERATING_MODE_REQ,
			      DmsSetOperatingModeReq, &OperatingMode);
    err = QmiThreadSendQMI(pRequest, &pResponse);
    qmi_rsp_check_and_return();

    free(pResponse);
    return 0;
}
#endif
