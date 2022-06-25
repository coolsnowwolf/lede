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

/****************************************************************************

        Abstract:

        All related IEEE802.11f IAPP + IEEE802.11r/11k IAPP extension.

***************************************************************************/

//#define IAPP_EVENT_LOG
//#define IAPP_TEST

/* Include */
#include "rtmpiapp.h"
#include "iappdefs.h"
#include "rt_config.h"
#include "rt_typedef.h"

/* Local Variable */
#define IAPP_SHOW_IP_HTONL(__IP)                                               \
  (htonl(__IP) >> 24) & 0xFF, (htonl(__IP) >> 16) & 0xFF,                      \
      (htonl(__IP) >> 8) & 0xFF, (htonl(__IP) & 0xFF)

#define IAPP_SHOW_IP(__IP)                                                     \
  (__IP) & 0xFF, ((__IP) >> 8) & 0xFF, ((__IP) >> 16) & 0xFF,                  \
      ((__IP) >> 24) & 0xFF

#define IAPP_IDENTIFIER_GET(__CB) (__CB)->PacketIdentifier++

#define IAPP_CMD_BUF_ALLOCATE(__pCmdBuf, __pBufMsg, __BufLen)                  \
  {                                                                            \
    if ((__BufLen) > IAPP_MAX_RCV_PKT_SIZE) {                                  \
      printf("iapp> Command Length is too large!\n");                          \
      __pBufMsg = NULL;                                                        \
    } else                                                                     \
      __pBufMsg = __pCmdBuf;                                                   \
  }

#define IAPP_RSP_BUF_ALLOCATE(__pRspBuf, __pBufMsg, __BufLen)                  \
  {                                                                            \
    if ((__BufLen) > IAPP_MAX_RCV_PKT_SIZE) {                                  \
      printf("iapp> Response Length is too large!\n");                         \
      __pBufMsg = NULL;                                                        \
    } else                                                                     \
      __pBufMsg = __pRspBuf;                                                   \
  }

static RTMP_IAPP IAPP_Ctrl_Block;
static UINT32 IAPP_MemAllocNum, IAPP_MemFreeNum;

/* Local Function */
#ifdef IAPP_EVENT_LOG
static BOOLEAN IAPP_EventLogClean(VOID);
static BOOLEAN IAPP_EventLogHandle(IAPP_IN PRT_802_11_EVENT_TABLE pEvtTab);
static VOID IAPP_EventLog_Query(VOID);
static VOID IAPP_SM_InfoHandle(IAPP_IN CHAR *pMsg);
#endif // IAPP_EVENT_LOG //

#if 0
static BOOLEAN IAPP_IP2MACTransfer(
	IAPP_IN		UCHAR				*pMac,
	IAPP_OUT	struct in_addr		*pApInfo);
#endif

static BOOLEAN IAPP_ArgumentParse(IAPP_IN RTMP_IAPP *pCtrlBK,
                                  IAPP_IN INT32 Argc, IAPP_IN CHAR *pArgv[]);

static BOOLEAN IAPP_DSIfInfoGet(IAPP_IN RTMP_IAPP *pCtrlBK);

BOOLEAN IAPP_IoctlToWLAN(IAPP_IN RTMP_IAPP *pCtrlBK, IAPP_IN INT32 Param,
                         IAPP_IN CHAR *pData, IAPP_IN INT32 *pDataLen,
                         IAPP_IN UCHAR ApIdx, IAPP_IN INT32 Flags);
static INT32 IAPP_IPC_MSG_Init(IAPP_IN RTMP_IAPP *pCtrlBK);
static BOOLEAN IAPP_L2UpdateFrameSend(IAPP_IN RTMP_IAPP *pCtrlBK,
                                      IAPP_IN UCHAR *pMac);

static BOOLEAN IAPP_MsgProcess(IAPP_IN RTMP_IAPP *pCtrlBK,
                               IAPP_IN INT32 MsgSubType, IAPP_IN UCHAR *pMsg,
                               IAPP_IN INT32 Len, IAPP_IN INT32 if_idx);

static VOID IAPP_PID_Backup(IAPP_IN pid_t PID);
static VOID IAPP_PID_Kill(VOID);

static BOOLEAN IAPP_SIG_Process(IAPP_IN RTMP_IAPP *pCtrlBK,
                                IAPP_IN UCHAR *WiFiMAC,
                                IAPP_IN RT_SIGNAL_STRUC *pSig,
                                IAPP_IN INT32 Len, IAPP_IN UCHAR *pCmdBuf,
                                IAPP_IN UCHAR *pRspBuf);

static BOOLEAN IAPP_SocketClose(IAPP_IN RTMP_IAPP *pCtrlBK);
static BOOLEAN IAPP_SocketOpen(IAPP_IN RTMP_IAPP *pCtrlBK);

static VOID IAPP_Start(IAPP_IN RTMP_IAPP *pCtrlBK);

#define IAPP_UDP_PACKET_SEND(__pCtrlBK, __pPkt, __Len, __pRspBuf, __IfIdx)     \
  IAPP_UDP_PacketSend(__pCtrlBK, (UCHAR *)(__pPkt), (UINT32)(__Len),           \
                      __pRspBuf, __IfIdx)
static BOOLEAN IAPP_UDP_PacketSend(IAPP_IN RTMP_IAPP *pCtrlBK,
                                   IAPP_IN UCHAR *pPkt, IAPP_IN UINT32 PktLen,
                                   IAPP_IN UCHAR *pRspBuf,
                                   IAPP_IN INT32 if_idx);

#define IAPP_TCP_PACKET_SEND(__pCtrlBK, __pPkt, __Len, __PeerIP, __FlgUdp,     \
                             __pRspBuf, __IfIdx)                               \
  IAPP_TCP_PacketSend(__pCtrlBK, (UCHAR *)(__pPkt), (UINT32)(__Len), __PeerIP, \
                      __FlgUdp, __pRspBuf, __IfIdx)
static BOOLEAN IAPP_TCP_PacketSend(IAPP_IN RTMP_IAPP *pCtrlBK,
                                   IAPP_IN UCHAR *pPkt, IAPP_IN UINT32 PktLen,
                                   IAPP_IN UINT32 PeerIP,
                                   IAPP_IN BOOLEAN FlgUsingUdpWhenNoIP,
                                   IAPP_IN UCHAR *pRspBuf,
                                   IAPP_IN INT32 if_idx);

static VOID IAPP_RcvHandler(IAPP_IN RTMP_IAPP *pCtrlBK, IAPP_IN INT32 Sig);

#ifdef FT_KDP_FUNC_SOCK_COMM
static VOID IAPP_RcvHandlerRawDrv(IAPP_IN RTMP_IAPP *pCtrlBK,
                                  IAPP_IN UCHAR *pPktBuf,
                                  IAPP_IN UCHAR *pCmdBuf,
                                  IAPP_IN UCHAR *pRspBuf);
static VOID IAPP_RcvHandlerRawRRB(IAPP_IN RTMP_IAPP *pCtrlBK,
                                  IAPP_IN UCHAR *pPktBuf,
                                  IAPP_IN UCHAR *pCmdBuf,
                                  IAPP_IN UCHAR *pRspBuf);

#ifdef IAPP_OS_VXWORKS
BOOLEAN IAPP_RcvHandlerRawDrvVxWorks(IAPP_IN VOID *pCookie, IAPP_IN long Type,
                                     IAPP_IN M_BLK_ID pMblk,
                                     IAPP_IN LL_HDR_INFO *pLinkHdrInfo,
                                     IAPP_IN VOID *pSpare);
BOOLEAN IAPP_RcvHandlerRawRRBVxWorks(IAPP_IN VOID *pCookie, IAPP_IN long Type,
                                     IAPP_IN M_BLK_ID pMblk,
                                     IAPP_IN LL_HDR_INFO *pLinkHdrInfo,
                                     IAPP_IN VOID *pSpare);
#endif // IAPP_OS_VXWORKS //
#endif // FT_KDP_FUNC_SOCK_COMM //

static VOID IAPP_RcvHandlerUdp(IAPP_IN RTMP_IAPP *pCtrlBK,
                               IAPP_IN UCHAR *pPktBuf, IAPP_IN UCHAR *pCmdBuf,
                               IAPP_IN UCHAR *pRspBuf);
static VOID IAPP_RcvHandlerTcp(IAPP_IN RTMP_IAPP *pCtrlBK,
                               IAPP_IN UCHAR *pPktBuf, IAPP_IN UCHAR *pCmdBuf,
                               IAPP_IN UCHAR *pRspBuf);

static VOID IAPP_RcvHandlerMoveReq(IAPP_IN RTMP_IAPP *pCtrlBK,
                                   IAPP_IN UCHAR *pPktBuf,
                                   IAPP_IN UINT32 PeerIP,
                                   IAPP_IN UCHAR *pCmdBuf,
                                   IAPP_IN UCHAR *pRspBuf);
static VOID IAPP_RcvHandlerSSB(IAPP_IN RTMP_IAPP *pCtrlBK,
                               IAPP_IN UCHAR *pPktBuf, IAPP_IN UINT32 PeerIP,
                               IAPP_IN UCHAR *pCmdBuf);
static VOID IAPP_RcvHandlerApInfor(IAPP_IN RTMP_IAPP *pCtrlBK,
                                   IAPP_IN UCHAR Type, IAPP_IN UCHAR *pPktBuf,
                                   IAPP_IN UINT32 PeerIP,
                                   IAPP_IN UCHAR *pCmdBuf,
                                   IAPP_IN INT32 if_idx);

static VOID IAPP_Usage(VOID);
static VOID IAPP_USR2Handle(IAPP_IN INT32 Sig);
static VOID IAPP_TerminateHandle(IAPP_IN INT32 Sig);

static VOID FT_KDP_SecurityBlockSend(IAPP_IN RTMP_IAPP *pCtrlBK,
                                     IAPP_IN FT_KDP_EVT_HEADER *pEvtHdr,
                                     IAPP_IN VOID *pEvt, IAPP_IN UCHAR *pRspBuf,
                                     IAPP_IN UCHAR *WifiMAC,
                                     IAPP_IN INT32 if_idx);
static VOID FT_KDP_SecurityBlockAck(IAPP_IN RTMP_IAPP *pCtrlBK,
                                    IAPP_IN FT_KDP_EVT_HEADER *pEvtHdr,
                                    IAPP_IN VOID *pEvt, IAPP_IN UCHAR *pRspBuf,
                                    IAPP_IN INT32 if_idx);
static VOID FT_KDP_InformationRequestSend(IAPP_IN RTMP_IAPP *pCtrlBK,
                                          IAPP_IN FT_KDP_EVT_HEADER *pEvtHdr,
                                          IAPP_IN VOID *pEvt,
                                          IAPP_IN UCHAR *pRspBuf,
                                          IAPP_IN INT32 if_idx);
static VOID FT_KDP_InformationResponseSend(IAPP_IN RTMP_IAPP *pCtrlBK,
                                           IAPP_IN FT_KDP_EVT_HEADER *pEvtHdr,
                                           IAPP_IN VOID *pEvt,
                                           IAPP_IN UCHAR *pRspBuf,
                                           IAPP_IN INT32 if_idx);
static VOID FT_KDP_InformationReportSend(IAPP_IN RTMP_IAPP *pCtrlBK,
                                         IAPP_IN FT_KDP_EVT_HEADER *pEvtHdr,
                                         IAPP_IN VOID *pEvt,
                                         IAPP_IN UCHAR *pRspBuf,
                                         IAPP_IN INT32 if_idx);
static VOID FT_RRB_ActionForward(IAPP_IN RTMP_IAPP *pCtrlBK,
                                 IAPP_IN FT_KDP_EVT_HEADER *pEvtHdr,
                                 IAPP_IN VOID *pEvt, IAPP_IN UINT16 PacketType,
                                 IAPP_IN UCHAR *pMacDa, IAPP_IN UCHAR *pMacSa,
                                 IAPP_IN UCHAR *pMacAp, IAPP_IN UCHAR *pRspBuf,
                                 IAPP_IN INT32 if_idx);

/* ---------------------------- No Use Function (Backup Use) ---------------- */

#ifdef IAPP_EVENT_LOG
/*
========================================================================
Routine Description:
        Clean event log file content.

Arguments:
        None

Return Value:
        None

Note:
        EVENT_LOG_FILE = "/etc/Wireless/RT61AP/RT61APEvt.dat"
========================================================================
*/
static BOOLEAN IAPP_EventLogClean(VOID) {
  FILE *pFile;

  /* re-open and truncate file to zero length by using "wb" */
  pFile = fopen(EVENT_LOG_FILE, "wb");

  if (pFile == NULL) {
    DBGPRINT(RT_DEBUG_ERROR,
             ("iapp> Open eventlog file %s failed!\n", EVENT_LOG_FILE));
    return FALSE;
  } /* End of if */

  fclose(pFile);
  return TRUE;
} /* End of IAPP_EventLogClean */

/*
========================================================================
Routine Description:
        Write a list events to event log file.

Arguments:
        pEvtTab		- event list

Return Value:
        TRUE		- write successfully
        FALSE		- write fail

Note:
========================================================================
*/
static BOOLEAN IAPP_EventLogHandle(IAPP_IN PRT_802_11_EVENT_TABLE pEvtTab) {
  FILE *pFile;
  INT32 FileLen;
  INT32 IdEvt;

  /* for append binary file format */
  pFile = fopen(EVENT_LOG_FILE, "ab");
  if (pFile == NULL) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Open eventlog file failed!\n"));
    return FALSE;
  } /* End of if */

  FileLen = ftell(pFile);

  if (FileLen > IAPP_MAX_SIZE_OF_EVENT_LOG) {
    DBGPRINT(RT_DEBUG_ERROR,
             ("iapp> eventlog file size exceeds limitation(FileLen=%d)!\n",
              FileLen));
    fclose(pFile);
    return FALSE;
  } /* End of if */

  if (pEvtTab->Num > MAX_NUM_OF_EVENT) {
    DBGPRINT(RT_DEBUG_ERROR,
             ("iapp> eventlog Num exceeds limitation(pEventTab->Num=%d)!\n",
              pEvtTab->Num));
    fclose(pFile);
    return FALSE;
  } /* End of if */

  for (IdEvt = 0; IdEvt < pEvtTab->Num; IdEvt++) {
    if (fwrite(&pEvtTab->Log[IdEvt], sizeof(RT_802_11_EVENT_LOG), 1, pFile) !=
        1) {
      DBGPRINT(RT_DEBUG_ERROR, ("iapp> File write error!\n"));
    } /* End of if */

    DBGPRINT(
        RT_DEBUG_TRACE,
        ("iapp> (%d/%d): (Event=0x%x) (D:%02x:%02x:%02x:%02x:%02x:%02x) "
         "(T:%02x:%02x:%02x:%02x:%02x:%02x)\n",
         IdEvt, pEvtTab->Num, pEvtTab->Log[IdEvt].Event,
         pEvtTab->Log[IdEvt].DetectorAddr[0],
         pEvtTab->Log[IdEvt].DetectorAddr[1],
         pEvtTab->Log[IdEvt].DetectorAddr[2],
         pEvtTab->Log[IdEvt].DetectorAddr[3],
         pEvtTab->Log[IdEvt].DetectorAddr[4],
         pEvtTab->Log[IdEvt].DetectorAddr[5],
         pEvtTab->Log[IdEvt].TriggerAddr[0], pEvtTab->Log[IdEvt].TriggerAddr[1],
         pEvtTab->Log[IdEvt].TriggerAddr[2], pEvtTab->Log[IdEvt].TriggerAddr[3],
         pEvtTab->Log[IdEvt].TriggerAddr[4],
         pEvtTab->Log[IdEvt].TriggerAddr[5]));
  } /* End of for */

  fclose(pFile);
  return TRUE;
} /* End of IAPP_EventLogHandle */

/*
========================================================================
Routine Description:
        Query event log content.

Arguments:
        *pCtrlBK	- IAPP control block

Return Value:
        None

Note:
========================================================================
*/
static VOID IAPP_EventLog_Query(IAPP_IN RTMP_IAPP *pCtrlBK) {
  CHAR *pEvtBuf;
  POID_REQ pOidReq;

  IAPP_MEM_ALLOC(pEvtBuf, sizeof(RT_802_11_EVENT_TABLE));
  if (pEvtBuf == NULL) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Out of memory!\n"));
    return;
  } /* End of if */

  pOidReq = (POID_REQ)pEvtBuf;
  pOidReq->OID = RT_QUERY_EVENT_TABLE;
  pOidReq->Len = sizeof(RT_802_11_EVENT_TABLE);

  IAPP_MsgProcess(pCtrlBK, IAPP_QUERY_OID_REQ, pEvtBuf,
                  sizeof(RT_802_11_EVENT_TABLE));
  IAPP_MEM_FREE(pEvtBuf);
} /* End of IAPP_EventLog_Query */

/*
========================================================================
Routine Description:
        Send out private IAPP packet with security management information and
        log current security management information to file.

Arguments:
        *pCtrlBK	- IAPP control blcok
        *pMsg		- security management information

Return Value:
        None

Note:
========================================================================
*/
static VOID IAPP_SM_InfoHandle(IAPP_IN RTMP_IAPP *pCtrlBK, IAPP_IN CHAR *pMsg) {
  PRT_802_11_EVENT_TABLE pEvtTab;
  POID_REQ OID_req_p;
  INT32 PktLen;

  OID_req_p = (POID_REQ)pMsg;

  DBGPRINT(RT_DEBUG_TRACE, ("iapp> (Receive IAPP_QUERY_OID_REQ)\n"));

  /* IAPP_IOCTL_TO_WLAN(pCtrlBK, RT_IOCTL_IAPP,
                  OID_req_p->Buf, &OID_req_p->Len, 0, OID_req_p->OID); */

  pEvtTab = (PRT_802_11_EVENT_TABLE)OID_req_p->Buf;

  if (OID_req_p->OID == RT_QUERY_EVENT_TABLE) {
    if ((OID_req_p->Len == sizeof(RT_802_11_EVENT_TABLE)) &&
        (pEvtTab->Num != 0)) {
      RT_IAPP_SECURITY_MONITOR SM;

      /* just send valid Data, not whole structure */
      PktLen = sizeof(UINT32) + (pEvtTab->Num * sizeof(RT_802_11_EVENT_LOG));

      SM.IappHeader.Version = 0;
      SM.IappHeader.Command = IAPP_CMD_SECURITY_MONITOR;

      /* will be update before send out */
      SM.IappHeader.Identifier = 0;
      SM.IappHeader.Length = SWAP_16(sizeof(RT_IAPP_HEADER) + PktLen);
      IAPP_MEM_MOVE(&SM.EvtTab, OID_req_p->Buf, PktLen);

      /* send our events to all RALINK IAPP daemons in other AP */
      IAPP_UDP_PACKET_SEND(pCtrlBK, &SM,
                           (sizeof(RT_IAPP_HEADER) + PktLen, pRspBuf));

      /* log our events */
      IAPP_EventLogHandle((PRT_802_11_EVENT_TABLE)OID_req_p->Buf);
    } else {
      DBGPRINT(RT_DEBUG_INFO,
               ("iapp> RT_QUERY_EVENT_TABLE invalid or zero in length!\n"));
    } /* End of if */
  }   /* End of if */
} /* End of IAPP_SM_InfoHandle */
#endif // IAPP_EVENT_LOG //

#if 0  /* no use */
/*
========================================================================
Routine Description:
	Find the IP address by the AP MAC address.

Arguments:
	*pCtrlBK	- IAPP control block
	*pMac		- the AP MAC address pointer
	*pApInfo	- the AP address information pointer

Return Value:
	TRUE		- open IP/MAC mapping file successfully
	FAIL		- open fail

Note:
========================================================================
*/
static BOOLEAN IAPP_IP2MACTransfer(
	IAPP_IN		RTMP_IAPP			*pCtrlBK,
	IAPP_IN		UCHAR				*pMac,
	IAPP_OUT	struct in_addr		*pApInfo)
{
	FILE *Fd;
	CHAR Buffer[200], Mask[32], DevName[32], Hwa[50], IP[50];
	INT32 Type, Flags, Num;
	UCHAR AddrMac[ETH_ALEN];


	/* init */
	IAPP_MEM_MOVE(AddrMac, pMac, ETH_ALEN);

	/*
		ping to broadcast address to get all neighboring AP MAC address/IP
		address (bad method ??? use IARP or Radius Server or manual).
	*/
	sprintf(Buffer, "%s %d.%d.%d.%d %s %s %s %s", "ping",
			IAPP_SHOW_IP_HTONL(pCtrlBK->AddrOwn.s_addr),
			"-b", "-q", "-w", "2");
	system(Buffer);
	sleep(3);

	/* open proc file: arp table */
	if ((Fd = fopen(PROC_NET_ARP_PATH, "r")) == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("iapp> Open proc file failed!\n"));
		return FALSE;
	} /* End of if */

	/* bypass header -- read until newline */
	if (fgets(Buffer, sizeof(Buffer), Fd) != (CHAR *) NULL)
	{
		/* read the ARP cache entries */
		for(; fgets(Buffer, sizeof(Buffer), Fd);)
		{
			Num = sscanf(Buffer, "%s 0x%x 0x%x %s %s %s\n",
						IP, &Type, &Flags, Hwa, Mask, DevName);
			if (Num < 4)
				break;
			/* End of if */

			/* if device differs, just skip it */
			if (strcmp(DevName, pCtrlBK->IfNameEth) != 0)
				continue; /* not same network interface */
			/* End of if */

			/* compare to find out the match one */
			{
				UINT32  Mac0, Mac1, Mac2, Mac3, Mac4, Mac5; /* MAC[0] ~ MAC[5] */
				INT32   Ip0, Ip1, Ip2, Ip3; /* IP[0] ~ IP[3] */

				sscanf(Hwa, "%x:%x:%x:%x:%x:%x\n",
						&Mac0, &Mac1, &Mac2, &Mac3, &Mac4, &Mac5);

				if ((AddrMac[0] == (UCHAR)Mac0) &&
					(AddrMac[1] == (UCHAR)Mac1) &&
					(AddrMac[2] == (UCHAR)Mac2) &&
					(AddrMac[3] == (UCHAR)Mac3) &&
					(AddrMac[4] == (UCHAR)Mac4) &&
					(AddrMac[5] == (UCHAR)Mac5))
				{
					sscanf(IP, "%d.%d.%d.%d\n", &Ip0, &Ip1, &Ip2, &Ip3);

					pApInfo->s_addr = Ip0 +
									(Ip1 << 8) +
									(Ip2 << 16) +
									(Ip3 << 24);

					DBGPRINT(RT_DEBUG_INFO, ("iapp> Found the ARP entry.\n"));
					break;
				} /* End of if */
			}

			DBGPRINT(RT_DEBUG_INFO,
					("iapp> IP = %s  Type = %d  Flags = %d  Hwa = %s "
					"Mask = %s  dev = %s\n",
					IP, Type, Flags, Hwa, Mask, DevName));
		} /* End of for(; fgets(Buffer, sizeof(Buffer), Fd);) */
	} /* End of if (fgets(Buffer, sizeof(Buffer), Fd) != (CHAR *) NULL) */

	/* close proc file */
	fclose(Fd);
	return TRUE;
} /* End of IAPP_IP2MACTransfer */
#endif // #if 0 */

/* ---------------------------- PRIVATE Function ---------------------------- */
/*
========================================================================
Routine Description:
        Dump a buffer content.

Arguments:
        pPromptStr	- prompt string
        *pSrcBufVA	- buffer content
        SrcBufLen	- display length

Return Value:
        None

Note:
        debug use
========================================================================
*/
VOID IAPP_HexDump(IAPP_IN CHAR *pPromptStr, IAPP_IN CHAR *pSrcBufVA,
                  IAPP_IN UINT32 SrcBufLen) {
  CHAR *pChar;
  UINT32 IdDump;

  pChar = pSrcBufVA;
  printf("%s: %p, len = %d\n", pPromptStr, pSrcBufVA, SrcBufLen);

  for (IdDump = 0; IdDump < SrcBufLen; IdDump++) {
    if ((IdDump & 0x0F) == 0)
      printf("0x%04x : ", IdDump);
    /* End of if */

    printf("%02x ", ((UCHAR)pChar[IdDump]));

    if ((IdDump & 0x0F) == 15)
      printf("\n");
    /* End of if */
  } /* End of for */

  printf("\n");
} /* End of IAPP_HexDump */

/*
========================================================================
Routine Description:
        Parse SysCmd arguments.

Arguments:
        *pCtrlBK	- IAPP Control Block
        Argc		- argument number
        *pArgv[]	- arguments

Return Value:
        None

Note:
========================================================================
*/
static BOOLEAN IAPP_ArgumentParse(IAPP_IN RTMP_IAPP *pCtrlBK,
                                  IAPP_IN INT32 Argc, IAPP_IN CHAR *pArgv[]) {
#define IAPP_AGP_CMD_PARSE_NEXT_ONE                                            \
  {                                                                            \
    Argc--;                                                                    \
    pArgv++;                                                                   \
  }

  INT i = 0;

  /* init */
  strcpy(pCtrlBK->IfNameEth, FT_KDP_DEFAULT_IF_ETH);
  strcpy(pCtrlBK->IfNameWlan, FT_KDP_DEFAULT_IF_WLAN);
  strcpy(pCtrlBK->IfNameWlanIoctl[0], FT_KDP_DEFAULT_IF_WLAN_IOCTL);

#ifdef FT_KDP_FUNC_PKT_ENCRYPT
#ifdef FT_KDP_KEY_FROM_DAEMON
  strcpy(pCtrlBK->CommonKey, FT_KDP_DEFAULT_PTK);
#endif // FT_KDP_KEY_FROM_DAEMON //
#endif // FT_KDP_FUNC_PKT_ENCRYPT //

  /* sanity check */
  if (Argc == 0)
    goto label_exit;
  /* End of if */

  /* parse command */
  IAPP_AGP_CMD_PARSE_NEXT_ONE;

  while (Argc > 0) {
    if (strncmp(pArgv[0], "-h", 2) == 0) {
      IAPP_AGP_CMD_PARSE_NEXT_ONE;
      IAPP_Usage();
      return FALSE;
    } /* End of if */

    if (strncmp(pArgv[0], "-e", 2) == 0) {
      /* ethernet interface */
      IAPP_AGP_CMD_PARSE_NEXT_ONE;

      if (Argc > 0) {
        strcpy(pCtrlBK->IfNameEth, pArgv[0]);
        IAPP_AGP_CMD_PARSE_NEXT_ONE;
      } /* End of if */
    } else if (strncmp(pArgv[0], "-wi", 3) == 0) {
      /* wireless ioctl interface */
      IAPP_AGP_CMD_PARSE_NEXT_ONE;

      if (Argc > 0) {
        strcpy(pCtrlBK->IfNameWlanIoctl[pCtrlBK->IfNameWlanCount++], pArgv[0]);
        IAPP_AGP_CMD_PARSE_NEXT_ONE;
      } /* End of if */
    } else if (strncmp(pArgv[0], "-w", 2) == 0) {
      /* wireless interface */
      IAPP_AGP_CMD_PARSE_NEXT_ONE;

      if (Argc > 0) {
        strcpy(pCtrlBK->IfNameWlan, pArgv[0]);
        IAPP_AGP_CMD_PARSE_NEXT_ONE;
      } /* End of if */
    }
#ifdef FT_KDP_FUNC_PKT_ENCRYPT
#ifdef FT_KDP_KEY_FROM_DAEMON
    else if (strncmp(pArgv[0], "-k", 2) == 0) {
      /* encrypt/decrypt key */
      IAPP_AGP_CMD_PARSE_NEXT_ONE;

      if (Argc > 0) {
        if (strlen(pArgv[0]) > IAPP_ENCRYPT_KEY_MAX_SIZE) {
          pArgv[0][IAPP_ENCRYPT_KEY_MAX_SIZE] = 0x00;

          DBGPRINT(RT_DEBUG_TRACE,
                   ("iapp> key length can not be larger than %d!",
                    IAPP_ENCRYPT_KEY_MAX_SIZE));
        } /* End of if */

        strcpy(pCtrlBK->CommonKey, pArgv[0]);
        IAPP_AGP_CMD_PARSE_NEXT_ONE;
      } /* End of if */
    }
#endif // FT_KDP_KEY_FROM_DAEMON //
#endif // FT_KDP_FUNC_PKT_ENCRYPT //
    else if (strncmp(pArgv[0], "-d", 2) == 0) {
      IAPP_AGP_CMD_PARSE_NEXT_ONE;
      RTDebugLevel = atoi(pArgv[0]);
      IAPP_AGP_CMD_PARSE_NEXT_ONE;
    } else {
      IAPP_AGP_CMD_PARSE_NEXT_ONE;
    } /* End of if */
  }   /* End of while */

label_exit:
  if (pCtrlBK->IfNameWlanCount == 0)
    pCtrlBK->IfNameWlanCount = 1;
  DBGPRINT(RT_DEBUG_TRACE,
           ("iapp> -e=%s, -w=%s", pCtrlBK->IfNameEth, pCtrlBK->IfNameWlan));

  for (i = 0; i < pCtrlBK->IfNameWlanCount; i++) {
    DBGPRINT(RT_DEBUG_TRACE, (", -wi=%s", pCtrlBK->IfNameWlanIoctl[i]));
  }
  DBGPRINT(RT_DEBUG_TRACE,
           (", IfNameWlanCount = %d\n", pCtrlBK->IfNameWlanCount));
  return TRUE;
} /* End of IAPP_ArgumentParse */

/*
========================================================================
Routine Description:
        Get interface information, such as IP, AddrNetmask, broadcast addr, etc.

Arguments:
        *pCtrlBK	- IAPP control block

Return Value:
        TRUE		- get successfully
        FAIL		- get fail

Note:
        Only for Ethernet interface of DS (distribution system).
========================================================================
*/
static BOOLEAN IAPP_DSIfInfoGet(IAPP_IN RTMP_IAPP *pCtrlBK) {
  INT32 SockIf;
  struct ifreq ReqIf;

  /* init */
  IAPP_MEM_MOVE(ReqIf.ifr_name, pCtrlBK->IfNameEth, IFNAMSIZ);

  /* open a UDP socket */
  if ((SockIf = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Open socket failed!\n"));
    return FALSE;
  } /* End of if */

  /* get own addr */
  if (ioctl(SockIf, SIOCGIFADDR, (int)&ReqIf) < 0) {
    DBGPRINT(RT_DEBUG_ERROR,
             ("iapp> Get own address of %s failed!\n", ReqIf.ifr_name));
    goto label_fail;
  } /* End of if */

  IAPP_MEM_MOVE(&pCtrlBK->AddrOwn,
                &((struct sockaddr_in *)&ReqIf.ifr_addr)->sin_addr,
                sizeof(pCtrlBK->AddrOwn));

  DBGPRINT(RT_DEBUG_TRACE, ("iapp> own address (%d.%d.%d.%d)\n",
                            IAPP_SHOW_IP_HTONL(pCtrlBK->AddrOwn.s_addr)));

  /* get broadcast address */
  if (ioctl(SockIf, SIOCGIFBRDADDR, (int)&ReqIf) < 0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Get broadcast address failed!\n"));
    goto label_fail;
  } /* End of if */

  IAPP_MEM_MOVE(&pCtrlBK->AddrBroadcast,
                &((struct sockaddr_in *)&ReqIf.ifr_addr)->sin_addr,
                sizeof(pCtrlBK->AddrBroadcast));

  /* can not use 255.255.255.255 or we can not send any packet */
  //	NdisFillMemory(&pCtrlBK->AddrBroadcast, sizeof(pCtrlBK->AddrBroadcast),
  // 0xFF);

  DBGPRINT(RT_DEBUG_TRACE, ("iapp> broadcast address (%d.%d.%d.%d)\n",
                            IAPP_SHOW_IP_HTONL(pCtrlBK->AddrBroadcast.s_addr)));

  /* get network Mask */
  if (ioctl(SockIf, SIOCGIFNETMASK, (int)&ReqIf) < 0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Get network Mask failed!\n"));
    goto label_fail;
  } /* End of if */

  IAPP_MEM_MOVE(&pCtrlBK->AddrNetmask,
                &((struct sockaddr_in *)&ReqIf.ifr_addr)->sin_addr,
                sizeof(pCtrlBK->AddrNetmask));

  DBGPRINT(RT_DEBUG_TRACE, ("iapp> network Mask address (%d.%d.%d.%d)\n",
                            IAPP_SHOW_IP_HTONL(pCtrlBK->AddrNetmask.s_addr)));

  close(SockIf);
  return TRUE;

label_fail:
  close(SockIf);
  return FALSE;
} /* End of IAPP_DSIfInfoGet */

/*
========================================================================
Routine Description:
        Issue a SysCmd to RALINK AP driver.

Arguments:
        *pCtrlBK		- the IAPP control block
        Param			- IOCTL command
        *pData			- command
        *pDataLen		- command length
        ApIdx			- BSS index
        Flags			- IOCTL sub-command

Return Value:
        Ret

Note:
========================================================================
*/
BOOLEAN IAPP_IoctlToWLAN(IAPP_IN RTMP_IAPP *pCtrlBK, IAPP_IN INT32 Param,
                         IAPP_IN CHAR *pData, IAPP_IN INT32 *pDataLen,
                         IAPP_IN UCHAR ApIdx, IAPP_IN INT32 Flags) {
  CHAR IfName[12]; /* in VxWorks, no iwreq.ifr_name */
  struct iwreq Wrq;

#ifdef IAPP_OS_LINUX
  if (strlen(pCtrlBK->IfNameWlanIoctl[ApIdx]) >= sizeof(IfName))
    strcpy(IfName, FT_KDP_DEFAULT_IF_WLAN_IOCTL);
  else
    strcpy(IfName, pCtrlBK->IfNameWlanIoctl[ApIdx]);
  /* End of if */

  /*	sprintf(IfName, "ra%d", ApIdx);
          IfName[3] = '\0'; */
  strcpy(Wrq.ifr_name, IfName);
#endif

  Wrq.u.data.flags = Flags;
  Wrq.u.data.length = *pDataLen;
  Wrq.u.data.pointer = (caddr_t)pData;

  DBGPRINT(RT_DEBUG_ERROR, ("iapp>[%s]IOCTL Flags = 0x%x!\n", IfName, Flags));

#ifdef IAPP_OS_LINUX
  if (ioctl(pCtrlBK->SocketIoctl, Param, (int)&Wrq) < 0) {
    DBGPRINT(RT_DEBUG_ERROR,
             ("iapp> IOCTL 0x%x to wlan %s failed!\n", Flags, IfName));
    return FALSE;
  }    /* End of if */
#endif // IAPP_OS_LINUX //

#ifdef IAPP_OS_VXWORKS
  if (muxIoctl(pCtrlBK->pDrvCookieTo, Param, (caddr_t)&Wrq) == ERROR) {
    DBGPRINT(RT_DEBUG_ERROR,
             ("iapp> IOCTL 0x%x to wlan %s failed!\n", Param, IfName));
    return FALSE;
  }    /* End of if */
#endif // IAPP_OS_VXWORKS //

  *pDataLen = Wrq.u.data.length;
  return TRUE;
} /* End of IAPP_IoctlToWLAN */

/*
========================================================================
Routine Description:
        Initialize IPC message control.

Arguments:
        *pCtrlBK	- IAPP control blcok

Return Value:
        TRUE		- always successfully

Note:
        IPC related SysCmd-
                ipcs, ipcrm, msgget, msgsnd, msgrcv, msgctl
========================================================================
*/
static INT32 IAPP_IPC_MSG_Init(IAPP_IN RTMP_IAPP *pCtrlBK) {
#if 0
	INT32  Key = 0x55116604;


	/*
		Translate an unique Key from our dat file, the Key is fixed for
		a fixed file content.
	*/
	if ((Key = ftok(MSG_FILE, 0xf)) == -1)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("iapp> ftok Error!\n"));
		exit(-1);
	} /* End of if */

	if ((pCtrlBK->MsgId = msgget(Key, 0)) == -1)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("iapp> msgget Error:%s\n", strerror(errno)));
		exit(-1);
	} /* End of if */

	pCtrlBK->FlgIsMsgReady = FALSE;
#endif

  pCtrlBK->PacketIdentifier = 0;
  return TRUE;
} /* End of IAPP_IPC_MSG_Init */

/*
========================================================================
Routine Description:
        Send Layer 2 Update Frame to update forwarding table in Layer 2 devices.

Arguments:
        *pCtrlBK	- IAPP control blcok
        *pMac		- the STATION MAC address pointer

Return Value:
        TRUE		- send successfully
        FAIL		- send fail

Note:
========================================================================
*/
static BOOLEAN IAPP_L2UpdateFrameSend(IAPP_IN RTMP_IAPP *pCtrlBK,
                                      IAPP_IN UCHAR *pMac) {
  RT_IAPP_L2_UPDATE_FRAME FrameBody, *pFrameL2;
  INT32 Status;

  /* init the update frame body */
  pFrameL2 = &FrameBody;

  IAPP_MEM_ZERO(pFrameL2, sizeof(FrameBody));
  IAPP_MEM_FILL(pFrameL2->ETH.DA, 0xFF, ETH_ALEN);
  IAPP_MEM_MOVE(pFrameL2->ETH.SA, pMac, ETH_ALEN);

  pFrameL2->ETH.Len = htons(8);
  pFrameL2->DSAP = 0;
  pFrameL2->SSAP = 0x01;
  pFrameL2->Control = 0xAF;

  pFrameL2->XIDInfo[0] = 0x81;
  pFrameL2->XIDInfo[1] = 1;
  pFrameL2->XIDInfo[2] = 1 << 1;

#ifdef IAPP_OS_LINUX
  /* send the update frame */
  Status = send(pCtrlBK->SocketRawBr, pFrameL2, sizeof(FrameBody), 0);
  if (Status < 0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Send L2 packet failed %d!\n", Status));
    return FALSE;
  } /* End of if */
#endif

#ifdef IAPP_OS_VXWORKS
  {
    M_BLK_ID pUpdatePkt;
    UINT32 IdIfNum;

    /* loop for eth0, eth1, eth2...... */
    for (IdIfNum = 0; IdIfNum < FT_KDP_BR_ETH_IF_NUM; IdIfNum++) {
      if ((pUpdatePkt = netTupleGet(_pNetDpool, sizeof(FrameBody), M_DONTWAIT,
                                    MT_DATA, TRUE)) == NULL) {
        DBGPRINT(RT_DEBUG_ERROR, ("iapp> Get packet buffer fail!\n"));
        return FALSE;
      } /* End of if */

      pUpdatePkt->mBlkHdr.mFlags |= M_PKTHDR;
      pUpdatePkt->m_len = sizeof(FrameBody);

      IAPP_MEM_MOVE(pUpdatePkt->m_data, pFrameL2, sizeof(FrameBody));

      if (muxSend(pCtrlBK->pBcCookie[IdIfNum], pUpdatePkt) == ERROR) {
        DBGPRINT(RT_DEBUG_ERROR, ("iapp> Send L2 packet failed!\n"));
        netMblkClChainFree(pUpdatePkt);
        return FALSE;
      } /* End of if */
    }   /* End of for */

    DBGPRINT(RT_DEBUG_TRACE, ("iapp> Send L2 packet OK!\n"));
  }
#endif

  return TRUE;
} /* End of IAPP_L2UpdateFrameSend */

/*
========================================================================
Routine Description:
        Process received message, such as rcv ADD-notify and inform AP to del
STA.

Arguments:
        *pCtrlBK	- the IAPP control block pointer
        MsgSubType	- message Type
        *pMsg		- message
        Len			- message length

Return Value:
        TRUE		- always successfully

Note:
========================================================================
*/
static BOOLEAN IAPP_MsgProcess(IAPP_IN RTMP_IAPP *pCtrlBK,
                               IAPP_IN INT32 MsgSubType, IAPP_IN UCHAR *pMsg,
                               IAPP_IN INT32 Len, IAPP_IN INT32 if_idx) {
  switch (MsgSubType) {
  case IAPP_OPEN_SERVICE_REQ:
    DBGPRINT(RT_DEBUG_TRACE, ("iapp> (FlgIsMsgReady is TRUE)\n"));
    break;

  case IAPP_CLOSE_SERVICE_REQ:
    DBGPRINT(RT_DEBUG_TRACE, ("iapp> (FlgIsMsgReady is FALSE)\n"));
    break;

  case IAPP_SET_OID_REQ: {
    POID_REQ OID_req_p;

    OID_req_p = (POID_REQ)pMsg;

    DBGPRINT(RT_DEBUG_TRACE, ("iapp> Command to WLAN (OID=%x, LEN=%d)\n",
                              OID_req_p->OID, OID_req_p->Len));

    IAPP_IOCTL_TO_WLAN(pCtrlBK, RT_IOCTL_IAPP, OID_req_p->Buf, &OID_req_p->Len,
                       if_idx, OID_req_p->OID);
  } /* case IAPP_SET_OID_REQ */
  break;

  case IAPP_QUERY_OID_REQ: /* old SysCmd from 8021X deamon */
                           //			IAPP_SM_InfoHandle(pMsg);
    break;

  default:
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> (unknown subtype)\n"));
    break;
  } /* End of switch */

  return TRUE;
} /* End of IAPP_MsgProcess */

/*
========================================================================
Routine Description:
        Open a file with filename = our PID.

Arguments:
        PID - our background process ID

Return Value:
        None

Note:
        Why to backup PID?

        Because always we will restart new IAPP daemon but forget to destroy
        the old IAPP daemon, many IAPP daemons will exist in kernel.

        So we will delete old IAPP daemon whenever you start a new IAPP daemon.
========================================================================
*/
static VOID IAPP_PID_Backup(IAPP_IN pid_t PID) {
#ifdef IAPP_OS_LINUX
  FILE *pFile;
  CHAR Buffer[30];

  /* prepare PID file content */
  IAPP_MEM_ZERO(Buffer, sizeof(Buffer));
  sprintf(Buffer, "PID = %d\n", PID);

  /* re-open and truncate file to zero length by using "wb" */
  /* write new PID in the file */
  pFile = fopen(IAPP_PID_BACKUP_FILE, "wb");
  if (pFile) {
    fwrite(Buffer, strlen(Buffer), 1, pFile);
    fclose(pFile);
  } else {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> File open error.\n"));
  }
#endif // IAPP_OS_LINUX //
} /* End of IAPP_PID_Backup */

/*
========================================================================
Routine Description:
        Kill running IAPP daemon if exists.

Arguments:
        None

Return Value:
        None

Note:
========================================================================
*/
static VOID IAPP_PID_Kill(VOID) {
#ifdef IAPP_OS_LINUX
  FILE *pFile;
  pid_t PID;
  CHAR SysCmd[30];

  /* get last PID */
  pFile = fopen(IAPP_PID_BACKUP_FILE, "rb");
  if (pFile == NULL)
    return;
  /* End of if */
  fscanf(pFile, "PID = %d\n", &PID);
  DBGPRINT(RT_DEBUG_OFF, ("iapp> Found PID=%d\n", PID));
  fclose(pFile);

  /* kill old IAPP daemon */
  sprintf(SysCmd, "kill %d\n", PID);
  system(SysCmd);

  /* sleep for a where to kill old daemon */
  sleep(2);
#endif // IAPP_OS_LINUX //
} /* End of IAPP_PID_Kill */

/*
========================================================================
Routine Description:
        Process received signal, such as STA ASSOCIATION and send a ADD-notify.

Arguments:
        *pCtrlBK	- IAPP control blcok
        *pSig		- signal
        Len			- message length
        *pCmdBuf	- used to issue command to WLAN driver
        *pRspBuf	- used to issue response to the peer

Return Value:
        TRUE		- always successfully

Note:
========================================================================
*/
static BOOLEAN IAPP_SIG_Process(IAPP_IN RTMP_IAPP *pCtrlBK,
                                IAPP_IN UCHAR *WifiMAC,
                                IAPP_IN RT_SIGNAL_STRUC *pSig,
                                IAPP_IN INT32 Len, IAPP_IN UCHAR *pCmdBuf,
                                IAPP_IN UCHAR *pRspBuf) {
  FT_KDP_EVT_HEADER *pEvtHeader;
  INT32 if_idx = -1;

  if_idx = mt_iapp_find_ifidx_by_mac(pCtrlBK, WifiMAC);
  if (if_idx == -1) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> %s - cannot find this wifi interface "
                              "(%02x:%02x:%02x:%02x:%02x:%02x)\n",
                              __FUNCTION__, WifiMAC[0], WifiMAC[1], WifiMAC[2],
                              WifiMAC[3], WifiMAC[4], WifiMAC[5]));
    return FALSE;
  }

  pEvtHeader = (FT_KDP_EVT_HEADER *)(pSig->Content);

  DBGPRINT(RT_DEBUG_TRACE, ("iapp> Sig = 0x%02x, pEvtHeader->EventLen = %d, "
                            "Peer IP = %d.%d.%d.%d\n",
                            pSig->Sig, pEvtHeader->EventLen,
                            IAPP_SHOW_IP_HTONL(pEvtHeader->PeerIpAddr)));

  switch (pSig->Sig) {
  case FT_KDP_SIG_NOTHING:
    DBGPRINT(RT_DEBUG_TRACE, ("iapp> NO event to handle.\n"));
    break;
  case FT_KDP_SIG_IAPP_ASSOCIATION: {
#if 1
    DBGPRINT(RT_DEBUG_TRACE, ("iapp> FT_KDP_SIG_IAPP_ASSOCIATION.\n"));
#else

    /*
            This service primitive is used when a STA associates with the
            AP using an 802.11 association request frame.

            One purpose of this primitive is to cause the forwarding
            tables of layer 2 internetworking devices, e.g. bridges
            and switches, to be updated.

            The second purpose of this primitive is to notify other APs
            within the multicast domain, i.e., that portion of a network
            in which a layer two frame addressed to a multicast address
            can be received, of the STA’s new association, to allow those
            APs to clean up context information left behind by STAs that
            do not properly reassociate when moving from one AP to another,
            but rather only use the 802.11 Association Request.
    */
    RT_IAPP_ADD_NOTIFY IappAddNotify, *pFrameNotify;
    UINT32 DataLen;

    /* pSig->MacAddr[] is the associated STATION MAC address */

    /* init */
    IAPP_MEM_ZERO(&IappAddNotify, sizeof(IappAddNotify));

    /* make up the frame content */
    pFrameNotify = &IappAddNotify;
    pFrameNotify->IappHeader.Version = 0;
    pFrameNotify->IappHeader.Command = IAPP_CMD_ADD_NOTIFY;
    pFrameNotify->IappHeader.Identifier = 0;

    DataLen = sizeof(RT_IAPP_ADD_NOTIFY);

    pFrameNotify->IappHeader.Length = SWAP_16(DataLen);
    pFrameNotify->Sequence = SWAP_16(pSig->Sequence);

    pFrameNotify->AddressLen = ETH_ALEN;
    IAPP_MEM_MOVE(pFrameNotify->MacAddr, pSig->MacAddr, ETH_ALEN);

    mt_iapp_ft_client_insert(&pCtrlBK->SelfFtStaTable, pSig->MacAddr, WifiMAC,
                             if_idx);
    /*
Send the ADD-notify with multicast address &
send the L2 update frame with broadcast address.
*/
    /*
            An ESS is a set of Basic Service Sets (BSSs) that form a
            single LAN, allowing an STA to move transparently from one
            BSS to another throughout the ESS.

            Because this packet is addressed to the IAPP multicast
            address, this packet may not reach all APs in an ESS.
            In particular, if the ESS spans multiple subnets, neither
            the ADD-notify packet nor the Layer 2 Update frame is likely
            to reach the APs on subnets other than the one on which
            the transmissions originate.

            Note that purpose of the IAPP ADD-notify packet is to remove
            stale associations, not to modify the learning table.
            The learning table update is done by the Layer 2 Update frame.

            This should allow for more efficient management of AP resources.

            There is no security provided for the Layer 2 Update frame.

            The Layer 2 Update frame does not open new potentials for
            attacks against the WLAN or the STAs. However, the ADD-notify
            is a UDP IP frame that COULD be sent from anywhere in the DS
            and attack the AP’s state for the STA.
    */
    IAPP_UDP_PACKET_SEND(pCtrlBK, pFrameNotify, DataLen, pRspBuf);
    IAPP_L2UpdateFrameSend(pCtrlBK, pSig->MacAddr);

    DBGPRINT(RT_DEBUG_TRACE,
             ("iapp> (IAPP_SIG_ASSOCIATION), Rcv assoc signal, and send out "
              "IAPP add-notify\n"));
#endif
  } /* case IAPP_SIG_ASSOCIATION */
  break;

  case IAPP_SIG_REASSOCIATION:
    /* test only, not support currently */
#if 0
		{
			struct in_addr IP;

			/* UCHAR   mac[ETH_ALEN] = {0x00,0x0c,0x43,0x26,0x65,0x09};

			if (IAPP_IP2MACTransfer(&IP, mac) >= 0)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("iapp> (IAPP_SIG_REASSOCIATION), "
						"IP: %d.%d.%d.%d\n", IP.s_addr & 0xFF,
						(IP.s_addr >> 8) & 0xFF,
						(IP.s_addr >> 16) & 0xFF,
						(IP.s_addr >> 24) & 0xFF));
			}*/

			IP.s_addr = 0;

			if (IAPP_IP2MACTransfer(pSig->CurrAPAddr, &IP) == TRUE)
			{
				DBGPRINT(RT_DEBUG_TRACE,
						("iapp> (IAPP_SIG_REASSOCIATION), IP=%d.%d.%d.%d\n",
						IAPP_SHOW_IP_HTONL(IP.s_addr)));
			} /* End of if */
		} /* case IAPP_SIG_REASSOCIATION */
#endif // #if 0 //
    break;

  case FT_KDP_SIG_FT_ASSOCIATION: {
    /* a station has already associated with us */
    /* so prepare and send out ADD_notify packet, L2 update frame */
    RT_IAPP_ADD_NOTIFY IappAddNotify, *pFrameNotify;
    UINT32 DataLen;

    /* init */
    IAPP_MEM_ZERO(&IappAddNotify, sizeof(IappAddNotify));

    /* make up the frame content */
    pFrameNotify = &IappAddNotify;
    pFrameNotify->IappHeader.Version = 0;
    pFrameNotify->IappHeader.Command = IAPP_CMD_ADD_NOTIFY;
    pFrameNotify->IappHeader.Identifier = 0;

    DataLen = sizeof(RT_IAPP_ADD_NOTIFY);

    pFrameNotify->IappHeader.Length = SWAP_16(DataLen);
    pFrameNotify->Sequence = SWAP_16(pSig->Sequence);

    pFrameNotify->AddressLen = ETH_ALEN;
    IAPP_MEM_MOVE(pFrameNotify->MacAddr, pSig->MacAddr, ETH_ALEN);

    mt_iapp_ft_client_insert(&pCtrlBK->SelfFtStaTable, pSig->MacAddr, WifiMAC,
                             if_idx);

    /* mark the notify is for 11r station */
    pFrameNotify->Rsvd |= FT_KDP_ADD_NOTIFY_RSVD_11R_SUPPORT;

    /*
            Send the ADD-notify with multicast address &
            send the L2 update frame with broadcast address.
    */
    DBGPRINT(
        RT_DEBUG_TRACE,
        ("iapp> (FT_KDP_SIG_FT_ASSOCIATION) Rcv a assoc signal and send out "
         "IAPP add-notify!\n"));

    IAPP_UDP_PACKET_SEND(pCtrlBK, pFrameNotify, DataLen, pRspBuf, if_idx);
    IAPP_L2UpdateFrameSend(pCtrlBK, pSig->MacAddr);
  } break;

  case FT_KDP_SIG_FT_REASSOCIATION: {
    /*
            This primitive should be issued by the APME when it receives
            an MLME-REASSOCIATE.indication from the MLME indicating that
            an STA has reassociated with the AP.

            In IAPP, If the APME is utilizing caching, then the APME should
            first lookup the STA’s context in the IAPP cache using the
            STA’s MAC Address. If found (a cache hit), then an
            IAPP-MOVE.request does not have to be issued until after an
            802.11 Reassociation Response frame. If the STA context is
            not found in the cache (a cache miss), then the APME should
            issue an IAPP-MOVE.request as usual.

            In IEEE802.11r, we just send MOVE-notify to inform the peer.
    */
    RT_IAPP_MOVE_NOTIFY IappMoveNotify, *pFrameNotify;
    UINT32 DataLen;

    /* init */
    IAPP_MEM_ZERO(&IappMoveNotify, sizeof(IappMoveNotify));

    /* make up the frame content */
    pFrameNotify = &IappMoveNotify;
    pFrameNotify->IappHeader.Version = 0;
    pFrameNotify->IappHeader.Command = IAPP_CMD_MOVE_NOTIFY;
    pFrameNotify->IappHeader.Identifier = 0;

    DataLen = sizeof(RT_IAPP_MOVE_NOTIFY);

    pFrameNotify->IappHeader.Length = SWAP_16(DataLen);
    pFrameNotify->Sequence = SWAP_16(pSig->Sequence);

    pFrameNotify->AddressLen = ETH_ALEN;
    IAPP_MEM_MOVE(pFrameNotify->MacAddr, pSig->MacAddr, ETH_ALEN);

    mt_iapp_ft_client_insert(&pCtrlBK->SelfFtStaTable, pSig->MacAddr, WifiMAC,
                             if_idx);

    /*
            Send the MOVE-notify with multicast address &
            send the L2 update frame with broadcast address.
    */
    IAPP_TCP_PACKET_SEND(pCtrlBK, pFrameNotify, sizeof(RT_IAPP_MOVE_NOTIFY),
                         pEvtHeader->PeerIpAddr, TRUE, pRspBuf, if_idx);
    IAPP_L2UpdateFrameSend(pCtrlBK, pSig->MacAddr);

    DBGPRINT(RT_DEBUG_TRACE,
             ("iapp> (IAPP_SIG_REASSOCIATION) Rcv a reassoc signal and "
              "send out IAPP move-notify!\n"));
  } break;

  case IAPP_SIG_TERMINATE: {
    /* terminate the daemon */
    IAPP_TerminateHandle(0);
  } break;

  case FT_KDP_SIG_KEY_TIMEOUT: {
    /* inform other APs the key timeouts */
  } break;

  case FT_KDP_SIG_KEY_REQ:
  case FT_KDP_SIG_KEY_REQ_AUTO:
    /* request PMK-R1 from the R0KH */
    FT_KDP_SecurityBlockSend(pCtrlBK, pEvtHeader,
                             pSig->Content + FT_KDP_EVT_HEADER_SIZE, pRspBuf,
                             WifiMAC, if_idx);
    break;

  case FT_KDP_SIG_ACTION:
    /* forward FT Action Req/Rsp for the peer */
    FT_RRB_ActionForward(pCtrlBK, pEvtHeader,
                         pSig->Content + FT_KDP_EVT_HEADER_SIZE, pSig->Sequence,
                         pSig->MacAddr, pSig->MacAddrSa, pSig->CurrAPAddr,
                         pRspBuf, if_idx);
    break;

  case FT_KDP_SIG_KEY_RSP_AUTO:
    /* response PMK-R1 to the R1KH */
    FT_KDP_SecurityBlockAck(pCtrlBK, pEvtHeader,
                            pSig->Content + FT_KDP_EVT_HEADER_SIZE, pRspBuf,
                            if_idx);
    break;

  case FT_KDP_SIG_INFO_BROADCAST:
    /* send my AP information to the ESS */
    FT_KDP_InformationReportSend(pCtrlBK, pEvtHeader,
                                 pSig->Content + FT_KDP_EVT_HEADER_SIZE,
                                 pRspBuf, if_idx);
    break;

  case FT_KDP_SIG_AP_INFO_REQ:
    /* request other AP information in the ESS */
    FT_KDP_InformationRequestSend(pCtrlBK, pEvtHeader,
                                  pSig->Content + FT_KDP_EVT_HEADER_SIZE,
                                  pRspBuf, if_idx);
    break;

  case FT_KDP_SIG_AP_INFO_RSP:
    /* response my AP information to a AP */
    FT_KDP_InformationResponseSend(pCtrlBK, pEvtHeader,
                                   pSig->Content + FT_KDP_EVT_HEADER_SIZE,
                                   pRspBuf, if_idx);
    break;

  case FT_KSP_SIG_DEBUG_TRACE:
    /* change debug level */
    RTDebugLevel = *(INT32 *)(pSig->Content + FT_KDP_EVT_HEADER_SIZE);
    DBGPRINT(RT_DEBUG_OFF, ("iapp> Change debug level to %d!\n", RTDebugLevel));
    break;

  default:
    DBGPRINT(RT_DEBUG_ERROR,
             ("iapp> Signal %d is not supported!\n", pSig->Sig));
    return FALSE;
  } /* End of switch(pSig->Sig) */

  return TRUE;
} /* End of IAPP_SIG_Process */

/*
========================================================================
Routine Description:
        Close TCP/UDP socket.

Arguments:
        *pCtrlBK	- IAPP control blcok

Return Value:
        TRUE		- always successfully

Note:
========================================================================
*/
static BOOLEAN IAPP_SocketClose(IAPP_IN RTMP_IAPP *pCtrlBK) {
  /*	struct ip_mreq MReq;*/

  DBGPRINT(RT_DEBUG_TRACE, ("iapp> Close TCP/UDP socket.\n"));

#if 0
	MReq.imr_multiaddr.s_addr = inet_addr(IAPP_MULTICAST_ADDR);
	MReq.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(pCtrlBK->SocketUdpSend,
					IPPROTO_IP,
					IP_DROP_MEMBERSHIP,
					&MReq,
					sizeof(MReq)) < 0)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("iapp> setsockopt-IP_DROP_MEMBERSHIP failed!\n"));
	} /* End of if */
#endif // #if 0 //

  if (pCtrlBK->SocketUdpSend >= 0)
    close(pCtrlBK->SocketUdpSend);
  /* End of if */

  if (pCtrlBK->SocketTcpRcv >= 0)
    close(pCtrlBK->SocketTcpRcv);
  /* End of if */

  if (pCtrlBK->SocketRawBr >= 0)
    close(pCtrlBK->SocketRawBr);
  /* End of if */

  if (pCtrlBK->SocketIoctl >= 0)
    close(pCtrlBK->SocketIoctl);
  /* End of if */

  if (pCtrlBK->SocketRawRRB >= 0)
    close(pCtrlBK->SocketRawRRB);
    /* End of if */

#ifdef FT_KDP_FUNC_SOCK_COMM
  if (pCtrlBK->SocketRawDrv >= 0)
    close(pCtrlBK->SocketRawDrv);
    /* End of if */
#endif // FT_KDP_FUNC_SOCK_COMM //

#ifndef FT_KDP_FUNC_SOCK_COMM
#ifdef IAPP_OS_LINUX
  if (pCtrlBK->PipeRawDrv[FT_KDP_PIPE_ID_READ] >= 0)
    close(pCtrlBK->PipeRawDrv[FT_KDP_PIPE_ID_READ]);
  /* End of if */

  if (pCtrlBK->PipeRawDrv[FT_KDP_PIPE_ID_WRITE] >= 0)
    close(pCtrlBK->PipeRawDrv[FT_KDP_PIPE_ID_WRITE]);
    /* End of if */
#endif // IAPP_OS_LINUX //
#endif // FT_KDP_FUNC_SOCK_COMM //

  pCtrlBK->FlgIsRcvRunning = FALSE;
  return TRUE;
} /* End of IAPP_SocketClose */

/*
========================================================================
Routine Description:
        Open TCP/UDP socket.

Arguments:
        *pCtrlBK	- IAPP control blcok

Return Value:
        TRUE		- open successfully
        FAIL		- open fail

Note:
========================================================================
*/
static BOOLEAN IAPP_SocketOpen(IAPP_IN RTMP_IAPP *pCtrlBK) {
  INT32 FlgIsLoop, FlgIsReUsed, FlgIsBroadcast;
#if 0
	struct hostent		*SerHostName;
#endif
  /*	struct ip_mreq		MReq;*/
  struct sockaddr_in AddrUdp, AddrTcp;
#ifdef IAPP_OS_LINUX
  struct ifreq ReqIf;
  struct sockaddr_ll AddrRaw;
#endif
#if 0
	struct sigaction	HandlerUdp;
#endif
#ifdef IAPP_OS_VXWORKS
  struct in_addr AddrIf;
#endif

  /* init */
  pCtrlBK->SocketUdpSend = -1;
  pCtrlBK->SocketTcpRcv = -1;
  pCtrlBK->SocketRawBr = -1;
  pCtrlBK->SocketIoctl = -1;
  pCtrlBK->SocketRawRRB = -1;

#ifdef FT_KDP_FUNC_SOCK_COMM
  pCtrlBK->SocketRawDrv = -1;
#endif // FT_KDP_FUNC_SOCK_COMM //

#ifndef FT_KDP_FUNC_SOCK_COMM
#ifdef IAPP_OS_LINUX
  pCtrlBK->PipeRawDrv[FT_KDP_PIPE_ID_READ] = -1;
  pCtrlBK->PipeRawDrv[FT_KDP_PIPE_ID_WRITE] = -1;
#endif // IAPP_OS_LINUX //
#endif // FT_KDP_FUNC_SOCK_COMM //

#ifdef IAPP_OS_VXWORKS
  /* muxBind a cookie for receiving commands from wireless driver */
  pCtrlBK->pDrvCookieFrom = muxBind(
      FT_KDP_BR_NAME, FT_KDP_BR_UNIT, (FUNCPTR)IAPP_RcvHandlerRawDrvVxWorks,
      (FUNCPTR)NULL, (FUNCPTR)NULL, (VOIDFUNCPTR)NULL, IAPP_ETH_PRO,
      "IAPP IO FROM", (VOID *)pCtrlBK);

  if (pCtrlBK->pDrvCookieFrom == NULL) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> muxBind device failed!\n"));
    goto label_fail;
  } /* End of if */

  /* muxBind a cookie for issuing commands to wireless driver */
  pCtrlBK->pDrvCookieTo =
      muxBind(FT_KDP_WLAN_NAME, FT_KDP_WLAN_UNIT, (FUNCPTR)NULL, (FUNCPTR)NULL,
              (FUNCPTR)NULL, (VOIDFUNCPTR)NULL, 0, "IAPP IO TO", NULL);

  if (pCtrlBK->pDrvCookieTo == NULL) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> muxBind device failed!\n"));
    goto label_fail;
  }    /* End of if */
#endif // IAPP_OS_VXWORKS //

#if 0
	/* get IP address */
	if ((SerHostName = gethostbyname(IAPP_MULTICAST_ADDR)) == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("iapp> gethostbyname failed!\n"));
		goto label_fail;
	} /* End of if */
#endif

  /* open a Send UDP socket */
  if ((pCtrlBK->SocketUdpSend = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Open UDP socket failed!\n"));
    goto label_fail;
  } /* End of if */

  /* set socket reusable */
  FlgIsReUsed = 1;
  if (setsockopt(pCtrlBK->SocketUdpSend, SOL_SOCKET, SO_REUSEADDR, &FlgIsReUsed,
                 sizeof(FlgIsReUsed)) < 0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> setsockopt-SO_REUSEADDR failed!\n"));
    goto label_fail;
  } /* End of if */

  /* bind the Send UDP socket */
  IAPP_MEM_ZERO(&AddrUdp, sizeof(AddrUdp));
  AddrUdp.sin_family = AF_INET;
  /*
          We can not use multicast address or in vxWorks, it will use the
          multicast address as the source IP address.

          And you will suffer "Address load Exception" in IAPP_Task().

          And you also can not use unicast address, or you can not receive
          any broadcast packet.

          So must use INADDR_ANY.
  */
  // AddrUdp.sin_addr.s_addr = inet_addr(IAPP_MULTICAST_ADDR);
  AddrUdp.sin_addr.s_addr = INADDR_ANY;
  AddrUdp.sin_port = htons(IAPP_UDP_PORT);

  if (bind(pCtrlBK->SocketUdpSend, (struct sockaddr *)&AddrUdp,
           sizeof(AddrUdp)) < 0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Bind Send UDP failed!\n"));
    goto label_fail;
  } /* End of if */

#ifdef IAPP_OS_VXWORKS
  /* support multicast packet sent */
  AddrIf.s_addr = pCtrlBK->AddrOwn.s_addr;

  if (setsockopt(pCtrlBK->SocketUdpSend, IPPROTO_IP, IP_MULTICAST_IF,
                 (char *)&AddrIf, sizeof(AddrIf)) == ERROR) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> iapp> setsockopt error!\n"));
    goto label_fail;
  }    /* End of if */
#endif // IAPP_OS_VXWORKS //

  /* no loopback support */
  FlgIsLoop = 0;
  if (setsockopt(pCtrlBK->SocketUdpSend, IPPROTO_IP, IP_MULTICAST_LOOP,
                 &FlgIsLoop, sizeof(FlgIsLoop)) < 0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> setsockopt-multicast failed!\n"));
    goto label_fail;
  } /* End of if */

#if 0
	IAPP_MEM_ZERO(&MReq, sizeof(MReq));
	MReq.imr_multiaddr.s_addr = inet_addr(IAPP_MULTICAST_ADDR);
	MReq.imr_interface.s_addr = pCtrlBK->AddrOwn.s_addr;
	if (MReq.imr_multiaddr.s_addr == -1)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("iapp> Illegal multicast address!\n"));
		goto label_fail;
	} /* End of if */

#ifndef RTL865X_SOC
	/* register with kernel which addr support multicast */
	if (setsockopt(pCtrlBK->SocketUdpSend,
					IPPROTO_IP,
					IP_ADD_MEMBERSHIP,
					&MReq,
					sizeof(MReq)) < 0)
	{
		/*
			1. Your machine doesn't have multicast support enabled. For example,
			on Linux and FreeBSD it is possible to compile a kernel which
			doesn't support multicast.
			2. You don't have a route for multicast traffic. Some systems don't
			add this by default, and you need to run.
			route add -net 224.0.0.0 AddrNetmask 224.0.0.0 eth0

			Because we bind the socket on multicast address, OS will not know
			what interface it will send any multicast packet to.
		*/

		DBGPRINT(RT_DEBUG_ERROR, ("iapp> setsockopt-IP_ADD_MEMBERSHIP failed!\n"));
		goto label_fail;
	} /* End of if */
#endif // RTL865X_SOC //
#endif // #if 0 //

  /* use broadcast address */
  FlgIsBroadcast = 1;
  if (setsockopt(pCtrlBK->SocketUdpSend, SOL_SOCKET, SO_BROADCAST,
                 &FlgIsBroadcast, sizeof(FlgIsBroadcast)) < 0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> setsockopt-SO_BROADCAST failed!\n"));
    goto label_fail;
  } /* End of if */

#ifdef IAPP_OS_LINUX
  /*
          Retrieve the interface index of the interface into ifr_ifindex by
          ReqIf.ifr_name
  */
  IAPP_MEM_MOVE(ReqIf.ifr_name, pCtrlBK->IfNameEth, IFNAMSIZ);

  DBGPRINT(RT_DEBUG_TRACE,
           ("iapp> Register ethernet interface as (%s)\n", ReqIf.ifr_name));

  if (ioctl(pCtrlBK->SocketUdpSend, SIOCGIFINDEX, &ReqIf) != 0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> ioctl(SIOCGIFINDEX) failed!\n"));
    goto label_fail;
  }    /* End of if */
#endif // IAPP_OS_LINUX //

  /* setup non-blocking RCV UDP */
#if 0
	HandlerUdp.sa_handler = IAPP_RcvHandler;

	/* init signal set */
	if (sigfillset(&HandlerUdp.sa_mask) < 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("iapp> sigfillset failed!\n"));
		goto label_fail;
	} /* End of if */

	HandlerUdp.sa_flags = 0;
	if (sigaction(SIGIO, &HandlerUdp, 0) < 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("iapp> sigaction failed!\n"));
		goto label_fail;
	} /* End of if */
#endif

#ifdef IAPP_OS_LINUX
  /* open RAW socket */
  if ((pCtrlBK->SocketRawBr = socket(
           PF_PACKET, SOCK_RAW,
           //										htons(ETH_P_ALL)))
           //< 0)
           htons(0x0008))) < 0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Open RAW socket failed!\n"));
    goto label_fail;
  } /* End of if */

  /* bind RAW socket to pCtrlBK->IfNameEth (br-lan) */
  IAPP_MEM_ZERO(&AddrRaw, sizeof(AddrRaw));
  AddrRaw.sll_family = AF_PACKET;
  AddrRaw.sll_ifindex = ReqIf.ifr_ifindex;

  if (bind(pCtrlBK->SocketRawBr, (struct sockaddr *)&AddrRaw, sizeof(AddrRaw)) <
      0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Bind RAW socket failed!\n"));
    goto label_fail;
  } /* End of if */

  /* open RAW socket */
  if ((pCtrlBK->SocketRawRRB =
           socket(PF_PACKET, SOCK_RAW, htons(RRB_ETH_PRO))) < 0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> open RRB RAW socket failed!\n"));
    goto label_fail;
  } /* End of if */

  /* bind RAW socket to pCtrlBK->IfNameEth (br-lan) */
  IAPP_MEM_ZERO(&AddrRaw, sizeof(AddrRaw));
  AddrRaw.sll_family = AF_PACKET;
  AddrRaw.sll_ifindex = ReqIf.ifr_ifindex;

  if (bind(pCtrlBK->SocketRawRRB, (struct sockaddr *)&AddrRaw,
           sizeof(AddrRaw)) < 0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Bind RRB RAW socket failed!\n"));
    goto label_fail;
  }    /* End of if */
#endif // IAPP_OS_LINUX //

#ifdef IAPP_OS_VXWORKS
  {
    UINT32 IdIfNum = 0;
    CHAR BindNameBc[20];
    CHAR BindNameRrb[20];

    /* open RAW socket */
    IAPP_MEM_ZERO(BindNameBc, sizeof(BindNameBc));

    for (IdIfNum = 0; IdIfNum < FT_KDP_BR_ETH_IF_NUM; IdIfNum++) {
      sprintf(BindNameBc, "IAPP KDP BC%d", IdIfNum);
      sprintf(BindNameRrb, "IAPP KDP RRB%d", IdIfNum);

      pCtrlBK->pBcCookie[IdIfNum] =
          muxBind(FT_KDP_ETH_NAME, IdIfNum, (FUNCPTR)NULL, (FUNCPTR)NULL,
                  (FUNCPTR)NULL, (VOIDFUNCPTR)NULL, 0, BindNameBc, NULL);

      if (pCtrlBK->pBcCookie[IdIfNum] == NULL) {
        DBGPRINT(RT_DEBUG_ERROR, ("iapp> muxBind BC failed!\n"));
        goto label_fail;
      } /* End of if */

      pCtrlBK->pRrbCookieTo[IdIfNum] = muxBind(
          FT_KDP_ETH_NAME, IdIfNum, (FUNCPTR)NULL, (FUNCPTR)NULL, (FUNCPTR)NULL,
          (VOIDFUNCPTR)NULL, RRB_ETH_PRO, BindNameRrb, NULL);

      if (pCtrlBK->pRrbCookieTo[IdIfNum] == NULL) {
        DBGPRINT(RT_DEBUG_ERROR, ("iapp> iapp> muxBind RRB failed!\n"));
        goto label_fail;
      } /* End of if */
    }   /* End of for */

    pCtrlBK->pRrbCookieFrom = muxBind(
        FT_KDP_BR_NAME, FT_KDP_BR_UNIT, (FUNCPTR)IAPP_RcvHandlerRawRRBVxWorks,
        (FUNCPTR)NULL, (FUNCPTR)NULL, (VOIDFUNCPTR)NULL, RRB_ETH_PRO,
        "IAPP RRB FROM", (VOID *)pCtrlBK);

    if (pCtrlBK->pRrbCookieFrom == NULL) {
      DBGPRINT(RT_DEBUG_ERROR, ("iapp> muxBind RRB failed!\n"));
      goto label_fail;
    } /* End of if */

    if ((pCtrlBK->SocketRawBr = socket(PF_PACKET, SOCK_RAW, 0)) < 0) {
      DBGPRINT(RT_DEBUG_ERROR, ("iapp> Open RAW socket failed!\n"));
      goto label_fail;
    } /* End of if */

    pipeDevCreate(IAPP_KDP_PIPE_DRV, 5, sizeof(RT_SIGNAL_STRUC));
    pCtrlBK->SocketRawDrv = open(IAPP_KDP_PIPE_DRV, O_RDWR, 0);
    if (pCtrlBK->SocketRawDrv < 0) {
      DBGPRINT(RT_DEBUG_ERROR, ("iapp> Open a PIPE DRV failed!\n"));
      goto label_fail;
    } /* End of if */

    pipeDevCreate(IAPP_KDP_PIPE_ETH, 5, IAPP_MAX_RCV_PKT_SIZE);
    pCtrlBK->SocketRawRRB = open(IAPP_KDP_PIPE_ETH, O_RDWR, 0);
    if (pCtrlBK->SocketRawRRB < 0) {
      DBGPRINT(RT_DEBUG_ERROR, ("iapp> Open a PIPE RRB failed!\n"));
      goto label_fail;
    } /* End of if */
  }
#endif // IAPP_OS_VXWORKS //

  /* open IOCTL socket */
  if ((pCtrlBK->SocketIoctl = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    goto label_fail;
  } /* End of if */

  /* Open TCP socket for accepting connection from other AP */
  if ((pCtrlBK->SocketTcpRcv = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Open TcpSocketForOtherAP failed!\n"));
    goto label_fail;
  } /* End of if */

  /* bind the Rcv TCP socket */
  IAPP_MEM_ZERO(&AddrTcp, sizeof(AddrTcp));
  AddrTcp.sin_family = AF_INET;
  AddrTcp.sin_addr.s_addr = htonl(INADDR_ANY);
  AddrTcp.sin_port = htons(IAPP_TCP_PORT);

  if (bind(pCtrlBK->SocketTcpRcv, (struct sockaddr *)&AddrTcp,
           sizeof(AddrTcp)) < 0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Bind Rcv TCP failed!\n"));
    goto label_fail;
  } /* End of if */

  listen(pCtrlBK->SocketTcpRcv, 10); /* max 10 TCP connections simultaneously */

#ifdef FT_KDP_FUNC_SOCK_COMM
#ifdef IAPP_OS_LINUX
  /* open a socket receiving message from driver */
  if ((pCtrlBK->SocketRawDrv =
           socket(PF_PACKET, SOCK_RAW, htons(IAPP_ETH_PRO))) < 0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Open RAW DRV socket failed!\n"));
    goto label_fail;
  } /* End of if */

  /* bind RAW socket to pCtrlBK->IfNameEth (br-lan) */
  IAPP_MEM_MOVE(ReqIf.ifr_name, pCtrlBK->IfNameWlan, IFNAMSIZ);

  if (ioctl(pCtrlBK->SocketRawDrv, SIOCGIFINDEX, &ReqIf) != 0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> ioctl(SIOCGIFINDEX) failed 2!\n"));
    goto label_fail;
  } /* End of if */

  IAPP_MEM_ZERO(&AddrRaw, sizeof(AddrRaw));
  AddrRaw.sll_family = AF_PACKET;
  AddrRaw.sll_ifindex = ReqIf.ifr_ifindex;

  if (bind(pCtrlBK->SocketRawDrv, (struct sockaddr *)&AddrRaw,
           sizeof(AddrRaw)) < 0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Bind RAW DRV socket failed!\n"));
    goto label_fail;
  }    /* End of if */
#endif // IAPP_OS_LINUX //
#endif // FT_KDP_FUNC_SOCK_COMM //

#ifndef FT_KDP_FUNC_SOCK_COMM
#ifdef IAPP_OS_LINUX
  pipe(pCtrlBK->PipeRawDrv);

  if ((pCtrlBK->PipeRawDrv[FT_KDP_PIPE_ID_READ] < 0) ||
      (pCtrlBK->PipeRawDrv[FT_KDP_PIPE_ID_WRITE] < 0)) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Open raw drv pipe failed!\n"));
    goto label_fail;
  }    /* End of if */
#endif // IAPP_OS_LINUX //
#endif // FT_KDP_FUNC_SOCK_COMM //

  return TRUE;

label_fail:
  IAPP_SocketClose(pCtrlBK);
  return FALSE;
} /* End of IAPP_SocketOpen */

/*
========================================================================
Routine Description:
        Start IAPP daemon.

Arguments:
        *pCtrlBK	- IAPP control blcok

Return Value:
        None

Note:
========================================================================
*/
static VOID IAPP_Start(IAPP_IN RTMP_IAPP *pCtrlBK) {
#ifdef IAPP_EVENT_LOG
  struct timeval now, backuptime, res;
  MsgSubType msg;
  INT32 flag = IPC_NOWAIT;
  INT32 msglen;
#endif // IAPP_EVENT_LOG //

  /* init */
  pCtrlBK->FlgIsTerminated = 0;

#ifdef IAPP_EVENT_LOG
  gettimeofday(&backuptime, NULL);
  IAPP_EventLogClean();
#endif // IAPP_EVENT_LOG //

  mt_iapp_ft_client_table_init(pCtrlBK);

  /* waiting for local AP SysCmd or peer AP IAPP packets */
  while (!pCtrlBK->FlgIsTerminated) {
#ifdef IAPP_EVENT_LOG
    /* check for message from 8021X daemon (no sleep) */
    if ((msglen = msgrcv(rtmpiapp.MsgId, &msg, sizeof(MsgSubType),
                         RADIUSMSGQUEID, flag)) > 0) {
      IAPP_MsgProcess(&msg, msglen);
    } /* End of if */

    /* implement timer function */
    gettimeofday(&now, NULL);
    timersub(&now, &backuptime, &res);

    /* 1s periodic timer to write events */
    if ((res.tv_sec >= 1000) /* || (res.tv_usec >= 100000)*/) {
      backuptime = now;
      IAPP_EventLog_Query();
    }  /* End of if */
#endif // IAPP_EVENT_LOG //

    /* CPU suspend will be done in IAPP_RcvHandler() */
    IAPP_RcvHandler(pCtrlBK, 0);
  } /* End of while */

#ifdef IAPP_EVENT_LOG
  /* clean all events */
  IAPP_EventLogClean();
#endif // IAPP_EVENT_LOG //
} /* End of IAPP_Start */

/*
========================================================================
Routine Description:
        Send out a IAPP UDP packet.

Arguments:
        *pCtrlBK	- IAPP control blcok
        *pPkt		- the UDP packet
        PktLen		- the packet size
        *pRspBuf	- used to issue response to the peer

Return Value:
        TRUE		- send successfully
        FAIL		- send fail

Note:
========================================================================
*/
static BOOLEAN IAPP_UDP_PacketSend(IAPP_IN RTMP_IAPP *pCtrlBK,
                                   IAPP_IN UCHAR *pPkt, IAPP_IN UINT32 PktLen,
                                   IAPP_IN UCHAR *pRspBuf,
                                   IAPP_IN INT32 if_idx) {
  struct sockaddr_in AddrMulticast;
  RT_IAPP_HEADER *pIappHdr;
  UINT16 Identifier;
  UCHAR *pBufEncrypt;
  BOOLEAN Status;
  UINT32 total_len;

  /* init */
  pIappHdr = (RT_IAPP_HEADER *)pPkt;
  Status = TRUE;

  /* for identification of the request/response packet */
  Identifier = IAPP_IDENTIFIER_GET(pCtrlBK);
  pIappHdr->Identifier = SWAP_16(Identifier);

  pCtrlBK->PacketIdentifier++;

  IAPP_MEM_ZERO(&AddrMulticast, sizeof(AddrMulticast));
  AddrMulticast.sin_family = AF_INET;
  //	AddrMulticast.sin_addr.s_addr = inet_addr(IAPP_MULTICAST_ADDR);
  AddrMulticast.sin_addr.s_addr = pCtrlBK->AddrBroadcast.s_addr;
  AddrMulticast.sin_port = htons(IAPP_UDP_PORT);

  IAPP_RSP_BUF_ALLOCATE(pRspBuf, pBufEncrypt,
                        PktLen + IAPP_SECURITY_EXTEND_LEN + ETH_ALEN);
  if (pBufEncrypt == NULL)
    return FALSE;
  /* End of if */
  IAPP_MEM_MOVE(pBufEncrypt, pPkt, PktLen);
  IAPP_MEM_MOVE(pBufEncrypt + PktLen, pCtrlBK->IfNameWlanMAC[if_idx], ETH_ALEN);
  total_len = PktLen + ETH_ALEN;

#ifdef FT_KDP_FUNC_PKT_ENCRYPT
  /* ioctl to encrypt */
  IAPP_IOCTL_TO_WLAN(pCtrlBK, RT_IOCTL_IAPP, pBufEncrypt, &total_len, if_idx,
                     RT_FT_DATA_ENCRYPT);
#endif // FT_KDP_FUNC_PKT_ENCRYPT //

  /* send out the packet */
  if (sendto(pCtrlBK->SocketUdpSend, pBufEncrypt, total_len, 0,
             (struct sockaddr *)&AddrMulticast,
             sizeof(AddrMulticast)) != (total_len)) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Send UDP packet failed!\n"));
    Status = FALSE;
  } /* End of if */

  DBGPRINT(RT_DEBUG_TRACE,
           ("iapp> Send UDP packet ok (Len = %d)\n\n", total_len));
  return Status;
} /* End of IAPP_UDP_PacketSend */

/*
========================================================================
Routine Description:
        Handle received IAPP UDP packets.

Arguments:
        *pCtrlBK	- IAPP control blcok
        Sig			- no use

Return Value:
        None

Note:
========================================================================
*/
VOID IAPP_RcvHandler(IAPP_IN RTMP_IAPP *pCtrlBK, IAPP_IN INT32 Sig) {
  UCHAR *pPktBuf, *pCmdBuf, *pRspBuf;
#if 0
	UCHAR *pIpHdr;
#endif
  fd_set FdSet;
  INT32 SockMax;
#ifdef FT_KDP_SELECT_TIMEOUT
  struct timeval Timeout;
#endif // FT_KDP_SELECT_TIMEOUT //

  /* sanity check */
  if (pCtrlBK->FlgIsRcvRunning == TRUE)
    return; /* the handler function is running */
  /* End of if */

  /* init */
  pPktBuf = NULL;
  pCmdBuf = NULL;
  pRspBuf = NULL;

  /* allocate packet & cmd Buffer */
  IAPP_MEM_ALLOC(pPktBuf, IAPP_MAX_RCV_PKT_SIZE + IAPP_MAX_RCV_PKT_SIZE_SAFE);
  if (pPktBuf == NULL) {
    printf("Allocate packet buffer fail!\n");
    goto LabelExit;
  } /* End of if */

  IAPP_MEM_ALLOC(pCmdBuf, IAPP_MAX_RCV_PKT_SIZE + IAPP_MAX_RCV_PKT_SIZE_SAFE);
  if (pCmdBuf == NULL) {
    printf("Allocate command buffer fail!\n");
    goto LabelExit;
  } /* End of if */

  IAPP_MEM_ALLOC(pRspBuf, IAPP_MAX_RCV_PKT_SIZE + IAPP_MAX_RCV_PKT_SIZE_SAFE);
  if (pRspBuf == NULL) {
    printf("Allocate response buffer fail!\n");
    goto LabelExit;
  } /* End of if */

  /* init */
  pCtrlBK->FlgIsRcvRunning = TRUE;
  SockMax = pCtrlBK->SocketUdpSend;

  if (pCtrlBK->SocketTcpRcv > pCtrlBK->SocketUdpSend)
    SockMax = pCtrlBK->SocketTcpRcv;
    /* End of if */

#ifdef FT_KDP_FUNC_SOCK_COMM
  if (pCtrlBK->SocketRawDrv > SockMax)
    SockMax = pCtrlBK->SocketRawDrv;
    /* End of if */
#endif // FT_KDP_FUNC_SOCK_COMM //

  if (pCtrlBK->SocketRawRRB > SockMax)
    SockMax = pCtrlBK->SocketRawRRB;
    /* End of if */

#ifndef FT_KDP_FUNC_SOCK_COMM
#ifdef IAPP_OS_LINUX
  if (pCtrlBK->PipeRawDrv[FT_KDP_PIPE_ID_READ] > SockMax)
    SockMax = pCtrlBK->PipeRawDrv[FT_KDP_PIPE_ID_READ];
    /* End of if */
#endif // IAPP_OS_LINUX //
#endif // FT_KDP_FUNC_SOCK_COMM //

  /* waiting for any UDP packet */
  while (1) {
    /* must re- FD_SET before each select() */
    FD_ZERO(&FdSet);

    FD_SET(pCtrlBK->SocketUdpSend, &FdSet);
    FD_SET(pCtrlBK->SocketTcpRcv, &FdSet);
    FD_SET(pCtrlBK->SocketRawRRB, &FdSet);

#ifdef FT_KDP_FUNC_SOCK_COMM
    FD_SET(pCtrlBK->SocketRawDrv, &FdSet);
#endif // FT_KDP_FUNC_SOCK_COMM //

#ifndef FT_KDP_FUNC_SOCK_COMM
#ifdef IAPP_OS_LINUX
    FD_SET(pCtrlBK->PipeRawDrv[FT_KDP_PIPE_ID_READ], &FdSet);
#endif // IAPP_OS_LINUX //
#endif // FT_KDP_FUNC_SOCK_COMM //

#ifdef FT_KDP_SELECT_TIMEOUT
    IAPP_MEM_ZERO(&Timeout, sizeof(Timeout));
    Timeout.tv_sec = IAPP_SELECT_TIMEOUT;
    Timeout.tv_usec = 0;

    if (select(SockMax + 1, &FdSet, NULL, NULL, &Timeout) < 0)
      break;
      /* End of if */
#else

    /* must use SockMax+1, not SockMax */
    if (select(SockMax + 1, &FdSet, NULL, NULL, NULL) < 0)
      break;
      /* End of if */
#endif // FT_KDP_SELECT_TIMEOUT //

    /* handle packets from UDP layer */
    if (FD_ISSET(pCtrlBK->SocketUdpSend, &FdSet))
      IAPP_RcvHandlerUdp(pCtrlBK, pPktBuf, pCmdBuf, pRspBuf);
    /* End of if */

    /* handle packets from TCP layer */
    if (FD_ISSET(pCtrlBK->SocketTcpRcv, &FdSet))
      IAPP_RcvHandlerTcp(pCtrlBK, pPktBuf, pCmdBuf, pRspBuf);
    /* End of if */

    if (FD_ISSET(pCtrlBK->SocketRawRRB, &FdSet))
      IAPP_RcvHandlerRawRRB(pCtrlBK, pPktBuf, pCmdBuf, pRspBuf);
      /* End of if */

#ifdef FT_KDP_FUNC_SOCK_COMM
    if (FD_ISSET(pCtrlBK->SocketRawDrv, &FdSet))
      IAPP_RcvHandlerRawDrv(pCtrlBK, pPktBuf, pCmdBuf, pRspBuf);
      /* End of if */
#endif // FT_KDP_FUNC_SOCK_COMM //

#ifndef FT_KDP_FUNC_SOCK_COMM
#ifdef IAPP_OS_LINUX
    if (FD_ISSET(pCtrlBK->PipeRawDrv[FT_KDP_PIPE_ID_READ], &FdSet)) {
      UINT32 CmdLen;

      CmdLen = read(pCtrlBK->PipeRawDrv[FT_KDP_PIPE_ID_READ], pPktBuf,
                    IAPP_MAX_RCV_PKT_SIZE);
      if (CmdLen > 0) {
        /* handle the signal context, assoicate or reassociate or terminate */
        IAPP_SIG_Process((RT_SIGNAL_STRUC *)pPktBuf, CmdLen, pCmdBuf, pRspBuf);
      } /* End of if */
    }   /* End of if */
#endif  // IAPP_OS_LINUX //
#endif  // FT_KDP_FUNC_SOCK_COMM //
  };

LabelExit:
  if (pPktBuf != NULL)
    IAPP_MEM_FREE(pPktBuf);
  /* End of if */
  if (pCmdBuf != NULL)
    IAPP_MEM_FREE(pCmdBuf);
  /* End of if */
  if (pRspBuf != NULL)
    IAPP_MEM_FREE(pRspBuf);
  /* End of if */
  pCtrlBK->FlgIsRcvRunning = FALSE;
} /* End of IAPP_RcvHandler */

/*
========================================================================
Routine Description:
        Handle received IAPP TCP packets.

Arguments:
        *pCtrlBK	- IAPP control blcok
        *pPktBuf	- packet buffer
        *pCmdBuf	- used to issue command to WLAN driver
        *pRspBuf	- used to issue response to the peer

Return Value:
        None

Note:
========================================================================
*/
static VOID IAPP_RcvHandlerTcp(IAPP_IN RTMP_IAPP *pCtrlBK,
                               IAPP_IN UCHAR *pPktBuf, IAPP_IN UCHAR *pCmdBuf,
                               IAPP_IN UCHAR *pRspBuf) {
  struct sockaddr_in AddrPeer;
  RT_IAPP_HEADER *pIappHdr;
  INT32 SockNew;
  socklen_t Length;
  INT32 SizeRcvMsg;
  INT32 if_idx = 0;
  // UCHAR WifiMAC[ETH_ALEN] = {0};

  /* init */
  IAPP_MEM_ZERO(&AddrPeer, sizeof(AddrPeer));
  Length = sizeof(struct sockaddr_in);

  SockNew =
      accept(pCtrlBK->SocketTcpRcv, (struct sockaddr *)&AddrPeer, &Length);
  if (SockNew < 0)
    return; /* no any new connection */
  /* End of if */

  /* handle the packet */
  SizeRcvMsg = read(SockNew, pPktBuf, IAPP_MAX_RCV_PKT_SIZE);
  close(SockNew);

  if (SizeRcvMsg > 0) {
    DBGPRINT(RT_DEBUG_TRACE, ("iapp> Recv TCP successfully from %d.%d.%d.%d\n",
                              IAPP_SHOW_IP(AddrPeer.sin_addr.s_addr)));

#ifdef FT_KDP_FUNC_PKT_ENCRYPT
    /* ioctl to decrypt */
    IAPP_IOCTL_TO_WLAN(pCtrlBK, RT_IOCTL_IAPP, pPktBuf, &SizeRcvMsg, 0,
                       RT_FT_DATA_DECRYPT);
#endif // FT_KDP_FUNC_PKT_ENCRYPT //

    /* get IAPP frame body */
    pIappHdr = (RT_IAPP_HEADER *)(pPktBuf);

    if (pIappHdr->Version != IAPP_VERSION) {
      DBGPRINT(RT_DEBUG_TRACE,
               ("iapp> IAPP version not match %d!\n", pIappHdr->Version));
      IAPP_HEX_DUMP("Wrong TCP Frame Content: ", pPktBuf, SizeRcvMsg);
      return; /* version not match */
    }         /* End of if */

    DBGPRINT(RT_DEBUG_TRACE, ("iapp> IAPP SysCmd = %d\n", pIappHdr->Command));

    switch (pIappHdr->Command) {
    case IAPP_CMD_MOVE_NOTIFY:
      IAPP_RcvHandlerMoveReq(pCtrlBK, (UCHAR *)pIappHdr,
                             AddrPeer.sin_addr.s_addr, pCmdBuf, pRspBuf);
      break;

    case IAPP_CMD_MOVE_RESPONSE: {
      /*
              If the received sequence number does not represent a more
              recent association than that at the AP where the
              IAPPMOVE.indication is received, the APME should ignore the
              indicated reassociation, the APME should issue an
              IAPP-MOVE.response with a status of STALE_MOVE that will
              cause an IAPP MOVE-response packet to be sent to the AP
              that originated the IAPP MOVE-notify packet, and the APME
              should issue an IAPPADD.request primitive of its own to
              ensure that all Layer 2 devices are properly informed of
              the correct location of the STA’s most recent association.
      */

      DBGPRINT(
          RT_DEBUG_TRACE,
          ("iapp> Receive IAPP_CMD_MOVE_RESPONSE! (size = %d)\n", SizeRcvMsg));

      /* not yet implement */
    } break;

    case IAPP_CMD_FT_SEND_SECURITY_BLOCK:
      IAPP_RcvHandlerSSB(pCtrlBK, (UCHAR *)pIappHdr, AddrPeer.sin_addr.s_addr,
                         pCmdBuf);
      break;

    case IAPP_CMD_FT_ACK_SECURITY_BLOCK: {
      RT_IAPP_SEND_SECURITY_BLOCK *pAckSB;
      UCHAR *pBufMsg;
      UINT32 BufLen;
      POID_REQ OidReq;
      INT32 idx;

      /* init */
      pAckSB = (RT_IAPP_SEND_SECURITY_BLOCK *)pIappHdr;

      BufLen = sizeof(OID_REQ);
      BufLen += FT_IP_ADDRESS_SIZE + IAPP_SB_INIT_VEC_SIZE + pAckSB->Length;

      IAPP_CMD_BUF_ALLOCATE(pCmdBuf, pBufMsg, BufLen);
      if (pBufMsg == NULL)
        break;
      /* End of if */

      /* command to notify that a Key Req is received */
      DBGPRINT(RT_DEBUG_TRACE, ("iapp> IAPP_CMD_FT_ACK_SECURITY_BLOCK\n"));

      OidReq = (POID_REQ)pBufMsg;
      OidReq->OID = (RT_SET_FT_KEY_RSP | OID_GET_SET_TOGGLE);

      /* peer IP address */
      IAPP_MEM_MOVE(OidReq->Buf, &(AddrPeer.sin_addr.s_addr),
                    FT_IP_ADDRESS_SIZE);

      /* IP & nonce & security block */
      IAPP_MEM_MOVE(OidReq->Buf + FT_IP_ADDRESS_SIZE, pAckSB->InitVec,
                    IAPP_SB_INIT_VEC_SIZE);
      IAPP_MEM_MOVE(OidReq->Buf + FT_IP_ADDRESS_SIZE + IAPP_SB_INIT_VEC_SIZE,
                    pAckSB->SB, pAckSB->Length);

      OidReq->Len = BufLen - sizeof(OID_REQ);

      /*
              send to all wifi 11r interface.
      */
      for (idx = 0; idx < pCtrlBK->IfNameWlanCount; idx++) {
        IAPP_MsgProcess(pCtrlBK, IAPP_SET_OID_REQ, pBufMsg, BufLen, idx);
      }
    } break;

    case IAPP_CMD_INFO_RESPONSE:
      IAPP_RcvHandlerApInfor(pCtrlBK, IAPP_INFO_TYPE_RSP, (UCHAR *)pIappHdr,
                             AddrPeer.sin_addr.s_addr, pCmdBuf, if_idx);
      break;

    default:
      IAPP_HEX_DUMP("Wrong TCP Frame Content: ", pPktBuf, SizeRcvMsg);
      break;
    }
  } /* End of if */
} /* End of IAPP_RcvHandlerTcp */

/*
========================================================================
Routine Description:
        Handle received IAPP Move Request packet.

Arguments:
        *pCtrlBK	- IAPP control blcok
        *pPktBuf	- packet buffer
        PeerIP		- the peer IP address
        *pCmdBuf	- used to issue command to WLAN driver
        *pRspBuf	- used to issue response to the peer

Return Value:
        None

Note:
========================================================================
*/
static VOID IAPP_RcvHandlerMoveReq(IAPP_IN RTMP_IAPP *pCtrlBK,
                                   IAPP_IN UCHAR *pPktBuf,
                                   IAPP_IN UINT32 PeerIP,
                                   IAPP_IN UCHAR *pCmdBuf,
                                   IAPP_IN UCHAR *pRspBuf) {
  /*
          In IAPP, when the Status is not SUCCESSFUL, the APME should
          disassociate the STA indicated by the MAC Address
          parameter, using the 802.11 MLME-DISASSOCIATE.request
          primitive with a Reason Code of 1, meaning "Unspecified
          Reason." Future revisions of the IEEE Std 802.11 may define
          a new Reason Code that means "Old AP did not verify
          previous association."
  */
  RT_IAPP_MOVE_NOTIFY *pNotify;
  RT_IAPP_MOVE_RSP *pRsp;
  UCHAR *pBufMsg;
  UINT32 BufLen;
  POID_REQ OidReq;
  INT32 if_idx;

  /* sanity check */
  pNotify = (RT_IAPP_MOVE_NOTIFY *)pPktBuf;

  if (SWAP_16(pNotify->IappHeader.Length) != sizeof(RT_IAPP_MOVE_NOTIFY)) {
    DBGPRINT(RT_DEBUG_TRACE, ("iapp> Len %d != Move Request Len %d!\n",
                              SWAP_16(pNotify->IappHeader.Length),
                              sizeof(RT_IAPP_MOVE_NOTIFY)));
    return;
  } /* End of if */

  /* delete MAC Entry when receive a add-notify packet */
  BufLen = sizeof(OID_REQ);
  BufLen += ETH_ALEN;

  IAPP_CMD_BUF_ALLOCATE(pCmdBuf, pBufMsg, BufLen);
  if (pBufMsg == NULL)
    return;
  /* End of if */

  OidReq = (POID_REQ)pBufMsg;
  OidReq->OID = (RT_SET_DEL_MAC_ENTRY | OID_GET_SET_TOGGLE);
  IAPP_MEM_MOVE(OidReq->Buf, pNotify->MacAddr, ETH_ALEN);
  OidReq->Len = BufLen - sizeof(OID_REQ);

  if_idx =
      mt_iapp_find_ifidx_by_sta_mac(&pCtrlBK->SelfFtStaTable, pNotify->MacAddr);
  if (if_idx < 0) {
    DBGPRINT(RT_DEBUG_TRACE,
             ("iapp> %s: cannot find wifi interface.\n", __FUNCTION__));
    return;
  }

  /*
          Note: RALINK AP driver delete the STATION MAC by MAC
          address, do NOT care which BSS index.
  */
  IAPP_MsgProcess(pCtrlBK, IAPP_SET_OID_REQ, pBufMsg, BufLen, if_idx);

  DBGPRINT(RT_DEBUG_TRACE,
           ("iapp> (Receive IAPP_CMD_MOVE_NOTIFY for "
            "%02x:%02x:%02x:%02x:%02x:%02x)\n",
            pNotify->MacAddr[0], pNotify->MacAddr[1], pNotify->MacAddr[2],
            pNotify->MacAddr[3], pNotify->MacAddr[4], pNotify->MacAddr[5]));

  /* reponse the MOVE request */
  pRsp = (RT_IAPP_MOVE_RSP *)pNotify;
  pRsp->IappHeader.Command = IAPP_CMD_MOVE_RESPONSE;
  pRsp->Status = IAPP_MOVE_RSP_STATUS_SUCCESS;

  IAPP_TCP_PACKET_SEND(pCtrlBK, pRsp, sizeof(RT_IAPP_MOVE_RSP), PeerIP, TRUE,
                       pRspBuf, if_idx);

  mt_iapp_ft_client_delete(&pCtrlBK->SelfFtStaTable, pNotify->MacAddr);

} /* End of IAPP_RcvHandlerMoveReq */

/*
========================================================================
Routine Description:
        Handle received IAPP Security Send Block packet.

Arguments:
        *pCtrlBK	- IAPP control blcok
        *pPktBuf	- packet buffer
        PeerIP		- the peer IP address
        *pCmdBuf	- used to issue command to WLAN driver

Return Value:
        None

Note:
========================================================================
*/
static VOID IAPP_RcvHandlerSSB(IAPP_IN RTMP_IAPP *pCtrlBK,
                               IAPP_IN UCHAR *pPktBuf, IAPP_IN UINT32 PeerIP,
                               IAPP_IN UCHAR *pCmdBuf) {
  RT_IAPP_SEND_SECURITY_BLOCK *pSendSB;
  UCHAR *pBufMsg;
  UINT32 BufLen, if_idx;
  POID_REQ OidReq;
  FT_KDP_EVT_KEY_ELM kdp_info;

  /* init */
  pSendSB = (RT_IAPP_SEND_SECURITY_BLOCK *)pPktBuf;

  BufLen = sizeof(OID_REQ);
  BufLen += FT_IP_ADDRESS_SIZE + IAPP_SB_INIT_VEC_SIZE + pSendSB->Length;

  IAPP_CMD_BUF_ALLOCATE(pCmdBuf, pBufMsg, BufLen);
  if (pBufMsg == NULL)
    return;
  /* End of if */

  /* command to notify that a Key Req is received */
  DBGPRINT(RT_DEBUG_TRACE, ("iapp> IAPP_RcvHandlerSSB\n"));

  OidReq = (POID_REQ)pBufMsg;
  OidReq->OID = (RT_SET_FT_KEY_REQ | OID_GET_SET_TOGGLE);

  /* peer IP address */
  IAPP_MEM_MOVE(OidReq->Buf, &PeerIP, FT_IP_ADDRESS_SIZE);

  /* nonce & security block */
  IAPP_MEM_MOVE(OidReq->Buf + FT_IP_ADDRESS_SIZE, pSendSB->InitVec,
                IAPP_SB_INIT_VEC_SIZE);
  IAPP_MEM_MOVE(OidReq->Buf + FT_IP_ADDRESS_SIZE + IAPP_SB_INIT_VEC_SIZE,
                pSendSB->SB, pSendSB->Length);
  IAPP_MEM_MOVE(&kdp_info, pSendSB->SB, pSendSB->Length);
  IAPP_HEX_DUMP("kdp_info.MacAddr", kdp_info.MacAddr, ETH_ALEN);
  if_idx =
      mt_iapp_find_ifidx_by_sta_mac(&pCtrlBK->SelfFtStaTable, kdp_info.MacAddr);
  if (if_idx < 0) {
    DBGPRINT(RT_DEBUG_TRACE,
             ("iapp> %s: cannot find wifi interface\n", __FUNCTION__));
    return;
  }

  OidReq->Len = BufLen - sizeof(OID_REQ);

  IAPP_MsgProcess(pCtrlBK, IAPP_SET_OID_REQ, pBufMsg, BufLen, if_idx);

#ifndef FT_KDP_FUNC_SOCK_COMM
  /*
          Note: in VxWorks, we can not send any signal to same task
          which issues a ioctl path.

          So we poll the event automatically.
  */
  /* receive event */
  IAPP_USR2Handle(0, if_idx);
#endif // FT_KDP_FUNC_SOCK_COMM //
} /* End of IAPP_RcvHandlerSSB */

/*
========================================================================
Routine Description:
        Handle received IAPP AP Information BC/REQ/RSP packet.

Arguments:
        *pCtrlBK	- IAPP control blcok
        Type		- IAPP_INFO_TYPE_BC, IAPP_INFO_TYPE_REQ,
IAPP_INFO_TYPE_RSP *pPktBuf	- packet buffer PeerIP		- the peer IP
address *pCmdBuf	- used to issue command to WLAN driver

Return Value:
        None

Note:
========================================================================
*/
static VOID IAPP_RcvHandlerApInfor(IAPP_IN RTMP_IAPP *pCtrlBK,
                                   IAPP_IN UCHAR Type, IAPP_IN UCHAR *pPktBuf,
                                   IAPP_IN UINT32 PeerIP,
                                   IAPP_IN UCHAR *pCmdBuf,
                                   IAPP_IN INT32 if_idx) {
  RT_IAPP_INFORMATION *pApIB;
  UCHAR *pBufMsg;
  UINT32 BufLen;
  POID_REQ OidReq;
  INT32 OID[IAPP_INFO_TYPE_MAX_NUM] = {
      RT_FT_NEIGHBOR_REPORT, RT_FT_NEIGHBOR_REQUEST, RT_FT_NEIGHBOR_RESPONSE};

  /* sanity check */
  if (Type >= IAPP_INFO_TYPE_MAX_NUM) {
    DBGPRINT(RT_DEBUG_TRACE, ("iapp> Wrong Info Type %d\n", Type));
    return;
  } /* End of if */

  /* init */
  pApIB = (RT_IAPP_INFORMATION *)pPktBuf;

  BufLen = sizeof(OID_REQ);
  BufLen += FT_IP_ADDRESS_SIZE + pApIB->Length;

  IAPP_CMD_BUF_ALLOCATE(pCmdBuf, pBufMsg, BufLen);
  if (pBufMsg == NULL)
    return;
  /* End of if */

  /* command to notify that a Key Req is received */
  DBGPRINT(RT_DEBUG_TRACE, ("iapp> IAPP_RcvHandlerApInfor %d\n", Type));

  OidReq = (POID_REQ)pBufMsg;
  OidReq->OID = (OID[Type] | OID_GET_SET_TOGGLE);

  /* peer IP address & information block */
  IAPP_MEM_MOVE(OidReq->Buf, &PeerIP, FT_IP_ADDRESS_SIZE);
  IAPP_MEM_MOVE(OidReq->Buf + FT_IP_ADDRESS_SIZE, pApIB->IB, pApIB->Length);
  OidReq->Len = BufLen - sizeof(OID_REQ);

  IAPP_MsgProcess(pCtrlBK, IAPP_SET_OID_REQ, pBufMsg, BufLen, if_idx);

#ifndef FT_KDP_FUNC_SOCK_COMM
  /*
          Note: in VxWorks, we can not send any signal to same task
          which issues a ioctl path.

          So we poll the event automatically.
  */
  /* receive event */
  IAPP_USR2Handle(0, if_idx);
#endif // FT_KDP_FUNC_SOCK_COMM //
} /* End of IAPP_RcvHandlerApInfor */

#ifdef FT_KDP_FUNC_SOCK_COMM
/*
========================================================================
Routine Description:
        Handle received IAPP RAW packets from WLAN driver.

Arguments:
        *pCtrlBK	- IAPP control blcok
        *pPktBuf	- packet buffer
        *pCmdBuf	- used to issue command to WLAN driver
        *pRspBuf	- used to issue response to the peer

Return Value:
        None

Note:
========================================================================
*/
static VOID IAPP_RcvHandlerRawDrv(IAPP_IN RTMP_IAPP *pCtrlBK,
                                  IAPP_IN UCHAR *pPktBuf,
                                  IAPP_IN UCHAR *pCmdBuf,
                                  IAPP_IN UCHAR *pRspBuf) {
  RT_SIGNAL_STRUC *pSignal;
  INT32 SizeRcvMsg;

#ifdef IAPP_OS_LINUX
  struct sockaddr_in AddrPeer;
  socklen_t Length;

  /* init */
  IAPP_MEM_ZERO(&AddrPeer, sizeof(AddrPeer));
  Length = sizeof(struct sockaddr_in);

  SizeRcvMsg = recvfrom(pCtrlBK->SocketRawDrv, pPktBuf, IAPP_MAX_RCV_PKT_SIZE,
                        0, (struct sockaddr *)&AddrPeer, &Length);
#endif // IAPP_OS_LINUX //

#ifdef IAPP_OS_VXWORKS
  SizeRcvMsg =
      read(pCtrlBK->SocketRawDrv, (CHAR *)pPktBuf, IAPP_MAX_RCV_PKT_SIZE);
#endif // IAPP_OS_VXWORKS //

  /* handle the packet */
  if (SizeRcvMsg > 0) {
    UCHAR WifiMAC[ETH_ALEN];

    DBGPRINT(RT_DEBUG_TRACE, ("iapp> Recvfrom RAW CMD successfully (%d, %d)!\n",
                              IAPP_MemAllocNum, IAPP_MemFreeNum));

    NdisZeroMemory(WifiMAC, ETH_ALEN);
    NdisCopyMemory(WifiMAC, pPktBuf, ETH_ALEN);

    /* handle the signal context, assoicate or reassociate or terminate */
    pSignal = (RT_SIGNAL_STRUC *)(pPktBuf + sizeof(FT_ETH_HEADER));
    IAPP_SIG_Process(pCtrlBK, WifiMAC, pSignal, SizeRcvMsg, pCmdBuf, pRspBuf);
  } /* End of if */
} /* End of IAPP_RcvHandlerRawDrv */

/*
========================================================================
Routine Description:
        Handle received RRB RAW packets from LAN.

Arguments:
        *pCtrlBK	- IAPP control blcok
        *pPktBuf	- packet buffer
        *pCmdBuf	- used to issue command to WLAN driver
        *pRspBuf	- used to issue response to the peer

Return Value:
        None

Note:
========================================================================
*/
static VOID IAPP_RcvHandlerRawRRB(IAPP_IN RTMP_IAPP *pCtrlBK,
                                  IAPP_IN UCHAR *pPktBuf,
                                  IAPP_IN UCHAR *pCmdBuf,
                                  IAPP_IN UCHAR *pRspBuf) {
  INT32 SizeRcvMsg;
  UCHAR *pBufMsg;
  POID_REQ OidReq;
  FT_RRB_FRAME *pFrameRRB;

#ifdef IAPP_OS_LINUX
  struct sockaddr_in AddrPeer;
  socklen_t Length;

  /* init */
  IAPP_MEM_ZERO(&AddrPeer, sizeof(AddrPeer));
  Length = sizeof(struct sockaddr_in);

  SizeRcvMsg = recvfrom(pCtrlBK->SocketRawRRB, pPktBuf, IAPP_MAX_RCV_PKT_SIZE,
                        0, (struct sockaddr *)&AddrPeer, &Length);
#endif // IAPP_OS_LINUX //

#ifdef IAPP_OS_VXWORKS
  SizeRcvMsg =
      read(pCtrlBK->SocketRawRRB, (CHAR *)pPktBuf, IAPP_MAX_RCV_PKT_SIZE);
#endif // IAPP_OS_VXWORKS //

  /* handle the packet */
  if (SizeRcvMsg > 0) {
    INT32 wifi_if_idx = 0;
    UCHAR WifiMAC[ETH_ALEN];

    DBGPRINT(RT_DEBUG_TRACE,
             ("iapp> Recvfrom RRB RAW successfully! (len = %d)\n", SizeRcvMsg));

#ifdef FT_KDP_FUNC_PKT_ENCRYPT
    /* ioctl to decrypt */
    pFrameRRB = (FT_RRB_FRAME *)pPktBuf;

    if ((wifi_if_idx = mt_iapp_find_ifidx_by_mac(pCtrlBK, pPktBuf)) == -1) {
      DBGPRINT(RT_DEBUG_ERROR, ("iapp> %s - Daemon doesn't hook this wifi "
                                "interface. Ignore this packet.\n",
                                __FUNCTION__));
      IAPP_HEX_DUMP("802.3 Hdr: ", pPktBuf, 14);
      return;
    }
    NdisZeroMemory(WifiMAC, ETH_ALEN);
    NdisCopyMemory(WifiMAC, pPktBuf, ETH_ALEN);

    /*
            Note: Can not use "SizeRcvMsg - FT_RRB_HEADER_SIZE" to get the
            encrypted data length, because when the ethernet frame length
            < 64B and ethernet driver will fill 0 to the end of the frame,
            we will get the wrong the encrypted data length.
    */
    SizeRcvMsg = pFrameRRB->FTActionLength;
    IAPP_ENCRYPTED_DATA_SIZE_CAL(SizeRcvMsg);

    IAPP_IOCTL_TO_WLAN(pCtrlBK, RT_IOCTL_IAPP, pPktBuf + FT_RRB_HEADER_SIZE,
                       &SizeRcvMsg, wifi_if_idx, RT_FT_DATA_DECRYPT);

    SizeRcvMsg += FT_RRB_HEADER_SIZE;
#endif // FT_KDP_FUNC_PKT_ENCRYPT //

    IAPP_CMD_BUF_ALLOCATE(pCmdBuf, pBufMsg, (sizeof(OID_REQ) + SizeRcvMsg));
    if (pBufMsg == NULL)
      return;
    /* End of if */

    OidReq = (POID_REQ)pBufMsg;
    OidReq->OID = (RT_FT_ACTION_FORWARD | OID_GET_SET_TOGGLE);

    /* nonce & security block */
    IAPP_MEM_MOVE(OidReq->Buf, pPktBuf, SizeRcvMsg);
    OidReq->Len = SizeRcvMsg;

    IAPP_MsgProcess(pCtrlBK, IAPP_SET_OID_REQ, pBufMsg,
                    (sizeof(OID_REQ) + SizeRcvMsg), wifi_if_idx);
  } /* End of if */
} /* End of IAPP_RcvHandlerRawRRB */

#ifdef IAPP_OS_VXWORKS
/*
========================================================================
Routine Description:
        Handle received IAPP RAW packets from WLAN driver.

Arguments:
        *pCookie		- protocol/device binding from muxBind()
        Type			- Protocol type
        *pMblk			- The whole packet
        *pSpareData		- pointer to link level header info
        *pSpare			- spare pointer from muxBind()

Return Value:
        None

Note:
========================================================================
*/
BOOLEAN IAPP_RcvHandlerRawDrvVxWorks(IAPP_IN void *pCookie, IAPP_IN long Type,
                                     IAPP_IN M_BLK_ID pMblk,
                                     IAPP_IN LL_HDR_INFO *pLinkHdrInfo,
                                     IAPP_IN void *pSpare) {
  RTMP_IAPP *pCtrlBK = (RTMP_IAPP *)pSpare;

  if ((pCtrlBK != NULL) && (pMblk != NULL)) {
    /* handle the signal context, assoicate or reassociate or terminate */
    write(pCtrlBK->SocketRawDrv, (CHAR *)pMblk->mBlkHdr.mData,
          pMblk->mBlkHdr.mLen);

    /* free the Mblk */
    netMblkClChainFree(pMblk);
  } /* End of if */

  return TRUE;
} /* End of IAPP_RcvHandlerRawDrvVxWorks */

/*
========================================================================
Routine Description:
        Handle received IAPP RAW RRB packets from ethernet driver.

Arguments:
        *pCookie		- protocol/device binding from muxBind()
        Type			- Protocol type
        *pMblk			- The whole packet
        *pSpareData		- pointer to link level header info
        *pSpare			- spare pointer from muxBind()

Return Value:
        None

Note:
========================================================================
*/
BOOLEAN IAPP_RcvHandlerRawRRBVxWorks(IAPP_IN void *pCookie, IAPP_IN long Type,
                                     IAPP_IN M_BLK_ID pMblk,
                                     IAPP_IN LL_HDR_INFO *pLinkHdrInfo,
                                     IAPP_IN void *pSpare) {
  RTMP_IAPP *pCtrlBK = (RTMP_IAPP *)pSpare;

  if ((pCtrlBK != NULL) && (pMblk != NULL)) {
    /* handle the signal context, assoicate or reassociate or terminate */
    write(pCtrlBK->SocketRawRRB, (CHAR *)pMblk->mBlkHdr.mData,
          pMblk->mBlkHdr.mLen);

    /* free the Mblk */
    netMblkClChainFree(pMblk);
  } /* End of if */

  return TRUE;
} /* End of IAPP_RcvHandlerRawRRBVxWorks */
#endif // IAPP_OS_VXWORKS //
#endif // FT_KDP_FUNC_SOCK_COMM //

/*
========================================================================
Routine Description:
        Handle received IAPP UDP packets.

Arguments:
        *pCtrlBK	- IAPP control blcok
        *pPktBuf	- packet buffer
        *pCmdBuf	- used to issue command to WLAN driver
        *pRspBuf	- used to issue response to the peer

Return Value:
        None

Note:
========================================================================
*/
static VOID IAPP_RcvHandlerUdp(IAPP_IN RTMP_IAPP *pCtrlBK,
                               IAPP_IN UCHAR *pPktBuf, IAPP_IN UCHAR *pCmdBuf,
                               IAPP_IN UCHAR *pRspBuf) {
  struct sockaddr_in AddrPeer;
  RT_IAPP_HEADER *pIappHdr;
  INT32 SizeRcvMsg;
#if 0
	UCHAR *pIpHdr;
#endif
  socklen_t Length;
  INT32 if_idx = -1, idx;

  /* init */
  IAPP_MEM_ZERO(&AddrPeer, sizeof(AddrPeer));
  Length = sizeof(struct sockaddr_in);

  SizeRcvMsg = recvfrom(pCtrlBK->SocketUdpSend, pPktBuf, IAPP_MAX_RCV_PKT_SIZE,
                        0, (struct sockaddr *)&AddrPeer, &Length);

  /* handle the packet */
  if (SizeRcvMsg > 0) {
    DBGPRINT(RT_DEBUG_TRACE,
             ("iapp> Recvfrom UDP (len%d) successfully from %d.%d.%d.%d\n",
              SizeRcvMsg, IAPP_SHOW_IP(AddrPeer.sin_addr.s_addr)));

#ifndef IAPP_TEST
    if (AddrPeer.sin_addr.s_addr == pCtrlBK->AddrOwn.s_addr) {
      DBGPRINT(RT_DEBUG_TRACE, ("iapp> Skip packet from us!\n\n"));
      return; /* same IP source address */
    }         /* End of if */
#endif        // IAPP_TEST //

#if 0
		/* find the IP header */
		pIpHdr = pPktBuf + IAPP_IP_HEADER_OFFSET;

		/* skip non UDP packets */
		if (pIpHdr[IAPP_IP_PROTO_OFFSET] != IAPP_IP_PROTO_UDP)
			continue; /* the IP packet is not UDP */
		/* End of if */

		/*
			Because we dont bind RCV port to 3517, so we need to
			its check UDP dest port, must be 3517.
		*/
		if (*((UINT16 *)(pIpHdr+IAPP_UDP_DST_PORT_OFFSET)) !=
			ntohs(IAPP_UDP_PORT))
		{
			continue; /* not for us */
		} /* End of if */

		/* get IAPP frame body */
		pIappHdr = (RT_IAPP_HEADER *)(pPktBuf + IAPP_MAC_IP_UDP_LEN);
#endif

#ifdef FT_KDP_FUNC_PKT_ENCRYPT
    /* ioctl to decrypt */
    IAPP_IOCTL_TO_WLAN(pCtrlBK, RT_IOCTL_IAPP, pPktBuf, &SizeRcvMsg, 0,
                       RT_FT_DATA_DECRYPT);
#endif // FT_KDP_FUNC_PKT_ENCRYPT //

    /* get IAPP frame body */
    pIappHdr = (RT_IAPP_HEADER *)(pPktBuf);

    if (pIappHdr->Version != IAPP_VERSION) {
      DBGPRINT(RT_DEBUG_TRACE,
               ("iapp> IAPP version not match %d!\n", pIappHdr->Version));
      IAPP_HEX_DUMP("Wrong UDP Frame Content: ", pPktBuf, SizeRcvMsg);
      return; /* version not match */
    }         /* End of if */

    // IAPP_HEX_DUMP("UDP Frame Content: ", pPktBuf, SizeRcvMsg);

    /* handle the IAPP */
    DBGPRINT(RT_DEBUG_TRACE, ("iapp> IAPP SysCmd = %d\n", pIappHdr->Command));

    switch (pIappHdr->Command) {
    case IAPP_CMD_ADD_NOTIFY: {
      RT_IAPP_ADD_NOTIFY *pNotify;
      UCHAR *pBufMsg;
      UINT32 BufLen;
      POID_REQ OidReq;

      /* sanity check for frame length */
      pNotify = (RT_IAPP_ADD_NOTIFY *)pIappHdr;

      if (SWAP_16(pIappHdr->Length) != sizeof(RT_IAPP_ADD_NOTIFY)) {
        DBGPRINT(RT_DEBUG_TRACE,
                 ("iapp> Len 0x%x != ADD Notify Len %d!\n",
                  SWAP_16(pIappHdr->Length), sizeof(RT_IAPP_ADD_NOTIFY)));
        break;
      } /* End of if */

      /*
              Upon receipt of this service primitive the APME should
              determine if the STA indicated by the MAC Address is shown
              to be associated with the AP receiving the
              IAPP-ADD.indication, with a sequence number older than that
              in the IAPP ADD-notify packet.

              If so, this service primitive should cause the generation
              of an 802.11 MLME-DISASSOCIATE.request by the APME.

              If the sequence number received in the IAPP ADD-notify
              packet is older than that received from the STA when it
              associated with the AP receiving the IAPP ADD-notify packet,
              the APME should ignore the indicated association and issue
              an IAPPADD.request.

              Implementers of STA MAC entities are advised of the
              importance of continuing the sequential assignment of
              sequence numbers for outgoing MPDUs and MMPDUs throughout
              STA operation, as required by 802.11.
      */
      /* not yet implement */

      BufLen = sizeof(OID_REQ) + FT_IP_ADDRESS_SIZE + ETH_ALEN;
      IAPP_CMD_BUF_ALLOCATE(pCmdBuf, pBufMsg, BufLen);
      if (pBufMsg == NULL)
        break;

      if_idx = mt_iapp_find_ifidx_by_sta_mac(&pCtrlBK->SelfFtStaTable,
                                             pNotify->MacAddr);

      if (if_idx >= 0) {
        /* delete MAC Entry when receive a add-notify packet */
        OidReq = (POID_REQ)pBufMsg;
        OidReq->OID = (RT_SET_DEL_MAC_ENTRY | OID_GET_SET_TOGGLE);
        IAPP_MEM_MOVE(OidReq->Buf, pNotify->MacAddr, ETH_ALEN);
        OidReq->Len = ETH_ALEN;

        /*
                Note: RALINK AP driver delete the STATION MAC by MAC
                address, do NOT care which BSS index.
        */
        IAPP_MsgProcess(pCtrlBK, IAPP_SET_OID_REQ, pBufMsg,
                        (sizeof(INT32) + sizeof(INT32) + ETH_ALEN), if_idx);
      }

      DBGPRINT(RT_DEBUG_TRACE,
               ("iapp> Receive IAPP_CMD_ADD_NOTIFY for "
                "%02x:%02x:%02x:%02x:%02x:%02x (size = %d)\n",
                pNotify->MacAddr[0], pNotify->MacAddr[1], pNotify->MacAddr[2],
                pNotify->MacAddr[3], pNotify->MacAddr[4], pNotify->MacAddr[5],
                SizeRcvMsg));

      /* command to notify that this is for 11r station */
      if (pNotify->Rsvd & FT_KDP_ADD_NOTIFY_RSVD_11R_SUPPORT) {
        OidReq = (POID_REQ)pBufMsg;
        OidReq->OID = (RT_SET_FT_STATION_NOTIFY | OID_GET_SET_TOGGLE);

        /* peer IP address */
        IAPP_MEM_MOVE(OidReq->Buf, &(AddrPeer.sin_addr.s_addr),
                      FT_IP_ADDRESS_SIZE);

        /* station MAC */
        IAPP_MEM_MOVE(OidReq->Buf + FT_IP_ADDRESS_SIZE, pNotify->MacAddr,
                      ETH_ALEN);

        OidReq->Len = FT_IP_ADDRESS_SIZE + ETH_ALEN;

        /*
                Send notify to all 11r interface.
        */
        for (idx = 0; idx < pCtrlBK->IfNameWlanCount; idx++) {
          IAPP_MsgProcess(
              pCtrlBK, IAPP_SET_OID_REQ, pBufMsg,
              (sizeof(INT32) + sizeof(INT32) + FT_IP_ADDRESS_SIZE + ETH_ALEN),
              idx);
        }

#ifndef FT_KDP_FUNC_SOCK_COMM
        /*
                Note: in VxWorks, we can not send any signal to same task
                which issues a ioctl path.

                So we poll the event automatically.
        */
        /* receive event */
        IAPP_USR2Handle(0, if_idx);
#endif  // FT_KDP_FUNC_SOCK_COMM //
      } /* End of if */
    }   /* IAPP_CMD_ADD_NOTIFY */
    break;

#ifdef IAPP_EVENT_LOG
    case IAPP_CMD_SECURITY_MONITOR: {
      PRT_IAPP_SECURITY_MONITOR SM_p;

      SM_p = (PRT_IAPP_SECURITY_MONITOR)pIappHdr;

      DBGPRINT(RT_DEBUG_TRACE, ("iapp> pkt = IAPP_CMD_SECURITY_MONITOR\n"));

      IAPP_EventLogHandle(&SM_p->EvtTab);
    } /* IAPP_CMD_SECURITY_MONITOR */
    break;
#endif // IAPP_EVENT_LOG //

    case IAPP_CMD_MOVE_NOTIFY:
      IAPP_RcvHandlerMoveReq(pCtrlBK, (UCHAR *)pIappHdr,
                             AddrPeer.sin_addr.s_addr, pCmdBuf, pRspBuf);
      break;

    case IAPP_CMD_FT_SEND_SECURITY_BLOCK:
      IAPP_RcvHandlerSSB(pCtrlBK, (UCHAR *)pIappHdr, AddrPeer.sin_addr.s_addr,
                         pCmdBuf);
      break;

    case IAPP_CMD_INFO_BROADCAST:
      IAPP_RcvHandlerApInfor(pCtrlBK, IAPP_INFO_TYPE_BC, (UCHAR *)pIappHdr,
                             AddrPeer.sin_addr.s_addr, pCmdBuf, if_idx);
      break;

    case IAPP_CMD_INFO_REQUEST:
      IAPP_RcvHandlerApInfor(pCtrlBK, IAPP_INFO_TYPE_REQ, (UCHAR *)pIappHdr,
                             AddrPeer.sin_addr.s_addr, pCmdBuf, if_idx);
      break;

    default:
      DBGPRINT(RT_DEBUG_TRACE,
               ("iapp> Unknown IAPP command %d!\n", pIappHdr->Command));
      break;
    } /* End of switch(pIappHdr->Command) */
  }   /* End of if (SizeRcvMsg > 0) */
} /* End of IAPP_RcvHandlerUdp */

/*
========================================================================
Routine Description:
        Print IAPP deamon usage.

Arguments:
        None

Return Value:
        None

Note:
========================================================================
*/
static VOID IAPP_Usage(VOID) {
  printf("\tUSAGE:\t\tmtkiappd <-e eth_if_name> <-w wireless_if_name>\n");
  printf("\t\t\t\t<-k security_key> <-d debug level>\n");
  printf("\tDefault:\tmtkiappd -e br-lan -w ra0 -k zpxrjs9uo2kvbuqo -d 3\n");
} /* End of IAPP_Usage */

/*
========================================================================
Routine Description:
        Handle driver commands from RALINK AP.

Arguments:
        Sig		- no use

Return Value:
        None

Note:
        When a station associates or re-associates to RALINK AP, RALINK AP will
        use kill_proc() to send SIGUSR2 signal to us.

        Then we should to get the Data content from RALINK AP.
========================================================================
*/
static VOID IAPP_USR2Handle(IAPP_IN INT32 Sig) {
#ifndef FT_KDP_FUNC_SOCK_COMM
  RT_SIGNAL_STRUC *pSigBuf;
  INT32 DataLen;
  INT32 if_idx = 0;

  /* get signal context from AP driver */
  DataLen = sizeof(RT_SIGNAL_STRUC);
  IAPP_MEM_ALLOC(pSigBuf, DataLen);
  if (pSigBuf == NULL) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Allocate signal buffer fail!\n"));
    return;
  } /* End of if */

  IAPP_IOCTL_TO_WLAN(&IAPP_Ctrl_Block, RT_IOCTL_IAPP, pSigBuf, &DataLen, if_idx,
                     RT_QUERY_SIGNAL_CONTEXT);

  DBGPRINT(RT_DEBUG_TRACE, ("iapp> Receive a signal (Len = %d)!\n", DataLen));

  /* pass event to raw drv socket */
  if (DataLen > 0)
    write(pCtrlBK->PipeRawDrv[FT_KDP_PIPE_ID_WRITE], pSigBuf, DataLen);
  /* End of if */

  IAPP_MEM_FREE(pSigBuf);
#endif // FT_KDP_FUNC_SOCK_COMM //
} /* End of IAPP_USR2Handle */

/*
========================================================================
Routine Description:
        Handle termination signal.  And we will close IAPP deamon at main().

Arguments:
        Sig		- no use

Return Value:
        None

Note:
        The APME should terminate operation of the local BSS, including
        disassociation of any associated STAs and ceasing of beacon
transmissions, prior to terminating IAPP operation.

        The UDP and TCP ports for the IAPP should be closed and the IAPP entity
        should cease operations.
========================================================================
*/
static VOID IAPP_TerminateHandle(IAPP_IN INT32 Sig) {
  /* close all used sockets */
  IAPP_SocketClose(&IAPP_Ctrl_Block);
  IAPP_Ctrl_Block.FlgIsTerminated = 1;
} /* End of IAPP_TerminateHandle */

/*
========================================================================
Routine Description:
        Main task.

Arguments:
        *pContext	- IAPP control block

Return Value:
        None

Note:
========================================================================
*/
VOID IAPP_Task(IAPP_IN VOID *pContext) {
  // INT idx = 0;
  RTMP_IAPP *pCtrlBK = (RTMP_IAPP *)pContext;
  pid_t PidAuth;
//#ifdef IAPP_OS_LINUX
#if 0
	CHAR RouteAddCmd[64];
	CHAR RouteDelCmd[64];
#endif

  DBGPRINT(RT_DEBUG_TRACE,
           ("iapp> (ver.%s) task start...\n", IAPP_DAEMON_VERSION));

  /* here is the child background process */
  if (IAPP_DSIfInfoGet(pCtrlBK) != TRUE) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Get interfce information failed\n"));
    goto label_err;
  } /* End of if */

  /* init IPC message queue with 8021X deamon (obsolete) */
  if (IAPP_IPC_MSG_Init(pCtrlBK) != TRUE) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Setup message failed\n"));
    goto label_err;
  } /* End of if */

//#ifdef IAPP_OS_LINUX
#if 0
    /* add multicast route path, use SysCmd 'route' to show */
    /*
		Destination  Gateway  Genmask          Flags  Metric  Ref  Use Iface
		224.0.1.178  *        255.255.255.255  UH     0       0    0   br-lan
	*/
	sprintf(RouteAddCmd, "route add %s %s\n",
			IAPP_MULTICAST_ADDR, pCtrlBK->IfNameEth);

	sprintf(RouteDelCmd, "route del %s %s\n",
			IAPP_MULTICAST_ADDR, pCtrlBK->IfNameEth);

	system(RouteAddCmd);
#endif

  /* init RV/TX Sockets */
  /* setup the message queue to be synchronous */
  IAPP_MsgProcess(pCtrlBK, IAPP_OPEN_SERVICE_REQ, NULL, 0, 0);

  pCtrlBK->FlgIsRcvRunning = FALSE;
  if (IAPP_SocketOpen(pCtrlBK) == FALSE) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Open Socket failed\n"));
//#ifdef IAPP_OS_LINUX
#if 0
		system(RouteDelCmd);
#endif
    goto label_err;
  } /* End of if */

  if (mt_iapp_get_wifi_iface_mac(pCtrlBK) == FALSE) {
    goto label_err;
  }

  /* init signal functions, driver will use SIGUSR1/2 to inform us */
  signal(SIGINT, IAPP_TerminateHandle);
  signal(SIGTERM, IAPP_TerminateHandle);

  /*
          Keep IAPP_USR2Handle() even you do not enable FT_KDP_FUNC_SOCK_COMM;
          Or when driver send a signal SIGUSR2 and we have not registered it,
          IAPP daemon will be closed by kernel automatically.
  */
  signal(SIGUSR2, IAPP_USR2Handle); /* handle message from AP driver */

  /* when RALINK AP (re)start up, move the line to 8021x deamon */
  /*	signal(SIGUSR1, IAPP_USR1_Handle); */

  /* set our PID to RALINK WLAN driver so WLAN driver can send Data to us */
#ifdef IAPP_OS_LINUX
  PidAuth = getpid();
#endif // IAPP_OS_LINUX //
#ifdef IAPP_OS_VXWORKS
  PidAuth = pCtrlBK->PID;
#endif // IAPP_OS_VXWORKS //

  pCtrlBK->PID = PidAuth;

  DBGPRINT(RT_DEBUG_TRACE, ("iapp> Process ID = 0x%x (%d %d)\n", PidAuth,
                            IAPP_MemAllocNum, IAPP_MemFreeNum));

  mt_iapp_set_daemon_information(pCtrlBK, &PidAuth);

  IAPP_PID_Backup(PidAuth);

  /* start IAPP function (while FlgIsLoop in the function) */
  IAPP_Start(pCtrlBK);

  /* will not be here except terminate signal */
//#ifdef IAPP_OS_LINUX
#if 0
	system(RouteDelCmd);
#endif

  DBGPRINT(RT_DEBUG_TRACE, ("iapp> IAPP_Task ends (%d, %d)!\n",
                            IAPP_MemAllocNum, IAPP_MemFreeNum));

label_err:
  return;
} /* End of IAPP_Task */

/*
========================================================================
Routine Description:
        User space main function.

Arguments:
        Argc		- argument number
        *pArgv[]	- arguments

Return Value:
        0			- exit daemon
        -1			- fork fail

Note:
========================================================================
*/
#ifdef IAPP_OS_LINUX
INT32 main(INT32 Argc, CHAR *pArgv[])
#endif // IAPP_OS_LINUX //
#ifdef IAPP_OS_VXWORKS
    STATUS IAPP_Init(INT32 Argc, CHAR *pArgv[])
#endif // IAPP_OS_VXWORKS //
{
  RTMP_IAPP *pCtrlBK = &IAPP_Ctrl_Block;
#ifdef IAPP_OS_LINUX
  pid_t PID;
#endif // IAPP_OS_LINUX //

  /* init */
  IAPP_MEM_ZERO(pCtrlBK, sizeof(RTMP_IAPP));

  /* kill old IAPP daemon if exists */
  IAPP_PID_Kill();

  /* parse arguments from SysCmd line */
  if (IAPP_ArgumentParse(pCtrlBK, Argc, pArgv) == FALSE)
    return 0;
    /* End of if */

#ifdef IAPP_OS_LINUX
  IAPP_Task((VOID *)pCtrlBK);
#endif

#ifdef IAPP_OS_VXWORKS
  pCtrlBK->PID = taskSpawn("tIappFt", 100, 0, 5000, (FUNCPTR)IAPP_Task,
                           (INT32)pCtrlBK, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  if (pCtrlBK->PID == ERROR) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Task  spawn failed\n"));
    goto label_err;
  } /* End of if */
#endif

  DBGPRINT(RT_DEBUG_TRACE, ("iapp> Exit daemon!\n"));
  return 0;

label_err:
#ifdef IAPP_OS_LINUX
  exit(-1);
#endif
  return 0;
} /* End of main */

/*
========================================================================
Routine Description:
        Send the IAPP move notify frame.

Arguments:
        *pCtrlBK	- IAPP control blcok
        *pPkt		- the UDP packet
        PktLen		- the packet size
        PeerIP		- the IP of peer device
        FlgUsingUdpWhenNoIP - TRUE: use UDP broadcast to send when PeerIP == 0
        *pRspBuf	- used to issue response to the peer

Return Value:
        TRUE		- send successfully
        FAIL		- send fail

Note:
        If PeerIP is 0, the func will use UDP broadcast frame.
========================================================================
*/
static BOOLEAN IAPP_TCP_PacketSend(IAPP_IN RTMP_IAPP *pCtrlBK,
                                   /*	IAPP_IN		UCHAR *WifiMAC,*/
                                   IAPP_IN UCHAR *pPkt, IAPP_IN UINT32 PktLen,
                                   IAPP_IN UINT32 PeerIP,
                                   IAPP_IN BOOLEAN FlgUsingUdpWhenNoIP,
                                   IAPP_IN UCHAR *pRspBuf,
                                   IAPP_IN INT32 if_idx) {
  INT32 SocketPeer;
  struct sockaddr_in AddrSockConn;
  UCHAR *pBufEncrypt;
  BOOLEAN FuncStatus;

  /* sanity check */
  if ((FlgUsingUdpWhenNoIP == FALSE) && (PeerIP == 0))
    return FALSE;
  /* End of if */

  /* init */
  DBGPRINT(RT_DEBUG_TRACE, ("iapp> IAPP_TCP_PacketSend %x\n", PeerIP));
  FuncStatus = FALSE;

  if (PeerIP != 0) {
    /* use TCP to send the packet */
    SocketPeer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (SocketPeer < 0) {
      DBGPRINT(RT_DEBUG_TRACE, ("iapp> Open TCP socket fail!\n"));
      return FALSE;
    } /* End of if */

    IAPP_MEM_ZERO(&AddrSockConn, sizeof(AddrSockConn));
    AddrSockConn.sin_family = AF_INET;
    AddrSockConn.sin_addr.s_addr = PeerIP;
    AddrSockConn.sin_port = htons(IAPP_TCP_PORT);

    if (connect(SocketPeer, (struct sockaddr *)&AddrSockConn,
                sizeof(AddrSockConn)) < 0) {
      DBGPRINT(RT_DEBUG_ERROR, ("iapp> Connect socket failed %d.%d.%d.%d!\n",
                                IAPP_SHOW_IP(AddrSockConn.sin_addr.s_addr)));
      goto label_fail;
    } /* End of if */
  }   /* End of if */

  IAPP_RSP_BUF_ALLOCATE(pRspBuf, pBufEncrypt,
                        PktLen + IAPP_SECURITY_EXTEND_LEN);
  if (pBufEncrypt == NULL) {
    if (PeerIP != 0)
      close(SocketPeer);
    /* End of if */
    return FuncStatus;
  } /* End of if */
  IAPP_MEM_MOVE(pBufEncrypt, pPkt, PktLen);

  if (PeerIP != 0) {
#ifdef FT_KDP_FUNC_PKT_ENCRYPT
    /* ioctl to encrypt */
    IAPP_IOCTL_TO_WLAN(pCtrlBK, RT_IOCTL_IAPP, pBufEncrypt, &PktLen, if_idx,
                       RT_FT_DATA_ENCRYPT);
#endif // FT_KDP_FUNC_PKT_ENCRYPT //

    IAPP_HEX_DUMP("Send Content: ", pPkt, PktLen); // snowpin test

    if (send(SocketPeer, pBufEncrypt, PktLen, 0) < 0) {
      DBGPRINT(RT_DEBUG_ERROR, ("iapp> Send socket failed %d.%d.%d.%d!\n",
                                IAPP_SHOW_IP(AddrSockConn.sin_addr.s_addr)));
      goto label_fail;
    } /* End of if */
  } else {
    /* use broadcast UDP packet but UDP can not be guaranted, no retry */
    IAPP_UDP_PACKET_SEND(pCtrlBK, pBufEncrypt, PktLen, pRspBuf, if_idx);
  } /* End of if */

  FuncStatus = TRUE;

label_fail:
  if (PeerIP != 0)
    close(SocketPeer);
  /* End of if */
  return FuncStatus;
} /* End of IAPP_TCP_PacketSend */

/*
========================================================================
Routine Description:
        Send the IAPP send security block frame.

Arguments:
        *pCtrlBK	- IAPP control blcok
        *pEvtHdr	- event header
        *pEvt		- event
        *pRspBuf	- used to issue response to the peer

Return Value:
        None

Note:
        If PeerIP is 0, the func will use UDP broadcast frame.
========================================================================
*/
static VOID FT_KDP_SecurityBlockSend(IAPP_IN RTMP_IAPP *pCtrlBK,
                                     IAPP_IN FT_KDP_EVT_HEADER *pEvtHdr,
                                     IAPP_IN VOID *pEvt, IAPP_IN UCHAR *pRspBuf,
                                     IAPP_IN UCHAR *WifiMAC,
                                     IAPP_IN INT32 if_idx) {
  INT32 SocketPeer;
  struct sockaddr_in AddrSockConn;
  UCHAR *pBufFrame;
  RT_IAPP_SEND_SECURITY_BLOCK *pIappSendSB;
  UINT32 PktLen, buf_len = 0;

  /* init */
  DBGPRINT(RT_DEBUG_TRACE, ("iapp> FT_KDP_SecurityBlockSend to %d.%d.%d.%d\n\n",
                            IAPP_SHOW_IP_HTONL(pEvtHdr->PeerIpAddr)));

  SocketPeer = 0;
  pBufFrame = NULL;

  /* sanity check */
  if (pEvtHdr->PeerIpAddr != 0) {
    /* use TCP to send the packet */
    SocketPeer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (SocketPeer < 0) {
      DBGPRINT(RT_DEBUG_TRACE, ("iapp> Open TCP socket fail!\n"));
      return;
    } /* End of if */

    IAPP_MEM_ZERO(&AddrSockConn, sizeof(AddrSockConn));
    AddrSockConn.sin_family = AF_INET;
#ifndef IAPP_TEST
    AddrSockConn.sin_addr.s_addr = pEvtHdr->PeerIpAddr;
#else
    AddrSockConn.sin_addr.s_addr = inet_addr("127.0.0.1"); /* test use */
#endif
    AddrSockConn.sin_port = htons(IAPP_TCP_PORT);

    if (connect(SocketPeer, (struct sockaddr *)&AddrSockConn,
                sizeof(AddrSockConn)) < 0) {
      DBGPRINT(RT_DEBUG_ERROR, ("iapp> Connect socket failed %d.%d.%d.%d!\n",
                                IAPP_SHOW_IP(AddrSockConn.sin_addr.s_addr)));
      goto label_fail;
    } /* End of if */
  }   /* End of if */

  /* init frame buffer */
  buf_len = sizeof(RT_IAPP_SEND_SECURITY_BLOCK) + pEvtHdr->EventLen +
            ETH_ALEN; /* ETH_ALEN is the length of WIFI Interface MAC. */

  IAPP_RSP_BUF_ALLOCATE(pRspBuf, pBufFrame, buf_len);
  if (pBufFrame == NULL)
    goto label_fail;
  /* End of if */

  IAPP_PKT_ZERO(pBufFrame, buf_len);

  /* init the Security-Block-Send frame */
  pIappSendSB = (RT_IAPP_SEND_SECURITY_BLOCK *)pBufFrame;
  pIappSendSB->IappHeader.Version = 0;
  pIappSendSB->IappHeader.Command = IAPP_CMD_FT_SEND_SECURITY_BLOCK;
  pIappSendSB->IappHeader.Identifier = SWAP_16(IAPP_IDENTIFIER_GET(pCtrlBK));
  PktLen = sizeof(RT_IAPP_SEND_SECURITY_BLOCK) + pEvtHdr->EventLen;
  pIappSendSB->IappHeader.Length = SWAP_16(PktLen);

  pIappSendSB->Length = pEvtHdr->EventLen;
  IAPP_MEM_MOVE(pIappSendSB->SB, pEvt, pEvtHdr->EventLen);
  IAPP_MEM_MOVE(pIappSendSB->SB + pEvtHdr->EventLen, WifiMAC, ETH_ALEN);

  /* send out the frame */
  if (pEvtHdr->PeerIpAddr != 0) {
#ifdef FT_KDP_FUNC_PKT_ENCRYPT
    /* ioctl to encrypt */
    IAPP_IOCTL_TO_WLAN(pCtrlBK, RT_IOCTL_IAPP, pBufFrame, &buf_len, if_idx,
                       RT_FT_DATA_ENCRYPT);
#endif // FT_KDP_FUNC_PKT_ENCRYPT //

    if (send(SocketPeer, pBufFrame, buf_len, 0) < 0) {
      DBGPRINT(RT_DEBUG_ERROR, ("iapp> Send socket failed %d.%d.%d.%d!\n",
                                IAPP_SHOW_IP(AddrSockConn.sin_addr.s_addr)));
      goto label_fail;
    } /* End of if */
  } else {
    /* use broadcast UDP packet but UDP can not be guaranted, no retry */
    IAPP_UDP_PACKET_SEND(pCtrlBK, pBufFrame, buf_len, pRspBuf, if_idx);
  } /* End of if */

label_fail:
  if (SocketPeer > 0)
    close(SocketPeer);
  /* End of if */
  return;
} /* End of FT_KDP_SecurityBlockSend */

/*
========================================================================
Routine Description:
        Send the IAPP ack security block frame.

Arguments:
        *pCtrlBK	- IAPP control blcok
        *pEvtHdr	- event header
        *pEvt		- event

Return Value:
        None

Note:
========================================================================
*/
static VOID FT_KDP_SecurityBlockAck(IAPP_IN RTMP_IAPP *pCtrlBK,
                                    IAPP_IN FT_KDP_EVT_HEADER *pEvtHdr,
                                    IAPP_IN VOID *pEvt, IAPP_IN UCHAR *pRspBuf,
                                    IAPP_IN INT32 if_idx) {
  INT32 SocketPeer;
  struct sockaddr_in AddrSockConn;
  UCHAR *pBufFrame;
  RT_IAPP_SEND_SECURITY_BLOCK *pIappSendSB;
  UINT32 PktLen;

  /* init */
  DBGPRINT(RT_DEBUG_TRACE, ("iapp> FT_KDP_SecurityBlockAck\n"));

  SocketPeer = 0;
  pBufFrame = NULL;

  /* open TCP socket to the peer */
  SocketPeer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (SocketPeer < 0) {
    DBGPRINT(RT_DEBUG_TRACE, ("iapp> Open TCP socket fail!\n"));
    return;
  } /* End of if */

  IAPP_MEM_ZERO(&AddrSockConn, sizeof(AddrSockConn));
  AddrSockConn.sin_family = AF_INET;
#ifndef IAPP_TEST
  AddrSockConn.sin_addr.s_addr = pEvtHdr->PeerIpAddr;
#else
  AddrSockConn.sin_addr.s_addr = inet_addr("127.0.0.1");   /* test use */
#endif
  AddrSockConn.sin_port = htons(IAPP_TCP_PORT);

  if (connect(SocketPeer, (struct sockaddr *)&AddrSockConn,
              sizeof(AddrSockConn)) < 0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Connect socket failed %d.%d.%d.%d!\n",
                              IAPP_SHOW_IP(AddrSockConn.sin_addr.s_addr)));
    goto label_fail;
  } /* End of if */

  /* init frame buffer */
  PktLen = sizeof(RT_IAPP_SEND_SECURITY_BLOCK) + pEvtHdr->EventLen;

  IAPP_RSP_BUF_ALLOCATE(pRspBuf, pBufFrame, PktLen);
  if (pBufFrame == NULL)
    goto label_fail;
  /* End of if */
  IAPP_PKT_ZERO(pBufFrame, PktLen);

  /* init the Security-Block-Ack frame */
  pIappSendSB = (RT_IAPP_SEND_SECURITY_BLOCK *)pBufFrame;
  pIappSendSB->IappHeader.Version = 0;
  pIappSendSB->IappHeader.Command = IAPP_CMD_FT_ACK_SECURITY_BLOCK;
  pIappSendSB->IappHeader.Identifier = SWAP_16(IAPP_IDENTIFIER_GET(pCtrlBK));

  pIappSendSB->IappHeader.Length = SWAP_16(PktLen);

  pIappSendSB->Length = pEvtHdr->EventLen;
  IAPP_MEM_MOVE(pIappSendSB->SB, pEvt, pEvtHdr->EventLen);

#ifdef FT_KDP_FUNC_PKT_ENCRYPT
  /* ioctl to encrypt */
  IAPP_IOCTL_TO_WLAN(pCtrlBK, RT_IOCTL_IAPP, pBufFrame, &PktLen, if_idx,
                     RT_FT_DATA_ENCRYPT);
#endif // FT_KDP_FUNC_PKT_ENCRYPT //

  /* send out the frame */
  if (send(SocketPeer, pBufFrame, PktLen, 0) < 0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Send socket failed %d.%d.%d.%d!\n",
                              IAPP_SHOW_IP(AddrSockConn.sin_addr.s_addr)));
    goto label_fail;
  } /* End of if */

label_fail:
  if (SocketPeer > 0)
    close(SocketPeer);
  /* End of if */
  return;
} /* End of FT_KDP_SecurityBlockAck */

/*
========================================================================
Routine Description:
        Send the Information Request frame.

Arguments:
        *pCtrlBK	- IAPP control blcok
        *pEvtHdr	- event header
        *pEvt		- event
        *pRspBuf	- used to issue response to the peer

Return Value:
        None

Note:
========================================================================
*/
static VOID FT_KDP_InformationRequestSend(IAPP_IN RTMP_IAPP *pCtrlBK,
                                          IAPP_IN FT_KDP_EVT_HEADER *pEvtHdr,
                                          IAPP_IN VOID *pEvt,
                                          IAPP_IN UCHAR *pRspBuf,
                                          IAPP_IN INT32 if_idx) {
  UCHAR *pBufFrame;
  RT_IAPP_INFORMATION *pIappInfor;
  UINT32 PktLen;

  /* init */
  DBGPRINT(RT_DEBUG_TRACE, ("iapp> FT_KDP_InformationRequestSend\n"));

  pBufFrame = NULL;

  /* init frame buffer */
  PktLen = sizeof(RT_IAPP_INFORMATION) + pEvtHdr->EventLen;

  IAPP_RSP_BUF_ALLOCATE(pRspBuf, pBufFrame, PktLen);
  if (pBufFrame == NULL)
    return;
  /* End of if */

  IAPP_PKT_ZERO(pBufFrame, PktLen);
  pIappInfor = (RT_IAPP_INFORMATION *)pBufFrame;

  /* init the AP Information frame */
  pIappInfor->IappHeader.Version = 0;
  pIappInfor->IappHeader.Command = IAPP_CMD_INFO_REQUEST;
  pIappInfor->IappHeader.Identifier = SWAP_16(IAPP_IDENTIFIER_GET(pCtrlBK));

  pIappInfor->IappHeader.Length = SWAP_16(PktLen);

  pIappInfor->Length = pEvtHdr->EventLen;
  IAPP_MEM_MOVE(pIappInfor->IB, pEvt, pEvtHdr->EventLen);

  /* use broadcast UDP packet but UDP can not be guaranted, no retry */
  IAPP_UDP_PACKET_SEND(pCtrlBK, pBufFrame, PktLen, pRspBuf, if_idx);
} /* End of FT_KDP_InformationRequestSend */

/*
========================================================================
Routine Description:
        Send the Information Response frame.

Arguments:
        *pCtrlBK	- IAPP control blcok
        *pEvtHdr	- event header
        *pEvt		- event
        *pRspBuf	- used to issue response to the peer

Return Value:
        None

Note:
========================================================================
*/
static VOID FT_KDP_InformationResponseSend(IAPP_IN RTMP_IAPP *pCtrlBK,
                                           IAPP_IN FT_KDP_EVT_HEADER *pEvtHdr,
                                           IAPP_IN VOID *pEvt,
                                           IAPP_IN UCHAR *pRspBuf,
                                           IAPP_IN INT32 if_idx) {
  INT32 SocketPeer;
  struct sockaddr_in AddrSockConn;
  UCHAR *pBufFrame;
  RT_IAPP_INFORMATION *pIappInfor;
  UINT32 PktLen;

  /* init */
  DBGPRINT(RT_DEBUG_TRACE,
           ("iapp> FT_KDP_InformationResponseSend to %d.%d.%d.%d\n",
            IAPP_SHOW_IP_HTONL(pEvtHdr->PeerIpAddr)));

  if (pEvtHdr->PeerIpAddr == 0)
    return;
  /* End of if */

  SocketPeer = 0;
  pBufFrame = NULL;

  /* open TCP socket to the peer */
  SocketPeer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (SocketPeer < 0) {
    DBGPRINT(RT_DEBUG_TRACE, ("iapp> Open TCP socket fail!\n"));
    return;
  } /* End of if */

  IAPP_MEM_ZERO(&AddrSockConn, sizeof(AddrSockConn));
  AddrSockConn.sin_family = AF_INET;
#ifndef IAPP_TEST
  AddrSockConn.sin_addr.s_addr = pEvtHdr->PeerIpAddr;
#else
  AddrSockConn.sin_addr.s_addr = inet_addr("127.0.0.1");   /* test use */
#endif
  AddrSockConn.sin_port = htons(IAPP_TCP_PORT);

  if (connect(SocketPeer, (struct sockaddr *)&AddrSockConn,
              sizeof(AddrSockConn)) < 0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Connect socket failed %d.%d.%d.%d!\n",
                              IAPP_SHOW_IP(AddrSockConn.sin_addr.s_addr)));
    goto label_fail;
  } /* End of if */

  /* init frame buffer */
  PktLen = sizeof(RT_IAPP_INFORMATION) + pEvtHdr->EventLen;

  IAPP_RSP_BUF_ALLOCATE(pRspBuf, pBufFrame, PktLen);
  if (pBufFrame == NULL)
    goto label_fail;
  /* End of if */
  IAPP_PKT_ZERO(pBufFrame, PktLen);

  /* init the Security-Block-Ack frame */
  pIappInfor = (RT_IAPP_INFORMATION *)pBufFrame;
  pIappInfor->IappHeader.Version = 0;
  pIappInfor->IappHeader.Command = IAPP_CMD_INFO_RESPONSE;
  pIappInfor->IappHeader.Identifier = SWAP_16(IAPP_IDENTIFIER_GET(pCtrlBK));

  pIappInfor->IappHeader.Length = SWAP_16(PktLen);

  pIappInfor->Length = pEvtHdr->EventLen;
  IAPP_MEM_MOVE(pIappInfor->IB, pEvt, pEvtHdr->EventLen);

#ifdef FT_KDP_FUNC_PKT_ENCRYPT
  /* ioctl to encrypt */
  IAPP_IOCTL_TO_WLAN(pCtrlBK, RT_IOCTL_IAPP, pBufFrame, &PktLen, if_idx,
                     RT_FT_DATA_ENCRYPT);
#endif // FT_KDP_FUNC_PKT_ENCRYPT //

  /* send out the frame */
  if (send(SocketPeer, pBufFrame, PktLen, 0) < 0) {
    DBGPRINT(RT_DEBUG_ERROR, ("iapp> Send socket failed %d.%d.%d.%d!\n",
                              IAPP_SHOW_IP(AddrSockConn.sin_addr.s_addr)));
    goto label_fail;
  } /* End of if */

label_fail:
  if (SocketPeer > 0)
    close(SocketPeer);
  /* End of if */
  return;
} /* End of FT_KDP_InformationResponseSend */

/*
========================================================================
Routine Description:
        Send the Information Report frame.

Arguments:
        *pCtrlBK	- IAPP control blcok
        *pEvtHdr	- event header
        *pEvt		- event
        *pRspBuf	- used to issue response to the peer

Return Value:
        None

Note:
========================================================================
*/
static VOID FT_KDP_InformationReportSend(IAPP_IN RTMP_IAPP *pCtrlBK,
                                         IAPP_IN FT_KDP_EVT_HEADER *pEvtHdr,
                                         IAPP_IN VOID *pEvt,
                                         IAPP_IN UCHAR *pRspBuf,
                                         IAPP_IN INT32 if_idx) {
  UCHAR *pBufFrame;
  RT_IAPP_INFORMATION *pIappInfor;
  UINT32 PktLen;

  /* init */
  DBGPRINT(RT_DEBUG_TRACE,
           ("iapp> FT_KDP_InformationReportSend to %d.%d.%d.%d\n",
            IAPP_SHOW_IP_HTONL(pEvtHdr->PeerIpAddr)));

  pBufFrame = NULL;

  /* init frame buffer */
  PktLen = sizeof(RT_IAPP_INFORMATION) + pEvtHdr->EventLen;

  IAPP_RSP_BUF_ALLOCATE(pRspBuf, pBufFrame, PktLen);
  if (pBufFrame == NULL)
    return;
  /* End of if */

  IAPP_PKT_ZERO(pBufFrame, PktLen);
  pIappInfor = (RT_IAPP_INFORMATION *)pBufFrame;

  /* init the AP Information frame */
  pIappInfor->IappHeader.Version = 0;
  pIappInfor->IappHeader.Command = IAPP_CMD_INFO_BROADCAST;
  pIappInfor->IappHeader.Identifier = SWAP_16(IAPP_IDENTIFIER_GET(pCtrlBK));

  pIappInfor->IappHeader.Length = SWAP_16(PktLen);

  pIappInfor->Length = pEvtHdr->EventLen;
  IAPP_MEM_MOVE(pIappInfor->IB, pEvt, pEvtHdr->EventLen);

  /* use broadcast UDP packet but UDP can not be guaranted, no retry */
  IAPP_UDP_PACKET_SEND(pCtrlBK, pBufFrame, PktLen, pRspBuf, if_idx);
} /* End of FT_KDP_InformationReportSend */

/*
========================================================================
Routine Description:
        Forward the FT Action frame to the peer.

Arguments:
        *pCtrlBK	- IAPP control blcok
        *pEvtHdr	- event header
        *pEvt		- event
        PacketType	- 0 for Remote Request, and to 1 for Remote Response
        *pMacDa		- DA MAC
        *pMacSa		- SA MAC
        *pRspBuf	- used to issue response to the peer

Return Value:
        None

Note:
========================================================================
*/
static VOID FT_RRB_ActionForward(IAPP_IN RTMP_IAPP *pCtrlBK,
                                 IAPP_IN FT_KDP_EVT_HEADER *pEvtHdr,
                                 IAPP_IN VOID *pEvt, IAPP_IN UINT16 PacketType,
                                 IAPP_IN UCHAR *pMacDa, IAPP_IN UCHAR *pMacSa,
                                 IAPP_IN UCHAR *pMacAp, IAPP_IN UCHAR *pRspBuf,
                                 IAPP_IN INT32 if_idx) {
  UCHAR *pBufFrame;
  FT_RRB_FRAME *pFrameRRB;
  INT32 Status;
  UINT32 PktLen;
  UINT32 EvtLen;

  /* init the update frame body */
  DBGPRINT(RT_DEBUG_TRACE, ("iapp> Forward a RRB packet (len = %d) to "
                            "%02x:%02x:%02x:%02x:%02x:%02x!\n",
                            pEvtHdr->EventLen, pMacDa[0], pMacDa[1], pMacDa[2],
                            pMacDa[3], pMacDa[4], pMacDa[5]));

  PktLen = sizeof(FT_RRB_FRAME) + pEvtHdr->EventLen;

  IAPP_RSP_BUF_ALLOCATE(pRspBuf, pBufFrame, PktLen);
  if (pBufFrame == NULL)
    return;
  /* End of if */
  IAPP_PKT_ZERO(pBufFrame, PktLen);

  pFrameRRB = (FT_RRB_FRAME *)pBufFrame;
  IAPP_MEM_MOVE(pFrameRRB->ETH.DA, pMacDa, sizeof(pFrameRRB->ETH.DA));
  IAPP_MEM_MOVE(pFrameRRB->ETH.SA, pMacSa, sizeof(pFrameRRB->ETH.SA));
  pFrameRRB->ETH.Len = htons(RRB_ETH_PRO);

  pFrameRRB->RemoteFrameType = FT_RRB_FRAME_TYPE;
  pFrameRRB->FTPacketType = PacketType;
  pFrameRRB->FTActionLength = pEvtHdr->EventLen;
  IAPP_MEM_MOVE(pFrameRRB->ApAddress, pMacAp, sizeof(pFrameRRB->ApAddress));

  IAPP_MEM_MOVE(pFrameRRB->FTActionFrame, pEvt, pEvtHdr->EventLen);

#ifdef FT_KDP_FUNC_PKT_ENCRYPT
  /* ioctl to encrypt */

  /* the address of pEvtHdr->EventLen is not 4B align */
  EvtLen = pEvtHdr->EventLen;

  IAPP_IOCTL_TO_WLAN(pCtrlBK, RT_IOCTL_IAPP, pFrameRRB->FTActionFrame, &EvtLen,
                     if_idx, RT_FT_DATA_ENCRYPT);

  /* reassign the packet length due to changed pEvtHdr->EventLen */
  PktLen = sizeof(FT_RRB_FRAME) + EvtLen;
#endif // FT_KDP_FUNC_PKT_ENCRYPT //

#ifdef IAPP_OS_LINUX
  {
    /* send the RRB frame */
    Status = send(pCtrlBK->SocketRawRRB, pBufFrame, PktLen, 0);
    if (Status < 0) {
      DBGPRINT(RT_DEBUG_ERROR, ("iapp> Send RRB packet failed %d!\n", Status));
      goto LabelFail;
    } /* End of if */
  }
#endif // IAPP_OS_LINUX //

#ifdef IAPP_OS_VXWORKS
  {
    M_BLK_ID pUpdatePkt;
    UINT32 IdIfNum;

    /* loop for eth0, eth1, eth2...... */
    for (IdIfNum = 0; IdIfNum < FT_KDP_BR_ETH_IF_NUM; IdIfNum++) {
      if ((pUpdatePkt = netTupleGet(_pNetDpool, PktLen, M_DONTWAIT, MT_DATA,
                                    TRUE)) == NULL) {
        DBGPRINT(RT_DEBUG_ERROR, ("iapp> iapp> Get packet buffer fail!\n"));
        goto LabelFail;
      } /* End of if */

      pUpdatePkt->mBlkHdr.mFlags |= M_PKTHDR;
      pUpdatePkt->m_len = PktLen;

      IAPP_MEM_MOVE(pUpdatePkt->m_data, pBufFrame, PktLen);

      if (muxSend(pCtrlBK->pRrbCookieTo[IdIfNum], pUpdatePkt) == ERROR) {
        DBGPRINT(RT_DEBUG_ERROR, ("iapp> Send RRB packet failed!\n"));
        netMblkClChainFree(pUpdatePkt);
        goto LabelFail;
      } /* End of if */
    }   /* End of for */
  }
#endif // IAPP_OS_VXWORKS //

  DBGPRINT(RT_DEBUG_TRACE, ("iapp> Send RRB packet OK!\n"));

LabelFail:
  return;
} /* End of FT_RRB_ActionForward */

/* End of rtmpiapp.c */
