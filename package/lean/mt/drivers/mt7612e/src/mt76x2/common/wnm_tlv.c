#include "rt_config.h"


VOID InsertDMSReqElement(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN WNM_DMS_REQUEST_ELEMENT DMSReqElement)
{
	ULONG TempLen;
	
	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						3,				&DMSReqElement,
						END_OF_ARGS);

/*	*pFrameLen = *pFrameLen + TempLen; */

	return;	
}



VOID WNM_InsertDMS(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Len,
	IN UCHAR DMSID,
	IN WNM_TCLAS wmn_tclas,
	IN ULONG IpAddr)
{
	ULONG TempLen;
	UINT8 DMSIDId = DMSID;
	WNM_TCLAS tclas;
	UCHAR 	TCLASElementID = WNM_CATEGORY;
	UCHAR 	TCLASLength = WNM_TCLAS_TYPE_IP_V4_LEN;

	tclas.UserPriority = 0;
	tclas.ClassifierType = 1;
	tclas.ClassifierMask = 255;
	tclas.Clasifier.IPv4.Version = 0;
	tclas.Clasifier.IPv4.IpSource = 0;
	tclas.Clasifier.IPv4.PortSource = 0;
	tclas.Clasifier.IPv4.PortDest = 0;
	tclas.Clasifier.IPv4.DSCP = 0;
	tclas.Clasifier.IPv4.Protocol = 4;

	tclas.Clasifier.IPv4.IpDest = IpAddr;
	
	MakeOutgoingFrame(	pFrameBuf,						&TempLen,
						1,								&DMSIDId,
						1,								&Len,
						1,								&TCLASElementID,
						1,								&TCLASLength,
						WNM_TCLAS_TYPE_IP_V4_LEN,				&tclas,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}


