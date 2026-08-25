#include <linux/crc32.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ip.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/if_arp.h>
#include <linux/dma-mapping.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <ecnt_hook/ecnt_hook_traffic_classify.h>
#if defined(TCSUPPORT_CT_SWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
#include "linux/libcompileoption.h"
#endif
#ifdef TCSUPPORT_RA_HWNAT
#include <linux/foe_hook.h>
#endif
#if defined(TCSUPPORT_CPU_PERFORMANCE_TEST)
#include <net/sock.h>
#endif

#include "common/drv_global.h"
#include "pwan/xpon_netif.h"
#if defined(TCSUPPORT_VOIP) && defined(RTP_RX_SHORTCUT)
#include <asm/tc3162/voip_hook.h>
#endif

#include <ecnt_hook/ecnt_hook_fe.h>
#include <ecnt_hook/ecnt_hook_multicast_general.h>

#if defined(TCSUPPORT_CT_SWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
static unsigned char qos_stop_enable = 1;
static unsigned int qos_stop_num = 1000;
#endif

#ifndef TCSUPPORT_CPU_EN7521
extern int cpu_protect;
extern int software_protect;
#endif

extern int (*wan_speed_test_hook)(struct sk_buff*);
#define SPEED_TEST_SUCCESS	0
unsigned int hw_qos_enable = 0;

#if defined(TCSUPPORT_XPON_LED) && defined(TR068_LED)
#include <asm/tc3162/ledcetrl.h>
#ifdef TCSUPPORT_CPU_EN7521
#include <ecnt_hook/ecnt_hook_fe.h>
#else
#include "../../../raeth/fe_api.h"
#include "epon/fe_reg.h"
#endif

#include <pwan/gpon_wan.h>

#if defined(TCSUPPORT_L2_PPP_MCAST_SHORTCUT)
int pppoeBridgeMultiShortCut = 1;  //0: close shortcut; 1: open shortcut
extern int pppoe_bridge_multicast_handle(struct sk_buff* skb);
#endif

extern int internet_led_on;
extern int internet_trying_led_on;
extern int internet_hwnat_pktnum;
extern int internet_hwnat_timer_switch;
#endif

#define KERNEL_3_18_21 		(LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,19))

#if defined(WAN2LAN)
#define TX_STAG_LEN 6
extern void macSend(uint32 chanId, struct sk_buff *skb);
extern int masko_on_off;
#if defined(TCSUPPORT_WAN2LAN_EXT)
extern int masko_ext;
#endif
#endif
extern unsigned long flash_base;
#if defined(TCSUPPORT_CPU_PERFORMANCE_TEST)
extern int (*pwan_tls_pon_passthrough_hook)(struct sk_buff *skb);
#endif
#if defined(TCSUPPORT_CT_PON) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
int trtcmEnable = 0;
int trtcmTsid = 0;
uint32 priPktChkLen = DEF_PRIORITY_PKT_CHK_LEN;
int priPktChk = 1;

#ifdef TCSUPPORT_QOS
#if defined(TCSUPPORT_XPON_HAL_API_QOS)|| defined(TCSUPPORT_XPON_HAL_API_EXT)
char qosFlag = QOS_HW_CAR;
#else
char qosFlag = NULLQOS;
#endif
#endif

#if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
#if defined(TCSUPPORT_XPON_HAL_API_QOS)|| defined(TCSUPPORT_XPON_HAL_API_EXT)
int gQueueMask = 0;
#else
static int gQueueMask = 0;
#endif
int isNeedHWQoS = 1;
#endif

#ifndef TCSUPPORT_QOS
#define QOS_REMARKING  1  
#endif

#define TCSUPPORT_HW_QOS

#if defined(QOS_REMARKING) || defined(TCSUPPORT_HW_QOS)
static int qos_wrr_info[5] = {0};
static uint8 maxPrio = 3;
#endif
#endif

#if defined(TCSUPPORT_XPON_LED) && defined(TR068_LED)
struct pwan_stats {
	unsigned long	rx_pkts;		/* total pkts received 	*/
	unsigned long	tx_pkts;		/* total pkts transmitted	*/
};

static struct pwan_stats pwanStats;
#endif

#ifdef TCSUPPORT_EPON_DUMMY
    extern atomic_t gDummyDscpCount;
    extern uint gDummySent;
    extern uint gDummyRecyle;
#endif


#ifndef TCSUPPORT_CPU_EN7521
#define SKB_ALLOC_SUCCESS	0
#define SKB_ALLOC_FAIL	1
#endif

#define CONFIG_8021P_REMARK 1
#ifdef CONFIG_8021P_REMARK
#define QOS_8021p_MARK			0x0F00 	/* 8~11 bits used for 802.1p */
#define QOS_8021P_0_MARK		0x08	/* default mark is zero */
#define VLAN_HLEN			    4
#define VLAN_ETH_ALEN			6
#endif

/*****************************************************************************
******************************************************************************
 common utility for interface driver
******************************************************************************
******************************************************************************/
/*****************************************************************************
******************************************************************************/
void __dump_skb(struct sk_buff *skb, uint pktLen) 
{
	unchar n, *p = (unchar *)skb->data ;
	int i ;

	for(i=0 ; i<pktLen ; i++) {
		n = i & 0x0f ;
		
		if(n == 0x00) 		printk(" \n%.4x: ", i) ;
		else if(n == 0x08) 	printk(" ") ;

		printk("%.2x ", *p++) ; 
	}
	printk("\n") ;
}

/* enable CPU path traffic */
void enable_cpu_us_traffic(void)
{
	gpPonSysData->isUpDataTraffic = XPON_ENABLE;
	gpPonSysData->isUpOmciOamTraffic = XPON_ENABLE;
}

/* disable CPU path traffic */
void disable_cpu_us_traffic(void)
{
	gpPonSysData->isUpDataTraffic = XPON_DISABLE;
	gpPonSysData->isUpOmciOamTraffic = XPON_DISABLE;
}
struct sk_buff *replace_oam_pkt(struct sk_buff *skb, int len)
{
	struct sk_buff *skb2 = NULL;
	if(skb == NULL || len < 0){
		printk("\r\n[%s][%d] skb or len error.\r\n",__FUNCTION__,__LINE__); 
		return skb;
	}

	skb2 = dev_alloc_skb(len);		
	if (!skb2){
		printk("\r\n[%s][%d]alloc skb failed.\r\n",__FUNCTION__,__LINE__); 
		return skb;
	}

	skb_put(skb2, skb->len);
	memcpy(skb2->data, skb->data, len);

	dev_kfree_skb(skb);
	return skb2;
}

#if defined(TCSUPPORT_CT_PON) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
#if defined (QOS_REMARKING) || defined (TCSUPPORT_HW_QOS)
static int pon_qoswrr_read_proc(
		char *page, char **start, off_t off,
		int count, int *eof, void *data
)
{
	/* qos_wrr_info[0]: 0:SP, 1:WRR 
	 * qos_wrr_info[1]: txq3 weight for WRR
	 * qos_wrr_info[0]: txq2 weight for WRR
	 * qos_wrr_info[0]: txq1 weight for WRR
	 * qos_wrr_info[0]: txq0 weight for WRR 
	 */
	printk("%d %d %d %d %d\n", *qos_wrr_info, 
		*(qos_wrr_info + 1), *(qos_wrr_info + 2), 
		*(qos_wrr_info + 3), *(qos_wrr_info + 4));
		
	return 0;
}


/* when setting QoS via webpage, cfg_manager will
 * "echo ..... > /proc/tc3162/ptm_qoswrr" to do
 * HW QDMA QoS config */
static int pon_qoswrr_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data
)
{
	int len, i;
	char get_buf[32];
	int max_wrr_val = 0;
	int path, txq;
	QDMA_TxQosScheduler_T txQos;
	int ret;

	if(count > 32)
		len = 32;
	else
		len = count;
		
	/* use the copy_from_user function to copy 
	 * buffer data to our get_buf */
	if(copy_from_user(get_buf, buffer, len))
		return -EFAULT;
		
	/* zero terminate get_buf */
	get_buf[len]='\0';


	/* qos_wrr_info[0]: 0:SP, 1:WRR 
	 * qos_wrr_info[1]: txq3 weight for WRR
	 * qos_wrr_info[2]: txq2 weight for WRR
	 * qos_wrr_info[3]: txq1 weight for WRR
	 * qos_wrr_info[4]: txq0 weight for WRR */
	if(sscanf(get_buf, "%d %d %d %d %d", qos_wrr_info, 
			(qos_wrr_info+1), (qos_wrr_info+2), 
			(qos_wrr_info+3), (qos_wrr_info+4)) != 5)
		return count;


	/* find the txq with max qos wrr weight */
	for (i = 0; i < 4; i++) {
		if (max_wrr_val < qos_wrr_info[i + 1]) {
			max_wrr_val = qos_wrr_info[i + 1];
			maxPrio = 3 - i;
		}
	}


	/* Strict Priority */
	if(*qos_wrr_info == 0) 
	{
		/* set QDMA as Strict Priority Mode,
	 	 * so that p7> ... >p0 for all paths */
		for (path = 0; path < PATH_NUM; path++)
		{
			memset(&txQos, 0, sizeof(QDMA_TxQosScheduler_T));
			txQos.channel = path;
			txQos.qosType = QDMA_TXQOS_TYPE_SP;
#ifdef TCSUPPORT_CPU_EN7521
			ret = QDMA_API_SET_TX_QOS(ECNT_QDMA_WAN, &txQos);
#else /* TCSUPPORT_CPU_EN7521 */
			ret = qdma_set_tx_qos(&txQos);
#endif /* TCSUPPORT_CPU_EN7521 */
			if (ret)
			{
				printk("\nFAILED(%s): qdma strict priority setting for path%d\n", __FUNCTION__, path);
				return -1;
			}
		}

		printk("\nSP setting for txq0~3 is done\n\n");

	}
	/* Weighted Round Robin */
	else 
	{
		/* set QDMA's SP & WRR priority registers,
	 	 * so that p7>p6>p5>p4> p3:p2:p1:p0 for path 0~7 && 
		 * p3:p2:p1:p0 == qos_wrr_info[1]:qos_wrr_info[2]:
		 * 				  qos_wrr_info[3]:qos_wrr_info[4] */
		for (path = 0; path < PATH_NUM; path++)
		{
			memset(&txQos, 0, sizeof(QDMA_TxQosScheduler_T));
			txQos.channel = path;
			txQos.qosType = QDMA_TXQOS_TYPE_SPWRR4;
			for(txq = 0 ; txq < 4; txq++)
				txQos.queue[txq].weight = qos_wrr_info[4-txq];
					
#ifdef TCSUPPORT_CPU_EN7521
			ret = QDMA_API_SET_TX_QOS(ECNT_QDMA_WAN, &txQos);
#else /* TCSUPPORT_CPU_EN7521 */
			ret = qdma_set_tx_qos(&txQos);
#endif /* TCSUPPORT_CPU_EN7521 */					
			if (ret)
			{
				printk("\nFAILED(%s): qdma wrr setting for path%d\n" , __FUNCTION__, path);
				return -1;
			}
		}

		//take path0's WRR setting as an example to show
		memset(&txQos, 0, sizeof(QDMA_TxQosScheduler_T));
		txQos.channel = 0;
#ifdef TCSUPPORT_CPU_EN7521
		QDMA_API_GET_TX_QOS(ECNT_QDMA_WAN, &txQos);
#else /* TCSUPPORT_CPU_EN7521 */
		qdma_get_tx_qos(&txQos) ;
#endif /* TCSUPPORT_CPU_EN7521 */	
		printk("\nWRR setting for txq0~3 is done:\n" 
		"(Type:%d) Q3:%d, Q2:%d, Q1:%d, Q0:%d\n\n",
					txQos.qosType, 
					txQos.queue[3].weight,
					txQos.queue[2].weight,
					txQos.queue[1].weight,
					txQos.queue[0].weight);
	}
		
	return len;
}
#endif

static uint8 isPriorityPkt(uint8 *cp, int *priority)
{
	uint16 etherType;
	uint8 ipVerLen;
	uint8 ipProtocol;
	uint8 tcpFlags;
	uint16 pppProtocol;


	if (cp == NULL)
	{
		printk("\nERROR: cp is NULL at isPriorityPkt\n");
		return 0;
	}
	if (priority == NULL)
	{
		printk("\nERROR: priority is NULL at isPriorityPkt\n");
		return 0;
	}

	/* skip DA and SA mac address */
	cp += 12;
	/* get ether type */
	etherType = *(uint16 *) cp;
	/* skip ether type */
	cp += 2;

	/*parse if vlan exists*/
	if (etherType == ETHER_TYPE_8021Q) {
		/*skip 802.1q tag field*/
		cp += 2;
		/*re-parse ether type*/
		etherType = *(uint16 *) cp;
		/* skip ether type */
		cp += 2;
	}

	/*check whether PPP packets*/ 
	if (etherType == 0x8864) {
		/* skip pppoe head */
		cp += 6; 					/* 6: PPPoE header 2: PPP protocol */
		/* get ppp protocol */
		pppProtocol = *(uint16 *) cp;
		/* check if LCP protocol */
		if (pppProtocol == 0xc021) {
			*priority = 3;
			return 1;
		/* check if IP protocol */
		} else if (pppProtocol != 0x0021) {
			return 0;
		}
		/* skip ppp protocol */
		cp += 2; 					/* 6: PPPoE header 2: PPP protocol */
	} else if (etherType == 0x8863) {
		*priority = 3;
		return 1;
	} else {
		/* check if ip packet */
		if (etherType != ETHER_TYPE_IP)
			return 0;
	}

	/* check if it is a ipv4 packet */
	ipVerLen = *cp;
	if ((ipVerLen & 0xf0) != 0x40) 
		return 0;

	/* get ip protocol */
	ipProtocol = *(cp + 9);
	/* check if ICMP/IGMP protocol */
	if ((ipProtocol == 1) || (ipProtocol == 2)) {
		*priority = 3;
		return 1;
	}
	/* check if TCP protocol */
	if (ipProtocol != 6)
		return 0;
		
	/* align to TCP header */
	cp += (ipVerLen & 0x0f) << 2;
	/* get TCP flags */
	tcpFlags = *(cp + 13);
	/* check if TCP syn/ack/psh ack/fin ack */
	if (((tcpFlags & 0x10) == 0x10) || (tcpFlags == 0x02)) {
		*priority = 3;
		return 1;
	}
		
	return 0;
}

static int pon_trafficShapingSet_write_proc(
		struct file *file, const char *buffer, 
		unsigned long count, void *data
)
{
#ifdef TCSUPPORT_CPU_EN7521
	printk("\npon_trafficShapingSet_write_proc EN7521 is not support\n");
#else
	char valString[64];
	int cir, pir, greenDropRate, yellowDropRate, i;
	QDMA_TxQueueTrtcm_T txqTrtcm;
	QDMA_TxQueueDropProbability_T txqDropRate;
	QDMA_TxQueueCongestScale_T txqScale;
	QDMA_TxQueueCongestThreshold_T txqThreshold;
	QDMA_Mode_t qdmaMode = QDMA_ENABLE;
	

	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EFAULT;

	valString[count] = '\0';

	sscanf(valString, "%d %d %d %d %d %d", &trtcmEnable, &trtcmTsid, &cir, &pir, &greenDropRate, &yellowDropRate);

	if (!trtcmEnable)
	{
		qdmaMode = QDMA_DISABLE;
		qdma_set_txqueue_trtcm_mode(qdmaMode);
		printk("\ntrtcm coloring is disabled!\n");
		printk("\nUsage: echo 1 (tsid) (cir)[*64Kbps] (pir)[*64Kbps] (greenDropRate)[/255] (yellowDropRate)[/255] > /proc/tc3162/pon_trafficShapingSet\n\n");
		return count;
	}


	/* Setting for CIR, PIR, CBS, PBS*/
	if (isFPGA)
	    write_reg_word(0xbfb0082c, 0x31000000); //let cir, pir more precise.
	
	memset(&txqTrtcm, 0, sizeof(QDMA_TxQueueTrtcm_T));
	txqTrtcm.tsid = trtcmTsid;
	//the uint of cir, pir is 64Kpbs
	txqTrtcm.cirParamValue = cir;
	txqTrtcm.pirParamValue = pir;
	//160*128=20480 byte (the largetest size that PTM can transmit in a burst)
	//Note: 128 is set by QDMA driver in initialization.
	//Note: 20480 must be at least 2~3 larger than cir, pir
	txqTrtcm.cbsParamValue = 160;
	txqTrtcm.pbsParamValue = 160;

	qdma_set_txqueue_trtcm_params(&txqTrtcm);
	qdma_set_txqueue_trtcm_mode(qdmaMode);


	/* Setting for green, yellow drop probability*/
	
	memset(&txqDropRate, 0, sizeof(QDMA_TxQueueDropProbability_T));
	txqDropRate.green = greenDropRate;
	txqDropRate.yellow = yellowDropRate;

	qdma_set_congest_drop_probability(&txqDropRate);


	/* Setting for green, yellow congestion threshold (descriptors based).
	 * 1. smaller than minThreshold, packets always pass
	 *    w/o caring about drop probility.
	 * 2. between minThreshold and maxThreshold, packets
	 *    are dropped by drop probability.
	 * 3. larger than maxThreshold, packets are all drop 
	 *    w/o caring about drop probility. */
	memset(&txqScale, 0, sizeof(QDMA_TxQueueCongestScale_T));
	txqScale.maxScale = QDMA_TXQUEUE_SCALE_16_DSCP;
	qdma_set_congestion_scale(&txqScale);

	/* we let each queue of each channel have the same
	 * threshold for grn/ylw packets (which is 16*8=128 dscps),
	 * so packets are dropped by drop probability. */
	memset(&txqThreshold, 0, sizeof(QDMA_TxQueueCongestThreshold_T));
	for (i = 0; i < TX_QUEUE_NUM; i++)
	{
		txqThreshold.queueIdx = i;
		txqThreshold.grnMaxThreshold = 5;
		txqThreshold.ylwMaxThreshold = 5;
#ifdef TCSUPPORT_CPU_EN7521
		//todo: not implement yet
#else
		qdma_set_congest_threshold(&txqThreshold);
#endif
	}

	printk("\ntrtcmEnable:%d\ntrtcmTsid:%d\n"
			"CIR:%d[*64kbps]\nPIR:%d[*64kbps]\n"
			"greenDropRate:%d[%%]\nyellowDropRate:%d[%%]\n\n",
			trtcmEnable, trtcmTsid, 
			cir, pir, 
			(greenDropRate*100)/255, 
			(yellowDropRate*100)/255);
#endif
	return count;
}

static int pon_trafficShapingSet_read_proc(
		char *buf, char **start, off_t off, int count,
        int *eof, void *data
)
{
	int index = 0;
#ifdef TCSUPPORT_CPU_EN7521
	printk("\n pon_trafficShapingSet_read_proc EN7521 is not support\n");
#else
	off_t pos = 0;
	off_t begin = 0;
	QDMA_TxQueueTrtcm_T txqTrtcm;
	QDMA_TxQueueCongestCfg_T txqCongestCfg;
	QDMA_Mode_t qdmaMode;
	int i;


	qdmaMode = qdma_get_txqueue_threshold_mode();
	if (qdmaMode)
	{
		index += sprintf(buf+index, 
				"\nQDMA threshold drop is enabled\n");
		CHK_BUF();	
	}
	else
	{
		index += sprintf(buf+index, 
				"\nQDMA threshold drop is disabled\n");
		CHK_BUF();	
	}
	
	qdmaMode = qdma_get_txqueue_trtcm_mode();
	if (qdmaMode)
	{
		index += sprintf(buf+index, 
				"\nQDMA trtcm coloring is enabled\n");
		CHK_BUF();	
	}
	else
	{
		index += sprintf(buf+index, 
				"\nQDMA trtcm coloring is disabled\n");
		CHK_BUF();	
	}
	
	if (!trtcmEnable)
	{
		*eof = 1;
		goto done;
	}

	memset(&txqTrtcm, 0, sizeof(QDMA_TxQueueTrtcm_T));

	for (i = 0; i < 32; i++)
	{
		txqTrtcm.tsid = i;
		if (!qdma_get_txqueue_trtcm_params(&txqTrtcm))
		{
			if (txqTrtcm.pirParamValue == 0)
				continue;
		
			index += sprintf(buf+index, "\nTSID: %d, CIR: %d [*64Kbps], PIR: %d [*64Kbps], CBS: %d [*128Byte], PBS: %d [*128Byte]\n", txqTrtcm.tsid, txqTrtcm.cirParamValue, txqTrtcm.pirParamValue, txqTrtcm.cbsParamValue, txqTrtcm.pbsParamValue);
			CHK_BUF();
		}
	}

	memset(&txqCongestCfg, 0, sizeof(QDMA_TxQueueCongestCfg_T));

	if (!qdma_get_congest_config(&txqCongestCfg))
	{
		index += sprintf(buf+index, "\nmaxScale: %d, minScale: %d\n", 0x2<<(int)txqCongestCfg.maxScale, 0x2<<(int)txqCongestCfg.minScale);
		CHK_BUF();
		index += sprintf(buf+index, "greenDropRate: %d(%%), yellowDropRate: %d(%%)\n", ((int)txqCongestCfg.grnDropProb*100)/255, ((int)txqCongestCfg.ylwDropProb*100)/255);
		CHK_BUF();
		for (i = 0; i < 8; i++)
		{
			index += sprintf(buf+index, "TxQueue[%d]:\n", i);
			CHK_BUF();			
			index += sprintf(buf+index, "grnMaxThld: %d, grnMinThld: %d, ylwMaxThld: %d, ylwMinThld: %d\n", (int)txqCongestCfg.queue[i].grnMaxThreshold, (int)txqCongestCfg.queue[i].grnMinThreshold, (int)txqCongestCfg.queue[i].ylwMaxThreshold, (int)txqCongestCfg.queue[i].ylwMinThreshold);
			CHK_BUF();
		}
	}

	*eof = 1;

done:
	*start = buf + (off - begin);
	index -= (off - begin);
	if (index < 0) 
		index = 0;
	if (index > count) 
		index = count;
#endif
	return index;
}


#if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
int isDefaultQueue(struct sk_buff *skb)
{
	int queue;
	queue = skb->mark & QOS_FILTER_MARK;
		
	if(queue == 0){
				return 1;
	}
	
	if(gQueueMask & (1 << ((queue >> 4) -1 ))){ 	//default queue
		return 0;
	}else{
		return 1;
	}
}

int isWeigt0(struct sk_buff *skb)
{
	int i = 0;
	int queue;
	
	queue = (skb->mark & QOS_FILTER_MARK) >> 4;

	if(queue < 1 || queue >4){	//wrr queue is from 1 to 4
		return 0;
	}

	if(qos_wrr_info[queue]){
		return 0;
	}else{				//weight is 0
		return 1;
	}	
}

#endif
#ifdef TCSUPPORT_QOS
/* when setting QoS via webpage, cfg_manager will
 * "echo ..... > /proc/tc3162/pon_tcqos_disc" to do
 * tcqos_disc config */
static int pon_tcqos_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data
)
{
	char qos_disc[10];
	int len;

	if (count > 10) {
		len = 10;
	}
	else {
		len = count;
	}
	memset(qos_disc, 0, sizeof(qos_disc));
	
	if(copy_from_user(qos_disc, buffer, len - 1))
		return -EFAULT;

	qos_disc[len] = '\0';

#if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	isNeedHWQoS = 1;
#endif

	if (!strcmp(qos_disc, "HWWRR")) {
		qosFlag = QOS_HW_WRR;
		printk("\nqos discipline is HW WRR.\n\n");
	}
	else if (!strcmp(qos_disc, "HWPQ")) {
		qosFlag = QOS_HW_PQ;
		printk("\nqos discipline is HW PQ.\n\n");
	}
#if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	else if (!strcmp(qos_disc, "HWCAR")) {
		qosFlag = QOS_HW_CAR;
		printk("\nqos discipline is HW CAR.\n\n");
	}
#endif
	else {
		qosFlag = NULLQOS;
		printk("\nqos discipline is disabled.\n\n");
	}

	return len;
}
#endif

#if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
static int pon_tcqos_read_proc(
			char *page, char **start, off_t off,
			int count, int *eof, void *data
)
{
	if(qosFlag == QOS_HW_WRR){
		printk("\nqos discipline is HW WRR.\n\n");
	}else if(qosFlag == QOS_HW_PQ){
		printk("\nqos discipline is HW PQ.\n\n");
	}else if(qosFlag == QOS_HW_CAR){
		printk("\nqos discipline is HW CAR.\n\n");
	}else {
		printk("\nqos discipline is disabled.\n\n");
	}

	printk("\n isNeedHWQoS =%d.\n\n",isNeedHWQoS);

	return 0;
}



static int pon_tcqos_queuemask_read_proc(
			char *page, char **start, off_t off,
			int count, int *eof, void *data
)
{
	printk("\nqos gQueueMask is 0x%x\n",gQueueMask);		
	return 0;
}


static int pon_tcqos_queuemask_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data
)
{
	char valString[4];
	int val = 0;
	
	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EINVAL;

	valString[count] = '\0';

	sscanf(valString, "%d", &val);

	if (val)
	{
		gQueueMask = val;
		printk("\nqos gQueueMask is 0x%x\n",gQueueMask);
	}

	return count;

}
#endif
#endif

#ifdef CONFIG_8021P_REMARK
/* 
 * Fill vlanTag's PCP (Priority Code Point) by skb->mark's bit8~11.
 * If packet's byte12-13 (vlanTag's TPID) is 0x8100, just change
 * the packet's vlanTag's PCP.
 * If packet's byte12-13 (vlanTag's TPID) isn't 0x8100, 
 * insert a VLAN tag with TPID=0x8100, PCP=mark, and VID=0.
 */
static inline struct sk_buff* vlanPriRemark(struct sk_buff *skb)
{
    char * vlan_p = NULL, *ether_type_ptr = NULL;
    unsigned char ucprio = 0;
    unsigned char uc802prio = 0;
    uint16 vid=0;
    int copy_len = 0;

    if ( skb->mark & QOS_8021p_MARK ) {
#if 1
        /*vlan tagging*/
        ether_type_ptr = skb->data + 12;
#endif		
        ucprio = (skb->mark & QOS_8021p_MARK) >> 8;
        if ( (ucprio < QOS_8021P_0_MARK) && (ucprio >= 0) ) { //0~7 remark
            uc802prio = ucprio;
        }
        else if ( QOS_8021P_0_MARK == ucprio ) {    //zero mark
            uc802prio = 0;
        }
        else{//pass through
            /*do nothing*/
            return skb;
        }
        if(*(unsigned short *)ether_type_ptr == 0x8100){
            vid=(*(unsigned short *)(ether_type_ptr+2) & 0xfff);
        }
        else{
           return skb; 
        }
    
        vlan_p = skb->data + 12;
        *(unsigned short *)vlan_p = 0x8100;
        
        vlan_p += 2;
        /*3 bits priority and vid vlaue*/
        *(unsigned short*)vlan_p = (((uc802prio & 0x7) << 13)|vid) ;
        skb->network_header -= VLAN_HLEN;
        skb->mac_header -= VLAN_HLEN;

    }
    return skb;
}
#endif /*CONFIG_8021P_REMARK*/

static int pon_hw_qos_enable_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char val_string[64];
	char* endpo; 
	int i = 0;
	int ret = 0;
		
	if (count > sizeof(val_string) - 1)
	{
		ret = -EFAULT;
		return ret;
	}
	memset(val_string,0,64);
	if (copy_from_user(val_string, buffer, count))
	{
		ret = -EFAULT;
		return ret;
	}

	hw_qos_enable = simple_strtol(val_string,&endpo,10);

	if(hw_qos_enable)
	{
		xpon_set_qdma_qos(1);
#ifdef TCSUPPORT_CPU_EN7521
		//todo: not implement yet
#else
		qdma_set_txqueue_threshold_mode(1);
#endif
	}
	else
	{
		xpon_set_qdma_qos(0);
#ifdef TCSUPPORT_CPU_EN7521
		//todo: not implement yet
#else
		qdma_set_txqueue_threshold_mode(1);
#endif

	}
	
	return count;
}

static int pon_hw_qos_enable_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	printk("%d\n",hw_qos_enable);
	
	return 0;
}

/*****************************************************************************
******************************************************************************
 network device operation function 
******************************************************************************
******************************************************************************/

static inline struct sk_buff * pwan_dev_alloc_skb(void)
{
    if (PWAN_MAX_MTU <= SKBMGR_RX_BUF_LEN){
        return skbmgr_dev_alloc_skb2k() ;
    }else if (PWAN_MAX_MTU <= SKBMGR_4K_RX_BUF_LEN){
        return skbmgr_dev_alloc_skb4k() ;
    }else{
        return dev_alloc_skb(PWAN_MAX_MTU) ;
    }
}

/*****************************************************************************
******************************************************************************/
#ifndef TCSUPPORT_CPU_EN7521
int pwan_prepare_rx_buffer(PWAN_FERxMsg_T *pRxBmMsg, struct sk_buff *skb) 
{
	int offset, ret = 0;
	int skb_alloc_flag = SKB_ALLOC_SUCCESS;
	struct sk_buff *skbNew = NULL;

#ifndef TCSUPPORT_MERGED_DSCP_FORMAT
	/* allocate QDMA message memory */	
	if(pRxBmMsg == NULL){
		pRxBmMsg= (PWAN_FERxMsg_T *)kzalloc(sizeof(PWAN_FERxMsg_T), GFP_KERNEL) ;/*kmalloc & memset*/
		if(pRxBmMsg == NULL) {
			PON_MSG(MSG_ERR, "Allocate QDMA message memory failed.\n") ;
			ret = -ENOMEM ;
			goto err ;
		}	
	}else{
		memset(pRxBmMsg, 0, sizeof(PWAN_FERxMsg_T)) ;
	}	
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */

	/* allocate the packet buffer */
	skbNew = pwan_dev_alloc_skb() ;
	if(skbNew != NULL) {
		/* Shift to 4 byte alignment */
		offset = ((uint)(skbNew->tail) & (SKB_ALIGNMENT-1)) ;
		if(offset) {
			skb_reserve(skbNew, (SKB_ALIGNMENT - offset)) ;
		}
		/* Reserve 2 bytes for zero copy */
		skb_reserve(skbNew, NET_IP_ALIGN) ;
	}else{
		skb_alloc_flag = SKB_ALLOC_FAIL;
		skbNew = skb;
		skb = NULL;
	}

	ret = qdma_hook_receive_buffer(pRxBmMsg, sizeof(PWAN_FERxMsg_T), skbNew) ;
	if(ret != 0) {
		ret = -ENODEV ;
		goto err ;
	}
	return skb_alloc_flag == SKB_ALLOC_FAIL ? SKB_ALLOC_FAIL : ret ;
	
err:	
	if(pRxBmMsg)		kfree(pRxBmMsg) ;
	if(skbNew) 		dev_kfree_skb(skbNew) ;
	if(skb) 		dev_kfree_skb(skb) ;
	return ret ;
}
#endif

#if defined(TCSUPPORT_XPON_LED) && defined(TR068_LED)
static void pwan_net_timer(unsigned long data)
{
	unsigned long rx_pkts_diff ,tx_pkts_diff;
	unsigned long rx_pkts,tx_pkts;

	if(TRUE == gpPonSysData->ponMacPhyReset
#ifndef TCSUPPORT_CPU_EN7521
       || atomic_read(&fe_reset_flag) 
#endif
        ) {
		goto restart_timer;
	}

	if ((gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON) ||
		(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_EPON)){

#ifdef TCSUPPORT_RA_HWNAT
	if (internet_hwnat_timer_switch) {		
		rx_pkts = pwanStats.rx_pkts; 	
		tx_pkts = pwanStats.tx_pkts; 		
#ifdef TCSUPPORT_CPU_EN7521
		FE_API_GET_ETH_FRAME_CNT(FE_GDM_SEL_RX, &(pwanStats.rx_pkts));
		FE_API_GET_ETH_FRAME_CNT(FE_GDM_SEL_TX, &(pwanStats.tx_pkts));
#else
		pwanStats.rx_pkts = fe_reg_read(GDMA2_RX_ETHERPCNT - FE_BASE);	
		pwanStats.tx_pkts = fe_reg_read(GDMA2_TX_ETHCNT - FE_BASE);
#endif
		rx_pkts_diff = pwanStats.rx_pkts- rx_pkts;
		tx_pkts_diff = pwanStats.tx_pkts- tx_pkts;

		if ((rx_pkts_diff > internet_hwnat_pktnum) ||
			(tx_pkts_diff > internet_hwnat_pktnum)) {
			//printk("\r\npwan_net_timer light led,internet_hwnat_pktnum=%d",internet_hwnat_pktnum);
			/*for interner traffic led*/
#if !defined(TCSUPPORT_C9_ROST_LED)
			
			if(internet_led_on) {//IP connected and IP traffic is passing
				ledTurnOn(LED_INTERNET_ACT_STATUS);
			} 
			else {
				if(!internet_trying_led_on) {
					ledTurnOff(LED_INTERNET_STATUS);
					ledTurnOff(LED_INTERNET_TRYING_STATUS);
				}
			}		
#endif			
		}
	}
#endif
	}

restart_timer:
	/* Schedule for the next time */
	gpWanPriv->pwan_timer.expires = jiffies + msecs_to_jiffies(250);
  	//add_timer(&gpWanPriv->pwan_timer); heredbg
  	mod_timer(&gpWanPriv->pwan_timer, (jiffies + msecs_to_jiffies(250)));
}
#endif

/*****************************************************************************
******************************************************************************/
static int pwan_net_open(struct net_device *dev)
{
	PON_MSG(MSG_TRACE, "%s entered\n", __FUNCTION__) ;

#if (defined CONFIG_USE_RX_NAPI) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	{
		PWAN_NetPriv_T *pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev) ;
		napi_enable(&pNetPriv->napi) ;
	}
#endif /* (defined CONFIG_USE_RX_NAPI) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)) */
	
	netif_start_queue(dev) ;

#if defined(TCSUPPORT_XPON_LED) && defined(TR068_LED)
	/* Schedule timer */
#if !defined(TCSUPPORT_XPON_HAL_API_EXT) 
	if(strcmp(dev->name, "pon") == 0)
#endif
	{
		init_timer(&gpWanPriv->pwan_timer);
		gpWanPriv->pwan_timer.expires = jiffies + msecs_to_jiffies(250);
		gpWanPriv->pwan_timer.function = pwan_net_timer;
		gpWanPriv->pwan_timer.data = 0;
		add_timer(&gpWanPriv->pwan_timer);
		memset(&pwanStats, 0, sizeof(pwanStats));
	}
#endif

	return 0 /* -ENODEV */ ;
}

/*****************************************************************************
******************************************************************************/
static int pwan_net_stop(struct net_device *dev)
{
	PON_MSG(MSG_TRACE, "%s entered\n", __FUNCTION__) ;

#if defined(TCSUPPORT_XPON_LED) && defined(TR068_LED)
#if !defined(TCSUPPORT_XPON_HAL_API_EXT) 
	if((strcmp(dev->name, "pon") == 0) && (dev->flags & IFF_UP))
#endif
	{
		del_timer_sync(&gpWanPriv->pwan_timer);
	}
#endif

	netif_stop_queue(dev) ;

#if (defined CONFIG_USE_RX_NAPI) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	{	
		PWAN_NetPriv_T *pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev) ;
		napi_disable(&pNetPriv->napi) ;
	}
#endif /* (defined CONFIG_USE_RX_NAPI) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)) */

	return 0 ;
}

#if defined(TCSUPPORT_CT_SWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
extern int (*sw_pktqosEnqueue) (struct sk_buff * bp);
extern void (*sw_PKTQOS_SET_STOP) (void);
extern int (*sw_isSWQosActive) (void);
#endif

int get_pon_link_type(void)
{
    return gpPonSysData->sysLinkStatus;
}

EXPORT_SYMBOL(get_pon_link_type);

/*****************************************************************************
******************************************************************************/

#ifdef TCSUPPORT_EPON_DUMMY
extern uint gDummyDrop_QDMA_TX_FAIL;
extern void start_epon_dummy();
#endif

int isHighestPriorityPacket(struct sk_buff *skb){
    u8 *cp = NULL;
    u16 etherType = 0;
    u16 pppProtocol = 0;
    u16 isHighestPPacket = 0;
    struct iphdr *iph = NULL;
    
    cp = skb->data;
    cp += 12;
    etherType = ntohs(*(u16*)cp);
    cp += 2;

    //skip vlan tag
    while((0x8100 == etherType) || (0x88a8 == etherType) || (0x9100 == etherType))
    {
        cp += 2;
        etherType = ntohs(*(u16*)cp);
        cp += 2;
    }
    
    if ((ETH_P_ARP == etherType) || (ETH_P_PPP_DISC == etherType)) {
        isHighestPPacket = 1;
    }
    else if (ETH_P_PPP_SES == etherType) {
        /* skip pppoe head */
        cp += 6;                    /* 6: PPPoE header 2: PPP protocol */
        /* get ppp protocol */
        pppProtocol = ntohs(*(u16 *) cp);
        /* check if LCP protocol */
        if (pppProtocol == 0xc021 || 
            pppProtocol == 0xc023 || 
            pppProtocol == 0xc025 || 
            pppProtocol == 0x8021 || 
            pppProtocol == 0xc223) {
            isHighestPPacket = 1;
        }
    }
    else if(ETH_P_IP == etherType){
        iph = (struct iphdr *)cp;
        if (IPPROTO_IGMP == iph->protocol) {
            isHighestPPacket = 1;
        }           
    }   

    return isHighestPPacket;
}

extern int isCtlDataPkt(struct sk_buff *skb);


#if defined(WAN2LAN)

    static inline int mirror_pre_tx_prepare(void)
    {
    #if defined(TCSUPPORT_WAN2LAN_EXT)
        if(masko_ext & 0x04){
            return 1;
        }
    #endif
        return 0;
    }

    static inline int mirror_post_tx_prepare(void)
    {
    #if defined(TCSUPPORT_WAN2LAN_EXT)
    	if(masko_on_off || (masko_ext & 0x01) )
    #else
    	if(masko_on_off)
    #endif
        {
            return 1;
        }

        return 0;
    }

    static struct sk_buff * copy_pwan_skb(struct sk_buff *skb, unchar netIdx) 
    {
		struct sk_buff *skb2 = NULL;
		int len = TX_STAG_LEN;
		static const u_char omciHdr[] = {0x00,0x00,0x00,0x00,0x00,0x02, 0x00,0x00,0x00,0x00,0x00,0x01, 0x88, 0xb5};
		
		if (PWAN_IF_OMCI == netIdx){
			len += 14;
		}

		//Check the skb headroom is enough or not. shnwind 20100121.
		if(skb_headroom(skb) < len )
			skb2 = skb_copy_expand(skb, len , skb_tailroom(skb) , GFP_ATOMIC);
		else
			skb2 = skb_copy(skb, GFP_ATOMIC);


		if(unlikely(!skb2)){
			printk("\nFAIL(%s): wan2lan allocation\n", __FUNCTION__);
            return NULL;
		}
#if defined(TCSUPPORT_RA_HWNAT)
		if (ra_sw_nat_hook_xfer)
			ra_sw_nat_hook_xfer(skb, skb2);
#endif	           
        
		if (PWAN_IF_OMCI == netIdx){
			skb_push(skb2, 14);
			memcpy(skb2->data, omciHdr, ARRAY_SIZE(omciHdr));
		}
		skb2->mark |= SKBUF_COPYTOLAN;
        return skb2;
    }

#else

    #define mirror_pre_tx_prepare()  (0)
    #define mirror_post_tx_prepare() (0)
    #define copy_pwan_skb(x,y) (NULL)

#endif

#ifndef TCSUPPORT_CPU_EN7521

    static inline void dec_pwan_tx_data_cnt(void)
    {
        unsigned long flags=0;
        spin_lock_irqsave(&gpWanPriv->txLock, flags) ;
        if(gpWanPriv->devCfg.txPacketCount > 0)
        	gpWanPriv->devCfg.txPacketCount-- ;
        spin_unlock_irqrestore(&gpWanPriv->txLock, flags) ;
    }

    static inline void inc_pwan_tx_data_cnt(void)
    {
        unsigned long flags=0;
		spin_lock_irqsave(&gpWanPriv->txLock, flags) ;
		gpWanPriv->devCfg.txPacketCount++ ;
		spin_unlock_irqrestore(&gpWanPriv->txLock, flags) ;
    }
    
#else

    #define dec_pwan_tx_data_cnt()
    #define inc_pwan_tx_data_cnt()

#endif

#if defined(TCSUPPORT_CT_PON) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
#if defined(TCSUPPORT_QOS) && !defined(TCSUPPORT_CPU_EN7521)
static int queue_policy(struct sk_buff *skb)
{
    int txq = 0;
    switch (qosFlag)
    {
        case QOS_HW_WRR:
            /* HW WRR mode */
        #if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
            if(isDefaultQueue(skb)){        //default queue
                txq = 0;
            }else if(isWeigt0(skb)){
                txq = -1 ;
            }else
        #endif
            if ((skb->mark & QOS_FILTER_MARK) == QOS_HH_PRIORITY)
                txq = 3;
            else if ((skb->mark & QOS_FILTER_MARK) == QOS_H_PRIORITY)
                txq = 2;
            else if ((skb->mark & QOS_FILTER_MARK) == QOS_M_PRIORITY)
                txq = 1;
            break;
        case QOS_HW_PQ:
            /* HW PQ mode */
        #if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
            if(isDefaultQueue(skb)){        //default queue
                txq = 0;
            }else
        #endif
            if (txq < 3 && (skb->mark & QOS_FILTER_MARK) == QOS_HH_PRIORITY)
                txq = 3;
            else if (txq < 2 && (skb->mark & QOS_FILTER_MARK) == QOS_H_PRIORITY)
                txq = 2;
            else if (txq < 1 && (skb->mark & QOS_FILTER_MARK) == QOS_M_PRIORITY)
                txq = 1;
            break;
    #if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
        case QOS_HW_CAR:            
            if(isDefaultQueue(skb)){        //default queue
                txq = 0;
            }else{
                txq = (skb->mark & QOS_FILTER_MARK)>> 4;
            }
            break;
    #endif
        case NULLQOS: 
            /* for putting rtp packets to HH priority 
             * when qosFlag is not selected as WRR or PQ*/
            if ((skb->mark & QOS_FILTER_MARK) == QOS_HH_PRIORITY)
                txq = 3;
            break;
        default:
            break;
    }
    return txq;
}
#endif
#endif

int pwan_net_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
#define GOTO_DROP(n) do {XPON_DROP_PRINT; goto drop##n; }while(0)

    int ret = NETDEV_TX_OK ;
    PWAN_NetPriv_T *pNetPriv ;
    PWAN_FETxMsg_T txBmMsg = {0};
    uint16 pktLens=0;

#ifdef TCSUPPORT_WAN_GPON
    u16    gemport = 0;
    u16    gemIdx  = 0;
#endif
    u8     dummyPkt= 0;
    int is_crtl_data_pkt = 0;

    int txq = 0;
    struct sk_buff * unshared_skb = NULL;
    struct port_info xpon_info= {0}; /* only used in gwan_prepare_tx_message() */
    struct sk_buff * skb_mirror = NULL;
    
    PON_MSG(MSG_TRACE, "Start to transmit a packets.\n") ;
    
    pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev) ;
    skb->dev = dev ; /* Use for tx DSCP recycle. */

    dummyPkt = isDummyPkt(skb);

    if(!dummyPkt)
    {
        unshared_skb = skb_unshare(skb, GFP_ATOMIC);
        if(NULL == unshared_skb)
        {
            printk("ERR %s skb_unshare failed! \n",__FUNCTION__);
            GOTO_DROP(2);
        }
        skb = unshared_skb;
    }
    
#ifndef TCSUPPORT_CPU_EN7521
    if((pNetPriv->netIdx != PWAN_IF_OMCI ) && (pNetPriv->netIdx != PWAN_IF_OAM ) ){
        is_crtl_data_pkt = isCtlDataPkt(skb);
    }
#endif    

    if(!dummyPkt && mirror_pre_tx_prepare() && (skb_mirror = copy_pwan_skb(skb, pNetPriv->netIdx) ) ){
        macSend(WAN2LAN_CH_ID, skb_mirror);
        skb_mirror = NULL;
    }

    if(SOFT_LOOPBACK_MODE(pNetPriv)) {
        skb->protocol = eth_type_trans(skb, dev) ;
        skb->ip_summed = CHECKSUM_UNNECESSARY ;
        dev->last_rx = jiffies ;
        netif_rx(skb) ;
        goto post_qdma_tx;
        
    }
    
    if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_OFF) {
        GOTO_DROP(2);
    } 

#if defined(TCSUPPORT_CT_SWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
if((PWAN_IF_DATA == pNetPriv->netIdx ) && sw_pktqosEnqueue){
    #ifndef TCSUPPORT_CPU_EN7521
    if(isHighestPriorityPacket(skb) ){
        skb->mark &= ~QOS_FILTER_MARK;
        skb->mark |= 0x10;
    }
    #endif
    if(sw_pktqosEnqueue(skb) == 1)
        return NETDEV_TX_OK;
        
}
#endif



    #ifdef  CONFIG_8021P_REMARK
#if !defined(TCSUPPORT_XPON_HAL_API_EXT) && !defined(TCSUPPORT_FWC_ENV) && !defined(TCSUPPORT_XPON_HAL_API)
    skb=vlanPriRemark(skb);
    if(skb==NULL){
        printk("(%s)802.1p remark failure\r\n", __FUNCTION__);
        return NETDEV_TX_OK;
    }
#endif
    #endif
#if defined(TCSUPPORT_CT_PON) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
    #if defined(TCSUPPORT_QOS) && !defined(TCSUPPORT_CPU_EN7521)
    txq = queue_policy(skb);
    if(-1 == txq ) {
        GOTO_DROP(2);
    }
    #endif
#endif


#ifdef TCSUPPORT_WAN_GPON
    if((gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON) && (GPON_CURR_STATE == GPON_STATE_O5)) {
        if(gwan_prepare_tx_message(&txBmMsg, pNetPriv->netIdx, skb, txq, &xpon_info) != 0)
        {
            GOTO_DROP(2);
        }
        gemport=skb->gem_port;
    } else 
#endif /* TCSUPPORT_WAN_GPON */
#ifdef TCSUPPORT_WAN_EPON
    if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_EPON) {
        if(ewan_prepare_tx_message(&txBmMsg, pNetPriv->netIdx, skb, txq, &xpon_info) != 0)  
        {
            GOTO_DROP(2);
        }
    } else 
#endif /* TCSUPPORT_WAN_EPON */
    {
        PON_MSG(MSG_ERR, "The xPON link status is off.\n") ;
        GOTO_DROP(2);
    }

#ifdef XPON_MAC_CONFIG_DEBUG
    if(gpPonSysData->debugLevel & MSG_CONTENT) {
        PON_MSG(MSG_CONTENT, "TX SKB LEN:%d, TX MSG: %.8x, %.8x", skb->len, txBmMsg.word[0], txBmMsg.word[1]) ;
        __dump_skb(skb, skb->len) ;
    }
#endif
    
    if(PWAN_IF_DATA != pNetPriv->netIdx) {
        if(((gpPonSysData->sysLinkStatus == PON_LINK_STATUS_EPON) && (pNetPriv->netIdx == PWAN_IF_OAM))||
            ((gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON) && (pNetPriv->netIdx == PWAN_IF_OMCI))){ 
            /* make the OAM packet to NO_DROP_PACKET */
        #ifndef TCSUPPORT_CPU_EN7521
            skb->mark |= QOS_NODROP_MARK ;
        #endif

            if(gpPonSysData->isUpOmciOamTraffic == XPON_DISABLE) {
                GOTO_DROP(2);
            }
        }
        else{
#ifdef XPON_MAC_CONFIG_DEBUG
            if(gpPonSysData->debugLevel & MSG_CONTENT){
                PON_MSG(MSG_CONTENT, "%s(%d)sysLinkStatus = %d, netIdx=%d.\n",
                    __func__, __LINE__, gpPonSysData->sysLinkStatus, pNetPriv->netIdx);
                __dump_skb(skb, skb->len);
            }
#endif
            GOTO_DROP(2);
        }
    } else {
        if(gpPonSysData->isUpDataTraffic == XPON_DISABLE) {
            GOTO_DROP(2);
        }

#ifndef TCSUPPORT_CPU_EN7521
        /* limit the buffer usage for tx packet, it's used to reserve DSCP for OAM packet */
        if(!is_crtl_data_pkt  && (gpWanPriv->devCfg.txPacketCount >= PWAN_MAX_PACKET_COUNT ) ) {
            GOTO_DROP(2);
        } else {
            inc_pwan_tx_data_cnt();
        }
#if defined(TCSUPPORT_CT_SWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)   
        if(qos_stop_enable){
            if (gpWanPriv->devCfg.txPacketCount >= (qos_stop_num))
            {
                if(sw_PKTQOS_SET_STOP)
                    sw_PKTQOS_SET_STOP();
            }
        }
#endif
#endif
        /* padding for the packet that length less than 64 byte */
        if(unlikely(skb->len < ETH_ZLEN)) {
            if(skb_padto(skb, ETH_ZLEN)) {
                GOTO_DROP(1);
            }
            skb_put(skb, ETH_ZLEN-skb->len) ;
        }
    } 

    if(!dummyPkt && mirror_post_tx_prepare() ){
        skb_mirror = copy_pwan_skb(skb, pNetPriv->netIdx);
    }

    pktLens= skb->len ;
    
    /* call QDMA API to transmit the packet */
#ifdef TCSUPPORT_CPU_EN7521
    ret = QDMA_API_TRANSMIT_PACKETS(ECNT_QDMA_WAN, skb, txBmMsg.word[0], txBmMsg.word[1], &xpon_info) ;
#else
    ret = qdma_transmit_packet(&txBmMsg, sizeof(txBmMsg), skb) ;
#endif
    /* 
       !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        skb should not be used blow this line !!!
       !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    */
    #ifdef TCSUPPORT_EPON_DUMMY
    if(dummyPkt){
        if(0 == ret ){
            atomic_inc(&gDummyDscpCount);
            gDummySent ++;
        }else{
            ++ gDummyDrop_QDMA_TX_FAIL;
        }
    } else{
        start_epon_dummy();
    }
    #endif

    if(ret != 0)  /* when drop keepalive packet,ret = 1 */
    {
        if(ret < 0) 
            PON_MSG(MSG_ERR, "Transmit packet to QDMA failed.\n") ;
        
        if (pNetPriv->netIdx==PWAN_IF_DATA){
            GOTO_DROP(1);
        }
        else{
            GOTO_DROP(2);
        }
    }
    else /* update packet conuter and length */
    { 
        pNetPriv->stats.tx_packets++ ;
        pNetPriv->stats.tx_bytes += pktLens ;

#ifdef TCSUPPORT_WAN_GPON
        if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON) 
        {
            if (pNetPriv->netIdx == PWAN_IF_OMCI)
                gemIdx = (gpWanPriv->gpon.gemIdToIndex[GPON_OMCC_ID] & GPON_GEM_IDX_MASK) ; 
            else
                gemIdx = (gemport) ? (gpWanPriv->gpon.gemIdToIndex[gemport] & GPON_GEM_IDX_MASK) : CONFIG_GPON_MAX_GEMPORT ;
            if(gemIdx < CONFIG_GPON_MAX_GEMPORT) {
                gpWanPriv->gpon.gemPort[gemIdx].stats.tx_packets++ ;
                gpWanPriv->gpon.gemPort[gemIdx].stats.tx_bytes += pktLens ;
            }
        }
#endif /* TCSUPPORT_WAN_GPON */
    }

post_qdma_tx:
    
    if(!dummyPkt && mirror_post_tx_prepare() && skb_mirror){
        macSend(WAN2LAN_CH_ID, skb_mirror);
		skb_mirror = NULL ;
    }


#if !defined(TCSUPPORT_C9_ROST_LED)
#if defined(TCSUPPORT_XPON_LED) && defined(TR068_LED)
    if(pNetPriv->netIdx == PWAN_IF_DATA){
        /*for interner traffic led*/
        if(internet_led_on) //IP connected and IP traffic is passing
            ledTurnOn(LED_INTERNET_ACT_STATUS);
        else
        {
            if(!internet_trying_led_on) 
            {
                ledTurnOff(LED_INTERNET_STATUS);
                ledTurnOff(LED_INTERNET_TRYING_STATUS);
            }
        }
    }
#endif
#endif

    dev->trans_start = jiffies ;
    return ret ;
    
drop1:
    dec_pwan_tx_data_cnt();
            
drop2:
    pNetPriv->stats.tx_dropped++ ;
    if( skb && !dummyPkt )
        dev_kfree_skb_any(skb) ;


    if(skb_mirror) dev_kfree_skb_any(skb_mirror);
    return ret ;
#undef GOTO_DROP
}

EXPORT_SYMBOL(pwan_net_start_xmit);

/*****************************************************************
*****************************************************************/
static struct net_device_stats *pwan_net_get_stats(struct net_device *dev)
{
	PWAN_NetPriv_T *pNetPriv ;

	pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev) ;
	return &pNetPriv->stats ;
}

/*****************************************************************
*****************************************************************/
static void pwan_net_set_multicast_list(struct net_device *dev)
{
	return; /* Do nothing */
}

/*****************************************************************
*****************************************************************/
static int pwan_net_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	int ret = 0 ;
	
	switch (cmd) {
		default :
			break ;
	}
		
	return ret ;
}

/*****************************************************************
*****************************************************************/
static int pwan_net_set_macaddr(struct net_device *dev, void *p)
{
	struct sockaddr *addr = p;

	/* Check if given address is valid ethernet MAC address */
  	if (!is_valid_ether_addr(addr->sa_data))
    	return(-EIO);

	/* Save the customize mac address */
  	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);

	return 0;
}

#ifdef CONFIG_USE_RX_NAPI
/*****************************************************************
*****************************************************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
static int pwan_net_poll(struct napi_struct *napi, int budget)
#else
static int pwan_net_poll(struct net_device *dev, int *budget)
#endif /* KERNEL_VERSION(2,6,26) */
{
	int npackets = 0 ;
	int done = 0 ;
	ulong flags ;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
	int quota = budget ;
#else
	int quota = min(dev->quota, *budget) ;
#endif /* KERNEL_VERSION(2,6,26) */

#ifdef TCSUPPORT_CPU_EN7521
	npackets = QDMA_API_RECEIVE_PACKETS(ECNT_QDMA_WAN, quota) ;
#else
	npackets = qdma_receive_packets(quota) ;
#endif
	if(npackets < quota) {
		done = 1 ;
		
		spin_lock_irqsave(&gpWanPriv->rxLock, flags) ;
		
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
		__napi_complete(napi);
#else
		__netif_rx_complete(dev) ;
#endif /* KERNEL_VERSION(2,6,26) */

#ifdef TCSUPPORT_CPU_EN7521
		QDMA_API_ENABLE_RXPKT_INT(ECNT_QDMA_WAN) ;
#else
		qdma_enable_rxpkt_int() ;
#endif
		spin_unlock_irqrestore(&gpWanPriv->rxLock, flags) ;
	} else {
		done = 0 ;
	}	
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
	return npackets ;
#else 
	*budget -= npackets ;
	dev->quota -= npackets ;
	return done ? 0 : 1 ;
#endif /* KERNEL_VERSION(2,6,26) */
}
#endif /* CONFIG_USE_RX_NAPI */


/**********************************************************************************
 net_device_ops2.6.29net_deviceoperationX
 net_device_opsc
**********************************************************************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
const struct net_device_ops xpon_netdev_ops = {
	.ndo_open				= pwan_net_open,
	.ndo_stop				= pwan_net_stop,
#if !defined(TCSUPPORT_XPON_HYBIRD) 
	.ndo_start_xmit			= pwan_net_start_xmit,
#endif
	.ndo_get_stats 			= pwan_net_get_stats,
#if !KERNEL_3_18_21
	.ndo_set_multicast_list 	= pwan_net_set_multicast_list,
#endif
	.ndo_do_ioctl          	 	= pwan_net_ioctl,
	.ndo_set_mac_address		= pwan_net_set_macaddr,
};
#endif /* KERNEL_VERSION(2,6,29) */


/*****************************************************************
*****************************************************************/
int pwan_create_net_interface(uint idx)
{
	unchar mac_addr[ETH_ADDR_LEN] = PWAN_BASE_MAC_ADDR ;
	unchar flashMacAddr[ETH_ADDR_LEN], i;
    PWAN_NetPriv_T *pNetPriv ;
	unchar if_name[16] ;

	if(idx >= PWAN_IF_NUMS) {
		return -EINVAL ;
	}

	if(gpWanPriv->pPonNetDev[idx]) {
		return -EEXIST ;
	}

#if !defined(TCSUPPORT_FWC_ENV) 
    	for (i = 0; i < ETH_ADDR_LEN; i++)
      		flashMacAddr[i] = READ_FLASH_BYTE(flash_base + 0xff48 + i);
#endif
	if (
		flashMacAddr[0] == 0 && flashMacAddr[1] == 0 && flashMacAddr[2] == 0 &&
		flashMacAddr[3] == 0 && flashMacAddr[4] == 0 && flashMacAddr[5] == 0
	)
		printk(KERN_INFO "\nThe MAC address in flash is null! Use default!\n");		
	else	
		memcpy(mac_addr, flashMacAddr, 6);
	
	memset(if_name, 0, 16) ;
	if(idx == PWAN_IF_OAM) {
		sprintf(if_name, "oam") ;
	} else if(idx == PWAN_IF_OMCI) {
		sprintf(if_name, "omci") ;
	} else
    {
#if !defined(TCSUPPORT_XPON_HAL_API_EXT) 
		sprintf(if_name, "pon") ;
#endif
	} 
#if KERNEL_3_18_21
	gpWanPriv->pPonNetDev[idx] = alloc_netdev(sizeof(PWAN_NetPriv_T), if_name, NET_NAME_USER,ether_setup);
#else	
	gpWanPriv->pPonNetDev[idx] = alloc_netdev(sizeof(PWAN_NetPriv_T), if_name, ether_setup) ;
#endif
	if(gpWanPriv->pPonNetDev[idx] == NULL) {
		PON_MSG(MSG_ERR, "Alloc net interface device %d (alloc_netdev()) failed\n", idx) ;
		return -ENOMEM ;
	}

  	pNetPriv = netdev_priv(gpWanPriv->pPonNetDev[idx]) ;
	memset(pNetPriv, 0, sizeof(PWAN_NetPriv_T)) ;
	pNetPriv->netIdx = idx ;
	pNetPriv->drvLb = 0 ;
 	memset(&pNetPriv->stats, 0, sizeof(struct net_device_stats)) ;
	gpWanPriv->pPonNetDev[idx]->addr_len = ETH_ADDR_LEN ;
	memcpy(gpWanPriv->pPonNetDev[idx]->dev_addr, mac_addr, ETH_ADDR_LEN) ;
	gpWanPriv->pPonNetDev[idx]->mtu = PWAN_MAX_MTU ;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
	gpWanPriv->pPonNetDev[idx]->netdev_ops = &xpon_netdev_ops;
#else
	gpWanPriv->pPonNetDev[idx]->open = pwan_net_open ;
	gpWanPriv->pPonNetDev[idx]->stop = pwan_net_stop ;
#if !defined(TCSUPPORT_XPON_HYBIRD) 
	gpWanPriv->pPonNetDev[idx]->hard_start_xmit = pwan_net_start_xmit ;
#endif
	gpWanPriv->pPonNetDev[idx]->get_stats = pwan_net_get_stats ;
	gpWanPriv->pPonNetDev[idx]->set_multicast_list = pwan_net_set_multicast_list ;
	gpWanPriv->pPonNetDev[idx]->do_ioctl = pwan_net_ioctl ;
	gpWanPriv->pPonNetDev[idx]->set_mac_address = pwan_net_set_macaddr;
#endif /* KERNEL_VERSION(2,6,29) */

#ifdef CONFIG_USE_RX_NAPI
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
	pNetPriv->napi.weight = 128 ;
	netif_napi_add(gpWanPriv->pPonNetDev[idx], &pNetPriv->napi, pwan_net_poll, 128) ;
#else
	gpWanPriv->pPonNetDev[idx]->poll = pwan_net_poll ;
	gpWanPriv->pPonNetDev[idx]->weight = 128 ;
#endif /* KERNEL_VERSION(2,6,26) */
#endif /* CONFIG_USE_RX_NAPI */

	if(idx==PWAN_IF_OAM || idx==PWAN_IF_OMCI) {
		gpWanPriv->pPonNetDev[idx]->flags |= IFF_NOARP ;
		gpWanPriv->pPonNetDev[idx]->flags |= IFF_BROADCAST ;
		gpWanPriv->pPonNetDev[idx]->flags &= ~IFF_MULTICAST ;
#if !KERNEL_3_18_21
		gpWanPriv->pPonNetDev[idx]->features |= NETIF_F_NO_CSUM ;
#endif
	}

	if(register_netdev(gpWanPriv->pPonNetDev[idx]) != 0) {
		PON_MSG(MSG_ERR, "Register net device %s failed\n", if_name) ;
		free_netdev(gpWanPriv->pPonNetDev[idx]) ;
		gpWanPriv->pPonNetDev[idx] = NULL ;
		return -EFAULT ;
	}
	
	return 0 ;
}

static int pwan_net_del(struct net_device *dev)
{

#if defined(TCSUPPORT_XPON_LED) && defined(TR068_LED)
#if !defined(TCSUPPORT_XPON_HAL_API_EXT) 
	if((strcmp(dev->name, "pon") == 0) && (dev->flags & IFF_UP))
#endif
	{
		del_timer_sync(&gpWanPriv->pwan_timer);
	}
#endif

#if (defined CONFIG_USE_RX_NAPI) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	{	
		PWAN_NetPriv_T *pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev) ;
		netif_napi_del(&pNetPriv->napi) ;
	
	}
#endif /* (defined CONFIG_USE_RX_NAPI) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)) */

	return 0 ;


}
/*****************************************************************
*****************************************************************/
int pwan_delete_net_interface(uint idx)
{
	if(idx >= PWAN_IF_NUMS) {
		return -EFAULT ;
	}
	
	if(!gpWanPriv->pPonNetDev[idx]) {
		return -EEXIST ;
	}
	
	pwan_net_del(gpWanPriv->pPonNetDev[idx]);

	unregister_netdev(gpWanPriv->pPonNetDev[idx]) ;
	free_netdev(gpWanPriv->pPonNetDev[idx]) ;
	gpWanPriv->pPonNetDev[idx] = NULL ;
 	
 	return 0 ;
}


/*****************************************************************************
******************************************************************************
   callback function for QDMA buffer management 
******************************************************************************
******************************************************************************/

#ifdef TCSUPPORT_MERGED_DSCP_FORMAT
/*****************************************************************************
******************************************************************************/
int pwan_cb_prepare_tx_message(PWAN_FETxMsg_T *pMsg, struct sk_buff *skb)
{
	PWAN_NetPriv_T *pNetPriv = (PWAN_NetPriv_T *)netdev_priv(skb->dev) ;
	int ret = -1 ;
	int txq = 0;
	struct port_info xpon_info; /* only used in gwan_prepare_tx_message() */
	memset(&xpon_info, 0, sizeof(struct port_info)) ;

#if defined(TCSUPPORT_CT_PON) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
#ifdef TCSUPPORT_QOS
	switch (qosFlag)
	{
		case QOS_HW_WRR:
			/* HW WRR mode */
	#if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
			if(isDefaultQueue(skb)){		//default queue
				txq = 0;
			}else if(isWeigt0(skb)){
				pNetPriv->stats.tx_dropped++ ;
				return ret ;
			}else
	#endif
			if ((skb->mark & QOS_FILTER_MARK) == QOS_HH_PRIORITY)
				txq = 3;
			else if ((skb->mark & QOS_FILTER_MARK) == QOS_H_PRIORITY)
				txq = 2;
			else if ((skb->mark & QOS_FILTER_MARK) == QOS_M_PRIORITY)
				txq = 1;
			break;
		case QOS_HW_PQ:
			/* HW PQ mode */
	#if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
			if(isDefaultQueue(skb)){		//default queue
				txq = 0;
			}else
	#endif
			if (txq < 3 && (skb->mark & QOS_FILTER_MARK) == QOS_HH_PRIORITY)
				txq = 3;
			else if (txq < 2 && (skb->mark & QOS_FILTER_MARK) == QOS_H_PRIORITY)
				txq = 2;
			else if (txq < 1 && (skb->mark & QOS_FILTER_MARK) == QOS_M_PRIORITY)
				txq = 1;
			break;
#if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
		case QOS_HW_CAR:			
			if(isDefaultQueue(skb)){		//default queue
				txq = 0;
			}else{
				txq = (skb->mark & QOS_FILTER_MARK)>> 4;
			}
			break;
#endif
		case NULLQOS: 
			/* for putting rtp packets to HH priority 
			 * when qosFlag is not selected as WRR or PQ*/
			if ((skb->mark & QOS_FILTER_MARK) == QOS_HH_PRIORITY)
				txq = 3;
			break;
		default:
			break;
	}
#endif
#endif



#ifdef TCSUPPORT_WAN_GPON
	if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON) {
		ret = gwan_prepare_tx_message(pMsg, pNetPriv->netIdx, skb, txq, &xpon_info) ;
	} else 
#endif /* TCSUPPORT_WAN_GPON */
#ifdef TCSUPPORT_WAN_EPON
	if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_EPON) {
		ret = ewan_prepare_tx_message(pMsg, pNetPriv->netIdx, skb, txq, &xpon_info) ;	
	} else 
#endif /* TCSUPPORT_WAN_EPON */
	{
		return ret ;
	}
	
#ifdef XPON_MAC_CONFIG_DEBUG
	if(gpPonSysData->debugLevel & MSG_CONTENT) {
		PON_MSG(MSG_CONTENT, "TX SKB LEN:%d, TX MSG: %.8x, %.8x", skb->len, pMsg->word[0], pMsg->word[1]) ;
		__dump_skb(skb, skb->len) ;
	}
#endif
	
	return ret ;
}
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */

/*****************************************************************************
******************************************************************************/
#ifndef TCSUPPORT_CPU_EN7521
/* skb recycle will finish automatically by QDMA2 driver at EN7512
 */
int pwan_cb_tx_finished(void *pMsg, struct sk_buff *skb)
{
	PWAN_NetPriv_T *pNetPriv=NULL;
 
 	if(skb == NULL){
		XPON_DPRINT_MSG("skb is null\n");
		return -1;
	}
	
	if(skb->dev==NULL){
		XPON_DPRINT_MSG("skb->dev is null , need to free the skb\n");
		if(skb)			dev_kfree_skb_any(skb) ;
		return -1;
	}
	
#ifdef TCSUPPORT_EPON_DUMMY	
    if(isDummyPkt(skb)){
        atomic_dec(&gDummyDscpCount);
        gDummyRecyle ++;
    }
#endif

	pNetPriv = (PWAN_NetPriv_T *)netdev_priv(skb->dev) ;

	if(pNetPriv && pNetPriv->netIdx!=PWAN_IF_OMCI && pNetPriv->netIdx!=PWAN_IF_OAM) {
		dec_pwan_tx_data_cnt();
	}

	if(skb && !isDummyPkt(skb) )
        dev_kfree_skb_any(skb) ;

	return 0 ;
}
#endif

/*****************************************************************************
******************************************************************************/
#if defined(TCSUPPORT_VOIP) && defined(RTP_RX_SHORTCUT)
extern unsigned int* (*recv_rtp_src_port_get_hook)(void);
#define DHCP_MODE 1
#define PPPOE_MODE 2

/*______________________________________________________________________________
**function name: checkUdpPacketIsRtp
**
**description:
*    check whether the udp packet is rtp or not
**parameters:
*    char *cp, int flag
**global:
*    none
**return:
*    int ret
**call:
*    none
**revision:
*     1.shelven.lu 20150331
**____________________________________________________________________________*/

int checkUdpPacketIsRtp(unsigned char *cp, int flag)
{ 

	int i=0;
	unsigned short srcPortNum=0;
#define MAX_SESSION_NUM 6
	unsigned int *rtp_src_port=NULL;
	int ret=0;
	
	cp += 11;
	srcPortNum = *((u16*)cp);
	if(recv_rtp_src_port_get_hook){
		rtp_src_port=recv_rtp_src_port_get_hook();			
		
		for(i=0; i<MAX_SESSION_NUM; i++)
		{
			if(srcPortNum ==  rtp_src_port[i])
			{
				ret= flag;/*1: dhcp mode, 2: pppoe mode, recv udp packet src port match, is RTP  packet*/
				break;
			}
		}
	}
#undef MAX_SESSION_NUM
return ret;
	
}

/*______________________________________________________________________________
**function name: isRtpPacket
**
**description:
*    check the packet is rtp or not
**parameters:
*    struct sk_buff *skb, unsigned char *vlanLayer, unsigned short *vlanId
**global:
*    none
**return:
*    int ret
**call:
*    checkUdpPacketIsRtp
**revision:
*     1.shelven.lu 20150331
**____________________________________________________________________________*/

int isRtpPacket(struct sk_buff *skb, unsigned char *vlanLayer, unsigned short *vlanId)
{
	unsigned char *cp = skb->data;	
	unsigned char tagNum = 0;
	unsigned short tpid = 0;
	unsigned short etherType= 0;
	int ret=0;
	cp += 12;

	while((tpid =((*cp << 8) + *(cp + 1))) == 0x8100 ||(tpid == 0x88a8)||(tpid == 0x9100))
	{
		tagNum ++;
		if(tagNum == 1)
			*vlanId = *(u16*)(cp+2);
		cp += 4;
	}

	*vlanLayer = tagNum;	
	etherType = *(u16*)cp;	
	/* check ftp test packet */
	if (etherType == htons(ETH_P_IP)) {/* IP */
		cp = skb->data;
		cp += (23+(tagNum<<2));
		if (*cp == 0x11) { /*0x11 for udp ,0x6 for tcp*/
			ret= checkUdpPacketIsRtp(cp,DHCP_MODE);
		}
	}
	else if(etherType == htons(ETH_P_PPP_SES))
	{
		/* pppoe session */
		cp = skb->data;
		cp += (20+(tagNum<<2));
		if(*(u16*)cp == 0x0021)
		{
			/* pppoe session ip protocol */
			cp += 11;
			if (*cp == 0x11) {  /*0x11 for udp ,0x6 for tcp*/
				ret= checkUdpPacketIsRtp(cp,PPPOE_MODE);
			}
		}
	}

	return ret;
}

#endif


#if defined(TCSUPPORT_CPU_PERFORMANCE_TEST)
int pwan_tls_pon_passthrough(struct sk_buff *skb)
{
    struct net_device *dev ;
    dev = gpWanPriv->pPonNetDev[(PWAN_IfType_t)PWAN_IF_DATA] ;
    pwan_net_start_xmit(skb, dev) ;
    return 0;
}
EXPORT_SYMBOL(pwan_tls_pon_passthrough) ;
#endif

#if defined(TCSUPPORT_CPU_PERFORMANCE_TEST)
#define FTP_PORT_NUM 10
extern unsigned int test_ftp_port[FTP_PORT_NUM];
int isFtpTestPacket(struct sk_buff *skb, unsigned char *vlanLayer, unsigned short *vlanId)
{
	unsigned char *cp = skb->data;	
	unsigned char tagNum = 0;
	unsigned short tpid = 0;
	unsigned short etherType= 0;
	unsigned short srcPortNum = 0;
	cp += 12;
	while((tpid =((*cp << 8) + *(cp + 1))) == 0x8100 ||(tpid == 0x88a8)||(tpid == 0x9100))
	{
		tagNum ++;
		if(tagNum == 1)
			*vlanId = *(u16*)(cp+2);
		cp += 4;
	}	
	*vlanLayer = tagNum;	
	etherType = *(u16*)cp;	
	/* check ftp test packet */
	if (etherType == htons(ETH_P_IP)) {/* IP */
		cp = skb->data;
		cp += (23+tagNum*4);
		
		if (*cp == 0x06) { /* tcp */
				int i=0;
			
				cp += 11;
				srcPortNum = *((u16*)cp);
				
				for(i=0; i<FTP_PORT_NUM; i++)
				{
					if(srcPortNum ==  test_ftp_port[i])
					{
						return 1;// is ftp test packet, tcp dst port match.
					}
				}

		}
	}
	else if(etherType == htons(ETH_P_PPP_SES))
	{
		/* pppoe session */
		cp = skb->data;
		cp += (20+tagNum*4);
		if(*(u16*)cp == 0x0021)
		{
			/* pppoe session ip protocol */
			cp += 11;
			if (*cp == 0x06) { /* tcp */
				int i=0;
			
				cp += 11;
				srcPortNum = *((u16*)cp);
				
				for(i=0; i<FTP_PORT_NUM; i++)
				{
					if(srcPortNum ==  test_ftp_port[i])
					{
						return 2;//pppoe mode, is ftp test packet, tcp dst port match.
					}
				}
			}
		}
	}

	return 0;
}


#endif


int getVlanType(unsigned short TPID)
{
	switch(TPID)
	{
		case ETH_P_8021Q:
			return ETH_P_8021Q;
		case 0x88a8:
			return ETH_P_8021Q;
		case 0x9100:
			return ETH_P_8021Q;
		case 0xAABB:      //only for test
			return ETH_P_8021Q;
		default:
			return -1;
	}
}

int fb_pwan_rx_vlan_proc(struct sk_buff *skb)
{
	return 0;
}


int fb_pwan_tx_vlan_proc(struct sk_buff *skb)
{
	return 0;
}

int pwan_cb_rx_packet(void *pMsg, uint msgLen, struct sk_buff *skb, uint pktLen)
{
#if (defined(TCSUPPORT_VOIP) && defined(RTP_RX_SHORTCUT)) || (defined(TCSUPPORT_CPU_PERFORMANCE_TEST)) || defined(TCSUPPORT_CT_PON_GDV20) || defined(TCSUPPORT_IS_FH_PON)

	unsigned char vlanLayer=0;
	unsigned short vlanId=0;
	int isRtpFlag = 0; /*1: ip mode rtp  packet 2: pppoe mode rtp  packet*/
	int ftpTestFlag = 0; // 1: ip mode ftp test packet 2: pppoe mode ftp test packet
#endif
	char netIdx=0xFF ;
	PWAN_NetPriv_T *pNetPriv = NULL ;
	PWAN_FERxMsg_T *pRxBmMsg_tmp = (PWAN_FERxMsg_T *)pMsg ; 
	struct sk_buff *tmpSkb, *newSkb = NULL ;	
	struct net_device *dev ;
	unchar LbFlag = 0 ;
	int ret = 0;
	unsigned char traffic_classify_flag = E_ECNT_FORWARD;
	
	PWAN_FERxMsg_T pRxBmMsg_s;
	PWAN_FERxMsg_T *pRxBmMsg = &pRxBmMsg_s;
	memcpy(pRxBmMsg, pRxBmMsg_tmp, sizeof(PWAN_FERxMsg_T));
	
#ifdef TCSUPPORT_RA_HWNAT
	int ppeMagic = 0 ;
#endif /* TCSUPPORT_RA_HWNAT */
#if defined(TCSUPPORT_L2_PPP_MCAST_SHORTCUT)
	int isShortCut = 0;
#endif
	int free_skb_flag = 0;
	
#ifndef TCSUPPORT_CPU_EN7521
#ifdef TCSUPPORT_MERGED_DSCP_FORMAT
	ret = pwan_prepare_rx_buffer(NULL, skb);
#else
	ret = pwan_prepare_rx_buffer(pRxBmMsg_tmp, skb);
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */
	if(ret != 0){
		if( ret == SKB_ALLOC_FAIL){
			/*skb == NULL*/
			gpWanPriv->dropForHookBuf++ ;
		}else {
			gpWanPriv->dropUnknownPackets++ ;
		}
		return -1;
	}
#endif

	if( pRxBmMsg->raw.oam )
		skb = replace_oam_pkt(skb, pktLen);
#ifdef XPON_MAC_CONFIG_DEBUG	
	if(gpPonSysData->debugLevel & MSG_CONTENT) {
		PON_MSG(MSG_CONTENT, "RX PKT LEN: %d, Rx Msg: %.8x, %.8x", pktLen, pRxBmMsg->word[0], pRxBmMsg->word[1]) ;
		__dump_skb(skb, pktLen) ;
	}
#endif

#ifdef TCSUPPORT_LAN_VLAN
    skb->lan_vlan_tci = 0;
    skb->lan_vlan_tci_valid = 0;
#endif
	/* 1. process the message information                          *
	 * 2. calculate the netIdx value from GEM port or LLID value   */
	/* parer the packet to corresponding interface */
#ifdef TCSUPPORT_WAN_GPON
	if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON) {
		if((netIdx = gwan_process_rx_message(pRxBmMsg, skb, pktLen, &LbFlag))<0 || (netIdx!=PWAN_IF_OMCI && (netIdx&0x7)!=PWAN_IF_DATA)) {
			if ((netIdx +1+PWAN_IF_DATA) < 0)
			{
				/*broadcast, unknown unicast, multicast*/
				ret = (netIdx +1+PWAN_IF_DATA) ;
			}
			goto drop ;
		}
#ifdef TCSUPPORT_RA_HWNAT
		ppeMagic = FOE_MAGIC_GPON ;
#endif /* TCSUPPORT_RA_HWNAT */
	} else 
#endif /* TCSUPPORT_WAN_GPON */
#ifdef TCSUPPORT_WAN_EPON
	if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_EPON) {
		if((netIdx = ewan_process_rx_message(pRxBmMsg, skb, pktLen, &LbFlag))<0 || (netIdx!=PWAN_IF_OAM && netIdx!=PWAN_IF_DATA)) {
			goto drop ;
		}
#ifdef TCSUPPORT_RA_HWNAT
		ppeMagic = FOE_MAGIC_EPON ;
#endif /* TCSUPPORT_RA_HWNAT */
	} else 
#endif /* TCSUPPORT_WAN_EPON */
	{
		goto drop ;
	}

	PON_MSG(MSG_TRACE, "pwan_cb_rx_packet: netIdx = %d line = %d\n", netIdx, __LINE__);

	/*broadcast, unknown unicast, multicast*/
	if (netIdx > BROADCAST_OFFSET)
	{
		ret = netIdx - PWAN_IF_DATA ;
		netIdx = PWAN_IF_DATA ;
	}
	
#if !defined(TCSUPPORT_XPON_HYBIRD) 
        dev = gpWanPriv->pPonNetDev[(PWAN_IfType_t)netIdx] ;
        pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev) ;
#endif

	if(LbFlag) {
		skb_put(skb, pktLen) ;
		pwan_net_start_xmit(skb, dev) ;
	} else {
		/* prepare new packet buffer for next packet receive */
		if(pktLen < PWAN_MAX_COPY_LENS) {	
			PON_MSG(MSG_TRACE, "The packet length < %d, copy the packet to new skb.\n", PWAN_MAX_COPY_LENS) ;
			/* copy the packet buffer to new skb for short packets */
			tmpSkb = dev_alloc_skb(pktLen + 2) ;
			if(tmpSkb) {
				skb_reserve(tmpSkb, NET_IP_ALIGN) ;
				memcpy(tmpSkb->data, skb->data, pktLen) ;

				newSkb = skb ;
				skb = tmpSkb ;
			}
		}
		
		/* process the packet data to high layer */
		skb_put(skb, pktLen) ;	
#ifndef TCSUPPORT_CPU_EN7521
	if(cpu_protect){
		if ( software_protect==1 ){
			if(!( pRxBmMsg->raw.oam )){
					  if(isCtlDataPkt(skb)!=1 ){	 
						goto drop;
					  }
			}
		}
	}
#endif

#if defined(WAN2LAN)
		/* do wan2lan after skb_put, 
		 * because skb->len starts to have value from it */		
#if defined(TCSUPPORT_WAN2LAN_EXT)
		if(masko_on_off || (masko_ext & 0x02))
#else
		if(masko_on_off)
#endif
		{
			struct sk_buff *skb2 = NULL;
			int len = TX_STAG_LEN;
			const u_char omciHdr[14] = {0x00,0x00,0x00,0x00,0x00,0x01, 0x00,0x00,0x00,0x00,0x00,0x02, 0x88, 0xb5};
			
			if (netIdx == PWAN_IF_OMCI){
				len += 14;	// add dst mac, src mac, ether type
			}
			//Check the skb headroom is enough or not. shnwind 20100121.
			if(skb_headroom(skb) < len)
				skb2 = skb_copy_expand(skb, len, skb_tailroom(skb) , GFP_ATOMIC);
			else
				skb2 = skb_copy(skb, GFP_ATOMIC);
				
			if(skb2 == NULL)
				printk("\nFAILED: wan2lan skb2 allocation in pon rx direction.\n");
			else
			{
				/* For oam: remove header 2 bytes: llid value  */
				if (netIdx == PWAN_IF_OAM){
					skb_pull(skb2, 2) ;
				}else if (netIdx == PWAN_IF_OMCI){
					skb_push(skb2, 14);
					memcpy(skb2->data, omciHdr, 14);
				}
				skb2->mark |= SKBUF_COPYTOLAN;
				macSend(WAN2LAN_CH_ID,skb2); //tc3262_mac_tx
			}
		}

#endif		

#if defined(TCSUPPORT_XPON_LED) && defined(TR068_LED)
#if !defined(TCSUPPORT_C9_ROST_LED)

#if !defined(TCSUPPORT_XPON_HYBIRD) 
        if(pNetPriv->netIdx == PWAN_IF_DATA){
#endif

			/*for interner traffic led*/
			if(internet_led_on) //IP connected and IP traffic is passing
				ledTurnOn(LED_INTERNET_ACT_STATUS);
			else
			{
				if(!internet_trying_led_on) 
				{
					ledTurnOff(LED_INTERNET_STATUS);
					ledTurnOff(LED_INTERNET_TRYING_STATUS);
				}
			}
		}
#endif
#endif
#if defined(TCSUPPORT_L2_PPP_MCAST_SHORTCUT)
		if(pppoeBridgeMultiShortCut == 1)
		{
			isShortCut = pppoe_bridge_multicast_handle(skb);
			if(isShortCut)
				goto MulticastShortCut;
		}
#endif
		skb->dev = dev ;
		dev->last_rx = jiffies ;
#if defined(TCSUPPORT_VOIP) && defined(RTP_RX_SHORTCUT)
		isRtpFlag = isRtpPacket(skb, &vlanLayer, &vlanId);

#endif	
#if defined(TCSUPPORT_CPU_PERFORMANCE_TEST)
		if(isRtpFlag == 0)
		{
			ftpTestFlag = isFtpTestPacket(skb, &vlanLayer, &vlanId);
		}
#endif			

		skb->protocol = eth_type_trans(skb, dev) ;
		skb->ip_summed = CHECKSUM_UNNECESSARY ;
		if (PWAN_IF_OMCI == netIdx ){
			skb->protocol = 0;
		}
		if(wan_speed_test_hook)
		{
			if(wan_speed_test_hook(skb)==SPEED_TEST_SUCCESS)
				return SPEED_TEST_SUCCESS;
		}
#ifdef TCSUPPORT_RA_HWNAT
		if(ra_sw_nat_hook_rxinfo && ppeMagic) {
			ra_sw_nat_hook_rxinfo(skb, ppeMagic, (char *)&(pRxBmMsg->word[1]), 4);
		}
		
		if(ra_sw_nat_hook_rx != NULL) {
			if(ra_sw_nat_hook_rx(skb)) {
                if(PWAN_IF_DATA == netIdx){
                }
#if defined(TCSUPPORT_VOIP) && defined(RTP_RX_SHORTCUT)
                if(isRtpFlag)
                {
                    rtpRxShortCut(skb, vlanLayer, isRtpFlag);
                }
                else
#endif
#if defined(TCSUPPORT_CPU_PERFORMANCE_TEST)
                if(ftpTestFlag)
                {
                    ftpRxShortCut(skb, vlanLayer, vlanId, ftpTestFlag);
                }
                else
#endif
                {

#if defined(TCSUPPORT_XPON_HAL_API_MCST)
                    if(mtk_multicast_data_handle_hook && PWAN_IF_DATA == netIdx)
                    {
#if !defined(TCSUPPORT_XPON_HAL_API_EXT) 
                        if(0 == mtk_multicast_data_handle_hook(skb))
                        {
                            free_skb_flag = 1;
                            goto free_skb;
                        }
#endif
                    }
#endif
		
		if((PWAN_IF_DATA == netIdx)&&(0 == ECNT_HOOK_MULTICAST_DATA_HANLDE(skb)))
		{
			free_skb_flag = 1;
			goto free_skb;
		}

#if 1 //def CONFIG_USE_RX_NAPI
#if defined(WAN2LAN)
#if defined(TCSUPPORT_WAN2LAN_EXT)
			if(masko_ext & 0x08)
			{
				struct sk_buff *skb2 = NULL;
				int len = TX_STAG_LEN;
				const u_char omciHdr[14] = {0x00,0x00,0x00,0x00,0x00,0x01, 0x00,0x00,0x00,0x00,0x00,0x02, 0x88, 0xb5};
				
				if (netIdx == PWAN_IF_OMCI){
					len += 14;	// add dst mac, src mac, ether type
				}
				//Check the skb headroom is enough or not. shnwind 20100121.
				if(skb_headroom(skb) < len)
					skb2 = skb_copy_expand(skb, len, skb_tailroom(skb) , GFP_ATOMIC);
				else
					skb2 = skb_copy(skb, GFP_ATOMIC);
					
				if(skb2 == NULL)
					printk("\nFAILED: wan2lan skb2 allocation in pon rx direction.\n");
				else
				{
					skb_push(skb2,14);
					/* For oam: remove header 2 bytes: llid value  */
					if (netIdx == PWAN_IF_OAM){
						skb_pull(skb2, 2) ;
					}else if (netIdx == PWAN_IF_OMCI){
						skb_push(skb2, 14);
						memcpy(skb2->data, omciHdr, 14);
					}
					skb2->mark |= SKBUF_COPYTOLAN;
					macSend(WAN2LAN_CH_ID,skb2); //tc3262_mac_tx

				}
			}
#endif		
#endif
			#ifdef TCSUPPORT_TLS
			if(-1 == FE_API_TLS_FORWARD(skb,FE_TLS_FORWARD_DOWNSTREAM))
			#endif
				netif_receive_skb(skb) ;
#else
				netif_rx(skb) ;
#endif /* CONFIG_USE_RX_NAPI */

		}
		}
		} else
#endif /* TCSUPPORT_RA_HWNAT */

		{
			if(PWAN_IF_DATA == netIdx){
			}
#if defined(TCSUPPORT_VOIP) && defined(RTP_RX_SHORTCUT)
		if(isRtpFlag)
		{		
			rtpRxShortCut(skb, vlanLayer, isRtpFlag);			
		}
		else
#endif
#if defined(TCSUPPORT_CPU_PERFORMANCE_TEST)
		if(ftpTestFlag)
		{		
			ftpRxShortCut(skb, vlanLayer, vlanId, ftpTestFlag);			
		}
		else
#endif
			{

#if defined(TCSUPPORT_XPON_HAL_API_MCST)
				if(mtk_multicast_data_handle_hook && PWAN_IF_DATA == netIdx)
				{
#if !defined(TCSUPPORT_XPON_HAL_API_EXT) 
					if(0 == mtk_multicast_data_handle_hook(skb))
					{
						free_skb_flag = 1;
						goto free_skb;
					}
#endif
				}
#endif

				if((PWAN_IF_DATA == netIdx)&&(0 == ECNT_HOOK_MULTICAST_DATA_HANLDE(skb)))
				{
					free_skb_flag = 1;
					goto free_skb;
				}

#if 1 //def CONFIG_USE_RX_NAPI
#if defined(WAN2LAN)
#if defined(TCSUPPORT_WAN2LAN_EXT)
				if(masko_ext & 0x08)
				{
					struct sk_buff *skb2 = NULL;
					int len = TX_STAG_LEN;
					const u_char omciHdr[14] = {0x00,0x00,0x00,0x00,0x00,0x01, 0x00,0x00,0x00,0x00,0x00,0x02, 0x88, 0xb5};
					
					if (netIdx == PWAN_IF_OMCI){
						len += 14;	// add dst mac, src mac, ether type
					}
					//Check the skb headroom is enough or not. shnwind 20100121.
					if(skb_headroom(skb) < len)
						skb2 = skb_copy_expand(skb, len, skb_tailroom(skb) , GFP_ATOMIC);
					else
						skb2 = skb_copy(skb, GFP_ATOMIC);
						
					if(skb2 == NULL)
						printk("\nFAILED: wan2lan skb2 allocation in pon rx direction.\n");
					else
					{
						skb_push(skb2,14);
						/* For oam: remove header 2 bytes: llid value  */
						if (netIdx == PWAN_IF_OAM){
							skb_pull(skb2, 2) ;
						}else if (netIdx == PWAN_IF_OMCI){
							skb_push(skb2, 14);
							memcpy(skb2->data, omciHdr, 14);
						}
						skb2->mark |= SKBUF_COPYTOLAN;
						macSend(WAN2LAN_CH_ID,skb2); //tc3262_mac_tx
					}
				}
#endif		
#endif

				netif_receive_skb(skb) ;
#else
				netif_rx(skb) ;
#endif /* CONFIG_USE_RX_NAPI */
			}
		}
	}

#if defined(TCSUPPORT_L2_PPP_MCAST_SHORTCUT)
MulticastShortCut:
#endif

free_skb:
if(free_skb_flag&&skb)
{
	dev_kfree_skb(skb);
	skb = NULL;
}

	/* calculate the statistic counters */
	pNetPriv->stats.rx_packets++ ;
	pNetPriv->stats.rx_bytes += pktLen ;
	
	return ret ;

drop:
	/* Dropped the packet */
	dev_kfree_skb(skb);
	/* calculate the statistic counters */
	gpWanPriv->dropUnknownPackets++ ;
	
	return -EFAULT+ret ;
}

/*****************************************************************************
******************************************************************************/
int pwan_cb_event_handler(QDMA_EventType_t event)
{
	switch(event) {
		case QDMA_EVENT_RECV_PKTS:
#ifdef CONFIG_USE_RX_NAPI
			{
				struct net_device *dev = gpWanPriv->pPonNetDev[PWAN_IF_DATA] ;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
				PWAN_NetPriv_T *pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev) ;
				if(pNetPriv == NULL) {
					return IRQ_HANDLED ;
				}
			
				if(napi_schedule_prep(&pNetPriv->napi)) {
#ifdef TCSUPPORT_CPU_EN7521
					QDMA_API_DISABLE_RXPKT_INT(ECNT_QDMA_WAN) ;
#else /* TCSUPPORT_CPU_EN7521 */
					qdma_disable_rxpkt_int() ;
#endif /* TCSUPPORT_CPU_EN7521 */
					__napi_schedule(&pNetPriv->napi) ;
				}
#else
				if(netif_rx_schedule_prep(dev)) {
#ifdef TCSUPPORT_CPU_EN7521
					QDMA_API_DISABLE_RXPKT_INT(ECNT_QDMA_WAN) ;
#else /* TCSUPPORT_CPU_EN7521 */
					qdma_disable_rxpkt_int() ;
#endif /* TCSUPPORT_CPU_EN7521 */
					__netif_rx_schedule(dev) ;
				}
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26) */
			}
#endif /* CONFIG_USE_RX_NAPI */
			break ;
		
		case QDMA_EVENT_NO_RX_BUFFER:
			break ;
			
		case QDMA_EVENT_TX_CROWDED:
			break ;
		
		default:
			break ;
	}
		
	return 0 ;
}
#if defined(TCSUPPORT_CPU_PERFORMANCE_TEST)
static int pon_add_ftp_port_read_proc(
		char *page, char **start, off_t off,
		int count, int *eof, void *data
){
	int i=0;
	for(i=0; i<FTP_PORT_NUM; i++)
	{
		printk("test ftp port %d: %d\n", i, test_ftp_port[i]);
	}

	return 0;
}

static int pon_add_ftp_port_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data)
{
	char get_buf[8];
	int len;
	int i=0;
	unsigned int add_port = 0;

	if (count > 8)
		len = 8;
	else 
		len = count;
	
	memset(get_buf, 0, sizeof(get_buf));
	
	if(copy_from_user(get_buf, buffer, len - 1))
		return -EFAULT;

	get_buf[len] = '\0';
	sscanf(get_buf, "%d", &add_port);

	if(add_port == 0)
	{
		for(i=0; i<FTP_PORT_NUM; i++)
		{
			test_ftp_port[i] = 0; 
		}
	}
	else
	{
	
		for(i=0; i<FTP_PORT_NUM; i++)
		{
			if(test_ftp_port[i] == 0)
			{
				test_ftp_port[i] = add_port;
			
				printk("open ftp port: %d\n", test_ftp_port[i]);		
				break;
			}
		}
	}
	
	return len;
	
}

static int pon_del_ftp_port_read_proc(
		char *page, char **start, off_t off,
		int count, int *eof, void *data
){
	int i=0;
	for(i=0; i<FTP_PORT_NUM; i++)
	{
		printk("test ftp port %d: %d\n", i, test_ftp_port[i]);
	}

	return 0;
}

static int pon_del_ftp_port_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data)
{
	char get_buf[8];
	int len;
	int i=0;
	unsigned int del_port = 0;
	

	if (count > 8)
		len = 8;
	else 
		len = count;
	
	memset(get_buf, 0, sizeof(get_buf));
	
	if(copy_from_user(get_buf, buffer, len - 1))
		return -EFAULT;

	get_buf[len] = '\0';
	sscanf(get_buf, "%d", &del_port);

	for(i=0; i<FTP_PORT_NUM; i++)
	{
		if(test_ftp_port[i] == del_port)
		{
			test_ftp_port[i] = 0;
			break;
		}
	}
	printk("close ftp port: %d\n", del_port);
	
	return len;
	
}

#endif


#if defined(TCSUPPORT_CT_SWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
static int pon_qos_stop_read_proc(
		char *page, char **start, off_t off,
		int count, int *eof, void *data
){
	printk("%d\n",qos_stop_enable);
	return 0;
}

static int pon_qos_stop_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data)
{
	char get_buf[8];
	int len;

	if (count > 8)
		len = 8;
	else 
		len = count;
	
	memset(get_buf, 0, sizeof(get_buf));
	
	if(copy_from_user(get_buf, buffer, len - 1))
		return -EFAULT;

	get_buf[len] = '\0';

	sscanf(get_buf, "%d", &qos_stop_enable);
	
	return len;
	
}

static int pon_qos_stop_num_read_proc(
		char *page, char **start, off_t off,
		int count, int *eof, void *data
){
	printk("%d\n",qos_stop_num);
	return 0;
}

static int pon_qos_stop_num_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data)
{
	char get_buf[8];
	int len;

	if (count > 8)
		len = 8;
	else 
		len = count;
	
	memset(get_buf, 0, sizeof(get_buf));
	
	if(copy_from_user(get_buf, buffer, len - 1))
		return -EFAULT;

	get_buf[len] = '\0';

	sscanf(get_buf, "%d", &qos_stop_num);
	
	return len;
	
}

#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
extern int xmcs_set_qos_policer_creat(struct XMCS_QosPolicerCreat_S *pQosPolicer);
extern int xmcs_set_qos_policer_delete(struct XMCS_QosPolicerDelete_S *pQosPolicerDelete);
extern int xmcs_set_qos_overall_ratelimit(struct XMCS_OverallRatelimitConfig_S *pOverallRatelimit);
static int qos_creat_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data)
{
	char valString[16];
	struct XMCS_QosPolicerCreat_S PolicerCreat;
	
	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EFAULT;

	valString[count] = '\0';

	sscanf(valString, "%d %d", &PolicerCreat.policer_id, &PolicerCreat.cir);

    xmcs_set_qos_policer_creat(&PolicerCreat);

	return  count;
}

static int qos_delete_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data)
{
	char valString[4];
	struct XMCS_QosPolicerDelete_S PolicerDelete;
	
	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EFAULT;

	valString[count] = '\0';

	sscanf(valString, "%d", &PolicerDelete.policer_id);

    xmcs_set_qos_policer_delete(&PolicerDelete);

	return  count;
}

static int qos_totalbandwidth_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data)
{
	char valString[16];
	struct  XMCS_OverallRatelimitConfig_S OverallRatelimit;
	
	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EFAULT;

	valString[count] = '\0';

	sscanf(valString, "%d", &OverallRatelimit.bandwidth);

    xmcs_set_qos_overall_ratelimit(&OverallRatelimit);

	return  count;
}
#endif
#endif


#if defined(TCSUPPORT_L2_PPP_MCAST_SHORTCUT)
static int pppoe_multicast_shortcut_read_proc(
		char *page, char **start, off_t off,
		int count, int *eof, void *data
){
	printk("pppoe bridge multicast short cut: %d\n",pppoeBridgeMultiShortCut);
	return 0;
}

static int pppoe_multicast_shortcut_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data)
{
	char get_buf[8];
	int len;

	if (count > 8)
		len = 8;
	else 
		len = count;
	
	memset(get_buf, 0, sizeof(get_buf));
	
	if(copy_from_user(get_buf, buffer, len - 1))
		return -EFAULT;

	get_buf[len] = '\0';

	sscanf(get_buf, "%d", &pppoeBridgeMultiShortCut);
	
	return len;
	
}
#endif

/*****************************************************************************
******************************************************************************/
static void pwan_tx_drop_timer(unsigned long data){
	int preTxDropOmcc = gpWanPriv->devCfg.flags.isTxDropOmcc;

#if defined(TCSUPPORT_RA_HWNAT) && defined(TCSUPPORT_RA_HWNAT_ENHANCE_HOOK)
	extern int is_hwnat_dont_clean;
	extern int (*ra_sw_nat_hook_clean_table) (void);
	
	if ((GPON_CURR_STATE == GPON_STATE_O5) && (preTxDropOmcc == 1)){
		PON_MSG(MSG_ACT, "PWAN: Data drop timer for OMCI done, clean HWNAT!\n");
		
		gpWanPriv->devCfg.flags.isTxDropOmcc = 0;
		/* when using dbgmgr test hotplug, the flag will be enable */
		if(likely(gpGponPriv->gponCfg.flags.hotplug == XPON_DISABLE)) {
			is_hwnat_dont_clean = 0;
		}
		if (ra_sw_nat_hook_clean_table){
			ra_sw_nat_hook_clean_table();
		}
		xpon_set_qdma_qos(gpWanPriv->devCfg.flags.isQosUp);
	}
#endif

}


/*****************************************************************************
******************************************************************************/
void pwan_destroy(void)
{
	int i ;
	
	for(i=0 ; i<PWAN_IF_NUMS ; i++) {
		pwan_delete_net_interface(i) ;
    }
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	remove_proc_entry("qos/pon_trafficShapingSet", 0);
#if defined(QOS_REMARKING) || defined(TCSUPPORT_HW_QOS)
	remove_proc_entry("qos/ptm_qoswrr", 0);
#endif
	remove_proc_entry("qos/pon_tcqos_queuemask", 0);
	remove_proc_entry("qos/pon_tcqos_dbg", 0);

#ifdef TCSUPPORT_QOS
	remove_proc_entry("qos/pon_tcqos_disc", 0);
#endif
	remove_proc_entry("qos/qos_stop_enable",NULL);//heredbg
	remove_proc_entry("qos/qos_stop_num",NULL);//heredbg
#else
	
	remove_proc_entry("qos/pon_hw_qos_enable",NULL);//heredbg
	remove_proc_entry("qos",NULL);
	

#endif

	
#ifdef TCSUPPORT_WAN_GPON
    remove_proc_entry("tc3162/service_change", 0);
#endif

#if defined(TCSUPPORT_CPU_PERFORMANCE_TEST)
	rcu_assign_pointer(pwan_tls_pon_passthrough_hook, NULL);
	remove_proc_entry("tc3162/add_ftp_port", 0);
	remove_proc_entry("tc3162/del_ftp_port", 0);
#endif

}

/*****************************************************************************
******************************************************************************/
int pwan_init(void)
{
	int i, ret ;
#if defined(TCSUPPORT_CPU_PERFORMANCE_TEST)
	struct proc_dir_entry *ftpTestProc=NULL;
#endif
	struct proc_dir_entry *ponProc;

	proc_mkdir("qos", NULL);

	for(i=0 ; i<PWAN_IF_NUMS ; i++) {
		gpWanPriv->pPonNetDev[i] = NULL ;
	}
	gpWanPriv->dropUnknownPackets = 0 ;
	gpWanPriv->dropForHookBuf = 0;
	gpWanPriv->activeChannelNum = 1; // default is 1 channel(LLID or T-CONT)
	gpWanPriv->greenMaxthreshold = 15 ;

#ifdef TCSUPPORT_WAN_GPON
	if((ret = gwan_init(&gpWanPriv->gpon)) != 0) {
		PON_MSG(MSG_ERR, "GPON WAN initialization failed\n") ;
		return ret ;
	}
#endif /* TCSUPPORT_WAN_GPON */

#ifdef TCSUPPORT_WAN_EPON
	if((ret = ewan_init(&gpWanPriv->epon)) != 0) {
		PON_MSG(MSG_ERR, "EPON WAN initialization failed\n") ;
		return ret ;
	}
#endif /* TCSUPPORT_WAN_EPON */


	pwan_create_net_interface(PWAN_IF_DATA) ;

		ponProc = create_proc_entry("qos/pon_hw_qos_enable", 0, NULL);
		if (!ponProc)
		{
			printk("\nFAILED: create proc for pon_hw_qos_enable\n");
			return -ENOMEM;
		}
		ponProc->write_proc = pon_hw_qos_enable_write_proc;
		ponProc->read_proc = pon_hw_qos_enable_read_proc;

#if defined(TCSUPPORT_CT_PON) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)		
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	ponProc = create_proc_entry("qos/pon_trafficShapingSet", 0, NULL);
#else 	
	ponProc = create_proc_entry("tc3162/pon_trafficShapingSet", 0, NULL);
#endif 
	if (!ponProc)
	{
		printk("\nFAILED: create proc for pon_trafficShapingSet\n");
		return -ENOMEM;
	}
	ponProc->write_proc = pon_trafficShapingSet_write_proc;
	ponProc->read_proc = pon_trafficShapingSet_read_proc;
#if defined(QOS_REMARKING) || defined(TCSUPPORT_HW_QOS)
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	ponProc = create_proc_entry("qos/pon_qoswrr", 0, NULL);
#else
	ponProc = create_proc_entry("tc3162/pon_qoswrr", 0, NULL);
#endif		
	if (!ponProc)
	{
		printk("\nFAILED: create proc for pon_qoswrr\n");
		return -ENOMEM;
	}
	ponProc->read_proc = pon_qoswrr_read_proc;
	ponProc->write_proc = pon_qoswrr_write_proc;
#endif

#ifdef TCSUPPORT_QOS
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	ponProc = create_proc_entry("qos/pon_tcqos_disc", 0, NULL);
#else 
	ponProc = create_proc_entry("tc3162/pon_tcqos_disc", 0, NULL);
#endif 	
	if (!ponProc)
	{
		printk("\nFAILED: create proc for pon_tcqos_disc\n");
		return -ENOMEM;
	}
	ponProc->write_proc = pon_tcqos_write_proc; 		
#endif

#if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	ponProc->read_proc = pon_tcqos_read_proc;
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	ponProc = create_proc_entry("qos/pon_tcqos_queuemask", 0, NULL);
#else 		
	ponProc = create_proc_entry("tc3162/pon_tcqos_queuemask", 0, NULL);
#endif 	
	if (!ponProc)
	{
		printk("\nFAILED: create proc for pon_tcqos_queuemask\n");
		return -ENOMEM;
	}

	ponProc->read_proc = pon_tcqos_queuemask_read_proc;
	ponProc->write_proc = pon_tcqos_queuemask_write_proc;


#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	ponProc = create_proc_entry("qos/pon_tcqos_dbg", 0, NULL);
#else
	ponProc = create_proc_entry("tc3162/pon_tcqos_dbg", 0, NULL);
#endif

	if (!ponProc)
	{
		printk("\nFAILED: create proc for pon_tcqos_dbg\n");
		return -ENOMEM;
	}
#endif
#if defined(TCSUPPORT_CT_SWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	ponProc = create_proc_entry("qos/qos_stop_enable", 0, NULL);
#else
	ponProc = create_proc_entry("tc3162/qos_stop_enable", 0, NULL);
#endif 	
	if(!ponProc){
		printk("\nFAILED: create proc for qos_stop_enable\n");
		return -ENOMEM;
	}
	ponProc->read_proc = pon_qos_stop_read_proc;
	ponProc->write_proc = pon_qos_stop_write_proc;
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	ponProc = create_proc_entry("qos/qos_stop_num", 0, NULL);
#else 
	ponProc = create_proc_entry("tc3162/qos_stop_num", 0, NULL);
#endif 
	if(!ponProc){
		printk("\nFAILED: create proc for qos_stop_num\n");
		return -ENOMEM;
	}
	ponProc->read_proc = pon_qos_stop_num_read_proc;
	ponProc->write_proc = pon_qos_stop_num_write_proc;
#endif

#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
		ponProc = create_proc_entry("qos/qos_creat", 0, NULL);
		if (!ponProc)
		{
			printk("\nFAILED: create proc for qos_creat\n");
			return -ENOMEM;
		}
		ponProc->write_proc = qos_creat_write_proc; 	

		ponProc = create_proc_entry("qos/qos_delete", 0, NULL);
		if (!ponProc)
		{
			printk("\nFAILED: create proc for qos_delete\n");
			return -ENOMEM;
		}
		ponProc->write_proc = qos_delete_write_proc;	

		ponProc = create_proc_entry("qos/qos_totalbandwidth", 0, NULL);
		if (!ponProc)
		{
			printk("\nFAILED: create proc for qos_totalbandwidth\n");
			return -ENOMEM;
		}
		ponProc->write_proc = qos_totalbandwidth_write_proc; 
#endif

#endif

#if defined(TCSUPPORT_CPU_PERFORMANCE_TEST)
	ftpTestProc = create_proc_entry("tc3162/add_ftp_port", 0, NULL);
	if(!ftpTestProc){
		printk("\nFAILED: create proc for add_ftp_port\n");
		return -ENOMEM;
	}
	ftpTestProc->read_proc = pon_add_ftp_port_read_proc;
	ftpTestProc->write_proc = pon_add_ftp_port_write_proc;

	ftpTestProc = create_proc_entry("tc3162/del_ftp_port", 0, NULL);
	if(!ftpTestProc){
		printk("\nFAILED: create proc for del_ftp_port\n");
		return -ENOMEM;
	}
	ftpTestProc->read_proc = pon_del_ftp_port_read_proc;
	ftpTestProc->write_proc = pon_del_ftp_port_write_proc;	
#endif


#if defined(TCSUPPORT_L2_PPP_MCAST_SHORTCUT)
	ponProc = create_proc_entry("tc3162/pppoemultishortcut", 0, NULL);

	if(!ponProc){
		printk("\nFAILED: create proc for pppoemultishortcut\n");
		return -ENOMEM;
	}
	ponProc->read_proc = pppoe_multicast_shortcut_read_proc;
	ponProc->write_proc = pppoe_multicast_shortcut_write_proc;
#endif

	memset(&gpWanPriv->devCfg, 0, sizeof(PWAN_Config_T)) ;
#ifndef TCSUPPORT_CPU_EN7521
	gpWanPriv->devCfg.txPacketCount = 0 ;
#endif
#if defined(TCSUPPORT_GPON_GREENDROP_OPEN)
  gpWanPriv->devCfg.flags.isQosUp = 1;
#else
#if !defined(TCSUPPORT_CT_C5_HEN_SFU) 
	gpWanPriv->devCfg.flags.isQosUp = 0 ;
#endif
#endif

	
	gpWanPriv->devCfg.flags.isTxDropOmcc = 0;
	init_timer(&gpWanPriv->txDropTimer);
	gpWanPriv->txDropTimer.function = pwan_tx_drop_timer;
	gpWanPriv->txDropTimer.data = 5000;
	
	spin_lock_init(&gpWanPriv->rxLock) ;
#ifndef TCSUPPORT_CPU_EN7521
	spin_lock_init(&gpWanPriv->txLock) ;
#endif

    PON_MSG(MSG_TRACE, "PON WAN interface initialization done\n") ;
#if defined(TCSUPPORT_CPU_PERFORMANCE_TEST)
	rcu_assign_pointer(pwan_tls_pon_passthrough_hook, pwan_tls_pon_passthrough);
#endif


	return 0 ;
}

