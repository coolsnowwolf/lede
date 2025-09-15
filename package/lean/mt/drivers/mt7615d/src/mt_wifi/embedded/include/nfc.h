#ifndef __WSC_NFC_H__
#define __WSC_NFC_H__

VOID	NfcParseRspCommand(
	IN  PRTMP_ADAPTER	pAd,
	IN  PUCHAR pData,
	IN  USHORT DataLen);

VOID	NfcCommand(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR Action,
	IN  UCHAR Type,
	IN  SHORT DataLen,
	IN  PUCHAR pData);

VOID	NfcGenRandomPasswd(
	IN  PRTMP_ADAPTER	pAd,
	IN  PWSC_CTRL       pWscCtrl);

INT	NfcBuildWscProfileTLV(
	IN	PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscCtrl,
	OUT	UCHAR * pbuf,
	OUT USHORT * pBufLen);

INT	NfcBuildOOBDevPasswdTLV(
	IN	PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscCtrl,
	IN	UCHAR	HandoverType,
	OUT	UCHAR * pbuf,
	OUT USHORT * pBufLen);

INT Set_NfcStatus_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_NfcPasswdToken_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_NfcConfigurationToken_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Get_NfcStatus_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_DoWpsByNFC_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_NfcRegenPK_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);


#endif /* __WSC_NFC_H__ */
