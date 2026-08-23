/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2012, MTK.
 *
 * All rights reserved.	MediaTeK's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of MediaTeK Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of MediaTeK, Inc. is obtained.
 ***************************************************************************

	Module Name:
	omci_me_equipment_management_func.c
	
	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	kenny.weng		2012/7/16	Create
*/

#include <stdio.h>
#include "omci_types.h"
#include "omci_me.h"
#include "omci_general_func.h"
#include "omci_me_equipment_management.h"

extern int tmrIdImage;

int omciCreateActionForOnuRemoteDebug(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
int omciInternalCreateForOnuRemoteDebug(uint16 classId, uint16 instanceId);
int omciInternalCreateForSoftwareImage(uint16 classId, uint16 instanceId);


/*******************************************************************************************************************************
9.1.1 ONU-G

********************************************************************************************************************************/
int omciMeInitForOnuG(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_REBOOT] = omciRebootAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_TEST] = omciTestAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SYNC_TIME] = omciSyncTimeAction;	

	return 0;
}

int omciInternalCreateForOnuG(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 attriMask = 0;
	char buffer[25] = {0};
	uint8 serialNumber[8] = {0};
	uint32 tmp = 0;
	int i;

	if(instanceId != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "ONU-G only one instance\n");
		return -1;
	}

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);

/*0---meId = 0*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnuG: tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);

/*1---vendor Id*/	
	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
#ifdef TCSUPPORT_VNPTT	
	uint8 vendor[4] = "HWTC";
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, vendor, length);
	}
#else
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, pOnuCap->sn, length);
	}
#endif
	else
		goto fail;	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnuG: tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName, length, buffer);

/*2---Version*/
	attriMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
#ifdef TCSUPPORT_VNPTT
	uint8 Version[14] = "NOT034NU1F";
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, Version, length);
	}
#else
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, pOnuCap->version, length);
	}
#endif
	else
		goto fail;	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnuG: tmpOmciManageEntity_p->omciAttriDescriptList[2].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[2].attriName, length, buffer);
	
	
/*3---Serial Number*/
	attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, pOnuCap->sn, length);			
	}
	else
		goto fail;	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnuG: tmpOmciManageEntity_p->omciAttriDescriptList[3].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[3].attriName, length, buffer);

/*4---Traffic Management Option*/
	attriMask = 1<<12;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = pOnuCap->tmOption;
	}
	else
		goto fail;	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnuG: tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName, length, buffer);

/*5---Deprecated=0*/

/*6---battery backup*/
	attriMask = 1<<10;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0;
	else
		goto fail;		
	
/*9---ONU Survival time*/
	attriMask = 1<<7;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = pOnuCap->onuSurvivalTime;
	}
	else
		goto fail;	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnuG: tmpOmciManageEntity_p->omciAttriDescriptList[9].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[9].attriName, length, buffer);

/*13---Extended TC layer*/
	attriMask = 1<<3;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = pOnuCap->extendedTcLayer;
	}
	else
		goto fail;	

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnuG: tmpOmciManageEntity_p->omciAttriDescriptList[13].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[13].attriName, length, buffer);

	return 0;

fail:
	omciFreeInstance(tmpomciMeInst_p);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL");
	return -1;
}

int omciInitInstForOnuG(void)
{
	int ret = 0;

	ret = omciInternalCreateForOnuG(OMCI_CLASS_ID_ONU_G, 0);/*internal create func*/
	if (ret == -1)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_ONU_G fail\n");

	return ret;
}

int32 setONUGAdminStateValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	/*lock all lan ports*/
	
//	tcapi_set("ONU_G", "AdminState", value);
	if (*value == 1)
		omci.onuAdminState = 1;
	else
		omci.onuAdminState = 0;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	memcpy(attributeValuePtr, value, length);
	
	return 0;
}

int32 getONUGOpStateValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char tempBuffer[25] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
		
	tcapi_get("GPON_ONU", "OperationalState", tempBuffer);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	memcpy(attributeValuePtr, tempBuffer, length);
	
	return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute);	
}

#ifdef TCSUPPORT_CUC
int32 getONUGLOID(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute,uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char buffer[64] = {'\0'};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memset(buffer, 0, sizeof(buffer));
		tcapi_get("GPON_LOIDAuth", "LOID", buffer);
		memcpy(attributeValuePtr,  buffer, 24);
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "getOperatorIDValue: attributeValuePtr == NULL\n");
		return -1;
	}
	
	return getTheValue(value, (char *)attributeValuePtr, 24, omciAttribute);	
}

int32 getONUGLOIDPWD(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute,uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char buffer[64] = {'\0'};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memset(buffer, 0, sizeof(buffer));
		tcapi_get("GPON_LOIDAuth", "Password", buffer);
		memcpy(attributeValuePtr,  buffer, 12);
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "getOperatorIDValue: attributeValuePtr == NULL\n");
		return -1;
	}
	
	return getTheValue(value, (char *)attributeValuePtr, 12, omciAttribute);	
}
#endif
/*******************************************************************************************************************************
9.1.2 ONU2-G

********************************************************************************************************************************/
int omciMeInitForOnu2G(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	/* if need ONU auto create a ME instance, create here */
	return 0;
}

int omciInternalCreateForOnu2G(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 attriMask = 0;
	char buffer[25] = {0};
	uint32 vendorProCode = 0;

	if(instanceId != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "ONU2-G only one instance\n");
		return -1;
	}

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);

	
/*0---meId = 0*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, 0);	
	else
		goto fail;
	
/*1---equipment Id*/
	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
#ifdef TCSUPPORT_VNPTT
	uint8 equipment[20]="F600WV5.0";
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, equipment, length);
	}
#else
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, pOnuCap->equipmentId, length);
	}
#endif
	else
		goto fail;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnu2G: tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName, length, buffer);

/*2---OMCC Version*/
	attriMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = pOnuCap->omccVersion;
	}
	else
		goto fail;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnu2G: tmpOmciManageEntity_p->omciAttriDescriptList[2].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[2].attriName, length, buffer);
	
/*3---Vendor Product Code*/
	attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		put16(attributeValuePtr, pOnuCap->vendorProCode);
	}
	else
		goto fail;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnu2G: tmpOmciManageEntity_p->omciAttriDescriptList[3].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[3].attriName, length, buffer);

/*4---security capability*/
	attriMask = 1<<12;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = pOnuCap->securityCap;
	}	
	else
		goto fail;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnu2G: tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName, length, buffer);

/*5---security mode*/
	attriMask = 1<<11;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = 1;
	}	
	else
		goto fail;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnu2G: tmpOmciManageEntity_p->omciAttriDescriptList[5].attriName = %s, length = %x, buffer = %s\n",	tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName, length, buffer);


/*6---total priority queue number*/
	attriMask = 1<<10;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		put16(attributeValuePtr,  pOnuCap->totalPQNum);
	}	
	else
		goto fail;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnu2G: tmpOmciManageEntity_p->omciAttriDescriptList[6].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[6].attriName, length, buffer);


/*7---total traffic scheduler number*/
	attriMask = 1<<9;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = pOnuCap->totalTSNum;
	}	
	else
		goto fail;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnu2G: tmpOmciManageEntity_p->omciAttriDescriptList[7].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[7].attriName, length, buffer);

	
/*8---Deprecated =1*/
	attriMask = 1<<8;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 1;
	else
		goto fail;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnu2G: tmpOmciManageEntity_p->omciAttriDescriptList[8].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[8].attriName, length, buffer);

/*9---Total Gem port Number*/
	attriMask = 1<<7;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		put16(attributeValuePtr,  pOnuCap->totalGEMPortNum);
	}	
	else
		goto fail;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnu2G: tmpOmciManageEntity_p->omciAttriDescriptList[9].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[9].attriName, length, buffer);

/*11---Connectivity capability*/
	attriMask = 1<<5;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		put16(attributeValuePtr, pOnuCap->connectivityCap);
	}	
	else
		goto fail;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnu2G: tmpOmciManageEntity_p->omciAttriDescriptList[11].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[11].attriName, length, buffer);

/*13---Qos configuration flexibility*/
	attriMask = 1<<3;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		put16(attributeValuePtr, pOnuCap->qoSConfFlex);
	}	
	else
		goto fail;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnu2G: tmpOmciManageEntity_p->omciAttriDescriptList[13].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[13].attriName, length, buffer);

	return 0;

fail:
	omciFreeInstance(tmpomciMeInst_p);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL");
	return -1;	
}

int omciInitInstForOnu2G(void)
{
	int ret = 0;

	ret = omciInternalCreateForOnu2G(OMCI_CLASS_ID_ONU2_G, 0);   /*internal create func*/
	if (ret == -1)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_ONU2_G fail\n");

	return ret;
}

int32 getOMCCVersionValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char buffer[25] = {'\0'};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = pOnuCap->omccVersion;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "getOMCCVersionValue: attributeValuePtr == NULL\n");
		return -1;
	}
	
	return getTheValue(value, (char *)attributeValuePtr, 1, omciAttribute);	
}
int32 getSysUpTimeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	FILE *fp = NULL;
	float timeSec = 0.0;
	uint32 sysUpTime = 0;

	if ((fp = fopen("/proc/uptime", "r")) == NULL)
		return -1;

	fscanf(fp, "%f", &timeSec);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"timeSec = %f\n", timeSec);
	sysUpTime = (uint32)(timeSec * 100); /*10 ms interval*/

	fclose(fp);
	//sysUpTime = secclock();
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
		put32(attributeValuePtr, sysUpTime);
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "getSysUpTimeValue: attributeValuePtr == NULL\n");
		return -1;
	}
	
	return getTheValue(value, (char *)&sysUpTime, 4, omciAttribute);	
}

int32 setSecurityModeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint8 modeEnable = 0 ;
	char buffer[25] ={0};

	if (*value == 1)
		modeEnable = 1;
	else
		modeEnable = 0;

	sprintf(buffer, "%x", modeEnable);
	tcApiSetAttriubte("GPON_ONU", "SecurityMode", buffer);
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, value, length);
	else
		return -1;
	
	return 0;
}


/*******************************************************************************************************************************
9.1.3 ONU data

********************************************************************************************************************************/
int omciMeInitForOnuData(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p = omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_ALL_ALARMS] = omciGetAllAlarmsAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_ALL_ALARMS_NEXT] = omciGetAllAlarmsNextAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_MIB_RESET] = omciMIBResetAction;	
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_MIB_UPLOAD] = omciMIBUploadAction;	
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_MIB_UPLOAD_NEXT] = omciMIBUploadNextAction;	
	
	return 0;
}

int omciInternalCreateForOnuData(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;

	if(instanceId != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "ONU data only a instance \n");
		return -1;
	}

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	
	return 0;
}

int omciInitInstForOnuData(void)
{
	int ret = 0;

	ret = omciInternalCreateForOnuData(OMCI_CLASS_ID_ONU_DATA, 0);/*internal create func*/
	if (ret == -1)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_ONU_data fail\n");

	return ret;
}

int32 getMIBDataSyncValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 MIBDataSync = getOmciDataSyncSeq();

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "getMIBDataSyncValue: The omciDataSync = %x\n", MIBDataSync);
	return getTheValue(value, (char*)&MIBDataSync, 1, omciAttribute);	
}
int32 setMIBDataSyncValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	if (value == NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setMIBDataSyncValue: The Vale is NULL!\n");
		return -1;
	}
	setGeneralValue(value, meInstantPtr, omciAttribute, flag);
	
	return setOmciDataSyncSeq(*value);
}

/*******************************************************************************************************************************
9.1.4 Software download

********************************************************************************************************************************/
int omciInitInstForSoftwareImage(void)
{
	int ret = 0;
	uint16 instanceNum=0;

	for(instanceNum = 0; instanceNum<2; instanceNum++)
	{
		ret = omciInternalCreateForSoftwareImage(OMCI_CLASS_ID_SOFTWARE_IMGAE, instanceNum);
		
		if(ret != 0)
		{
			return ret;
		}
	}

	return ret;
}

int omciMeInitForSoftwareImage(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_START_SW_DOWNLOAD] = omciStartSwDownloadAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DOWNLOAD_SEC] = omciDownloadSecAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_END_SW_DOWNLOAD] = omciEndSwDownloadAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_ACTIVE_SW] = omciActiveSwAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_COMMIT_SW] = omciCommitSwAction;
	/* if need ONU auto create a ME instance, create here */
	return 0;
}

int omciInternalCreateForSoftwareImage(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	char buffer[25] = {0};
	char entryName[32]={0};
	uint16 attriMask = 0;


	if(instanceId > 1)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"Software image only two instance \n");
		return -1;
	}

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\ncome into omciInternalCreateForSoftwareImage \n");
	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;

	if(attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);
	
	sprintf(entryName, "GPON_SoftImage%d", instanceId);	


	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if(attributeValuePtr != NULL)
	{
		memset(buffer, 0, sizeof(buffer));
#ifdef TCSUPPORT_VNPTT
		sprintf(buffer, "V5.0.10P1T4");
#else
		tcapi_get(entryName, "Version", buffer);
#endif
		memcpy(attributeValuePtr, buffer, length);
	}
	else
		goto fail;

	attriMask = 1<<14;	
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if(attributeValuePtr != NULL)
	{
		memset(buffer, 0, sizeof(buffer));
		tcapi_get(entryName, "Committed", buffer);
		*attributeValuePtr = atoi(buffer);
	}
	else 
		goto fail;
	

	attriMask = 1<<13;	
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if(attributeValuePtr != NULL)
	{
		memset(buffer, 0, sizeof(buffer));
		tcapi_get(entryName, "Active", buffer);
		*attributeValuePtr = atoi(buffer);
	}
	else 
		goto fail;

	attriMask = 1<<12;	
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if(attributeValuePtr != NULL)
	{
		memset(buffer, 0, sizeof(buffer));
		tcapi_get(entryName, "Valid", buffer);
		*attributeValuePtr = atoi(buffer);
	}	
	else 
		goto fail;

	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	
	return 0;
	
fail:
	omciFreeInstance(tmpomciMeInst_p);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL");
	return -1;		
}

int8 getSoftwareImageState(omciMeInst_t *image_0, omciMeInst_t *image_1){
	uint8 image0_active=0;
	uint8 image0_commit=0;
	uint8 image0_valid=0;
	uint8 image1_active=0;
	uint8 image1_commit=0;
	uint8 image1_valid=0;
	uint16 length=0;
	uint8 *attributeValuePtr=NULL;
	uint16 attriMask = 0;
	
	if((image_0 == NULL) || (image_1 == NULL))
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\ngetSoftwareImageState:---image not exesit!!");
		return -1;
	}
	/*we can read state by instance or tcapi */
	//2--- Is committed
	attriMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
	image0_commit = *attributeValuePtr;
	attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
	image1_commit = *attributeValuePtr;

	//3--- Is active
	attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
	image0_active = *attributeValuePtr;
	attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
	image1_active = *attributeValuePtr;

	//4--- Is valid
	attriMask = 1<<12;
	attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
	image0_valid = *attributeValuePtr;
	attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
	image1_valid = *attributeValuePtr;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\nimage0_active=%d, image0_commit=%d, image0_valid=%d\n",image0_active,image0_commit,image0_valid);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\nimage1_active=%d, image1_commit=%d, image1_valid=%d\n",image1_active,image1_commit,image1_valid);
	
	if(((image0_active == ACTIVE)&&(image0_commit == COMMITTED)&&(image0_valid == VALID))
		&&((image1_active == INACTIVE)&&(image1_commit == UNCOMMITTED)&&(image1_valid == INVALID)))
	{
		if(tmrIdImage > 0)/*if receive start download action*/
			return SOFTWARE_IMAGE_STATE_S2_0;
		else
			return SOFTWARE_IMAGE_STATE_S1_0;
	}

	if(((image0_active == ACTIVE)&&(image0_commit == COMMITTED)&&(image0_valid == VALID))
		&&((image1_active == INACTIVE)&&(image1_commit == UNCOMMITTED)&&(image1_valid == VALID)))
	{
		return SOFTWARE_IMAGE_STATE_S3_0;
	}

	if(((image0_active == INACTIVE)&&(image0_commit == COMMITTED)&&(image0_valid == VALID))
		&&((image1_active == ACTIVE)&&(image1_commit == UNCOMMITTED)&&(image1_valid == VALID)))
	{
		return SOFTWARE_IMAGE_STATE_S4_0;
	}

	if(((image0_active == ACTIVE)&&(image0_commit == UNCOMMITTED)&&(image0_valid == VALID))
		&&((image1_active == INACTIVE)&&(image1_commit == COMMITTED)&&(image1_valid == VALID)))
	{
		return SOFTWARE_IMAGE_STATE_S4_1;
	}

	if(((image0_active == INACTIVE)&&(image0_commit == UNCOMMITTED)&&(image0_valid == VALID))
		&&((image1_active == ACTIVE)&&(image1_commit == COMMITTED)&&(image1_valid == VALID)))
	{
		return SOFTWARE_IMAGE_STATE_S3_1;
	}

	if(((image0_active == INACTIVE)&&(image0_commit == UNCOMMITTED)&&(image0_valid == INVALID))
		&&((image1_active == ACTIVE)&&(image1_commit == COMMITTED)&&(image1_valid == VALID)))
	{
		if(tmrIdImage > 0)/*if receive start download action*/
			return SOFTWARE_IMAGE_STATE_S2_1;
		else
			return SOFTWARE_IMAGE_STATE_S1_1;
	}

	return SOFTWARE_IMAGE_STATE_S0;
}

int8 setSoftwareImageState(omciMeInst_t *image_0, omciMeInst_t *image_1, int imageState){
	uint16 length=0;
	uint8 *attributeValuePtr=NULL;
	char entryName_0[32] = {0};
	char entryName_1[32] = {0};
	uint16 attriMask = 0;
	
	if((image_0 == NULL) || (image_1 == NULL))
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\ngetSoftwareImageState:---image not exesit!!");
		return -1;
	}
	/*set state by instance and tcapi */
	sprintf(entryName_0, "GPON_SoftImage0");
	sprintf(entryName_1, "GPON_SoftImage1");
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "setSoftwareImageState: imageState =  %d\n",imageState);

	switch(imageState)
	{
		case SOFTWARE_IMAGE_STATE_S1_0:
			//2--- Is committed
			attriMask = 1<<14;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = COMMITTED;
			tcapi_set(entryName_0, "Committed", "1");

			//3--- Is active
			attriMask = 1<<13;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = ACTIVE;
			tcapi_set(entryName_0, "Active", "1");

			//4--- Is valid
			attriMask = 1<<12;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = VALID;
			tcapi_set(entryName_0, "Valid", "1");

			//2--- Is committed
			attriMask = 1<<14;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = UNCOMMITTED;
			tcapi_set(entryName_1, "Committed", "0");

			//3--- Is active
			attriMask = 1<<13;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = INACTIVE;
			tcapi_set(entryName_1, "Active", "0");

			//4--- Is valid
			attriMask = 1<<12;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = INVALID;
			tcapi_set(entryName_1, "Valid", "0");

			tcapi_set(GPONCOMMONNODE,OMCI_SOFTWAREIMAGE_STATE,"SOFTWARE_IMAGE_STATE_S1_0");
			break;

		case SOFTWARE_IMAGE_STATE_S2_0:
			//2--- Is committed
			attriMask = 1<<14;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = COMMITTED;
			tcapi_set(entryName_0, "Committed", "1");

			//3--- Is active
			attriMask = 1<<13;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = ACTIVE;
			tcapi_set(entryName_0, "Active", "1");

			//4--- Is valid
			attriMask = 1<<12;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = VALID;
			tcapi_set(entryName_0, "Valid", "1");

			//2--- Is committed
			attriMask = 1<<14;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = UNCOMMITTED;
			tcapi_set(entryName_1, "Committed", "0");

			//3--- Is active
			attriMask = 1<<13;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = INACTIVE;
			tcapi_set(entryName_1, "Active", "0");

			//4--- Is valid
			attriMask = 1<<12;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = INVALID;
			tcapi_set(entryName_1, "Valid", "0");

			tcapi_set(GPONCOMMONNODE,OMCI_SOFTWAREIMAGE_STATE,"SOFTWARE_IMAGE_STATE_S2_0");
			break;	

		case SOFTWARE_IMAGE_STATE_S3_0:
			//2--- Is committed
			attriMask = 1<<14;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = COMMITTED;
			tcapi_set(entryName_0, "Committed", "1");

			//3--- Is active
			attriMask = 1<<13;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = ACTIVE;
			tcapi_set(entryName_0, "Active", "1");

			//4--- Is valid
			attriMask = 1<<12;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = VALID;
			tcapi_set(entryName_0, "Valid", "1");

			//2--- Is committed
			attriMask = 1<<14;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = UNCOMMITTED;
			tcapi_set(entryName_1, "Committed", "0");

			//3--- Is active
			attriMask = 1<<13;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = INACTIVE;
			tcapi_set(entryName_1, "Active", "0");

			//4--- Is valid
			attriMask = 1<<12;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = VALID;
			tcapi_set(entryName_1, "Valid", "1");

			tcapi_set(GPONCOMMONNODE,OMCI_SOFTWAREIMAGE_STATE,"SOFTWARE_IMAGE_STATE_S3_0");
			break;			

		case SOFTWARE_IMAGE_STATE_S4_0:
			//2--- Is committed
			attriMask = 1<<14;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = COMMITTED;
			tcapi_set(entryName_0, "Committed", "1");

			//3--- Is active
			attriMask = 1<<13;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = INACTIVE;
			tcapi_set(entryName_0, "Active", "0");

			//4--- Is valid
			attriMask = 1<<12;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = VALID;
			tcapi_set(entryName_0, "Valid", "1");

			//2--- Is committed
			attriMask = 1<<14;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = UNCOMMITTED;
			tcapi_set(entryName_1, "Committed", "0");

			//3--- Is active
			attriMask = 1<<13;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = ACTIVE;
			tcapi_set(entryName_1, "Active", "1");

			//4--- Is valid
			attriMask = 1<<12;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = VALID;	
			tcapi_set(entryName_1, "Valid", "1");

			tcapi_set(GPONCOMMONNODE,OMCI_SOFTWAREIMAGE_STATE,"SOFTWARE_IMAGE_STATE_S4_0");
			break;

		case SOFTWARE_IMAGE_STATE_S1_1:
			//2--- Is committed
			attriMask = 1<<14;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = COMMITTED;
			tcapi_set(entryName_1, "Committed", "1");

			//3--- Is active
			attriMask = 1<<13;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = ACTIVE;
			tcapi_set(entryName_1, "Active", "1");

			//4--- Is valid
			attriMask = 1<<12;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = VALID;
			tcapi_set(entryName_1, "Valid", "1");

			//2--- Is committed
			attriMask = 1<<14;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = UNCOMMITTED;
			tcapi_set(entryName_0, "Committed", "0");

			//3--- Is active
			attriMask = 1<<13;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = INACTIVE;
			tcapi_set(entryName_0, "Active", "0");

			//4--- Is valid
			attriMask = 1<<12;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = VALID;	
			tcapi_set(entryName_0, "Valid", "1");

			tcapi_set(GPONCOMMONNODE,OMCI_SOFTWAREIMAGE_STATE,"SOFTWARE_IMAGE_STATE_S1_1");
			break;

		case SOFTWARE_IMAGE_STATE_S2_1:
			//2--- Is committed
			attriMask = 1<<14;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = COMMITTED;
			tcapi_set(entryName_1, "Committed", "1");

			//3--- Is active
			attriMask = 1<<13;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = ACTIVE;
			tcapi_set(entryName_1, "Active", "1");

			//4--- Is valid
			attriMask = 1<<12;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = VALID;
			tcapi_set(entryName_1, "Valid", "1");

			//2--- Is committed
			attriMask = 1<<14;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = UNCOMMITTED;
			tcapi_set(entryName_0, "Committed", "0");

			//3--- Is active
			attriMask = 1<<13;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = INACTIVE;
			tcapi_set(entryName_0, "Active", "0");

			//4--- Is valid
			attriMask = 1<<12;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = INVALID;	
			tcapi_set(entryName_0, "Valid", "0");

			tcapi_set(GPONCOMMONNODE,OMCI_SOFTWAREIMAGE_STATE,"SOFTWARE_IMAGE_STATE_S2_1");
			break;	

		case SOFTWARE_IMAGE_STATE_S3_1:
			//2--- Is committed
			attriMask = 1<<14;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = COMMITTED;
			tcapi_set(entryName_1, "Committed", "1");

			//3--- Is active
			attriMask = 1<<13;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = ACTIVE;
			tcapi_set(entryName_1, "Active", "1");

			//4--- Is valid
			attriMask = 1<<12;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = VALID;
			tcapi_set(entryName_1, "Valid", "1");

			//2--- Is committed
			attriMask = 1<<14;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = UNCOMMITTED;
			tcapi_set(entryName_0, "Committed", "0");

			//3--- Is active
			attriMask = 1<<13;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = INACTIVE;
			tcapi_set(entryName_0, "Active", "0");

			//4--- Is valid
			attriMask = 1<<12;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = VALID;
			tcapi_set(entryName_0, "Valid", "1");

			tcapi_set(GPONCOMMONNODE,OMCI_SOFTWAREIMAGE_STATE,"SOFTWARE_IMAGE_STATE_S3_1");
			break;			

		case SOFTWARE_IMAGE_STATE_S4_1:
			//2--- Is committed
			attriMask = 1<<14;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = COMMITTED;
			tcapi_set(entryName_1, "Committed", "1");

			//3--- Is active
			attriMask = 1<<13;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = INACTIVE;
			tcapi_set(entryName_1, "Active", "0");

			//4--- Is valid
			attriMask = 1<<12;
			attributeValuePtr = omciGetInstAttriByMask(image_1, attriMask, &length);
			*attributeValuePtr = VALID;
			tcapi_set(entryName_1, "Valid", "1");

			//2--- Is committed
			attriMask = 1<<14;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = UNCOMMITTED;
			tcapi_set(entryName_0, "Committed", "0");

			//3--- Is active
			attriMask = 1<<13;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = ACTIVE;
			tcapi_set(entryName_0, "Active", "1");

			//4--- Is valid
			attriMask = 1<<12;
			attributeValuePtr = omciGetInstAttriByMask(image_0, attriMask, &length);
			*attributeValuePtr = VALID;
			tcapi_set(entryName_0, "Valid", "1");

			tcapi_set(GPONCOMMONNODE,OMCI_SOFTWAREIMAGE_STATE,"SOFTWARE_IMAGE_STATE_S4_1");
			break;			
			
		default:
			return -1;
	}
	tcapi_commit(entryName_0);
	tcapi_commit(entryName_1);
	tcapi_commit(GPONCOMMONNODE);
	tcapi_save();
	return 0;
}

int8 softwareImageStateMachine(int action, uint8 *currentState, uint16 instanceId, uint8 CRC_OK)
{
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "softwareImageStateMachine: action =  %d\n",action);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "softwareImageStateMachine: currentState = %d\n",*currentState);	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "softwareImageStateMachine: instanceId =  %d\n",instanceId);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "softwareImageStateMachine: CRC_OK =  %d\n",CRC_OK);

	switch(*currentState)
	{
		case SOFTWARE_IMAGE_STATE_S1_0:
			switch(action)
			{
				case MT_OMCI_MSG_TYPE_START_SW_DOWNLOAD:
					/*start download image 1*/
					if(instanceId == 1)
					{
						*currentState = SOFTWARE_IMAGE_STATE_S2_0;
						omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "softwareImageStateMachine: change currentState to  SOFTWARE_IMAGE_STATE_S2_0\n");

					}
					else
					{
						return -1;
					}
					break;

				case MT_OMCI_MSG_TYPE_ACTIVE_SW:
					/*acive image 0*/
					if(instanceId == 0)
					{
						*currentState = SOFTWARE_IMAGE_STATE_S1_0;
						return SOFTWARE_RESTART;
					}
					else
					{
						return -1;
					}					
					break;

				case MT_OMCI_MSG_TYPE_COMMIT_SW:
					/*commit image 0*/
					if(instanceId == 0)
					{
						*currentState = SOFTWARE_IMAGE_STATE_S1_0;
					}
					else
					{
						return -1;
					}
					break;

				default:
					/*invalid action*/
					return -1;
			}
			break;

		case SOFTWARE_IMAGE_STATE_S2_0:
			switch(action)
			{
				case MT_OMCI_MSG_TYPE_START_SW_DOWNLOAD:
					/*start download image 1*/
					if(instanceId == 1)
					{
						*currentState = SOFTWARE_IMAGE_STATE_S2_0;
					}
					else
					{
						return -1;
					}					
					break;

				case MT_OMCI_MSG_TYPE_ACTIVE_SW:
					/*acive image 0*/
					if(instanceId == 0)
					{
						*currentState = SOFTWARE_IMAGE_STATE_S2_0;
						return SOFTWARE_RESTART;
					}
					else
					{
						return -1;
					}						
					break;

				case MT_OMCI_MSG_TYPE_COMMIT_SW:
					/*commit image 0*/
					if(instanceId == 0)
					{
						*currentState = SOFTWARE_IMAGE_STATE_S2_0;
					}
					else
					{
						return -1;
					}						
					break;

				case MT_OMCI_MSG_TYPE_DOWNLOAD_SEC:
					/*donload image 1 sections*/
					if(instanceId == 1)
					{
						*currentState = SOFTWARE_IMAGE_STATE_S2_0;
					}
					else
					{
						return -1;
					}						
					break;	

				case MT_OMCI_MSG_TYPE_END_SW_DOWNLOAD:
					/*end download image 1*/
					if(instanceId == 1)
					{
						if(CRC_OK)	/*do CRC-32 check*/	
						{
							*currentState = SOFTWARE_IMAGE_STATE_S3_0;
						}
						else
						{
							*currentState = SOFTWARE_IMAGE_STATE_S1_0;
						}					
					}
					else
					{
						return -1;
					}					
			
					break;
				case MT_OMCI_MSG_TYPE_SW_TIMEOUT:
					/*time out,back to s1_0*/
					if(instanceId == 0)
					{	
						/*time out,back to s1_1*/			
						*currentState = SOFTWARE_IMAGE_STATE_S1_0;
					}
					else
					{
						/*time out,back to s3_1*/
						*currentState = SOFTWARE_IMAGE_STATE_S3_0;				

					}
					break;						
					
				default:
					/*invalid action*/
					return -1;
			}
			break;	

		case SOFTWARE_IMAGE_STATE_S3_0:
			switch(action)
			{
				case MT_OMCI_MSG_TYPE_START_SW_DOWNLOAD:
					/*start download image 1*/
					if(instanceId == 1)
					{
						*currentState = SOFTWARE_IMAGE_STATE_S2_0;
					}
					else
					{
						return -1;
					}						
					break;

				case MT_OMCI_MSG_TYPE_ACTIVE_SW:
					if(instanceId == 0)
					{
						/*acive image 0*/
						*currentState = SOFTWARE_IMAGE_STATE_S3_0;
						return SOFTWARE_RESTART;
					}
					else
					{
						/*acive image 1*/
						*currentState = SOFTWARE_IMAGE_STATE_S4_0;
					}					
					break;

				case MT_OMCI_MSG_TYPE_COMMIT_SW:
					if(instanceId == 0)
					{
						/*commit image 0*/
						*currentState = SOFTWARE_IMAGE_STATE_S3_0;
					}
					else
					{
						/*commit image 1*/
						*currentState = SOFTWARE_IMAGE_STATE_S4_1;
					}	
					break;
					
				default:
					/*invalid action*/
					return -1;
			}
			break;

		case SOFTWARE_IMAGE_STATE_S4_0:
			switch(action)
			{
				case MT_OMCI_MSG_TYPE_ACTIVE_SW:
					if(instanceId == 0)
					{
						/*acive image 0*/
						*currentState = SOFTWARE_IMAGE_STATE_S3_0;
					}
					else
					{
						/*acive image 1*/
						*currentState = SOFTWARE_IMAGE_STATE_S4_0;
						return SOFTWARE_RESTART;						
					}					
					break;

				case MT_OMCI_MSG_TYPE_COMMIT_SW:
					if(instanceId == 0)
					{
						/*commit image 0*/
						*currentState = SOFTWARE_IMAGE_STATE_S4_0;
					}
					else
					{
						/*commit image 1*/
						*currentState = SOFTWARE_IMAGE_STATE_S3_1;
					}	
					break;
					
				default:
					/*invalid action*/
					return -1;
			}
			break;

		case SOFTWARE_IMAGE_STATE_S4_1:
			switch(action)
			{
				case MT_OMCI_MSG_TYPE_ACTIVE_SW:
					if(instanceId == 0)
					{
						/*acive image 0*/
						*currentState = SOFTWARE_IMAGE_STATE_S4_1;
						return SOFTWARE_RESTART;												
					}
					else
					{
						/*acive image 1*/
						*currentState = SOFTWARE_IMAGE_STATE_S3_1;
					}					
					break;

				case MT_OMCI_MSG_TYPE_COMMIT_SW:
					if(instanceId == 0)
					{
						/*commit image 0*/
						*currentState = SOFTWARE_IMAGE_STATE_S3_0;
					}
					else
					{
						/*commit image 1*/
						*currentState = SOFTWARE_IMAGE_STATE_S4_1;
					}	
					break;
					
				default:
					/*invalid action*/
					return -1;
			}
			break;

		case SOFTWARE_IMAGE_STATE_S3_1:
			switch(action)
			{
				case MT_OMCI_MSG_TYPE_START_SW_DOWNLOAD:
					/*start download image 0*/
					if(instanceId == 0)
					{
						*currentState = SOFTWARE_IMAGE_STATE_S2_1;
					}
					else
					{
						return -1;
					}						
					break;

				case MT_OMCI_MSG_TYPE_ACTIVE_SW:
					if(instanceId == 1)
					{
						/*acive image 1*/
						*currentState = SOFTWARE_IMAGE_STATE_S3_1;
						return SOFTWARE_RESTART;												
					}
					else
					{
						/*acive image 0*/
						*currentState = SOFTWARE_IMAGE_STATE_S4_1;
					}					
					break;

				case MT_OMCI_MSG_TYPE_COMMIT_SW:
					if(instanceId == 1)
					{
						/*commit image 1*/
						*currentState = SOFTWARE_IMAGE_STATE_S3_1;
					}
					else
					{
						/*commit image 0*/
						*currentState = SOFTWARE_IMAGE_STATE_S4_0;
					}	
					break;
					
				default:
					/*invalid action*/
					return -1;
			}
			break;

		case SOFTWARE_IMAGE_STATE_S2_1:
			switch(action)
			{
				case MT_OMCI_MSG_TYPE_START_SW_DOWNLOAD:
					/*start download image 0*/
					if(instanceId == 0)
					{
						*currentState = SOFTWARE_IMAGE_STATE_S2_1;
					}
					else
					{
						return -1;
					}					
					break;

				case MT_OMCI_MSG_TYPE_ACTIVE_SW:
					/*acive image 1*/
					if(instanceId == 1)
					{
						*currentState = SOFTWARE_IMAGE_STATE_S2_0;
						return SOFTWARE_RESTART;						
					}
					else
					{
						return -1;
					}						
					break;

				case MT_OMCI_MSG_TYPE_COMMIT_SW:
					/*commit image 1*/
					if(instanceId == 1)
					{
						*currentState = SOFTWARE_IMAGE_STATE_S2_0;
					}
					else
					{
						return -1;
					}						
					break;

				case MT_OMCI_MSG_TYPE_DOWNLOAD_SEC:
					/*donload image 0 sections*/
					if(instanceId == 0)
					{
						*currentState = SOFTWARE_IMAGE_STATE_S2_0;
					}
					else
					{
						return -1;
					}						
					break;	

				case MT_OMCI_MSG_TYPE_END_SW_DOWNLOAD:
					/*end download image 0*/
					if(instanceId == 0)
					{
						if(CRC_OK)	/*do CRC-32 check*/	
						{
							*currentState = SOFTWARE_IMAGE_STATE_S3_1;
						}
						else
						{
							*currentState = SOFTWARE_IMAGE_STATE_S1_1;
						}					
					}
					else
					{
						return -1;
					}					
			
					break;
					
				case MT_OMCI_MSG_TYPE_SW_TIMEOUT:
					if(instanceId == 0)
					{	
						/*time out,back to s1_1*/			
						*currentState = SOFTWARE_IMAGE_STATE_S1_1;
					}
					else
					{
						/*time out,back to s3_1*/
						*currentState = SOFTWARE_IMAGE_STATE_S3_1;
					}
					break;						
					
				default:
					/*invalid action*/
					return -1;
			}
			break;	

			break;

		case SOFTWARE_IMAGE_STATE_S1_1:
			switch(action)
			{
				case MT_OMCI_MSG_TYPE_START_SW_DOWNLOAD:
					/*start download image 0*/
					if(instanceId == 0)
					{
						*currentState = SOFTWARE_IMAGE_STATE_S2_1;
					}
					else
					{
						return -1;
					}
					break;

				case MT_OMCI_MSG_TYPE_ACTIVE_SW:
					/*acive image 10*/
					if(instanceId == 1)
					{
						*currentState = SOFTWARE_IMAGE_STATE_S1_1;
						return SOFTWARE_RESTART;						
					}
					else
					{
						return -1;
					}					
					break;

				case MT_OMCI_MSG_TYPE_COMMIT_SW:
					/*commit image 1*/
					if(instanceId == 1)
					{
						*currentState = SOFTWARE_IMAGE_STATE_S1_1;
					}
					else
					{
						return -1;
					}
					break;

				default:
					/*invalid action*/
					return -1;
			}
			break;
			
		default:

			break;
	}

	return 0;
}

/*******************************************************************************************************************************
9.1.5 Cardholder

********************************************************************************************************************************/
int omciMeInitForCardholder(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p = omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;

	return 0;
}

int omciInternalCreateForCardholder(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 attriMask = 0;
	char buffer[25] = {0};
	uint8 unitType = 0;
	uint8 portNum = 0;
	
	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);

	if ((instanceId & 0xFF) == BASET101001000_SLOT)
	{
		unitType = UNIT_TYPE_BASET_101001000;
		portNum = pSystemCap->geNum;
	}
	else if ((instanceId & 0xFF) == BASET10100_SLOT)
	{
		unitType = UNIT_TYPE_BASET_10100;
		portNum =  pSystemCap->feNum;
	}
	else if ((instanceId & 0xFF) == POTS_SLOT)
	{
		unitType = UNIT_TYPE_POTS;
		portNum =  pSystemCap->potsNum;
	}
	else if ((instanceId & 0xFF) == VEIP_SLOT)
	{
		unitType = UNIT_TYPE_VEIP;
		portNum =  pSystemCap->veipNum;		
	}
	else if ((instanceId & 0xFF) == GPON_SLOT)
	{
		unitType = UNIT_TYPE_GPON24881244;
		portNum =  pSystemCap->gponNum;				
	}

	
/*0---meId = 0*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);	
	else
		goto fail;	
/*1---Actual unit type*/
	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = unitType;
	}
	else
		goto fail;	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForCardholder: tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName, length, buffer);

/*2---Expected unit type*/
	attriMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = unitType;
	}
	else
		goto fail;	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForCardholder: tmpOmciManageEntity_p->omciAttriDescriptList[2].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[2].attriName, length, buffer);
	
/*3---Expected port count	*/
	attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = portNum;
	}
	else
		goto fail;	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForCardholder: tmpOmciManageEntity_p->omciAttriDescriptList[3].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[3].attriName, length, buffer);

/*4---Expected equipment id	*/
	attriMask = 1<<12;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, pOnuCap->equipmentId, length);
	}
	else
		goto fail;	

/*5---Actual equipment id	*/
	attriMask = 1<<11;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, pOnuCap->equipmentId, length);
	}
	else
		goto fail;	

	return 0;

fail:
		omciFreeInstance(tmpomciMeInst_p);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL");
		return -1;
}

int omciInitInstForCardholder(void)
{
	int ret = 0;

	if (pSystemCap->gponNum != 0)
	{
		ret = omciInternalCreateForCardholder(OMCI_CLASS_ID_CARDHOLDER,  (0x100 + GPON_SLOT));   /*internal create func*/
		if (ret == -1)
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_CARDHOLDER fail\n");
	}

	if (pSystemCap->feNum != 0)
	{
		ret = omciInternalCreateForCardholder(OMCI_CLASS_ID_CARDHOLDER,  (0x100 + BASET10100_SLOT));   /*internal create func*/
		if (ret == -1)
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_CARDHOLDER fail\n");
	}

	if (pSystemCap->geNum != 0)
	{
		ret = omciInternalCreateForCardholder(OMCI_CLASS_ID_CARDHOLDER,  (0x100 + BASET101001000_SLOT));   /*internal create func*/
		if (ret == -1)
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_CARDHOLDER fail\n");
	}

	if (pSystemCap->potsNum != 0)
	{
		ret = omciInternalCreateForCardholder(OMCI_CLASS_ID_CARDHOLDER,  (0x100 + POTS_SLOT));   /*internal create func*/
		if (ret == -1)
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_CARDHOLDER fail\n");
	}

	if (pSystemCap->veipNum!= 0)
	{
		ret = omciInternalCreateForCardholder(OMCI_CLASS_ID_CARDHOLDER,  (0x100 + VEIP_SLOT));   /*internal create func*/
		if (ret == -1)
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_CARDHOLDER fail\n");
	}

	return ret;
}
/*******************************************************************************************************************************
9.1.6 Circuit pack

********************************************************************************************************************************/
int omciMeInitForCircuitPack(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p = omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_REBOOT] = omciRebootAction;
//	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_TEST] = omciTestAction;

	return 0;
}

int omciInternalCreateForCircuitPack(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 attriMask = 0;
	char buffer[25] = {0};
	uint8 unitType = 0;
	uint8 portNum = 0;
	
	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);

	if ((instanceId & 0xFF) == BASET101001000_SLOT)
	{
		unitType = UNIT_TYPE_BASET_101001000;
		portNum = pSystemCap->geNum;
	}
	else if ((instanceId & 0xFF) == BASET10100_SLOT)
	{
		unitType = UNIT_TYPE_BASET_10100;
		portNum = pSystemCap->feNum;
	}
	else if ((instanceId & 0xFF) == POTS_SLOT)
	{
		unitType = UNIT_TYPE_POTS;
		portNum = pSystemCap->potsNum;
	}
	else if ((instanceId & 0xFF) == VEIP_SLOT)
	{
		unitType = UNIT_TYPE_VEIP;
		portNum = pSystemCap->veipNum;		
	}
	else if ((instanceId & 0xFF) == GPON_SLOT)
	{
		unitType = UNIT_TYPE_GPON24881244;
		portNum = pSystemCap->gponNum;				
	}


/*0---meId = instanceId*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);	
	else
		goto fail;	
/*1---Type*/
	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = unitType;
	}
	else
		goto fail;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForCircuitPack: tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName, length, buffer);

/*2---Number of ports*/
	attriMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = portNum;
	}
	else
		goto fail;	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForCircuitPack: tmpOmciManageEntity_p->omciAttriDescriptList[2].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[2].attriName, length, buffer);
	
/*3---Serial number	*/
	attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, pOnuCap->sn, length);
	}
	else
		goto fail;	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForCircuitPack: tmpOmciManageEntity_p->omciAttriDescriptList[3].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[3].attriName, length, buffer);

/*4---Version*/
	attriMask = 1<<12;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, pOnuCap->version, length);
	}
	else
		goto fail;	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForCircuitPack: tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName, length, buffer);

	
/*9---Equipment id */
	attriMask = 1<<7;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, pOnuCap->equipmentId, length);
	}
	else
		goto fail;	

	if ((instanceId & 0xFF) == GPON_SLOT)
	{
		/*11---Total T-CONT buffer*/
		attriMask = 1<<5;
		attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
		if (attributeValuePtr != NULL)
		{
			*attributeValuePtr = (uint8)pAniCap->totalTcontNum;
		}
		else
			goto fail;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForCircuitPack: tmpOmciManageEntity_p->omciAttriDescriptList[11].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[12].attriName, length, buffer);

		/*12---Total priority queue*/
		attriMask = 1<<4;
		attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
		if (attributeValuePtr != NULL)
		{
			*attributeValuePtr = (uint8)pAniCap->totalTcontNum * 8;
		}
		else
			goto fail;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForCircuitPack: tmpOmciManageEntity_p->omciAttriDescriptList[12].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[13].attriName, length, buffer);

		/*13---Total traffic scheduler*/
		attriMask = 1<<3;
		attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
		if (attributeValuePtr != NULL)
		{
			*attributeValuePtr = (uint8)pOnuCap->totalTSNum;
		}
		else
			goto fail;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForCircuitPack: tmpOmciManageEntity_p->omciAttriDescriptList[13].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[14].attriName, length, buffer);
	}

	return 0;

fail:
	omciFreeInstance(tmpomciMeInst_p);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL");
	return -1;

}

int omciInitInstForCircuitPack(void)
{
	int ret = 0;

	if (pSystemCap->gponNum != 0)
	{
		ret = omciInternalCreateForCircuitPack(OMCI_CLASS_ID_CIRCUIT_PACK, (0x100 + GPON_SLOT));   /*internal create func*/
		if (ret == -1)
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_CIRCUIT_PACK fail\n");
	}

	if (pSystemCap->feNum != 0)
	{
		ret = omciInternalCreateForCircuitPack(OMCI_CLASS_ID_CIRCUIT_PACK, (0x100 + BASET10100_SLOT));   /*internal create func*/
		if (ret == -1)
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_CIRCUIT_PACK fail\n");
	}

	if (pSystemCap->geNum != 0)
	{
		ret = omciInternalCreateForCircuitPack(OMCI_CLASS_ID_CIRCUIT_PACK,  (0x100 + BASET101001000_SLOT));   /*internal create func*/
		if (ret == -1)
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_CIRCUIT_PACK fail\n");
	}

	if (pSystemCap->potsNum != 0)
	{
		ret = omciInternalCreateForCircuitPack(OMCI_CLASS_ID_CIRCUIT_PACK, (0x100 + POTS_SLOT));   /*internal create func*/
		if (ret == -1)
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_CIRCUIT_PACK fail\n");
	}

	if (pSystemCap->veipNum!= 0)
	{
		ret = omciInternalCreateForCircuitPack(OMCI_CLASS_ID_CIRCUIT_PACK, (0x100 + VEIP_SLOT));   /*internal create func*/
		if (ret == -1)
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_CIRCUIT_PACK fail\n");
	}

	return ret;
}
/*******************************************************************************************************************************
9.1.7 ONU power shedding

********************************************************************************************************************************/
int omciMeInitForOnuPowerShedding(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;

	/* if need ONU auto create a ME instance, create here */
	return 0;
}

int omciInternalCreateForOnuPowerShedding(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;

	if(instanceId != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "ONU power shedding only a instance \n");
		return -1;
	}

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	
	return 0;
}


int omciInitInstForOnuPowerShedding(void)
{
	int ret = 0;

	ret = omciInternalCreateForOnuPowerShedding(OMCI_CLASS_ID_ONU_POWER_SHEDDING, 0);/*internal create func*/
	if (ret == -1)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_ONU_POWER_SHEDDING fail\n");

	return ret;
}


/*******************************************************************************************************************************
9.1.8 Port mapping package 

********************************************************************************************************************************/
int omciMeInitForPortMapping(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
//	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_NEXT] = omciGetNextAction;

	/* if need ONU auto create a ME instance, create here */
	return 0;
}

int omciInternalCreateForPortMapping(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 attriMask = 0;
	char buffer[64] = {0};
	int portId[8] = {0};
	int8 allPortsNum = 0;
	char attrName[25] = {0};
	int i = 0;
	int list = 0;

	if(instanceId != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "Port Mapping only one instance\n");
		return -1;
	}

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	
/*0---meId = 0*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	if (attributeValuePtr != NULL)
	{
		put16(attributeValuePtr, 0);
	}
	else 
		goto fail;

/*1---MaxPorts*/
	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		memset(buffer, 0, sizeof(buffer));
		tcapi_get("GPON_ONU", "MaxPorts", buffer);
		*attributeValuePtr = (uint8)atoi(buffer);
		allPortsNum = *attributeValuePtr;
	}
	else 
		goto fail;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForPortMapping: tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName, length, buffer);

/*2---Ports List*/
	attriMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		for (list = 1; allPortsNum > 0; allPortsNum -= MAP_PORTS_NUM, list ++)
		{
		memset(buffer, 0, sizeof(buffer));
			memset(attrName, 0, sizeof(attrName));
			sprintf(attrName, "PortList%d", list);
			tcapi_get("GPON_ONU", attrName, buffer);
		sscanf(buffer, "%d,%d,%d,%d,%d,%d,%d,%d", &portId[0], &portId[1], &portId[2], &portId[3], &portId[4], &portId[5],&portId[6], &portId[7]);
			
		for (i=0; i<MAP_PORTS_NUM; i++)
			{
			attributeValuePtr = put16(attributeValuePtr, (uint16)portId[i]);
	}
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForPortMapping: allPortsNum = %d\n", allPortsNum);
		}
	}
	else 
		goto fail;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForPortMapping: tmpOmciManageEntity_p->omciAttriDescriptList[2].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[2].attriName, length, buffer);

	
	return 0;

fail:
	omciFreeInstance(tmpomciMeInst_p);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL");
	return -1;
}

int omciInitInstForPortMapping(void)
{
	int ret = 0;

	ret = omciInternalCreateForPortMapping(OMCI_CLASS_ID_PORT_MAPPING, 0);   /*internal create func*/
	if (ret == -1)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_PORT_MAPPING fail\n");

	return ret;
}

/*******************************************************************************************************************************
9.1.9 Equipment extension package 

********************************************************************************************************************************/

int omciMeInitForExtensionPackage(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;

	return 0;
}


/*******************************************************************************************************************************
9.1.10 Protection data 

********************************************************************************************************************************/
int omciMeInitForProtectionData(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;

	return 0;
}


/*******************************************************************************************************************************
9.1.11 Equipment protection profile 

********************************************************************************************************************************/
int omciMeInitForProtectionProfile(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciDeleteAction;	
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;
}
/*******************************************************************************************************************************
9.1.12 ONU remote debug

********************************************************************************************************************************/
int omciInitInstForOnuRemoteDebug(void)
{
	int ret=0;
	ret = omciInternalCreateForOnuRemoteDebug(OMCI_CLASS_ID_ONU_REMOTE_DEBUG,0);
	return ret;
}


int omciMeInitForOnuRemoteDebug(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateActionForOnuRemoteDebug;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_NEXT] = omciGetNextAction;

	return 0;
}

int omciCreateActionForOnuRemoteDebug(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc)
{
	uint16 classId;
	uint16 instanceId;

	if(msgSrc == INTERNAL_MSG)
	{
		classId = get16((uint8*)&(omciPayLoad->meId));
		instanceId = get16((uint8*)&(omciPayLoad->meId)+2);
		omciInternalCreateForOnuRemoteDebug(classId, instanceId);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"omciMeInitForOnuRemoteDebug: ONU create success\n");
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"omciMeInitForOnuRemoteDebug: fail, can be only created by ONU!\n");
		return -1;
	}
}

int omciInternalCreateForOnuRemoteDebug(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;

	if(instanceId != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"ONU remote debug only a instance \n");
		return -1;
	}

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	if(tmpOmciManageEntity_p == NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n omciInternalCreateForOnuRemoteDebug:---tmpOmciManageEntity_p == NULL\n");	
		return -1;
	}
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	if(tmpomciMeInst_p == NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n omciInternalCreateForOnuRemoteDebug:---tmpomciMeInst_p == NULL \n");
		return -1;
	}
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"omciInternalCreateForOnuRemoteDebug: tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName);
	if(attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);
	else
		return -1;

	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, (1<<15), &length);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"omciInternalCreateForOnuRemoteDebug: tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName);
	if(attributeValuePtr != NULL)
		*attributeValuePtr = DEBUG_CMD_ASCII_FORMAT;//ASCII Format
	else
		return -1;
	
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	
	return 0;
}


int32 setRemoteDebugCmd(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length=0;
	int status=0;
	char debugCmd[64]={0};
	char testCmd[128]={0};
	omciTableAttriValue_t *tableConent_ptr=NULL;
	int ret = 0;
	FILE *fp=NULL;	
	char *tableBuffer = NULL;
	int filesize=0;

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);
	memcpy(attributeValuePtr, value, length);
	memcpy(debugCmd, value, length);

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\nsetRemoteDebugCmd: length = %d\r\n",length);	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\nsetRemoteDebugCmd: %s\r\n",debugCmd);

#if 0
	sprintf(testCmd, "%s > /tmp/omci/debug_reply_table.txt", debugCmd);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\nsetRemoteDebugCmd: testCmd = %s\r\n",testCmd);
#else
	sprintf(testCmd, "echo \"ADSL Link status: down ADSL Link status: up\" > /tmp/omci/debug_reply_table.txt");
#endif

	if((status = system(testCmd)) < 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\nsetRemoteDebugCmd:  fail!\r\n");
		return -1;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\nsetRemoteDebugCmd:  success!\r\n");
	}

	fp = fopen("/tmp/omci/debug_reply_table.txt", "r");
	if(fp == NULL)
	{
		return -1;
	}
	else
	{
		/*1.calculate the table size*/
		fseek(fp, 0, SEEK_END);
		filesize = ftell(fp);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\nsetRemoteDebugCmd:---filesize = %d",filesize);

		/*2.malloc memory for table*/
		tableBuffer = (char *)malloc(filesize * sizeof(char));	
		if(tableBuffer == NULL)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\nsetRemoteDebugCmd:---malloc fail!!\r\n");
			fclose(fp);
			return -1;
		}
		memset(tableBuffer,0, filesize);
		
		/*3.get  the table copy */
		fseek(fp, 0, SEEK_SET);
		fread(tableBuffer, filesize, sizeof(char),fp);
		fclose(fp);
		
	}

	tableConent_ptr = omciGetTableValueByIndex((omciMeInst_t *)meInstantPtr, 3);
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setRemoteDebugCmd->update table content in instance table list");
	
	/*update table content in instance table list*/
	if(tableConent_ptr != NULL){//del table content  in instance table list
		ret = omciDelTableValueOfInst((omciMeInst_t *)meInstantPtr, 3);
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setRemoteDebugCmd->omciDelTableValueOfInst failure");
			if(tableBuffer) {
				free(tableBuffer);
			}
			return -1;
		}
	}
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setRemoteDebugCmd->filesize = %d",filesize);
		
	if(filesize == 0){//current table is empty
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setRemoteDebugCmd->current table is empty");
		if(tableBuffer) {
			free(tableBuffer);
		}
		return 0;
	}

//	omciDumpTable(tableBuffer, filesize);


	tableConent_ptr = omciAddTableValueToInst((omciMeInst_t *)meInstantPtr, 3, filesize, (uint8 *)tableBuffer);
	if(tableBuffer) {
		free(tableBuffer);
	}
	if(tableConent_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setRemoteDebugCmd->omciAddTableValueToInst failure");
		return -1;
	}	
	
	return 0;
}

int32 getRemoteDebugReplyTab(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	int tableSize=0;
	uint16 instId=0;
	int ret = 0;
	char *tableBuffer = NULL;

	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	omciTableAttriValue_t *tmp_ptr=NULL;
	instId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	tmp_ptr = omciGetTableValueByIndex(tmpomciMeInst_p, omciAttribute->attriIndex);
	if(tmp_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [getRemoteDebugReplyTab]->tmp_ptr is NULL");
		goto failure;
	}
	
	/*1.calculate the table size*/
	tableSize = tmp_ptr->tableSize;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n [getRemoteDebugReplyTab]: tableSize = %d",tableSize);

	/*2.get table content*/
	tableBuffer = (char *)tmp_ptr->tableValue_p;
	if(tableBuffer == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n [getRemoteDebugReplyTab]->tableBuffer is NULL");
		goto failure;	
	}

	/*3. add the table to omci archtecture*/
	ret = omciAddTableContent(tmpomciMeInst_p->classId, instId, (uint8 *)tableBuffer, tableSize ,omciAttribute->attriName);
	
	if (ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n[getRemoteDebugReplyTab]----omciAddTableContent fail!!\r\n");
		goto failure;
	}
	
	return getTheValue(value, (char*)&tableSize, 4, omciAttribute);

failure:
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n [getRemoteDebugReplyTab]  failure! ");
	tableSize = 0;
	return getTheValue(value, (char*)&tableSize, 4, omciAttribute);
	
}

/*******************************************************************************************************************************
9.1.13 ONU-E

********************************************************************************************************************************/
int omciMeInitForOnuE(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_REBOOT] = omciRebootAction;

	return 0;
}

int omciInternalCreateForOnuE(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 attriMask = 0;
	char buffer[25] = {0};
	
	if(instanceId != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "ONU-E only one instance\n");
		return -1;
	}

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	
/*0---meId = 0*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	if (attributeValuePtr != NULL)
	{
		put16(attributeValuePtr, 0);
	}
	else
		goto fail;
	
/*1---Vendor Id*/
	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		memset(buffer, 0, sizeof(buffer));
		tcapi_get("GPON_ONU", "VendorId", buffer);
		memcpy(attributeValuePtr, buffer, 4);
	}
	else
		goto fail;

/*2---Version*/
	attriMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		memset(buffer, 0, sizeof(buffer));
		
#if !defined(TCSUPPORT_CUC) && !defined(TCSUPPORT_CT) 
		tcapi_get("GPON_ONU", "Version", buffer);
#endif
		memcpy(attributeValuePtr, buffer, 14);
	}
	else
		goto fail;
	
/*3---SerialNumber*/
	attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		memset(buffer, 0, sizeof(buffer));
		tcapi_get("GPON_ONU", "SerialNumber", buffer);
		memcpy(attributeValuePtr, buffer, 8);
	}
	else
		goto fail;
	return 0;

fail:
	omciFreeInstance(tmpomciMeInst_p);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL");
	return -1;

}
int omciInitInstForOnuE(void)
{
	int ret = 0;

	ret = omciInternalCreateForOnuE(OMCI_CLASS_ID_ONU_E,  0);   /*internal create func*/
	if (ret == -1)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_ONU_E fail\n");

	return ret;
}

/*******************************************************************************************************************************
9.1.14 ONU dynamic power management control

********************************************************************************************************************************/
int omciMeInitForOnuDynamicPower(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;

	return 0;
}
#ifdef TCSUPPORT_OMCI_ALCATEL
int omciInternalCreateForONTOpticalSupervisionStatus(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 attriMask = 0;
	char buffer[25] = {0};

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);

/* 0---meId = 0 */
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);

/* 1---Power feed voltage measurement indicator */
	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 1; //0 is not supported;1 is supported.
	else
		goto fail;

/* 3---Receive optical power measurement indicator */
	attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 1; //0 is not supported;1 is supported.
	else
		goto fail;	

/* 5---Transmit optical power measurement indicator */
	attriMask = 1<<11;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 1; //0 is not supported;1 is supported.
	else
		goto fail;	

/* 7---Laser Bias Current measurement indicator */
	attriMask = 1<<9;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 1; //0 is not supported;1 is supported.
	else
		goto fail;	

/* 9---Temperature measurement indicator */
	attriMask = 1<<7;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 1; //0 is not supported;1 is supported.
	else
		goto fail;		

	return 0;

fail:
	omciFreeInstance(tmpomciMeInst_p);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL \n");
	return -1;
}

int omciInitInstForONTOpticalSupervisionStatus(void)
{
	int ret = 0;

	ret = omciInternalCreateForONTOpticalSupervisionStatus(OMCI_CLASS_ID_ONT_OPTICAL_SUPERVISION_STATUS,  ((GPON_SLOT<<8) + 1)); /*internal create func*/
	if (ret == -1)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_ONT_OPTICAL_SUPERVISION_STATUS fail\n");

	return ret;
}

int omciMeInitForONTOpticalSupervisionStatus(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForONTOpticalSupervisionStatus \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	
	return 0;
}

int32 getPowerFeedVoltageValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char tempBuffer[125] = {0};
	int phyPara[5] = {0};
		
	tcapi_get("GPON_ANI", "PhyTransParameters", tempBuffer);
	sscanf(tempBuffer, "Temperature:%x,Voltage:%x,TxCurrent:%x,TxPower:%x,RxPower:%x", &phyPara[0], &phyPara[1], &phyPara[2], &phyPara[3], &phyPara[4]);
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, (uint16)phyPara[1]/10000); 
	else
		return -1;
	
	return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute);	
}

int32 getReceiveOpticalPowerValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char tempBuffer[125] = {0};
	int phyPara[5] = {0};

	tcapi_get("GPON_ANI", "PhyTransParameters", tempBuffer);
	
	sscanf(tempBuffer, "Temperature:%x,Voltage:%x,TxCurrent:%x,TxPower:%x,RxPower:%x", &phyPara[0], &phyPara[1], &phyPara[2], &phyPara[3], &phyPara[4]);
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		put16(attributeValuePtr, (uint16)phyPara[4]/10000);
	}
	else
		return -1;
	
	return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute);	
}

int32 getTransmitOpticalPowerValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char tempBuffer[125] = {0};
	int phyPara[5] = {0};

	tcapi_get("GPON_ANI", "PhyTransParameters", tempBuffer);
	
	sscanf(tempBuffer, "Temperature:%x,Voltage:%x,TxCurrent:%x,TxPower:%x,RxPower:%x", &phyPara[0], &phyPara[1], &phyPara[2], &phyPara[3], &phyPara[4]);
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		put16(attributeValuePtr, (uint16)phyPara[3]/10000);
	}
	else
		return -1;
	
	return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute);	
}

int32 getLaserBiasCurrentValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char tempBuffer[125] = {0};
	int phyPara[5] = {0};

	tcapi_get("GPON_ANI", "PhyTransParameters", tempBuffer);
	
	sscanf(tempBuffer, "Temperature:%x,Voltage:%x,TxCurrent:%x,TxPower:%x,RxPower:%x", &phyPara[0], &phyPara[1], &phyPara[2], &phyPara[3], &phyPara[4]);
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		put16(attributeValuePtr, (uint16)phyPara[2] * 2/(1000 * 1000));
	}
	else
		return -1;
	
	return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute);	
}

int32 getTemperatureValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char tempBuffer[125] = {0};
	int phyPara[5] = {0};

	tcapi_get("GPON_ANI", "PhyTransParameters", tempBuffer);
	
	sscanf(tempBuffer, "Temperature:%x,Voltage:%x,TxCurrent:%x,TxPower:%x,RxPower:%x", &phyPara[0], &phyPara[1], &phyPara[2], &phyPara[3], &phyPara[4]);
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , omciAttribute->attriName, &length);
	
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, (uint16)phyPara[0]/256);
	else 
		return -1;
	
	return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute);	
}


int omciInternalCreateForONTGenericV2(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 attriMask = 0;
	char buffer[32] = {0};

	if(instanceId != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInternalCreateForONTGenericV2 only one instance, instanceId=%d,error!\n",instanceId);
		return -1;
	}

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);

/* 0---meId = 0 */
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);

/* 1---IGMPSnoopEnable */
	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 1; 
	else
		goto fail;

/* 2---ForceDataStorageCommand */
	attriMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0; 
	else
		goto fail;	

/* 3---TimeOfLastDataStorageOperation */
	attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	memset(buffer, 0, sizeof(buffer));
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, buffer, 4);
	else
		goto fail;	

/* 4---CraftEnableDisableIndicator */
	attriMask = 1<<12;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0; 
	else
		goto fail;	

/* 5---EthernetPortToPortTrafficEnableIndicator */
	attriMask = 1<<11;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0;
	else
		goto fail;

/* 6---StaticMulticastVLANTranslationIndicator */
	attriMask = 1<<10;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0;
	else
		goto fail;
	
/* 7---ALUProprietaryMECapabilitiesBitmap */
	attriMask = 1<<9;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		put32(attributeValuePtr, 0x390c0);  //0xd00390c0
	else
		goto fail;

/* 8---FlexiblePQAssignmentCapability */
	attriMask = 1<<8;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0;
	else
		goto fail;
	
/* 9---SLIDCapability */
	attriMask = 1<<7;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0;
	else
		goto fail;

/* 10---YPSerialNumber */
	attriMask = 1<<6;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	memset(buffer, 0, sizeof(buffer));
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, buffer, 12);
	else
		goto fail;

/* 11---ALUSpecialBehaviorsField */
	attriMask = 1<<5;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	memset(buffer, 0, sizeof(buffer));
	if (attributeValuePtr != NULL) {
		put32(attributeValuePtr, 0x1);
	}
	else
		goto fail;

/* 12---ALUProprietaryMECapabilitiesBitmapPart2 */
	attriMask = 1<<4;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL) {
		memset(buffer, 0, sizeof(buffer));
		memcpy(attributeValuePtr, buffer, 25);
		*attributeValuePtr = 0x0;
	}
	else
		goto fail;

	return 0;

fail:
	omciFreeInstance(tmpomciMeInst_p);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL \n");
	return -1;
}

int omciMeInitForONTGenericV2(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForONTGenericV2 \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_AVC] = omciHandshakeAVCResponseAction;
	
	return 0;
}

int omciInitInstForONTGenericV2(void)
{
	int ret = 0;

	ret = omciInternalCreateForONTGenericV2(OMCI_CLASS_ID_ONT_GENERIC_V2,  0); /*internal create func*/
	if (ret == -1)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_ONT_GENERIC_V2 fail\n");

	return ret;
}


int32 getEthernetPortToPortTrafficEnableIndicatorValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 val  = 0;
	uint8 mask[MAX_OMCI_LAN_PORT_NUM] = {0};

	if(((macMT7530GetPortBrgInd(OMCI_MAC_BRIDGE_LAN_PORT1, &mask[0]) == 0)	&& ((mask[0] & MAX_OMCI_LAN_PORT_MASK)== MAX_OMCI_LAN_PORT_MASK))
		&& ((macMT7530GetPortBrgInd(OMCI_MAC_BRIDGE_LAN_PORT2, &mask[1]) == 0)	&& ((mask[1] & MAX_OMCI_LAN_PORT_MASK)== MAX_OMCI_LAN_PORT_MASK))
		&& ((macMT7530GetPortBrgInd(OMCI_MAC_BRIDGE_LAN_PORT3, &mask[2]) == 0)	&& ((mask[2] & MAX_OMCI_LAN_PORT_MASK)== MAX_OMCI_LAN_PORT_MASK))
		&& ((macMT7530GetPortBrgInd(OMCI_MAC_BRIDGE_LAN_PORT4, &mask[3]) == 0)	&& ((mask[3] & MAX_OMCI_LAN_PORT_MASK)== MAX_OMCI_LAN_PORT_MASK))){
		val = 1;
	}else{
		val = 0;
	}
#if 0
	for(i = 0; i<MAX_OMCI_LAN_PORT_NUM; i++){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMBSPPortBridingIndValue mask[%d]= 0x02x", i, mask[i]);	
	}
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMBSPPortBridingIndValue val = %d", val);
#endif

	return getTheValue(value, (char *)&val, 1, omciAttribute);	
}

int32 setEthernetPortToPortTrafficEnableIndicatorValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 portBridgingInd = 0;
	int	ret = 0;
	uint8 mask[MAX_OMCI_LAN_PORT_NUM] = {0};
	int i = 0;

	if(omci.omciCap.systemCap.onuType != 2) //2--hgu modes
	{
	portBridgingInd = *((uint8*)value);
	switch(portBridgingInd){
		case 0:
			for(i = 0; i<MAX_OMCI_LAN_PORT_NUM; i++){
				mask[i] = (OMCI_LAN_PORT1_OTHER_MASK | (1<<i));
			}
			break;
		case 1:
			for(i = 0; i<MAX_OMCI_LAN_PORT_NUM; i++){
				mask[i] = MAX_OMCI_LAN_PORT_DEFAULT_MASK;
			}
			break;
		default:
			goto failure;
			break;
	
	}

	for(i = 0; i<MAX_OMCI_LAN_PORT_NUM; i++){
		if(macMT7530SetPortBrgInd(i, mask[i]) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBSPPortBridingIndValue->macMT7530SetPortBrgInd mask[%d]= 0x%02x, fail[%d]",i, mask[i],__LINE__);
		}
	}
	}

	ret = 0;
	return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
failure:
	ret = -1;
	return ret;
}
#endif

