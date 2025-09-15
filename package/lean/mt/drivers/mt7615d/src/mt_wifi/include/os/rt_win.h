/*


*/


#ifdef WIN_NDIS

#define NIC_TAG             ((ULONG)'0682')
#define NIC_DBG_STRING      ("**RT28xx**")

#define MlmeAllocateMemory(_pAd, _ppVA) NdisAllocateMemoryWithTag(_ppVA, MGMT_DMA_BUFFER_SIZE, NIC_TAG)
#define MlmeFreeMemory( _pVA)      NdisFreeMemory(_pVA, MGMT_DMA_BUFFER_SIZE, 0)


#define RTMP_INC_REF(_A)            NdisInterlockedIncrement(&(_A)->RefCount)
#define RTMP_DEC_REF(_A)            NdisInterlockedDecrement(&(_A)->RefCount); ASSERT(_A->RefCount >= 0)
#define RTMP_GET_REF(_A)            ((_A)->RefCount)

#define RTMP_INC_RCV_REF(_A)        ((_A)->RcvRefCount++)
#define RTMP_DEC_RCV_REF(_A)        ((_A)->RcvRefCount--)
#define RTMP_GET_RCV_REF(_A)        ((_A)->RcvRefCount)

/*#define RTMP_INC_SEND_REF(_A)       ((_A)->SendRefCount++) */
/*#define RTMP_DEC_SEND_REF(_A)       ((_A)->SendRefCount--) */
/*#define RTMP_GET_SEND_REF(_A)       ((_A)->SendRefCount) */

#define RTMP_OFFSET(field)          ((UINT)FIELD_OFFSET(RTMP_ADAPTER, field))
#define RTMP_SIZE(field)            sizeof(((PRTMP_ADAPTER)0)->field)

#define COMMON_CFG_OFFSET(field)    ((UINT)FIELD_OFFSET(RTMP_ADAPTER, CommonCfg) + (UINT)FIELD_OFFSET(COMMON_CONFIG, field))
#define STA_CFG_OFFSET(field)       ((UINT)FIELD_OFFSET(RTMP_ADAPTER, StaCfg) + (UINT)FIELD_OFFSET(STA_ADMIN_CONFIG, field))
#define AP_CFG_OFFSET(field)        ((UINT)FIELD_OFFSET(RTMP_ADAPTER, ApCfg) + (UINT)FIELD_OFFSET(AP_ADMIN_CONFIG, field))
#define COMMON_CFG_SIZE(field)      sizeof(((PCOMMON_CONFIG)0)->field)
#define STA_CFG_SIZE(field)         sizeof(((struct _STA_ADMIN_CONFIG *)0)->field)
#define AP_CFG_SIZE(field)          sizeof(((struct _AP_ADMIN_CONFIG *)0)->field)

#define INC_RING_INDEX(_idx, _RingSize)    \
{                                          \
    (_idx)++;                              \
    if ((_idx) >= (_RingSize)) _idx=0;     \
}

#if ME_98
/* */
/* Re-define NdisInitializeString. */
/* Since the DDK NdisInitializeString is not support on Win9X */
/* */
#define NdisInitializeString(Destination,Source)                                 \
{                                                                                \
    NDIS_PHYSICAL_ADDRESS phyaddr= {-1,-1};                                      \
    PNDIS_STRING _D = (Destination);                                             \
    UCHAR *_S = (Source);                                                        \
    WCHAR *_P;                                                                   \
    _D->Length = (strlen(_S)) * sizeof(WCHAR);                                   \
    _D->MaximumLength = _D->Length + sizeof(WCHAR);                              \
    NdisAllocateMemory((PVOID *)&(_D->Buffer), _D->MaximumLength, 0, phyaddr);   \
    _P = _D->Buffer;                                                             \
    while(*_S != '\0'){                                                          \
        *_P = (WCHAR)(*_S);                                                      \
        _S++;                                                                    \
        _P++;                                                                    \
    }                                                                            \
    *_P = UNICODE_NULL;                                                          \
}
#endif


/* */
/*  NDIS Version definitions */
/* */
#ifdef	NDIS51_MINIPORT
#define RTMP_NDIS_MAJOR_VERSION     5
#define RTMP_NDIS_MINOR_VERSION     1

#define BEACON_LOST_TIME            4000       /* 2048 msec = 2 sec */

#endif


/* */
/* NDIS version in use by the NIC driver. */
/* The high byte is the major version. The low byte is the minor version. */
/* */
#ifdef  NDIS51_MINIPORT
#define NIC_DRIVER_VERSION      0x0501
#else
#define NIC_DRIVER_VERSION      0x0500
#endif

/* */
/* NDIS media type, current is ethernet, change if native wireless supported */
/* */
#define NIC_MEDIA_TYPE          NdisMedium802_3
#define NIC_PCI_HDR_LENGTH      0xe2
#define NIC_MAX_PACKET_SIZE     2304
#define NIC_HEADER_SIZE         14
#define MAX_MAP_REGISTERS_NEEDED 32
#define MIN_MAP_REGISTERS_NEEDED 2   /*Todo: should consider fragment issue. */

/* */
/* interface type, we use PCI */
/* */
#define NIC_INTERFACE_TYPE      NdisInterfacePci
#define NIC_INTERRUPT_MODE      NdisInterruptLevelSensitive

/* */
/* buffer size passed in NdisMQueryAdapterResources */
/* We should only need three adapter resources (IO, interrupt and memory), */
/* Some devices get extra resources, so have room for 10 resources */
/*                    UF_SIZE   (sizeof(NDIS_RESOURCE_LIST) + (10*sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR))) */


#define NIC_RESOURCE_B/* */
/* IO space length */
/* */
#define NIC_MAP_IOSPACE_LENGTH  sizeof(CSR_STRUC)


/* */
/* MACRO for 32-bit PCI register read / write */
/* */
/* Usage : RTMP_IO_READ32( */
/*              PRTMP_ADAPTER pAd, */
/*              ULONG Register_Offset, */
/*              PULONG  pValue) */
/* */
/*         RTMP_IO_WRITE32( */
/*              PRTMP_ADAPTER pAd, */
/*              ULONG Register_Offset, */
/*              ULONG Value) */
/* */
#define RTMP_IO_READ32(_A, _R, _pV)												\
{																				\
	NdisReadRegisterUlong((PULONG)((_A)->CSRBaseAddress + (_R)), (_pV));		\
}
#define RTMP_IO_WRITE32(_A, _R, _V)												\
{																				\
	NdisWriteRegisterUlong((PULONG)((_A)->CSRBaseAddress + (_R)), (_V));		\
}
#define RTMP_IO_WRITE8(_A, _R, _V)												\
{																				\
	NdisWriteRegisterUchar((PUCHAR)((_A)->CSRBaseAddress + (_R)), (_V));		\
}
#define RTMP_IO_WRITE16(_A, _R, _V)												\
{																				\
	ULONG	Val;																\
	NdisWriteRegisterUshort((PSHORT)((_A)->CSRBaseAddress + (_R)), (_V));		\
}


/* NOTE: 2004-12-1 John */
/*   check DDK NDIS_PACKET data structure and find out only MiniportReservedEx[0..7] can be used by our driver without */
/*   ambiguity. Fields after pPacket->MiniportReservedEx[8] may be used by other wrapper layer thus crashes the driver */
/* */
#define RTMP_GET_PACKET_MR(_p)                  (&(_p)->MiniportReservedEx[0]) 

/* b0-b3 as User Priority */
#define RTMP_SET_PACKET_UP(_p, _prio)           ((_p)->MiniportReservedEx[sizeof(PVOID)] = ((_p)->MiniportReservedEx[sizeof(PVOID)] & 0xf0) | (_prio))
#define RTMP_GET_PACKET_UP(_p)                  ((_p)->MiniportReservedEx[sizeof(PVOID)] & 0x0f)

/* b4-b7 as fragment # */
#define RTMP_SET_PACKET_FRAGMENTS(_p, _num)     ((_p)->MiniportReservedEx[sizeof(PVOID)] = ((_p)->MiniportReservedEx[sizeof(PVOID)] & 0x0f) | (_num << 4))
#define RTMP_GET_PACKET_FRAGMENTS(_p)           (((_p)->MiniportReservedEx[sizeof(PVOID)] & 0xf0) >> 4)

/* 0x0 ~0x7f: TX to AP's own BSS which has the specified AID. if AID>127, set bit 7 in RTMP_SET_PACKET_EMACTAB too. */
/*(this value also as MAC(on-chip WCID) table index) */
/* 0x80~0xff: TX to a WDS link. b0~6: WDS index */
#define RTMP_SET_PACKET_WCID(_p, _wcid)        ((_p)->MiniportReservedEx[sizeof(PVOID) + 1] = _wcid)
#define RTMP_GET_PACKET_WCID(_p)                 ((_p)->MiniportReservedEx[sizeof(PVOID) + 1])

/* 0xff: PKTSRC_NDIS, others: local TX buffer index. This value affects how to a packet */
#define RTMP_SET_PACKET_SOURCE(_p, _pktsrc)     ((_p)->MiniportReservedEx[sizeof(PVOID) + 2] = _pktsrc)
#define RTMP_GET_PACKET_SOURCE(_p)              ((_p)->MiniportReservedEx[sizeof(PVOID) + 2])

/* b0~2: RTS/CTS-to-self protection method */
#define RTMP_SET_PACKET_RTS(_p, _num)           ((_p)->MiniportReservedEx[sizeof(PVOID) + 3] = ((_p)->MiniportReservedEx[sizeof(PVOID) + 3] & 0xf8) | (_num))
#define RTMP_GET_PACKET_RTS(_p)                 ((_p)->MiniportReservedEx[sizeof(PVOID) + 3] & 0x07)
/* b7: see RTMP_S(G)ET_PACKET_EMACTAB */

/* b3~7: TX rate index */
#define RTMP_SET_PACKET_TXRATE(_p, _rate)       ((_p)->MiniportReservedEx[sizeof(PVOID) + 3] = ((_p)->MiniportReservedEx[sizeof(PVOID) + 3] & 0x07) | (_rate << 3))
#define RTMP_GET_PACKET_TXRATE(_p)              (((_p)->MiniportReservedEx[sizeof(PVOID) + 3] & 0xf8) >> 3)


/* value recorded inside pNdisPacket->MiniportReservedEx[x] via RTMP_SET_PACKET_SOURCE()/RTMP_GET_PACKET_SOURCE() */
/* this value is used to decide how to release this NDIS packet - either release to NDIS layer or release */
/* to driver's pre-allocated packet pool */
/*      0xff: NDIS PACKET is pass from Windows NDIS layer */
/*      0 ~ NUM_OF_PREALLOCATEDP_SHARED_MEMORY: NDIS PACKET is created internally */
#define PKTSRC_NDIS             0xff


#define SWITCH_PhyAB(_pAA, _pBB)    \
{                                                                           \
    ULONG	AABasePaHigh;                           \
    ULONG	AABasePaLow;                           \
    ULONG	BBBasePaHigh;                           \
    ULONG	BBBasePaLow;                           \
    BBBasePaHigh = NdisGetPhysicalAddressHigh(_pBB);                                                 \
    BBBasePaLow = NdisGetPhysicalAddressLow(_pBB);                                                 \
    AABasePaHigh = NdisGetPhysicalAddressHigh(_pAA);                                                 \
    AABasePaLow = NdisGetPhysicalAddressLow(_pAA);                                                 \
    NdisSetPhysicalAddressHigh(_pAA, BBBasePaHigh);                                                 \
    NdisSetPhysicalAddressLow(_pAA, BBBasePaLow);                                                 \
    NdisSetPhysicalAddressHigh(_pBB, AABasePaHigh);                                                 \
    NdisSetPhysicalAddressLow(_pBB, AABasePaLow);                                                 \
}


/* NOTE: Has to copy 802.3 header to head of pData for compatibility */
/* with older OS eariler than W2K */
#define REPORT_ETHERNET_FRAME_TO_LLC(_pAd, _p8023hdr, _pData, _DataSize)    \
{                                                                           \
    os_move_mem(_pData - LENGTH_802_3, _p8023hdr, LENGTH_802_3);         \
    _pAd->pRxData = _pData;                                                 \
    NdisMEthIndicateReceive(_pAd->AdapterHandle,                            \
                        (NDIS_HANDLE)_pAd,                                  \
                        (PVOID)(_pData - LENGTH_802_3),                     \
                        LENGTH_802_3,                                       \
                        (PVOID)_pData,                                      \
                        _DataSize,                                          \
                        _DataSize);                                         \
    NdisMEthIndicateReceiveComplete(_pAd->AdapterHandle);                   \
    _pAd->Counters8023.GoodReceives++;                                      \
}


#define COPY_MAC_ADDR(Addr1, Addr2)             ETH_COPY_NETWORK_ADDRESS((Addr1), (Addr2))
#define RELEASE_NDIS_PACKET(_pAd, _pPacket, _Status)                    \
{                                                                       \
    if (RTMP_GET_PACKET_SOURCE(_pPacket) == PKTSRC_NDIS)                \
    {                                                                   \
        NdisMSendComplete(_pAd->AdapterHandle, _pPacket, _Status);      \
        _pAd->RalinkCounters.PendingNdisPacketCount --;                 \
    }                                                                   \
    else                                                                \
        RTMPFreeNdisPacket(_pAd, _pPacket);                             \
}


/* */
/*  Registery definition */
/* */
typedef struct  _RTMP_REG_ENTRY
{
	NDIS_STRING     RegName;            /* variable name text */
	BOOLEAN         bRequired;          /* 1 -> required, 0 -> optional */
	UCHAR           Type;               /* Field Type */
	UINT            FieldOffset;        /* offset to MP_ADAPTER field */
	UINT            FieldSize;          /* size (in bytes) of the field */
	UINT            Default;            /* default value to use */
	UINT            Min;                /* minimum value allowed */
	UINT            Max;                /* maximum value allowed */
} RTMP_REG_ENTRY, *PRTMP_REG_ENTRY;


/* Unify all delay routine by using NdisStallExecution */
_inline     VOID    RtmpusecDelay(
    IN      ULONG   usec)
{
	ULONG   i;

	for (i = 0; i < (usec / 50); i++)
		NdisStallExecution(50);

	if (usec % 50)
		NdisStallExecution(usec % 50);
}


/* Modified by Wu Xi-Kun 4/21/2006 */
typedef void (*TIMER_FUNCTION)(
	IN  PVOID   SystemSpecific1, 
	IN  PVOID   FunctionContext, 
	IN  PVOID   SystemSpecific2, 
	IN  PVOID   SystemSpecific3);

#define RTMP_GREKEYPeriodicExec				GREKEYPeriodicExec
#define RTMP_CMTimerExec					CMTimerExec
#define RTMP_APQuickResponeForRateUpExec	APQuickResponeForRateUpExec


typedef	NDIS_MINIPORT_TIMER	RTMP_OS_TIMER;


/* */
/*  Miniport routines in rtmp_main.c */
/* */
NDIS_STATUS DriverEntry(
	IN  PDRIVER_OBJECT      pDriverObject,
	IN  PUNICODE_STRING     pRegistryPath
	);



extern  char    NIC_VENDOR_DESC[];
extern  int     NIC_VENDOR_DESC_LEN;

/* Increase TxTsc value for next transmission */
/* When i==6, means TSC has done one full cycle, do re-keying stuff follow specs */
/* Should send a special event microsoft defined to request re-key */
#define INC_TX_TSC(_tsc)                                \
{                                                       \
    int i=0;                                            \
    while (++_tsc[i] == 0x0)                            \
    {                                                   \
        i++;                                            \
        if (i == 6)                                     \
            break;                                      \
    }                                                   \
}

#define TRACE_MCU_CMD_INFO

#endif

