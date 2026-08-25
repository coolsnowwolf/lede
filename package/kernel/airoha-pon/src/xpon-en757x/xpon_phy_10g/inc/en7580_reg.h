#ifndef _EN7580_REG_H_
#define _EN7580_REG_H_

#if A60972_SERDES
/* ============================== FPGA interface RG define ============================================ */
#define REG_BASE_EN7580_INTERFACE						0xBFAF0000           //control system register 0xBFAF1500


#define EN7580_IF_RG_INTERFACE_0                  		(REG_BASE_EN7580_INTERFACE+0x0D00)	
#define EN7580_IF_RG_INTERFACE_0_BIST_PAT_SEL_PRBS7		(1<<8)
#define EN7580_IF_RG_INTERFACE_0_BIST_PAT_TX_EN			(1<<1)
#define EN7580_IF_RG_INTERFACE_0_BIST_PAT_RX_CHECK_EN 	(1)


#define EN7580_IF_RG_INTERFACE_1                  		(REG_BASE_EN7580_INTERFACE+0x0D04)


#define EN7580_IF_RG_INTERFACE_9                  		(REG_BASE_EN7580_INTERFACE+0x0D24)	
#define EN7580_IF_RG_INTERFACE_9_BIST_PRBS_FAIL			(1<<16)
#define EN7580_IF_RG_INTERFACE_9_BIST_PRBS_COMPARING	(1<<8)
#define EN7580_IF_RG_INTERFACE_9_BIST_PRBS_DONE 		(1)



#define EN7580_IF_RG_PROGRAME_N1                  		(REG_BASE_EN7580_INTERFACE+0x0D60)	
//#define EN7580_IF_RG_PROGRAME_N1_LATCH_CLK_SEL_SW_RST	(1<<16)

#define EN7580_IF_RG_PROGRAME_0 						(REG_BASE_EN7580_INTERFACE+0x0D64)
#define EN7580_IF_RG_PROGRAME_1 						(REG_BASE_EN7580_INTERFACE+0x0D68)
#define EN7580_IF_RG_PROGRAME_2 						(REG_BASE_EN7580_INTERFACE+0x0D6C)
#define EN7580_IF_RG_PROGRAME_3 						(REG_BASE_EN7580_INTERFACE+0x0D70)


#define EN7580_IF_RG_A60972_CTRL                  		(REG_BASE_EN7580_INTERFACE+0x0D84)	
#define EN7580_IF_RG_A60972_CTRL_IF_TX_SW_RESET_ON 		(1<<9)
#define EN7580_IF_RG_A60972_CTRL_IF_RX_SW_RESET_ON 		(1<<8)
#define EN7580_IF_RG_A60972_CTRL_A60972_SW_RESET_ON 	0
#define EN7580_IF_RG_A60972_CTRL_A60972_SW_RESET_OFF 	1
#define EN7580_IF_RG_A60972_CTRL_NORMAL_OPERATION       (0x00000001)
#define EN7580_IF_RG_A60972_CTRL_IF_SW_RESET_ONLY    	(EN7580_IF_RG_A60972_CTRL_IF_TX_SW_RESET_ON|EN7580_IF_RG_A60972_CTRL_IF_RX_SW_RESET_ON|EN7580_IF_RG_A60972_CTRL_NORMAL_OPERATION)
#define EN7580_IF_RG_A60972_CTRL_IF_TX_SW_RESET_ONLY    (EN7580_IF_RG_A60972_CTRL_IF_TX_SW_RESET_ON|EN7580_IF_RG_A60972_CTRL_NORMAL_OPERATION)
#define EN7580_IF_RG_A60972_CTRL_IF_RX_SW_RESET_ONLY    (EN7580_IF_RG_A60972_CTRL_IF_RX_SW_RESET_ON|EN7580_IF_RG_A60972_CTRL_NORMAL_OPERATION)


#define EN7580_IF_RO_INTERFACE_D                  		(REG_BASE_EN7580_INTERFACE+0x0D88)	
#define EN7580_IF_RO_INTERFACE_D_TX_MAX 				(0xffff0000)
#define EN7580_IF_RO_INTERFACE_D_RX_MAX 				(0x0000ffff)


#define EN7580_IF_RO_INTERFACE_G_TX_MAX_LSB           	(REG_BASE_EN7580_INTERFACE+0x0D94)


#define EN7580_IF_RO_INTERFACE_H_RX_MAX_LSB             (REG_BASE_EN7580_INTERFACE+0x0D98)
/* =============================================================================================== */ 
#endif

/*   =================  EN7580 ASIC RG ===================   */

#define TOP_RST_CTRL_SW2							(0xBFB00830)
#define TOP_RST_CTRL_SW2_HOLD						1
#define TOP_RST_CTRL_SW2_RELEASE					0

/*   =================  EN7580GEPON REG  =================   */

#define REG_BASE_GEPON_CSR							0xBFAF0000                

#define GEPON_CSR_PHYSET2                       	(REG_BASE_GEPON_CSR+0x104)
#define GEPON_CSR_PHYSET3                       	(REG_BASE_GEPON_CSR+0x108)
#define GEPON_CSR_PHYSET5                       	(REG_BASE_GEPON_CSR+0x110)
#define GEPON_CSR_PHYSET10                          (REG_BASE_GEPON_CSR+0x124)
#define GEPON_CSR_PHYSTA1                       	(REG_BASE_GEPON_CSR+0x130)
#define GEPON_CSR_XPON_SETTING                      (REG_BASE_GEPON_CSR+0x138)
#define GEPON_CSR_MISC								(REG_BASE_GEPON_CSR+0x1FC)
#define GEPON_CSR_EPON_SYNC_CTL                     (REG_BASE_GEPON_CSR+0x200)
#define GEPON_CSR_EPON_FRAM_SRH_CTL                 (REG_BASE_GEPON_CSR+0x204)
#define GEPON_CSR_GPON_PSYNC_CTL                    (REG_BASE_GEPON_CSR+0x20C)
#define GEPON_CSR_GPON_INDENT_CTL                   (REG_BASE_GEPON_CSR+0x210)
#define GEPON_CSR_RS_CTL                            (REG_BASE_GEPON_CSR+0x214)
#define GEPON_CSR_PHYRX_TEST_CTL                    (REG_BASE_GEPON_CSR+0x218)
#define GEPON_CSR_PHYRX_STATUS                      (REG_BASE_GEPON_CSR+0x21C)
#define GEPON_CSR_PHYRX_MISC_TRIG                   (REG_BASE_GEPON_CSR+0x220)
#define GEPON_CSR_PHYRX_TEST_DBG_TRIG               (REG_BASE_GEPON_CSR+0x224)
#define GEPON_CSR_PHYRX_EPON_FRAME_ERR              (REG_BASE_GEPON_CSR+0x228)
#define GEPON_CSR_RX_SRAM_BIST_STATUS               (REG_BASE_GEPON_CSR+0x22C)
#define GEPON_CSR_XP_ERRCNT_EN                      (REG_BASE_GEPON_CSR+0x230)
#define GEPON_CSR_XP_ERRCNT_CTL                     (REG_BASE_GEPON_CSR+0x234)
#define GEPON_CSR_ERR_BYTE_CNT                      (REG_BASE_GEPON_CSR+0x238)
#define GEPON_CSR_ERR_CODE_CNT                      (REG_BASE_GEPON_CSR+0x23C)
#define GEPON_CSR_NOSOL_CODE_CNT                    (REG_BASE_GEPON_CSR+0x240)
#define GEPON_CSR_RX_CODE_CNT                       (REG_BASE_GEPON_CSR+0x244)
#define GEPON_CSR_FEC_SECONDS                       (REG_BASE_GEPON_CSR+0x248)
#define GEPON_CSR_BIP_CNT                           (REG_BASE_GEPON_CSR+0x24C)
#define GEPON_CSR_FRAME_CNT_L                       (REG_BASE_GEPON_CSR+0x250)
#define GEPON_CSR_FRAME_CNT_H                       (REG_BASE_GEPON_CSR+0x254)
#define GEPON_CSR_LOF_CNT                           (REG_BASE_GEPON_CSR+0x258)
#define GEPON_CSR_FECDEC_TESTCTL                    (REG_BASE_GEPON_CSR+0x260)
#define GEPON_CSR_FECRS_TESTCTL                     (REG_BASE_GEPON_CSR+0x264)
#define GEPON_CSR_FECDEC_CTL                        (REG_BASE_GEPON_CSR+0x268)
#define GEPON_CSR_FECDEC_SRAMCTL                    (REG_BASE_GEPON_CSR+0x270)
#define GEPON_CSR_SRAM_TEST_RX_CTRL                 (REG_BASE_GEPON_CSR+0x280)
#define GEPON_CSR_SRAM_TEST_RX_WR_DATA              (REG_BASE_GEPON_CSR+0x284)
#define GEPON_CSR_SRAM_TEST_RX_RD_DATA              (REG_BASE_GEPON_CSR+0x288)
#define GEPON_CSR_SRAM_TEST_RX_RW_TRIG              (REG_BASE_GEPON_CSR+0x28C)
#define GEPON_CSR_DUMMY_REG_0290                   	(REG_BASE_GEPON_CSR+0x290)
#define GEPON_CSR_PHYRX_RESET                       (REG_BASE_GEPON_CSR+0x294)
#define GEPON_CSR_PHYRX_EPON_CNT_CTL                (REG_BASE_GEPON_CSR+0x298)
#define GEPON_CSR_PHYRX_EPON_SYNC_STATUS            (REG_BASE_GEPON_CSR+0x29C)
#define GEPON_CSR_PRBS_ALIGN_PATTERN                (REG_BASE_GEPON_CSR+0x2A0)
#define GEPON_CSR_PRBS_ALIGN_MASK                   (REG_BASE_GEPON_CSR+0x2A4)
#define GEPON_CSR_PRBS_ERR_CNT                      (REG_BASE_GEPON_CSR+0x2C0)
#define GEPON_CSR_PRBS_LOOPBACK_STATUS              (REG_BASE_GEPON_CSR+0x2C4)
#define GEPON_CSR_RPMA_SRAM_CTRL                    (REG_BASE_GEPON_CSR+0x2C8)
#define GEPON_CSR_ROUND_TRIP_DELAY_CTRL             (REG_BASE_GEPON_CSR+0x2CC)
#define GEPON_CSR_ROUND_TRIP_CAL_MASK_CTRL          (REG_BASE_GEPON_CSR+0x2D0)
#define GEPON_CSR_ROUND_TRIP_DELAY_VALUE            (REG_BASE_GEPON_CSR+0x2D4)
#define GEPON_CSR_ROUND_TRIP_DELAY_VALUE_STATIC     (REG_BASE_GEPON_CSR+0x2D8)
#define GEPON_CSR_PSYNC_DET_ALIGN_PHASE             (REG_BASE_GEPON_CSR+0x2DC)
#define GEPON_CSR_RX_TX_HEAD_TO_HEAD_DELAY          (REG_BASE_GEPON_CSR+0x2E0)
#define GEPON_CSR_ROUND_TRIP_CAL_MASK_CTRL_1        (REG_BASE_GEPON_CSR+0x2E4)
#define GEPON_CSR_ROUND_TRIP_DELAY_VALUE_1          (REG_BASE_GEPON_CSR+0x2E8)
#define GEPON_CSR_ROUND_TRIP_DELAY_VALUE_2          (REG_BASE_GEPON_CSR+0x2EC)
#define GEPON_CSR_ROUND_TRIP_DELAY_VALUE_STATIC_1   (REG_BASE_GEPON_CSR+0x2F0)
#define GEPON_CSR_ROUND_TRIP_DELAY_VALUE_STATIC_2   (REG_BASE_GEPON_CSR+0x2F4)
#define GEPON_CSR_RX_TX_HEAD_TO_HEAD_DELAY_1        (REG_BASE_GEPON_CSR+0x2F8)
#define GEPON_CSR_RX_TX_HEAD_TO_HEAD_DELAY_2        (REG_BASE_GEPON_CSR+0x2FC)
#define GEPON_CSR_PSYNC_UPDATE_CTRL                 (REG_BASE_GEPON_CSR+0x300)
#define GEPON_CSR_PSYNC_FORCE_VALUE                 (REG_BASE_GEPON_CSR+0x304)
#define GEPON_CSR_PSYNC_DET_TINMEOUT                (REG_BASE_GEPON_CSR+0x308)
#define GEPON_CSR_PSYNC_DIFF_TIMER                  (REG_BASE_GEPON_CSR+0x30C)
#define GEPON_CSR_FOUND_PSYNC_TIMER                 (REG_BASE_GEPON_CSR+0x310)
#define GEPON_CSR_ABS_TIMER                         (REG_BASE_GEPON_CSR+0x314)
#define GEPON_CSR_PSYNC_DET_CNT                     (REG_BASE_GEPON_CSR+0x318)
#define GEPON_CSR_MGMII_PHY_DELAY                   (REG_BASE_GEPON_CSR+0x3A0)
#define GEPON_CSR_RX_FIFO_EMPTY_CNT                 (REG_BASE_GEPON_CSR+0x3A4)
#define GEPON_CSR_GPON_PREAMBLE                     (REG_BASE_GEPON_CSR+0x400)
#define GEPON_CSR_GPON_DELIMITER_GUARD              (REG_BASE_GEPON_CSR+0x404)
#define GEPON_CSR_GPON_EXTENDED_PREAMBLE            (REG_BASE_GEPON_CSR+0x408)
#define GEPON_CSR_PHYTX_STATUS                      (REG_BASE_GEPON_CSR+0x40C)
#define GEPON_CSR_EPON_TX_CTL                       (REG_BASE_GEPON_CSR+0x410)
#define GEPON_CSR_PHYTX_TEST_CTL                    (REG_BASE_GEPON_CSR+0x420)
#define GEPON_CSR_PHYTX_TEST_TRIG                   (REG_BASE_GEPON_CSR+0x424)
#define GEPON_CSR_EPON_LOOPBAK_MODE                 (REG_BASE_GEPON_CSR+0x428)
#define GEPON_CSR_PHYTX_FECENC_SW_RST               (REG_BASE_GEPON_CSR+0x42C)
#define GEPON_CSR_PHYTX_MISC                        (REG_BASE_GEPON_CSR+0x430)
#define GEPON_CSR_TX_FRAME_COUNTER                  (REG_BASE_GEPON_CSR+0x434)
#define GEPON_CSR_TX_BURST_COUNTER                  (REG_BASE_GEPON_CSR+0x438)
#define GEPON_CSR_TX_FEC_FRAME_COUNTER              (REG_BASE_GEPON_CSR+0x43C)
#define GEPON_CSR_SRAM_TEST_TX_CTRL                 (REG_BASE_GEPON_CSR+0x440)
#define GEPON_CSR_SRAM_TEST_TX_RD_DATA              (REG_BASE_GEPON_CSR+0x448)
#define GEPON_CSR_SRAM_TEST_TX_RW_TRIG              (REG_BASE_GEPON_CSR+0x44C)
#define GEPON_CSR_FECENC_STATUS                     (REG_BASE_GEPON_CSR+0x450)
#define GEPON_CSR_DUMMY_REG_0460                   	(REG_BASE_GEPON_CSR+0x460)
#define GEPON_CSR_TX_SRAM_BIST_STATUS               (REG_BASE_GEPON_CSR+0x464)
#define GEPON_CSR_TX_MULT2                          (REG_BASE_GEPON_CSR+0x490)
#define GEPON_CSR_BISTCTL_LOOPBACK_SEL              (REG_BASE_GEPON_CSR+0x4A0)
#define GEPON_CSR_BISTCTL_PRBS_TX_EN                (REG_BASE_GEPON_CSR+0x4A4)
#define GEPON_CSR_CID_LENGTH_SEL                    (REG_BASE_GEPON_CSR+0x4B0)
#define GEPON_CSR_TPMA_SRAM_CTRL                    (REG_BASE_GEPON_CSR+0x4B4)
#define GEPON_CSR_TEST_FRAME_EN                     (REG_BASE_GEPON_CSR+0x510)
#define GEPON_CSR_TEST_FRAME_NUM                    (REG_BASE_GEPON_CSR+0x514)
#define GEPON_CSR_TEST_FRAME_MAX_LENGTH             (REG_BASE_GEPON_CSR+0x518)
#define GEPON_CSR_TEST_FRAME_MIN_LENGTH             (REG_BASE_GEPON_CSR+0x51C)
#define GEPON_CSR_TF_PAT_SEL                        (REG_BASE_GEPON_CSR+0x520)
#define GEPON_CSR_TEST_FRAME_ERR_CNT                (REG_BASE_GEPON_CSR+0x524)
#define GEPON_CSR_TEST_FRAME_TX_INFO                (REG_BASE_GEPON_CSR+0x528)
#define GEPON_CSR_TEST_FRAME_DONE                   (REG_BASE_GEPON_CSR+0x52C)
#define GEPON_CSR_EPON_SPD                          (REG_BASE_GEPON_CSR+0x530)
#define GEPON_CSR_EPON_PREAMBLE0                    (REG_BASE_GEPON_CSR+0x534)
#define GEPON_CSR_EPON_PREAMBLE1                    (REG_BASE_GEPON_CSR+0x538)
#define GEPON_CSR_EPON_SFD                          (REG_BASE_GEPON_CSR+0x53C)
#define GEPON_CSR_EPON_TF_DST0                      (REG_BASE_GEPON_CSR+0x540)
#define GEPON_CSR_EPON_TF_DST1                      (REG_BASE_GEPON_CSR+0x544)
#define GEPON_CSR_EPON_TF_SRC0                      (REG_BASE_GEPON_CSR+0x548)
#define GEPON_CSR_EPON_TF_SRC1                      (REG_BASE_GEPON_CSR+0x54C)
#define GEPON_CSR_EPON_TF_LEN_TYPE                  (REG_BASE_GEPON_CSR+0x550)
#define GEPON_CSR_EPON_TF_FIRST_DATA0               (REG_BASE_GEPON_CSR+0x554)
#define GEPON_CSR_EPON_TF_FIRST_DATA1               (REG_BASE_GEPON_CSR+0x558)
#define GEPON_CSR_EPON_TF_FIRST_DATA2               (REG_BASE_GEPON_CSR+0x55C)
#define GEPON_CSR_EPON_TF_FIRST_DATA3               (REG_BASE_GEPON_CSR+0x560)
#define GEPON_CSR_EPON_TF_FIRST_DATA4               (REG_BASE_GEPON_CSR+0x564)
#define GEPON_CSR_EPON_TF_FIRST_DATA5               (REG_BASE_GEPON_CSR+0x568)
#define GEPON_CSR_EPON_TF_FIRST_DATA6               (REG_BASE_GEPON_CSR+0x56C)
#define GEPON_CSR_EPON_TF_FIRST_DATA7               (REG_BASE_GEPON_CSR+0x570)
#define GEPON_CSR_XPON_STA                          (REG_BASE_GEPON_CSR+0x5E0)
#define GEPON_CSR_GIO1_SETTING                      (REG_BASE_GEPON_CSR+0x5E8)
#define GEPON_CSR_GIO2_SETTING                      (REG_BASE_GEPON_CSR+0x5EC)
#define GEPON_CSR_XPON_INT_EN                       (REG_BASE_GEPON_CSR+0x5F0)
#define GEPON_CSR_XPON_INT_STA_CLR                  (REG_BASE_GEPON_CSR+0x5F4)
#define GEPON_CSR_XPON_INT_STA                      (REG_BASE_GEPON_CSR+0x5F8)
#define GEPON_CSR_EPON_TF_ASYNC_FIFO                (REG_BASE_GEPON_CSR+0x5FC)


/* XPON SETTING */
#define GEPON_CSR_XPON_SETTING_TX_DLY_EN            (1<<8)                     
//this bit is invalid in EN7580 ASIC #define GEPON_CSR_XPON_SETTING_TRANS_BURST_INV      (1<<7)
#define GEPON_CSR_XPON_SETTING_TRANS_RX_SD_INV      (1<<6)
#define GEPON_CSR_XPON_SETTING_TRANS_TX_FAULT_INV   (1<<5)
#define GEPON_CSR_XPON_SETTING_TRANS_TX_SD_INV      (1<<4)


/*RX sync*/
#define GEPON_CSR_PHYRX_STATUS_SYNC_STATUS          (0xf)
#define GEPON_CSR_PHYRX_STATUS_SYNC_OK              (0xa)

/*burst_enable inv*/
#define GEPON_CSR_XPON_SETTING_BURST_EN_INV         (0x80)

/*Interrupt*/

//Interrupt enable
#define GEPON_CSR_XPON_RX_SD_INT_EN           		(1<<9)
#define GEPON_CSR_XPON_NO_LOS_INT_EN           		(1<<8)
#define GEPON_CSR_XPON_I2CM_IRQ_INT_EN				(1<<7)
#define GEPON_CSR_XPON_ILLEGAL_INT_EN           	(1<<6)
#define GEPON_CSR_XPON_PHYRDY_INT_EN            	(1<<5)
#define GEPON_CSR_XPON_TRANS_SD_FAIL_INT_EN     	(1<<4)
#define GEPON_CSR_XPON_TRANS_INT_EN             	(1<<3)
#define GEPON_CSR_XPON_TRANS_FAULT_INT_EN       	(1<<2)
#define GEPON_CSR_XPON_LOF_INT_EN               	(1<<1)
#define GEPON_CSR_XPON_TRANS_LOS_INT_EN         	(1)

//Interrupt status clear
#define GEPON_CSR_XPON_RX_SD_INTCLR           		(1<<9)
#define GEPON_CSR_XPON_NO_LOS_INTCLR           		(1<<8)
#define GEPON_CSR_XPON_I2CM_IRQ_INTCLR        		(1<<7)
#define GEPON_CSR_XPON_ILLEGAL_INTCLR             	(1<<6)
#define GEPON_CSR_XPON_PHYRDY_INTCLR              	(1<<5)
#define GEPON_CSR_XPON_TRANS_SD_FAIL_INTCLR       	(1<<4)
#define GEPON_CSR_XPON_TRANS_INTCLR               	(1<<3)
#define GEPON_CSR_XPON_TRANS_FAULT_INTCLR         	(1<<2)
#define GEPON_CSR_XPON_LOF_INTCLR                 	(1<<1)
#define GEPON_CSR_XPON_TRANS_LOS_INTCLR           	(1) 

//Interrupt status
#define GEPON_CSR_XPON_RX_SD_INT            		(1<<9)
#define GEPON_CSR_XPON_NO_LOS_INT           		(1<<8)
#define GEPON_CSR_XPON_I2CM_IRQ_INT           		(1<<7)
#define GEPON_CSR_XPON_ILLEGAL_INT                	(1<<6)
#define GEPON_CSR_XPON_PHYRDY_INT                 	(1<<5)
#define GEPON_CSR_XPON_TRANS_SD_FAIL_INT          	(1<<4)
#define GEPON_CSR_XPON_TRANS_INT                  	(1<<3)
#define GEPON_CSR_XPON_TRANS_FAULT_INT            	(1<<2)
#define GEPON_CSR_XPON_LOF_INT                    	(1<<1)
#define GEPON_CSR_XPON_TRANS_LOS_INT              	(1)

/*error cnt en*/
#define GEPON_CSR_XP_ERRCNT_EN_ERRCNT				1
#define GEPON_CSR_XP_ERRCNT_EN_BIPCNT				(1<<1)
#define GEPON_CSR_XP_ERRCNT_EN_FMCNT				(1<<2)
#define GEPON_CSR_XP_ERRCNT_EN_ERRCNT_OFFSET		0
#define GEPON_CSR_XP_ERRCNT_EN_BIPCNT_OFFSET		1
#define GEPON_CSR_XP_ERRCNT_EN_FMCNT_OFFSET			2
#define GEPON_CSR_XP_ERRCNT_EN_MASK					0xfffffff8

/*error cnt CTL*/
#define GEPON_CSR_XP_ERRCNT_CTL_ERRCNT_LATCH		1
#define GEPON_CSR_XP_ERRCNT_CTL_ERRCNT_CLR			(1<<1)
#define GEPON_CSR_XP_ERRCNT_CTL_BIPCNT_LATCH		(1<<2)
#define GEPON_CSR_XP_ERRCNT_CTL_BIPCNT_CLR			(1<<3)
#define GEPON_CSR_XP_ERRCNT_CTL_FMCNT_LATCH			(1<<4)
#define GEPON_CSR_XP_ERRCNT_CTL_FMCNT_CLR			(1<<5)


/*XPON STA*/
#define GEPON_CSR_XPON_STA_LOS						1

/*pll reset*/
#define GEPON_CSR_PHYSET3_PLL_RST					(1<<31)
/*los reset*/
#define GEPON_CSR_PHYSET3_LOS_RST					(1<<29)
/*phy reset*/
#define GEPON_CSR_PHYSET3_PHY_RST					(1<<28)
/*software reset*/
#define GEPON_CSR_PHYSET3_SOFTWARE_RST				(1<<27)
/*counter reset*/
#define GEPON_CSR_PHY_COUNT_RST                     (1<<27)
/*PHY ready*/
#define GEPON_PHYRDY_OFFSET							18

#define GEPON_PHYRDY_STATUS							0x06

#define GEPON_PHYRDY_MASK							0x07

/*FEC status*/
#define GEPON_PHY_RX_FEC							(1<<15)
#define GEPON_PHY_TX_FEC							(1<<15)


/*Tx Burst mode*/
#define GEPON_PHY_TX_CONT_MODE_MASK					0xffffff5f
#define GEPON_PHY_TX_CONT_MODE_OFFSET				0xa0
#define GEPON_PHY_TX_EPON_CONT_MODE					0xffffffdf


/*XPON TX config setting*/
#define GEPON_PHY_TX_SD_INV_MASK					0xffffffef
#define GEPON_PHY_TX_SD_INV_OFFSET					4
#define GEPON_PHY_BURST_EN_INV_MASK					0xffffff7f
#define GEPON_PHY_BURST_EN_INV_OFFSET				7
#define	GEPON_PHY_TX_FAULT_INV_MASK					0xffffffdf
#define	GEPON_PHY_TX_FAULT_INV_OFFSET				5
#define GEPON_PHY_CSR_TX_SW_RST_MASK				0xfffffffd	//YMC 20150325


/*XPON RX config setting*/
#define GEPON_PHY_RX_SD_INV 						(1<<6)
#define GEPON_PHY_RX_SD								(0<<6)


/*GPON delimiter setting*/
#define GEPON_PHY_GPON_DELIM_PAT_LEN_MASK			0xff000000
#define GEPON_PHY_GPON_GUARD_PAT_OFFSET				24


/*Tx fec manual*/
#define GEPON_PHY_TX_FEC_MANUAL						(1<<2)
#define	GEPON_PHY_TX_FEC_EN							(1<<3)
#define GEPON_PHY_TX_FEC_MANUAL_MASK				0xfffffff3

/* PHY_GponPreb_T offset and length mask */
#define GEPON_PHY_EXTB_LENG_SEL_LEN_MASK			0xfe
#define GEPON_PHY_EXTB_LENG_SEL_MASK				0xfff7ffff
#define GEPON_PHY_EXTB_LENG_SEL_OFFSET				19

#define GEPON_PHY_GUARD_BIT_NUM_MASK				0xffffff00
#define GEPON_PHY_PRE_T1_NUM_MAKS					0xffff00ff
#define GEPON_PHY_PRE_T1_NUM_OFFSET					8
#define GEPON_PHY_PRE_T2_NUM_MASK					0xff00ffff
#define GEPON_PHY_PRE_T2_NUM_OFFSET					16
#define GEPON_PHY_PRE_T3_PAT_MASK					0x00ffffff
#define GEPON_PHY_PRE_T3_PAT_OFFSET					24
#define GEPON_PHY_T3_O4_PRE_MASK					0xffffff00
#define GEPON_PHY_T3_O5_PRE_MASK					0xffff00ff
#define GEPON_PHY_T3_O5_PRE_OFFSET					8
#define GEPON_PHY_EXT_BUR_MODE_MASK					0xfffeffff
#define GEPON_PHY_EXT_BUR_MODE_OFFSET				16
#define GEPON_PHY_OPER_RANG_LEG_MASK				0xfc
#define GEPON_PHY_OPER_RANG_MASK					0xfff9ffff
#define GEPON_PHY_OPER_RANG_OFFSET					17
#define GEPON_PHY_DIS_SCRAM_MASK					0xfeffffff
#define GEPON_PHY_DIS_SCRAM_OFFSET					24

/*phy fw ready*/
#define GEPON_PHY_FW_RDY_EN							1
#define GEPON_PHY_FW_RDY_MASK							0xfffffffe

/*bit delay*/

#define GEPON_PHY_DIG_BIT_DELAY_OFFSET				19
//#define	GEPON_PHY_BIT_DELAY_ADDR						PHY_CSR_PHYSET5
#define GEPON_PHY_DIG_BIT_DELAY_MASK					0xff87ffff

#define GEPON_PHY_BIT_DELAY_LEN_MASK					0xf8
#define GEPON_PHY_ANA_BIT_DELAY_OFFSET				24
#define GEPON_PHY_TX_BIT_DEL_SEL						(1<<23)
#define GEPON_PHY_ANA_BIT_DELAY_MASK					0xf8ffffff
#define	GEPON_PHY_BIT_DELAY_MASK						0xff87ffff


/* XPON mode select */
#define	GEPON_PHY_GPON_MODE							(1<<31)

/*   =================  EN7580 XGPON REG  =================   */

#define REG_BASE_XGPON_PHY                  		0xBFAF0000

#define XGPON_PHY_XG_PON_RX_SYNC_CTRL                (REG_BASE_XGPON_PHY+0xA04)
#define XGPON_PHY_XG_PON_TX_CTRL                     (REG_BASE_XGPON_PHY+0xA08)
#define XGPON_PHY_XG_PHY_RST_N                       (REG_BASE_XGPON_PHY+0xA0C)
#define XGPON_PHY_XG_PON_INT_STA                     (REG_BASE_XGPON_PHY+0xA10)
#define XGPON_PHY_XG_PON_INT_EN                      (REG_BASE_XGPON_PHY+0xA14)
#define XGPON_PHY_PREAMBLE1_UPPER                    (REG_BASE_XGPON_PHY+0xA18)
#define XGPON_PHY_PREAMBLE1_LOWER                    (REG_BASE_XGPON_PHY+0xA1C)
#define XGPON_PHY_PREAMBLE2_UPPER                    (REG_BASE_XGPON_PHY+0xA20)
#define XGPON_PHY_PREAMBLE2_LOWER                    (REG_BASE_XGPON_PHY+0xA24)
#define XGPON_PHY_PREAMBLE3_UPPER                    (REG_BASE_XGPON_PHY+0xA28)
#define XGPON_PHY_PREAMBLE3_LOWER                    (REG_BASE_XGPON_PHY+0xA2C)
#define XGPON_PHY_PREAMBLE4_UPPER                    (REG_BASE_XGPON_PHY+0xA30)
#define XGPON_PHY_PREAMBLE4_LOWER                    (REG_BASE_XGPON_PHY+0xA34)
#define XGPON_PHY_DELIMITER1_UPPER                   (REG_BASE_XGPON_PHY+0xA38)
#define XGPON_PHY_DELIMITER1_LOWER                   (REG_BASE_XGPON_PHY+0xA3C)
#define XGPON_PHY_DELIMITER2_UPPER                   (REG_BASE_XGPON_PHY+0xA40)
#define XGPON_PHY_DELIMITER2_LOWER                   (REG_BASE_XGPON_PHY+0xA44)
#define XGPON_PHY_DELIMITER3_UPPER                   (REG_BASE_XGPON_PHY+0xA48)
#define XGPON_PHY_DELIMITER3_LOWER                   (REG_BASE_XGPON_PHY+0xA4C)
#define XGPON_PHY_DELIMITER4_UPPER                   (REG_BASE_XGPON_PHY+0xA50)
#define XGPON_PHY_DELIMITER4_LOWER                   (REG_BASE_XGPON_PHY+0xA54)
#define XGPON_PHY_XG_TX_FEC_EN_CTRL                  (REG_BASE_XGPON_PHY+0xA58)
#define XGPON_PHY_PSBU_INFO1                         (REG_BASE_XGPON_PHY+0xA5C)
#define XGPON_PHY_PSBU_INFO2                         (REG_BASE_XGPON_PHY+0xA60)
#define XGPON_PHY_PSBU_INFO3                         (REG_BASE_XGPON_PHY+0xA64)
#define XGPON_PHY_PSBU_INFO4                         (REG_BASE_XGPON_PHY+0xA68)
#define XGPON_PHY_LASER_ON_LEN                       (REG_BASE_XGPON_PHY+0xA6C)
#define XGPON_PHY_LASER_ON_PATTERN                   (REG_BASE_XGPON_PHY+0xA70)
#define XGPON_PHY_XG_TX_IDLE_CTRL                    (REG_BASE_XGPON_PHY+0xA74)
#define XGPON_PHY_XG_CONTINUE_CTRL                   (REG_BASE_XGPON_PHY+0xA78)
#define XGPON_PHY_XG_CONTINUE_CFG_PATTERN_LOWER      (REG_BASE_XGPON_PHY+0xA7C)
#define XGPON_PHY_XG_CONTINUE_CFG_PATTERN_UPPER      (REG_BASE_XGPON_PHY+0xA80)
#define XGPON_PHY_DBG_CTRL                           (REG_BASE_XGPON_PHY+0xA84)
#define XGPON_PHY_DBG_RX_CW_START_CNT                (REG_BASE_XGPON_PHY+0xA88)
#define XGPON_PHY_DBG_RX_CW_END_CNT                  (REG_BASE_XGPON_PHY+0xA8C)
#define XGPON_PHY_DBG_RX_FRAME2PHYD_CNT              (REG_BASE_XGPON_PHY+0xA90)
#define XGPON_PHY_DBG_RX_SOF2MAC_CNT                 (REG_BASE_XGPON_PHY+0xA94)
#define XGPON_PHY_DBG_RX_EOF2MAC_CNT                 (REG_BASE_XGPON_PHY+0xA98)
#define XGPON_PHY_DBG_TX_SOF2PHYD_CNT                (REG_BASE_XGPON_PHY+0xA9C)
#define XGPON_PHY_DBG_TX_EOF2PHYD_CNT                (REG_BASE_XGPON_PHY+0xB00)
#define XGPON_PHY_DBG_TX_START2PHYA_CNT              (REG_BASE_XGPON_PHY+0xB04)
#define XGPON_PHY_DBG_TX_END2PHYA_CNT                (REG_BASE_XGPON_PHY+0xB08)
#define XGPON_PHY_DBG_TX_PHY_RDY2MAC_DLY             (REG_BASE_XGPON_PHY+0xB0C)
#define XGPON_PHY_DBG_PSYNC_MISMATCH_CNT             (REG_BASE_XGPON_PHY+0xB10)
#define XGPON_PHY_DBG_SFC_HEC_ERR_CNT                (REG_BASE_XGPON_PHY+0xB14)
#define XGPON_PHY_DBG_LOF_CNT                        (REG_BASE_XGPON_PHY+0xB18)
#define XGPON_PHY_DBG_RX_SYNC_ST                     (REG_BASE_XGPON_PHY+0xB1C)
#define XGPON_PHY_DBG_PON_ID_HEC_ERR_CNT             (REG_BASE_XGPON_PHY+0xB20)
#define XGPON_PHY_FEC_UNCORRECTED_CW_CNT             (REG_BASE_XGPON_PHY+0xB24)
#define XGPON_PHY_FEC_CORRECTED_CW_CNT               (REG_BASE_XGPON_PHY+0xB28)
#define XGPON_PHY_DBG_TX_ERR_FLAG                    (REG_BASE_XGPON_PHY+0xB2C)
#define XGPON_PHY_DBG_XG_PHYD_PROBE_SEL              (REG_BASE_XGPON_PHY+0xB30)
#define XGPON_PHY_DBG_XG_PHYD_UPPER_PROBE            (REG_BASE_XGPON_PHY+0xB34)
#define XGPON_PHY_DBG_XG_PHYD_LOWER_PROBE            (REG_BASE_XGPON_PHY+0xB38)
#define XGPON_PHY_DBG_PHYA_STA                       (REG_BASE_XGPON_PHY+0xB3C)
#define XGPON_PHY_PHYA_INT_STA                       (REG_BASE_XGPON_PHY+0xB40)
#define XGPON_PHY_PHYA_INT_EN                        (REG_BASE_XGPON_PHY+0xB44)
#define XGPON_PHY_SFP_VLD_LEVEL                      (REG_BASE_XGPON_PHY+0xB48)
#define XGPON_PHY_SFP_STA                            (REG_BASE_XGPON_PHY+0xB4C)
#define XGPON_PHY_SFP_TX_SD_TH                       (REG_BASE_XGPON_PHY+0xB50)
#define XGPON_PHY_XG_PHY_STA                         (REG_BASE_XGPON_PHY+0xB54)
#define XGPON_PHY_AFIFO_OUT_D1                       (REG_BASE_XGPON_PHY+0xB58)
#define XGPON_PHY_AFIFO_OUT_D2                       (REG_BASE_XGPON_PHY+0xB5C)
#define XGPON_PHY_AFIFO_OUT_D3                       (REG_BASE_XGPON_PHY+0xB60)
#define XGPON_PHY_AFIFO_OUT_D4                       (REG_BASE_XGPON_PHY+0xB64)
#define XGPON_PHY_AFIFO_OUT_D5                       (REG_BASE_XGPON_PHY+0xB68)
#define XGPON_PHY_AFIFO_OUT_D6                       (REG_BASE_XGPON_PHY+0xB6C)
#define XGPON_PHY_MAC_TX_D1                          (REG_BASE_XGPON_PHY+0xB70)
#define XGPON_PHY_MAC_TX_D2                          (REG_BASE_XGPON_PHY+0xB74)
#define XGPON_PHY_MAC_TX_D3                          (REG_BASE_XGPON_PHY+0xB78)
#define XGPON_PHY_MAC_TX_D4                          (REG_BASE_XGPON_PHY+0xB7C)
#define XGPON_PHY_MAC_TX_D5                          (REG_BASE_XGPON_PHY+0xB80)
#define XGPON_PHY_MAC_TX_D6                          (REG_BASE_XGPON_PHY+0xB84)
#define XGPON_PHY_XG_PON_SERDES_CTR                  (REG_BASE_XGPON_PHY+0xB88)
#define XGPON_PHY_DBG_SFC_HEC_1BIT_CORRECT_CNT       (REG_BASE_XGPON_PHY+0xB8C)
#define XGPON_PHY_DBG_SFC_HEC_2BIT_CORRECT_CNT       (REG_BASE_XGPON_PHY+0xB90)
#define XGPON_PHY_DBG_PONID_HEC_1BIT_CORRECT_CNT     (REG_BASE_XGPON_PHY+0xB94)
#define XGPON_PHY_DBG_PONID_HEC_2BIT_CORRECT_CNT     (REG_BASE_XGPON_PHY+0xB98)
#define XGPON_PHY_DBG_PHYD_DLY_STA                   (REG_BASE_XGPON_PHY+0xB9C)
#define XGPON_PHY_TX_BURST_ADJUST                    (REG_BASE_XGPON_PHY+0xBA0)
#define XGPON_PHY_FEC_TOTAL_CW_CNT                   (REG_BASE_XGPON_PHY+0xBA4)
#define XGPON_PHY_FEC_CORRECTED_BYTE_CNT             (REG_BASE_XGPON_PHY+0xBA8)
#define XGPON_PHY_PSBD_HEC_ERR_CNT                   (REG_BASE_XGPON_PHY+0xBAC)
#define XGPON_PHY_PHY_PM_CNT_CLR                     (REG_BASE_XGPON_PHY+0xBB0)
#define XGPON_PHY_PON_ID_UPPER                       (REG_BASE_XGPON_PHY+0xBB4)
#define XGPON_PHY_PON_ID_LOWER                       (REG_BASE_XGPON_PHY+0xBB8)
#define XGPON_PHY_FEC_ERR_SECONDS                    (REG_BASE_XGPON_PHY+0xBBC)
#define XGPON_PHY_SFP_DEGLITCH_FF_CASCADE            (REG_BASE_XGPON_PHY+0xBC0)
#define XGPON_PHY_DBG_SUPPOSED_LASER_ON              (REG_BASE_XGPON_PHY+0xBC4)
#define XGPON_PHY_DBG_ACTUAL_LASER_ON                (REG_BASE_XGPON_PHY+0xBC8)
#define XGPON_PHY_DBG_TX_FEC_STA                     (REG_BASE_XGPON_PHY+0xBCC)



/*Rx Sync ctrl*/

#define XGPON_PHY_XG_PON_RX_SYNC_CTRL_RX_DATA_BIT_SWAP  	(1<<17) 
#define XGPON_PHY_XG_PON_RX_SYNC_CTRL_RX_ENABLE 			(1<<16)

/*Interrupt*/
#define XGPON_PHY_XG_PON_INT_DISABLE			0

//Interrupt status
#define XGPON_PHY_XG_PHYA_RDY_INT           	(1<<31)
#define XGPON_PHY_TX_SFP_CONTINUE_INT       	(1<<23)
#define XGPON_PHY_TX_SFP_ABNORMAL_INT       	(1<<22)
#define XGPON_PHY_TX_FAULT_INT              	(1<<21)
#define XGPON_PHY_TX_BURST_SPACE_ERR_INT    	(1<<20)
#define XGPON_PHY_TX_MPI_ERR_INT            	(1<<19)
#define XGPON_PHY_TX_PSBU_INFO_ERR_INT      	(1<<18)
#define XGPON_PHY_TX_INFO_FIFO_OVERFLOW     	(1<<17)
#define XGPON_PHY_TX_INFO_FIFO_UNDERFLOW    	(1<<16)
#define XGPON_PHY_RX_RDY_INT                	(1<<9) 
#define XGPON_PHY_RX_CW_CNT_ERR_INT         	(1<<8) 
#define XGPON_PHY_RX_INFO_FIFO_OVERFLOW     	(1<<7) 
#define XGPON_PHY_RX_INFO_FIFO_UNDERFLOW    	(1<<6) 
#define XGPON_PHY_RX_NGPON2_OC_ERR_INT      	(1<<5) 
#define XGPON_PHY_RX_FEC_ERR_INT            	(1<<4) 
#define XGPON_PHY_RX_BER_HIGH_INT           	(1<<3) 
#define XGPON_PHY_RX_LOF_INT                	(1<<2) 
#define XGPON_PHY_RX_SYNC_OK_INT            	(1<<1) 
#define XGPON_PHY_RX_LOS_INT                	(1)



//Interrupt enable
#define XGPON_PHY_XG_PHYA_RDY_INT_EN         	(1<<31)
#define XGPON_PHY_TX_SFP_CONTINUE_INT_EN     	(1<<23)
#define XGPON_PHY_TX_SFP_ABNORMAL_INT_EN     	(1<<22)
#define XGPON_PHY_TX_FAULT_INT_EN            	(1<<21)
#define XGPON_PHY_TX_BURST_SPACE_ERR_INT_EN  	(1<<20)
#define XGPON_PHY_TX_MPI_ERR_INT_EN          	(1<<19)
#define XGPON_PHY_TX_PSBU_INFO_ERR_INT_EN    	(1<<18)
#define XGPON_PHY_TX_INFO_FIFO_INT_EN        	(1<<16)
#define XGPON_PHY_RX_RDY_INT_EN              	(1<<9)
#define XGPON_PHY_RX_CW_CNT_ERR_INT_EN       	(1<<8)
#define XGPON_PHY_RX_INFO_FIFO_INT_EN        	(1<<6)
#define XGPON_PHY_RX_NGPON2_OC_ERR_INT_EN    	(1<<5)
#define XGPON_PHY_RX_FEC_ERR_INT_EN          	(1<<4)
#define XGPON_PHY_RX_BER_HIGH_INT_EN         	(1<<3)
#define XGPON_PHY_RX_LOF_INT_EN              	(1<<2)
#define XGPON_PHY_RX_SYNC_OK_INT_EN          	(1<<1)
#define XGPON_PHY_RX_LOS_INT_EN              	(1)

/* TX FEC */
#define XGPON_PHY_XG_TX_FEC_EN_CTRL_ENABLE      0x01010101
#define XGPON_PHY_XG_TX_FEC_EN_CTRL_DISABLE     0

/*FEC status*/
#define XGPON_PHY_TX_FEC						(1)	//by ang_20180115

/*RX enable*/
#define XGPON_PHY_RX_ENABLE						(1<<16)

/*Preamble Delimiter mask*/
#define XGPON_PHY_PREAMBLE_MASK					(0xFF)
#define XGPON_PHY_DELIMITER_MASK				(0xFF)


/* TX_D padding option */
#define XGPON_PHY_XG_TX_IDLE_CTRL_PATTERN_SEL(i)				((UINT32)((i)&0x3)<<16)
#define XGPON_PHY_XG_TX_IDLE_CTRL_OFF 							(0)


/*CONTINUE ctrl*/
#define XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_MODE_SEL_ALL_PRBS 	(1<<16)
#define XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_MODE_SEL(i) 		((UINT32)((i)&0x1)<<16)
#define XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_PATTERN_SEL(i)		((UINT32)((i)&0x3)<<8)
#define XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_EN					(1)
#define XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_DISABLE				(0)


/*DBG ctrl*/
#define XGPON_PHY_DBG_CTRL_DBG_CNT_CLEAR 		(1<<16)
#define XGPON_PHY_DBG_CTRL_DBG_RX_DESCRMB_EN	(1<<9)
#define XGPON_PHY_DBG_CTRL_DBG_TX_SCRMB_EN		(1<<8)
#define XGPON_PHY_DBG_CTRL_DBG_RX_FEC_OC_REF_EN	(1<<1)
#define XGPON_PHY_DBG_CTRL_DBG_RX_FEC_DISABLE	(1)


/*SFP status*/
#define XGPON_PHY_SFP_MOD_ABS_ST			(1<<3)
#define XGPON_PHY_SFP_TX_FAULT_ST			(1<<2)
#define XGPON_PHY_SFP_TX_SD_ST				(1<<1)
#define XGPON_PHY_SFP_RX_LOS_ST				(1)


/* XG PHY Reset */
#define XGPON_PHY_XG_PHY_RST_N_PHYD			0x00000002  // bit 0 = 0 
#define XGPON_PHY_XG_PHY_RST_N_SERDES		0x00000001  // bit 1 = 0
#define XGPON_PHY_XG_PHY_RST_N_ALL			0x00000000
#define XGPON_PHY_XG_PHY_RST_N_DISABLE		0x00000003

/* SFP valid level */
#define XGPON_PHY_SFP_MOD_ABS_VLD_LEVEL_LOW	    (1<<3)
#define XGPON_PHY_SFP_TX_FAULT_VLD_LEVEL_LOW    (1<<2) 
#define XGPON_PHY_SFP_TX_SD_VLD_LEVEL_LOW	    (1<<1) 
#define XGPON_PHY_SFP_RX_LOS_VLD_LEVEL_LOW	    (1) 

/* XG PHY Status */
#define XGPON_PHY_XG_PHY_STA_PHYA_RDY       1

/* RX_SYNC_OK status */
#define XGPON_PHY_DBG_RX_SYNC_ST_HUNT		(0x0)
#define XGPON_PHY_DBG_RX_SYNC_ST_PRE_SYNC	(0x1)
#define XGPON_PHY_DBG_RX_SYNC_ST_IN_SYNC	(0x2)
#define XGPON_PHY_DBG_RX_SYNC_ST_RE_SYNC	(0x3)
#define XGPON_PHY_DBG_RX_SYNC_ST_SYNC		(1<<1)


/* PM cnt clear  */
#define XGPON_PHY_PHY_PM_CNT_CLR_FEC_DECODE_CNT_CLR 	(1<<1)
#define XGPON_PHY_PHY_PM_CNT_CLR_PSBD_HEC_ERR_CNT_CLR	(1)  


/* PON ID related info */
#define XGPON_PHY_PON_ID_UPPER_DS_FEC 		(1<<14)

/*PCS RESET*/
#define XGPON_PHY_XG_PHY_RST_N_ON		(0x0)
#define XGPON_PHY_XG_PHY_RST_N_OFF		(0x3)

/*   =================  EN7580 XEPON REG  =================   */
#define REG_BASE_XEPON_PCS 0xBFAF1000

#define XEPON_PCS_TX_SP0                     (REG_BASE_XEPON_PCS+0x000)
#define XEPON_PCS_TX_SP1                     (REG_BASE_XEPON_PCS+0x004)
#define XEPON_PCS_TX_START_BST_D0            (REG_BASE_XEPON_PCS+0x008)
#define XEPON_PCS_TX_START_BST_D1            (REG_BASE_XEPON_PCS+0x00C)
#define XEPON_PCS_TX_END_BST_D0              (REG_BASE_XEPON_PCS+0x010)
#define XEPON_PCS_TX_END_BST_D1              (REG_BASE_XEPON_PCS+0x014)
#define XEPON_PCS_TX_CTRL_CFG                (REG_BASE_XEPON_PCS+0x018)
#define XEPON_PCS_RX_CTRL_CFG                (REG_BASE_XEPON_PCS+0x01C)
#define XEPON_PCS_INT_STATUS                 (REG_BASE_XEPON_PCS+0x020)
#define XEPON_PCS_INT_EN                     (REG_BASE_XEPON_PCS+0x024)
#define XEPON_PCS_CONTINUE_CTRL              (REG_BASE_XEPON_PCS+0x028)
#define XEPON_PCS_CONTINUE_USER_CFG_DATA0    (REG_BASE_XEPON_PCS+0x02C)
#define XEPON_PCS_CONTINUE_USER_CFG_DATA1    (REG_BASE_XEPON_PCS+0x030)
#define XEPON_PCS_LOGIC_RST                  (REG_BASE_XEPON_PCS+0x034)
#define XEPON_PCS_BER_MONITOR_THRE           (REG_BASE_XEPON_PCS+0x038)
#define XEPON_PCS_BER_MONITOR_INTERVAL       (REG_BASE_XEPON_PCS+0x03C)
#define XEPON_PCS_BER_START_MONITOR          (REG_BASE_XEPON_PCS+0x040)
#define XEPON_PCS_BER_CNT                    (REG_BASE_XEPON_PCS+0x044)
#define XEPON_PCS_BER_6466DEC_ERROR_CNT      (REG_BASE_XEPON_PCS+0x048)
#define XEPON_PCS_BER_FEC_DEC_ERROR_CNT      (REG_BASE_XEPON_PCS+0x04C)
#define XEPON_PCS_CLR_ALL_NUM                (REG_BASE_XEPON_PCS+0x050)
#define XEPON_PCS_RX_SOF_NUM                 (REG_BASE_XEPON_PCS+0x054)
#define XEPON_PCS_RX_EOF_NUM                 (REG_BASE_XEPON_PCS+0x058)
#define XEPON_PCS_TX_MAC_SOF_NUM             (REG_BASE_XEPON_PCS+0x05C)
#define XEPON_PCS_TX_MAC_EOF_NUM             (REG_BASE_XEPON_PCS+0x060)
#define XEPON_PCS_TX_GB_SOF_NUM              (REG_BASE_XEPON_PCS+0x064)
#define XEPON_PCS_TX_GB_EOF_NUM              (REG_BASE_XEPON_PCS+0x068)
#define XEPON_PCS_RX_SYNC_STATUS             (REG_BASE_XEPON_PCS+0x06C)
#define XEPON_PCS_PROBE_SEL                  (REG_BASE_XEPON_PCS+0x070)
#define XEPON_PCS_PROBE_H                    (REG_BASE_XEPON_PCS+0x074)
#define XEPON_PCS_PROBE_L                    (REG_BASE_XEPON_PCS+0x078)
#define XEPON_PCS_TX_TEST_CTRL_CFG           (REG_BASE_XEPON_PCS+0x07C)

#define XEPON_PCS_RX_TEST_DONE            	 (REG_BASE_XEPON_PCS+0x150)
                                                                       
#define XEPON_PCS_SFP_STATUS                 (REG_BASE_XEPON_PCS+0x224)
#define XEPON_PCS_TX_BST_INVLD_DATA          (REG_BASE_XEPON_PCS+0x228)
#define XEPON_PCS_TX_LASER_ON_PATTERN0       (REG_BASE_XEPON_PCS+0x22C)
#define XEPON_PCS_TX_LASER_ON_PATTERN1       (REG_BASE_XEPON_PCS+0x230)
#define XEPON_PCS_TX_LASER_ON_HEAD           (REG_BASE_XEPON_PCS+0x234)
#define XEPON_PCS_TX_RX_GB_ALERT_THRE        (REG_BASE_XEPON_PCS+0x238)
#define XEPON_PCS_TX_EN_DLY_SEL              (REG_BASE_XEPON_PCS+0x280)
#define XEPON_PCS_ALL_CDWD                   (REG_BASE_XEPON_PCS+0x294)
#define XEPON_PCS_ALL_NO_ERR_CDWD            (REG_BASE_XEPON_PCS+0x298)
#define XEPON_PCS_ALL_ERR_CORR_CDWD          (REG_BASE_XEPON_PCS+0x29C)
#define XEPON_PCS_ALL_ERR_NO_CORR_CDWD       (REG_BASE_XEPON_PCS+0x2A0)
#define XEPON_PCS_ALL_ERR_BYTE               (REG_BASE_XEPON_PCS+0x2A4)
#define XEPON_PCS_ALL_ERR_BIT                (REG_BASE_XEPON_PCS+0x2A8)
#define XEPON_PCS_FEC_ERR_SECOND             (REG_BASE_XEPON_PCS+0x2AC)
#define XEPON_PCS_XE_RX_INFO_CSR             (REG_BASE_XEPON_PCS+0x2B0)
#define XEPON_PCS_RX_GB_RD_THRE              (REG_BASE_XEPON_PCS+0x2B4)
#define XEPON_PCS_RX_SYNC_OK_CNT             (REG_BASE_XEPON_PCS+0x2B8)
#define XEPON_PCS_RX_SYNC_LOSS_CNT           (REG_BASE_XEPON_PCS+0x2BC)
#define XEPON_PCS_LPBK_TIME_CNT              (REG_BASE_XEPON_PCS+0x2C0)
#define XEPON_PCS_MAC_GB_SOB_CNT             (REG_BASE_XEPON_PCS+0x2C4)



/*Interrupt*/
#define XEPON_PCS_INT_MASK                     0x0000001F  // bits related to ISR = 0
#define XEPON_PCS_INT_DISABLE    				0

//Interrupt status
#define XEPON_PCS_INT_SYNC_OK				(1<<31)
#define XEPON_PCS_INT_SYNC_LOSS				(1<<30)
#define XEPON_PCS_INT_FEC_ERR				(1<<29)
#define XEPON_PCS_INT_HEAD_ERR_BIGGER_THRE	(1<<28)
#define XEPON_PCS_INT_DEC_64B66B_ERR		(1<<27)
#define XEPON_PCS_INT_PHYA_TX_RDY			(1<<26)
#define XEPON_PCS_INT_PHYA_RX_RDY			(1<<25)
#define XEPON_PCS_INT_LASER_RX_LOSS			(1<<24)
#define XEPON_PCS_INT_LASER_TX_FAULT		(1<<23)
#define XEPON_PCS_INT_BIGGER_THAN_16K		(1<<22)
#define XEPON_PCS_INT_TX_DET_OVERRUN		(1<<21)
#define XEPON_PCS_INT_TX_DET_UNDERRUN		(1<<20)
#define XEPON_PCS_INT_TX_GB_OVERRUN			(1<<19)
#define XEPON_PCS_INT_TX_GB_UNDERRUN		(1<<18)
#define XEPON_PCS_INT_RX_GB_OVERRUN			(1<<17)
#define XEPON_PCS_INT_RX_GB_EMPTY_APPEAR	(1<<16)
#define XEPON_PCS_INT_RX_SOF_NO_EOF			(1<<15)
#define XEPON_PCS_INT_RX_EOF_NO_SOF			(1<<14)
#define XEPON_PCS_INT_BER_HIGH_FLAG			(1<<13)
#define XEPON_PCS_INT_BER_MONITOR_DONE		(1<<12)
#define XEPON_PCS_INT_TX_SD_NORMAL			(1<<11)
#define XEPON_PCS_INT_TXSD_NEQUAL_TXBST		(1<<10)
#define XEPON_PCS_INT_CDR_STABLE			(1<<9) 
#define XEPON_PCS_INT_CDR_STABLE_LOSS		(1<<8) 
#define XEPON_PCS_INT_PHYA_TX_RDY_LOSS		(1<<7) 
#define XEPON_PCS_INT_PHYA_RX_RDY_LOSS		(1<<6) 
#define XEPON_PCS_INT_NOT_LASER_RX_LOSS     (1<<5) 

//Interrupt enable
#define XEPON_PCS_INT_SYNC_OK_EN              	(1<<31)
#define XEPON_PCS_INT_SYNC_LOSS_EN            	(1<<30)
#define XEPON_PCS_INT_FEC_ERR_EN              	(1<<29)
#define XEPON_PCS_INT_HEAD_ERR_BIGGER_THRE_EN 	(1<<28)
#define XEPON_PCS_INT_DEC_64B66B_ERR_EN       	(1<<27)
#define XEPON_PCS_INT_PHYA_TX_RDY_EN          	(1<<26)
#define XEPON_PCS_INT_PHYA_RX_RDY_EN          	(1<<25)
#define XEPON_PCS_INT_LASER_RX_LOSS_EN        	(1<<24)
#define XEPON_PCS_INT_LASER_TX_FAULT_EN       	(1<<23)
#define XEPON_PCS_INT_BIGGER_THAN_16K_EN      	(1<<22)
#define XEPON_PCS_INT_TX_DET_OVERRUN_EN       	(1<<21)
#define XEPON_PCS_INT_TX_DET_UNDERRUN_EN      	(1<<20)
#define XEPON_PCS_INT_TX_GB_OVERRUN_EN        	(1<<19)
#define XEPON_PCS_INT_TX_GB_UNDERRUN_EN       	(1<<18)
#define XEPON_PCS_INT_RX_GB_OVERRUN_EN        	(1<<17)
#define XEPON_PCS_INT_RX_GB_UNDERRUN_EN       	(1<<16)
#define XEPON_PCS_INT_RX_SOF_NO_EOF_EN        	(1<<15)
#define XEPON_PCS_INT_RX_EOF_NO_SOF_EN        	(1<<14)
#define XEPON_PCS_INT_BER_HIGH_FLAG_EN        	(1<<13)
#define XEPON_PCS_INT_BER_MONITOR_DONE_EN     	(1<<12)
#define XEPON_PCS_INT_TX_SD_NORMAL_EN         	(1<<11)
#define XEPON_PCS_INT_TXSD_NEQUAL_TXBST_EN    	(1<<10)
#define XEPON_PCS_INT_CDR_STABLE_EN           	(1<<9) 
#define XEPON_PCS_INT_CDR_STABLE_LOSS_EN      	(1<<8) 
#define XEPON_PCS_INT_PHYA_TX_RDY_LOSS_EN     	(1<<7) 
#define XEPON_PCS_INT_PHYA_RX_RDY_LOSS_EN     	(1<<6) 
#define XEPON_PCS_INT_NOT_LASER_RX_LOSS_EN    	(1<<5)

/*Continue ctrl*/
#define XEPON_PCS_CONTINUE_CTRL_MODE_NORMAL		(1<<7)
#define XEPON_PCS_CONTINUE_CTRL_MODE_SEL(i) 	((UINT32)((i)&0x1)<<7)
#define XEPON_PCS_CONTINUE_CTRL_DATA_SEL(i) 	((UINT32)((i)&0x3)<<1)
#define XEPON_PCS_CONTINUE_CTRL_EN				(1)
#define XEPON_PCS_CONTINUE_CTRL_DISABLE			(0)

/*PHY ready*/
#define XEPON_PCS_RX_SYNC_STATUS_OK				(1<<31)

/*SFP status*/
#define XEPON_PCS_SFP_STATUS_TX_FAULT_POLARITY_CFG_HIGH		(1<<31)
#define XEPON_PCS_SFP_STATUS_TX_FAULT         				(1<<30)
#define XEPON_PCS_SFP_STATUS_RX_LOSS_POLARITY_CFG_HIGH	 	(1<<29)
#define XEPON_PCS_SFP_STATUS_RX_LOSS         				(1<<28)
#define XEPON_PCS_SFP_STATUS_TX_SD_POLARITY_CFG_HIGH	    (1<<27)
#define XEPON_PCS_SFP_STATUS_TX_SD         					(1<<26)
#define XEPON_PCS_SFP_STATUS_MOD_ABS         				(1<<25)
#define XEPON_PCS_SFP_STATUS_P_DOWN         				(1<<24)
#define XEPON_PCS_SFP_STATUS_TX_BST_POLARITY_CFG_HIGH    	(1<<23)

/*PCS reset*/
#define XEPON_PCS_LOGIC_RST_ON 					0
#define XEPON_PCS_LOGIC_RST_OFF 				1

/*PCS Tx Ctrl Cfg*/
#define XEPON_PCS_TX_CTRL_CFG_TX_GB_THR_MASK 	0xffffffe0
#define XEPON_PCS_TX_CTRL_CFG_FEC_EN			(1<<9)
#define XEPON_PCS_TX_CTRL_CFG_SCR_EN			(1<<8) 

/*PCS Rx Ctrl Cfg*/
#define XEPON_PCS_RX_CTRL_CFG_FEC_DEC_EN 		(1<<31)
#define XEPON_PCS_RX_CTRL_CFG_DESCR_EN 			(1<<23)
#define XEPON_PCS_RX_CTRL_CFG_PCS_RX_EN			(1<<1)
#define XEPON_PCS_RX_CTRL_CFG_BYPASS_SYNC 		(1)
#define XEPON_PCS_RX_CTRL_CFG_SYNC_TIMES(i) 	((UINT32)((i)&0x7)<<5)



/*PCS clear all counter */
#define XEPON_PCS_CLR_ALL_NUM_CLR 				(1)

/*PCS Tx test*/
#define XEPON_PCS_TX_TEST_CTRL_CFG_LOOP_BACK 			(1<<31)
#define XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN 				(1<<28)
#define XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG 	(1<<24)
#define XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(i) 	((UINT32)((i)&0xff)<<8)
#define XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG 		(1)

/*PCS Rx test*/
#define XEPON_PCS_RX_TEST_DONE_RX_TEST_DONE 			(1)
#define XEPON_PCS_RX_BER_START_MONITOR					(1)

/*FEC status*/
#define XEPON_PHY_RX_FEC							(1<<31)
#define XEPON_SYMM_PHY_TX_FEC						(1<<9)
#define XEPON_ASYM_PHY_TX_FEC						(1<<15)

/*PCS ENABLE*/
#define XEPON_PCS_RX_ENABLE						0x80810302
#define XEPON_PCS_RX_DISABLE					0x80810300



/*   =================  EN7580 XEPON 1G mode REG  =================   */

#define REG_BASE_XEPON_1G						0xBFAF1500           //control system register 

#define XEPON_1G_PHYSET2                           (REG_BASE_XEPON_1G+0x104)
#define XEPON_1G_PHYSET3                           (REG_BASE_XEPON_1G+0x108)
#define XEPON_1G_PHYSTA1                           (REG_BASE_XEPON_1G+0x130)

#define XEPON_1G_EPON_SYNC_CTL                     (REG_BASE_XEPON_1G+0x200)
#define XEPON_1G_EPON_FRAM_SRH_CTL                 (REG_BASE_XEPON_1G+0x204)
#define XEPON_1G_GPON_PSYNC_CTL                    (REG_BASE_XEPON_1G+0x20C)
#define XEPON_1G_GPON_INDENT_CTL                   (REG_BASE_XEPON_1G+0x210)
#define XEPON_1G_RS_CTL                            (REG_BASE_XEPON_1G+0x214)
#define XEPON_1G_PHYRX_TEST_CTL                    (REG_BASE_XEPON_1G+0x218)
#define XEPON_1G_PHYRX_STATUS                      (REG_BASE_XEPON_1G+0x21C)
#define XEPON_1G_PHYRX_MISC_TRIG                   (REG_BASE_XEPON_1G+0x220)
#define XEPON_1G_PHYRX_TEST_DBG_TRIG               (REG_BASE_XEPON_1G+0x224)
#define XEPON_1G_PHYRX_EPON_FRAME_ERR              (REG_BASE_XEPON_1G+0x228)
#define XEPON_1G_RX_SRAM_BIST_STATUS               (REG_BASE_XEPON_1G+0x22C)
#define XEPON_1G_XP_ERRCNT_EN                      (REG_BASE_XEPON_1G+0x230)
#define XEPON_1G_XP_ERRCNT_CTL                     (REG_BASE_XEPON_1G+0x234)
#define XEPON_1G_ERR_BYTE_CNT                      (REG_BASE_XEPON_1G+0x238)
#define XEPON_1G_ERR_CODE_CNT                      (REG_BASE_XEPON_1G+0x23C)
#define XEPON_1G_NOSOL_CODE_CNT                    (REG_BASE_XEPON_1G+0x240)
#define XEPON_1G_RX_CODE_CNT                       (REG_BASE_XEPON_1G+0x244)
#define XEPON_1G_FEC_SECONDS                       (REG_BASE_XEPON_1G+0x248)
#define XEPON_1G_BIP_CNT                           (REG_BASE_XEPON_1G+0x24C)
#define XEPON_1G_FRAME_CNT_L                       (REG_BASE_XEPON_1G+0x250)
#define XEPON_1G_FRAME_CNT_H                       (REG_BASE_XEPON_1G+0x254)
#define XEPON_1G_LOF_CNT                           (REG_BASE_XEPON_1G+0x258)
#define XEPON_1G_FECDEC_TESTCTL                    (REG_BASE_XEPON_1G+0x260)
#define XEPON_1G_FECRS_TESTCTL                     (REG_BASE_XEPON_1G+0x264)
#define XEPON_1G_FECDEC_CTL                        (REG_BASE_XEPON_1G+0x268)
#define XEPON_1G_FECDEC_SRAMCTL                    (REG_BASE_XEPON_1G+0x270)
#define XEPON_1G_SRAM_TEST_RX_CTRL                 (REG_BASE_XEPON_1G+0x280)
#define XEPON_1G_SRAM_TEST_RX_WR_DATA              (REG_BASE_XEPON_1G+0x284)
#define XEPON_1G_SRAM_TEST_RX_RD_DATA              (REG_BASE_XEPON_1G+0x288)
#define XEPON_1G_SRAM_TEST_RX_RW_TRIG              (REG_BASE_XEPON_1G+0x28C)
#define XEPON_1G_DUMMY_REG_0290                    (REG_BASE_XEPON_1G+0x290)
#define XEPON_1G_PHYRX_RESET                       (REG_BASE_XEPON_1G+0x294)
#define XEPON_1G_PHYRX_EPON_CNT_CTL                (REG_BASE_XEPON_1G+0x298)
#define XEPON_1G_PHYRX_EPON_SYNC_STATUS            (REG_BASE_XEPON_1G+0x29C)
#define XEPON_1G_PRBS_ALIGN_PATTERN                (REG_BASE_XEPON_1G+0x2A0)
#define XEPON_1G_PRBS_ALIGN_MASK                   (REG_BASE_XEPON_1G+0x2A4)
#define XEPON_1G_PRBS_ERR_CNT                      (REG_BASE_XEPON_1G+0x2C0)
#define XEPON_1G_PRBS_LOOPBACK_STATUS              (REG_BASE_XEPON_1G+0x2C4)
#define XEPON_1G_RPMA_SRAM_CTRL                    (REG_BASE_XEPON_1G+0x2C8)
#define XEPON_1G_ROUND_TRIP_DELAY_CTRL             (REG_BASE_XEPON_1G+0x2CC)
#define XEPON_1G_ROUND_TRIP_CAL_MASK_CTRL          (REG_BASE_XEPON_1G+0x2D0)
#define XEPON_1G_ROUND_TRIP_DELAY_VALUE            (REG_BASE_XEPON_1G+0x2D4)
#define XEPON_1G_ROUND_TRIP_DELAY_VALUE_STATIC     (REG_BASE_XEPON_1G+0x2D8)
#define XEPON_1G_PSYNC_DET_ALIGN_PHASE             (REG_BASE_XEPON_1G+0x2DC)
#define XEPON_1G_RX_TX_HEAD_TO_HEAD_DELAY          (REG_BASE_XEPON_1G+0x2E0)
#define XEPON_1G_ROUND_TRIP_CAL_MASK_CTRL_1        (REG_BASE_XEPON_1G+0x2E4)
#define XEPON_1G_ROUND_TRIP_DELAY_VALUE_1          (REG_BASE_XEPON_1G+0x2E8)
#define XEPON_1G_ROUND_TRIP_DELAY_VALUE_2          (REG_BASE_XEPON_1G+0x2EC)
#define XEPON_1G_ROUND_TRIP_DELAY_VALUE_STATIC_1   (REG_BASE_XEPON_1G+0x2F0)
#define XEPON_1G_ROUND_TRIP_DELAY_VALUE_STATIC_2   (REG_BASE_XEPON_1G+0x2F4)
#define XEPON_1G_RX_TX_HEAD_TO_HEAD_DELAY_1        (REG_BASE_XEPON_1G+0x2F8)
#define XEPON_1G_RX_TX_HEAD_TO_HEAD_DELAY_2        (REG_BASE_XEPON_1G+0x2FC)
#define XEPON_1G_PSYNC_UPDATE_CTRL                 (REG_BASE_XEPON_1G+0x300)
#define XEPON_1G_PSYNC_FORCE_VALUE                 (REG_BASE_XEPON_1G+0x304)
#define XEPON_1G_PSYNC_DET_TINMEOUT                (REG_BASE_XEPON_1G+0x308)
#define XEPON_1G_PSYNC_DIFF_TIMER                  (REG_BASE_XEPON_1G+0x30C)
#define XEPON_1G_FOUND_PSYNC_TIMER                 (REG_BASE_XEPON_1G+0x310)
#define XEPON_1G_ABS_TIMER                         (REG_BASE_XEPON_1G+0x314)
#define XEPON_1G_PSYNC_DET_CNT                     (REG_BASE_XEPON_1G+0x318)
#define XEPON_1G_MGMII_PHY_DELAY                   (REG_BASE_XEPON_1G+0x3A0)
#define XEPON_1G_RX_FIFO_EMPTY_CNT                 (REG_BASE_XEPON_1G+0x3A4)
#define XEPON_1G_GPON_PREAMBLE                     (REG_BASE_XEPON_1G+0x400)
#define XEPON_1G_GPON_DELIMITER_GUARD              (REG_BASE_XEPON_1G+0x404)
#define XEPON_1G_GPON_EXTENDED_PREAMBLE            (REG_BASE_XEPON_1G+0x408)
#define XEPON_1G_PHYTX_STATUS                      (REG_BASE_XEPON_1G+0x40C)
#define XEPON_1G_EPON_TX_CTL                       (REG_BASE_XEPON_1G+0x410)
#define XEPON_1G_PHYTX_TEST_CTL                    (REG_BASE_XEPON_1G+0x420)
#define XEPON_1G_PHYTX_TEST_TRIG                   (REG_BASE_XEPON_1G+0x424)
#define XEPON_1G_EPON_LOOPBAK_MODE                 (REG_BASE_XEPON_1G+0x428)
#define XEPON_1G_PHYTX_FECENC_SW_RST               (REG_BASE_XEPON_1G+0x42C)
#define XEPON_1G_PHYTX_MISC                        (REG_BASE_XEPON_1G+0x430)
#define XEPON_1G_TX_FRAME_COUNTER                  (REG_BASE_XEPON_1G+0x434)
#define XEPON_1G_TX_BURST_COUNTER                  (REG_BASE_XEPON_1G+0x438)
#define XEPON_1G_TX_FEC_FRAME_COUNTER              (REG_BASE_XEPON_1G+0x43C)
#define XEPON_1G_SRAM_TEST_TX_CTRL                 (REG_BASE_XEPON_1G+0x440)
#define XEPON_1G_SRAM_TEST_TX_RD_DATA              (REG_BASE_XEPON_1G+0x448)
#define XEPON_1G_SRAM_TEST_TX_RW_TRIG              (REG_BASE_XEPON_1G+0x44C)
#define XEPON_1G_FECENC_STATUS                     (REG_BASE_XEPON_1G+0x450)
#define XEPON_1G_DUMMY_REG_0460                    (REG_BASE_XEPON_1G+0x460)
#define XEPON_1G_TX_SRAM_BIST_STATUS               (REG_BASE_XEPON_1G+0x464)
#define XEPON_1G_TX_MULT2                          (REG_BASE_XEPON_1G+0x490)
#define XEPON_1G_BISTCTL_LOOPBACK_SEL              (REG_BASE_XEPON_1G+0x4A0)
#define XEPON_1G_BISTCTL_PRBS_TX_EN                (REG_BASE_XEPON_1G+0x4A4)
#define XEPON_1G_CID_LENGTH_SEL                    (REG_BASE_XEPON_1G+0x4B0)
#define XEPON_1G_TPMA_SRAM_CTRL                    (REG_BASE_XEPON_1G+0x4B4)
#define XEPON_1G_TEST_FRAME_EN                     (REG_BASE_XEPON_1G+0x510)
#define XEPON_1G_TEST_FRAME_NUM                    (REG_BASE_XEPON_1G+0x514)
#define XEPON_1G_TEST_FRAME_MAX_LENGTH             (REG_BASE_XEPON_1G+0x518)
#define XEPON_1G_TEST_FRAME_MIN_LENGTH             (REG_BASE_XEPON_1G+0x51C)
#define XEPON_1G_TF_PAT_SEL                        (REG_BASE_XEPON_1G+0x520)
#define XEPON_1G_TEST_FRAME_ERR_CNT                (REG_BASE_XEPON_1G+0x524)
#define XEPON_1G_TEST_FRAME_TX_INFO                (REG_BASE_XEPON_1G+0x528)
#define XEPON_1G_TEST_FRAME_DONE                   (REG_BASE_XEPON_1G+0x52C)
#define XEPON_1G_EPON_SPD                          (REG_BASE_XEPON_1G+0x530)
#define XEPON_1G_EPON_PREAMBLE0                    (REG_BASE_XEPON_1G+0x534)
#define XEPON_1G_EPON_PREAMBLE1                    (REG_BASE_XEPON_1G+0x538)
#define XEPON_1G_EPON_SFD                          (REG_BASE_XEPON_1G+0x53C)
#define XEPON_1G_EPON_TF_DST0                      (REG_BASE_XEPON_1G+0x540)
#define XEPON_1G_EPON_TF_DST1                      (REG_BASE_XEPON_1G+0x544)
#define XEPON_1G_EPON_TF_SRC0                      (REG_BASE_XEPON_1G+0x548)
#define XEPON_1G_EPON_TF_SRC1                      (REG_BASE_XEPON_1G+0x54C)
#define XEPON_1G_EPON_TF_LEN_TYPE                  (REG_BASE_XEPON_1G+0x550)
#define XEPON_1G_EPON_TF_FIRST_DATA0               (REG_BASE_XEPON_1G+0x554)
#define XEPON_1G_EPON_TF_FIRST_DATA1               (REG_BASE_XEPON_1G+0x558)
#define XEPON_1G_EPON_TF_FIRST_DATA2               (REG_BASE_XEPON_1G+0x55C)
#define XEPON_1G_EPON_TF_FIRST_DATA3               (REG_BASE_XEPON_1G+0x560)
#define XEPON_1G_EPON_TF_FIRST_DATA4               (REG_BASE_XEPON_1G+0x564)
#define XEPON_1G_EPON_TF_FIRST_DATA5               (REG_BASE_XEPON_1G+0x568)
#define XEPON_1G_EPON_TF_FIRST_DATA6               (REG_BASE_XEPON_1G+0x56C)
#define XEPON_1G_EPON_TF_FIRST_DATA7               (REG_BASE_XEPON_1G+0x570)
#define XEPON_1G_XPON_STA                          (REG_BASE_XEPON_1G+0x5E0)
#define XEPON_1G_GIO1_SETTING                      (REG_BASE_XEPON_1G+0x5E8)
#define XEPON_1G_GIO2_SETTING                      (REG_BASE_XEPON_1G+0x5EC)
#define XEPON_1G_XPON_INT_EN                       (REG_BASE_XEPON_1G+0x5F0)
#define XEPON_1G_XPON_INT_STA_CLR                  (REG_BASE_XEPON_1G+0x5F4)
#define XEPON_1G_XPON_INT_STA                      (REG_BASE_XEPON_1G+0x5F8)
#define XEPON_1G_EPON_TF_ASYNC_FIFO                (REG_BASE_XEPON_1G+0x5FC)


/*Interrupt*/
#define XEPON_1G_XPON_INT_MASK						0xFFFFFF00

//Interrupt enable
#define XEPON_1G_XPON_I2CM_IRQ_INT_EN				(1<<7)
#define XEPON_1G_XPON_ILLEGAL_INT_EN           	(1<<6)
#define XEPON_1G_XPON_PHYRDY_INT_EN            	(1<<5)
#define XEPON_1G_XPON_TRANS_SD_FAIL_INT_EN     	(1<<4)
#define XEPON_1G_XPON_TRANS_INT_EN             	(1<<3)
#define XEPON_1G_XPON_TRANS_FAULT_INT_EN       	(1<<2)
#define XEPON_1G_XPON_LOF_INT_EN               	(1<<1)
#define XEPON_1G_XPON_TRANS_LOS_INT_EN         	(1)

//Interrupt status clear
#define XEPON_1G_XPON_I2CM_IRQ_INTCLR        		(1<<7)
#define XEPON_1G_XPON_ILLEGAL_INTCLR             	(1<<6)
#define XEPON_1G_XPON_PHYRDY_INTCLR              	(1<<5)
#define XEPON_1G_XPON_TRANS_SD_FAIL_INTCLR       	(1<<4)
#define XEPON_1G_XPON_TRANS_INTCLR               	(1<<3)
#define XEPON_1G_XPON_TRANS_FAULT_INTCLR         	(1<<2)
#define XEPON_1G_XPON_LOF_INTCLR                 	(1<<1)
#define XEPON_1G_XPON_TRANS_LOS_INTCLR           	(1) 

//Interrupt status
#define XEPON_1G_XPON_I2CM_IRQ_INT           		(1<<7)
#define XEPON_1G_XPON_ILLEGAL_INT                	(1<<6)
#define XEPON_1G_XPON_PHYRDY_INT                 	(1<<5)
#define XEPON_1G_XPON_TRANS_SD_FAIL_INT          	(1<<4)
#define XEPON_1G_XPON_TRANS_INT                  	(1<<3)
#define XEPON_1G_XPON_TRANS_FAULT_INT            	(1<<2)
#define XEPON_1G_XPON_LOF_INT                    	(1<<1)
#define XEPON_1G_XPON_TRANS_LOS_INT              	(1)

/*error cnt en*/
#define XEPON_1G_XP_ERRCNT_EN_ERRCNT				1
#define XEPON_1G_XP_ERRCNT_EN_BIPCNT				(1<<1)
#define XEPON_1G_XP_ERRCNT_EN_FMCNT				(1<<2)
#define XEPON_1G_XP_ERRCNT_EN_ERRCNT_OFFSET		0
#define XEPON_1G_XP_ERRCNT_EN_BIPCNT_OFFSET		1
#define XEPON_1G_XP_ERRCNT_EN_FMCNT_OFFSET			2
#define XEPON_1G_XP_ERRCNT_EN_MASK					0xfffffff8

/*error cnt CTL*/
#define XEPON_1G_XP_ERRCNT_CTL_ERRCNT_LATCH		1
#define XEPON_1G_XP_ERRCNT_CTL_ERRCNT_CLR			(1<<1)
#define XEPON_1G_XP_ERRCNT_CTL_BIPCNT_LATCH		(1<<2)
#define XEPON_1G_XP_ERRCNT_CTL_BIPCNT_CLR			(1<<3)
#define XEPON_1G_XP_ERRCNT_CTL_FMCNT_LATCH			(1<<4)
#define XEPON_1G_XP_ERRCNT_CTL_FMCNT_CLR			(1<<5)

/*XPON STA*/
#define XEPON_1G_XPON_STA_LOS						1

/*PMA INT STA*/
#define XPON_PMA_PHYRDY_INT							(1<<16)
#define XPON_PMA_FBCK_LOCK							1

#if ASIC_SERDES
/*   =================  EN7580 XPON PMA REG  =================   */

#define XPON_PMA_BASE                    0xBFAF3000
#define XFI_PMA_BASE                     0xBFAF6000
#define MD32_BASE                        0xBFAF5000


#define XPON_PMA_RG_XPON_CMN_EN_0                     (XPON_PMA_BASE+0x100)
#define XPON_PMA_RG_XPON_BENTX_BIAS_EN_0              (XPON_PMA_BASE+0x104)
#define XPON_PMA_RG_XPON_RX_MPXSEL_0                  (XPON_PMA_BASE+0x108)
#define XPON_PMA_RG_XPON_RX_REV_0_0                   (XPON_PMA_BASE+0x10C)
#define XPON_PMA_RG_XPON_RX_PHYCK_DIV_0               (XPON_PMA_BASE+0x110)
#define XPON_PMA_RG_XPON_CDR_PD_PICAL_CKD8_INV_0      (XPON_PMA_BASE+0x114)
#define XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0             (XPON_PMA_BASE+0x118)
#define XPON_PMA_RG_XPON_CDR_LPF_SETVALUE_0           (XPON_PMA_BASE+0x11C)
#define XPON_PMA_RG_XPON_CDR_PR_INJ_FORCE_OFF_0       (XPON_PMA_BASE+0x120)
#define XPON_PMA_RG_XPON_CDR_PR_CKREF_DIV_0           (XPON_PMA_BASE+0x124)
#define XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0            (XPON_PMA_BASE+0x128)
#define XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0          (XPON_PMA_BASE+0x12C)
#define XPON_PMA_RG_XPON_PLL_IC_0                     (XPON_PMA_BASE+0x130)
#define XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0            (XPON_PMA_BASE+0x134)
#define XPON_PMA_RG_XPON_PLL_SDM_ORD_0                (XPON_PMA_BASE+0x138)
#define XPON_PMA_RG_XPON_PLL_RESERVE0_0               (XPON_PMA_BASE+0x13C)
#define XPON_PMA_RGS_XPON_PLL_AUTOK_BAND_0            (XPON_PMA_BASE+0x140)
#define XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0           (XPON_PMA_BASE+0x144)
#define XPON_PMA_RG_XPON_TX_EN_0                      (XPON_PMA_BASE+0x148)
#define XPON_PMA_RG_XPON_TX_LSDATA_EN_0               (XPON_PMA_BASE+0x14C)
#define XPON_PMA_RG_XPON_TX_RESERVED_0                (XPON_PMA_BASE+0x150)
#define XPON_PMA_SS_TX_DA_1                           (XPON_PMA_BASE+0x170)
#define XPON_PMA_SS_LCPLL_PWCTL_SETTING_0             (XPON_PMA_BASE+0x200)
#define XPON_PMA_SS_LCPLL_PWCTL_SETTING_1             (XPON_PMA_BASE+0x204)
#define XPON_PMA_SS_LCPLL_PWCTL_SETTING_2             (XPON_PMA_BASE+0x208)
#define XPON_PMA_SS_LCPLL_PWCTL_DBG_SETTING           (XPON_PMA_BASE+0x20C)
#define XPON_PMA_SS_LCPLL_TDC_PW_0                    (XPON_PMA_BASE+0x210)
#define XPON_PMA_SS_LCPLL_TDC_PW_1                    (XPON_PMA_BASE+0x214)
#define XPON_PMA_SS_LCPLL_TDC_PW_2                    (XPON_PMA_BASE+0x218)
#define XPON_PMA_SS_LCPLL_TDC_PW_3                    (XPON_PMA_BASE+0x21C)
#define XPON_PMA_SS_LCPLL_TDC_PW_4                    (XPON_PMA_BASE+0x220)
#define XPON_PMA_SS_LCPLL_TDC_PW_5                    (XPON_PMA_BASE+0x224)
#define XPON_PMA_SS_LCPLL_TDC_FLT_0                   (XPON_PMA_BASE+0x228)
#define XPON_PMA_SS_LCPLL_TDC_FLT_1                   (XPON_PMA_BASE+0x22C)
#define XPON_PMA_SS_LCPLL_TDC_FLT_2                   (XPON_PMA_BASE+0x230)
#define XPON_PMA_SS_LCPLL_TDC_FLT_3                   (XPON_PMA_BASE+0x234)
#define XPON_PMA_SS_LCPLL_TDC_FLT_4                   (XPON_PMA_BASE+0x238)
#define XPON_PMA_SS_LCPLL_TDC_FLT_5                   (XPON_PMA_BASE+0x23C)
#define XPON_PMA_SS_LCPLL_TDC_FLT_6                   (XPON_PMA_BASE+0x240)
#define XPON_PMA_SS_LCPLL_TDC_FLT_7                   (XPON_PMA_BASE+0x244)
#define XPON_PMA_SS_LCPLL_TDC_PCW_1                   (XPON_PMA_BASE+0x248)
#define XPON_PMA_SS_LCPLL_TDC_PCW_2                   (XPON_PMA_BASE+0x24C)
#define XPON_PMA_SS_LCPLL_TDC_RO_1                    (XPON_PMA_BASE+0x250)
#define XPON_PMA_SS_LCPLL_TDC_RO_2                    (XPON_PMA_BASE+0x254)
#define XPON_PMA_SS_LCPLL_TDC_RO_3                    (XPON_PMA_BASE+0x258)
#define XPON_PMA_SS_LCPLL_TDC_RO_4                    (XPON_PMA_BASE+0x25C)
#define XPON_PMA_SS_LCPLL_TDC_RO_5                    (XPON_PMA_BASE+0x260)
#define XPON_PMA_SS_LCPLL_TDC_CTRL_0                  (XPON_PMA_BASE+0x264)
#define XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0           (XPON_PMA_BASE+0x300)
#define XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1           (XPON_PMA_BASE+0x304)
#define XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2           (XPON_PMA_BASE+0x308)
#define XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3           (XPON_PMA_BASE+0x30C)
#define XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0         (XPON_PMA_BASE+0x320)
#define XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1         (XPON_PMA_BASE+0x324)
#define XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0             (XPON_PMA_BASE+0x330)
#define XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1             (XPON_PMA_BASE+0x334)
#define XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_2             (XPON_PMA_BASE+0x338)
#define XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0              (XPON_PMA_BASE+0x340)
#define XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1              (XPON_PMA_BASE+0x344)
#define XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2              (XPON_PMA_BASE+0x348)
#define XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3              (XPON_PMA_BASE+0x34C)
#define XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4              (XPON_PMA_BASE+0x350)
#define XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5              (XPON_PMA_BASE+0x354)
#define XPON_PMA_RX_CTRL_SEQUENCE_CTRL_6              (XPON_PMA_BASE+0x358)
#define XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0         (XPON_PMA_BASE+0x35C)
#define XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1         (XPON_PMA_BASE+0x360)
#define XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0        (XPON_PMA_BASE+0x364)
#define XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1        (XPON_PMA_BASE+0x368)
#define XPON_PMA_PHY_EQ_CTRL_0                        (XPON_PMA_BASE+0x370)
#define XPON_PMA_PHY_EQ_CTRL_1                        (XPON_PMA_BASE+0x374)
#define XPON_PMA_PHY_EQ_CTRL_2                        (XPON_PMA_BASE+0x378)
#define XPON_PMA_PHY_EQ_CTRL_3                        (XPON_PMA_BASE+0x37C)
#define XPON_PMA_PHY_EQ_CTRL_4                        (XPON_PMA_BASE+0x380)
#define XPON_PMA_PHY_EQ_CTRL_5                        (XPON_PMA_BASE+0x384)
#define XPON_PMA_PHY_EQ_CTRL_6                        (XPON_PMA_BASE+0x388)
#define XPON_PMA_PHY_EQ_CTRL_7                        (XPON_PMA_BASE+0x38C)
#define XPON_PMA_PHY_EQ_CTRL_8                        (XPON_PMA_BASE+0x390)
#define XPON_PMA_PHY_EQ_CTRL_9                        (XPON_PMA_BASE+0x394)
#define XPON_PMA_PHY_EQ_CTRL_10                       (XPON_PMA_BASE+0x398)
#define XPON_PMA_SS_RX_FEOS                           (XPON_PMA_BASE+0x400)
#define XPON_PMA_SS_RX_BLWC                           (XPON_PMA_BASE+0x410)
#define XPON_PMA_SS_RX_FREQ_DET_1                     (XPON_PMA_BASE+0x420)
#define XPON_PMA_SS_RX_FREQ_DET_2                     (XPON_PMA_BASE+0x424)
#define XPON_PMA_SS_RX_FREQ_DET_3                     (XPON_PMA_BASE+0x428)
#define XPON_PMA_SS_RX_FREQ_DET_4                     (XPON_PMA_BASE+0x42C)
#define XPON_PMA_SS_RX_PI_CAL                         (XPON_PMA_BASE+0x430)
#define XPON_PMA_SS_RX_CAL_1                          (XPON_PMA_BASE+0x440)
#define XPON_PMA_SS_RX_CAL_2                          (XPON_PMA_BASE+0x444)
#define XPON_PMA_SS_RX_SIGDET_0                       (XPON_PMA_BASE+0x450)
#define XPON_PMA_SS_RX_SIGDET_1                       (XPON_PMA_BASE+0x454)
#define XPON_PMA_SS_RX_FLL_0                          (XPON_PMA_BASE+0x460)
#define XPON_PMA_SS_RX_FLL_1                          (XPON_PMA_BASE+0x464)
#define XPON_PMA_SS_RX_FLL_2                          (XPON_PMA_BASE+0x468)
#define XPON_PMA_SS_RX_FLL_3                          (XPON_PMA_BASE+0x46C)
#define XPON_PMA_SS_RX_FLL_4                          (XPON_PMA_BASE+0x470)
#define XPON_PMA_SS_RX_FLL_5                          (XPON_PMA_BASE+0x474)
#define XPON_PMA_SS_RX_FLL_6                          (XPON_PMA_BASE+0x478)
#define XPON_PMA_SS_RX_FLL_7                          (XPON_PMA_BASE+0x47C)
#define XPON_PMA_SS_RX_FLL_8                          (XPON_PMA_BASE+0x480)
#define XPON_PMA_SS_RX_FLL_9                          (XPON_PMA_BASE+0x484)
#define XPON_PMA_SS_RX_FLL_a                          (XPON_PMA_BASE+0x488)
#define XPON_PMA_SS_RX_FLL_b                          (XPON_PMA_BASE+0x48C)
#define XPON_PMA_RX_PDOS_CTRL_0                       (XPON_PMA_BASE+0x490)
#define XPON_PMA_RX_RESET_0                           (XPON_PMA_BASE+0x494)
#define XPON_PMA_RX_RESET_1                           (XPON_PMA_BASE+0x498)
#define XPON_PMA_RX_DEBUG_0                           (XPON_PMA_BASE+0x49C)
#define XPON_PMA_BISTCTL_CONTROL                      (XPON_PMA_BASE+0x500)
#define XPON_PMA_BISTCTL_ALIGN_PAT                    (XPON_PMA_BASE+0x504)
#define XPON_PMA_BISTCTL_PROGRAM_PAT_0                (XPON_PMA_BASE+0x508)
#define XPON_PMA_BISTCTL_PROGRAM_PAT_1                (XPON_PMA_BASE+0x50C)
#define XPON_PMA_BISTCTL_POLLUTION                    (XPON_PMA_BASE+0x510)
#define XPON_PMA_BISTCTL_PRBS_INITIAL_SEED            (XPON_PMA_BASE+0x514)
#define XPON_PMA_BISTCTL_PRBS_EVENT                   (XPON_PMA_BASE+0x518)
#define XPON_PMA_BISTCTL_PRBS_ERRCNT                  (XPON_PMA_BASE+0x51C)
#define XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD          (XPON_PMA_BASE+0x520)
#define XPON_PMA_RX_TORGS_DEBUG_0                     (XPON_PMA_BASE+0x524)
#define XPON_PMA_RX_TORGS_DEBUG_1                     (XPON_PMA_BASE+0x528)
#define XPON_PMA_RX_TORGS_DEBUG_2                     (XPON_PMA_BASE+0x52C)
#define XPON_PMA_RX_TORGS_DEBUG_3                     (XPON_PMA_BASE+0x530)
#define XPON_PMA_RX_TORGS_DEBUG_4                     (XPON_PMA_BASE+0x534)
#define XPON_PMA_RX_TORGS_DEBUG_5                     (XPON_PMA_BASE+0x538)
#define XPON_PMA_RX_TORGS_DEBUG_6                     (XPON_PMA_BASE+0x53C)
#define XPON_PMA_RX_TORGS_DEBUG_7                     (XPON_PMA_BASE+0x540)
#define XPON_PMA_RX_TORGS_DEBUG_8                     (XPON_PMA_BASE+0x544)
#define XPON_PMA_RX_TORGS_DEBUG_9                     (XPON_PMA_BASE+0x548)
#define XPON_PMA_RX_TORGS_DEBUG_10                    (XPON_PMA_BASE+0x54C)
#define XPON_PMA_SS_TX_RST_B                          (XPON_PMA_BASE+0x550)
#define XPON_PMA_SS_TX_CALIB_0                        (XPON_PMA_BASE+0x554)
#define XPON_PMA_SS_TX_CALIB_1                        (XPON_PMA_BASE+0x558)
#define XPON_PMA_SS_TX_CALIB_2                        (XPON_PMA_BASE+0x55C)
#define XPON_PMA_XPON_SETTING_0                       (XPON_PMA_BASE+0x600)
#define XPON_PMA_XPON_SETTING_1                       (XPON_PMA_BASE+0x604)
#define XPON_PMA_XPON_SETTING_2                       (XPON_PMA_BASE+0x608)
#define XPON_PMA_XPON_STA                             (XPON_PMA_BASE+0x60C)
#define XPON_PMA_XPON_INT_EN_0                        (XPON_PMA_BASE+0x610)
#define XPON_PMA_XPON_INT_EN_1                        (XPON_PMA_BASE+0x614)
#define XPON_PMA_XPON_INT_STA_0                       (XPON_PMA_BASE+0x620)
#define XPON_PMA_XPON_INT_STA_1                       (XPON_PMA_BASE+0x624)
#define XPON_PMA_RX_TORGS_DEBUG_11                    (XPON_PMA_BASE+0x628)
#define XPON_PMA_RX_FORCE_MODE_0                      (XPON_PMA_BASE+0x630)
#define XPON_PMA_RX_FORCE_MODE_1                      (XPON_PMA_BASE+0x634)
#define XPON_PMA_RX_FORCE_MODE_2                      (XPON_PMA_BASE+0x638)
#define XPON_PMA_RX_DISB_MODE_0                       (XPON_PMA_BASE+0x63C)
#define XPON_PMA_RX_DISB_MODE_1                       (XPON_PMA_BASE+0x640)
#define XPON_PMA_RX_DISB_MODE_2                       (XPON_PMA_BASE+0x644)
#define XPON_PMA_RX_FORCE_MODE_3                      (XPON_PMA_BASE+0x648)
#define XPON_PMA_RX_FORCE_MODE_4                      (XPON_PMA_BASE+0x64C)
#define XPON_PMA_RX_FORCE_MODE_5                      (XPON_PMA_BASE+0x650)
#define XPON_PMA_RX_FORCE_MODE_6                      (XPON_PMA_BASE+0x654)
#define XPON_PMA_RX_DISB_MODE_3                       (XPON_PMA_BASE+0x658)
#define XPON_PMA_RX_DISB_MODE_4                       (XPON_PMA_BASE+0x65C)
#define XPON_PMA_RX_DISB_MODE_5                       (XPON_PMA_BASE+0x660)
#define XPON_PMA_RX_FORCE_MODE_7                      (XPON_PMA_BASE+0x664)
#define XPON_PMA_RX_FORCE_MODE_8                      (XPON_PMA_BASE+0x668)
#define XPON_PMA_RX_FORCE_MODE_9                      (XPON_PMA_BASE+0x66C)
#define XPON_PMA_RX_DISB_MODE_6                       (XPON_PMA_BASE+0x670)
#define XPON_PMA_RX_DISB_MODE_7                       (XPON_PMA_BASE+0x674)
#define XPON_PMA_RX_DISB_MODE_8                       (XPON_PMA_BASE+0x678)
#define XPON_PMA_SS_BIST_0                            (XPON_PMA_BASE+0x680)
#define XPON_PMA_SS_BIST_1                            (XPON_PMA_BASE+0x684)
#define XPON_PMA_SS_BIST_2                            (XPON_PMA_BASE+0x688)
#define XPON_PMA_SS_DA_XPON_PWDB_0                    (XPON_PMA_BASE+0x68C)
#define XPON_PMA_SS_DA_XPON_PWDB_1                    (XPON_PMA_BASE+0x690)
#define XPON_PMA_SS_LCPLL_0                           (XPON_PMA_BASE+0x694)
#define XPON_PMA_SS_LCPLL_1                           (XPON_PMA_BASE+0x698)
#define XPON_PMA_SS_LCPLL_2                           (XPON_PMA_BASE+0x69C)
#define XPON_PMA_ADD_DIG_RESERVE_0                    (XPON_PMA_BASE+0x700)
#define XPON_PMA_ADD_DIG_RESERVE_1                    (XPON_PMA_BASE+0x704)
#define XPON_PMA_ADD_DIG_RESERVE_2                    (XPON_PMA_BASE+0x708)
#define XPON_PMA_ADD_DIG_RESERVE_3                    (XPON_PMA_BASE+0x70C)
#define XPON_PMA_ADD_DIG_RESERVE_4                    (XPON_PMA_BASE+0x710)
#define XPON_PMA_RG_XPON_RX_RESERVED_1                (XPON_PMA_BASE+0x714)
#define XPON_PMA_ADD_DIG_RO_RESERVE_0                 (XPON_PMA_BASE+0x718)
#define XPON_PMA_ADD_DIG_RO_RESERVE_1                 (XPON_PMA_BASE+0x71C)
#define XPON_PMA_ADD_DIG_RO_RESERVE_2                 (XPON_PMA_BASE+0x720)
#define XPON_PMA_ADD_DIG_RO_RESERVE_3                 (XPON_PMA_BASE+0x724)
#define XPON_PMA_ADD_DIG_RO_RESERVE_4                 (XPON_PMA_BASE+0x728)
#define XPON_PMA_ADD_RX_SYS_EN_SEL_0                  (XPON_PMA_BASE+0x72C)
#define XPON_PMA_PLL_TDC_FREQDET_0                    (XPON_PMA_BASE+0x730)
#define XPON_PMA_PLL_TDC_FREQDET_1                    (XPON_PMA_BASE+0x734)
#define XPON_PMA_PLL_TDC_FREQDET_2                    (XPON_PMA_BASE+0x738)
#define XPON_PMA_PLL_TDC_FREQDET_3                    (XPON_PMA_BASE+0x73C)
#define XPON_PMA_DA_XPON_TX_FORCE_0                   (XPON_PMA_BASE+0x740)
#define XPON_PMA_DA_XPON_TX_FORCE_1                   (XPON_PMA_BASE+0x744)
#define XPON_PMA_DA_XPON_TX_FORCE_2                   (XPON_PMA_BASE+0x748)
#define XPON_PMA_RX_FORCE_MODE_10                     (XPON_PMA_BASE+0x74C)
#define XPON_PMA_ADD_CLKPATH_RST_0                    (XPON_PMA_BASE+0x750)
#define XPON_PMA_ADD_XPON_MODE_1                      (XPON_PMA_BASE+0x754)
#define XPON_PMA_ADD_R2T_MODE_1                       (XPON_PMA_BASE+0x758)
#define XPON_PMA_ADD_T2R_MODE_1                       (XPON_PMA_BASE+0x75C)
#define XPON_PMA_ADD_LCPLL_RO_1                       (XPON_PMA_BASE+0x760)
#define XPON_PMA_ADD_RO_RX2ANA_1                      (XPON_PMA_BASE+0x764)
#define XPON_PMA_ADD_RO_RX2ANA_2                      (XPON_PMA_BASE+0x768)
#define XPON_PMA_ADD_RO_RX2ANA_3                      (XPON_PMA_BASE+0x76C)
#define XPON_PMA_ADD_RO_R2TMODE_1                     (XPON_PMA_BASE+0x780)
#define XPON_PMA_RG_LCPLL_XPON_CMN_STB_CTRL_0         (XPON_PMA_BASE+0x784)
#define XPON_PMA_RG_LCPLL_XPON_CMN_STB_CTRL_1         (XPON_PMA_BASE+0x788)
#define XPON_PMA_RG_TX_HSDATA_EN_EXT_CNT_0            (XPON_PMA_BASE+0x78C)
#define XPON_PMA_PON_TX_COUNTER_0                     (XPON_PMA_BASE+0x790)
#define XPON_PMA_PON_TX_COUNTER_1                     (XPON_PMA_BASE+0x794)
#define XPON_PMA_PON_TX_COUNTER_2                     (XPON_PMA_BASE+0x798)
#define XPON_PMA_PON_TX_COUNTER_3                     (XPON_PMA_BASE+0x79C)
#define XPON_PMA_PON_CK_SET                           (XPON_PMA_BASE+0x7A0)
#define XPON_PMA_TX_FIFO_MODE_SEL                     (XPON_PMA_BASE+0x7A4)
#define XPON_PMA_XPON_PLL_STB_CNT                     (XPON_PMA_BASE+0x7A8)
#define XPON_PMA_XPON_PLL_STOP_CNT                    (XPON_PMA_BASE+0x7AC)
#define XPON_PMA_SW_RST_SET                           (XPON_PMA_BASE+0x7B0)
#define XPON_PMA_ADD_RO_TX2ANA_1                      (XPON_PMA_BASE+0x7B4)
#define XPON_PMA_TX_DLY_CTRL                          (XPON_PMA_BASE+0x7B8)
#define XPON_PMA_MEM_WRAPPER_CTRL                     (XPON_PMA_BASE+0x7BC)
#define XPON_PMA_XPON_INT_EN_2                        (XPON_PMA_BASE+0x7C0)
#define XPON_PMA_XPON_INT_EN_3                        (XPON_PMA_BASE+0x7C4)
#define XPON_PMA_XPON_INT_STA_2                       (XPON_PMA_BASE+0x7C8)
#define XPON_PMA_XPON_INT_STA_3                       (XPON_PMA_BASE+0x7CC)
#define XPON_PMA_XPON_SD_BEHAVIOR_SETTING_0           (XPON_PMA_BASE+0x7D0)
#define XPON_PMA_XPON_SD_BEHAVIOR_SETTING_1           (XPON_PMA_BASE+0x7D4)
#define XPON_PMA_XPON_SD_BEHAVIOR_STA                 (XPON_PMA_BASE+0x7D8)
#define XPON_PMA_RX_EXTRAL_CTRL                       (XPON_PMA_BASE+0x7DC)
#define XPON_PMA_RX_LEQ_DISB_CTRL0                    (XPON_PMA_BASE+0x7E0)
#define XPON_PMA_RX_LEQ_DISB_CTRL1                    (XPON_PMA_BASE+0x7E4)
#define XPON_PMA_RX_LEQ_FORCE_CTRL0                   (XPON_PMA_BASE+0x7E8)
#define XPON_PMA_RX_LEQ_FORCE_CTRL1                   (XPON_PMA_BASE+0x7EC)
#define XPON_PMA_RX_LEQ_ENTIME_CTRL0                  (XPON_PMA_BASE+0x7F0)
#define XPON_PMA_RX_LEQ_ENTIME_CTRL1                  (XPON_PMA_BASE+0x7F4)
#define XPON_PMA_RX_LEQ_ENTIME_CTRL2                  (XPON_PMA_BASE+0x7F8)
#define XPON_PMA_RX_LEQ_CTRL0                         (XPON_PMA_BASE+0x7FC)
#define XPON_PMA_RX_LEQ_CTRL1                         (XPON_PMA_BASE+0x800)
#define XPON_PMA_RX_LEQ_CTRL2                         (XPON_PMA_BASE+0x804)
#define XPON_PMA_RX_LEQ_RO0                           (XPON_PMA_BASE+0x808)
#define XPON_PMA_RX_LEQ_RO1                           (XPON_PMA_BASE+0x80C)
#define XPON_PMA_PLL_FT_FREQDET_0                     (XPON_PMA_BASE+0x810)
#define XPON_PMA_PLL_FT_FREQDET_1                     (XPON_PMA_BASE+0x814)
#define XPON_PMA_PLL_FT_FREQDET_2                     (XPON_PMA_BASE+0x818)
#define XPON_PMA_PLL_FT_FREQDET_3                     (XPON_PMA_BASE+0x81C)
#define XPON_PMA_RO_RX_FREQDET                        (XPON_PMA_BASE+0x820)
#define XPON_PMA_RO_PLL_FREQDET                       (XPON_PMA_BASE+0x824)
#define XPON_PMA_RO_PLL_FT_FREQDET                    (XPON_PMA_BASE+0x828)
#define XPON_PMA_RO_PMA_FREQDET                       (XPON_PMA_BASE+0x82C)
#define XPON_PMA_RG_PMA_FREQDET                       (XPON_PMA_BASE+0x830)
#define XPON_PMA_RG_EXT_BEN_DATA                      (XPON_PMA_BASE+0x834)
#define XPON_PMA_RG_PRE_BEN_DATA                      (XPON_PMA_BASE+0x838)
#define XPON_PMA_RX_TORGS_DEBUG_12                    (XPON_PMA_BASE+0x83C)
#define XPON_PMA_RO_FLL_ADC_0                         (XPON_PMA_BASE+0x844)
#define XPON_PMA_RO_FLL_ADC_1                         (XPON_PMA_BASE+0x848)
#define XPON_PMA_RO_FLL_ADC_2                         (XPON_PMA_BASE+0x84C)
#define XPON_PMA_RO_FLL_ADC_3                         (XPON_PMA_BASE+0x850)
#define XPON_PMA_RO_FLL_ADC_4                         (XPON_PMA_BASE+0x854)
#define XPON_PMA_RG_AD_XPON_PLL_FT_CK_MON_MUX_SEL     (XPON_PMA_BASE+0x858)
#define XPON_PMA_XPON_INT_EN_4                        (XPON_PMA_BASE+0x85C)
#define XPON_PMA_XPON_INT_STA_4                       (XPON_PMA_BASE+0x860)
#define XPON_PMA_XPON_PMA_AFIFO_REACH_TH              (XPON_PMA_BASE+0x864)
#define XPON_PMA_XPON_MD32_PBUS_CSR_CTRL              (XPON_PMA_BASE+0x868)
#define XPON_PMA_BENOFF_BIST_SEED                     (XPON_PMA_BASE+0x86C)
#define XPON_PMA_PWR_PLL_CTRL                         (XPON_PMA_BASE+0x870)
#define XPON_PMA_XPON_TX_RATE_CTRL                    (XPON_PMA_BASE+0x874)
#define XPON_PMA_BENOFF_DATA0                         (XPON_PMA_BASE+0x878)
#define XPON_PMA_BENOFF_DATA1                         (XPON_PMA_BASE+0x87C)
#define XPON_PMA_BENOFF_DATA2                         (XPON_PMA_BASE+0x880)
#define XPON_PMA_BENOFF_DATA3                         (XPON_PMA_BASE+0x884)
#define XPON_PMA_BENOFF_CTRL                          (XPON_PMA_BASE+0x888)
#define XPON_PMA_HS_DATA_EN_SEL                       (XPON_PMA_BASE+0x88C)
#define XPON_PMA_FIFO_CK_STATUS                       (XPON_PMA_BASE+0x890)
#define XPON_PMA_TDC_DLF_GAIN_STATUS                  (XPON_PMA_BASE+0x894)
#define XPON_PMA_TDC_DLF_MODE_SETTING                 (XPON_PMA_BASE+0x898)
#define XPON_PMA_MD32_ISR_CTRL                        (XPON_PMA_BASE+0x89C)
#define XPON_PMA_MD32_MEM_CLK_CTRL                    (XPON_PMA_BASE+0x8A0)
#define XPON_PMA_MD32_MEM_CLK_CG_CTRL                 (XPON_PMA_BASE+0x8A4)
#define XPON_PMA_XG_EQD_STA                           (XPON_PMA_BASE+0x8A8)
#define XPON_PMA_XG_EQD_CTRL                          (XPON_PMA_BASE+0x8AC)
#define XPON_PMA_RAWDATA_DUMP_CTRL_0                  (XPON_PMA_BASE+0x8B0)
#define XPON_PMA_RAWDATA_DUMP_CTRL_1                  (XPON_PMA_BASE+0x8B4)
#define XPON_PMA_RAWDATA_DUMP_CTRL_2                  (XPON_PMA_BASE+0x8B8)
#define XPON_PMA_RAWDATA_DUMP_CTRL_3                  (XPON_PMA_BASE+0x8BC)
#define XPON_PMA_RAWDATA_DUMP_STA_0                   (XPON_PMA_BASE+0x8C0)
#define XPON_PMA_RAWDATA_DUMP_STA_1                   (XPON_PMA_BASE+0x8C4)
#define XPON_PMA_RAWDATA_DUMP_STA_2                   (XPON_PMA_BASE+0x8C8)
#define XPON_PMA_MD32MEM_DIV2_CNT                     (XPON_PMA_BASE+0x8CC)
#define XPON_PMA_AD_XPON_CDR_LPF_SV                   (XPON_PMA_BASE+0x8D0)
#define XPON_PMA_K_VALUE_RD                           (XPON_PMA_BASE+0x900)
#define XPON_PMA_XPONPLL_CTRL                         (XPON_PMA_BASE+0x904)


//-------------- XPON BIST-------------- 

#define XPON_PMA_BISTCTL_PRBS_EVENT_BISTCTL_PRBS_DONE 		(1<<16)
#define XPON_PMA_BISTCTL_PRBS_EVENT_BISTCTL_PRBS_FAIL 		(1<<8)
#define XPON_PMA_BISTCTL_PRBS_EVENT_BISTCTL_PRBS_COMPARE 	(1)

#define XPON_PMA_DA_XPON_TX_FORCE_1_NORMAL_MODE_MASK        (0x1000000)
#define XPON_PMA_DA_XPON_TX_FORCE_2_FORCE_MODE_MASK         (0xffff0000)

#define XPON_PMA_XPON_SETTING_1_TX_SD_INV_MASK              (0xFFFFFFFE)
#define XPON_PMA_XPON_SETTING_1_TX_SD_INV_OFFSET			 0
#define XPON_PMA_XPON_SETTING_0_TX_FAULT_INV_MASK           (0xFEFFFFFF)
#define XPON_PMA_XPON_SETTING_0_TX_FAULT_INV_OFFSET			 24
#define XPON_PMA_XPON_SETTING_0_BURST_EN_INV_MASK           (0xFFFFFEFF)
#define XPON_PMA_XPON_SETTING_0_BURST_EN_INV_OFFSET			 8
#define XPON_PMA_XPON_SETTING_0_RX_SD_INV                   (1<<16)
//this bit is not used by PCS #define XPON_PMA_XPON_SETTING_0_RX_SD_INV_MASK              (0xFFFEFFFF)
//this bit is not used by PCS #define XPON_PMA_XPON_SETTING_0_RX_SD_INV_OFFSET			 16

#define XPON_PMA_TX_DLY_CTRL_BIT_DELAY_MASK                 (0xFFFEFF80) 

#define XPON_PMA_BISTCTL_CONTROL_PAT_SEL_MASK               (0xFFFFFFE0) 
#define XPON_PMA_BISTCTL_CONTROL_PAT_TX_EN_MASK             (0xFFFFFEFF) 

#define XPON_PMA_ADD_XPON_MODE_1_R2T_MODE_MASK              (0xFFFFFEFF) 
#define XPON_PMA_ADD_XPON_MODE_1_TX_BIST_MASK               (0xFFFEFFFF) 

#define XPON_PMA_MD32_MEM_CLK_CTRL_CLK_SEL_MASK             (0xFFFFFFFC) 
#define XPON_PMA_TX_DLY_CTRL_OUTBEN_DATA_MASK               (0x8FFFFFFF)

//-------------- XPON RX -------------- 

//RX SIGDET
#define XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0_SIGDET_VTH_SEL(i)		((UINT32)((i)&0x1f)<<27)	
#define XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0_FE_50OHMS_SEL(i)		((UINT32)((i)&0x3)<<25)
#define XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0_FE_EQ_HZEN				(1<<24)
#define XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0_FE_VB_EQ1_EN			(1<<23)
#define XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0_FE_VB_EQ2_EN			(1<<22)
#define XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0_FE_VB_EQ3_EN			(1<<21)
#define XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0_FE_VCM_GEN_PWDB		(1<<20)
#define XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0_FE_VCM_SEL(i)			((UINT32)((i)&0x7)<<17)

//RX CDR PR Reserve
#define XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0_TDC_REF_SEL(i)			((UINT32)((i)&0x3)<<22) 
#define XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0_COR_HBW_EN				(1<<21)
#define XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0_LDO_FORCE_ON				(1<<20)
#define XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0_CKREF_DIV1(i)			((UINT32)((i)&0x3)<<18)
#define XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0_MONPR_EN					(1<<17)
#define XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0_MONPI_EN					(1<<16)
#define XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0_XFICK_EN					(1<<15)
#define XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0_RX_DAC_MON(i)			((UINT32)((i)&0x1f)<<10)
#define XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0_RX_DAC_RANG(i)			((UINT32)((i)&0x3)<<8)
#define XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0_RX_DAC_RANGE_EYE(i)		((UINT32)((i)&0x3)<<6)
#define XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0_RX_SIGDET_DCTEST_EN		(1<<5)
#define XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0_RX_SIGDET_LPF_CTRL(i)	((UINT32)((i)&0x3)<<3)
#define XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0_RX_SIGDET_NOVTH			(1<<2)
#define XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0_RX_SIGDET_PEAK(i)		((UINT32)((i)&0x3))



//RX CDR PWDB
#define XPON_PMA_SS_DA_XPON_PWDB_0_CDR_PD_PWDB						(1<<24)
#define XPON_PMA_SS_DA_XPON_PWDB_0_CDR_PR_PIEYE_PWDB				(1<<16)
#define XPON_PMA_SS_DA_XPON_PWDB_0_CDR_PR_PWDB						(1<<8)
#define XPON_PMA_SS_DA_XPON_PWDB_0_RX_FE_PWDB						(1)


#define XPON_PMA_SS_DA_XPON_PWDB_1_RX_SIGDET_PWDB					(1)


//RX force mode switch
#define XPON_PMA_RX_DISB_MODE_0_DA_XPON_CDR_LPF_RSTB_NORMAL_MODE		(1<<24)
#define XPON_PMA_RX_DISB_MODE_0_DA_XPON_CDR_LPF_RSTB_FORCE_MODE			(0<<24)

#define XPON_PMA_RX_DISB_MODE_0_DA_XPON_CDR_LPF_LCK2DATA_NORMAL_MODE	(1<<16)
#define XPON_PMA_RX_DISB_MODE_0_DA_XPON_CDR_LPF_LCK2DATA_FORCE_MODE		(0<<16)

#define XPON_PMA_RX_DISB_MODE_0_DA_XPON_RX_FE_PEAKING_CTRL_NORMAL_MODE	(1<<8)
#define XPON_PMA_RX_DISB_MODE_0_DA_XPON_RX_FE_PEAKING_CTRL_FORCE_MODE	(0<<8)

#define XPON_PMA_RX_DISB_MODE_0_DA_XPON_RX_FE_GAIN_CTRL_NORMAL_MODE		(1)
#define XPON_PMA_RX_DISB_MODE_0_DA_XPON_RX_FE_GAIN_CTRL_FORCE_MODE		(0)


//		RX force mode set
#define XPON_PMA_RX_FORCE_MODE_0_DA_XPON_CDR_LPF_RSTB				(1<<24)
#define XPON_PMA_RX_FORCE_MODE_0_DA_XPON_CDR_LPF_LCK2DATA			(1<<16)
#define XPON_PMA_RX_FORCE_MODE_0_DA_XPON_CDR_LPF_LCK2REF			(0<<16)
#define XPON_PMA_RX_FORCE_MODE_0_DA_XPON_RX_FE_PEAKING_CTRL(i)		((UINT32)((i)&0x7)<<8)
#define XPON_PMA_RX_FORCE_MODE_0_DA_XPON_RX_FE_GAIN_CTRL(i)			((UINT32)((i)&0x3))


//RX force mode switch
#define XPON_PMA_RX_DISB_MODE_1_DA_XPON_RX_DAC_E0_NORMAL_MODE		(1<<24)
#define XPON_PMA_RX_DISB_MODE_1_DA_XPON_RX_DAC_E0_FORCE_MODE		(0<<24)

#define XPON_PMA_RX_DISB_MODE_1_DA_XPON_RX_DAC_D1_NORMAL_MODE		(1<<16)
#define XPON_PMA_RX_DISB_MODE_1_DA_XPON_RX_DAC_D1_FORCE_MODE		(0<<16)

#define XPON_PMA_RX_DISB_MODE_1_DA_XPON_RX_DAC_D0_NORMAL_MODE		(1<<8)
#define XPON_PMA_RX_DISB_MODE_1_DA_XPON_RX_DAC_D0_FORCE_MODE		(0<<8)

#define XPON_PMA_RX_DISB_MODE_1_DA_XPON_RX_SIGDET_OS_NORMAL_MODE	(1)
#define XPON_PMA_RX_DISB_MODE_1_DA_XPON_RX_SIGDET_OS_FORCE_MODE		(0)


//		RX force mode set
#define XPON_PMA_RX_FORCE_MODE_1_DA_XPON_RX_DAC_E0(i)				((UINT32)((i)&0x7f)<<24)
#define XPON_PMA_RX_FORCE_MODE_1_DA_XPON_RX_DAC_D1(i)				((UINT32)((i)&0x7f)<<16)
#define XPON_PMA_RX_FORCE_MODE_1_DA_XPON_RX_DAC_D0(i)				((UINT32)((i)&0x7f)<<8)
#define XPON_PMA_RX_FORCE_MODE_1_DA_XPON_RX_SIGDET_OS(i)			((UINT32)((i)&0x1f))




//RX force mode switch
#define XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1_RX_RDY_NORMAL_MODE		(1<<24)
#define XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1_RX_RDY_FORCE_MODE			(0<<24)
#define XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1_RX_BLWC_EN_NORMAL_MODE	(1<<16)
#define XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1_RX_BLWC_EN_FORCE_MODE		(0<<16)
#define XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1_RX_OS_RDY_NORMAL_MODE		(1<<8)
#define XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1_RX_OS_RDY_FORCE_MODE		(0<<8)
#define XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1_RX_SDCAL_EN_NORMAL_MODE	(1)
#define XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1_RX_SDCAL_EN_FORCE_MODE	(0)



//		RX force mode set
#define XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1_RX_RDY			(1<<24)
#define XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1_RX_RDY_OFF		(0<<24)

#define XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1_RX_BLWC_EN		(1<<16)
#define XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1_RX_OS_RDY		(1<<8)
#define XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1_RX_SDCAL_EN		(1)


//TDC filter

#define XPON_PMA_SS_LCPLL_TDC_FLT_1_TDC_GAIN(i)			((UINT32)((i)&0x7ff)<<16)
#define XPON_PMA_SS_LCPLL_TDC_FLT_1_A_TDC(i)			((UINT32)((i)&0xf)<<8)
#define XPON_PMA_SS_LCPLL_TDC_FLT_1_GPON_SEL			(1)


#define XPON_PMA_SS_LCPLL_TDC_FLT_3_TDC_SWRST			(1<<24)
#define XPON_PMA_SS_LCPLL_TDC_FLT_3_NCPO_HOLD			(1<<16)
#define XPON_PMA_SS_LCPLL_TDC_FLT_3_NCPO_LOAD			(1<<8)
#define XPON_PMA_SS_LCPLL_TDC_FLT_3_NCPO_SHIFT(i)		((UINT32)((i)&0x3))


//CLK Path
#define XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0_ACBUF_EN			(1<<31)
#define XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0_DFM_DA(i)			((UINT32)((i)&0xff)<<23)
#define XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0_LDO_EN				(1<<22)
#define XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0_LDO_VOD_EN			(1<<21)
#define XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0_TDC_AUTOEN			(1<<20)
#define XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0_TDC_FT_CK_EN		(1<<19)
#define XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0_TDC_MON_RXCK_EN		(1<<18)
#define XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0_TDC_MON_TXCK_EN		(1<<17)
#define XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0_TDC_SYNC_CK_SEL		(1<<16)



//TDC

#define XPON_PMA_SS_LCPLL_TDC_PW_0_DPMA_DDS1_ISO_DISABLE	(1<<24)
#define XPON_PMA_SS_LCPLL_TDC_PW_0_DPMA_DDS1_PWDB			(1<<16)
#define XPON_PMA_SS_LCPLL_TDC_PW_0_DPMA_AUTOPW_EN			(1<<8)
#define XPON_PMA_SS_LCPLL_TDC_PW_0_TDC_DIG_PWDB				(1)


#define XPON_PMA_SS_LCPLL_TDC_PW_4_SIGDET_CG			(1<<24)
#define XPON_PMA_SS_LCPLL_TDC_PW_4_EN					(1<<16)
#define XPON_PMA_SS_LCPLL_TDC_PW_4_CAL_OFFSET			(1<<8)
#define XPON_PMA_SS_LCPLL_TDC_PW_4_CAL_BW				(1)


#define XPON_PMA_SS_LCPLL_TDC_PW_5_SYNC_IN_MODE		(1<<24)
#define XPON_PMA_SS_LCPLL_TDC_PW_5_AUTOK_TDC		(1<<16)
#define XPON_PMA_SS_LCPLL_TDC_PW_5_AUTOEN			(1<<8)
#define XPON_PMA_SS_LCPLL_TDC_PW_5_EN_MUX			(1)




//-------------- XPON TX -------------- 

//XPON BENTX BIAS EN
#define XPON_PMA_RG_XPON_BENTX_BIAS_EN_0_BIAS_EN		(1<<31)
#define XPON_PMA_RG_XPON_BENTX_BIAS_EN_0_DRV_EN			(1<<30)
#define XPON_PMA_RG_XPON_BENTX_BIAS_EN_0_DRV_IBIAS(i)	((UINT32)((i)&0x3f)<<24)
#define XPON_PMA_RG_XPON_BENTX_BIAS_EN_0_DRV_IMP(i)		((UINT32)((i)&0x3f)<<18)
#define XPON_PMA_RG_XPON_BENTX_BIAS_EN_0_IMP_EN			(1<<17)
#define XPON_PMA_RG_XPON_BENTX_BIAS_EN_0_PRED_EN		(1<<16)
#define XPON_PMA_RG_XPON_BENTX_BIAS_EN_0_PRED_IBIAS(i)	((UINT32)((i)&0xf)<<12)
#define XPON_PMA_RG_XPON_BENTX_BIAS_EN_0_PRED_IMP_SMALL	(1<<11)


//XPON TX en
#define XPON_PMA_RG_XPON_TX_EN_0_TX_EN 				(1<<31)
#define XPON_PMA_RG_XPON_TX_EN_0_SER_LOADSEL_T9		(0<<29)
#define XPON_PMA_RG_XPON_TX_EN_0_SER_LOADSEL_T8		(1<<29)
#define XPON_PMA_RG_XPON_TX_EN_0_SER_LOADSEL_T7		(2<<29)
#define XPON_PMA_RG_XPON_TX_EN_0_SER_LOADSEL_T6		(3<<29)
#define XPON_PMA_RG_XPON_TX_EN_0_DIGFF_CK_INV		(1<<28)
#define XPON_PMA_RG_XPON_TX_EN_0_FIR_CN1(i)			((UINT32)((i)&0xf)<<24)
#define XPON_PMA_RG_XPON_TX_EN_0_FIR_INV_C0(i)		((UINT32)((i)&0x1f)<<19)
#define XPON_PMA_RG_XPON_TX_EN_0_FIR_C1(i)			((UINT32)((i)&0x1f)<<14)
#define XPON_PMA_RG_XPON_TX_EN_0_FIR_C2(i)			((UINT32)((i)&0x7)<<11)
#define XPON_PMA_RG_XPON_TX_EN_0_FORCE_CX			(1<<10)
#define XPON_PMA_RG_XPON_TX_EN_0_SR_CTRL(i)			((UINT32)((i)&0xf)<<6)
#define XPON_PMA_RG_XPON_TX_EN_0_LPEIDLE_EN 		(1<<5)
#define XPON_PMA_RG_XPON_TX_EN_0_CKMON_CDR5G_EN		(1<<4)
#define XPON_PMA_RG_XPON_TX_EN_0_CKMON_EN			(1<<3)
#define XPON_PMA_RG_XPON_TX_EN_0_CKMON_SEL(i)		((i)&0x7)


//XPON TX LSDATA_EN 
#define XPON_PMA_RG_XPON_TX_LSDATA_EN_0_LSDATA_EN		(1<<31)
#define XPON_PMA_RG_XPON_TX_LSDATA_EN_0_LSDATA			(1<<30)
#define XPON_PMA_RG_XPON_TX_LSDATA_EN_0_FIR_CN1_INV		(1<<29)
#define XPON_PMA_RG_XPON_TX_LSDATA_EN_0_FIR_C0_INV		(1<<28)
#define XPON_PMA_RG_XPON_TX_LSDATA_EN_0_FIR_C1_INV		(1<<27)
#define XPON_PMA_RG_XPON_TX_LSDATA_EN_0_FIR_C2_INV		(1<<26)
#define XPON_PMA_RG_XPON_TX_LSDATA_EN_0_MPX_OP_EN		(1<<25)
#define XPON_PMA_RG_XPON_TX_LSDATA_EN_0_MPX_OP_BYPASS	(1<<24)
#define XPON_PMA_RG_XPON_TX_LSDATA_EN_0_MPX_SEL(i)		((UINT32)((i)&0x1f)<<19)
#define XPON_PMA_RG_XPON_TX_LSDATA_EN_0_TXLBRX_EN		(1<<17)


//XPON TX RESERVED
#define XPON_PMA_RG_XPON_TX_RESERVED_0_CKLDO_VOD_EN			(1<15)
#define XPON_PMA_RG_XPON_TX_RESERVED_0_FIRLDO_VOD_EN		(1<14) 
#define XPON_PMA_RG_XPON_TX_RESERVED_0_INT_LPBK_EN			(1<13)
#define XPON_PMA_RG_XPON_TX_RESERVED_0_CKSEL(i)				((UINT32)((i)&0x7)<<10)
#define XPON_PMA_RG_XPON_TX_RESERVED_0_CALI_VREF_H(i)		((UINT32)((i)&0x3)<<8)
#define XPON_PMA_RG_XPON_TX_RESERVED_0_CALI_VREF_L(i)		((UINT32)((i)&0x3)<<6)
#define XPON_PMA_RG_XPON_TX_RESERVED_0_TX_RATE_CTRL_8bit	(0<<4)
#define XPON_PMA_RG_XPON_TX_RESERVED_0_TX_RATE_CTRL_10bit	(1<<4)
#define XPON_PMA_RG_XPON_TX_RESERVED_0_TX_RATE_CTRL_16bit	(2<<4)

//XPON_PMA_SS_TX_RST_B
#define XPON_PMA_SS_TX_RST_B_CALIB_RST_B					(1<<8)
#define XPON_PMA_SS_TX_RST_B_TOP_RST_B						(1)


//XPON_PMA_SS_TX_CALIB_0
#define XPON_PMA_SS_TX_CALIB_0_CALIB_P_EN					(1)


//XPON_PMA_SS_TX_CALIB_1
#define XPON_PMA_SS_TX_CALIB_1_CALIB_N_EN					(1)

//XPON_PMA_ADD_RO_RX2ANA_3
#define XPON_PMA_ADD_RO_RX2ANA_3_FIFO_FULL_CNT_MASK         (0xf)
#define XPON_PMA_ADD_RO_RX2ANA_3_FIFO_FULL_CNT_OFFSET       (16)

#define XPON_PMA_ADD_RO_RX2ANA_3_FIFO_EMPTY_CNT_MASK        (0xf)
#define XPON_PMA_ADD_RO_RX2ANA_3_FIFO_EMPTY_CNT_OFFSET      (8)

#define XPON_PMA_ADD_RO_RX2ANA_3_PI_CAL_DATA_OUT_MASK       (0x7f)
#define XPON_PMA_ADD_RO_RX2ANA_3_PI_CAL_DATA_OUT_OFFSET     (0)

#define XPON_PMA_ADD_LCPLL_RO_1_RO_PLL_FBCK_LOCK2_MASK		(0x10000)


#define XPON_FIR_DEFAULT_VALUE_LOWRATE				(0x0)		// XPON_PMA_ADD_RG_XPON_TX_EN_0[27:11] for 1.25G & 1.24416G & 2.48816G
#define XPON_FIR_DEFAULT_VALUE_HIGHRATE				(0x234b)	// XPON_PMA_ADD_RG_XPON_TX_EN_0[27:11] for 10.3125G & 9.95328G
#define XPON_FIR_DEFAULT_VALUE_NULL					(0xFFFFFFFF)	// in case of flash without fir settings

// pma_optimize_value
#define XPON_PMA_OPT_DEFAULT_VALUE_LOW_RATE			(0xc0000420)
#define XPON_PMA_OPT_DEFAULT_VALUE_HIGH_RATE		(0xC11A5C20)

// pma_interrupt

#define XPON_PMA_TRANS_ROGUE_ONU_INT_EN				(1)
#define XPON_PMA_TRANS_ROGUE_ONU_INT				(1)


/*   =================  EN7580 XFI PMA REG  =================   */

                                                 
#define XFI_PMA_RG_XFI_XTALCK_CGM_EN_0                 (XFI_PMA_BASE+0x000)
#define XFI_PMA_RG_JCPLL_IC_0                          (XFI_PMA_BASE+0x004)
#define XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0                 (XFI_PMA_BASE+0x008)
#define XFI_PMA_RG_JCPLL_SDM_ORD_0                     (XFI_PMA_BASE+0x00C)
#define XFI_PMA_RG_JCPLL_RESERVE0_0                    (XFI_PMA_BASE+0x010)
#define XFI_PMA_RGS_JCPLL_AUTOK_BAND_0                 (XFI_PMA_BASE+0x014)
#define XFI_PMA_RG_XFI_CMN_CUREN_0                     (XFI_PMA_BASE+0x018)
#define XFI_PMA_RG_XFI_CLKPATH_LDO_EN_0                (XFI_PMA_BASE+0x01C)
#define XFI_PMA_RG_XFI_TOP_REV_0_0                     (XFI_PMA_BASE+0x020)
#define XFI_PMA_RG_XFI_RX_MPXSEL_0                     (XFI_PMA_BASE+0x024)
#define XFI_PMA_RG_XFI_RX_REV_0_0                      (XFI_PMA_BASE+0x028)
#define XFI_PMA_RG_XFI_RX_PHYCK_DIV_0                  (XFI_PMA_BASE+0x02C)
#define XFI_PMA_RG_XFI_CDR_LPF_KI_GAIN_0               (XFI_PMA_BASE+0x030)
#define XFI_PMA_RG_XFI_CDR_LPF_TOP_LIM_0               (XFI_PMA_BASE+0x034)
#define XFI_PMA_RG_XFI_CDR_LPF_SETVALUE_0              (XFI_PMA_BASE+0x038)
#define XFI_PMA_RG_XFI_CDR_PR_INJ_FORCE_OFF_0          (XFI_PMA_BASE+0x03C)
#define XFI_PMA_RG_XFI_CDR_PR_CKREF_DIV_0              (XFI_PMA_BASE+0x040)
#define XFI_PMA_RG_XFI_CDR_PR_RESERVE0_0               (XFI_PMA_BASE+0x044)
#define XFI_PMA_RG_XFI_RX_DAC_RANGE_EYE_0              (XFI_PMA_BASE+0x048)
#define XFI_PMA_RG_XFI_TXPLL1_IPATH_LOAD_KBAND_EN_0    (XFI_PMA_BASE+0x04C)
#define XFI_PMA_RG_XFI_TXPLL1_BR_0                     (XFI_PMA_BASE+0x050)
#define XFI_PMA_RG_XFI_TXPLL1_DAC_BAND_0               (XFI_PMA_BASE+0x054)
#define XFI_PMA_RG_XFI_TXPLL1_DDS_MONEN_0              (XFI_PMA_BASE+0x058)
#define XFI_PMA_RG_XFI_TXPLL1_DDS_SSC_DELTA_0          (XFI_PMA_BASE+0x05C)
#define XFI_PMA_RG_XFI_TXPLL1_DDS_SSC_EN_0             (XFI_PMA_BASE+0x060)
#define XFI_PMA_RGS_XFI_TXPLL1_SDM_TD_0                (XFI_PMA_BASE+0x064)
#define XFI_PMA_RG_XFI_TX_EN_0                         (XFI_PMA_BASE+0x068)
#define XFI_PMA_RG_XFI_TX_LSDATA_EN_0                  (XFI_PMA_BASE+0x06C)
#define XFI_PMA_RG_XFI_TX_RESERVED_0                   (XFI_PMA_BASE+0x070)
#define XFI_PMA_SS_TX_DA_1                             (XFI_PMA_BASE+0x170)
#define XFI_PMA_SS_LCPLL_PWCTL_SETTING_0               (XFI_PMA_BASE+0x200)
#define XFI_PMA_SS_LCPLL_PWCTL_SETTING_1               (XFI_PMA_BASE+0x204)
#define XFI_PMA_SS_LCPLL_PWCTL_SETTING_2               (XFI_PMA_BASE+0x208)
#define XFI_PMA_SS_LCPLL_PWCTL_DBG_SETTING             (XFI_PMA_BASE+0x20C)
#define XFI_PMA_SS_LCPLL_TDC_PW_0                      (XFI_PMA_BASE+0x210)
#define XFI_PMA_SS_LCPLL_TDC_PW_1                      (XFI_PMA_BASE+0x214)
#define XFI_PMA_SS_LCPLL_TDC_PW_2                      (XFI_PMA_BASE+0x218)
#define XFI_PMA_SS_LCPLL_TDC_PW_3                      (XFI_PMA_BASE+0x21C)
#define XFI_PMA_SS_LCPLL_TDC_PW_4                      (XFI_PMA_BASE+0x220)
#define XFI_PMA_SS_LCPLL_TDC_PW_5                      (XFI_PMA_BASE+0x224)
#define XFI_PMA_SS_LCPLL_TDC_FLT_0                     (XFI_PMA_BASE+0x228)
#define XFI_PMA_SS_LCPLL_TDC_FLT_1                     (XFI_PMA_BASE+0x22C)
#define XFI_PMA_SS_LCPLL_TDC_FLT_2                     (XFI_PMA_BASE+0x230)
#define XFI_PMA_SS_LCPLL_TDC_FLT_3                     (XFI_PMA_BASE+0x234)
#define XFI_PMA_SS_LCPLL_TDC_FLT_4                     (XFI_PMA_BASE+0x238)
#define XFI_PMA_SS_LCPLL_TDC_FLT_5                     (XFI_PMA_BASE+0x23C)
#define XFI_PMA_SS_LCPLL_TDC_FLT_6                     (XFI_PMA_BASE+0x240)
#define XFI_PMA_SS_LCPLL_TDC_FLT_7                     (XFI_PMA_BASE+0x244)
#define XFI_PMA_SS_LCPLL_TDC_PCW_1                     (XFI_PMA_BASE+0x248)
#define XFI_PMA_SS_LCPLL_TDC_PCW_2                     (XFI_PMA_BASE+0x24C)
#define XFI_PMA_SS_LCPLL_TDC_RO_1                      (XFI_PMA_BASE+0x250)
#define XFI_PMA_SS_LCPLL_TDC_RO_2                      (XFI_PMA_BASE+0x254)
#define XFI_PMA_SS_LCPLL_TDC_RO_3                      (XFI_PMA_BASE+0x258)
#define XFI_PMA_SS_LCPLL_TDC_RO_4                      (XFI_PMA_BASE+0x25C)
#define XFI_PMA_SS_LCPLL_TDC_RO_5                      (XFI_PMA_BASE+0x260)
#define XFI_PMA_SS_LCPLL_TDC_CTRL_0                    (XFI_PMA_BASE+0x264)
#define XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0             (XFI_PMA_BASE+0x300)
#define XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1             (XFI_PMA_BASE+0x304)
#define XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2             (XFI_PMA_BASE+0x308)
#define XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3             (XFI_PMA_BASE+0x30C)
#define XFI_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0           (XFI_PMA_BASE+0x320)
#define XFI_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1           (XFI_PMA_BASE+0x324)
#define XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_0               (XFI_PMA_BASE+0x330)
#define XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1               (XFI_PMA_BASE+0x334)
#define XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_2               (XFI_PMA_BASE+0x338)
#define XFI_PMA_RX_CTRL_SEQUENCE_CTRL_0                (XFI_PMA_BASE+0x340)
#define XFI_PMA_RX_CTRL_SEQUENCE_CTRL_1                (XFI_PMA_BASE+0x344)
#define XFI_PMA_RX_CTRL_SEQUENCE_CTRL_2                (XFI_PMA_BASE+0x348)
#define XFI_PMA_RX_CTRL_SEQUENCE_CTRL_3                (XFI_PMA_BASE+0x34C)
#define XFI_PMA_RX_CTRL_SEQUENCE_CTRL_4                (XFI_PMA_BASE+0x350)
#define XFI_PMA_RX_CTRL_SEQUENCE_CTRL_5                (XFI_PMA_BASE+0x354)
#define XFI_PMA_RX_CTRL_SEQUENCE_CTRL_6                (XFI_PMA_BASE+0x358)
#define XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0           (XFI_PMA_BASE+0x35C)
#define XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1           (XFI_PMA_BASE+0x360)
#define XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0          (XFI_PMA_BASE+0x364)
#define XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1          (XFI_PMA_BASE+0x368)
#define XFI_PMA_PHY_EQ_CTRL_0                          (XFI_PMA_BASE+0x370)
#define XFI_PMA_PHY_EQ_CTRL_1                          (XFI_PMA_BASE+0x374)
#define XFI_PMA_PHY_EQ_CTRL_2                          (XFI_PMA_BASE+0x378)
#define XFI_PMA_PHY_EQ_CTRL_3                          (XFI_PMA_BASE+0x37C)
#define XFI_PMA_PHY_EQ_CTRL_4                          (XFI_PMA_BASE+0x380)
#define XFI_PMA_PHY_EQ_CTRL_5                          (XFI_PMA_BASE+0x384)
#define XFI_PMA_PHY_EQ_CTRL_6                          (XFI_PMA_BASE+0x388)
#define XFI_PMA_PHY_EQ_CTRL_7                          (XFI_PMA_BASE+0x38C)
#define XFI_PMA_PHY_EQ_CTRL_8                          (XFI_PMA_BASE+0x390)
#define XFI_PMA_PHY_EQ_CTRL_9                          (XFI_PMA_BASE+0x394)
#define XFI_PMA_PHY_EQ_CTRL_10                         (XFI_PMA_BASE+0x398)
#define XFI_PMA_SS_RX_FEOS                             (XFI_PMA_BASE+0x400)
#define XFI_PMA_SS_RX_BLWC                             (XFI_PMA_BASE+0x410)
#define XFI_PMA_SS_RX_FREQ_DET_1                       (XFI_PMA_BASE+0x420)
#define XFI_PMA_SS_RX_FREQ_DET_2                       (XFI_PMA_BASE+0x424)
#define XFI_PMA_SS_RX_FREQ_DET_3                       (XFI_PMA_BASE+0x428)
#define XFI_PMA_SS_RX_FREQ_DET_4                       (XFI_PMA_BASE+0x42C)
#define XFI_PMA_SS_RX_PI_CAL                           (XFI_PMA_BASE+0x430)
#define XFI_PMA_SS_RX_CAL_1                            (XFI_PMA_BASE+0x440)
#define XFI_PMA_SS_RX_CAL_2                            (XFI_PMA_BASE+0x444)
#define XFI_PMA_SS_RX_SIGDET_0                         (XFI_PMA_BASE+0x450)
#define XFI_PMA_SS_RX_SIGDET_1                         (XFI_PMA_BASE+0x454)
#define XFI_PMA_SS_RX_FLL_0                            (XFI_PMA_BASE+0x460)
#define XFI_PMA_SS_RX_FLL_1                            (XFI_PMA_BASE+0x464)
#define XFI_PMA_SS_RX_FLL_2                            (XFI_PMA_BASE+0x468)
#define XFI_PMA_SS_RX_FLL_3                            (XFI_PMA_BASE+0x46C)
#define XFI_PMA_SS_RX_FLL_4                            (XFI_PMA_BASE+0x470)
#define XFI_PMA_SS_RX_FLL_5                            (XFI_PMA_BASE+0x474)
#define XFI_PMA_SS_RX_FLL_6                            (XFI_PMA_BASE+0x478)
#define XFI_PMA_SS_RX_FLL_7                            (XFI_PMA_BASE+0x47C)
#define XFI_PMA_SS_RX_FLL_8                            (XFI_PMA_BASE+0x480)
#define XFI_PMA_SS_RX_FLL_9                            (XFI_PMA_BASE+0x484)
#define XFI_PMA_SS_RX_FLL_a                            (XFI_PMA_BASE+0x488)
#define XFI_PMA_SS_RX_FLL_b                            (XFI_PMA_BASE+0x48C)
#define XFI_PMA_RX_PDOS_CTRL_0                         (XFI_PMA_BASE+0x490)
#define XFI_PMA_RX_RESET_0                             (XFI_PMA_BASE+0x494)
#define XFI_PMA_RX_RESET_1                             (XFI_PMA_BASE+0x498)
#define XFI_PMA_RX_DEBUG_0                             (XFI_PMA_BASE+0x49C)
#define XFI_PMA_BISTCTL_CONTROL                        (XFI_PMA_BASE+0x500)
#define XFI_PMA_BISTCTL_ALIGN_PAT                      (XFI_PMA_BASE+0x504)
#define XFI_PMA_BISTCTL_PROGRAM_PAT_0                  (XFI_PMA_BASE+0x508)
#define XFI_PMA_BISTCTL_PROGRAM_PAT_1                  (XFI_PMA_BASE+0x50C)
#define XFI_PMA_BISTCTL_POLLUTION                      (XFI_PMA_BASE+0x510)
#define XFI_PMA_BISTCTL_PRBS_INITIAL_SEED              (XFI_PMA_BASE+0x514)
#define XFI_PMA_BISTCTL_PRBS_EVENT                     (XFI_PMA_BASE+0x518)
#define XFI_PMA_BISTCTL_PRBS_ERRCNT                    (XFI_PMA_BASE+0x51C)
#define XFI_PMA_BISTCTL_PRBS_FAIL_THRESHOLD            (XFI_PMA_BASE+0x520)
#define XFI_PMA_RX_TORGS_DEBUG_0                       (XFI_PMA_BASE+0x524)
#define XFI_PMA_RX_TORGS_DEBUG_1                       (XFI_PMA_BASE+0x528)
#define XFI_PMA_RX_TORGS_DEBUG_2                       (XFI_PMA_BASE+0x52C)
#define XFI_PMA_RX_TORGS_DEBUG_3                       (XFI_PMA_BASE+0x530)
#define XFI_PMA_RX_TORGS_DEBUG_4                       (XFI_PMA_BASE+0x534)
#define XFI_PMA_RX_TORGS_DEBUG_5                       (XFI_PMA_BASE+0x538)
#define XFI_PMA_RX_TORGS_DEBUG_6                       (XFI_PMA_BASE+0x53C)
#define XFI_PMA_RX_TORGS_DEBUG_7                       (XFI_PMA_BASE+0x540)
#define XFI_PMA_RX_TORGS_DEBUG_8                       (XFI_PMA_BASE+0x544)
#define XFI_PMA_RX_TORGS_DEBUG_9                       (XFI_PMA_BASE+0x548)
#define XFI_PMA_RX_TORGS_DEBUG_10                      (XFI_PMA_BASE+0x54C)
#define XFI_PMA_SS_TX_RST_B                            (XFI_PMA_BASE+0x550)
#define XFI_PMA_SS_TX_CALIB_0                          (XFI_PMA_BASE+0x554)
#define XFI_PMA_SS_TX_CALIB_1                          (XFI_PMA_BASE+0x558)
#define XFI_PMA_SS_TX_CALIB_2                          (XFI_PMA_BASE+0x55C)
#define XFI_PMA_XFI_SETTING_0                          (XFI_PMA_BASE+0x600)
#define XFI_PMA_XFI_SETTING_1                          (XFI_PMA_BASE+0x604)
#define XFI_PMA_XFI_SETTING_2                          (XFI_PMA_BASE+0x608)
#define XFI_PMA_XFI_STA                                (XFI_PMA_BASE+0x60C)
#define XFI_PMA_XFI_INT_EN_0                           (XFI_PMA_BASE+0x610)
#define XFI_PMA_XFI_INT_EN_1                           (XFI_PMA_BASE+0x614)
#define XFI_PMA_XFI_INT_EN_2                           (XFI_PMA_BASE+0x618)
#define XFI_PMA_XFI_INT_STA_0                          (XFI_PMA_BASE+0x61C)
#define XFI_PMA_XFI_INT_STA_1                          (XFI_PMA_BASE+0x620)
#define XFI_PMA_XFI_INT_STA_2                          (XFI_PMA_BASE+0x624)
#define XFI_PMA_RX_TORGS_DEBUG_11                      (XFI_PMA_BASE+0x628)
#define XFI_PMA_RX_FORCE_MODE_0                        (XFI_PMA_BASE+0x630)
#define XFI_PMA_RX_FORCE_MODE_1                        (XFI_PMA_BASE+0x634)
#define XFI_PMA_RX_FORCE_MODE_2                        (XFI_PMA_BASE+0x638)
#define XFI_PMA_RX_DISB_MODE_0                         (XFI_PMA_BASE+0x63C)
#define XFI_PMA_RX_DISB_MODE_1                         (XFI_PMA_BASE+0x640)
#define XFI_PMA_RX_DISB_MODE_2                         (XFI_PMA_BASE+0x644)
#define XFI_PMA_RX_FORCE_MODE_3                        (XFI_PMA_BASE+0x648)
#define XFI_PMA_RX_FORCE_MODE_4                        (XFI_PMA_BASE+0x64C)
#define XFI_PMA_RX_FORCE_MODE_5                        (XFI_PMA_BASE+0x650)
#define XFI_PMA_RX_FORCE_MODE_6                        (XFI_PMA_BASE+0x654)
#define XFI_PMA_RX_DISB_MODE_3                         (XFI_PMA_BASE+0x658)
#define XFI_PMA_RX_DISB_MODE_4                         (XFI_PMA_BASE+0x65C)
#define XFI_PMA_RX_DISB_MODE_5                         (XFI_PMA_BASE+0x660)
#define XFI_PMA_RX_FORCE_MODE_7                        (XFI_PMA_BASE+0x664)
#define XFI_PMA_RX_FORCE_MODE_8                        (XFI_PMA_BASE+0x668)
#define XFI_PMA_RX_FORCE_MODE_9                        (XFI_PMA_BASE+0x66C)
#define XFI_PMA_RX_DISB_MODE_6                         (XFI_PMA_BASE+0x670)
#define XFI_PMA_RX_DISB_MODE_7                         (XFI_PMA_BASE+0x674)
#define XFI_PMA_RX_DISB_MODE_8                         (XFI_PMA_BASE+0x678)
#define XFI_PMA_SS_BIST_0                              (XFI_PMA_BASE+0x680)
#define XFI_PMA_SS_BIST_1                              (XFI_PMA_BASE+0x684)
#define XFI_PMA_SS_BIST_2                              (XFI_PMA_BASE+0x688)
#define XFI_PMA_SS_DA_XFI_PWDB_0                       (XFI_PMA_BASE+0x68C)
#define XFI_PMA_SS_DA_XFI_PWDB_1                       (XFI_PMA_BASE+0x690)
#define XFI_PMA_SS_LCPLL_0                             (XFI_PMA_BASE+0x694)
#define XFI_PMA_SS_LCPLL_1                             (XFI_PMA_BASE+0x698)
#define XFI_PMA_SS_LCPLL_2                             (XFI_PMA_BASE+0x69C)
#define XFI_PMA_ADD_DIG_RESERVE_0                      (XFI_PMA_BASE+0x700)
#define XFI_PMA_ADD_DIG_RESERVE_1                      (XFI_PMA_BASE+0x704)
#define XFI_PMA_ADD_DIG_RESERVE_2                      (XFI_PMA_BASE+0x708)
#define XFI_PMA_ADD_DIG_RESERVE_3                      (XFI_PMA_BASE+0x70C)
#define XFI_PMA_ADD_DIG_RESERVE_4                      (XFI_PMA_BASE+0x710)
#define XFI_PMA_RG_XFI_RX_RESERVED_1                   (XFI_PMA_BASE+0x714)
#define XFI_PMA_ADD_DIG_RO_RESERVE_0                   (XFI_PMA_BASE+0x718)
#define XFI_PMA_ADD_DIG_RO_RESERVE_1                   (XFI_PMA_BASE+0x71C)
#define XFI_PMA_ADD_DIG_RO_RESERVE_2                   (XFI_PMA_BASE+0x720)
#define XFI_PMA_ADD_DIG_RO_RESERVE_3                   (XFI_PMA_BASE+0x724)
#define XFI_PMA_ADD_DIG_RO_RESERVE_4                   (XFI_PMA_BASE+0x728)
#define XFI_PMA_ADD_RX_SYS_EN_SEL_0                    (XFI_PMA_BASE+0x72C)
#define XFI_PMA_PLL_TDC_FREQDET_0                      (XFI_PMA_BASE+0x730)
#define XFI_PMA_PLL_TDC_FREQDET_1                      (XFI_PMA_BASE+0x734)
#define XFI_PMA_PLL_TDC_FREQDET_2                      (XFI_PMA_BASE+0x738)
#define XFI_PMA_PLL_TDC_FREQDET_3                      (XFI_PMA_BASE+0x73C)
#define XFI_PMA_DA_XFI_TX_FORCE_0                      (XFI_PMA_BASE+0x740)
#define XFI_PMA_DA_XFI_TX_FORCE_1                      (XFI_PMA_BASE+0x744)
#define XFI_PMA_DA_XFI_TX_FORCE_2                      (XFI_PMA_BASE+0x748)
#define XFI_PMA_RX_FORCE_MODE_10                       (XFI_PMA_BASE+0x74C)
#define XFI_PMA_ADD_CLKPATH_RST_0                      (XFI_PMA_BASE+0x750)
#define XFI_PMA_ADD_XFI_MODE_1                         (XFI_PMA_BASE+0x754)
#define XFI_PMA_ADD_R2T_MODE_1                         (XFI_PMA_BASE+0x758)
#define XFI_PMA_ADD_T2R_MODE_1                         (XFI_PMA_BASE+0x75C)
#define XFI_PMA_ADD_LCPLL_RO_1                         (XFI_PMA_BASE+0x760)
#define XFI_PMA_ADD_RO_RX2ANA_1                        (XFI_PMA_BASE+0x764)
#define XFI_PMA_ADD_RO_RX2ANA_2                        (XFI_PMA_BASE+0x768)
#define XFI_PMA_ADD_RO_RX2ANA_3                        (XFI_PMA_BASE+0x76C)
#define XFI_PMA_ADD_RO_R2TMODE_1                       (XFI_PMA_BASE+0x780)
#define XFI_PMA_SS_JCPLL_PWCTL_SETTING_2               (XFI_PMA_BASE+0x7FC)
#define XFI_PMA_SS_JCPLL_PWCTL_SETTING_0               (XFI_PMA_BASE+0x800)
#define XFI_PMA_SS_JCPLL_PWCTL_SETTING_1               (XFI_PMA_BASE+0x804)
#define XFI_PMA_SS_JCPLL_TDC_FLT_2                     (XFI_PMA_BASE+0x808)
#define XFI_PMA_JCPLL_CHG                              (XFI_PMA_BASE+0x80C)
#define XFI_PMA_RG_JCPLL_SDM_PCW_CTRL                  (XFI_PMA_BASE+0x810)
#define XFI_PMA_RG_JCPLL_SDM_PCW_CHG_CTRL              (XFI_PMA_BASE+0x814)
#define XFI_PMA_RG_JCPLL_XFI_CMN_STB_CTRL              (XFI_PMA_BASE+0x818)
#define XFI_PMA_PLL_TDC_TX_FREQDET_0                   (XFI_PMA_BASE+0x820)
#define XFI_PMA_PLL_TDC_TX_FREQDET_1                   (XFI_PMA_BASE+0x824)
#define XFI_PMA_PLL_TDC_TX_FREQDET_2                   (XFI_PMA_BASE+0x828)
#define XFI_PMA_PLL_TDC_TX_FREQDET_3                   (XFI_PMA_BASE+0x82C)
#define XFI_PMA_XFI_IPATH_CTRL_0                       (XFI_PMA_BASE+0x830)
#define XFI_PMA_XFI_IPATH_CTRL_1                       (XFI_PMA_BASE+0x834)
#define XFI_PMA_XFI_IPATH_CTRL_2                       (XFI_PMA_BASE+0x838)
#define XFI_PMA_XFI_IPATH_CTRL_3                       (XFI_PMA_BASE+0x83C)
#define XFI_PMA_XFI_IPATH_CTRL_4                       (XFI_PMA_BASE+0x840)
#define XFI_PMA_XFI_IPATH_CTRL_5                       (XFI_PMA_BASE+0x844)
#define XFI_PMA_XFI_IPATH_CTRL_6                       (XFI_PMA_BASE+0x848)
#define XFI_PMA_PLL_JCPLL_FT_FREQDET_0                 (XFI_PMA_BASE+0x84C)
#define XFI_PMA_PLL_JCPLL_FT_FREQDET_1                 (XFI_PMA_BASE+0x850)
#define XFI_PMA_PLL_JCPLL_FT_FREQDET_2                 (XFI_PMA_BASE+0x854)
#define XFI_PMA_PLL_JCPLL_FT_FREQDET_3                 (XFI_PMA_BASE+0x858)
#define XFI_PMA_JCPLL_FT_CK_MUX                        (XFI_PMA_BASE+0x85C)
#define XFI_PMA_JCPLL_DA_RG_CTRL_0                     (XFI_PMA_BASE+0x860)
#define XFI_PMA_JCPLL_DA_RG_CTRL_1                     (XFI_PMA_BASE+0x864)
#define XFI_PMA_JCPLL_DA_RG_CTRL_2                     (XFI_PMA_BASE+0x868)
#define XFI_PMA_XFI_PLL_DA_RG_CTRL_0                   (XFI_PMA_BASE+0x86C)
#define XFI_PMA_XFI_PLL_DA_RG_CTRL_1                   (XFI_PMA_BASE+0x870)
#define XFI_PMA_XFI_PLL_DA_RG_CTRL_2                   (XFI_PMA_BASE+0x874)
#define XFI_PMA_AD_DA_PROBE_STS_0                      (XFI_PMA_BASE+0x878)
#define XFI_PMA_AD_DA_PROBE_STS_1                      (XFI_PMA_BASE+0x87C)
#define XFI_PMA_JCPLL_SCAN_CTRL                        (XFI_PMA_BASE+0x880)
#define XFI_PMA_XFI_TX_COUNTER_0                       (XFI_PMA_BASE+0x884)
#define XFI_PMA_XFI_TX_COUNTER_1                       (XFI_PMA_BASE+0x888)
#define XFI_PMA_XFI_TX_COUNTER_2                       (XFI_PMA_BASE+0x88C)
#define XFI_PMA_XFI_TX_COUNTER_3                       (XFI_PMA_BASE+0x890)
#define XFI_PMA_XFI_GPIO_SEL_0                         (XFI_PMA_BASE+0x894)
#define XFI_PMA_TXPLL_SCAN                             (XFI_PMA_BASE+0x898)
#define XFI_PMA_SW_RST_0                               (XFI_PMA_BASE+0x900)
#define XFI_PMA_CK_SEL_MODE1                           (XFI_PMA_BASE+0x904)
#define XFI_PMA_TX_AFIFO_1                             (XFI_PMA_BASE+0x908)
#define XFI_PMA_XFI_PLL_STB_CNT                        (XFI_PMA_BASE+0x90C)
#define XFI_PMA_XFI_PLL_STOP_CNT                       (XFI_PMA_BASE+0x910)
#define XFI_PMA_TX_PAR_PROBE_PLL_CTRL                  (XFI_PMA_BASE+0x914)
#define XFI_PMA_DUMP_RX_PCSRST_CTRL                    (XFI_PMA_BASE+0x918)
#define XFI_PMA_TX_ASIC_FF_CTRL                        (XFI_PMA_BASE+0x91C)
#define XFI_PMA_RG_XFI_TX_RX_MODE                      (XFI_PMA_BASE+0x920)
#define XFI_PMA_XFI_TX_FIFO_STS                        (XFI_PMA_BASE+0x924)
#define XFI_PMA_JCPLL_CK_RSTB_CTRL                     (XFI_PMA_BASE+0x928)
#define XFI_PMA_XFI_XTALCK_GM_CTRL                     (XFI_PMA_BASE+0x92C)
#define XFI_PMA_RX_EXTRAL_CTRL                         (XFI_PMA_BASE+0x930)
#define XFI_PMA_RX_LEQ_DISB_CTRL0                      (XFI_PMA_BASE+0x934)
#define XFI_PMA_RX_LEQ_DISB_CTRL1                      (XFI_PMA_BASE+0x938)
#define XFI_PMA_RX_LEQ_FORCE_CTRL0                     (XFI_PMA_BASE+0x93C)
#define XFI_PMA_RX_LEQ_FORCE_CTRL1                     (XFI_PMA_BASE+0x940)
#define XFI_PMA_RX_LEQ_ENTIME_CTRL0                    (XFI_PMA_BASE+0x944)
#define XFI_PMA_RX_LEQ_ENTIME_CTRL1                    (XFI_PMA_BASE+0x948)
#define XFI_PMA_RX_LEQ_ENTIME_CTRL2                    (XFI_PMA_BASE+0x94C)
#define XFI_PMA_RX_LEQ_CTRL0                           (XFI_PMA_BASE+0x950)
#define XFI_PMA_RX_LEQ_CTRL1                           (XFI_PMA_BASE+0x954)
#define XFI_PMA_RX_LEQ_CTRL2                           (XFI_PMA_BASE+0x958)
#define XFI_PMA_RX_LEQ_RO0                             (XFI_PMA_BASE+0x95C)
#define XFI_PMA_RX_LEQ_RO1                             (XFI_PMA_BASE+0x960)
#define XFI_PMA_RG_JCPLL_500M_FREQ_DET_1               (XFI_PMA_BASE+0x964)
#define XFI_PMA_RG_JCPLL_500M_FREQ_DET_2               (XFI_PMA_BASE+0x968)
#define XFI_PMA_RG_JCPLL_500M_FREQ_DET_3               (XFI_PMA_BASE+0x96C)
#define XFI_PMA_RG_JCPLL_500M_FREQ_DET_4               (XFI_PMA_BASE+0x970)
#define XFI_PMA_RO_RX_FREQDET                          (XFI_PMA_BASE+0x974)
#define XFI_PMA_RO_PLL_FREQDET                         (XFI_PMA_BASE+0x978)
#define XFI_PMA_RO_TDC_TX_FREQDET                      (XFI_PMA_BASE+0x97C)
#define XFI_PMA_RO_JCPLL_FT_FREQDET                    (XFI_PMA_BASE+0x980)
#define XFI_PMA_RO_JCPLL_500M_FREQDET                  (XFI_PMA_BASE+0x984)
#define XFI_PMA_RX_TORGS_DEBUG_12                      (XFI_PMA_BASE+0x988)
#define XFI_PMA_RO_PMA_FREQDET                         (XFI_PMA_BASE+0x98C)
#define XFI_PMA_RG_PMA_FREQDET                         (XFI_PMA_BASE+0x990)
#define XFI_PMA_RO_FLL_ADC_0                           (XFI_PMA_BASE+0x994)
#define XFI_PMA_RO_FLL_ADC_1                           (XFI_PMA_BASE+0x998)
#define XFI_PMA_RO_FLL_ADC_2                           (XFI_PMA_BASE+0x99C)
#define XFI_PMA_RO_FLL_ADC_3                           (XFI_PMA_BASE+0x9A0)
#define XFI_PMA_RO_FLL_ADC_4                           (XFI_PMA_BASE+0x9A4)
#define XFI_PMA_AD_XPON_CDR_LPF_SV                     (XFI_PMA_BASE+0x9A8)
#define XFI_PMA_JCPLL_AUTO_CTRL                        (XFI_PMA_BASE+0x9AC)
#define XFI_PMA_K_VALUE_MON                            (XFI_PMA_BASE+0x9B0)
                                               
                                                 
#define MD32_GATE_HALT                       (MD32_BASE+0x000)
#define MD32_MONCCNT                         (MD32_BASE+0x004)
#define MD32_MONPCNT0                        (MD32_BASE+0x008)
#define MD32_MONPCNT1                        (MD32_BASE+0x00C)
#define MD32_MONPCNT2                        (MD32_BASE+0x010)
#define MD32_MONCONTID                       (MD32_BASE+0x014)
#define MD32_MONPC                           (MD32_BASE+0x018)
#define MD32_TBUF                            (MD32_BASE+0x01C)
#define MD32_TBUFWDATA_L                     (MD32_BASE+0x020)
#define MD32_TBUFWDATA_H                     (MD32_BASE+0x024)
#define MD32_TBUFRDATA_L                     (MD32_BASE+0x028)
#define MD32_TBUFRDATA_H                     (MD32_BASE+0x02C)
#define ADDR_MD32_PMEM_ADDR_REV              (MD32_BASE+0xFF0)
#define ADDR_MD32_PMEM_DATA_REV              (MD32_BASE+0xFF4)
#define ADDR_MD32_DMEM_ADDR_REV              (MD32_BASE+0xFF8)
#define ADDR_MD32_DMEM_DATA_REV              (MD32_BASE+0xFFC)

#endif	/*	EN7580_SERDES	*/

#endif /* _EN7580_REG_H_ */

