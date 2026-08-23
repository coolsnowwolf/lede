#ifndef _PHY_DEF_H_
#define _PHY_DEF_H_
#include <linux/mtd/rt_flash.h>

#include <linux/time.h>
#include <linux/string.h>
#include <linux/jiffies.h>
#include "phy_reg.h"

		/* del_timer_sync cannot be used from interrupt context. */
#define PHY_STOP_TIMER(timer) \
			{ \
				if(in_interrupt()) { \
					del_timer(&timer) ; \
				} else {\
					del_timer_sync(&timer) ; \
				} \
			}

/* Replace line 19 with this: */
#define PHY_START_TIMER(timer) { mod_timer(&timer, (jiffies + msecs_to_jiffies(100))); }

#define ENABLE	1
#define DISABLE	0

#define PHY_TX_LOCK_2_REF						0x1000000
#define PHY_TX_LOCK_2_DATA						0xFEFFFFFF	//by HC 20150522

#define GPON_MAC_RESET	1
#define GPON_MAC_PHY_RESET	2
#define GPON_PHY_TASK_FLAG	3

/*test pattern*/
#define PHY_BIST_IDLE							0x00
#define PHY_BIST_CLOCK							0x01
#define PHY_BIST_CLOCK_DIV5						0x02
#define PHY_BIST_CLOCK_MIX						0x03
#define PHY_BIST_REVRS_LP						0x04
#define PHY_BIST_PRBS7							0x05
#define PHY_BIST_PRBS23							0x06
#define PHY_BIST_PRBS31							0x07
#define PHY_BIST_ALL1							0x09
#define PHY_BIST_ALL0							0x0a
#define PHY_EPON_TEST_PAT						0x0b		//YMC 20150325
#define PHY_EPON_TEST_PAT_0						0x0c		//YMC 20150325


/*PBUS control*/
#define PHY_PBUS_CTRL_DISABLE					(1<<2)

/*transceiver control*/
#define PHY_DISABLE_BURST						(1<<1)

#if !defined(TCSUPPORT_CHS) && !defined(TCSUPPORT_TW_BOARD_CJ) && !defined(TCSUPPORT_CT_PON_C7) && !defined(TCSUPPORT_GPIO40)
#if !defined(TCSUPPORT_CT_PON_CY) && !defined(TCSUPPORT_GPIO25) && !defined(TCSUPPORT_PON_FH) && !defined(TCSUPPORT_CT_PON_C5)
#define PHY_DISABLE_TRANS						(1<<2)
#endif
#endif

#define TOP_LED1_MODE							(1<<10)
#define GPIO_PHY_MODE							(1<<25)
#define GPIO_GPIO2_OUTPUT_MODE					(1<<4)
#define	GPIO_GPIO2_OUTPUT_MASK					0xffffffcf
#define GPIO_GPIO2_OUTPUT_ENABLE				(1<<2)

#define GPIO_GPIO40_OUTPUT_MODE					(1<<16)
#define	GPIO_GPIO40_OUTPUT_MASK					0xfffcffff
#define GPIO_GPIO40_OUTPUT_ENABLE				(1<<8)
/*squelch deglitch*/
#define PHY_SQUELCH_DEGLITCH					(1<<2)

#ifdef TCSUPPORT_CPU_EN7527
#define RG_GPIO_PON_MODE						(1<<21)
#elif TCSUPPORT_CPU_EN7521
#define RG_GPIO_PON_MODE						(1<<15)
#endif
#define RG_PON_I2C_MODE						    (1<<0)


/*
#define GPIO_GPIO15_OUTPUT_MODE					(1<<30)
#define	GPIO_GPIO15_OUTPUT_MASK					0x3fffffff
#define GPIO_GPIO15_OUTPUT_ENABLE				(1<<15)
*/
#define GPIO_GPIO17_OUTPUT_MODE					(1<<2)
#define	GPIO_GPIO17_OUTPUT_MASK					0xfffffff3
#define GPIO_GPIO17_OUTPUT_ENABLE				(1<<17)
#define GPIO_GPIO25_OUTPUT_MODE					(1<<18)
#define	GPIO_GPIO25_OUTPUT_MASK					0xfff3ffff
#define GPIO_GPIO25_OUTPUT_ENABLE				(1<<25)

#define PHY_7570_DISABLE_TRANS				(1<<25)
#define PHY_7571_DISABLE_TRANS				(1<<25)


#define GPIO_GPIO28_OUTPUT_MODE					(1<<24)
#define	GPIO_GPIO28_OUTPUT_MASK					0xfcffffff
#define GPIO_GPIO28_OUTPUT_ENABLE				(1<<28)


/*phy fw ready*/
#define PHY_FW_RDY_EN							1
#define PHY_FW_RDY_MASK							0xfffffffe

/* XPON mode select */
#define	PHY_GPON_MODE							(1<<31)
#define PHY_EPON_MODE							(0)
#define PHY_GPON_DEMASK							(1<<29)
#define PHY_NOTYPE_MODE							(0x00000001)

/*Interrupt*/
#define TRANS_LOS_INT							0x01
#define	PHY_LOF_INT								0x02
#define	TRANS_TF_INT							0x04
#define	TRANS_INT								0x08
#define	TRANS_SD_FAIL_INT						0x10
#define	PHYRDY_INT								0x20
#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
	#define PHY_ILLG_INT						0x40
#endif  /* CONFIG_USE_MT7520_ASIC */
#define	PHY_I2CM_INT							0x80
#define PHY_INT_MASK							0xFFFFFF00

/*Tx Burst mode*/
#define PHY_TX_CONT_MODE_MASK					0xffffff5f
#define PHY_TX_CONT_MODE_OFFSET					0xa0
#define PHY_TX_EPON_CONT_MODE					0xffffffdf

/*Tx Amp setting*/
#define PHY_TX_AMP_LEVEL_LEN_MASK				0xf0
#define PHY_TX_AMP_LEVEL_MASK					0xfff0ffff
#define PHY_TX_AMP_LEVEL_OFFSET					16


/*Tx counter setting*/
#define PHY_TX_CNT_CLR							(1<<2)
#define PHY_TX_CNT_LCH							(1<<3)

/*EPON error frame setting*/
#define PHY_EPON_ERR_FRAME_CLR					(1<<4)
#define	PHY_EPON_ERR_FRAME_LCH					(1<<5)

/*XPON RX config setting*/
#define PHY_RX_SD_INV 							(1<<6)
#define PHY_RX_SD								(0<<6)

/*XPON TX config setting*/
#define PHY_TX_SD_INV_MASK							0xffffffef
#define PHY_TX_SD_INV_OFFSET						4
#define PHY_BURST_EN_INV_MASK						0xffffff7f
#define PHY_BURST_EN_INV_OFFSET						7
#define	PHY_TX_FAULT_INV_MASK						0xffffffdf
#define	PHY_TX_FAULT_INV_OFFSET						5
#define PHY_CSR_TX_SW_RST_MASK						0xfffffffd	//YMC 20150325

/*pll reset*/
#define PHY_PLL_RST								(1<<31)
#define PHY_Software_Reset 						(1<<27)		//60928 Software rest , 20150209 JayYC

/* reset*/
#define PHY_fw_rdy								(1<<0)		//Phy_fw_rdy , 20150318 JayYC

/*counter reset*/
#define PHY_COUNT_RST							(1<<27)

/*eye monitor*/
#define PHY_PILPO_ROUTE							(1<<27)
#define PHY_EYE_MON_CNT_ENABLE					(1<<12)
#define PHY_EYE_MON_ENABLE						(1<<11)

/*bit delay*/
#ifdef	CONFIG_USE_A60901
	#define PHY_DIG_BIT_DELAY_OFFSET			3
	#define	PHY_BIT_DELAY_ADDR					PHY_CSR_PHYSET2
	#define PHY_DIG_BIT_DELAY_MASK				0xffffffc7
#elif defined CONFIG_USE_MT7520_ASIC || defined(CONFIG_USE_A60928)
	#define PHY_DIG_BIT_DELAY_OFFSET			19
	#define	PHY_BIT_DELAY_ADDR					PHY_CSR_PHYSET5
	#define PHY_DIG_BIT_DELAY_MASK				0xff87ffff
#endif /* CONFIG_USE_MT7520_ASIC */

#define PHY_BIT_DELAY_LEN_MASK					0xf8
#define PHY_ANA_BIT_DELAY_OFFSET				24
#define PHY_TX_BIT_DEL_SEL						(1<<23)
#define PHY_ANA_BIT_DELAY_MASK					0xf8ffffff
#define	PHY_BIT_DELAY_MASK						0xff87ffff


/*error cnt offset and setting*/
#define PHY_ERR_LATCH							1
#define	PHY_ERR_CLR								(1<<1)
#define PHY_BIP_LATCH							(1<<2)
#define	PHY_BIP_CLR								(1<<3)
#define PHY_RXFRAME_LATCH							(1<<4)
#define PHY_RXFRAME_CLR							(1<<5)
#define PHY_BIP_CNT_OFFSET						1
#define PHY_FRAME_CNT_OFFEST					2
#define PHY_ERRCNT_MASK							0xfffffff8
#define PHY_BER_UNKNOWN							0

/*FEC status*/
#define PHY_RX_FEC								(1<<15)
#define PHY_TX_FEC								(1<<15)

/*GPON delimiter setting*/
#define PHY_GPON_DELIM_PAT_LEN_MASK				0xff000000
#define PHY_GPON_GUARD_PAT_OFFSET				24

/*PHY ready*/
#define PHYRDY_OFFSET							18
#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
#define PHYRDY_STATUS							0x06
#else
#define PHYRDY_STATUS							0x02
#endif /* #ifdef CONFIG_USE_MT7520_ASIC  */
#define PHYRDY_MASK								0x07

/*PHY LOS*/
#define PHYLOS_STATUS							0x01

/*frequency meter*/
#define PHY_FREQ_METER_EN						(1<<16)
#define PHY_MON_PROB_CLK						(1<<24)
#define PHY_FREQ_METER_DONE						(1<<16)
#define PHY_FREQ_METER_FAIL						(1<<17)
#define PHY_FREQ_SELE_LEN_MASK					0xf0
#define PHY_FREQ_SELE_MASK						0xf0ffffff
#define	PHY_FREQ_METER_MASK						0x7fff
#define PHY_FREQ_METER_OFFSET					24

/*byte delay manual*/
#define PHY_BYTE_DELAY_MANUAL					(1<<4)
#define PHY_BYTE_DELAY_LEN_MASK					0xfc
#define PHY_BYTE_DELAY_MASK						0xffffff8f
#define PHY_BYTE_DELAY_OFFSET					5



/*Tx fec manual*/
#define PHY_TX_FEC_MANUAL						(1<<2)
#define	PHY_TX_FEC_EN							(1<<3)
#define PHY_TX_FEC_MANUAL_MASK					0xfffffff3


/*Phy Transceiver */
#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
#define	PHY_TRANS_REG_TABLE1					0x50
#define	PHY_TRANS_REG_TABLE2					0x51
#else
#define	PHY_TRANS_REG_TABLE1					0xA0
#define	PHY_TRANS_REG_TABLE2					0xA2
#endif
#define PHY_TRANS_OFFSET						8
#define PHY_TRANS_TEMP_HBYTE					0x60
#define PHY_TRANS_TEMP_LBYTE					0x61
#define PHY_TRANS_VOLT_HBYTE					0x62
#define	PHY_TRANS_VOLT_LBYTE					0x63
#define PHY_TRANS_TX_CUR_HBYTE					0x64
#define	PHY_TRANS_TX_CUR_LBYTE					0x65
#define	PHY_TRANS_TX_POW_HBYTE					0x66
#define PHY_TRANS_TX_POW_LBYTE					0x67
#define	PHY_TRANS_RX_POW_HBYTE					0x68
#define	PHY_TRANS_RX_POW_LBYTE					0x69
#define	PHY_TRANS_ALARM1						0x70
#define	PHY_TRANS_ALARM2						0x71
#define	PHY_TRANS_WARNING						0x75
#define PHY_TRANS_RX_HIGH_ALARM_HBYTE			0x20
#define PHY_TRANS_RX_HIGH_ALARM_LBYTE			0x21
#define PHY_TRANS_RX_LOW_ALARM_HBYTE			0x22
#define PHY_TRANS_RX_LOW_ALARM_LBYTE			0x23
#define PHY_TRANS_TX_HIGH_ALARM_HBYTE			0x18
#define PHY_TRANS_TX_HIGH_ALARM_LBYTE			0x19
#define PHY_TRANS_TX_LOW_ALARM_HBYTE			0x1A
#define PHY_TRANS_TX_LOW_ALARM_LBYTE			0x1B
#define PHY_TRANS_VENDOR_NAME					0x14
#define PHY_TRANS_VENDOR_PN						0x28
#define PHY_TRANS_VENDOR_OFFSET					16


/*Alarm mask*/
#define	PHY_RX_POWER_LOW_ALARM					0x40
#define	PHY_RX_POWER_HIGH_ALARM					0x80
#define	PHY_TX_POWER_LOW_ALARM					0x01
#define	PHY_TX_POWER_HIGH_ALARM					0x02
#define	PHY_TX_CUR_LOW_ALARM					0x04
#define	PHY_TX_CUR_HIGH_ALARM					0x08

/*Warning mask*/
#define	PHY_RX_POWER_LOW_WARNING					0x40

#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
	/* PHY_GponPreb_T offset and length mask */
	#define PHY_EXTB_LENG_SEL_LEN_MASK			0xfe
	#define PHY_EXTB_LENG_SEL_MASK				0xfff7ffff
	#define PHY_EXTB_LENG_SEL_OFFSET			19
#endif /* CONFIG_USE_MT7520_ASIC */
#define PHY_GUARD_BIT_NUM_MASK					0xffffff00
#define PHY_PRE_T1_NUM_MAKS						0xffff00ff
#define PHY_PRE_T1_NUM_OFFSET					8
#define PHY_PRE_T2_NUM_MASK						0xff00ffff
#define PHY_PRE_T2_NUM_OFFSET					16
#define PHY_PRE_T3_PAT_MASK						0x00ffffff
#define PHY_PRE_T3_PAT_OFFSET					24
#define PHY_T3_O4_PRE_MASK						0xffffff00
#define PHY_T3_O5_PRE_MASK						0xffff00ff
#define PHY_T3_O5_PRE_OFFSET					8
#define PHY_EXT_BUR_MODE_MASK					0xfffeffff
#define PHY_EXT_BUR_MODE_OFFSET					16
#define PHY_OPER_RANG_LEG_MASK					0xfc
#define PHY_OPER_RANG_MASK						0xfff9ffff
#define PHY_OPER_RANG_OFFSET					17
#define PHY_DIS_SCRAM_MASK						0xfeffffff
#define PHY_DIS_SCRAM_OFFSET					24




/*GPON RX control*/
typedef enum {
	PHY_GPON_M1_LMT_EN = 0x01,//set by PHY_GponRxCtl_T->mask to modify gpon_m1_lmt
 	PHY_GPON_M2_LMT_EN = 0x02,//set by PHY_GponRxCtl_T->mask to modify gpon_m2_lmt1_lmt
 	PHY_PRESYNC_MIS_LMT_EN = 0x04,//set by PHY_GponRxCtl_T->mask to modify presync_mismatch_lmt
 	PHY_INSYNC_MIS_LMT_EN = 0x08,//set by PHY_GponRxCtl_T->mask to modify insync_mismatch_lmt
	PHY_GPON_GSYNC_PROT_EN = 0x10//set by PHY_GponRxCtl_T->mask to modify gpon_gsync_prot
} ENUM_GponRxCtl_T ;


typedef struct
{
	unchar 			gpon_m1_lmt; //[3 bits] presync state to insync state after m1 consecutive correct Psync
	unchar 			gpon_m2_lmt; //[3 bits ] insync state to hunt state after m2 consecutive incorrect Psync
	unchar 			presync_mismatch_lmt; //[3 bits ] in presync state, the limit of error bits can be ignored in Psync field
	unchar 			insync_mismatch_lmt; //[3 bits ] in insync state, the limit of error bits can be ignored in Psync field
	unchar 			gpon_gsync_prot; //[1 bits ] enforce the Psync value are right during insync state
	uint 			mask;//use ENUM_GponRxCtl_T  to define it
}PHY_GponRxCtl_T,*PPHY_GponRxCtl_T;

/*PHY_GponRxCtl_T offset and length mask*/
#define	PHY_GPON_M1_LEN_MASK					0xF8
#define	PHY_GPON_M1_MASK						0xfffffff8
#define PHY_GPON_M2_LEN_MASK					0xF8
#define PHY_GPON_M2_MASK						0xffffffC7
#define PHY_GPON_M2_OFFSET						3
#define PHY_PRESYNC_MIS_LEN_MASK				0xF8
#define PHY_PRESYNC_MIS_MASK					0xfffff8ff
#define PHY_PRESYNC_MIS_OFFSET					8
#define PHY_INSYNC_MIS_LEN_MASK					0xF8
#define PHY_INSYNC_MIS_MASK						0xffffc7ff
#define PHY_INSYNC_MIS_OFFSET					11
#define PHY_GPON_GSYNC_LEN_MASK					0xFE
#define PHY_GPON_GSYNC_MASK						0xffffbfff
#define PHY_GPON_GSYNC_OFFSET					14

/*transceiver alarm threshold*/
typedef struct
{
	ushort 			rx_power_high_alarm_thod;//alarm threshold
	ushort 			rx_power_low_alarm_thod;//alarm threshold
	ushort 			tx_power_high_alarm_thod;//alarm threshold
	ushort 			tx_power_low_alarm_thod;//alarm threshold
}PHY_TransAlarm_T, *PPHY_TransAlarm_T;

#if 0 //move to phy.h
/*transceiver register*/
typedef struct
{
	ushort 			temprature;
	ushort 			supply_voltage;
	ushort 			tx_current;
	ushort 			tx_power;
	ushort 			rx_power;
}PHY_TransParam_T, *PPHY_TransParam_T;
#endif

#endif /* _PHY_DEF_H_ */
