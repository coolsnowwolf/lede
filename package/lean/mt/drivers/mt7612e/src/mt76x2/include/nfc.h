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

INT 	NfcBuildWscProfileTLV(
	IN	PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscCtrl,
	OUT	UCHAR *pbuf,
	OUT USHORT *pBufLen);

INT 	NfcBuildOOBDevPasswdTLV(
	IN	PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscCtrl,
	IN	UCHAR	HandoverType,
	OUT	UCHAR *pbuf,
	OUT USHORT *pBufLen);

INT		Set_NfcStatus_Proc(
	IN  RTMP_ADAPTER		*pAd,
	IN  PSTRING			arg);

INT 	Set_NfcPasswdToken_Proc(
	IN  RTMP_ADAPTER		*pAd,
	IN  PSTRING			arg);

INT 	Set_NfcConfigurationToken_Proc(
	IN  RTMP_ADAPTER		*pAd,
	IN  PSTRING			arg);

INT 	Get_NfcStatus_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT		Set_DoWpsByNFC_Proc(
	IN  PRTMP_ADAPTER 	pAd, 
	IN  PSTRING 			arg);

INT Set_NfcRegenPK_Proc(
	IN PRTMP_ADAPTER 	pAd, 
	IN PSTRING 			arg);


#endif /* __WSC_NFC_H__ */
