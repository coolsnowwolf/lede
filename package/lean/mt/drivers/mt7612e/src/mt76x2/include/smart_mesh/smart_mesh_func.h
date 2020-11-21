#ifndef __SMART_MESH_FUNC_H__
#define __SMART_MESH_FUNC_H__

#ifdef SMART_MESH_MONITOR
INT Set_Enable_Monitor_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_MonitorTarget_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT Set_MonitorIndex_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_MonitorShowAll_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_MonitorClearCounter_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_MonitorTarget0_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_MonitorTarget1_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_MonitorTarget2_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_MonitorTarget3_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_MonitorTarget4_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_MonitorTarget5_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_MonitorTarget6_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT Smart_Mesh_Pkt_Report_Action(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR WCID,
	IN RXWI_STRUC	*pRxWI,	
	IN PUCHAR buff);

INT UpdateMonitorEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR MntWCID,
	IN UCHAR *MntAddr,
	IN BOOLEAN bClear);

BOOLEAN IsValidUnicastToMe(IN PRTMP_ADAPTER pAd,
                          IN UCHAR WCID,
                          IN PUCHAR pDA);
#endif /*SMART_MESH_MONITOR*/

#ifdef SMART_MESH
INT smart_mesh_init(PSMART_MESH_CFG pSmartMeshCfg);
INT smart_mesh_enable(PSMART_MESH_CFG pSmartMeshCfg,BOOLEAN Enable);
#ifdef MWDS
INT dwds_enable(PSMART_MESH_CFG pSmartMeshCfg, BOOLEAN Enable);
#endif /* MWDS */
#ifdef WSC_AP_SUPPORT
#ifdef SMART_MESH_HIDDEN_WPS
INT hidden_wps_enable(PSMART_MESH_CFG pSmartMeshCfg, BOOLEAN Enable);
INT Set_HiddenWps_State(PSMART_MESH_CFG pSmartMeshCfg, UINT8 state);
INT Set_HiddenWps_Role(PSMART_MESH_CFG pSmartMeshCfg, UINT8 role);
#endif /* SMART_MESH_HIDDEN_WPS */
#endif /* WSC_AP_SUPPORT */

INT Set_SmartMesh_IE_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
INT Set_SmartMesh_IE_Crtl_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
INT Set_SmartMesh_IE_Show_Proc(PRTMP_ADAPTER	pAd,PSTRING	arg);
INT Set_SmartMesh_Unicast_ProbeReq_SSID_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
INT Set_SmartMesh_Unicast_ProbeReq_Send_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
INT Set_SmartMesh_DFSScanAP_Enable_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
INT Set_SmartMesh_DFSScanAP_Show_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
VOID Set_Check_RadarChannelAP(PRTMP_ADAPTER pAd,UCHAR Ch,BSS_ENTRY *pBss);
VOID Set_Check_RadarChannelAPExist(PRTMP_ADAPTER pAd);
BOOLEAN IsRadarChannelAPExist(PRTMP_ADAPTER pAd, UCHAR Ch);
BOOLEAN Set_Scan_False_CCA(PRTMP_ADAPTER pAd,UCHAR ch_index, UCHAR type);
INT Set_SmartMesh_HyperFiPeer_Filter_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
INT Set_SmartMesh_HyperFiFlagMask_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
INT Set_SmartMesh_HyperFiFlagValue_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
#ifdef WSC_AP_SUPPORT
ULONG Get_BssSsidTableSearchByMAC(BSS_TABLE *Tab, UCHAR *pBssAddr);
INT Set_SmartMesh_WscPBCMAC_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
#endif /* WSC_AP_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT
INT Set_vMacPrefix_Show_Proc(PRTMP_ADAPTER	pAd,PSTRING	arg);
#endif /* MAC_REPEATER_SUPPORT */
VOID rtmp_read_smart_mesh_from_file(IN  PRTMP_ADAPTER pAd,PSTRING tmpbuf,PSTRING buffer);
#ifdef MWDS
VOID rtmp_read_DWDS_from_file(IN  PRTMP_ADAPTER pAd,PSTRING tmpbuf,PSTRING buffer);
#endif /* MWDS */

INT Set_Ap_SmartMesh_ACL_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
#ifdef APCLI_SUPPORT
INT Set_ApCli_SmartMesh_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg);

INT Set_ApCli_SmartMesh_Show_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg);

INT Set_ApCli_DWDS_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg);

#ifdef WSC_AP_SUPPORT
#ifdef SMART_MESH_HIDDEN_WPS
INT Set_ApCli_HiddenWPS_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg);

INT Set_ApCli_HiddenWPS_Show_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg);
#endif /* SMART_MESH_HIDDEN_WPS */
#endif /* WSC_AP_SUPPORT */
#endif /* APCLI_SUPPORT */

INT Set_Ap_SmartMesh_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg);

INT Set_Ap_SmartMesh_Show_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg);

INT Set_Ap_DWDS_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg);

#ifdef WSC_AP_SUPPORT
#ifdef SMART_MESH_HIDDEN_WPS
INT Set_Ap_HiddenWPS_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg);

INT Set_Ap_HiddenWPS_Show_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg);
#endif /* SMART_MESH_HIDDEN_WPS */
#endif /* WSC_AP_SUPPORT */

INT Set_Cli_Status_Show_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg);

VOID Update_CliPktStats(
	IN  PRTMP_ADAPTER pAd,
	IN  struct _MAC_TABLE_ENTRY *pEntry,
	IN  UINT32 MacSeq,
	IN  BOOLEAN bFromTx);

INT Set_Cli_Pkt_Stats_Enable_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg);

INT Set_Cli_Pkt_Stats_TX_Show_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg);

INT Set_Cli_Pkt_Stats_RX_Show_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg);

INT Set_MLME_Queue_Full_Show_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg);


#define SMART_MESH_INSERT_IE(_SmartMeshCfg, _pBuffer, _FrameLen, _ie_ctrl_flag)	\
{	\
	ULONG TmpLen,TotalLen;	\
	PSMART_MESH_CFG pSmartMeshCfg = &_SmartMeshCfg;	\
	PNTGR_IE pNtgr_IE = &pSmartMeshCfg->Ntgr_IE;\
\
	if ((pNtgr_IE->id == IE_VENDOR_SPECIFIC) && (pSmartMeshCfg->ie_ctrl_flags & _ie_ctrl_flag))	\
	{	\
		TotalLen = pNtgr_IE->len + 2;\
 		MakeOutgoingFrame(_pBuffer+_FrameLen, &TmpLen, TotalLen, pNtgr_IE, END_OF_ARGS);	\
 		_FrameLen += TmpLen;\
	}	\
}

#endif /* SMART_MESH */

#endif /* __SMART_MESH_FUNC_H__ */
