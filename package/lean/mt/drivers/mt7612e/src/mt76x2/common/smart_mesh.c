#ifdef SMART_MESH
#include "rt_config.h"
UCHAR NETGEAR_OUI[NTGR_OUI_LEN] = {0x00, 0x14, 0x6c};

INT smart_mesh_ie_init(PSMART_MESH_CFG pSmartMeshCfg)
{
	PNTGR_IE pNtgr_IE = &pSmartMeshCfg->Ntgr_IE;

	pSmartMeshCfg->ie_ctrl_flags = SM_IE_ALL;
	pNtgr_IE->id = IE_VENDOR_SPECIFIC;
	pNtgr_IE->len = NTGR_OUI_LEN;
	NdisCopyMemory(pNtgr_IE->oui, NETGEAR_OUI, sizeof(NETGEAR_OUI));
	
	return TRUE;
}

INT smart_mesh_init(PSMART_MESH_CFG pSmartMeshCfg)
{
	PNTGR_IE pNtgr_IE = &pSmartMeshCfg->Ntgr_IE;
	
	pSmartMeshCfg->bSupportSmartMesh = FALSE;
	smart_mesh_ie_init(pSmartMeshCfg);
	pNtgr_IE->len += 1; /* basic need 1 bytes*/
	return TRUE;
}

#ifdef MWDS
INT dwds_init(PSMART_MESH_CFG pSmartMeshCfg)
{
	PNTGR_IE pNtgr_IE = &pSmartMeshCfg->Ntgr_IE;
	
	smart_mesh_ie_init(pSmartMeshCfg);
	pNtgr_IE->len += 1; /* basic need 1 bytes*/
	return TRUE;
}
#endif /* MWDS */

#ifdef WSC_AP_SUPPORT
#ifdef SMART_MESH_HIDDEN_WPS
INT hidden_wps_init(PSMART_MESH_CFG pSmartMeshCfg)
{
	PNTGR_IE pNtgr_IE = &pSmartMeshCfg->Ntgr_IE;
	
	smart_mesh_ie_init(pSmartMeshCfg);
	pNtgr_IE->len += NTGR_CUSTOM_IE_MAX_LEN; /* basic need 3 bytes*/
	return TRUE;
} 
#endif /* SMART_MESH_HIDDEN_WPS */
#endif /* WSC_AP_SUPPORT */

INT smart_mesh_enable(PSMART_MESH_CFG pSmartMeshCfg, BOOLEAN Enable)
{
	PNTGR_IE pNtgr_IE = &pSmartMeshCfg->Ntgr_IE;

	pSmartMeshCfg->bSupportSmartMesh = Enable;
	if(Enable)
	{
		/* NTGR IE: Set SmartMesh bit */
		pNtgr_IE->private[0] |= 0x2; //SmartMesh bit

		DBGPRINT(RT_DEBUG_TRACE, (" SmartMesh Mode (ON)\n"));
	}
	else
	{
		/* Clear SmartMesh bit */
		pNtgr_IE->private[0] &= (~0x2); //SmartMesh bit
		DBGPRINT(RT_DEBUG_TRACE, (" SmartMesh Mode (OFF)\n"));
	}

	return TRUE;
}

#ifdef MWDS
INT dwds_enable(PSMART_MESH_CFG pSmartMeshCfg, BOOLEAN Enable)
{
	PNTGR_IE pNtgr_IE = &pSmartMeshCfg->Ntgr_IE;

	if(Enable)
	{
		/* NTGR IE: Set DWDS bit */
		pNtgr_IE->private[0] |= 0x1; //DWDS bit
		DBGPRINT(RT_DEBUG_TRACE, (" DWDS (ON)\n"));
	}
	else
	{
		/* Clear DWDS bit */
		pNtgr_IE->private[0] &= (~0x1); //DWDS bit
		DBGPRINT(RT_DEBUG_TRACE, (" DWDS (OFF)\n"));
	}
    
    return TRUE;
}
#endif /* MWDS */

#ifdef WSC_AP_SUPPORT
#ifdef SMART_MESH_HIDDEN_WPS
INT hidden_wps_enable(PSMART_MESH_CFG pSmartMeshCfg, BOOLEAN Enable)
{
    PNTGR_IE pNtgr_IE = &pSmartMeshCfg->Ntgr_IE;
    
    pSmartMeshCfg->bSupportHiddenWPS = Enable;
	if(Enable)
	{
		/* NTGR IE: Set Hidden WPS bit */
		pNtgr_IE->private[0] |= 0x4;
		DBGPRINT(RT_DEBUG_TRACE, (" HiddenWPS (ON)\n"));
	}
	else
	{
		/* Clear Hidden WPS bit */
		pNtgr_IE->private[0] &= (~0x4);
		DBGPRINT(RT_DEBUG_TRACE, (" HiddenWPS (OFF)\n"));
	}
    
    return TRUE;
}

INT Set_HiddenWps_State(PSMART_MESH_CFG pSmartMeshCfg, UINT8 state)
{
    PNTGR_IE pNtgr_IE = &pSmartMeshCfg->Ntgr_IE;
    
    if(pSmartMeshCfg->bSupportHiddenWPS &&
       (pNtgr_IE->len >= NTGR_CUSTOM_IE_MAX_LEN))
    {
         if(state == HIDDEN_WPS_STATE_STOP)
            pNtgr_IE->private[NTGR_CUSTOM_IE_MAX_LEN-1] &= (~HIDDEN_WPS_STATE_RUNNING);
         else if(state == HIDDEN_WPS_STATE_RUNNING)
            pNtgr_IE->private[NTGR_CUSTOM_IE_MAX_LEN-1] |= HIDDEN_WPS_STATE_RUNNING;
    }
    
    return TRUE;
}

INT Set_HiddenWps_Role(PSMART_MESH_CFG pSmartMeshCfg, UINT8 role)
{
    PNTGR_IE pNtgr_IE = &pSmartMeshCfg->Ntgr_IE;
    
    if(pSmartMeshCfg->bSupportHiddenWPS &&
       (pNtgr_IE->len >= NTGR_CUSTOM_IE_MAX_LEN))
    {
        if(role == HIDDEN_WPS_ROLE_ENROLLEE)
            pNtgr_IE->private[NTGR_CUSTOM_IE_MAX_LEN-1] &= (~HIDDEN_WPS_ROLE_REGISTRAR);
        else if(role == HIDDEN_WPS_ROLE_REGISTRAR)
           pNtgr_IE->private[NTGR_CUSTOM_IE_MAX_LEN-1] |= HIDDEN_WPS_ROLE_REGISTRAR;
    }
    
    return TRUE;
}
#endif /* SMART_MESH_HIDDEN_WPS */
#endif /* WSC_AP_SUPPORT */

INT	Set_Enable_SmartMesh_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  PSTRING arg,
	IN  BOOLEAN isAP)
{
	UCHAR Enable;
	POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	PSMART_MESH_CFG pSmartMeshCfg = NULL;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	
	if(isAP)
	{
		if(ifIndex < HW_BEACON_MAX_NUM)
			pSmartMeshCfg = &pAd->ApCfg.MBSSID[ifIndex].SmartMeshCfg;
	}
#ifdef APCLI_SUPPORT
	else
	{
		if (pObj->ioctl_if_type != INT_APCLI)
			return FALSE;
		
		if(ifIndex < MAX_APCLI_NUM)
			pSmartMeshCfg = &pAd->ApCfg.ApCliTab[ifIndex].SmartMeshCfg;
	}
#endif /* APCLI_SUPPORT */

	if(!pSmartMeshCfg)
		return FALSE;

	Enable = simple_strtol(arg, 0, 10);
	if (Enable)
	{
		smart_mesh_init(pSmartMeshCfg);
		smart_mesh_enable(pSmartMeshCfg,TRUE);
	}
	else
	{
		smart_mesh_enable(pSmartMeshCfg,FALSE);
	}
	DBGPRINT(RT_DEBUG_TRACE, (" SmartMesh Mode = %d \n",pSmartMeshCfg->bSupportSmartMesh));

	return TRUE;
}

INT	Set_Enable_DWDS_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  PSTRING arg,
	IN  BOOLEAN isAP)
{
	UCHAR Enable;
	POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	PSMART_MESH_CFG pSmartMeshCfg = NULL;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	
	if(isAP)
	{
		if(ifIndex < HW_BEACON_MAX_NUM)
			pSmartMeshCfg = &pAd->ApCfg.MBSSID[ifIndex].SmartMeshCfg;
	}
#ifdef APCLI_SUPPORT
	else
	{
		if (pObj->ioctl_if_type != INT_APCLI)
			return FALSE;
		
		if(ifIndex < MAX_APCLI_NUM)
			pSmartMeshCfg = &pAd->ApCfg.ApCliTab[ifIndex].SmartMeshCfg;
	}
#endif /* APCLI_SUPPORT */

	if(!pSmartMeshCfg)
		return FALSE;

	Enable = simple_strtol(arg, 0, 10);
#ifdef MWDS
	if (Enable)
	{
		dwds_init(pSmartMeshCfg);
		dwds_enable(pSmartMeshCfg,TRUE);
		MWDSEnable(pAd,ifIndex,isAP);
	}
	else
	{
		dwds_enable(pSmartMeshCfg,FALSE);
		MWDSDisable(pAd,ifIndex,isAP);
	}
#endif /* MWDS */
	DBGPRINT(RT_DEBUG_TRACE, (" DWDS mode = %d \n",Enable));

	return TRUE;
}

#ifdef WSC_AP_SUPPORT
#ifdef SMART_MESH_HIDDEN_WPS
INT	Set_Enable_HiddenWPS_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  PSTRING arg,
	IN  BOOLEAN isAP)
{
    UCHAR Enable;
	POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	PSMART_MESH_CFG pSmartMeshCfg = NULL;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	
	if(isAP)
	{
		if(ifIndex < HW_BEACON_MAX_NUM)
			pSmartMeshCfg = &pAd->ApCfg.MBSSID[ifIndex].SmartMeshCfg;
	}
#ifdef APCLI_SUPPORT
	else
	{
		if (pObj->ioctl_if_type != INT_APCLI)
			return FALSE;
		
		if(ifIndex < MAX_APCLI_NUM)
			pSmartMeshCfg = &pAd->ApCfg.ApCliTab[ifIndex].SmartMeshCfg;
	}
#endif /* APCLI_SUPPORT */

	if(!pSmartMeshCfg)
		return FALSE;

    Enable = simple_strtol(arg, 0, 10);
    if (Enable)
	{
		hidden_wps_init(pSmartMeshCfg);
		hidden_wps_enable(pSmartMeshCfg,TRUE);
	}
	else
	{
	    Set_HiddenWps_State(pSmartMeshCfg, HIDDEN_WPS_STATE_STOP);
        Set_HiddenWps_Role(pSmartMeshCfg, HIDDEN_WPS_ROLE_ENROLLEE);
		hidden_wps_enable(pSmartMeshCfg,FALSE);
	}

    DBGPRINT(RT_DEBUG_TRACE, (" HiddenWPS mode = %d \n",Enable));
    
    return TRUE;
}
#endif /* SMART_MESH_HIDDEN_WPS */
#endif /* WSC_AP_SUPPORT */

#ifdef APCLI_SUPPORT
INT Set_ApCli_SmartMesh_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{
	return Set_Enable_SmartMesh_Proc(pAd,arg,FALSE);
}

INT Set_ApCli_SmartMesh_Show_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{
	POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	PSMART_MESH_CFG pSmartMeshCfg = NULL;
	PNTGR_IE pNtgr_IE = NULL;
	BOOLEAN bEnable;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	if((pObj->ioctl_if_type == INT_APCLI) && (ifIndex < MAX_APCLI_NUM))
		pSmartMeshCfg = &pAd->ApCfg.ApCliTab[ifIndex].SmartMeshCfg;
	else
		return FALSE;

	if(pSmartMeshCfg)
	{
		pNtgr_IE = &pSmartMeshCfg->Ntgr_IE;
		if(pSmartMeshCfg->bSupportSmartMesh && (pNtgr_IE->private[0]&0x2))
			bEnable = TRUE;
		else
			bEnable = FALSE;
		printk("%d\n",bEnable);
	}

	return TRUE;
}

INT Set_ApCli_DWDS_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{
	return Set_Enable_DWDS_Proc(pAd,arg,FALSE);
}

#ifdef WSC_AP_SUPPORT
#ifdef SMART_MESH_HIDDEN_WPS
INT Set_ApCli_HiddenWPS_Proc(
   IN  PRTMP_ADAPTER pAd, 
   IN  PSTRING arg)
{
    return Set_Enable_HiddenWPS_Proc(pAd,arg,FALSE);
}

INT Set_ApCli_HiddenWPS_Show_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{
	
	POS_COOKIE		pObj;
	UCHAR			ifIndex;
	PSMART_MESH_CFG pSmartMeshCfg = NULL;
	PNTGR_IE pNtgr_IE = NULL;
	BOOLEAN bEnable;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	if((pObj->ioctl_if_type == INT_APCLI) && (ifIndex < MAX_APCLI_NUM))
		pSmartMeshCfg = &pAd->ApCfg.ApCliTab[ifIndex].SmartMeshCfg;
	else
		return FALSE;

	if(pSmartMeshCfg)
	{
		pNtgr_IE = &pSmartMeshCfg->Ntgr_IE;
		if(pSmartMeshCfg->bSupportHiddenWPS && (pNtgr_IE->private[0]&0x4))
			bEnable = TRUE;
		else
			bEnable = FALSE;
		printk("%d\n",bEnable);
	}

	return TRUE;
}
#endif /* SMART_MESH_HIDDEN_WPS */
#endif /* WSC_AP_SUPPORT */
#endif /* APCLI_SUPPORT */

INT Set_Ap_SmartMesh_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{
	return Set_Enable_SmartMesh_Proc(pAd,arg,TRUE);
}

INT Set_Ap_SmartMesh_Show_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{
	POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	PSMART_MESH_CFG pSmartMeshCfg = NULL;
	PNTGR_IE pNtgr_IE = NULL;
	BOOLEAN bEnable;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	if(ifIndex < HW_BEACON_MAX_NUM)
		pSmartMeshCfg = &pAd->ApCfg.MBSSID[ifIndex].SmartMeshCfg;
	else
		return FALSE;

	if(pSmartMeshCfg)
	{
		pNtgr_IE = &pSmartMeshCfg->Ntgr_IE;
		if(pSmartMeshCfg->bSupportSmartMesh && (pNtgr_IE->private[0]&0x2))
			bEnable = TRUE;
		else
			bEnable = FALSE;
		printk("%d\n",bEnable);
	}

	return TRUE;
}

INT Set_Ap_DWDS_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{
	return Set_Enable_DWDS_Proc(pAd,arg,TRUE);
}

#ifdef WSC_AP_SUPPORT
#ifdef SMART_MESH_HIDDEN_WPS
INT Set_Ap_HiddenWPS_Proc(
   IN  PRTMP_ADAPTER pAd, 
   IN  PSTRING arg)
{
    return Set_Enable_HiddenWPS_Proc(pAd,arg,TRUE);
}

INT Set_Ap_HiddenWPS_Show_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{
    POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	PSMART_MESH_CFG pSmartMeshCfg = NULL;
	PNTGR_IE pNtgr_IE = NULL;
	BOOLEAN bEnable;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	if(ifIndex < HW_BEACON_MAX_NUM)
		pSmartMeshCfg = &pAd->ApCfg.MBSSID[ifIndex].SmartMeshCfg;
	else
		return FALSE;

	if(pSmartMeshCfg)
	{
		pNtgr_IE = &pSmartMeshCfg->Ntgr_IE;
		if(pSmartMeshCfg->bSupportHiddenWPS && (pNtgr_IE->private[0]&0x4))
			bEnable = TRUE;
		else
			bEnable = FALSE;
		printk("%d\n",bEnable);
	}

	return TRUE;
}
#endif /* SMART_MESH_HIDDEN_WPS */
#endif /* WSC_AP_SUPPORT */

INT Set_Ap_SmartMesh_ACL_Proc(PRTMP_ADAPTER pAd, PSTRING arg)
{
	UCHAR Enable = FALSE;

	POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	
	ifIndex = pObj->ioctl_if;
	Enable = simple_strtol(arg, 0, 10);
	
	switch (pObj->ioctl_if_type)
	{
		case INT_MAIN:
		case INT_MBSSID:
			if(ifIndex < HW_BEACON_MAX_NUM)
				pAd->ApCfg.MBSSID[ifIndex].bSmartMeshACL = Enable;
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR,("%s(): Invalid interface (%u).\n",__FUNCTION__, pObj->ioctl_if_type));
			return FALSE;
			break;
	}
	
	DBGPRINT(RT_DEBUG_TRACE, (" bSmartMeshACL = %d \n",Enable));

	return TRUE;
}

INT smart_mesh_set_ie(PRTMP_ADAPTER	pAd, PSMART_MESH_CFG pSmartMeshCfg,
                            PUINT8 value,INT len, BOOLEAN isAP)
{
	PNTGR_IE pNtgr_IE = &pSmartMeshCfg->Ntgr_IE;
    UINT8 CurPrivLen = 0,CustomContent = 0;

	if((value != NULL) && (len > 0))
	{
		if(len > NTGR_IE_PRIV_LEN)
		{
			DBGPRINT(RT_DEBUG_ERROR, 
					("The input data length over buffer size! (Maximum is %d)\n",NTGR_IE_PRIV_LEN));
			return FALSE;
		}
        if(pNtgr_IE->len >= NTGR_OUI_LEN)
            CurPrivLen = pNtgr_IE->len - NTGR_OUI_LEN;
        
        if(CurPrivLen >= NTGR_CUSTOM_IE_MAX_LEN)
            CustomContent = pNtgr_IE->private[NTGR_CUSTOM_IE_MAX_LEN-1];

#ifdef MWDS
        /* DWDS */
        if((*value & 0x1))
            Set_Enable_DWDS_Proc(pAd, "1", isAP);
        else
            Set_Enable_DWDS_Proc(pAd, "0", isAP);
#endif /* MWDS */  

        /* Smart Mesh */
        if((*value & 0x2))
            Set_Enable_SmartMesh_Proc(pAd, "1", isAP);
        else
            Set_Enable_SmartMesh_Proc(pAd, "0", isAP);

#ifdef WSC_AP_SUPPORT
#ifdef SMART_MESH_HIDDEN_WPS
         /* Hidden WPS */
        if((*value & 0x4))
            Set_Enable_HiddenWPS_Proc(pAd, "1", isAP);
        else
            Set_Enable_HiddenWPS_Proc(pAd, "0", isAP);
#endif /* SMART_MESH_HIDDEN_WPS */
#endif /* WSC_AP_SUPPORT */

        smart_mesh_ie_init(pSmartMeshCfg);
		NdisZeroMemory(pNtgr_IE->private,NTGR_IE_PRIV_LEN);
		NdisCopyMemory(pNtgr_IE->private, value, len);
		pNtgr_IE->len += len;

         if(pNtgr_IE->len >= NTGR_OUI_LEN)
            CurPrivLen = pNtgr_IE->len - NTGR_OUI_LEN;

        if(CurPrivLen < NTGR_CUSTOM_IE_MAX_LEN)
            pNtgr_IE->len = (NTGR_OUI_LEN + NTGR_CUSTOM_IE_MAX_LEN);

        pNtgr_IE->private[NTGR_CUSTOM_IE_MAX_LEN-1] = CustomContent;
	}

	return TRUE;
}

INT Set_SmartMesh_IE_Proc(
	PRTMP_ADAPTER	pAd,
	PSTRING			arg)
{
	INT len,private_Len = 0;	
	POS_COOKIE 		pObj;
	UCHAR 			ifIndex,buf[256],private[NTGR_IE_PRIV_LEN];
	PSMART_MESH_CFG pSmartMeshCfg = NULL;
	PNTGR_IE pNtgr_IE;
    BOOLEAN isAP = FALSE;
	
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	switch (pObj->ioctl_if_type)
	{
		case INT_MAIN:
		case INT_MBSSID:
			if(ifIndex < HW_BEACON_MAX_NUM)
            {         
				pSmartMeshCfg = &pAd->ApCfg.MBSSID[ifIndex].SmartMeshCfg;
                isAP = TRUE;
            }
			break;
#ifdef APCLI_SUPPORT
		case INT_APCLI:
			if(ifIndex < MAX_APCLI_NUM)
				pSmartMeshCfg = &pAd->ApCfg.ApCliTab[ifIndex].SmartMeshCfg;
			break;
#endif /* APCLI_SUPPORT */
		default:
			DBGPRINT(RT_DEBUG_ERROR,("%s(): Invalid interface (%u).\n",__FUNCTION__, pObj->ioctl_if_type));
			return FALSE;
			break;
	}

	if(!pSmartMeshCfg)
		return FALSE;

	strncpy(buf,arg,sizeof(buf));
	len = strlen(buf);
	if(len > 0)
	{
		if((len%2 != 0))
		{
			buf[len]='0';
			len++;
		}
		
		AtoH(buf, private, len);
		private_Len = len/2;
		smart_mesh_set_ie(pAd, pSmartMeshCfg, private, private_Len, isAP);
	}
	return TRUE;
}


/* 
	bit 0 - Beacon
	bit 1 - Probe Rsp
	bit 2 - Assoc Rsp
	bit 3 - Auth Rsp
	bit 4 - Probe Req
	bit 5 - Assoc Req
	Ex iwpriv ra0/apcli0 set SmIeCtrl=1f
*/
INT Set_SmartMesh_IE_Crtl_Proc(
	PRTMP_ADAPTER	pAd,
	PSTRING			arg)
{
	POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	UINT32			ie_ctrl_flags;
	PSMART_MESH_CFG pSmartMeshCfg = NULL;
	
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	ie_ctrl_flags = simple_strtol(arg, 0, 16);

	switch (pObj->ioctl_if_type)
	{
		case INT_MAIN:
		case INT_MBSSID:
			if(ifIndex < HW_BEACON_MAX_NUM)
				pSmartMeshCfg = &pAd->ApCfg.MBSSID[ifIndex].SmartMeshCfg;
			break;
#ifdef APCLI_SUPPORT
		case INT_APCLI:
			if(ifIndex < MAX_APCLI_NUM)
				pSmartMeshCfg = &pAd->ApCfg.ApCliTab[ifIndex].SmartMeshCfg;
			break;
#endif /* APCLI_SUPPORT */
		default:
			DBGPRINT(RT_DEBUG_ERROR,("%s(): Invalid interface (%u).\n",__FUNCTION__, pObj->ioctl_if_type));
			return FALSE;
			break;
	}

	if (!pSmartMeshCfg)
		return FALSE;
	
	pSmartMeshCfg->ie_ctrl_flags = ie_ctrl_flags;
	DBGPRINT(RT_DEBUG_TRACE, ("%s(): ie_ctrl_flags = 0x%08x\n",__FUNCTION__, pSmartMeshCfg->ie_ctrl_flags));

	return TRUE;
}

INT Set_SmartMesh_IE_Show_Proc(
	PRTMP_ADAPTER	pAd,
	PSTRING			arg)
{
	POS_COOKIE 		pObj;
	UCHAR 			ifIndex,i;
	PSMART_MESH_CFG pSmartMeshCfg = NULL;
	PNTGR_IE pNtgr_IE = NULL;
	
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	switch (pObj->ioctl_if_type)
	{
		case INT_MAIN:
		case INT_MBSSID:
			if(ifIndex < HW_BEACON_MAX_NUM)
				pSmartMeshCfg = &pAd->ApCfg.MBSSID[ifIndex].SmartMeshCfg;
			break;
#ifdef APCLI_SUPPORT
		case INT_APCLI:
			if(ifIndex < MAX_APCLI_NUM)
				pSmartMeshCfg = &pAd->ApCfg.ApCliTab[ifIndex].SmartMeshCfg;
			break;
#endif /* APCLI_SUPPORT */
		default:
			DBGPRINT(RT_DEBUG_ERROR,("%s(): Invalid interface (%u).\n",__FUNCTION__, pObj->ioctl_if_type));
			return FALSE;
			break;
	}

	if (!pSmartMeshCfg)
		return FALSE;
	
	pNtgr_IE = &pSmartMeshCfg->Ntgr_IE;
	for(i=0;i<NTGR_IE_PRIV_LEN;i++)
		printk("%02x ",pNtgr_IE->private[i]);
	printk("\n");

#ifdef ACOS_PATCH /* foxconn */
	if (arg && copy_to_user(arg, pNtgr_IE, sizeof(NTGR_IE)))
		return FALSE;
#endif

	return TRUE;
}

INT Set_SmartMesh_Unicast_ProbeReq_SSID_Proc(PRTMP_ADAPTER pAd, PSTRING arg)
{
	POS_COOKIE 		pObj;
	UCHAR 			ifIndex,i;
	PSMART_MESH_CFG pSmartMeshCfg = NULL;
	PUNI_PROBE_REQ_CFG pUniProbeReqCfg = NULL;
	PNTGR_IE pNtgr_IE = NULL;
	UINT8 ssid_len = 0;

	ssid_len = strlen(arg);
	if(ssid_len > MAX_LEN_OF_SSID)
		return FALSE;
		
	
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	switch (pObj->ioctl_if_type)
	{
		case INT_MAIN:
		case INT_MBSSID:
			if(ifIndex < HW_BEACON_MAX_NUM)
				pSmartMeshCfg = &pAd->ApCfg.MBSSID[ifIndex].SmartMeshCfg;
			break;
#ifdef APCLI_SUPPORT
		case INT_APCLI:
			if(ifIndex < MAX_APCLI_NUM)
				pSmartMeshCfg = &pAd->ApCfg.ApCliTab[ifIndex].SmartMeshCfg;
			break;
#endif /* APCLI_SUPPORT */
		default:
			DBGPRINT(RT_DEBUG_ERROR,("%s(): Invalid interface (%u).\n",__FUNCTION__, pObj->ioctl_if_type));
			return FALSE;
			break;
	}

	if (!pSmartMeshCfg)
		return FALSE;

	pUniProbeReqCfg = &pSmartMeshCfg->UniProbeReqCfg;
	
	NdisZeroMemory(pUniProbeReqCfg->ssid, sizeof(pUniProbeReqCfg->ssid));
	NdisMoveMemory(pUniProbeReqCfg->ssid, arg, ssid_len);
	pUniProbeReqCfg->ssid_len = ssid_len;

	return TRUE;
}

INT Enqueue_SmartMesh_UnicastProbeReq(RTMP_ADAPTER *pAd, UCHAR ScanType, INT IfType, INT ifIndex)
{
	UCHAR *frm_buf = NULL, SsidLen = 0;
	HEADER_802_11 Hdr80211;
	ULONG FrameLen = 0;
	PUNI_PROBE_REQ_CFG pUniProbeReqCfg = NULL;

	if(IfType == INT_APCLI)
		pUniProbeReqCfg = &pAd->ApCfg.ApCliTab[ifIndex].SmartMeshCfg.UniProbeReqCfg;
	else
		pUniProbeReqCfg = &pAd->ApCfg.MBSSID[ifIndex].SmartMeshCfg.UniProbeReqCfg;

	if (!pUniProbeReqCfg)
		return FALSE;

	if (MlmeAllocateMemory(pAd, &frm_buf) != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s(): allocate memory fail\n",__func__));
		return FALSE;
	}
	
	if(IfType == INT_APCLI)
		pAd->ApCfg.ApCliTab[ifIndex].SyncCurrState = APCLI_SYNC_IDLE;
	pAd->MlmeAux.Channel = pAd->CommonCfg.Channel;
	
	SsidLen = pUniProbeReqCfg->ssid_len;
	MgtMacHeaderInitExt(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, pUniProbeReqCfg->addr1, 
						pUniProbeReqCfg->addr2,BROADCAST_ADDR);
	
	MakeOutgoingFrame(frm_buf,               &FrameLen,
					  sizeof(HEADER_802_11),    &Hdr80211,
					  1,                        &SsidIe,
					  1,                        &SsidLen,
					  SsidLen,			        pUniProbeReqCfg->ssid,
					  1,                        &SupRateIe,
					  1,                        &pAd->CommonCfg.SupRateLen,
					  pAd->CommonCfg.SupRateLen,  pAd->CommonCfg.SupRate, 
					  END_OF_ARGS);

	if (pAd->CommonCfg.ExtRateLen)
	{
		ULONG Tmp;
		MakeOutgoingFrame(frm_buf + FrameLen,            &Tmp,
						  1,                                &ExtRateIe,
						  1,                                &pAd->CommonCfg.ExtRateLen,
						  pAd->CommonCfg.ExtRateLen,          pAd->CommonCfg.ExtRate, 
						  END_OF_ARGS);
		FrameLen += Tmp;
	}
#ifdef DOT11_N_SUPPORT
	if (WMODE_CAP_N(pAd->CommonCfg.PhyMode))
	{
		ULONG	Tmp;
		UCHAR	HtLen;
		UCHAR	BROADCOM[4] = {0x0, 0x90, 0x4c, 0x33};
#ifdef RT_BIG_ENDIAN
		HT_CAPABILITY_IE HtCapabilityTmp;
#endif
		if (pAd->bBroadComHT == TRUE)
		{
			HtLen = pAd->MlmeAux.HtCapabilityLen + 4;
#ifdef RT_BIG_ENDIAN
			NdisMoveMemory(&HtCapabilityTmp, &pAd->MlmeAux.HtCapability, SIZE_HT_CAP_IE);
			*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
			{
				EXT_HT_CAP_INFO extHtCapInfo;

				NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
				*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
				NdisMoveMemory((PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));		
			}
#else				
			*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = cpu2le16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */

			MakeOutgoingFrame(frm_buf + FrameLen,          &Tmp,
							1,                                &WpaIe,
							1,                                &HtLen,
							4,                                &BROADCOM[0],
							pAd->MlmeAux.HtCapabilityLen,     &HtCapabilityTmp, 
							END_OF_ARGS);
#else
			MakeOutgoingFrame(frm_buf + FrameLen,          &Tmp,
							1,                                &WpaIe,
							1,                                &HtLen,
							4,                                &BROADCOM[0],
							pAd->MlmeAux.HtCapabilityLen,     &pAd->MlmeAux.HtCapability, 
							END_OF_ARGS);
#endif /* RT_BIG_ENDIAN */
		}
		else				
		{
			HtLen = sizeof(HT_CAPABILITY_IE);
#ifdef RT_BIG_ENDIAN
			NdisMoveMemory(&HtCapabilityTmp, &pAd->CommonCfg.HtCapability, SIZE_HT_CAP_IE);
			*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
			{
				EXT_HT_CAP_INFO extHtCapInfo;

				NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
				*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
				NdisMoveMemory((PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));		
			}
#else				
			*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = cpu2le16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */

			MakeOutgoingFrame(frm_buf + FrameLen,          &Tmp,
							1,                                &HtCapIe,
							1,                                &HtLen,
							HtLen,                            &HtCapabilityTmp, 
							END_OF_ARGS);
#else
			MakeOutgoingFrame(frm_buf + FrameLen,          &Tmp,
							1,                                &HtCapIe,
							1,                                &HtLen,
							HtLen,                            &pAd->CommonCfg.HtCapability, 
							END_OF_ARGS);
#endif /* RT_BIG_ENDIAN */
		}
		FrameLen += Tmp;

#ifdef DOT11N_DRAFT3
		if ((pAd->MlmeAux.Channel <= 14) && (pAd->CommonCfg.bBssCoexEnable == TRUE))
		{
			ULONG Tmp;
			HtLen = 1;
			MakeOutgoingFrame(frm_buf + FrameLen,            &Tmp,
							  1,					&ExtHtCapIe,
							  1,					&HtLen,
							  1,          			&pAd->CommonCfg.BSSCoexist2040.word, 
							  END_OF_ARGS);

			FrameLen += Tmp;
		}
#endif /* DOT11N_DRAFT3 */
	}
#endif /* DOT11_N_SUPPORT */

#ifdef APCLI_SUPPORT
#ifdef WSC_INCLUDED
	if (ScanType == SCAN_WSC_ACTIVE)
	{
		BOOLEAN bHasWscIe = FALSE;
		/* 
			Append WSC information in probe request if WSC state is running
		*/
		if (pAd->ApCfg.ApCliTab[0].WscControl.bWscTrigger)
		{
			bHasWscIe = TRUE;
		}
#ifdef WSC_V2_SUPPORT
		else if (pAd->ApCfg.ApCliTab[0].WscControl.WscV2Info.bEnableWpsV2)
		{
			bHasWscIe = TRUE;	
		}
#endif /* WSC_V2_SUPPORT */

#ifdef WSC_AP_SUPPORT
#ifdef SMART_MESH_HIDDEN_WPS
        if(pAd->ApCfg.ApCliTab[ifIndex].SmartMeshCfg.bSupportHiddenWPS)
            bHasWscIe = FALSE;
#endif /* SMART_MESH_HIDDEN_WPS */
#endif /* WSC_AP_SUPPORT */


		if (bHasWscIe)
		{
			UCHAR		*pWscBuf = NULL, WscIeLen = 0;
			ULONG 		WscTmpLen = 0;

			os_alloc_mem(NULL, (UCHAR **)&pWscBuf, 512);
			if (pWscBuf != NULL)
			{
				NdisZeroMemory(pWscBuf, 512);
				WscBuildProbeReqIE(pAd, STA_MODE, pWscBuf, &WscIeLen);

				MakeOutgoingFrame(frm_buf + FrameLen,              &WscTmpLen,
								WscIeLen,                             pWscBuf,
								END_OF_ARGS);

				FrameLen += WscTmpLen;
				os_free_mem(NULL, pWscBuf);
			}
			else
				DBGPRINT(RT_DEBUG_WARN, ("%s:: WscBuf Allocate failed!\n", __FUNCTION__));
		}
	}
#endif /* WSC_INCLUDED */			
#endif /* APCLI_SUPPORT */

#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode) &&
		(pAd->MlmeAux.Channel > 14)) {		
		FrameLen += build_vht_ies(pAd, (UCHAR *)(frm_buf + FrameLen), SUBTYPE_PROBE_REQ);
	}
#endif /* DOT11_VHT_AC */

	if (IfType == INT_APCLI)
	{
		SMART_MESH_INSERT_IE(pAd->ApCfg.ApCliTab[ifIndex].SmartMeshCfg,
						frm_buf,
						FrameLen,
						SM_IE_PROBE_REQ);
	}
	else
	{
		SMART_MESH_INSERT_IE(pAd->ApCfg.MBSSID[ifIndex].SmartMeshCfg,
						frm_buf,
						FrameLen,
						SM_IE_PROBE_REQ);
	}

	MiniportMMRequest(pAd, 0, frm_buf, FrameLen);

	MlmeFreeMemory(pAd, frm_buf);

	return TRUE;
}

INT Set_SmartMesh_Unicast_ProbeReq_Send_Proc(PRTMP_ADAPTER pAd, PSTRING arg)
{
	POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	PSMART_MESH_CFG pSmartMeshCfg = NULL;
	PUNI_PROBE_REQ_CFG pUniProbeReqCfg = NULL;
	PSTRING value;
	INT i;

	if (strlen(arg) != 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
	{
		DBGPRINT(RT_DEBUG_ERROR, ("illegal MAC address length! (acceptable format 01:02:03:04:05:06 length 17)\n"));
		return FALSE;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	switch (pObj->ioctl_if_type)
	{
		case INT_MAIN:
		case INT_MBSSID:
			if(ifIndex < HW_BEACON_MAX_NUM)
				pSmartMeshCfg = &pAd->ApCfg.MBSSID[ifIndex].SmartMeshCfg;
			break;
#ifdef APCLI_SUPPORT
		case INT_APCLI:
			if(ifIndex < MAX_APCLI_NUM)
				pSmartMeshCfg = &pAd->ApCfg.ApCliTab[ifIndex].SmartMeshCfg;
			break;
#endif /* APCLI_SUPPORT */
		default:
			DBGPRINT(RT_DEBUG_ERROR,("%s(): Invalid interface (%u).\n",__FUNCTION__, pObj->ioctl_if_type));
			return FALSE;
			break;
	}

	if (!pSmartMeshCfg)
		return FALSE;

	pUniProbeReqCfg = &pSmartMeshCfg->UniProbeReqCfg;
	
	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":")) 
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
		{
			DBGPRINT(RT_DEBUG_ERROR, ("illegal MAC address format or octet!\n"));
			/* Do not use "continue" to replace "break" */
			break;
		}
		AtoH(value, &pUniProbeReqCfg->addr1[i++], 1);
	}

	if (i != MAC_ADDR_LEN)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("MAC address length is wrong!\n"));
		return FALSE;
	}

	if(pObj->ioctl_if_type == INT_APCLI)
		COPY_MAC_ADDR(pUniProbeReqCfg->addr2,pAd->ApCfg.ApCliTab[ifIndex].wdev.if_addr);
	else
		COPY_MAC_ADDR(pUniProbeReqCfg->addr2,pAd->ApCfg.MBSSID[ifIndex].wdev.bssid);
		
	return Enqueue_SmartMesh_UnicastProbeReq(pAd,pAd->MlmeAux.ScanType,pObj->ioctl_if_type,ifIndex);
}

INT Set_SmartMesh_DFSScanAP_Enable_Proc(PRTMP_ADAPTER pAd, PSTRING arg)
{
	INT i;
	BOOLEAN Enable = FALSE;

	if (pAd->CommonCfg.bIEEE80211H == TRUE)
	{
		Enable = simple_strtol(arg, 0, 10);
		/* Reset all DFS channel first */
		for (i=0; i < pAd->ChannelListNum; i++)
		{
			if(pAd->ChannelList[i].DfsReq)
				pAd->ChannelList[i].bDfsAPExist = FALSE;
		}

		pAd->ApCfg.bDFSAPScanEnable = Enable;
	}
	
	return TRUE;
}

INT Set_SmartMesh_DFSScanAP_Show_Proc(PRTMP_ADAPTER pAd,PSTRING arg)
{
	INT i,j,count;
	UCHAR Ch = 0;
	BSS_TABLE *pTab = &pAd->ScanTab;
	BSS_ENTRY *pBss = NULL;
	
	if(pAd->CommonCfg.bIEEE80211H == TRUE)
	{
		for (i=0; i < pAd->ChannelListNum; i++)
		{
			if (pAd->ChannelList[i].DfsReq && 
			    (pAd->ChannelList[i].Channel > 14) && 
				(pAd->ChannelList[i].bDfsAPExist))
			{
				count = 0;
				Ch = pAd->ChannelList[i].Channel;
				for (j = 0; j < pTab->BssNr; j++) 
				{
					pBss = &pTab->BssEntry[j];
					if ((pBss->Channel == Ch) && pBss->bDfsAP) 
						count++;
				}
				printk("Channel:%d\tCount:%d\n",Ch,count);
			}
		}
	}

	return TRUE;
}

VOID Set_Check_RadarChannelAP(PRTMP_ADAPTER pAd,UCHAR Ch,BSS_ENTRY *pBss)
{
	INT i;

	if(pAd->ApCfg.bDFSAPScanEnable != TRUE)
		return;

	if(pBss)
	{
		if ((Ch > 14) && (pAd->CommonCfg.bIEEE80211H == TRUE))
		{
			for (i=0; i < pAd->ChannelListNum; i++)
			{
				if (Ch == pAd->ChannelList[i].Channel)
				{
					if(pAd->ChannelList[i].DfsReq)
						pBss->bDfsAP = TRUE;
					else
						pBss->bDfsAP = FALSE;
					break;
				}
			}
		}
	}
}

VOID Set_Check_RadarChannelAPExist(PRTMP_ADAPTER pAd)
{
	INT i,j;
	UCHAR Ch = 0;
	BSS_TABLE *pTab = &pAd->ScanTab;
	BSS_ENTRY *pBss = NULL;

	if(pAd->ApCfg.bDFSAPScanEnable != TRUE)
		return;
	
	if(pAd->CommonCfg.bIEEE80211H == TRUE)
	{
		for (i=0; i < pAd->ChannelListNum; i++)
		{
			if (pAd->ChannelList[i].DfsReq && 
			    (pAd->ChannelList[i].Channel > 14))
			{
				pAd->ChannelList[i].bDfsAPExist = FALSE;
				Ch = pAd->ChannelList[i].Channel;
				for (j = 0; j < pTab->BssNr; j++) 
				{
					pBss = &pTab->BssEntry[j];
					if (pBss->bDfsAP)
					{
						pAd->ChannelList[i].bDfsAPExist = TRUE;
						break;
					}
				}
			}
		}
	}
}

BOOLEAN IsRadarChannelAPExist(PRTMP_ADAPTER pAd,UCHAR Ch)
{
	INT i;
	BOOLEAN result = FALSE;

	if(pAd->ApCfg.bDFSAPScanEnable != TRUE)
		return FALSE;
	
	if ((Ch > 14) && (pAd->CommonCfg.bIEEE80211H == TRUE))
	{
		for (i=0; i < pAd->ChannelListNum; i++)
		{
			if (Ch == pAd->ChannelList[i].Channel)
			{
				result = pAd->ChannelList[i].bDfsAPExist;
				break;
			}
		}
	}

	return result;
}

BOOLEAN Set_Scan_False_CCA(PRTMP_ADAPTER pAd,UCHAR ch_index, UCHAR type)
{
	INT i;

	if (type == CCA_RESET)
	{
		for (i = 0;i < pAd->ChannelListNum;i++)
		{
			pAd->ChannelList[i].FalseCCA = 0;
		}
		pAd->ApCfg.scan_channel_index = 0;
	}

	if (type == CCA_STORE)
	{
        RX_STA_CNT1_STRUC RxStaCnt1;
        RTMP_IO_READ32(pAd, RX_STA_CNT1, &RxStaCnt1.word);
        pAd->ChannelList[ch_index].FalseCCA += RxStaCnt1.field.FalseCca;
    }   

	return TRUE;
}

INT Set_SmartMesh_HyperFiPeer_Filter_Proc(PRTMP_ADAPTER pAd, PSTRING arg)
{
	POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	PSMART_MESH_CFG pSmartMeshCfg = NULL;
	UCHAR Enable = FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	switch (pObj->ioctl_if_type)
	{
		case INT_MAIN:
		case INT_MBSSID:
			if(ifIndex < HW_BEACON_MAX_NUM)
				pSmartMeshCfg = &pAd->ApCfg.MBSSID[ifIndex].SmartMeshCfg;
			break;
#ifdef APCLI_SUPPORT
		case INT_APCLI:
			if(ifIndex < MAX_APCLI_NUM)
				pSmartMeshCfg = &pAd->ApCfg.ApCliTab[ifIndex].SmartMeshCfg;
			break;
#endif /* APCLI_SUPPORT */
		default:
			DBGPRINT(RT_DEBUG_ERROR,("%s(): Invalid interface (%u).\n",__FUNCTION__, pObj->ioctl_if_type));
			return FALSE;
			break;
	}

	if (!pSmartMeshCfg)
		return FALSE;

	Enable = simple_strtol(arg, 0, 10);
	pSmartMeshCfg->bHiFiPeerFilter = Enable;
	
	return TRUE;
}

INT Set_SmartMesh_HyperFiFlagMask_Proc(PRTMP_ADAPTER pAd, PSTRING arg)
{
	POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	PSMART_MESH_CFG pSmartMeshCfg = NULL;


	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	switch (pObj->ioctl_if_type)
	{
		case INT_MAIN:
		case INT_MBSSID:
			if(ifIndex < HW_BEACON_MAX_NUM)
				pSmartMeshCfg = &pAd->ApCfg.MBSSID[ifIndex].SmartMeshCfg;
			break;
#ifdef APCLI_SUPPORT
		case INT_APCLI:
			if(ifIndex < MAX_APCLI_NUM)
				pSmartMeshCfg = &pAd->ApCfg.ApCliTab[ifIndex].SmartMeshCfg;
			break;
#endif /* APCLI_SUPPORT */
		default:
			DBGPRINT(RT_DEBUG_ERROR,("%s(): Invalid interface (%u).\n",__FUNCTION__, pObj->ioctl_if_type));
			return FALSE;
			break;
	}

	if (!pSmartMeshCfg)
		return FALSE;

	pSmartMeshCfg->HiFiFlagMask = simple_strtol(arg, 0, 10);
	
	return TRUE;
}

INT Set_SmartMesh_HyperFiFlagValue_Proc(PRTMP_ADAPTER pAd, PSTRING arg)
{
	POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	PSMART_MESH_CFG pSmartMeshCfg = NULL;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	switch (pObj->ioctl_if_type)
	{
		case INT_MAIN:
		case INT_MBSSID:
			if(ifIndex < HW_BEACON_MAX_NUM)
				pSmartMeshCfg = &pAd->ApCfg.MBSSID[ifIndex].SmartMeshCfg;
			break;
#ifdef APCLI_SUPPORT
		case INT_APCLI:
			if(ifIndex < MAX_APCLI_NUM)
				pSmartMeshCfg = &pAd->ApCfg.ApCliTab[ifIndex].SmartMeshCfg;
			break;
#endif /* APCLI_SUPPORT */
		default:
			DBGPRINT(RT_DEBUG_ERROR,("%s(): Invalid interface (%u).\n",__FUNCTION__, pObj->ioctl_if_type));
			return FALSE;
			break;
	}

	if (!pSmartMeshCfg)
		return FALSE;

	pSmartMeshCfg->HiFiFlagValue = simple_strtol(arg, 0, 10);
	
	return TRUE;
}

#ifdef WSC_AP_SUPPORT
ULONG Get_BssSsidTableSearchByMAC(BSS_TABLE *Tab, UCHAR *pBssAddr)
{
	UCHAR i;

	if(Tab && pBssAddr)
	{
		for (i = 0; i < Tab->BssNr; i++) 
		{
			if (MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssAddr)) 
			{ 
				return i;
			}
		}	
	}
	
	return (ULONG)BSS_NOT_FOUND;
}

INT Set_SmartMesh_WscPBCMAC_Proc(PRTMP_ADAPTER pAd, PSTRING arg)
{
	POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	PWSC_CTRL pWscControl = NULL;
	PSTRING value;
	INT i;
	
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	switch (pObj->ioctl_if_type)
	{
		case INT_MAIN:
		case INT_MBSSID:
			if(ifIndex < HW_BEACON_MAX_NUM)
				pWscControl = &pAd->ApCfg.MBSSID[ifIndex].WscControl;
			break;
#ifdef APCLI_SUPPORT
		case INT_APCLI:
			if(ifIndex < MAX_APCLI_NUM)
				pWscControl = &pAd->ApCfg.ApCliTab[ifIndex].WscControl;
			break;
#endif /* APCLI_SUPPORT */
		default:
			DBGPRINT(RT_DEBUG_ERROR,("%s(): Invalid interface (%u).\n",__FUNCTION__, pObj->ioctl_if_type));
			return FALSE;
			break;
	}

	if (!pWscControl)
		return FALSE;

	pWscControl->bWscPBCAddrMode = FALSE;
	if (strlen(arg) != 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
	{
		DBGPRINT(RT_DEBUG_ERROR, ("illegal MAC address length! (acceptable format 01:02:03:04:05:06 length 17)\n"));
		return FALSE;
	}

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":")) 
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
		{
			DBGPRINT(RT_DEBUG_ERROR, ("illegal MAC address format or octet!\n"));
			/* Do not use "continue" to replace "break" */
			break;
		}
		AtoH(value, &pWscControl->WscPBCAddr[i++], 1);
	}

	if (i != MAC_ADDR_LEN)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("MAC address length is wrong!\n"));
		return FALSE;
	}
    
    /* Help to change to PBC mode */
    pWscControl->WscMode = WSC_PBC_MODE;
    WscGetRegDataPIN(pAd, pWscControl->WscPinCode, pWscControl);
    
    pWscControl->bWscPBCAddrMode = TRUE;
	return TRUE;
}
#endif /* WSC_AP_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
INT Set_vMacPrefix_Show_Proc(
	PRTMP_ADAPTER	pAd,
	PSTRING			arg)
{
	UCHAR i,len;
	if(pAd)
	{
		len = sizeof(pAd->vMacAddrPrefix);
		for(i=0;i<len;i++)
			printk("%02x",pAd->vMacAddrPrefix[i]);
		printk("\n");

		#ifdef ACOS_PATCH /* foxconn */
		if (arg && copy_to_user(arg, pAd->vMacAddrPrefix, len))
			return FALSE;
		#endif
	}

	return TRUE;
}
#endif /* MAC_REPEATER_SUPPORT */

VOID rtmp_read_smart_mesh_from_file(
			IN  PRTMP_ADAPTER pAd,
			PSTRING tmpbuf,
			PSTRING buffer)
{

	PSTRING	tmpptr = NULL;
	PSMART_MESH_CFG pSmartMeshCfg = NULL;
	
#ifdef CONFIG_AP_SUPPORT
	/* ApSmartMesh */
	if(RTMPGetKeyParameter("ApSmartMesh", tmpbuf, 256, buffer, TRUE))
	{
		INT	Value;
		UCHAR i=0;
		
		Value = (INT) simple_strtol(tmpbuf, 0, 10);
		for (i = 0, tmpptr = rstrtok(tmpbuf,";"); tmpptr; tmpptr = rstrtok(NULL,";"), i++)
		{
			if (i >= pAd->ApCfg.BssidNum)
				break;

			pSmartMeshCfg = &pAd->ApCfg.MBSSID[i].SmartMeshCfg;
			if (Value == 0)
			{
				smart_mesh_enable(pSmartMeshCfg,FALSE);
			}
		 	else
		 	{
		 		smart_mesh_init(pSmartMeshCfg);
				smart_mesh_enable(pSmartMeshCfg,TRUE);
		 	}
			DBGPRINT(RT_DEBUG_TRACE, ("ApSmartMesh=%d\n", Value));
		}
	}
#ifdef WSC_AP_SUPPORT
#ifdef SMART_MESH_HIDDEN_WPS
    /* ApHiddenWPS */
	if(RTMPGetKeyParameter("ApHiddenWPS", tmpbuf, 256, buffer, TRUE))
	{
		INT	Value;
		UCHAR i=0;
		
		Value = (INT) simple_strtol(tmpbuf, 0, 10);
		for (i = 0, tmpptr = rstrtok(tmpbuf,";"); tmpptr; tmpptr = rstrtok(NULL,";"), i++)
		{
			if (i >= pAd->ApCfg.BssidNum)
				break;

			pSmartMeshCfg = &pAd->ApCfg.MBSSID[i].SmartMeshCfg;
			if (Value == 0)
			{
				hidden_wps_enable(pSmartMeshCfg,FALSE);
			}
		 	else
		 	{
		 		hidden_wps_init(pSmartMeshCfg);
				hidden_wps_enable(pSmartMeshCfg,TRUE);
		 	}
			DBGPRINT(RT_DEBUG_TRACE, ("ApSmartMesh=%d\n", Value));
		}
	}
#endif /* SMART_MESH_HIDDEN_WPS */
#endif /* WSC_AP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef APCLI_SUPPORT
	/* ApCliSmartMesh */
	if(RTMPGetKeyParameter("ApCliSmartMesh", tmpbuf, 256, buffer, TRUE))
	{
		INT	Value;
		UCHAR i=0;
		
		Value = (INT) simple_strtol(tmpbuf, 0, 10);
		for (i = 0, tmpptr = rstrtok(tmpbuf,";"); tmpptr; tmpptr = rstrtok(NULL,";"), i++)
		{
			if (i >= MAX_APCLI_NUM)
				break;

			pSmartMeshCfg = &pAd->ApCfg.ApCliTab[i].SmartMeshCfg;
			if (Value == 0)
			{
				smart_mesh_enable(pSmartMeshCfg,FALSE);
			}
		 	else
		 	{
		 		smart_mesh_init(pSmartMeshCfg);
				smart_mesh_enable(pSmartMeshCfg,TRUE);
		 	}
			DBGPRINT(RT_DEBUG_TRACE, ("ApCliSmartMesh=%d\n", Value));
		}
	}
#ifdef WSC_AP_SUPPORT
#ifdef SMART_MESH_HIDDEN_WPS
    /* ApCliHiddenWPS */
	if(RTMPGetKeyParameter("ApCliHiddenWPS", tmpbuf, 256, buffer, TRUE))
	{
		INT	Value;
		UCHAR i=0;
		
		Value = (INT) simple_strtol(tmpbuf, 0, 10);
		for (i = 0, tmpptr = rstrtok(tmpbuf,";"); tmpptr; tmpptr = rstrtok(NULL,";"), i++)
		{
			if (i >= MAX_APCLI_NUM)
				break;

			pSmartMeshCfg = &pAd->ApCfg.ApCliTab[i].SmartMeshCfg;
			if (Value == 0)
			{
				hidden_wps_enable(pSmartMeshCfg,FALSE);
			}
		 	else
		 	{
		 		hidden_wps_init(pSmartMeshCfg);
				hidden_wps_enable(pSmartMeshCfg,TRUE);
		 	}
			DBGPRINT(RT_DEBUG_TRACE, ("ApCliSmartMesh=%d\n", Value));
		}
	}
#endif /* SMART_MESH_HIDDEN_WPS */
#endif /* WSC_AP_SUPPORT */
#endif /* APCLI_SUPPORT */
}

#ifdef MWDS
VOID rtmp_read_DWDS_from_file(
	IN  PRTMP_ADAPTER pAd,
	PSTRING tmpbuf,
	PSTRING buffer)
{
	PSTRING	tmpptr = NULL;
	PSMART_MESH_CFG pSmartMeshCfg = NULL;
	
#ifdef CONFIG_AP_SUPPORT
	/* ApDWDS */
	if(RTMPGetKeyParameter("ApDWDS", tmpbuf, 256, buffer, TRUE))
	{
		INT	Value;
		UCHAR i=0;
		
		Value = (INT) simple_strtol(tmpbuf, 0, 10);
		for (i = 0, tmpptr = rstrtok(tmpbuf,";"); tmpptr; tmpptr = rstrtok(NULL,";"), i++)
		{
			if (i >= pAd->ApCfg.BssidNum)
				break;

			pSmartMeshCfg = &pAd->ApCfg.MBSSID[i].SmartMeshCfg;
			if (Value == 0)
			{
				dwds_enable(pSmartMeshCfg,FALSE);
				MWDSDisable(pAd,i,TRUE);
			}
			else
			{
				dwds_init(pSmartMeshCfg);
				dwds_enable(pSmartMeshCfg,TRUE);
				MWDSEnable(pAd,i,TRUE);
			}
			DBGPRINT(RT_DEBUG_TRACE, ("ApDWDS=%d\n", Value));
		}
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef APCLI_SUPPORT
	/* ApCliDWDS */
	if(RTMPGetKeyParameter("ApCliDWDS", tmpbuf, 256, buffer, TRUE))
	{
		INT	Value;
		UCHAR i=0;
		
		Value = (INT) simple_strtol(tmpbuf, 0, 10);
		for (i = 0, tmpptr = rstrtok(tmpbuf,";"); tmpptr; tmpptr = rstrtok(NULL,";"), i++)
		{
			if (i >= MAX_APCLI_NUM)
				break;

			pSmartMeshCfg = &pAd->ApCfg.ApCliTab[i].SmartMeshCfg;
			if (Value == 0)
			{
				dwds_enable(pSmartMeshCfg,FALSE);
				MWDSDisable(pAd,i,FALSE);
			}
		 	else
		 	{
		 		dwds_init(pSmartMeshCfg);
		 		dwds_enable(pSmartMeshCfg,TRUE);
		 		MWDSEnable(pAd,i,FALSE);
		 	}
			DBGPRINT(RT_DEBUG_TRACE, ("ApCliDWDS=%d\n", Value));
		}
	}
#endif /* APCLI_SUPPORT */
}
#endif /* MWDS */

INT Set_Cli_Status_Show_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{
	INT i;
	SMART_MESH_CLI_INFO cli_info;
	PMAC_TABLE_ENTRY pEntry = NULL;
	HTTRANSMIT_SETTING RX_HTSetting,TX_HTSetting;

	for (i=1; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		pEntry = &pAd->MacTab.Content[i];

		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry))
#ifdef MAC_REPEATER_SUPPORT
			&& (pEntry->bReptCli == FALSE)
#endif /* MAC_REPEATER_SUPPORT */
			&& (pEntry->Sst == SST_ASSOC))
		{
			NdisZeroMemory(&cli_info, sizeof(SMART_MESH_CLI_INFO));
			cli_info.aid = pEntry->Aid;
			cli_info.last_rssi[0] = pEntry->RssiSample.LastRssi0;
			cli_info.last_rssi[1] = pEntry->RssiSample.LastRssi1;
			cli_info.last_rssi[2] = pEntry->RssiSample.LastRssi2;
			cli_info.last_snr[0] = pEntry->RssiSample.LastSnr0;
			cli_info.last_snr[1] = pEntry->RssiSample.LastSnr1;
			cli_info.last_snr[2] = pEntry->RssiSample.LastSnr2;
			
			/* RX Info */
			NdisCopyMemory(&RX_HTSetting, &pEntry->LastRxRate, sizeof(RX_HTSetting));
#ifdef DOT11_VHT_AC
			if(RX_HTSetting.field.MODE >= MODE_VHT)
				cli_info.rx_mcs = RX_HTSetting.field.MCS & 0x0F;
			else
#endif /* DOT11_VHT_AC */
				cli_info.rx_mcs = RX_HTSetting.field.MCS;
			cli_info.rx_stream = newRateGetAntenna(RX_HTSetting.field.MCS,RX_HTSetting.field.MODE);

			cli_info.rx_bw = RX_HTSetting.field.BW;
			cli_info.rx_sgi = RX_HTSetting.field.ShortGI;
			cli_info.rx_phymode = RX_HTSetting.field.MODE;

			/* TX Info */
			NdisCopyMemory(&TX_HTSetting, &pEntry->HTPhyMode, sizeof(TX_HTSetting));
#ifdef DOT11_VHT_AC
			if(TX_HTSetting.field.MODE >= MODE_VHT)
				cli_info.tx_mcs = TX_HTSetting.field.MCS & 0x0F;
			else
#endif /* DOT11_VHT_AC */
				cli_info.tx_mcs = TX_HTSetting.field.MCS;
			cli_info.tx_stream = newRateGetAntenna(TX_HTSetting.field.MCS, TX_HTSetting.field.MODE);

			cli_info.tx_bw = TX_HTSetting.field.BW;
			cli_info.tx_sgi = TX_HTSetting.field.ShortGI;
			cli_info.tx_phymode = TX_HTSetting.field.MODE;

			/* Flow Statistics */
			cli_info.rx_total_packets = pEntry->RxPackets.QuadPart;
			cli_info.rx_total_bytes = pEntry->RxBytes;
			cli_info.tx_total_packets = pEntry->TxPackets.QuadPart;
			cli_info.tx_total_bytes = pEntry->TxBytes;
			cli_info.rx_fail_cnt = pEntry->RxDecryptErrCnt;
			cli_info.tx_fail_cnt = pEntry->StatTxFailCount;

			DBGPRINT(RT_DEBUG_OFF, 
					("Client AID %d: %02X:%02X:%02X:%02X:%02X:%02X\n", cli_info.aid, PRINT_MAC(pEntry->Addr)));

			DBGPRINT(RT_DEBUG_OFF, 
				("\tLast Signal Stauts:  RSSI = %d/%d/%d, SNR = %d/%d/%d\n",
				cli_info.last_rssi[0], cli_info.last_rssi[1], cli_info.last_rssi[2],
				cli_info.last_snr[0], cli_info.last_snr[1], cli_info.last_snr[2]));

			DBGPRINT(RT_DEBUG_OFF, 
				("\tLast RX Status:  MCS = %d, BW = %s, SS = %d, SGI = %d, MODE = %s\n",
				cli_info.rx_mcs,get_bw_str(cli_info.rx_bw),cli_info.rx_stream,
				cli_info.rx_sgi,get_phymode_str(cli_info.rx_phymode)));

			DBGPRINT(RT_DEBUG_OFF, 
				("\tLast TX Status:  MCS = %d, BW = %s, SS = %d, SGI = %d, MODE = %s\n",
				cli_info.tx_mcs,get_bw_str(cli_info.tx_bw),cli_info.tx_stream,
				cli_info.tx_sgi,get_phymode_str(cli_info.tx_phymode)));

			DBGPRINT(RT_DEBUG_OFF, 
				("\tRX Flow Statistics: Total Count = %lld, Error Count = %lu, Total Bytes  = %lu\n",
				cli_info.rx_total_packets, cli_info.rx_fail_cnt, cli_info.rx_total_bytes));

			DBGPRINT(RT_DEBUG_OFF, 
				("\tTX Flow Statistics: Total Count = %lld, Failure Count = %lu, Total Bytes = %lu\n",
				cli_info.tx_total_packets, cli_info.tx_fail_cnt, cli_info.tx_total_bytes));
		}
	} 
	DBGPRINT(RT_DEBUG_TRACE, ("<==%s\n", __FUNCTION__));

	return TRUE;
}

VOID Update_CliPktStats(
	IN  PRTMP_ADAPTER pAd, 
	IN  struct _MAC_TABLE_ENTRY *pEntry,
	IN  UINT32 MacSeq,
	IN  BOOLEAN bFromTx)
{
    UINT DataRate = 0;
    ULONG CurTime = 0;
    PCLINET_PKT_STATS_INFO pCliPktStats = NULL;
    HTTRANSMIT_SETTING RX_HTSetting,TX_HTSetting;

    if(!pEntry || !pAd->ApCfg.bCliPktStatEnable)
        return;
    
    if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry))
#ifdef MAC_REPEATER_SUPPORT
			&& (pEntry->bReptCli == FALSE)
#endif /* MAC_REPEATER_SUPPORT */
			&& (pEntry->Sst == SST_ASSOC))
	{
	    if(bFromTx)
        {
             if(pEntry->CliPktStatTxIdx >= MAX_LAST_PKT_STATS)
                pEntry->CliPktStatTxIdx = 0;
             pCliPktStats = &pEntry->CliPktStat[pEntry->CliPktStatTxIdx++];
             pCliPktStats->bTxValid = TRUE;
        }
        else
        {
             if(pEntry->CliPktStatRxIdx >= MAX_LAST_PKT_STATS)
                pEntry->CliPktStatRxIdx = 0;
              pCliPktStats = &pEntry->CliPktStat[pEntry->CliPktStatRxIdx++];
              pCliPktStats->bRxValid = TRUE;
        }

        if(bFromTx)
        {
            /* TX Info */
    		NdisCopyMemory(&TX_HTSetting, &pEntry->HTPhyMode, sizeof(TX_HTSetting));
#ifdef DOT11_VHT_AC
    		if(TX_HTSetting.field.MODE >= MODE_VHT)
    			pCliPktStats->tx_mcs = TX_HTSetting.field.MCS & 0x0F;
    		else
#endif /* DOT11_VHT_AC */
    			pCliPktStats->tx_mcs = TX_HTSetting.field.MCS;
            pCliPktStats->tx_stream = newRateGetAntenna(TX_HTSetting.field.MCS, TX_HTSetting.field.MODE);
    		pCliPktStats->tx_bw = TX_HTSetting.field.BW;
    		pCliPktStats->tx_sgi = TX_HTSetting.field.ShortGI;
    		pCliPktStats->tx_phymode = TX_HTSetting.field.MODE;
            RtmpDrvRateGet(pAd, pCliPktStats->tx_phymode, pCliPktStats->tx_sgi,
    		                    pCliPktStats->tx_bw, pCliPktStats->tx_mcs,
    		                    pCliPktStats->tx_stream, &DataRate);
            DataRate /= 500000;
    		DataRate /= 2;
            pCliPktStats->tx_rate = DataRate;
            pCliPktStats->tx_seq = MacSeq;
            NdisGetSystemUpTime(&CurTime);
            pCliPktStats->tx_time_sec = (CurTime-INITIAL_JIFFIES)/OS_HZ;
        }
        else
        {
            /* RX RSSI */
    	    pCliPktStats->last_rssi[0] = pEntry->RssiSample.LastRssi0;
    	    pCliPktStats->last_rssi[1] = pEntry->RssiSample.LastRssi1;
    		pCliPktStats->last_rssi[2] = pEntry->RssiSample.LastRssi2;

             /* RX Info */
            NdisCopyMemory(&RX_HTSetting, &pEntry->LastRxRate, sizeof(RX_HTSetting));
#ifdef DOT11_VHT_AC
			if(RX_HTSetting.field.MODE >= MODE_VHT)
				pCliPktStats->rx_mcs = RX_HTSetting.field.MCS & 0x0F;
			else
#endif /* DOT11_VHT_AC */
				pCliPktStats->rx_mcs = RX_HTSetting.field.MCS;
            pCliPktStats->rx_stream = newRateGetAntenna(RX_HTSetting.field.MCS,RX_HTSetting.field.MODE);
			pCliPktStats->rx_bw = RX_HTSetting.field.BW;
			pCliPktStats->rx_sgi = RX_HTSetting.field.ShortGI;
			pCliPktStats->rx_phymode = RX_HTSetting.field.MODE;
            RtmpDrvRateGet(pAd, pCliPktStats->rx_phymode, pCliPktStats->rx_sgi,
    		                    pCliPktStats->rx_bw, pCliPktStats->rx_mcs,
    		                    pCliPktStats->rx_stream, &DataRate);
            DataRate /= 500000;
    		DataRate /= 2;
            pCliPktStats->rx_rate = DataRate;
            pCliPktStats->rx_seq = MacSeq;
            NdisGetSystemUpTime(&CurTime);
            pCliPktStats->rx_time_sec = (CurTime-INITIAL_JIFFIES)/OS_HZ;
        }
    }
}

INT Set_Cli_Pkt_Stats_Show(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg,
	IN  BOOLEAN bTx)
{
    INT i,j;
    PSTRING value;
    UCHAR CliAddr[MAC_ADDR_LEN];
    PMAC_TABLE_ENTRY pEntry = NULL;
    PCLINET_PKT_STATS_INFO pCliPktStats = NULL;
    BOOLEAN bFirst = TRUE;

    if (strlen(arg) != 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
	{
		DBGPRINT(RT_DEBUG_ERROR, ("illegal MAC address length! (acceptable format 01:02:03:04:05:06 length 17)\n"));
		return FALSE;
	}

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":")) 
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
		{
			DBGPRINT(RT_DEBUG_ERROR, ("illegal MAC address format or octet!\n"));
			/* Do not use "continue" to replace "break" */
			break;
		}
		AtoH(value, &CliAddr[i++], 1);
	}

	if (i != MAC_ADDR_LEN)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("MAC address length is wrong!\n"));
		return FALSE;
	}

    for (i=1; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		pEntry = &pAd->MacTab.Content[i];
		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry))
#ifdef MAC_REPEATER_SUPPORT
			&& (pEntry->bReptCli == FALSE)
#endif /* MAC_REPEATER_SUPPORT */
			&& (pEntry->Sst == SST_ASSOC)
			&& MAC_ADDR_EQUAL(pEntry->Addr,CliAddr))
		{
		    for(j=0; j<MAX_LAST_PKT_STATS; j++)
            {
                pCliPktStats = &pEntry->CliPktStat[j];
                if(bTx)
                {
                    if(!pCliPktStats->bTxValid)
                        continue;

                    if(bFirst)
                    {
                        DBGPRINT(RT_DEBUG_OFF, ("\n%-7s%-6s%-8s%-4s%-4s%-3s%-4s%-5s\n", 
                                                "TIME","SEQ", "TX_RATE", "MCS", "BW", "SS", "SGI", "MODE"));
                        bFirst = FALSE;
                    }
                    DBGPRINT(RT_DEBUG_OFF, ("%-7lu%-6d%-8d%-4d%-4s%-3d%-4d%-5s\n",
                                            pCliPktStats->tx_time_sec, pCliPktStats->tx_seq, pCliPktStats->tx_rate,
                                            pCliPktStats->tx_mcs, get_bw_str(pCliPktStats->tx_bw), pCliPktStats->tx_stream,
                                            pCliPktStats->tx_sgi, get_phymode_str(pCliPktStats->tx_phymode)));
                }
                else
                {
                     if(!pCliPktStats->bRxValid)
                        continue;

                    if(bFirst)
                    {
                        DBGPRINT(RT_DEBUG_OFF, ("\n%-7s%-6s%-6s%-6s%-8s%-4s%-4s%-3s%-4s%-5s\n",
                                                "TIME","SEQ", "RSSI0", "RSSI1", "RX_RATE", "MCS",
                                                "BW", "SS", "SGI", "MODE"));
                        bFirst = FALSE;
                    }
                    DBGPRINT(RT_DEBUG_OFF, ("%-7lu%-6d%-6d%-6d%-8d%-4d%-4s%-3d%-4d%-5s\n",
                            pCliPktStats->rx_time_sec,pCliPktStats->rx_seq,pCliPktStats->last_rssi[0],
                            pCliPktStats->last_rssi[1],pCliPktStats->rx_rate, pCliPktStats->rx_mcs,
                            get_bw_str(pCliPktStats->rx_bw), pCliPktStats->rx_stream,
                            pCliPktStats->rx_sgi, get_phymode_str(pCliPktStats->rx_phymode)));
                }
            }
            break;
        }
    }
    
    return TRUE;
}

INT Set_Cli_Pkt_Stats_Enable_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{
    INT i;
    BOOLEAN bEnable = FALSE;
    PMAC_TABLE_ENTRY pEntry = NULL;

    bEnable = simple_strtol(arg, 0, 10);

    if(pAd->ApCfg.bCliPktStatEnable != bEnable)
    {
        pAd->ApCfg.bCliPktStatEnable = bEnable;
        
        /* Clear all previous collected data*/
        if(bEnable == FALSE)
        {
            for (i=1; i<MAX_LEN_OF_MAC_TABLE; i++)
        	{
        		pEntry = &pAd->MacTab.Content[i];
        		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry))
#ifdef MAC_REPEATER_SUPPORT
        			&& (pEntry->bReptCli == FALSE)
#endif /* MAC_REPEATER_SUPPORT */
        			&& (pEntry->Sst == SST_ASSOC))
        		{
        		    pEntry->CliPktStatTxIdx = 0;
                    pEntry->CliPktStatRxIdx = 0;
                    NdisZeroMemory(&pEntry->CliPktStat,sizeof(pEntry->CliPktStat));
                }
            }
        }
    }

    return TRUE;
}

INT Set_Cli_Pkt_Stats_TX_Show_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{
    return Set_Cli_Pkt_Stats_Show(pAd, arg, TRUE);
}

INT Set_Cli_Pkt_Stats_RX_Show_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{
    return Set_Cli_Pkt_Stats_Show(pAd, arg, FALSE);
}

INT Set_MLME_Queue_Full_Show_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{
    LONG Value = 0;

    if (strlen(arg) == 0)
        Value = 1;
    else
        Value = simple_strtol(arg, 0, 10);

    if(Value >= 1)
    {
        DBGPRINT(RT_DEBUG_OFF, ("MLME Queue Full Count = %lu\n",pAd->Mlme.Queue.QueueFullCnt));
#ifdef EAPOL_QUEUE_SUPPORT
        DBGPRINT(RT_DEBUG_OFF, ("EAP Queue Full Count = %lu\n",pAd->Mlme.EAP_Queue.QueueFullCnt));
#endif /* EAPOL_QUEUE_SUPPORT */
    }
    else
    {
        pAd->Mlme.Queue.QueueFullCnt = 0;
#ifdef EAPOL_QUEUE_SUPPORT
        pAd->Mlme.EAP_Queue.QueueFullCnt = 0;
#endif /* EAPOL_QUEUE_SUPPORT */
    }

    return TRUE;
}
#endif /* SMART_MESH */

#ifdef SMART_MESH_MONITOR
INT Set_Enable_Monitor_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg)
{
	INT   success = TRUE, i;
	UINT  reg_offset = 0x1098;
	UINT  target0, target1, reg;
	UCHAR *p,wcid,MntEnable;
	MNT_STA_ENTRY *pEntry;

	DBGPRINT(RT_DEBUG_OFF, ("--> %s()\n", __FUNCTION__));

	MntEnable = (UCHAR) simple_strtol(arg, 0, 10);

	if(pAdapter->MntEnable != MntEnable)
	{
		if(MntEnable == 0)
		{
			//Clear all settings
			p = ZERO_MAC_ADDR;
			target0 = (p[3]<<24) | (p[2]<<16) | (p[1]<<8) | p[0];
			target1 = (p[5]<<8) | p[4];	
			for (i=0; i< MAX_NUM_OF_MONITOR_STA; i++)
			{
				pEntry = pAdapter->MntTable + i;
				if(pEntry->bValid)
				{
					wcid = i+WCID_OF_MONITOR_STA_BASE;
					reg_offset = 0x1098 + (i << 3);	// 8 bytes per-entry
					RTMP_IO_WRITE32(pAdapter, reg_offset, target0);
					RTMP_IO_WRITE32(pAdapter, reg_offset+4, target1);
					AsicUpdateRxWCIDTable(pAdapter, wcid, ZERO_MAC_ADDR);
				}
			}
			pAdapter->MonitrCnt = 0;
			NdisZeroMemory(&pAdapter->MntTable,sizeof(pAdapter->MntTable));

			// 0x1094: bit16 set to 0
			RTMP_IO_READ32(pAdapter, 0x1094, &reg);	//MAC_APCLI_BSSID_DW1
			reg &= ~(1 << 16);
			RTMP_IO_WRITE32(pAdapter, 0x1094, reg);

			// 0x1400: bit3=0 and bit2=1
			RTMP_IO_READ32(pAdapter, RX_FILTR_CFG, &reg);
			reg &= 0xFFFFFFF3;	// clear bit3 and bit2
			reg |= 0x00000004;
			RTMP_IO_WRITE32(pAdapter, RX_FILTR_CFG, reg);

			// ECO CR: 0x1328: bit31 set to 0
			RTMP_IO_READ32(pAdapter, 0x1328, &reg);
			reg &= ~(1<<31);	// clear bit31
			RTMP_IO_WRITE32(pAdapter, 0x1328, reg);
			
			pAdapter->MntEnable = 0;
		}
		else
		{
			if(pAdapter->MonitrCnt > 0)
			{
				// 0x1094: bit16 set to 1
				RTMP_IO_READ32(pAdapter, 0x1094, &reg);	//MAC_APCLI_BSSID_DW1
				reg |= (1 << 16);
				RTMP_IO_WRITE32(pAdapter, 0x1094, reg);

				// 0x1400: bit3=1 and bit2=0
				RTMP_IO_READ32(pAdapter, RX_FILTR_CFG, &reg);
				reg &= 0xFFFFFFF3;	// clear bit3 and bit2
				reg |= 0x00000008;
				RTMP_IO_WRITE32(pAdapter, RX_FILTR_CFG, reg);

				// ECO CR: 0x1328: bit31 set to 1
				// STA can associate AP and can receive probe request.
				RTMP_IO_READ32(pAdapter, 0x1328, &reg);
				reg |= (1<<31);
				RTMP_IO_WRITE32(pAdapter, 0x1328, reg);
				
				pAdapter->MntEnable = 1;
			}
		}
	}

	DBGPRINT(RT_DEBUG_OFF, ("<-- %s()\n", __FUNCTION__));
	return success;
}

INT	Set_MonitorTarget_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg)
{
	INT   success = TRUE;
	PSTRING this_char;
	PSTRING value;
	INT 	i;

	DBGPRINT(RT_DEBUG_OFF, ("--> %s()\n", __FUNCTION__));

	while ((this_char = strsep((char **)&arg, ";")) != NULL)
	{
		if (*this_char == '\0')
		{
			DBGPRINT(RT_DEBUG_WARN, ("An unnecessary delimiter entered!\n"));
			continue;
		}
		if (strlen(this_char) != 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
		{
			DBGPRINT(RT_DEBUG_ERROR, ("illegal MAC address length! (acceptable format 01:02:03:04:05:06 length 17)\n"));
			continue;
		}
		for (i=0, value = rstrtok(this_char,":"); value; value = rstrtok(NULL,":")) 
		{
			if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
			{
				DBGPRINT(RT_DEBUG_ERROR, ("illegal MAC address format or octet!\n"));
				/* Do not use "continue" to replace "break" */
				break;
			}
			AtoH(value, &pAdapter->curMntAddr[i++], 1);
		}

		if (i != MAC_ADDR_LEN)
		{
			continue;
		}
	}

	for (i=0; i<MAC_ADDR_LEN; i++)
	   DBGPRINT(RT_DEBUG_OFF, ("%02X ", pAdapter->curMntAddr[i]));
	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	DBGPRINT(RT_DEBUG_OFF, ("<-- %s()\n", __FUNCTION__));
	return success;
}


INT Set_MonitorIndex_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg)
{
	INT   success = TRUE;
	UINT	reg_offset = 0x1098;
	UINT	target0, target1;
	UCHAR	*p,MntIndex,wcid;
	CHAR  strTemp[8];
	MNT_STA_ENTRY   *pMntEntry = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	DBGPRINT(RT_DEBUG_OFF, ("--> %s()\n", __FUNCTION__));

	MntIndex = (UCHAR) simple_strtol(arg, 0, 10);
	if(MntIndex < MAX_NUM_OF_MONITOR_STA)
	{
		if(!pAdapter->MntTable[MntIndex].bValid)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("An invalid index of existed monitor entry.\n"));
			return FALSE;
		}
		
		pAdapter->MntIdx = MntIndex;
		wcid = MntIndex+WCID_OF_MONITOR_STA_BASE;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("The index is over the maximum limit.\n"));
		return FALSE;
	}

	pMntEntry = &pAdapter->MntTable[MntIndex];
	pMacEntry = (MAC_TABLE_ENTRY *)pMntEntry->pMacEntry;
	//Clear MonitorWCID for stopping report if exist
	if(pMacEntry && IS_VALID_ENTRY(pMacEntry))
		pMacEntry->MonitorWCID = 0;
	pMntEntry->pMacEntry = NULL;
	
	NdisZeroMemory(pMntEntry, sizeof(MNT_STA_ENTRY));
	COPY_MAC_ADDR(pMntEntry->addr, pAdapter->curMntAddr);
	if(MAC_ADDR_EQUAL(ZERO_MAC_ADDR,pAdapter->curMntAddr))
	{
		if(pAdapter->MonitrCnt > 0)
			pAdapter->MonitrCnt--;
		pMntEntry->bValid = FALSE;
	}
	else
	{
		if(pAdapter->MonitrCnt < MAX_NUM_OF_MONITOR_STA)
			pAdapter->MonitrCnt++;
		pMntEntry->bValid = TRUE;
		UpdateMonitorEntry(pAdapter,wcid,pMntEntry->addr,FALSE);
	}
	
	p = pMntEntry->addr;
	DBGPRINT(RT_DEBUG_OFF, ("index: %d\n", MntIndex));
	DBGPRINT(RT_DEBUG_OFF, ("entry: %02X:%02X:%02X:%02X:%02X:%02X\n", p[0], p[1], p[2], p[3], p[4], p[5]));

	target0 = (p[3]<<24) | (p[2]<<16) | (p[1]<<8) | p[0];
	target1 = (p[5]<<8) | p[4];	
	reg_offset += (MntIndex << 3);	// 8 bytes per-entry
	RTMP_IO_WRITE32(pAdapter, reg_offset, target0);
	RTMP_IO_WRITE32(pAdapter, reg_offset+4, target1);
	
	DBGPRINT(RT_DEBUG_OFF, ("WriteCR  0x%04X:0x%08X 0x%04X:0x%08X\n", reg_offset, target0, reg_offset+4, target1));

	AsicUpdateRxWCIDTable(pAdapter, wcid, pAdapter->curMntAddr);

	if(pAdapter->MonitrCnt > 0)
		sprintf(strTemp,"%u",1);
	else
		sprintf(strTemp,"%u",0);

	success = Set_Enable_Monitor_Proc(pAdapter,strTemp);

	DBGPRINT(RT_DEBUG_OFF, ("<-- %s()\n", __FUNCTION__));
	return success;
}


INT	Set_MonitorShowAll_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg)
{
	INT   success = TRUE;
	MNT_STA_ENTRY *pEntry;
	UCHAR	i, j;

	DBGPRINT(RT_DEBUG_OFF, ("--> %s()\n", __FUNCTION__));

	DBGPRINT(RT_DEBUG_OFF, ("  Monitor Enable: %d\n", pAdapter->MntEnable));
	DBGPRINT(RT_DEBUG_OFF, ("  Index last set: %d\n", pAdapter->MntIdx));

	for (i=0; i<MAX_NUM_OF_MONITOR_STA; i++)
	{
		pEntry = pAdapter->MntTable + i;

		DBGPRINT(RT_DEBUG_OFF, ("  Monitor STA[%d]\t", i));

		for (j=0; j<MAC_ADDR_LEN; j++)
		   DBGPRINT(RT_DEBUG_OFF, ("%02X ", pEntry->addr[j]));
		DBGPRINT(RT_DEBUG_OFF, ("\t"));

		DBGPRINT(RT_DEBUG_OFF, ("Packet Receive Count=%lu\t", pEntry->Count));
		DBGPRINT(RT_DEBUG_OFF, ("RSSI:%d,%d,%d\t", pEntry->RssiSample.AvgRssi0, pEntry->RssiSample.AvgRssi1, pEntry->RssiSample.AvgRssi2));
		DBGPRINT(RT_DEBUG_OFF, ("SNR:%d,%d,%d\n", pEntry->RssiSample.AvgSnr0, pEntry->RssiSample.AvgSnr1, pEntry->RssiSample.AvgSnr2));
	}

	DBGPRINT(RT_DEBUG_OFF, ("<-- %s()\n", __FUNCTION__));
	return success;
}


INT	Set_MonitorClearCounter_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg)
{
	INT   success = TRUE;
	MNT_STA_ENTRY *pEntry;
	UCHAR	i;

	DBGPRINT(RT_DEBUG_OFF, ("--> %s()\n", __FUNCTION__));

	for (i=0; i<MAX_NUM_OF_MONITOR_STA; i++)
	{
		pEntry = pAdapter->MntTable + i;
		pEntry->Count = 0;
		NdisZeroMemory(&pEntry->RssiSample, sizeof(RSSI_SAMPLE));
	}

	DBGPRINT(RT_DEBUG_OFF, ("<-- %s()\n", __FUNCTION__));
	return success;
}

INT	Set_Enable_MonitorTarget_Proc(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	PSTRING			arg,
	IN 	USHORT 			wcid)
{
	INT   success = TRUE;
	CHAR  strTemp[8];
	UCHAR MntIndex;
	
	success = Set_MonitorTarget_Proc(pAdapter,arg);
	if(!success) 
		return success;
	
	MntIndex = (wcid-WCID_OF_MONITOR_STA_BASE);
	if(MntIndex < MAX_NUM_OF_MONITOR_STA)
	{
		pAdapter->MntIdx = MntIndex;
		pAdapter->MntTable[MntIndex].bValid = TRUE;
		sprintf(strTemp,"%u",MntIndex);
		success = Set_MonitorIndex_Proc(pAdapter,strTemp);
	}
	else
		success = FALSE;

	return success;
}

INT	Set_MonitorTarget0_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg)
{
	return Set_Enable_MonitorTarget_Proc(pAdapter,arg,WCID_OF_MONITOR_STA_BASE);
}

INT	Set_MonitorTarget1_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg)
{
	return Set_Enable_MonitorTarget_Proc(pAdapter,arg,(WCID_OF_MONITOR_STA_BASE+1));
}

INT	Set_MonitorTarget2_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg)
{
	return Set_Enable_MonitorTarget_Proc(pAdapter,arg,(WCID_OF_MONITOR_STA_BASE+2));
}

INT	Set_MonitorTarget3_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg)
{
	return Set_Enable_MonitorTarget_Proc(pAdapter,arg,(WCID_OF_MONITOR_STA_BASE+3));
}

INT	Set_MonitorTarget4_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg)
{
	return Set_Enable_MonitorTarget_Proc(pAdapter,arg,(WCID_OF_MONITOR_STA_BASE+4));
}

INT	Set_MonitorTarget5_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg)
{
	return Set_Enable_MonitorTarget_Proc(pAdapter,arg,(WCID_OF_MONITOR_STA_BASE+5));
}

INT	Set_MonitorTarget6_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg)
{
	return Set_Enable_MonitorTarget_Proc(pAdapter,arg,(WCID_OF_MONITOR_STA_BASE+6));
}

INT Smart_Mesh_Pkt_Report_Action(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR WCID,
	IN RXWI_STRUC	*pRxWI,	
	IN PUCHAR buff)
{

	SMART_MESH_RAW raw_tmp;
	PHEADER_802_11	pHeader = (PHEADER_802_11)buff;
	HTTRANSMIT_SETTING HTSetting;
	INT				apidx = MAIN_MBSSID;	
	UCHAR			s_addr[MAC_ADDR_LEN];
	UCHAR			MESH_IE[] = {0x55, 0x72};
	UCHAR			FrameBuf[512];
	UINT32 			MCS = 0, BW = 0, ShortGI = 0, PHYMODE = 0, frame_len, offset=0;
	UCHAR 			MntIdx = (WCID - WCID_OF_MONITOR_STA_BASE);
	MNT_STA_ENTRY   *pEntry = pAd->MntTable + MntIdx;
	struct sk_buff *skb = NULL;

	if(!pEntry->bValid)
		return FALSE;

	switch (pHeader->FC.Type)
	{
		case BTYPE_DATA:
			break;

		case BTYPE_MGMT:
			break;

		case BTYPE_CNTL:
			break;
			
		default:
			goto done;
			break;
	}

	/* Update entry's statistic infomation*/
	pEntry->Count++;
	Update_Rssi_Sample(pAd,&pEntry->RssiSample,pRxWI);
	
	/* Init frame buffer */
	NdisZeroMemory(FrameBuf, sizeof(FrameBuf));
	NdisZeroMemory(&raw_tmp, sizeof(raw_tmp));

	/* Fake a Source Address for transmission */
	COPY_MAC_ADDR(s_addr, pAd->ApCfg.MBSSID[apidx].wdev.if_addr);
    if(s_addr[1] == 0xff)
        s_addr[1]=0;
    else
        s_addr[1]++;
    
	/* Prepare the 802.3 header */
	MAKE_802_3_HEADER(FrameBuf, pAd->ApCfg.MBSSID[apidx].wdev.if_addr,s_addr, MESH_IE);
	offset += LENGTH_802_3;

	//generate radio part
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
	{
		MCS = pRxWI->RXWI_N.mcs;
		BW = pRxWI->RXWI_N.bw;
		ShortGI = pRxWI->RXWI_N.sgi;
		PHYMODE = pRxWI->RXWI_N.phy_mode;
	}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
	{
		MCS = pRxWI->RXWI_O.mcs;
		BW = pRxWI->RXWI_O.bw;
		ShortGI = pRxWI->RXWI_O.sgi;
		PHYMODE = pRxWI->RXWI_O.phy_mode;
	}
#endif /* RTMP_MAC */

	//for RSSI
	raw_tmp.wlan_radio_tap.RSSI0 = pEntry->RssiSample.AvgRssi0;
	raw_tmp.wlan_radio_tap.RSSI1 = pEntry->RssiSample.AvgRssi1;

	//for SNR
	raw_tmp.wlan_radio_tap.SNR0 = pEntry->RssiSample.AvgSnr0;
	raw_tmp.wlan_radio_tap.SNR1 = pEntry->RssiSample.AvgSnr1;

	//for RXWI
	raw_tmp.wlan_radio_tap.PHYMODE = PHYMODE;
	raw_tmp.wlan_radio_tap.MCS = MCS;
	raw_tmp.wlan_radio_tap.BW = BW;
	raw_tmp.wlan_radio_tap.ShortGI = ShortGI;

	//for RATE
	HTSetting.field.MODE = PHYMODE;
	HTSetting.field.MCS = MCS;
	HTSetting.field.BW = BW;
	HTSetting.field.ShortGI = ShortGI;	
	getRate(HTSetting, &(raw_tmp.wlan_radio_tap.RATE));

	if(pHeader->FC.ToDs == 1 && pHeader->FC.FrDs == 1)
		NdisCopyMemory((CHAR*)(&raw_tmp.wlan_header), pHeader, sizeof(HEADER_802_11_4_ADDR));
	else
		NdisCopyMemory((CHAR*)(&raw_tmp.wlan_header), pHeader, sizeof(HEADER_802_11));

	/* Prepare the real payload*/
	NdisCopyMemory(&FrameBuf[offset], (CHAR*)&raw_tmp, sizeof(raw_tmp));
	offset += sizeof(raw_tmp);
	frame_len = offset;

	/* Create skb */
	skb = dev_alloc_skb(frame_len + 2);
	if (!skb) {
		DBGPRINT(RT_DEBUG_ERROR,
			 ("%s : Error! Can't allocate a skb.\n", __FUNCTION__));
		return FALSE;
	}

	SET_OS_PKT_NETDEV(skb, get_netdev_from_bssid(pAd,apidx));

	/* 16 byte align the IP header */
	skb_reserve(skb, 2);

	/* Insert the frame content */
	NdisMoveMemory(GET_OS_PKT_DATAPTR(skb), FrameBuf, frame_len);

	/* End this frame */
	skb_put(skb, frame_len);

	/* Report to upper layer */
	skb->protocol = eth_type_trans(skb, skb->dev);
	netif_rx(skb);
done:	
	return TRUE;
}

INT UpdateMonitorEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR MntWCID,
	IN UCHAR *MntAddr,
	IN BOOLEAN bClear)
{
	INT i;
	UCHAR 			MntIdx = (MntWCID - WCID_OF_MONITOR_STA_BASE);
	MNT_STA_ENTRY   *pMntEntry = pAd->MntTable + MntIdx;
	PMAC_TABLE_ENTRY pMacEntry = NULL;

	if(!pMntEntry->bValid)
		return FALSE;

	if(bClear)
	{
		pMacEntry = pMntEntry->pMacEntry;
		if(pMacEntry != NULL && IS_VALID_ENTRY(pMacEntry))
			pMacEntry->MonitorWCID = 0;
		
		pMntEntry->pMacEntry = NULL;
	}
	else
	{
		for (i=1; i<MAX_LEN_OF_MAC_TABLE; i++)
		{
			pMacEntry = &pAd->MacTab.Content[i];
			if(!IS_VALID_ENTRY(pMacEntry))
				continue;
		
			if(MntAddr && MAC_ADDR_EQUAL(MntAddr,pMacEntry->Addr))
			{
				pMntEntry->pMacEntry = (VOID*)pMacEntry;
				pMacEntry->MonitorWCID = MntWCID;
				break;
			}
		}
	}
	
	return TRUE;
}

BOOLEAN IsValidUnicastToMe(IN PRTMP_ADAPTER pAd,
                      IN UCHAR WCID,
                      IN PUCHAR pDA)
{
     BOOLEAN bUToMe = FALSE;
     
#ifdef CONFIG_AP_SUPPORT
    IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
    {
        if (!VALID_WCID(WCID))
           bUToMe = FALSE;

        if (MAC_ADDR_EQUAL(pDA, pAd->CurrentAddress))
             bUToMe = TRUE;

        if (!bUToMe)
        {
#ifdef APCLI_SUPPORT
            UINT index;
            PAPCLI_STRUCT pApCliEntry = NULL;
            for(index = 0; index < MAX_APCLI_NUM; index++)
            {
                pApCliEntry = &pAd->ApCfg.ApCliTab[index];
                if (pApCliEntry && 
                   (pApCliEntry->Enable && pApCliEntry->Valid))
                {
                    if (MAC_ADDR_EQUAL(pApCliEntry->wdev.if_addr,pDA))
                    {
                        bUToMe = TRUE;
                        break;
                    }
                }      
            }

#ifdef MAC_REPEATER_SUPPORT
            if (!bUToMe &&
               (pAd->ApCfg.bMACRepeaterEn == TRUE) &&
               (RTMPQueryLookupRepeaterCliEntry(pAd, pDA) == TRUE))
               bUToMe = TRUE;
#endif /* MAC_REPEATER_SUPPORT */       
#endif /* APCLI_SUPPORT */
        }
    }
#endif /* CONFIG_AP_SUPPORT */

    return bUToMe;
}
#endif /*SMART_MESH_MONITOR*/

