#include "QMIThread.h"
extern char *strndup (const char *__string, size_t __n);

int qmiclientId[QMUX_TYPE_WDS_ADMIN + 1]; //GobiNet use fd to indicate client ID, so type of qmiclientId must be int
static uint32_t WdsConnectionIPv4Handle = 0;
static uint32_t WdsConnectionIPv6Handle = 0;
static int s_is_cdma = 0;
static int s_hdr_personality = 0; // 0x01-HRPD, 0x02-eHRPD
static char *qstrcpy(char *to, const char *from) { //no __strcpy_chk
	char *save = to;
	for (; (*to = *from) != '\0'; ++from, ++to);
	return(save);
}

static int s_9x07 = -1;

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
    pRequest->QMIHdr.ClientId = qmiclientId[QMIType] & 0xFF;

    if (qmiclientId[QMIType] == 0) {
        dbg_time("QMIType %d has no clientID", QMIType);
        return NULL;
    }

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
    if (profile->apn) {
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
    if (profile->pdp && !s_is_cdma) { //cdma only allow one pdp
        PQMIWDS_PROFILE_IDENTIFIER pProfileIndex = (PQMIWDS_PROFILE_IDENTIFIER)(pTLV + TLVLength);
        pProfileIndex->TLVLength = cpu_to_le16(0x01);
        pProfileIndex->TLVType = 0x31;
        pProfileIndex->ProfileIndex = profile->pdp;
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

static USHORT WdaSetDataFormat(PQMUX_MSG pMUXMsg, void *arg) {
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
        dbg_time("%s sizeof(*linkProto) = %d, is not 7!", __func__, sizeof(*linkProto) );

    return sizeof(QCQMUX_MSG_HDR) + sizeof(*pWdsAdminQosTlv) + sizeof(*linkProto) + sizeof(*dlTlp);
}

#ifdef CONFIG_SIM
static USHORT DmsUIMVerifyPinReqSend(PQMUX_MSG pMUXMsg, void *arg) {
    pMUXMsg->UIMVerifyPinReq.TLVType = 0x01;
    pMUXMsg->UIMVerifyPinReq.PINID = 0x01; //Pin1, not Puk
    pMUXMsg->UIMVerifyPinReq.PINLen = strlen((const char *)arg);
    qstrcpy((PCHAR)&pMUXMsg->UIMVerifyPinReq.PINValue, ((const char *)arg));
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
    qstrcpy((PCHAR)&pMUXMsg->UIMUIMVerifyPinReq.PINValue, ((const char *)arg));
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

    pReadTransparent = (PREAD_TRANSPARENT_TLV)(&pMUXMsg->UIMUIMReadTransparentReq.path[0] + pMUXMsg->UIMUIMReadTransparentReq.path_len);
    pReadTransparent->TLVType = 0x03;
    pReadTransparent->TLVLength = cpu_to_le16(0x04);
    pReadTransparent->Offset = cpu_to_le16(0x00);
    pReadTransparent->Length = cpu_to_le16(0x00);

    return (sizeof(QMIUIM_READ_TRANSPARENT_REQ_MSG) + pMUXMsg->UIMUIMReadTransparentReq.path_len + sizeof(READ_TRANSPARENT_TLV));
}
#endif
#endif

#ifdef CONFIG_APN
static USHORT WdsGetProfileSettingsReqSend(PQMUX_MSG pMUXMsg, void *arg) {
    PROFILE_T *profile = (PROFILE_T *)arg;
    pMUXMsg->GetProfileSettingsReq.Length = cpu_to_le16(sizeof(QMIWDS_GET_PROFILE_SETTINGS_REQ_MSG) - 4);
    pMUXMsg->GetProfileSettingsReq.TLVType = 0x01;
    pMUXMsg->GetProfileSettingsReq.TLVLength = cpu_to_le16(0x02);
    pMUXMsg->GetProfileSettingsReq.ProfileType = 0x00; // 0 ~ 3GPP, 1 ~ 3GPP2
    pMUXMsg->GetProfileSettingsReq.ProfileIndex = profile->pdp;
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
    pMUXMsg->ModifyProfileSettingsReq.ProfileIndex = profile->pdp;

    pTLV = (UCHAR *)(&pMUXMsg->ModifyProfileSettingsReq + 1);

    pPdpType = (PQMIWDS_PDPTYPE)(pTLV + TLVLength);
    pPdpType->TLVType = 0x11;
    pPdpType->TLVLength = cpu_to_le16(0x01);
// 0 ?C PDP-IP (IPv4)
// 1 ?C PDP-PPP
// 2 ?C PDP-IPv6
// 3 ?C PDP-IPv4v6
    pPdpType->PdpType = 3;
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

static USHORT WdsGetRuntimeSettingReq(PQMUX_MSG pMUXMsg, void *arg) {
   pMUXMsg->GetRuntimeSettingsReq.TLVType = 0x10;
   pMUXMsg->GetRuntimeSettingsReq.TLVLength = cpu_to_le16(0x04);
   // the following mask also applies to IPV6
   pMUXMsg->GetRuntimeSettingsReq.Mask = cpu_to_le32(QMIWDS_GET_RUNTIME_SETTINGS_MASK_IPV4DNS_ADDR |
                                          QMIWDS_GET_RUNTIME_SETTINGS_MASK_IPV4_ADDR |
                                          QMIWDS_GET_RUNTIME_SETTINGS_MASK_MTU |
                                          QMIWDS_GET_RUNTIME_SETTINGS_MASK_IPV4GATEWAY_ADDR); // |
                                          // QMIWDS_GET_RUNTIME_SETTINGS_MASK_PCSCF_SV_ADDR |
                                          // QMIWDS_GET_RUNTIME_SETTINGS_MASK_PCSCF_DOM_NAME;

    return sizeof(QMIWDS_GET_RUNTIME_SETTINGS_REQ_MSG);
}

static PQCQMIMSG s_pRequest;
static PQCQMIMSG s_pResponse;
static pthread_mutex_t s_commandmutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_commandcond = PTHREAD_COND_INITIALIZER;

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

int QmiThreadSendQMITimeout(PQCQMIMSG pRequest, PQCQMIMSG *ppResponse, unsigned msecs) {
    int ret;

    if (!pRequest)
    {
        return -EINVAL;
    }

    pthread_mutex_lock(&s_commandmutex);

    if (ppResponse)
        *ppResponse = NULL;

    dump_qmi(pRequest, le16_to_cpu(pRequest->QMIHdr.Length) + 1);

    s_pRequest = pRequest;
    s_pResponse = NULL;

    if (!strncmp(qmichannel, "/dev/qcqmi", strlen("/dev/qcqmi")))
        ret = GobiNetSendQMI(pRequest);
    else
        ret = QmiWwanSendQMI(pRequest);

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
            dbg_time("%s pthread_cond_timeout_np=%d, errno: %d (%s)", __func__, ret, errno, strerror(errno));
        }
    }

    pthread_mutex_unlock(&s_commandmutex);

    return ret;
}

int QmiThreadSendQMI(PQCQMIMSG pRequest, PQCQMIMSG *ppResponse) {
    return QmiThreadSendQMITimeout(pRequest, ppResponse, 30 * 1000);
}

void QmiThreadRecvQMI(PQCQMIMSG pResponse) {
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
            memcpy(s_pResponse, pResponse, le16_to_cpu(pResponse->QMIHdr.Length) + 1);
        }
        pthread_cond_signal(&s_commandcond);
    } else if ((pResponse->QMIHdr.QMIType == QMUX_TYPE_NAS)
                    && (le16_to_cpu(pResponse->MUXMsg.QMUXMsgHdrResp.Type) == QMINAS_SERVING_SYSTEM_IND)) {
        qmidevice_send_event_to_main(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED);
    } else if ((pResponse->QMIHdr.QMIType == QMUX_TYPE_WDS)
                    && (le16_to_cpu(pResponse->MUXMsg.QMUXMsgHdrResp.Type) == QMIWDS_GET_PKT_SRVC_STATUS_IND)) {
        qmidevice_send_event_to_main(RIL_UNSOL_DATA_CALL_LIST_CHANGED);
    } else if ((pResponse->QMIHdr.QMIType == QMUX_TYPE_NAS)
                    && (le16_to_cpu(pResponse->MUXMsg.QMUXMsgHdrResp.Type) == QMINAS_SYS_INFO_IND)) {
        qmidevice_send_event_to_main(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED);
    } else {
        if (debug_qmi)
            dbg_time("nobody care this qmi msg!!");
    }
    pthread_mutex_unlock(&s_commandmutex);
}

int requestSetEthMode(PROFILE_T *profile) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV linkProto;
    UCHAR autoconnect_setting = 0;

    if (profile->IsDualIPSupported) {
        UCHAR IpPreference;

        IpPreference = IpFamilyV4;
        pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS, QMIWDS_SET_CLIENT_IP_FAMILY_PREF_REQ, WdsSetClientIPFamilyPref, (void *)&IpPreference);
        QmiThreadSendQMI(pRequest, &pResponse);
        if (pResponse) free(pResponse);

        IpPreference = IpFamilyV6;
        pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS_IPV6, QMIWDS_SET_CLIENT_IP_FAMILY_PREF_REQ, WdsSetClientIPFamilyPref, (void *)&IpPreference);
        QmiThreadSendQMI(pRequest, &pResponse);
        if (pResponse) free(pResponse);
    }
    
    pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS, QMIWDS_SET_AUTO_CONNECT_REQ , WdsSetAutoConnect, (void *)&autoconnect_setting);
    QmiThreadSendQMI(pRequest, &pResponse);
    if (pResponse) free(pResponse);

    pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS_ADMIN, QMIWDS_ADMIN_SET_DATA_FORMAT_REQ, WdaSetDataFormat, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);

    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);
    }

    linkProto = (PQMIWDS_ADMIN_SET_DATA_FORMAT_TLV)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x11);
    if (linkProto != NULL) {
        profile->rawIP = (le32_to_cpu(linkProto->Value) == 2);
        s_9x07 = profile->rawIP;
    }

    free(pResponse);
    return 0;
}

#ifdef CONFIG_SIM
int requestGetPINStatus(SIM_Status *pSIMStatus) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    PQMIDMS_UIM_PIN_STATUS pPin1Status = NULL;
    //PQMIDMS_UIM_PIN_STATUS pPin2Status = NULL;

    if (s_9x07 && qmiclientId[QMUX_TYPE_UIM])
        pRequest = ComposeQMUXMsg(QMUX_TYPE_UIM, QMIUIM_GET_CARD_STATUS_REQ, NULL, NULL);
    else
        pRequest = ComposeQMUXMsg(QMUX_TYPE_DMS, QMIDMS_UIM_GET_PIN_STATUS_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);

    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);
    }

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

int requestGetSIMStatus(SIM_Status *pSIMStatus) { //RIL_REQUEST_GET_SIM_STATUS
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

__requestGetSIMStatus:
    if (s_9x07 && qmiclientId[QMUX_TYPE_UIM])
        pRequest = ComposeQMUXMsg(QMUX_TYPE_UIM, QMIUIM_GET_CARD_STATUS_REQ, NULL, NULL);
    else
        pRequest = ComposeQMUXMsg(QMUX_TYPE_DMS, QMIDMS_UIM_GET_STATE_REQ, NULL, NULL);

    err = QmiThreadSendQMI(pRequest, &pResponse);

    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        if (QMI_ERR_OP_DEVICE_UNSUPPORTED == le16_to_cpu(pResponse->MUXMsg.QMUXMsgHdrResp.QMUXError)) {
            sleep(1);
            goto __requestGetSIMStatus;
        }
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);
    }

    *pSIMStatus = SIM_ABSENT;
    if (s_9x07 && qmiclientId[QMUX_TYPE_UIM])
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

int requestEnterSimPin(const CHAR *pPinCode) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;

    if (s_9x07 && qmiclientId[QMUX_TYPE_UIM])
        pRequest = ComposeQMUXMsg(QMUX_TYPE_UIM, QMIUIM_VERIFY_PIN_REQ, UimVerifyPinReqSend, (void *)pPinCode);
    else
        pRequest = ComposeQMUXMsg(QMUX_TYPE_DMS, QMIDMS_UIM_VERIFY_PIN_REQ, DmsUIMVerifyPinReqSend, (void *)pPinCode);
    err = QmiThreadSendQMI(pRequest, &pResponse);

    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);
    }

    free(pResponse);
    return 0;
}

#ifdef CONFIG_IMSI_ICCID
int requestGetICCID(void) { //RIL_REQUEST_GET_IMSI
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    PQMIUIM_CONTENT pUimContent;
    int err;

    if (s_9x07 && qmiclientId[QMUX_TYPE_UIM]) {
        pRequest = ComposeQMUXMsg(QMUX_TYPE_UIM, QMIUIM_READ_TRANSPARENT_REQ, UimReadTransparentIMSIReqSend, (void *)"EF_ICCID");
        err = QmiThreadSendQMI(pRequest, &pResponse);
    } else {
        return 0;
    }

    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);
    }

    pUimContent = (PQMIUIM_CONTENT)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x11);
    if (pUimContent != NULL) {
        static char DeviceICCID[32] = {'\0'};
        int i = 0, j = 0;

        for (i = 0, j = 0; i < le16_to_cpu(pUimContent->content_len); ++i) {
            if  ((pUimContent->content[i] & 0x0F) >= 0x0A)
                DeviceICCID[j++] = 'A' + (pUimContent->content[i] & 0x0F);
            else
                DeviceICCID[j++] = '0' + (pUimContent->content[i] & 0x0F);

            if (((pUimContent->content[i] & 0xF0) >> 0x04) >= 0x0A)
                DeviceICCID[j++] = 'A' + ((pUimContent->content[i] & 0xF0) >> 0x04);
            else
                DeviceICCID[j++] = '0' + ((pUimContent->content[i] & 0xF0) >> 0x04);
        }
        DeviceICCID[j] = '\0';

        dbg_time("%s DeviceICCID: %s", __func__, DeviceICCID);
    }

    free(pResponse);
    return 0;
}

int requestGetIMSI(void) { //RIL_REQUEST_GET_IMSI
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    PQMIUIM_CONTENT pUimContent;
    int err;

    if (s_9x07 && qmiclientId[QMUX_TYPE_UIM]) {
        pRequest = ComposeQMUXMsg(QMUX_TYPE_UIM, QMIUIM_READ_TRANSPARENT_REQ, UimReadTransparentIMSIReqSend, (void *)"EF_IMSI");
        err = QmiThreadSendQMI(pRequest, &pResponse);
    } else {
        return 0;
    }

    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);
    }

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

int requestGetHomeNetwork(USHORT *p_mcc, USHORT *p_mnc, USHORT *p_sid, USHORT *p_nid) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    PHOME_NETWORK pHomeNetwork;
    PHOME_NETWORK_SYSTEMID pHomeNetworkSystemID;

    pRequest = ComposeQMUXMsg(QMUX_TYPE_NAS, QMINAS_GET_HOME_NETWORK_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);

    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);
    }

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
    "302",	//Canada
    "310",	//United States of America
    "311",	//United States of America
    "312",	//United States of America
    "313",	//United States of America
    "314",	//United States of America
    "315",	//United States of America
    "316",	//United States of America
    "334",	//Mexico
    "338",	//Jamaica
    "342", //Barbados
    "344",	//Antigua and Barbuda
    "346",	//Cayman Islands
    "348",	//British Virgin Islands
    "365",	//Anguilla
    "708",	//Honduras (Republic of)
    "722",	//Argentine Republic
    "732"	//Colombia (Republic of)
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

    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);
    }

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
};

CHAR *wwan_data_class2str(ULONG class)
{
    unsigned int i = 0;
    for (i = 0; i < sizeof(class2str)/sizeof(class2str[0]); i++) {
        if (class2str[i].class == class) {
            return class2str[i].str;
        }
    }
    return "UNKNOWN";
}

int requestRegistrationState2(UCHAR *pPSAttachedState) {
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
    UCHAR DeviceClass = 0;
    ULONG DataCapList = 0;

    *pPSAttachedState = 0;
    pRequest = ComposeQMUXMsg(QMUX_TYPE_NAS, QMINAS_GET_SYS_INFO_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);

    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);
    }

    pServiceStatusInfo = (PSERVICE_STATUS_INFO)(((PCHAR)&pMUXMsg->GetSysInfoResp) + QCQMUX_MSG_HDR_SIZE);
    remainingLen = le16_to_cpu(pMUXMsg->GetSysInfoResp.Length);

    s_is_cdma = 0;
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
        case 0x24: // TDSCDMA
            if (pServiceStatusInfo->SrvStatus == 0x02) {
                pDataCapStr = "TD-SCDMA";
            }
            break;
        case 0x15: // CDMA
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
        case 0x16: // HDR
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
        case 0x18: // WCDMA
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
        case 0x19: // LTE_SYSTEM_INFO
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
        case 0x25: // TDSCDMA
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
        pServiceStatusInfo = (PSERVICE_STATUS_INFO)((PCHAR)&pServiceStatusInfo->TLVLength + le16_to_cpu(pServiceStatusInfo->TLVLength) + sizeof(USHORT));
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

int requestRegistrationState(UCHAR *pPSAttachedState) {
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

    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);
    }

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

int requestQueryDataCall(UCHAR  *pConnectionStatus, int curIpFamily) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    PQMIWDS_PKT_SRVC_TLV pPktSrvc;
    UCHAR oldConnectionStatus = *pConnectionStatus;
    UCHAR QMIType = (curIpFamily == IpFamilyV4) ? QMUX_TYPE_WDS : QMUX_TYPE_WDS_IPV6;

    pRequest = ComposeQMUXMsg(QMIType, QMIWDS_GET_PKT_SRVC_STATUS_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);

    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);
    }

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

#if 0
BOOLEAN QCMAIN_IsDualIPSupported(PMP_ADAPTER pAdapter)
{
  return (pAdapter->QMUXVersion[QMUX_TYPE_WDS].Major >= 1 && pAdapter->QMUXVersion[QMUX_TYPE_WDS].Minor >= 9);
}  // QCMAIN_IsDualIPSupported
#endif

int requestSetupDataCall(PROFILE_T *profile, int curIpFamily) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err = 0;
    int old_auth = profile->auth;
    UCHAR QMIType = (curIpFamily == IpFamilyV4) ? QMUX_TYPE_WDS : QMUX_TYPE_WDS_IPV6;

//DualIPSupported means can get ipv4 & ipv6 address at the same time, one wds for ipv4, the other wds for ipv6
__requestSetupDataCall:
    profile->curIpFamily = curIpFamily;
    pRequest = ComposeQMUXMsg(QMIType, QMIWDS_START_NETWORK_INTERFACE_REQ, WdsStartNwInterfaceReq, profile);
    err = QmiThreadSendQMITimeout(pRequest, &pResponse, 120 * 1000);

    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError))
    {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));

        if (curIpFamily == IpFamilyV4 && old_auth == profile->auth
            && profile->user && profile->user[0] && profile->password && profile->password[0]) {
            profile->auth = (profile->auth == 1) ? 2 : 1;
            free(pResponse);
            goto __requestSetupDataCall;
        }

        profile->auth = old_auth;

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

int requestDeactivateDefaultPDP(PROFILE_T *profile, int curIpFamily) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;
    UCHAR QMIType = (curIpFamily == 0x04) ? QMUX_TYPE_WDS : QMUX_TYPE_WDS_IPV6;

    if (curIpFamily == IpFamilyV4 && WdsConnectionIPv4Handle == 0)
        return 0;
    if (curIpFamily == IpFamilyV6 && WdsConnectionIPv6Handle == 0)
        return 0;

    pRequest = ComposeQMUXMsg(QMIType, QMIWDS_STOP_NETWORK_INTERFACE_REQ , WdsStopNwInterfaceReq, &curIpFamily);
    err = QmiThreadSendQMI(pRequest, &pResponse);

    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);
    }

    if (curIpFamily == IpFamilyV4)
        WdsConnectionIPv4Handle = 0;
    else
         WdsConnectionIPv6Handle = 0;
    free(pResponse);
    return 0;
}

int requestGetIPAddress(PROFILE_T *profile, int curIpFamily) {
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

    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);
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
        pIpv4->Mtu =  pIpv6->Mtu =  le32_to_cpu(pMtu->Mtu);
    }

    free(pResponse);
    return 0;
}

#ifdef CONFIG_APN
int requestSetProfile(PROFILE_T *profile) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;

    if (!profile->pdp)
        return 0;

    dbg_time("%s[%d] %s/%s/%s/%d", __func__, profile->pdp, profile->apn, profile->user, profile->password, profile->auth);
    pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS, QMIWDS_MODIFY_PROFILE_SETTINGS_REQ, WdsModifyProfileSettingsReq, profile);
    err = QmiThreadSendQMI(pRequest, &pResponse);

    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);
    }

    free(pResponse);
    return 0;
}

int requestGetProfile(PROFILE_T *profile) {
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

    pRequest = ComposeQMUXMsg(QMUX_TYPE_WDS, QMIWDS_GET_PROFILE_SETTINGS_REQ, WdsGetProfileSettingsReqSend, profile);
    err = QmiThreadSendQMI(pRequest, &pResponse);

    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);
    }

    pApnName = (PQMIWDS_APNNAME)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x14);
    pUserName = (PQMIWDS_USERNAME)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x1B);
    pPassWd = (PQMIWDS_PASSWD)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x1C);
    pAuthPref = (PQMIWDS_AUTH_PREFERENCE)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x1D);

    if (pApnName/* && le16_to_cpu(pApnName->TLVLength)*/)
        apn = strndup((const char *)(&pApnName->ApnName), le16_to_cpu(pApnName->TLVLength));
    if (pUserName/*  && pUserName->UserName*/)
        user = strndup((const char *)(&pUserName->UserName), le16_to_cpu(pUserName->TLVLength));
    if (pPassWd/*  && le16_to_cpu(pPassWd->TLVLength)*/)
        password = strndup((const char *)(&pPassWd->Passwd), le16_to_cpu(pPassWd->TLVLength));
    if (pAuthPref/*  && le16_to_cpu(pAuthPref->TLVLength)*/) {
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

    dbg_time("%s[%d] %s/%s/%s/%d", __func__, profile->pdp, apn, user, password, auth);

    free(pResponse);
    return 0;
}
#endif

#ifdef CONFIG_VERSION
int requestBaseBandVersion(const char **pp_reversion) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    PDEVICE_REV_ID revId;
    int err;

    if (pp_reversion) *pp_reversion = NULL;

    pRequest = ComposeQMUXMsg(QMUX_TYPE_DMS, QMIDMS_GET_DEVICE_REV_ID_REQ, NULL, NULL);
    err = QmiThreadSendQMI(pRequest, &pResponse);

    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);
    }

    revId = (PDEVICE_REV_ID)GetTLV(&pResponse->MUXMsg.QMUXMsgHdr, 0x01);

    if (revId && le16_to_cpu(revId->TLVLength))
    {
        char *DeviceRevisionID = strndup((const char *)(&revId->RevisionID), le16_to_cpu(revId->TLVLength));
        dbg_time("%s %s", __func__, DeviceRevisionID);
        if (s_9x07 == -1) { //fail to get QMUX_TYPE_WDS_ADMIN
            if (strncmp(DeviceRevisionID, "EC20", strlen("EC20")))
                s_9x07 = 1;
            else
                s_9x07 = DeviceRevisionID[5] == 'F' || DeviceRevisionID[6] == 'F'; //EC20CF,EC20EF,EC20CEF
        }
        if (pp_reversion) *pp_reversion = DeviceRevisionID;
    }

    free(pResponse);
    return 0;
}
#endif

#ifdef CONFIG_RESET_RADIO
static USHORT DmsSetOperatingModeReq(PQMUX_MSG pMUXMsg, void *arg) {
    pMUXMsg->SetOperatingModeReq.TLVType = 0x01;
    pMUXMsg->SetOperatingModeReq.TLVLength = cpu_to_le16(1);
    pMUXMsg->SetOperatingModeReq.OperatingMode = *((UCHAR *)arg);

    return sizeof(QMIDMS_SET_OPERATING_MODE_REQ_MSG);
}

int requestSetOperatingMode(UCHAR OperatingMode) {
    PQCQMIMSG pRequest;
    PQCQMIMSG pResponse;
    PQMUX_MSG pMUXMsg;
    int err;

    dbg_time("%s(%d)", __func__, OperatingMode);
    
    pRequest = ComposeQMUXMsg(QMUX_TYPE_DMS, QMIDMS_SET_OPERATING_MODE_REQ, DmsSetOperatingModeReq, &OperatingMode);
    err = QmiThreadSendQMI(pRequest, &pResponse);

    if (err < 0 || pResponse == NULL) {
        dbg_time("%s err = %d", __func__, err);
        return err;
    }

    pMUXMsg = &pResponse->MUXMsg;
    if (le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult) || le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError)) {
        dbg_time("%s QMUXResult = 0x%x, QMUXError = 0x%x", __func__,
            le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXResult), le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError));
        return le16_to_cpu(pMUXMsg->QMUXMsgHdrResp.QMUXError);
    }

    free(pResponse);
    return 0;
}
#endif
