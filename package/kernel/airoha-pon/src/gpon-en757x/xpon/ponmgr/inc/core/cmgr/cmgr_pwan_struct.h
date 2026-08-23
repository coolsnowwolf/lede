#ifndef _CMGR_PWAN_STRUCT_H_
#define _CMGR_PWAN_STRUCT_H_

#include "core/cmgr/cmgr_enum.h"
#include "core/cmgr/cmgr_pwan_const.h"

MACRO_CMD_STRUCT(PWanChannelQoS)
{
	MGR_QoSWeightType_t			WeightType ;
	MGR_QoSWeightScale_t		WeightScale ;
	struct {
		MGR_U8						Channel ;
		MGR_QosType_t				QosType ;
		struct {
			MGR_U8					Weight ;
		} Queue[CONST_QUEUE_NUMBER] ;
	} Channel[CONST_CHANNEL_NUMBER] ;
} ;

MACRO_CMD_STRUCT(PWanCongestConfig)
{
	MGR_Enable_t			TrtcmMode ;
	MGR_Enable_t			DeiDropMode ;
	MGR_Enable_t			ThresholdMode ;
	MGR_CongestionScale_t	MaxScale ;
	MGR_CongestionScale_t	MinScale ;
	MGR_U8					GreenDropProbability ;
	MGR_U8					YellowDropProbability ;
	struct {
		MGR_U8				QueueIdx ;
		MGR_U8				GreenMaxThreshold ;
		MGR_U8				GreenMinThreshold ;
		MGR_U8				YellowMaxThreshold ;
		MGR_U8				YellowMinThreshold ;
	} Thrshld[CONST_QUEUE_NUMBER] ;
} ;

MACRO_CMD_STRUCT(PWanTrtcmConfig)
{
	MGR_TrtcmScale_t			TrtcmScale ; //The scale for PBS,CBS. The system default is 128 Byte
	struct {
		MGR_U8					TSIdx ;
		MGR_U16					CIRValue ;
		MGR_U16					CBSUnit ;
		MGR_U16					PIRValue ;
		MGR_U16					PBSUnit ;
	} Trtcm[CONST_SHAPING_NUMBER] ;
} ;

MACRO_CMD_STRUCT(PWanPcpConfig)
{
	MGR_PcpMode_t				CdmTxEncode ;
	MGR_PcpMode_t				CdmRxDecode ;
	MGR_PcpMode_t				GdmRxDecode ;
} ;

MACRO_CMD_STRUCT(PWanCntStats)
{
	MGR_U32			 mask;
	MGR_U32			 TxFrameCnt;
	MGR_U32			 TxFrameLen;
	MGR_U32			 TxDropCnt;
	MGR_U32			 TxBroadcastCnt;
	MGR_U32			 TxMulticastCnt;
	MGR_U32			 TxLess64Cnt;
	MGR_U32			 TxMore1518Cnt;
	MGR_U32			 Tx64Cnt;
	MGR_U32			 Tx65To127Cnt;
	MGR_U32			 Tx128To255Cnt;
	MGR_U32			 Tx256To511Cnt;
	MGR_U32			 Tx512To1023Cnt;
	MGR_U32			 Tx1024To1518Cnt;

	MGR_U32			 RxFrameCnt;
	MGR_U32			 RxFrameLen;
	MGR_U32			 RxDropCnt;
	MGR_U32			 RxBroadcastCnt;
	MGR_U32			 RxMulticastCnt;
	MGR_U32			 RxCrcCnt;
	MGR_U32			 RxFragFameCnt;
	MGR_U32			 RxJabberFameCnt;
	MGR_U32			 RxLess64Cnt;
	MGR_U32			 RxMore1518Cnt;
	MGR_U32			 Rx64Cnt;
	MGR_U32			 Rx65To127Cnt;
	MGR_U32			 Rx128To255Cnt;
	MGR_U32			 Rx256To511Cnt;
	MGR_U32			 Rx512To1023Cnt;
	MGR_U32			 Rx1024To1518Cnt;

	MGR_U32			 RxHecErrorCnt;
	MGR_U32 		 RxFecErrorCnt;
} ;

MACRO_CMD_STRUCT(StormCtrlConfig)
{
	MGR_U32			 mask;
	MGR_U32				threld ;
	MGR_U32				timer ;
} ;

#endif /* _CMGR_PWAN_STRUCT_H_ */
