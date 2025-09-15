/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

#include	"rt_config.h"

VOID RT28xx_UpdateBeaconToAsic(
	RTMP_ADAPTER *pAd,
	INT apidx,
	ULONG FrameLen,
	ULONG UpdatePos)
{

}
	

VOID RTMPResetTxRxRingMemory(RTMP_ADAPTER *pAd)
{
}


VOID RTMPFreeTxRxRingMemory(RTMP_ADAPTER *pAd)
{
	UINT32 Index;

	for (Index = 0; Index < NUM_OF_TX_RING; Index++)
	{
		NdisFreeSpinLock(&SDIOTxPacketListLock[Index]);
	}
}


NDIS_STATUS RTMPAllocTxRxRingMemory(RTMP_ADAPTER *pAd)
{	
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	INT32 Index;
	ULONG ErrorValue = 0;
	
	DBGPRINT(RT_DEBUG_TRACE, ("-->RTMPAllocTxRxRingMemory\n"));
	

	for (Index = 0; Index < NUM_OF_TX_RING; Index++)
	{
		NdisAllocateSpinLock(pAd, &pAd->SDIOTxPacketListLock[Index]);
	}
		
	/* Alloc MGMT ring desc buffer except Tx ring allocated eariler */


	/* Alloc Beacon ring desc buffer */
						
	/* Allocate Tx ring descriptor's memory (BMC)*/

	/* Alloc RX ring desc memory except Tx ring allocated eariler */



	DBGPRINT_S(("<-- RTMPAllocTxRxRingMemory, Status=%x\n", Status));

	return Status;
}


NDIS_STATUS RTMPInitTxRxRingMemory(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS Status;
	UINT32 Index;
	unsigned long Flags;	

	for (Index = 0; Index < NUM_OF_TX_RING; Index++)
	{	
		/* 
         * (5 TX rings = 4 ACs + 1 HCCA)
         */
		RTMP_SPIN_LOCK_IRQSAVE(&pAd->SDIOTxPacketListLock[Index], &Flags);
		DlListInit(&pAd->SDIOTxPacketList[Index]);
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->SDIOTxPacketListLock[Index], &Flags);
	}
		
	return Status;
}


/*
========================================================================
Routine Description:
    Disable DMA.

Arguments:
	*pAd				the raxx interface data pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RT28XXDMADisable(
	IN RTMP_ADAPTER 		*pAd)
{
	/* no use*/
}


/*
========================================================================
Routine Description:
    Enable DMA.

Arguments:
	*pAd				the raxx interface data pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RT28XXDMAEnable(RTMP_ADAPTER *pAd)
{

}


/********************************************************************
  *
  *	SDIO Radio on/off Related functions.
  *
  ********************************************************************/
VOID RT28xxSdioMlmeRadioOn(
	IN PRTMP_ADAPTER pAd)
{
}


VOID RT28xxSdioMlmeRadioOFF(
	IN PRTMP_ADAPTER pAd)
{
}


VOID RT28xxSdioAsicRadioOff(RTMP_ADAPTER *pAd)
{
	DBGPRINT(RT_DEBUG_TRACE, ("--> %s\n", __FUNCTION__));

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);

	DBGPRINT(RT_DEBUG_TRACE, ("<== %s\n", __FUNCTION__));

}


VOID RT28xxSdioAsicRadioOn(RTMP_ADAPTER *pAd)
{
	/* make some traffic to invoke EvtDeviceD0Entry callback function*/
	
	DBGPRINT(RT_DEBUG_TRACE, ("<== %s\n", __FUNCTION__));
}


