/*
 ***************************************************************************
 * MediaTek Inc. 
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mtsdio_data.c
*/

#include	"rt_config.h"


/* <-------------- to be removed to another header file */


void MTSDIOProcessSoftwareInterrupt(IN RTMP_ADAPTER *pAd);
void MTSDIOProcessFwOwnBackInterrupt(IN RTMP_ADAPTER *pAd);
void MTSDIOProcessAbnormalInterrupt(IN RTMP_ADAPTER *pAd);
void MTSDIOProcessTxInterrupt(IN RTMP_ADAPTER *pAd);
void MTSDIOProcessRxInterrupt(IN RTMP_ADAPTER *pAd);
INT32 MTSDIOSingleProcessRx0(RTMP_ADAPTER *pAd, INT32 Length);
INT32 MTSDIOSingleProcessRx1(RTMP_ADAPTER *pAd, INT32 Length);

typedef struct _INT_EVENT_MAP_T {
    UINT32      u4Int;
    UINT32      u4Event;
} INT_EVENT_MAP_T, *P_INT_EVENT_MAP_T;

typedef VOID (*IST_EVENT_FUNCTION)(RTMP_ADAPTER *);

enum ENUM_INT_EVENT_T {
    INT_EVENT_ABNORMAL,
    INT_EVENT_SW_INT,
    INT_EVENT_TX,
    INT_EVENT_RX,
    INT_EVENT_NUM
};

#define PORT_INDEX_LMAC                         0
#define PORT_INDEX_MCU                          1

/* MCU quque index */
typedef enum _ENUM_MCU_Q_INDEX_T {
    MCU_Q0_INDEX = 0,
    MCU_Q1_INDEX,
    MCU_Q2_INDEX,
    MCU_Q3_INDEX,
    MCU_Q_NUM
} ENUM_MCU_Q_INDEX_T;

/* LMAC Tx queue index */
typedef enum _ENUM_MAC_TXQ_INDEX_T {
    MAC_TXQ_AC0_INDEX = 0,
    MAC_TXQ_AC1_INDEX,
    MAC_TXQ_AC2_INDEX,
    MAC_TXQ_AC3_INDEX,
    MAC_TXQ_AC4_INDEX,
    MAC_TXQ_AC5_INDEX,
    MAC_TXQ_AC6_INDEX,
    MAC_TXQ_BMC_INDEX,
    MAC_TXQ_BCN_INDEX,
    MAC_TXQ_AC10_INDEX,
    MAC_TXQ_AC11_INDEX,
    MAC_TXQ_AC12_INDEX,
    MAC_TXQ_AC13_INDEX,
    MAC_TXQ_AC14_INDEX,
    MAC_TXQ_NUM
} ENUM_MAC_TXQ_INDEX_T;

/* HIF Tx interrupt status queue index*/
typedef enum _ENUM_HIF_TX_INDEX_T {
    HIF_TX_AC0_INDEX = 0,   /* HIF TX: AC0 packets */
    HIF_TX_AC1_INDEX,       /* HIF TX: AC1 packets */
    HIF_TX_AC2_INDEX,       /* HIF TX: AC2 packets */
    HIF_TX_AC3_INDEX,       /* HIF TX: AC3 packets */
    HIF_TX_AC4_INDEX,       /* HIF TX: AC4 packets */
    HIF_TX_AC5_INDEX,       /* HIF TX: AC5 packets */
    HIF_TX_AC6_INDEX,       /* HIF TX: AC6 packets */
    HIF_TX_BMC_INDEX,       /* HIF TX: BMC packets */
    HIF_TX_BCN_INDEX,       /* HIF TX: BCN packets */
    HIF_TX_AC10_INDEX,      /* HIF TX: AC10 packets */
    HIF_TX_AC11_INDEX,      /* HIF TX: AC11 packets */
    HIF_TX_AC12_INDEX,      /* HIF TX: AC12 packets */
    HIF_TX_AC13_INDEX,      /* HIF TX: AC13 packets */
    HIF_TX_AC14_INDEX,      /* HIF TX: AC14 packets */
    HIF_TX_FFA_INDEX,       /* HIF TX: free-for-all */
    HIF_TX_CPU_INDEX,       /* HIF TX: CPU */
    HIF_TX_NUM       /* Maximum number of HIF TX port. */
} ENUM_HIF_TX_INDEX_T;

typedef struct _TX_RESOURCE_CONTROL_T {
    /* HW TX queue definition */
    UINT8      ucDestPortIndex;
    UINT8      ucDestQueueIndex;
    /* HIF Interrupt status index*/
    UINT8      ucHifTxQIndex;
} TX_RESOURCE_CONTROL_T, *PTX_RESOURCE_CONTROL_T;

static const TX_RESOURCE_CONTROL_T arTcResourceControl[TC_NUM] = {
    /* dest port index, dest queue index,   HIF TX queue index */
    /* First HW queue */
    {PORT_INDEX_LMAC,   MAC_TXQ_AC0_INDEX,  HIF_TX_AC0_INDEX},
    {PORT_INDEX_LMAC,   MAC_TXQ_AC1_INDEX,  HIF_TX_AC1_INDEX},
    {PORT_INDEX_LMAC,   MAC_TXQ_AC2_INDEX,  HIF_TX_AC2_INDEX},
    {PORT_INDEX_LMAC,   MAC_TXQ_AC3_INDEX,  HIF_TX_AC3_INDEX},
    {PORT_INDEX_MCU,    MCU_Q1_INDEX,       HIF_TX_CPU_INDEX},
    {PORT_INDEX_LMAC,   MAC_TXQ_AC4_INDEX,  HIF_TX_AC4_INDEX},

    /* Second HW queue */
    {PORT_INDEX_LMAC,   MAC_TXQ_AC10_INDEX, HIF_TX_AC10_INDEX},
    {PORT_INDEX_LMAC,   MAC_TXQ_AC11_INDEX, HIF_TX_AC11_INDEX},
    {PORT_INDEX_LMAC,   MAC_TXQ_AC12_INDEX, HIF_TX_AC12_INDEX},
    {PORT_INDEX_LMAC,   MAC_TXQ_AC13_INDEX, HIF_TX_AC13_INDEX},
    {PORT_INDEX_LMAC,   MAC_TXQ_AC14_INDEX, HIF_TX_AC14_INDEX},
};

static INT_EVENT_MAP_T arIntEventMapTable[] = {
    {ABNORMAL_INT,                         INT_EVENT_ABNORMAL},
    {D2H_SW_INT,                           INT_EVENT_SW_INT},
    {TX_DONE_INT,                          INT_EVENT_TX},
    {(RX0_DONE_INT | RX1_DONE_INT),        INT_EVENT_RX}

};


#define SDIO_TX_DESC_LONG_FORMAT_LENGTH_DW       7       //in unit of double word
#define SDIO_TX_DESC_LONG_FORMAT_LENGTH          (SDIO_TX_DESC_LONG_FORMAT_LENGTH_DW << 2)
#define SDIO_TX_DESC_PADDING_LENGTH_DW           0       //in unit of double word
#define SDIO_TX_DESC_PADDING_LENGTH              (SDIO_TX_DESC_PADDING_LENGTH_DW << 2)
#define SDIO_TX_PAGE_SIZE_IS_POWER_OF_2          TRUE
#define SDIO_TX_PAGE_SIZE_IN_POWER_OF_2          7
#define SDIO_TX_PAGE_SIZE        128
#define SDIO_TX_MAX_PAGE_PER_FRAME    MAX_AGGREGATION_SIZE
#define SDIO_TX_BUFF_COUNT_TC0   1
#define SDIO_TX_BUFF_COUNT_TC1   34
#define SDIO_TX_BUFF_COUNT_TC2   1
#define SDIO_TX_BUFF_COUNT_TC3   1
#define SDIO_TX_BUFF_COUNT_TC4   4
#define SDIO_TX_BUFF_COUNT_TC5   1


#define SDIO_TX_PAGE_COUNT_TC0   (SDIO_TX_BUFF_COUNT_TC0 * SDIO_TX_MAX_PAGE_PER_FRAME / SDIO_TX_PAGE_SIZE)
#define SDIO_TX_PAGE_COUNT_TC1   (SDIO_TX_BUFF_COUNT_TC0 * SDIO_TX_MAX_PAGE_PER_FRAME / SDIO_TX_PAGE_SIZE)
#define SDIO_TX_PAGE_COUNT_TC2   (SDIO_TX_BUFF_COUNT_TC0 * SDIO_TX_MAX_PAGE_PER_FRAME / SDIO_TX_PAGE_SIZE)
#define SDIO_TX_PAGE_COUNT_TC3   (SDIO_TX_BUFF_COUNT_TC0 * SDIO_TX_MAX_PAGE_PER_FRAME / SDIO_TX_PAGE_SIZE)
#define SDIO_TX_PAGE_COUNT_TC4   (SDIO_TX_BUFF_COUNT_TC0 * SDIO_TX_MAX_PAGE_PER_FRAME / SDIO_TX_PAGE_SIZE)
#define SDIO_TX_PAGE_COUNT_TC5   (SDIO_TX_BUFF_COUNT_TC0 * SDIO_TX_MAX_PAGE_PER_FRAME / SDIO_TX_PAGE_SIZE)


/* --------------- end ----------------------------------> */

static const UINT8 ucIntEventMapSize = (sizeof(arIntEventMapTable) / sizeof(INT_EVENT_MAP_T));

static IST_EVENT_FUNCTION apfnEventFuncTable[] = {
    MTSDIOProcessAbnormalInterrupt,            /*!< INT_EVENT_ABNORMAL */
    MTSDIOProcessSoftwareInterrupt,            /*!< INT_EVENT_SW_INT   */
    MTSDIOProcessTxInterrupt,                  /*!< INT_EVENT_TX       */
    MTSDIOProcessRxInterrupt,                  /*!< INT_EVENT_RX       */
};



VOID
MTSDIOTxResetResource (
    IN RTMP_ADAPTER *pAd
)
{
    P_TX_CTRL_T prTxCtrl = NULL;
    unsigned long flags = 0;

    ASSERT(pAd);

    prTxCtrl = &pAd->rTxCtrl;

    RTMP_SPIN_LOCK_IRQSAVE(&pAd->TcCountLock, &flags);

    NdisZeroMemory(prTxCtrl->rTc.au2TxDonePageCount, sizeof(prTxCtrl->rTc.au2TxDonePageCount));
    prTxCtrl->rTc.ucNextTcIdx = TC0_INDEX;
    prTxCtrl->rTc.u2AvailiablePageCount = 0;

    prTxCtrl->rTc.au2MaxNumOfBuffer[TC0_INDEX] = SDIO_TX_BUFF_COUNT_TC0;
    prTxCtrl->rTc.au2FreeBufferCount[TC0_INDEX] = SDIO_TX_BUFF_COUNT_TC0;

    prTxCtrl->rTc.au2MaxNumOfBuffer[TC1_INDEX] = SDIO_TX_BUFF_COUNT_TC1;
    prTxCtrl->rTc.au2FreeBufferCount[TC1_INDEX] = SDIO_TX_BUFF_COUNT_TC1;

    prTxCtrl->rTc.au2MaxNumOfBuffer[TC2_INDEX] = SDIO_TX_BUFF_COUNT_TC2;
    prTxCtrl->rTc.au2FreeBufferCount[TC2_INDEX] = SDIO_TX_BUFF_COUNT_TC2;

    prTxCtrl->rTc.au2MaxNumOfBuffer[TC3_INDEX] = SDIO_TX_BUFF_COUNT_TC3;
    prTxCtrl->rTc.au2FreeBufferCount[TC3_INDEX] = SDIO_TX_BUFF_COUNT_TC3;

    prTxCtrl->rTc.au2MaxNumOfBuffer[TC4_INDEX] = SDIO_TX_BUFF_COUNT_TC4;
    prTxCtrl->rTc.au2FreeBufferCount[TC4_INDEX] = SDIO_TX_BUFF_COUNT_TC4;

    prTxCtrl->rTc.au2MaxNumOfBuffer[TC5_INDEX] = SDIO_TX_BUFF_COUNT_TC5;
    prTxCtrl->rTc.au2FreeBufferCount[TC5_INDEX] = SDIO_TX_BUFF_COUNT_TC5;

    prTxCtrl->rTc.au2MaxNumOfPage[TC0_INDEX] = SDIO_TX_PAGE_COUNT_TC0;
    prTxCtrl->rTc.au2FreePageCount[TC0_INDEX] = SDIO_TX_PAGE_COUNT_TC0;

    prTxCtrl->rTc.au2MaxNumOfPage[TC1_INDEX] = SDIO_TX_PAGE_COUNT_TC1;
    prTxCtrl->rTc.au2FreePageCount[TC1_INDEX] = SDIO_TX_PAGE_COUNT_TC1;

    prTxCtrl->rTc.au2MaxNumOfPage[TC2_INDEX] = SDIO_TX_PAGE_COUNT_TC2;
    prTxCtrl->rTc.au2FreePageCount[TC2_INDEX] = SDIO_TX_PAGE_COUNT_TC2;

    prTxCtrl->rTc.au2MaxNumOfPage[TC3_INDEX] = SDIO_TX_PAGE_COUNT_TC3;
    prTxCtrl->rTc.au2FreePageCount[TC3_INDEX] = SDIO_TX_PAGE_COUNT_TC3;

    prTxCtrl->rTc.au2MaxNumOfPage[TC4_INDEX] = SDIO_TX_PAGE_COUNT_TC4;
    prTxCtrl->rTc.au2FreePageCount[TC4_INDEX] = SDIO_TX_PAGE_COUNT_TC4;

    prTxCtrl->rTc.au2MaxNumOfPage[TC5_INDEX] = SDIO_TX_PAGE_COUNT_TC5;
    prTxCtrl->rTc.au2FreePageCount[TC5_INDEX] = SDIO_TX_PAGE_COUNT_TC5;

    RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->TcCountLock, &flags);

    return;
}

INT32
MTSDIOTxAcquireResource(
    IN RTMP_ADAPTER *pAd,
    IN UINT8       ucTC,
    IN UINT8       ucPageCount
)
{
    P_TX_CTRL_T prTxCtrl = NULL;
    INT32 u4Ret = 0;
    unsigned long flags = 0;

    prTxCtrl = &pAd->rTxCtrl;

    RTMP_SPIN_LOCK_IRQSAVE(&pAd->TcCountLock, &flags);
    if (prTxCtrl->rTc.au2FreePageCount[ucTC] >= ucPageCount) {
        prTxCtrl->rTc.au2FreePageCount[ucTC] -= ucPageCount;
        prTxCtrl->rTc.au2FreeBufferCount[ucTC] = (prTxCtrl->rTc.au2FreePageCount[ucTC] / SDIO_TX_MAX_PAGE_PER_FRAME);
    }
    RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->TcCountLock, &flags);

    return u4Ret;
}


UINT8
MTSDIOTxGetPageCount (
    IN UINT32 u4FrameLength,
    IN BOOLEAN fgIncludeDesc
    )
{
    UINT32 u4RequiredBufferSize = 0;
    UINT8  ucPageCount = 0;

  /* Frame Buffer
   *  |<--Tx Descriptor-->|<--Tx descriptor padding-->|<--802.3/802.11 Header-->|<--Header padding-->|<--Payload-->|
   */
    if(fgIncludeDesc) {
        u4RequiredBufferSize = u4FrameLength;
    }
    else {
        u4RequiredBufferSize =
            SDIO_TX_DESC_LONG_FORMAT_LENGTH +
            SDIO_TX_DESC_PADDING_LENGTH +
            u4FrameLength;
    }

    if(SDIO_TX_PAGE_SIZE_IS_POWER_OF_2) {
        ucPageCount = (UINT8)((u4RequiredBufferSize + (SDIO_TX_PAGE_SIZE - 1)) >> SDIO_TX_PAGE_SIZE_IN_POWER_OF_2);
    }
    else {
        ucPageCount = (UINT8)((u4RequiredBufferSize + (SDIO_TX_PAGE_SIZE - 1)) / SDIO_TX_PAGE_SIZE);
    }

    return ucPageCount;
}



VOID
MTSDIOTxAdjustTcq (
    IN RTMP_ADAPTER *pAd
)
{
    ASSERT(pAd);
    // TODO: implement QM for adaptive quota control
}

BOOLEAN 
MTSDIOTxCalculateResource(
    IN RTMP_ADAPTER *pAd,
    IN UINT16 *au2TxRlsCnt,
    OUT UINT16 *au2FreeTcResource
)
{
    P_TX_TCQ_STATUS_T prTcqStatus = NULL;
    prTcqStatus = &pAd->rTxCtrl.rTc;
    UINT16 u2TotalTxDonePageCount = 0;
    UINT8 ucTcIdx = 0;
    prTcqStatus = &pAd->rTxCtrl.rTc;
    BOOLEAN bStatus = FALSE;

    prTcqStatus->u2AvailiablePageCount += au2TxRlsCnt[HIF_TX_FFA_INDEX];

    for (ucTcIdx = TC0_INDEX; ucTcIdx < TC_NUM; ucTcIdx++) {
        prTcqStatus->au2TxDonePageCount[ucTcIdx] += au2TxRlsCnt[arTcResourceControl[ucTcIdx].ucHifTxQIndex];
        u2TotalTxDonePageCount += prTcqStatus->au2TxDonePageCount[ucTcIdx];
    }

    DBGPRINT(RT_DEBUG_OFF, ("Tx Done INT result, FFA[%u] AC[%u:%u:%u:%u:%u] CPU[%u]\n",
                au2TxRlsCnt[HIF_TX_FFA_INDEX],
                au2TxRlsCnt[HIF_TX_AC0_INDEX],
                au2TxRlsCnt[HIF_TX_AC1_INDEX],
                au2TxRlsCnt[HIF_TX_AC2_INDEX],
                au2TxRlsCnt[HIF_TX_AC3_INDEX],
                au2TxRlsCnt[HIF_TX_AC4_INDEX],
                au2TxRlsCnt[HIF_TX_CPU_INDEX]));


    if (prTcqStatus->u2AvailiablePageCount) {
        if (prTcqStatus->u2AvailiablePageCount >= u2TotalTxDonePageCount) {

            NdisCopyMemory(au2FreeTcResource, prTcqStatus->au2TxDonePageCount,
                    sizeof(prTcqStatus->au2TxDonePageCount));
            NdisZeroMemory(prTcqStatus->au2TxDonePageCount,
                    sizeof(prTcqStatus->au2TxDonePageCount));
            prTcqStatus->u2AvailiablePageCount -= u2TotalTxDonePageCount;

            if (prTcqStatus->u2AvailiablePageCount) {
            }
        } else {
            ucTcIdx = prTcqStatus->ucNextTcIdx;
            while (prTcqStatus->u2AvailiablePageCount) {
                if (prTcqStatus->u2AvailiablePageCount >= prTcqStatus->au2TxDonePageCount[ucTcIdx]) {
                    au2FreeTcResource[ucTcIdx] = prTcqStatus->au2TxDonePageCount[ucTcIdx];
                    prTcqStatus->u2AvailiablePageCount -= prTcqStatus->au2TxDonePageCount[ucTcIdx];
                    ucTcIdx++;
                    ucTcIdx %= TC_NUM;
                } else {
                    au2FreeTcResource[ucTcIdx] = prTcqStatus->u2AvailiablePageCount;
                    prTcqStatus->au2TxDonePageCount[ucTcIdx] -= prTcqStatus->u2AvailiablePageCount;
                    prTcqStatus->u2AvailiablePageCount = 0;
                }
                prTcqStatus->ucNextTcIdx = ucTcIdx;
            }
            bStatus = TRUE;
        }
    } else {
        DBGPRINT(RT_DEBUG_OFF, ("No FFA count to release\n"));
    }

    return bStatus;
}

BOOLEAN
MTSDIOTxReleaseResource(
    IN RTMP_ADAPTER *pAd,
    IN UINT16       *au2TxRlsCnt
)
{
    P_TX_TCQ_STATUS_T prTcqStatus = NULL;
    BOOLEAN bStatus = FALSE;
    UINT16 au2FreeTcResource[TC_NUM] = {0};
    UINT32 u4BufferCountToBeFeed = 0;
    UINT8 i = 0;
    unsigned long flags = 0;

    prTcqStatus = &pAd->rTxCtrl.rTc;

    if (MTSDIOTxCalculateResource(pAd, au2TxRlsCnt, au2FreeTcResource)) {

        RTMP_SPIN_LOCK_IRQSAVE(&pAd->TcCountLock, &flags);

        for (i = TC0_INDEX; i < TC_NUM; i++) {
            /* Real page counter */
            prTcqStatus->au2FreePageCount[i] += au2FreeTcResource[i];
            /* Buffer counter. For development only */
            /* Convert page count to buffer count */
            u4BufferCountToBeFeed = (prTcqStatus->au2FreePageCount[i] / SDIO_TX_MAX_PAGE_PER_FRAME);
            prTcqStatus->au2FreeBufferCount[i] = u4BufferCountToBeFeed;

            if (au2FreeTcResource[i]) {
                DBGPRINT(RT_DEBUG_OFF, ("Release: TC%lu ReturnPageCnt[%u] FreePageCnt[%u] FreeBufferCnt[%u]\n",
                            i,
                            au2FreeTcResource[i],
                            prTcqStatus->au2FreePageCount[i],
                            prTcqStatus->au2FreeBufferCount[i]));
            }
        }
        RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->TcCountLock, &flags);

        bStatus = TRUE;
    }

    return bStatus;
}


VOID
MTSDIOProcessIST_impl (
    IN RTMP_ADAPTER *pAd,
    IN UINT32 u4IntStatus
)
{
    UINT32 u4Status = 0;
    UINT32 u4IntCount = 0;
    P_INT_EVENT_MAP_T prIntEventMap = NULL;
    //unsigned long flags = 0;

    ASSERT(pAd);

    //pAd->IntStatus = u4IntStatus;
	if (pAd->IntStatus & ABNORMAL_INT)
	{

	}
	if (pAd->IntStatus & (RX0_DONE_INT | RX1_DONE_INT))
	{
		MTSDIOProcessRxInterrupt(pAd);
	}	
	if (pAd->IntStatus & TX_DONE_INT)
	{

	}

}


VOID
MTSDIOProcessIST (IN RTMP_ADAPTER *pAd)
{
    UINT32 u4IntStatus = 0;
    unsigned long flags = 0;

    //MTSDIORead32(pAd, WHISR, &u4IntStatus);
     //u4IntStatus = pAd->IntStatus;
	DBGPRINT(RT_DEBUG_ERROR, ("MTSDIOProcessIST pAd->IntStatus: %x\n",pAd->IntStatus));

    if (pAd->IntStatus & ~(WHIER_DEFAULT | FW_OWN_BACK_INT_EN)) {
 	 	//RTMP_SPIN_LOCK_IRQSAVE(&pAd->IntStatusLock, &flags);
       	 pAd->IntStatus = pAd->IntStatus & WHIER_DEFAULT;
		//RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->IntStatusLock, &flags);
    }

    MTSDIOProcessIST_impl(pAd, pAd->IntStatus);
}



VOID
MTSDIOProcessSoftwareInterrupt(IN RTMP_ADAPTER *pAd)
{
    return;
}


VOID
MTSDIOProcessFwOwnBackInterrupt(IN RTMP_ADAPTER *pAd)
{
    return;
}


VOID
MTSDIOProcessAbnormalInterrupt(IN RTMP_ADAPTER *pAd)
{
    return;
}

VOID
MTSDIOProcessTxInterrupt(IN RTMP_ADAPTER *pAd)
{
    P_TX_CTRL_T prTxCtrl = NULL;
    UINT32 au4TxCount[8];

    MTSDIORead32(pAd, &au4TxCount[0], WTQCR0); // AC0
    MTSDIORead32(pAd, &au4TxCount[1], WTQCR1); // AC1
    MTSDIORead32(pAd, &au4TxCount[2], WTQCR2); // AC2
    MTSDIORead32(pAd, &au4TxCount[3], WTQCR3); // AC3
    MTSDIORead32(pAd, &au4TxCount[4], WTQCR4); // AC4
    MTSDIORead32(pAd, &au4TxCount[5], WTQCR5); // AC5
    MTSDIORead32(pAd, &au4TxCount[6], WTQCR6); // AC6
    MTSDIORead32(pAd, &au4TxCount[7], WTQCR7); // AC7
    //MTSDIOTxReleaseResource(pAd, (PUINT8) au4TxCount);

    //MTSDIOTxAdjustTcq(pAd);
    /* TODO:  Check if there's any packets */
    return;
}


void MTSDIOProcessRxInterrupt(IN RTMP_ADAPTER *pAd)
{
    UINT32 Value = 0;
    UINT32 RX0PacketLen = 0, RX1PacketLen = 0;

    ASSERT(pAd);

    MTSDIORead32(pAd, WRPLR, &Value);
    RX0PacketLen = GET_RX0_PACKET_LENGTH(Value);
    RX1PacketLen = GET_RX1_PACKET_LENGTH(Value);

    if (RX0PacketLen != 0) {
        MTSDIOSingleProcessRx0(pAd, RX0PacketLen);
    } 
    if (RX1PacketLen != 0) {
        MTSDIOSingleProcessRx1(pAd, RX1PacketLen);
    }
}


INT32 MTSDIOCmdTx(RTMP_ADAPTER *pAd, UCHAR *Buf, UINT32 Length)
{
    INT32 Ret;
    
    Ret = MTSDIOMultiWrite(pAd, WTDR1, Buf, Length); 

    return Ret;
}


INT32 MTSDIODataTx(RTMP_ADAPTER *pAd, UCHAR *Buf, UINT32 Length)
{
	INT32 Ret;
	
	Ret = MTSDIOMultiWrite(pAd, WTDR1, Buf, Length); 

	return Ret;
}


INT32 MTSDIODataRx0(RTMP_ADAPTER *pAd, UCHAR *Buf, UINT32 Length)
{
	INT32 Ret;

	Ret = MTSDIOMultiRead(pAd, WRDR0, Buf, Length);

	return Ret;
}


INT32 MTSDIODataRx1(RTMP_ADAPTER *pAd, UCHAR *Buf, UINT32 Length)
{
	INT32 Ret;

	Ret = MTSDIOMultiRead(pAd, WRDR1, Buf, Length);

	return Ret;
}


VOID MTSDIOAddWorkerTaskList(RTMP_ADAPTER *pAd)
{
	SDIOWorkTask *NewTask;
	unsigned long flags;

	os_alloc_mem(pAd, (UCHAR **)&NewTask, sizeof(*NewTask));

	if (NewTask == NULL)
		return;

	RTMP_SPIN_LOCK_IRQSAVE(&pAd->SdioWorkTaskLock, &flags);
	DlListAddTail(&pAd->SdioWorkTaskList, &NewTask->List);
	RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->SdioWorkTaskLock, &flags);
}


INT32 MTSDIOSingleProcessRx0(RTMP_ADAPTER *pAd, INT32 Length)
{
	DBGPRINT(RT_DEBUG_OFF, ("%s: =============>\n", __FUNCTION__));

	UINT32 i;
	PUCHAR buf;

	if (Length >= pAd->BlockSize)
		Length = (Length + ((-Length) & (pAd->BlockSize - 1)));

	pAd->SDIORxPacket = RTMP_AllocateFragPacketBuffer(pAd, Length);
	DBGPRINT(RT_DEBUG_OFF, ("%s: =============>\n", __FUNCTION__));

	if (!pAd->SDIORxPacket)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: allocate net packet fail\n", 
														__FUNCTION__));

		return;
	}
	
	MTSDIODataRx0(pAd, GET_OS_PKT_DATAPTR(pAd->SDIORxPacket), Length);

	SET_OS_PKT_LEN(pAd->SDIORxPacket, Length);
	buf = GET_OS_PKT_DATAPTR(pAd->SDIORxPacket);

	DBGPRINT(RT_DEBUG_OFF, ("%s: ================len = %x ===\n", __FUNCTION__, Length));
	for(i=0;i<Length;i++){
		printk("%x,",*(buf+i));
	}
	printk("\n");

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | 
									fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMPFreeNdisPacket(pAd, pAd->SDIORxPacket);
		return;
	}

	rtmp_rx_done_handle(pAd);
}


INT32 MTSDIOSingleProcessRx1(RTMP_ADAPTER *pAd, INT32 Length)
{
	UINT32 i;
	PUCHAR buf;

	if (Length >= pAd->BlockSize)
		Length = (Length + ((-Length) & (pAd->BlockSize - 1)));

	pAd->SDIORxPacket = RTMP_AllocateFragPacketBuffer(pAd, Length);
	
	if (!pAd->SDIORxPacket)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: allocate net packet fail\n", 
														__FUNCTION__));

		return;
	}

	MTSDIODataRx1(pAd, GET_OS_PKT_DATAPTR(pAd->SDIORxPacket), Length);

	SET_OS_PKT_LEN(pAd->SDIORxPacket, Length);
	buf = GET_OS_PKT_DATAPTR(pAd->SDIORxPacket);
	DBGPRINT(RT_DEBUG_OFF, ("%s: ================len = %x ===\n", __FUNCTION__, Length));
	for(i=0;i<Length;i++){
		printk("%x,",*(buf+i));
	}
	printk("\n");

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | 
									fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMPFreeNdisPacket(pAd, pAd->SDIORxPacket);
		return;
	}

	rtmp_rx_done_handle(pAd);
}


VOID MTSDIODataWorkerTask(RTMP_ADAPTER *pAd)
{
	//UINT32 IntStatus;
	void* handle = pAd->OS_Cookie;
	struct sdio_func *dev_func = ((POS_COOKIE)handle)->sdio_dev;
	UINT32 Value;

	//MTSDIORead32(pAd, WHISR, &pAd->IntStatus);	
	//IntStatus = pAd->IntStatus;

	/*  Process RX0/RX1, Abnormal, TX done ISR */
	if (pAd->IntStatus & WHIER_DEFAULT) {
         	MTSDIOProcessIST(pAd);
			pAd->IntStatus =0;
			/* Enable Interrupt */
			RTMP_SDIO_WRITE32(pAd, WHLPCR, W_INT_EN_SET);
	}
	/*  Process Command */

	AndesMTSendCmdMsgToSdio(pAd);

	/*  Process TX */
}


VOID MTSDIODataIsr(RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT32 Value;
	unsigned long flags;
	struct sdio_func *dev_func =pObj->sdio_dev;
	INT32 Ret = 0;

	/* Disable Interrupt */
	RTMP_SDIO_WRITE32(pAd, WHLPCR, W_INT_EN_CLR);

	Value  = sdio_readl(dev_func, WHISR, &Ret);
	
	if (Ret) {
		DBGPRINT(RT_DEBUG_ERROR, ("RTSDIORead32 failure!\n"));
	}

	if (Value) {
//	DBGPRINT(RT_DEBUG_ERROR, ("%s()!!!! WHISR:%x \n", __FUNCTION__,Value));

		//RTMP_SPIN_LOCK_IRQSAVE(&pAd->IntStatusLock, &flags);
		pAd->IntStatus |= Value;
		//RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->IntStatusLock, &flags);
		DBGPRINT(RT_DEBUG_TRACE, ("%s() disbale interrupt \n", __FUNCTION__));
		DBGPRINT(RT_DEBUG_OFF, ("%s() ======================> \n", __FUNCTION__));
		MTSDIOAddWorkerTaskList(pAd);
		queue_work(pObj->SdioWq, &pObj->SdioWork); 
	}  
}
