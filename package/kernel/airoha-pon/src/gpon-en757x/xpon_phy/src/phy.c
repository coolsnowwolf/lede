/*
* File Name: phy.c
* Description: .
*
******************************************************************/
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/delay.h>

#include <ecnt_hook/ecnt_hook_pon_mac.h>

#include "phy_global.h"
#include "i2c.h"
#include "phy.h"
#include "phy_def.h"
#include "phy_api.h"
#include "phy_debug.h"
#include "mt7570_reg.h"
#include "mt7570_def.h"
#include "en7571_reg.h"
#include "en7571_def.h"

#ifdef TCSUPPORT_XPON_LED
#include <asm/tc3162/ledcetrl.h>
#endif

#include "phy_init.h"
#include <ecnt_hook/ecnt_hook_pon_phy.h>
#include "phy_tx.h"
#include "phy.h"


int isNeedResetTransciver = 0;
extern int	mt7570_select;
extern int	en7571_select;			// by HC 20160608
extern int internal_DDMI;
extern int en7571_internal_DDMI;	// by HC 20160608
extern int en7571_cross;				// by HC 20170531
extern int en7571_cross_cnt;			// by HC 20170531
extern unsigned int en7571_delay1;	// by HC 20170531
extern unsigned int en7571_delay2;	// by HC 20170531

extern int a_tdc_setting;

/*****************************************************************************
XPON PHY Version
******************************************************************************/
extern int mt7570_version;					//by HC 20150320
int phy_version = 106;							//The version of phy.c in main trunk
extern int phy_date_version;					//The version of phy_init.c in main trunk

extern UINT16 SIF_X_Read(UINT8 u1CHannelID, UINT16 u2ClkDiv, UINT8 u1DevAddr, UINT8 u1WordAddrNum,
                         UINT32 u4WordAddr, UINT8 *pu1Buf, UINT16 u2ByteCnt);
extern UINT16 SIF_X_Write(UINT8 u1CHannelID, UINT16 u2ClkDiv, UINT8 u1DevAddr, UINT8 u1WordAddrNum,
                          UINT32 u4WordAddr, UINT8 *pu1Buf, UINT16 u2ByteCnt);

UINT16 phy_I2C_read(UINT8 u1CHannelID, UINT16 u2ClkDiv, UINT8 u1DevAddr, UINT8 u1WordAddrNum,
                         UINT32 u4WordAddr, UINT8 *pu1Buf, UINT16 u2ByteCnt)
{
	uint ret;

	ret = SIF_X_Read(u1CHannelID, u2ClkDiv, u1DevAddr,
					  u1WordAddrNum, u4WordAddr, pu1Buf,
					  u2ByteCnt);
	if(ret == 0) {
			printk("\nError! phy_I2C_read return value = 0\n"
				   "u1DevAddr:0x%x, u4WordAddr:0x%x\n",
				   u1DevAddr, u4WordAddr);
		}


	return ret;
}

UINT16 phy_I2C_write(UINT8 u1CHannelID, UINT16 u2ClkDiv, UINT8 u1DevAddr, UINT8 u1WordAddrNum,
                          UINT32 u4WordAddr, UINT8 *pu1Buf, UINT16 u2ByteCnt)
{
	uint ret;

	ret = SIF_X_Write(u1CHannelID, u2ClkDiv, u1DevAddr,
					   u1WordAddrNum, u4WordAddr, pu1Buf,
					   u2ByteCnt);

	if(ret == 0) {
		printk("\nError! phy_I2C_write return value = 0\n"
			   "u1DevAddr:0x%x, u4WordAddr:0x%x, pu1Buf:0x%x\n",
			   u1DevAddr, u4WordAddr, *(uint *)pu1Buf);
	}

	return ret;
}

#if defined(CONFIG_USE_A60901) || defined(CONFIG_USE_A60928)
uint phy_I2C_read_translet(uint sub_addr)
{
	uint buffer;

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, U1_DEV_ADDR, 2, sub_addr, (unchar *)&buffer, 4);

	buffer = ((buffer & 0x000000FF) << 24) |
			 ((buffer & 0x0000FF00) << 8) |
			 ((buffer & 0x00FF0000) >> 8) |
			 ((buffer & 0xFF000000) >> 24);

	return buffer;
}
EXPORT_SYMBOL(phy_I2C_read_translet);

void phy_I2C_write_translet(uint sub_addr, uint val)
{
	uint buffer = val;

	buffer = ((val & 0x000000FF) << 24) |
			 ((val & 0x0000FF00) << 8) |
			 ((val & 0x00FF0000) >> 8) |
			 ((val & 0xFF000000) >> 24);

	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, U1_DEV_ADDR, 2, sub_addr, (unchar *)&buffer, 4);
}
EXPORT_SYMBOL(phy_I2C_write_translet);
#endif


/*****************************************************************************
//function :
//		phy_counter_clear
//description :
//		this function is used to clear phy counter
//input :
//		counter_type, defined in phy_def.h (ENUM_PhyCounterClr_T)
//output :
//		N/A
******************************************************************************/
void phy_counter_clear(unchar counter_type)
{
	uint read_data = 0;
	uint write_data = 0;
	if((counter_type & PHY_ERR_CNT_CLR)== PHY_ERR_CNT_CLR)
		IO_SPHYREG(PHY_CSR_XP_ERRCNT_CTL, PHY_ERR_CLR);//clear
	if((counter_type & PHY_BIP_CNT_CLR)== PHY_BIP_CNT_CLR)
		IO_SPHYREG(PHY_CSR_XP_ERRCNT_CTL, PHY_BIP_CLR);//clear
	if((counter_type & PHY_RXFRAME_CNT_CLR)== PHY_RXFRAME_CNT_CLR)
		IO_SPHYREG(PHY_CSR_XP_ERRCNT_CTL, PHY_RXFRAME_CLR);//clear
	#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
	if((counter_type & PHY_TXFRAME_CNT_CLR)== PHY_TXFRAME_CNT_CLR)
	{
		read_data = IO_GPHYREG(PHY_CSR_TX_TEST_TRIG);
		write_data = read_data | PHY_TX_CNT_CLR;//clear
		IO_SPHYREG(PHY_CSR_TX_TEST_TRIG,write_data);
	}
	#ifdef TCSUPPORT_WAN_EPON
	if((counter_type & PHY_EPON_ERR_CNT_CLR)== PHY_EPON_ERR_CNT_CLR)
		IO_SPHYREG(PHY_CSR_RX_EPON_CNT_CTL, PHY_EPON_ERR_FRAME_CLR);//clear
	#endif /* TCSUPPORT_WAN_EPON */
	#endif /* CONFIG_USE_MT7520_ASIC */
}

/*****************************************************************************
//function :
//		io_trans_reg
//description :
//		this function is used to process EEPROM register of transceiver
//input :
//		addr : I2C device address
//		reg : I2C sub address
//output :
//		N/A
******************************************************************************/
#if !defined(CONFIG_USE_MT7520_ASIC) && !defined(CONFIG_USE_A60928)
void io_trans_reg(uint addr, unchar reg)
{
#if !defined(CONFIG_USE_A60901)
	ioWritePhyReg8(PHY_CSR_I2C_DEBUGCTRL,0x03);
	mdelay(1);
	ioWritePhyReg8(PHY_CSR_I2C_DATA_PORT,reg);
	mdelay(1);
	ioWritePhyReg8(PHY_CSR_I2C_SLAVE_ADDR,addr);
	mdelay(1);
	ioWritePhyReg8(PHY_CSR_I2C_TIMING+0x1,0x17);
	mdelay(1);
	ioWritePhyReg8(PHY_CSR_I2C_START,0x1);
	mdelay(1);
	ioWritePhyReg8(PHY_CSR_I2C_SLAVE_ADDR,addr+0x1);
	mdelay(1);
	ioWritePhyReg8(PHY_CSR_I2C_START,0x1);
	mdelay(1);
#endif
}
#endif /* !defined CONFIG_USE_MT7520_ASIC  */

void phy_int_enable(uint int_mask)
{
	uint write_data = 0;
	uint read_data = 0;
	read_data = IO_GPHYREG(PHY_CSR_XPON_INT_EN);
	write_data = (read_data & 0xFF ) | (int_mask & 0xFF ) ;

	IO_SPHYREG(PHY_CSR_XPON_INT_EN, write_data)  ;
}

void phy_int_disable(uint int_mask)
{
	uint write_data = 0;
	uint read_data = 0;

	IO_SPHYREG(PHY_CSR_XPON_INT_STA_CLR, (int_mask & 0xFF ) ) ;

	read_data = IO_GPHYREG(PHY_CSR_XPON_INT_EN);
	write_data = (read_data & 0xFF ) & (~(int_mask & 0xFF ) ) ;

	IO_SPHYREG(PHY_CSR_XPON_INT_EN, write_data)  ;
}

/*****************************************************************************
//function :
//		phy_int_config
//description :
//		this function is used to config the system's interrupt
//input :
//		int_type : interrupt type, defined in phy_def.h  Interrupt
//output :
//		N/A
******************************************************************************/
int phy_int_config(uint int_type)
{
	uint write_data = 0;
	uint read_data = 0;
	read_data = IO_GPHYREG(PHY_CSR_XPON_INT_EN);
	write_data = (read_data & (PHY_INT_MASK)) | int_type;//clear bit[7:0] and assign "int_type" to bit[7:0]

	PON_PHY_MSG(PHY_MSG_DBG, "Phy_Int_Config read_data:0x%.8x\n", read_data);
	PON_PHY_MSG(PHY_MSG_DBG, "Phy_Int_Config write_data:0x%.8x\n", write_data);
	IO_SPHYREG(PHY_CSR_XPON_INT_EN, write_data);
	return PHY_SUCCESS;
}


/*****************************************************************************
//function :
//		phy_int_config
//description :
//		this function is used to get the system's interrupt
//input :
//		N/A
//output :
//		interrupt config
******************************************************************************/
int phy_int_getting(void)
{
	return IO_GPHYREG(PHY_CSR_XPON_INT_EN);
}



/*****************************************************************************
//function :
//		phy_gpon_mode
//description :
//		this function is used to read which mode the system work at
//input :
//		N/A
//output :
//		1 : means GPON mode ; 0 : means EPON mode
******************************************************************************/
unchar phy_gpon_mode(void)
{
	uint read_data ;

	read_data = IO_GPHYREG(PHY_CSR_PHYSET10) ;
	return ((read_data&PHY_GPON_MODE) ? 1 : 0) ;
}

/*****************************************************************************
//function :
//		phy_mode_config
//description :
//		this function is used to config the setting of GPON/EPON mode
//input :
//		xpon_mode : PHY_GPON_CONFIG used to config GPON mode ; PHY_EPON_CONFIG  used to config EPON mode
//output :
//		setting success or failure
******************************************************************************/
int phy_mode_config(Xpon_Phy_Mode_t xpon_mode, int tx_enable)
{
	uint write_data = 0 ;
	uint read_data = 0 ;

	//uint counter = 0;
	//unchar ptr[4];
	//printk("phy_mode_config start\n");		//debug msg YMC 20150125


    if(PHY_GPON_CONFIG != xpon_mode && PHY_EPON_CONFIG != xpon_mode) {
        printk("can not recognize phy mode %d\n", xpon_mode);
        return PHY_FAILURE;
    }

    if(PHY_ENABLE != tx_enable && PHY_DISABLE != tx_enable) {
        printk("can not recognize tx_enable %d\n", tx_enable);
        return PHY_FAILURE;
    }


	if(xpon_mode == PHY_GPON_CONFIG) {
		/* disable power */
		phy_trans_power_switch(PHY_DISABLE);
		/* set to burst mode*/
		read_data = IO_GPHYREG(PHY_CSR_PHYSET3) ;
		write_data = read_data & PHY_TX_EPON_CONT_MODE;
		IO_SPHYREG(PHY_CSR_PHYSET3,write_data);
		/* change mode*/
		read_data = IO_GPHYREG(PHY_CSR_PHYSET10) ;
		#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
		write_data = (read_data | PHY_GPON_MODE);
		#else
		write_data = (read_data | PHY_GPON_MODE) | PHY_GPON_DEMASK;
		#endif  /* CONFIG_USE_MT7520_ASIC */
		IO_SPHYREG(PHY_CSR_PHYSET10, write_data) ;
		/*reset phy*/
		read_data = IO_GPHYREG(PHY_CSR_PHYSET3);
		write_data = read_data | (PHY_PLL_RST) |(PHY_COUNT_RST);
		IO_SPHYREG(PHY_CSR_PHYSET3, write_data) ;
		mdelay(1);
		IO_SPHYREG(PHY_CSR_PHYSET3, read_data) ;

		#ifdef CONFIG_USE_A60901
		IO_SPHYREG(PHY_CSR_ANASET10, 0x211A0202) ;//cdr_bir
		#endif /* CONFIG_USE_A60901 */
	} else if(xpon_mode == PHY_EPON_CONFIG) {
		/* disable power */
		phy_trans_power_switch(PHY_DISABLE);
		/* set to burst mode*/
		read_data = IO_GPHYREG(PHY_CSR_PHYSET3) ;
		write_data = read_data & PHY_TX_EPON_CONT_MODE;
		IO_SPHYREG(PHY_CSR_PHYSET3,write_data);
		/* change mode*/
		read_data = IO_GPHYREG(PHY_CSR_PHYSET10) ;
		#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
		write_data = (read_data & ~PHY_GPON_MODE);
		#else
		write_data = (read_data & ~PHY_GPON_MODE) & ~PHY_GPON_DEMASK  ;
		#endif  /* CONFIG_USE_MT7520_ASIC */
		IO_SPHYREG(PHY_CSR_PHYSET10, write_data) ;
		/* reset phy*/
		read_data = IO_GPHYREG(PHY_CSR_PHYSET3);
		write_data = read_data | (PHY_PLL_RST) |(PHY_COUNT_RST);
		IO_SPHYREG(PHY_CSR_PHYSET3, write_data) ;
		mdelay(1);
		IO_SPHYREG(PHY_CSR_PHYSET3, read_data) ;

		#ifdef CONFIG_USE_A60901
		IO_SPHYREG(PHY_CSR_ANASET12, 0x211A0202) ;//cdr_bir
		IO_SPHYREG(PHY_CSR_EPON_FRAM_SRH_CTL, 0x00000045) ;
		#endif /* CONFIG_USE_A60901 */
		/* set to continuous mode*/
		read_data = IO_GPHYREG(PHY_CSR_PHYSET3) ;
		write_data = read_data | ~(PHY_TX_EPON_CONT_MODE);
		IO_SPHYREG(PHY_CSR_PHYSET3, write_data) ;
		if(mt7570_select == 1)
		{
			#if 0			//not necessary in run time. YMC 20150125
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SAFE_PROTECT, ptr, 4);
			ptr[1] = (ptr[1] & safe_circuit_mask) | (safe_circuit_reset);
			phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SAFE_PROTECT, ptr, 4);
			#endif
		}
	}

    if (PHY_ENABLE == tx_enable){
        phy_trans_power_switch(PHY_ENABLE);
	}


	//PON_PHY_MSG(PHY_MSG_DBG, "Phy_Mode_Config read_data:0x%.8x, write_data:0x%.8x\n", read_data, write_data) ;
	//printk("phy_mode_config end\n");		//debug msg YMC 20150125
	gpPhyPriv->phyCfg.flags.mode = xpon_mode;
	return PHY_SUCCESS ;
}

/*****************************************************************************
//function :
//		phy_set_traffic_status
//description :
//		this function is used to set traffic status from MAC layer
//input :
//		traffic status : TRAFFIC_STATUS_DOWN / TRAFFIC_STATUS_UP
//output :
//		setting success or failure
******************************************************************************/
int phy_set_traffic_status(ENUM_PhyTrafficStatus  traffic_status)
{
    if (TRAFFIC_STATUS_DOWN != traffic_status && TRAFFIC_STATUS_UP !=  traffic_status)
        return PHY_FAILURE;

	gpPhyPriv->phyCfg.flags.trafficStatus  = traffic_status;
	return PHY_SUCCESS ;
}


#ifdef TCSUPPORT_CPU_EN7521
/*****************************************************************************
//function :
//		phy_set_epon_ts_continue_mode
//description :
//		this function is used to set burst mode or continue mode
//input :
//		mode : PHY_ENABLE meas continue mode(phy_burst_en | mac_burst_en); PHY_DISABLE means burst_mode(phy_burst_en)
//output :
//		PHY_SUCCESS
******************************************************************************/
int phy_set_epon_ts_continue_mode(uint mode)
{
	uint read_data = 0;

	if (mode == PHY_ENABLE)
	{
		read_data = IO_GPHYREG(PHY_CSR_PHYSET10);
		read_data |= (0x01 << 26);
		IO_SPHYREG(PHY_CSR_PHYSET10, read_data);
	}
	else if (mode == PHY_DISABLE)
	{
		read_data = IO_GPHYREG(PHY_CSR_PHYSET10);
		read_data &= ~(0x01 << 26);
		IO_SPHYREG(PHY_CSR_PHYSET10, read_data);
	}

	return PHY_SUCCESS;
}

/*****************************************************************************
//function :
//		phy_round_trip_delay_sof
//description :
//		this function is used to set burst mode or continue mode
//input :
//		void
//output :
//		ushort round_trip_delay_sof
******************************************************************************/

ushort phy_round_trip_delay_sof(void)
{
	  uint read_data = 0;
	  ushort round_trip_delay_sof=0;

	  read_data = IO_GPHYREG(PHY_CSR_ROUND_TRIP_DELAY);
	  round_trip_delay_sof=read_data&0x0000ffff;
	  return round_trip_delay_sof;
}

#endif

int eponMode_phy_reset(void)		//Fixed by HC 20150207
{
	uint write_data = 0 ;
	uint read_data = 0 ;
	//uint counter = 0;
	int isPowerDisable = 0;
	uint tmp_data = 0 ;

#ifdef CONFIG_USE_MT7520_ASIC
#if !defined(TCSUPPORT_CHS) && !defined(TCSUPPORT_TW_BOARD_CJ) && !defined(TCSUPPORT_CT_PON_C7) && !defined(TCSUPPORT_TW_BOARD_CDS) && !defined(TCSUPPORT_GPIO40)
	read_data = IO_GREG(GPIO_CSR_DATA);
#endif
	PON_PHY_MSG(PHY_MSG_DBG, "eponMode_phy_reset 1:isPowerDisable = %d,read_data=0x%.8x\n", isPowerDisable,read_data) ;
#endif

#if !defined(TCSUPPORT_CHS) && !defined(TCSUPPORT_TW_BOARD_CJ) && !defined(TCSUPPORT_CT_PON_C7) && !defined(TCSUPPORT_GPIO40)

	if(en7571_select == 1)
		tmp_data = PHY_7571_DISABLE_TRANS;
	else if(mt7570_select == 1)
		tmp_data = PHY_7570_DISABLE_TRANS;
	else
		tmp_data = PHY_DISABLE_TRANS;
#endif


	if(read_data & tmp_data)

	{
		isPowerDisable = 1;
		PON_PHY_MSG(PHY_MSG_DBG, "eponMode_phy_reset:isPowerDisable = %d\n", isPowerDisable) ;
	}


	/* disable power */
	phy_trans_power_switch(PHY_DISABLE);
	PON_PHY_MSG(PHY_MSG_DBG, "eponMode_phy_reset 1:isPowerDisable = %d,read_data=0x%.8x\n", isPowerDisable,read_data) ;

	/* set to burst mode*/
	read_data = IO_GPHYREG(PHY_CSR_PHYSET3) ;
	write_data = read_data & PHY_TX_EPON_CONT_MODE;
	IO_SPHYREG(PHY_CSR_PHYSET3,write_data);
	/* change mode*/
	read_data = IO_GPHYREG(PHY_CSR_PHYSET10) ;
	#ifdef CONFIG_USE_MT7520_ASIC
	write_data = (read_data & ~PHY_GPON_MODE);
	#else
	write_data = (read_data & ~PHY_GPON_MODE) & ~PHY_GPON_DEMASK  ;
	#endif  /* CONFIG_USE_MT7520_ASIC */
	IO_SPHYREG(PHY_CSR_PHYSET10, write_data) ;
	/* reset phy*/
	read_data = IO_GPHYREG(PHY_CSR_PHYSET3);
	write_data = read_data | (PHY_PLL_RST) |(PHY_COUNT_RST);
	IO_SPHYREG(PHY_CSR_PHYSET3, write_data) ;
	mdelay(1);
	IO_SPHYREG(PHY_CSR_PHYSET3, read_data) ;

	#ifdef CONFIG_USE_A60901
	IO_SPHYREG(PHY_CSR_ANASET12, 0x211A0202) ;//cdr_bir
	IO_SPHYREG(PHY_CSR_EPON_FRAM_SRH_CTL, 0x00000045) ;
	#endif /* CONFIG_USE_A60901 */
	/* set to continuous mode*/
	read_data = IO_GPHYREG(PHY_CSR_PHYSET3) ;
	write_data = read_data | ~(PHY_TX_EPON_CONT_MODE);
	IO_SPHYREG(PHY_CSR_PHYSET3, write_data) ;

	if(isPowerDisable == 0){
		/*enable power*/
		phy_trans_power_switch(PHY_ENABLE);
		PON_PHY_MSG(PHY_MSG_DBG, "eponMode_phy_reset 1:isPowerDisable = %d,read_data=0x%.8x\n", isPowerDisable,read_data) ;
	}else if (gpPhyPriv->phyCfg.flags.txPowerEnFlag == 1)
	{
		phy_trans_power_switch(PHY_ENABLE);
		PON_PHY_MSG(PHY_MSG_DBG, "eponMode_phy_reset 1:isPowerDisable = %d,read_data=0x%.8x\n", isPowerDisable,read_data) ;
	}

	return PHY_SUCCESS ;
}

/*****************************************************************************
//function :
//		phy_ready_status
//description :
//		this function is used to get the phy ready state of system
//input :
//		N/A
//output :
//		PHY_TRUE : the system is now phy ready
//		PHY_FALSE : the system isn't phy ready
******************************************************************************/
int phy_ready_status(void)
{
		uint read_data = 0;
		read_data = IO_GPHYREG(PHY_CSR_PHYSTA1);//read bit[18:20]
		if(((read_data >> PHYRDY_OFFSET) & PHYRDY_MASK) == PHYRDY_STATUS)
		{
			return PHY_TRUE;
		}
		else
		{
			return PHY_FALSE;
		}
}

/*****************************************************************************
//function :
//		phy_los_status
//description :
//		this function is used to get the LOS state of system
//input :
//		N/A
//output :
//		PHY_LOS_HAPPEN : LOS happen
//		PHY_NO_LOS_HAPPEN: LOS does not happen
******************************************************************************/
int phy_los_status(void)
{
		uint read_data = 0;
		read_data = IO_GPHYREG(PHY_CSR_XPON_STA);
		if( (read_data & PHYLOS_STATUS) == PHYLOS_STATUS) //rx sd
		{
			return PHY_LOS_HAPPEN;
		}
		else
		{
			return PHY_NO_LOS_HAPPEN;
		}


}


/*****************************************************************************
//function :
//		phy_lof_status
//description :
//		this function is used to get the LOF state of system
//input :
//		N/A
//output :
//		PHY_TRUE : LOF happen
//		PHY_FALSE : LOF does not happen or LOF interrupt is not enabled
******************************************************************************/
int phy_lof_status(void)
{
	uint read_data = 0;
	read_data = IO_GPHYREG(PHY_CSR_XPON_INT_EN);
	if((read_data & PHY_LOF_INT) != PHY_LOF_INT)
	{
		PON_PHY_MSG(PHY_MSG_DBG, "LOF Interrupt is not turned on. \n");
		return PHY_FALSE;
	}
	else
	{
		read_data = IO_GPHYREG(PHY_CSR_XPON_INT_STA);
		if((read_data & PHY_LOF_INT) == PHY_LOF_INT)
		{
			PON_PHY_MSG(PHY_MSG_DBG, "Phy LOF. \n");
			return PHY_TRUE;
		}
		else
		{
			PON_PHY_MSG(PHY_MSG_DBG, "Phy not LOF. \n");
			return PHY_FALSE;
		}
	}

}

/*****************************************************************************
//function :
//		phy_reset_counter
//description :
//		this function is used to reset phy and reset counter
//input :
//		N/A
//output :
//		setting success or failure
******************************************************************************/
int phy_reset_counter(void)
{
	#if 0
	uint write_data = 0;
	uint read_data = 0;
	read_data = IO_GPHYREG(PHY_CSR_PHYSET3);
	PON_PHY_MSG(PHY_MSG_DBG, "Phy_Reset_Counter read_data before reset:0x%.8x\n", read_data);
	write_data = (read_data & ~(PHY_COUNT_RST)) | (PHY_COUNT_RST);
	PON_PHY_MSG(PHY_MSG_DBG, "Phy_Reset_Counter write_data:0x%.8x\n", write_data);
	IO_SPHYREG(PHY_CSR_PHYSET3, write_data);
	write_data = (read_data & ~(PHY_COUNT_RST));
	PON_PHY_MSG(PHY_MSG_DBG, "Phy_Reset_Counter read_data after reset:0x%.8x\n", read_data);

	PON_PHY_MSG(PHY_MSG_DBG, "Phy Reset ok. \n");
	IO_SPHYREG(PHY_CSR_PHYSET3, write_data);
	return PHY_SUCCESS;
	#else
	uint read_data = 0;
	uint write_data = 0;
		IO_SPHYREG(PHY_CSR_XP_ERRCNT_CTL, PHY_ERR_CLR);//clear
		IO_SPHYREG(PHY_CSR_XP_ERRCNT_CTL, PHY_BIP_CLR);//clear
		IO_SPHYREG(PHY_CSR_XP_ERRCNT_CTL, PHY_RXFRAME_CLR);//clear
	#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
		read_data = IO_GPHYREG(PHY_CSR_TX_TEST_TRIG);
		write_data = read_data | PHY_TX_CNT_CLR;//clear
		IO_SPHYREG(PHY_CSR_TX_TEST_TRIG,write_data);
	#ifdef TCSUPPORT_WAN_EPON
		IO_SPHYREG(PHY_CSR_RX_EPON_CNT_CTL, PHY_EPON_ERR_FRAME_CLR);//clear
	#endif /* TCSUPPORT_WAN_EPON */
	#endif /* CONFIG_USE_MT7520_ASIC */
	#endif
    return PHY_SUCCESS;
}

/*****************************************************************************
//function :
//		phy_cnt_enable
//description :
//		this function is used to enable counter statistic
//input :
//		errcnt_en : PHY_ENABLE means turn on rx error counter ; PHY_DISABLE means turn off rx error counter
//		bipcnt_en : PHY_ENABLE means turn on rx bip error counter ; PHY_DISABLE means turn off rx bip error counter
//		fmcnt_en : PHY_ENABLE means turn on rx frame counter ; PHY_DISABLE means turn off rx frame counter
//output :
//		setting success or failure
******************************************************************************/
int phy_cnt_enable(unchar errcnt_en, unchar bipcnt_en, unchar fmcnt_en)
{
	uint write_data = 0;
	uint read_data = 0;
	uint clear_type = 0;
	read_data = IO_GPHYREG(PHY_CSR_XP_ERRCNT_EN);
	read_data = read_data & PHY_ERRCNT_MASK;
	if((errcnt_en == PHY_ENABLE) || (errcnt_en == PHY_DISABLE))
	{
		read_data = read_data | errcnt_en;
		clear_type = clear_type | PHY_ERR_CNT_CLR;
	}
	if((bipcnt_en == PHY_ENABLE) || (bipcnt_en == PHY_DISABLE))
	{
		read_data = read_data | (bipcnt_en << PHY_BIP_CNT_OFFSET);
		clear_type = clear_type | PHY_BIP_CNT_CLR;
	}
	if((fmcnt_en == PHY_ENABLE) || (fmcnt_en == PHY_DISABLE))
	{
		read_data = read_data | (fmcnt_en << PHY_FRAME_CNT_OFFEST);
		clear_type = clear_type | PHY_RXFRAME_CNT_CLR;
	}
	write_data = read_data;
	udelay(1);
	IO_SPHYREG(PHY_CSR_XP_ERRCNT_EN, write_data);
	phy_counter_clear(clear_type);
	return PHY_SUCCESS;
}


/*****************************************************************************
//function :
//		phy_trans_model_setting
//description :
//		this function is used to match transceiver pin define
//input :
//		N/A
//output :
//		N/A
******************************************************************************/
#define TCSUPPORT_PHY_TRANSCEIVER_LIST
#ifdef TCSUPPORT_PHY_TRANSCEIVER_LIST
#define MAX(a, b)          (((a)>(b))?(a):(b))

struct phy_trans_bsten_info {
	char vendor_id[PHY_TRANS_VENDOR_OFFSET+1];
	char vendor_pn[PHY_TRANS_VENDOR_OFFSET+1];
	ushort phy_csr_xpon_val;
};

struct phy_trans_bsten_info phy_trans_spt_list[] =
{
	{"WTD", "RTXM167-407-C10", 0x1cf},
	{"WTD", "RTXM169-504", 0x1cf},
	{"WTD", "RTXM169-514", 0x1cf},
	{"SOURCEPHOTONICS", "SFA3424THPCDFJ", 0x14f},
	{"SOURCEPHOTONICS", "SFU34GBP2CDFA", 0x1cf},
	{"Hisense", "LTB34D2-SC+", 0x1cf},
	{"Hisense", "LTB34D3-SC+", 0x1cf},
	{"Hisense", "LTE3468L-BHG+", 0x18f},

#if !defined(TCSUPPORT_CT_PON_C7)
	{"Hisense", "LTB3468-BHG+", 0x14f},
#endif
	{"DARE TECH OPT", "E31LA21B+", 0x1cf},
	{"APAC Opto", "LSF2C3STCN322", 0x14f},
	{"APAC Opto", "LSF2C3MTCN3G4", 0x14f},
	{"MENTECHOPTO", "NOG22-D6C-ND", 0x14f},
	{"DELTA",  "OPEP-33-A4G1RC", 0x1cf},
	{"MENTECHOPTO", "NOE22-LD6C-NT",	0x1cf}, /* epon */
	{"SUPERXON LTD.", "SOEB3466-FSGF",	0x1cf}, /* epon */
	{"EOPTOLINK INC", "EOLF-GUA-25-DZTE", 0x14f},	/* gpon */
	{"MENTECHOPTO", "NOG22-D6C-ND", 0x14f},	/* gpon */
	{"SUPERXON LTD.", "SOGP3412-FSGA", 0x14f},	/* gpon */
	{"SUPERXON LTD.", "SOEB3466-FUGF", 0x1cf},
	{"WTD", "RTXM167-407", 0x1cf},
        {"XUGUANG LTD.", "XTEC3411F-C", 0x1cf},/* jiuzhou epon */
	{"XUGUANG LTD.", "XTGP3434FRS-A", 0x14f},	/* jiuzhou gpon */
    {"T&W", "TW2362G-CDEH", 0x10f},
	{"HUAWEI", "HPSP2120", 0x1cf},
	{"Hisense", "LTE3468-BCG+", 0x10f},
	{"Hisense", "LTE34D3-SH+", 0x1cf},
	{"HONLUS", "HOLSFP3420330C", 0x1cf},
	{"HONLUS", "HOLSP3412452DC0C", 0x1cf},
	{"T&W", "TW2362G-CDEH", 0x10f},
	{"EOPTOLINK INC", "EOLF-GEUA-25-D1A", 0x1cf},	/* epon */
	{"Q-STAR", "BYFS-462A4-HB-12", 0x14f}, /* gpon */
	{"Q-STAR", "BYFS-44234-LB-12", 0x1cf}, /* epon */
	{"MENTECHOPTO", "NOG22-D6C-SD", 0x14f}, /* gpon */
	{"MENTECHOPTO", "NOE22-LD6C-SD", 0x1cf}, /* epon */
	{"MENTECHOPTO", "NOG22-LD6C-ST", 0x14f},
	{"MENTECHOPTO", "NOE22-LD6C-ST", 0x1cf},
	{"FIBERTOWER", "FEU3411S-2DC-0C", 0x1cf}, /* epon */
	{"FIBERTOWER", "FGU3412S-2DC-1C", 0x14f},
	{"ZKTEL", "ZP4342034-KCST", 0x14f},
	/* add support transceiver support before here */
	{"", "", 0}
};

char* rtrim(char *name){
	int i = 0, len = strlen(name);

	for (i = len-1; i >= 0 && name[i] ==' '; i--)
		name[i] = '\0';
	return name;
}
#endif

void phy_trans_model_setting(void)
{
	int disp = 0;
	if(en7571_select == 1)	// by HC 20160608
	{
		en7571_trans_model_setting();
	}
	else if(mt7570_select == 1)
	{
		mt7570_trans_model_setting();
	}
	else
	{
#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
	//int index = 0;
	static unchar vendor_id[PHY_TRANS_VENDOR_OFFSET+1]={0};
	static unchar vendor_pn[PHY_TRANS_VENDOR_OFFSET+1]={0};
	//unchar ptr[2];
	//ushort u2ByteCnt;
#ifdef TCSUPPORT_PHY_TRANSCEIVER_LIST
	struct phy_trans_bsten_info *pTrans = NULL;
#endif

	if ( (strlen(vendor_id) == 0)
		&& (strlen(vendor_pn) == 0))
	{
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE1, 1, PHY_TRANS_VENDOR_NAME , vendor_id, PHY_TRANS_VENDOR_OFFSET);//read 16byte once
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE1, 1, PHY_TRANS_VENDOR_PN , vendor_pn, PHY_TRANS_VENDOR_OFFSET);
		vendor_pn[PHY_TRANS_VENDOR_OFFSET]='\0';
		vendor_id[PHY_TRANS_VENDOR_OFFSET]='\0';
#ifdef TCSUPPORT_PHY_TRANSCEIVER_LIST
		rtrim(vendor_id);
		rtrim(vendor_pn);
#endif
	}

	if(disp!=0){
		printk("vendor id : %s\n", vendor_id);
		printk("vendor pn : %s\n", vendor_pn);
	}

#ifdef TCSUPPORT_PHY_TRANSCEIVER_LIST
	pTrans = phy_trans_spt_list;
	while (strcmp(pTrans->vendor_id, "") != 0){
		if ((strncmp(pTrans->vendor_id, vendor_id, MAX(strlen(pTrans->vendor_id), strlen(vendor_id))) == 0) &&
			(strncmp(pTrans->vendor_pn, vendor_pn, MAX(strlen(pTrans->vendor_pn), strlen(vendor_pn))) == 0)){
			if(disp!=0)
				printk("Match Vendor\n");
			IO_SPHYREG(PHY_CSR_XPON_SETTING, pTrans->phy_csr_xpon_val);


			break;
		}
		pTrans ++;
	}
#else
	if(strcmp(vendor_id, "WTD             ") == 0 )
	{
		if(strcmp(vendor_pn, "RTXM167-407-C10 ") == 0)
			IO_SPHYREG(PHY_CSR_XPON_SETTING, 0x1cf);
		else if(strcmp(vendor_pn, "RTXM169-504     ") == 0)
			IO_SPHYREG(PHY_CSR_XPON_SETTING, 0x1cf);
	}
	else if(strcmp(vendor_id, "SOURCEPHOTONICS ") == 0)
	{
		if(strcmp(vendor_pn, "SFA3424THPCDFJ  ") == 0)
			IO_SPHYREG(PHY_CSR_XPON_SETTING, 0x14f);
		else if(strcmp(vendor_pn, "SFU34GBP2CDFA   ") == 0)
			IO_SPHYREG(PHY_CSR_XPON_SETTING, 0x1cf);
	}
	else if(strcmp(vendor_id, "Hisense         ") == 0)
	{
		if(strcmp(vendor_pn, "LTB34D3-SC+     ") == 0)
			IO_SPHYREG(PHY_CSR_XPON_SETTING, 0x1cf);
		else if(strcmp(vendor_pn, "LTB3468-BHG+    ") == 0)
			IO_SPHYREG(PHY_CSR_XPON_SETTING, 0x14f);
	}
#endif
#endif /* CONFIG_USE_MT7520_ASIC */
	}
}



/*****************************************************************************
//function :
//		phy_trans_param_status_real
//description :
//		this function is used to get transceiver parameter realtime
		Note: before call this function, should check if(i2c_protect == 0)
//input :
//		PPHY_TransParam_T structure, defined in phy_api.h
//output :
//		N/A
******************************************************************************/
#if 1
void phy_trans_param_status_real(PPHY_TransParam_T transceiver_param)
#else
void trans_param_status_real(PPHY_TransParam_T transceiver_param)
#endif
{
	#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
		//unchar ptr[2];
		//ushort u2ByteCnt;
#if 1
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, 1, PHY_TRANS_TEMP_HBYTE, transceiver_param, sizeof(PHY_TransParam_T));//read 10byte once
#else
		//temprature
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, 1, PHY_TRANS_TEMP_HBYTE, ptr, 2);
		transceiver_param->temprature = (ptr[1] | (ptr[0]<<8));

		//voltage
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, 1, PHY_TRANS_VOLT_HBYTE, ptr, 2);
		transceiver_param->supply_voltage = (ptr[1] | (ptr[0]<<8));

		//tx_curretn
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, 1, PHY_TRANS_TX_CUR_HBYTE, ptr, 2);
		transceiver_param->tx_current = (ptr[1] | (ptr[0]<<8));

		//tx_power
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, 1, PHY_TRANS_TX_POW_HBYTE, ptr, 2);
		transceiver_param->tx_power = (ptr[1] | (ptr[0]<<8));

		//rx_power
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, 1, PHY_TRANS_RX_POW_HBYTE, ptr, 2);
		transceiver_param->rx_power = (ptr[1] | (ptr[0]<<8));
#endif
	#else
		uint read_data = 0;
		//temprature
		io_trans_reg(PHY_TRANS_REG_TABLE2,PHY_TRANS_TEMP_HBYTE);
		read_data = (IO_GPHYREG(PHY_CSR_I2C_DATA_PORT) << PHY_TRANS_OFFSET);
		io_trans_reg(PHY_TRANS_REG_TABLE2,PHY_TRANS_TEMP_LBYTE);
		read_data = read_data + IO_GPHYREG(PHY_CSR_I2C_DATA_PORT);
		transceiver_param->temprature = read_data;
		//voltage
		io_trans_reg(PHY_TRANS_REG_TABLE2,PHY_TRANS_VOLT_HBYTE);
		read_data = (IO_GPHYREG(PHY_CSR_I2C_DATA_PORT) << PHY_TRANS_OFFSET);
		io_trans_reg(PHY_TRANS_REG_TABLE2,PHY_TRANS_VOLT_LBYTE);
		read_data = read_data + IO_GPHYREG(PHY_CSR_I2C_DATA_PORT);
		transceiver_param->supply_voltage = read_data;
		//tx_current
		io_trans_reg(PHY_TRANS_REG_TABLE2,PHY_TRANS_TX_CUR_HBYTE);
		read_data = (IO_GPHYREG(PHY_CSR_I2C_DATA_PORT) << PHY_TRANS_OFFSET);
		io_trans_reg(PHY_TRANS_REG_TABLE2,PHY_TRANS_TX_CUR_LBYTE);
		read_data = read_data + IO_GPHYREG(PHY_CSR_I2C_DATA_PORT);
		transceiver_param->tx_current = read_data;
		//tx_power
		io_trans_reg(PHY_TRANS_REG_TABLE2,PHY_TRANS_TX_POW_HBYTE);
		read_data = (IO_GPHYREG(PHY_CSR_I2C_DATA_PORT) << PHY_TRANS_OFFSET);
		io_trans_reg(PHY_TRANS_REG_TABLE2,PHY_TRANS_TX_POW_LBYTE);
		read_data = read_data + IO_GPHYREG(PHY_CSR_I2C_DATA_PORT);
		transceiver_param->tx_power = read_data;
		//rx_power
		io_trans_reg(PHY_TRANS_REG_TABLE2,PHY_TRANS_RX_POW_HBYTE);
		read_data = (IO_GPHYREG(PHY_CSR_I2C_DATA_PORT) << PHY_TRANS_OFFSET);
		io_trans_reg(PHY_TRANS_REG_TABLE2,PHY_TRANS_RX_POW_LBYTE);
		read_data = read_data + IO_GPHYREG(PHY_CSR_I2C_DATA_PORT);
		transceiver_param->rx_power= read_data;
	#endif /* CONFIG_USE_MT7520_ASIC */
		//PON_PHY_MSG(PHY_MSG_DBG, "temprature Verify: %.8x\n", transceiver_param->temprature);
		//PON_PHY_MSG(PHY_MSG_DBG, "voltage Verify: %.8x\n", transceiver_param->supply_voltage);
		//PON_PHY_MSG(PHY_MSG_DBG, "current Verify: %.8x\n", transceiver_param->tx_current);
		//PON_PHY_MSG(PHY_MSG_DBG, "tx power Verify: %.8x\n", transceiver_param->tx_power);
		//PON_PHY_MSG(PHY_MSG_DBG, "rx power Verify: %.8x\n", transceiver_param->rx_power);
}

/*****************************************************************************
//function :
//		phy_trans_alarm_getting
//description :
//		this function is used to get alarm threshold of transceiver (not for EN7570)
//input :
//		PPHY_TransAlarm_T structure, defined in phy_api.h
//output :
//		N/A
******************************************************************************/
void phy_trans_alarm_getting(PPHY_TransAlarm_T transceiver_alarm)
{
		#if 0
		if(i2c_protect == 0)
		{
			i2c_protect = 1;
		#endif
		#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
			unchar ptr[2];
			//ushort u2ByteCnt;
		//rx power high alarm threshold
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, 1, PHY_TRANS_RX_HIGH_ALARM_HBYTE, ptr, 2);
			transceiver_alarm->rx_power_high_alarm_thod = (ptr[1] | (ptr[0]<<8));
		//rx power low alarm threshold
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, 1, PHY_TRANS_RX_LOW_ALARM_HBYTE, ptr, 2);
			transceiver_alarm->rx_power_low_alarm_thod = (ptr[1] | (ptr[0]<<8));
		//tx power high alarm threshold
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, 1, PHY_TRANS_TX_HIGH_ALARM_HBYTE, ptr, 2);
			transceiver_alarm->tx_power_high_alarm_thod = (ptr[1] | (ptr[0]<<8));
		//tx power low alarm threshold
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, 1, PHY_TRANS_TX_LOW_ALARM_HBYTE, ptr, 2);
			transceiver_alarm->tx_power_low_alarm_thod = (ptr[1] | (ptr[0]<<8));
		#else
			uint read_data = 0;
		//rx power high alarm threshold
			io_trans_reg(PHY_TRANS_REG_TABLE2,PHY_TRANS_RX_HIGH_ALARM_HBYTE);
			read_data = (IO_GPHYREG(PHY_CSR_I2C_DATA_PORT) << PHY_TRANS_OFFSET);
			io_trans_reg(PHY_TRANS_REG_TABLE2,PHY_TRANS_RX_HIGH_ALARM_LBYTE);
			read_data = read_data + IO_GPHYREG(PHY_CSR_I2C_DATA_PORT);
			transceiver_alarm->rx_power_high_alarm_thod= read_data;
		//rx power low alarm threshold
			io_trans_reg(PHY_TRANS_REG_TABLE2,PHY_TRANS_RX_LOW_ALARM_HBYTE);
			read_data = (IO_GPHYREG(PHY_CSR_I2C_DATA_PORT) << PHY_TRANS_OFFSET);
			io_trans_reg(PHY_TRANS_REG_TABLE2,PHY_TRANS_RX_LOW_ALARM_LBYTE);
			read_data = read_data + IO_GPHYREG(PHY_CSR_I2C_DATA_PORT);
			transceiver_alarm->rx_power_low_alarm_thod= read_data;
		//tx power high alarm threshold
			io_trans_reg(PHY_TRANS_REG_TABLE2,PHY_TRANS_TX_HIGH_ALARM_HBYTE);
			read_data = (IO_GPHYREG(PHY_CSR_I2C_DATA_PORT) << PHY_TRANS_OFFSET);
			io_trans_reg(PHY_TRANS_REG_TABLE2,PHY_TRANS_TX_HIGH_ALARM_LBYTE);
			read_data = read_data + IO_GPHYREG(PHY_CSR_I2C_DATA_PORT);
			transceiver_alarm->tx_power_high_alarm_thod= read_data;
		//tx power low alarm threshold
			io_trans_reg(PHY_TRANS_REG_TABLE2,PHY_TRANS_TX_LOW_ALARM_HBYTE);
			read_data = (IO_GPHYREG(PHY_CSR_I2C_DATA_PORT) << PHY_TRANS_OFFSET);
			io_trans_reg(PHY_TRANS_REG_TABLE2,PHY_TRANS_TX_LOW_ALARM_LBYTE);
			read_data = read_data + IO_GPHYREG(PHY_CSR_I2C_DATA_PORT);
			transceiver_alarm->tx_power_low_alarm_thod= read_data;
		#endif /* CONFIG_USE_MT7520_ASIC */
			PON_PHY_MSG(PHY_MSG_DBG, "rx power high alarm threshold: %.8x\n", transceiver_alarm->rx_power_high_alarm_thod);
			PON_PHY_MSG(PHY_MSG_DBG, "rx power low alarm threshold: %.8x\n", transceiver_alarm->rx_power_low_alarm_thod);
			PON_PHY_MSG(PHY_MSG_DBG, "tx power high alarm threshold: %.8x\n", transceiver_alarm->tx_power_high_alarm_thod);
			PON_PHY_MSG(PHY_MSG_DBG, "rx power low alarm threshold: %.8x\n", transceiver_alarm->tx_power_low_alarm_thod);
		#if 0
			i2c_protect = 0;
		}
		else
		{
			transceiver_alarm->rx_power_high_alarm_thod = 0;
			transceiver_alarm->rx_power_low_alarm_thod = 0;
			transceiver_alarm->tx_power_high_alarm_thod = 0;
			transceiver_alarm->tx_power_low_alarm_thod = 0;
		}
		#endif
}


/*****************************************************************************
//function :
//		phy_bit_delay
//description :
//		this function is used to set bit delay
//input :
//		delay_value : [3 bits] used to set bit delay value
//output :
//		setting success or failure
******************************************************************************/
int phy_bit_delay(unchar delay_value)
{
	uint write_data = 0;
	uint read_data = 0;
	if((delay_value & PHY_BIT_DELAY_LEN_MASK) == 0x00)
	{
#if defined(CONFIG_USE_A60901)
		/* This block can use for A60901 */
		read_data = IO_GPHYREG(PHY_BIT_DELAY_ADDR);//set bit delay
		PON_PHY_MSG(PHY_MSG_DBG, "bit delay read_data 0x%.8x\n", read_data );
		write_data = (read_data & PHY_DIG_BIT_DELAY_MASK) | (delay_value << PHY_DIG_BIT_DELAY_OFFSET);
		IO_SPHYREG(PHY_BIT_DELAY_ADDR,write_data);
		PON_PHY_MSG(PHY_MSG_DBG, "bit delay write_data 0x%.8x\n", write_data );
		read_data = IO_GPHYREG(PHY_CSR_PHYSET5);//set tx_en delay
		PON_PHY_MSG(PHY_MSG_DBG, "bit delay read_data 0x%.8x\n", read_data );
		write_data = (read_data & PHY_ANA_BIT_DELAY_MASK) | (delay_value << PHY_ANA_BIT_DELAY_OFFSET);
		IO_SPHYREG(PHY_CSR_PHYSET5,write_data);
		PON_PHY_MSG(PHY_MSG_DBG, "bit delay write_data 0x%.8x\n", write_data );
		return PHY_SUCCESS;
#elif defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
		/* This block can only use for MT7520 & 60928 */
		/* set bit delay & set tx_en delay */
		read_data = IO_GPHYREG(PHY_CSR_PHYSET5);
		write_data = (read_data & PHY_BIT_DELAY_MASK) | (delay_value << PHY_DIG_BIT_DELAY_OFFSET) | PHY_TX_BIT_DEL_SEL;
		IO_SPHYREG(PHY_CSR_PHYSET5,write_data);
		return PHY_SUCCESS;
#endif
	}
	else
	{
		PON_PHY_MSG(PHY_MSG_DBG, "error input : delay_value. \n");
		return PHY_FAILURE;
	}
}


/*****************************************************************************
//function :
//		phy_calibration_test
//description :
//		this function is used to test calibration
//input :
//		N/A
//output :
//		N/A
******************************************************************************/
void phy_calibration_test(void)
{
	int index = 0;
	uint read_data = 0;
	uint write_data = 0;
	for(index; index<100; index++)
	{
		read_data = IO_GPHYREG(PHY_CSR_PHYSET10);
		write_data = read_data | PHY_GPON_MODE;
		IO_SPHYREG(PHY_CSR_PHYSET10, write_data);
		read_data = IO_GPHYREG(PHY_CSR_PHYSET3);
		write_data = read_data | (PHY_PLL_RST) | (PHY_COUNT_RST);
		IO_SPHYREG(PHY_CSR_PHYSET3,write_data);
		IO_SPHYREG(PHY_CSR_PHYSET3,read_data);
		PON_PHY_MSG(PHY_MSG_ERR, "GPON calibration done. \n");
		read_data = IO_GPHYREG(PHY_CSR_ANACAL2);
		PON_PHY_MSG(PHY_MSG_ERR, "VCO calibration output : tx[15:8]: %.8x\n", (read_data & 0xff00)>>8 );
		PON_PHY_MSG(PHY_MSG_ERR, "VCO calibration output : rx[7:0]: %.8x\n", (read_data & 0xff) );
		PON_PHY_MSG(PHY_MSG_ERR, "PI calibration output : %.8x\n", (read_data & 0xff0000)>>16 );
		read_data = IO_GPHYREG(PHY_CSR_ANACAL1);
		PON_PHY_MSG(PHY_MSG_ERR, "imp calibration output : tx[4:0]: %.8x\n", (read_data & 0x1f));
		PON_PHY_MSG(PHY_MSG_ERR, "imp calibration output : rx[12:8]: %.8x\n", (read_data & 0x1f00)>>8 );
		PON_PHY_MSG(PHY_MSG_ERR, "SQTH calibration output[19:16]: %.8x\n", (read_data & 0xf0000)>>16 );
		#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
		read_data = IO_GPHYREG(PHY_CSR_PHYSAP);

		#endif /* CONFIG_USE_MT7520_ASIC */
		read_data = IO_GPHYREG(PHY_CSR_TDCSTA1);
		PON_PHY_MSG(PHY_MSG_ERR, "TDC calibration output : gain[10:0]: %.8x\n", (read_data & 0x7ff));
		PON_PHY_MSG(PHY_MSG_ERR, "TDC calibration output : offset[21:11]: %.8x\n", (read_data & 0x3ff800)>>11);
		msleep(1);
		read_data = IO_GPHYREG(PHY_CSR_PHYSET10);
		write_data = read_data & ~(PHY_GPON_MODE);
		IO_SPHYREG(PHY_CSR_PHYSET10, write_data);
		read_data = IO_GPHYREG(PHY_CSR_PHYSET3);
		write_data = read_data | (PHY_PLL_RST) | (PHY_COUNT_RST);
		IO_SPHYREG(PHY_CSR_PHYSET3,write_data);
		IO_SPHYREG(PHY_CSR_PHYSET3,read_data);
		PON_PHY_MSG(PHY_MSG_ERR, "EPON calibration done. \n");
		read_data = IO_GPHYREG(PHY_CSR_ANACAL2);
		PON_PHY_MSG(PHY_MSG_ERR, "VCO calibration output : tx[15:8]: %.8x\n", (read_data & 0xff00)>>8 );
		PON_PHY_MSG(PHY_MSG_ERR, "VCO calibration output : rx[7:0]: %.8x\n", (read_data & 0xff) );
		PON_PHY_MSG(PHY_MSG_ERR, "PI calibration output : %.8x\n", (read_data & 0xff0000)>>16 );
		read_data = IO_GPHYREG(PHY_CSR_ANACAL1);
		PON_PHY_MSG(PHY_MSG_ERR, "imp calibration output : tx[4:0]: %.8x\n", (read_data & 0x1f));
		PON_PHY_MSG(PHY_MSG_ERR, "imp calibration output : rx[12:8]: %.8x\n", (read_data & 0x1f00)>>8 );
		PON_PHY_MSG(PHY_MSG_ERR, "SQTH calibration output[19:16]: %.8x\n", (read_data & 0xf0000)>>16 );
		#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
		read_data = IO_GPHYREG(PHY_CSR_PHYSAP);
		PON_PHY_MSG(PHY_MSG_ERR, "snese amp calibration output : DCD[6:0]: %.8x\n", (read_data & 0x7f));
		PON_PHY_MSG(PHY_MSG_ERR, "snese amp calibration output : DCE[14:8]: %.8x\n", (read_data & 0x7f00)>>8);
		PON_PHY_MSG(PHY_MSG_ERR, "snese amp calibration output : DCEYE[22:16]: %.8x\n", (read_data & 0x7f0000)>>16);
		#endif /* CONFIG_USE_MT7520_ASIC */
		read_data = IO_GPHYREG(PHY_CSR_TDCSTA1);
		PON_PHY_MSG(PHY_MSG_ERR, "TDC calibration output : gain[10:0]: %.8x\n", (read_data & 0x7ff));
		PON_PHY_MSG(PHY_MSG_ERR, "TDC calibration output : offset[21:11]: %.8x\n", (read_data & 0x3ff800)>>11);
		msleep(1);
	}

}


void phy_pbus_stress_test(void)
{
	uint write_bank_data[5];
	uint read_bank_data[5] ;
	uint original[5];
	uint phyreg[5];

	original[0] = write_bank_data[0] = read_bank_data[0] = IO_GPHYREG(PHY_CSR_ANASET2);
	original[1] = write_bank_data[1] = read_bank_data[1] = IO_GPHYREG(PHY_CSR_EPON_SYNC_CTL);
	original[2] = write_bank_data[2] = read_bank_data[2] = IO_GPHYREG(PHY_CSR_MGMII_DELAY);
	original[3] = write_bank_data[3] = read_bank_data[3] = IO_GPHYREG(PHY_CSR_GPON_PREAMBLE);
	original[4] = write_bank_data[4] = read_bank_data[4] = IO_GPHYREG(PHY_CSR_EPON_PREAMBLE0);
	phyreg[0]=PHY_CSR_ANASET2;
	phyreg[1]=PHY_CSR_EPON_SYNC_CTL;
	phyreg[2]=PHY_CSR_MGMII_DELAY;
	phyreg[3]=PHY_CSR_GPON_PREAMBLE;
	phyreg[4]=PHY_CSR_EPON_PREAMBLE0;

	int index = 0;
	int read_write_index1 = 0;
	int read_write_index2 = 0;
	int bank_select1 = 0;
	int bank_select2 = 0;
	for(index; index < 100000 ; index++)
	{
		read_write_index1 = rand()%2;
		printk("test 1 = %d \n", read_write_index1);
		msleep(10);
		read_write_index2 = rand()%2;
		printk("test 2 = %d \n", read_write_index2);
		msleep(10);
		bank_select1 = rand()%5;
		printk("test 3 = %d \n", bank_select1);
		msleep(10);
		bank_select2 = rand()%5;
		printk("test 4 = %d \n", bank_select2);

		if(read_write_index1 == 0)
		{
			read_bank_data[bank_select1] = IO_GPHYREG(phyreg[bank_select1]);
		}
		else
		{
			IO_SPHYREG(phyreg[bank_select1], write_bank_data[bank_select1]);
		}
		if(read_write_index2 == 0)
		{
			read_bank_data[bank_select2] = IO_GPHYREG(phyreg[bank_select2]);
		}
		else
		{
			IO_SPHYREG(phyreg[bank_select2], write_bank_data[bank_select2]);
		}
	}
	if((original[0] == read_bank_data[0]) && (original[0] == write_bank_data[0]))
	{
		printk("bank 1 pass \n");
	}
	if((original[1] == read_bank_data[1]) && (original[1] == write_bank_data[1]))
	{
		printk("bank 2 pass \n");
	}
	if((original[2] == read_bank_data[2]) && (original[2] == write_bank_data[2]))
	{
		printk("bank 3 pass \n");
	}
	if((original[3] == read_bank_data[3]) && (original[3] == write_bank_data[3]))
	{
		printk("bank 4 pass \n");
	}
	if((original[4] == read_bank_data[4]) && (original[4] == write_bank_data[4]))
	{
		printk("bank 5 pass \n");
	}

}

/*****************************************************************************
//function :
//		phy_fw_ready
//description :
//		set fw ready to start HW function
//input :
//		fwrdy_en : PHY_ENABLE means fw is now ready ; PHY_DISABLE means fw is not ready
//output :
//		setting success or failure
******************************************************************************/
int phy_fw_ready(unchar fwrdy_en)
{
#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928) || defined(TCSUPPORT_LED_SWITCH_BUTTON)
	uint read_data = 0;
	uint write_data = 0;
	read_data = IO_GPHYREG(PHY_CSR_PHYSET2);
	if(fwrdy_en == PHY_ENABLE)
		write_data = read_data & (PHY_FW_RDY_MASK) | (PHY_FW_RDY_EN);
	else if(fwrdy_en == PHY_DISABLE)
		write_data = read_data & (PHY_FW_RDY_MASK);
	else
		return PHY_FAILURE;
	PON_PHY_MSG(PHY_MSG_DBG, "Phy_Fw_Ready function success. \n");

	IO_SPHYREG(PHY_CSR_PHYSET2,write_data);
#endif

	return PHY_SUCCESS;
}

#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928) || defined(TCSUPPORT_LED_SWITCH_BUTTON)
/*****************************************************************************
//function :
//		phy_freq_meter
//description :
//		this function is used to monitor frequency
//input :
//		freq_en : PHY_ENABLE is used to enable monitor function ; PHY_DISABLE is used to disable monitor function
//		freq_sele : [4 bits], used to select which clock to be monitored,
//output :
//		setting success or failure
******************************************************************************/
int phy_freq_meter(unchar freq_en, unchar freq_sele)
{
	uint read_data = 0;
	uint write_data = 0;
	if((freq_en == PHY_ENABLE))
	{
		if((freq_sele & PHY_FREQ_SELE_LEN_MASK) != 0x00)//check length
		{
			PON_PHY_MSG(PHY_MSG_ERR, "error input : freq_sele. \n");
			return PHY_FAILURE;
		}
		read_data = IO_GPHYREG(PHY_CSR_PHYSET3);
		write_data = read_data & ~(PHY_MON_PROB_CLK); //enable prob clk
		IO_SPHYREG(PHY_CSR_PHYSET3,write_data);
		read_data = IO_GPHYREG(PHY_CSR_PHYSET11);
		read_data = read_data & PHY_FREQ_SELE_MASK;
		write_data = read_data | (freq_sele << PHY_FREQ_METER_OFFSET); //select freq meter
		IO_SPHYREG(PHY_CSR_PHYSET11 ,write_data);
		write_data = read_data & ~(PHY_FREQ_METER_EN) | (PHY_FREQ_METER_EN) ; //enable freq meter
		IO_SPHYREG(PHY_CSR_PHYSET11,write_data);
		do// wait for ready
		{
			read_data = IO_GPHYREG(PHY_CSR_ANACAL4);
		}while( (read_data & PHY_FREQ_METER_DONE) == PHY_FREQ_METER_DONE); //|| ((read_data & PHY_FREQ_METER_FAIL) == PHY_FREQ_METER_FAIL));
		if((read_data & PHY_FREQ_METER_DONE) == PHY_FREQ_METER_DONE)
		{
			PON_PHY_MSG(PHY_MSG_DBG, "Phy Freq Meter enable success. \n");
			read_data = IO_GPHYREG(PHY_CSR_PHYSET11);
			read_data = read_data & (PHY_FREQ_METER_MASK);// only capture freq_meter
			PON_PHY_MSG(PHY_MSG_DBG, "freq meter result : 0x%.8x\n", read_data);
			return PHY_SUCCESS;
		}
		//else if((read_data & PHY_FREQ_METER_FAIL) == PHY_FREQ_METER_FAIL)
		//{
		//	PON_PHY_MSG(PHY_MSG_DBG, "Phy Freq Meter fail. \n");
		//	return PHY_SUCCESS;
		//}
		else
			return PHY_FAILURE;
	}
	else if((freq_en == PHY_DISABLE))
	{
		read_data = IO_GPHYREG(PHY_CSR_PHYSET3);
		write_data = read_data & ~(PHY_MON_PROB_CLK) | (PHY_MON_PROB_CLK); //disable prob clk
		IO_SPHYREG(PHY_CSR_PHYSET3,write_data);
		read_data = IO_GPHYREG(PHY_CSR_PHYSET11);
		write_data = read_data & ~(PHY_FREQ_METER_EN); //disable freq meter
		IO_SPHYREG(PHY_CSR_PHYSET11,write_data);
		PON_PHY_MSG(PHY_MSG_DBG, "Phy Freq Meter disable success. \n");
		return PHY_SUCCESS;
	}
	else
		return PHY_FAILURE;
}

/*****************************************************************************
//function :
//		 pon_phy_status
//description :
//		this function is used to monitor pon phy status
//input :
//		N/A
//output :
//		G/EPON MODE, FW RDY, PHY RDY and MT7570 version
******************************************************************************/
void pon_phy_status (void)
{
uint read_data = 0;
unchar ptr[4];
unchar vendor_id[PHY_TRANS_VENDOR_OFFSET+1];
unchar vendor_pn[PHY_TRANS_VENDOR_OFFSET+1];

		read_data = IO_GPHYREG(PHY_CSR_PHYSET10);	// read bit 31 ,check GEPON mode
		if ((read_data >>31) == 0x01 )
			{
			printk("GPON Mode \n");
			}
		else
			printk("EPON Mode \n");


		read_data = IO_GPHYREG(PHY_CSR_PHYSET2);	// read bit 0 ,check fw ready
		if ((read_data & 0x01) == PHY_FW_RDY_EN )
			{
			printk("FW Ready \n");
		 	}
		else
			printk("FW Not Ready \n");


		read_data = IO_GPHYREG(PHY_CSR_PHYSTA1);	//read bit[18:20] as 110, check phy ready
		if(((read_data >> PHYRDY_OFFSET) & PHYRDY_MASK) == PHYRDY_STATUS)
		{
			printk("PHY Ready \n");
		}
		else
		{
			printk("PHY Not Ready \n");
		}

		read_data = IO_GPHYREG(PHY_CSR_RX_STATUS);	//read bit[7:0] as 0xa, check RX sync
		if((read_data & 0xf) == 0xa)
		{
			printk("RX Sync OK \n");
		}
		else
		{
			printk("RX Not Sync\n");
		}

		printk("PON PHY driver version is %d.%d\n",phy_date_version,phy_version);
		printk("LOS status = %d\n", phy_los_status()); // check LOS

		if(mt7570_select == 1)
		{
			printk("MT7570 version is %d\n", mt7570_version);

			if ( internal_DDMI == 1 )
				printk("Internal DDMI Enabled\n");
			else
				printk("Internal DDMI Disabled\n");

			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_P0_PWR_CTRL_CS3	, ptr, 4);
			read_data = (ptr[0] & 0x02) >> 1 ;
			if (read_data == 1)
				printk("P0 under open loop mode \n");
			else
				printk("P0 under close loop mode \n");


			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_P1_PWR_CTRL_CS3 , ptr, 4);
			read_data = (ptr[0] & 0x02) >> 1 ;
			if (read_data == 1)
				printk("P1 under open loop mode \n");
			else
				printk("P1 under close loop mode \n");

		}
		else
		{
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE1, 1, PHY_TRANS_VENDOR_NAME , vendor_id, PHY_TRANS_VENDOR_OFFSET);//read 16byte once
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE1, 1, PHY_TRANS_VENDOR_PN , vendor_pn, PHY_TRANS_VENDOR_OFFSET);

			vendor_pn[PHY_TRANS_VENDOR_OFFSET]='\0';
			vendor_id[PHY_TRANS_VENDOR_OFFSET]='\0';

			printk("Vendor ID : %s\n", vendor_id);
			printk("Vendor PN : %s\n", vendor_pn);
		}
}
/*****************************************************************************
//function :
//		pon_register
//description :
//		this function is used to check phy register
//input :
//		none
//output :
//		pon register
******************************************************************************/
void pon_register (int regsiter)
{
	uint read_data = 0;
	read_data = IO_GPHYREG(regsiter);
	printk("0x%x = 0x%x \n",regsiter,read_data);

}



/*****************************************************************************
//function :
//		phy_eye_monitor
//description :
//		this function is used to enable eye monitor flow
//input :
//		eye_cnt : [16 bits] used to set how many counters to perform one sampling point
//output :
//		setting success or failure
******************************************************************************/
void phy_eye_monitor(ushort eye_cnt, unchar step, int offset)
{
	uint write_data = 0;
	uint read_data = 0;
	int x_axis, y_axis;
	int x_buffer = 0;
	pon_phy_deinit();
	read_data = IO_GPHYREG(PHY_CSR_PHYSET8);
	PON_PHY_MSG(PHY_MSG_DBG, "eye register 0x11C read_data :0x%.8x\n", read_data);
	read_data = (read_data & 0xffff0000) | eye_cnt;//set eye_cnt
	read_data = (read_data & ~(PHY_PILPO_ROUTE)) | PHY_PILPO_ROUTE;//set pilpo
	write_data = read_data;
	PON_PHY_MSG(PHY_MSG_DBG, "eye register 0x11C write_data :0x%.8x\n", write_data);
	IO_SPHYREG(PHY_CSR_PHYSET8,write_data);
	read_data = IO_GPHYREG(PHY_CSR_PHYSET5);
	PON_PHY_MSG(PHY_MSG_DBG, "eye register 0x110 read_data :0x%.8x\n", read_data);
	write_data = (read_data & ~(PHY_EYE_MON_ENABLE)) | (PHY_EYE_MON_ENABLE); //enable eye monitor
	msleep(1);
	PON_PHY_MSG(PHY_MSG_DBG, "eye register 0x110 write_data :0x%.8x\n", write_data);
	IO_SPHYREG(PHY_CSR_PHYSET5,write_data);
	//test
	for (x_axis = 64 ; x_axis > (0 - offset) ; x_axis = x_axis - step)
	{
		x_buffer = x_axis;
		if(x_axis < 0)
			x_buffer = x_buffer + 128;
		read_data = IO_GPHYREG(PHY_CSR_PHYSAP);
		PON_PHY_MSG(PHY_MSG_DBG, "eye register 0x12C read_data :0x%.8x\n", read_data);
		write_data = (read_data & 0x80ffffff) | (((x_buffer + 0x40) & 0x7f )<< 24);//setting x_axis of sampling point
		PON_PHY_MSG(PHY_MSG_DBG, "eye register 0x12C write_data :0x%.8x\n", write_data);
		IO_SPHYREG(PHY_CSR_PHYSAP,write_data);//set x axis
	}
	//
	int y_index = 0;
	int y_step = 0;
	for (x_axis = (0 - offset) ; x_axis<(128-offset) ; x_axis = x_axis + step)//for (x_axis = 0 ; x_axis<128; x_axis = x_axis + step)
	{
		x_buffer = x_axis;
		if(x_axis < 0)
			x_buffer = x_buffer + 128;
		if((x_buffer % 2) == 0)//set searching method
		{
			y_index = 0;
			y_step = 1*step;
		}
		else
		{
			y_index = 127;
			y_step = -1*step;
		}
		read_data = IO_GPHYREG(PHY_CSR_PHYSAP);
		PON_PHY_MSG(PHY_MSG_DBG, "eye register 0x12C read_data :0x%.8x\n", read_data);
		write_data = (read_data & 0x80ffffff) | (((x_buffer + 0x40) & 0x7f )<< 24);//setting x_axis of sampling point
		PON_PHY_MSG(PHY_MSG_DBG, "eye register 0x12C write_data :0x%.8x\n", write_data);
		IO_SPHYREG(PHY_CSR_PHYSAP,write_data);//set x axis
		for (y_axis = y_index ; (y_axis<128) && (y_axis>-1); y_axis = y_axis + y_step)
		{
			read_data = IO_GPHYREG(PHY_CSR_PHYSET7);
			PON_PHY_MSG(PHY_MSG_DBG, "eye register 0x118 read_data :0x%.8x\n", read_data);
			write_data = (read_data & 0xffffff80) | ((y_axis + 0x40) & 0x7f);
			schedule();
			PON_PHY_MSG(PHY_MSG_DBG, "eye register 0x118 write_data :0x%.8x\n", write_data);
			IO_SPHYREG(PHY_CSR_PHYSET7,write_data);//set y axis
			read_data = IO_GPHYREG(PHY_CSR_PHYSET5);
			PON_PHY_MSG(PHY_MSG_DBG, "eye register 0x110 read_data :0x%.8x\n", read_data);
			write_data = (read_data & ~(PHY_EYE_MON_CNT_ENABLE)) | (PHY_EYE_MON_CNT_ENABLE); //eye monitor counter enable
			//msleep(1);
			PON_PHY_MSG(PHY_MSG_DBG, "eye register 0x110 write_data :0x%.8x\n", write_data);
			IO_SPHYREG(PHY_CSR_PHYSET5,write_data);
			do// wait for ready
			{
				read_data = IO_GPHYREG(PHY_CSR_ANASTA1);
			}while((read_data & 0x00000400) != 0x400);
			read_data = IO_GPHYREG(PHY_CSR_ANACAL3);//read error cnt
			read_data = read_data & 0x000fffff;
			printk("%d %d %d \n",x_buffer ,y_axis, read_data);
			read_data = IO_GPHYREG(PHY_CSR_PHYSET5);
			PON_PHY_MSG(PHY_MSG_DBG, "eye register 0x110 read_data :0x%.8x\n", read_data);
			write_data = (read_data & ~(PHY_EYE_MON_CNT_ENABLE)); //eye monitor counter disable
			//msleep(1);
			PON_PHY_MSG(PHY_MSG_DBG, "eye register 0x110 write_data :0x%.8x\n", write_data);
			IO_SPHYREG(PHY_CSR_PHYSET5,write_data);
		}
	}
	read_data = IO_GPHYREG(PHY_CSR_PHYSET5);
	write_data = (read_data & ~(PHY_EYE_MON_ENABLE)); //disable eye monitor
	//test
	for (x_axis = (127-offset) ; x_axis > 64; x_axis = x_axis - step)
	{
		read_data = IO_GPHYREG(PHY_CSR_PHYSAP);
		PON_PHY_MSG(PHY_MSG_DBG, "eye register 0x12C read_data :0x%.8x\n", read_data);
		write_data = (read_data & 0x80ffffff) | (((x_axis + 0x40) & 0x7f )<< 24);//setting x_axis of sampling point
		PON_PHY_MSG(PHY_MSG_DBG, "eye register 0x12C write_data :0x%.8x\n", write_data);
		IO_SPHYREG(PHY_CSR_PHYSAP,write_data);//set x axis
	}
	//
	IO_SPHYREG(PHY_CSR_PHYSET5,write_data);
}
#endif /* CONFIG_USE_MT7520_ASIC */

/*****************************************************************************
******************************************************************************/
static int phy_rogue_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "%d\n", gpPhyPriv->phyCfg.flags.rogue);

	len -= off;
	*start = page + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;

}

unsigned int gphy_test_rx_ber = 0;


static int phy_info_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
	// Get PHY Status
	uint phy_rdy_val     = IO_GPHYREG(PHY_CSR_PHYSTA1);
	uint phy_los_val     = IO_GPHYREG(PHY_CSR_XPON_STA);
	uint phy_rx_sync_val = IO_GPHYREG(PHY_CSR_RX_STATUS);

	len = sprintf(buf, "\n");
	len += sprintf(buf+len, "%s\n", "**************************************");
	len += sprintf(buf+len, "%s\n", "**       XPON  PHY Status           **");
	len += sprintf(buf+len, "%s\n", "**************************************");
	len += sprintf(buf+len, "\n");


	if(!phy_los_val) {
		len += sprintf(buf+len, " PHY Status: plug   (PHY_RD: %#x <b[20:18]>   RX_SYNC: %#x <b[3:0]>)\n",  (((phy_rdy_val)&0x00180000)>>18)&0x7, (phy_rx_sync_val)&0xf);
	}else
		len += sprintf(buf+len, " PHY Status: unplug (PHY_RD: %#x <b[20:18]>   RX_SYNC: %#x <b[3:0]>)\n", (((phy_rdy_val)&0x00180000)>>18)&0x7, (phy_rx_sync_val)&0xf);

	len += sprintf((len+buf), "**************************************\n");
	len += sprintf((len+buf), "**             Raw data             **\n");
	len += sprintf((len+buf), "**************************************\n");


    len += sprintf((len+buf), " PHYSTA1          (0xBFAF0130): %#03x\n", phy_rdy_val);
	len += sprintf((len+buf), " XPON_STA         (0xBFAF05E0): %#03x\n", phy_los_val);
	len += sprintf((len+buf), " PHYRX_STATUS     (0xBFAF021C): %#03x\n", phy_rx_sync_val);

	len -= off;
	*start = buf + off;

	if(len > count)
		len = count;
	else
		*eof = 1;

	if(len < 0)
		len = 0;

	return len;
}



/*****************************************************************************
//function :
//		phy_read_proc
//description :
//		This function is to output EN7570 ADC codes for external DDMI usage
//		Type "cat /proc/pon_phy/debug" to get these codes
//input :
//		N/A
//output :
//		EN7570 ADC codes
******************************************************************************/
// static int phy_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
// {
// 	int index=0 ;
// 	PHY_TransParam_T phy_info;
// 	char buf_if[10] = {0};

// 	int temperature_code   = 0x0;
// 	int vcc_code           = 0x0;
// 	int bias_current       = 0x0;
// 	int modulation_current = 0x0;
// 	int rssi_current       = 0x0;
// 	int mpd_current        = 0x0;
// 	temperature_code   = mt7570_ADC_temperature_get();//Temperature code by HC
// 	vcc_code		   = mt7570_ADC_voltage_get();	//VCC code by HC
// 	bias_current	   = mt7570_information_output(Ibias);			//I_bias by HC
// 	modulation_current = mt7570_information_output(Imod);			//I_mod by HC
// 	rssi_current       = mt7570_RSSI_current();						//RSSI by HC
// 	mpd_current        = mt7570_MPD_current();						//MPD current by HC
// #if 0																//What is this? marked by HC 20150420
// /*	memset(&phy_info, 0, sizeof(PHY_TransParam_T));
// 	if(mt7570_select == 1)
// 		mt7570_param_status_real(&phy_info);
// 	else
// 		phy_trans_param_status_real(&phy_info);
// */
// 	buf_if[0] = 0x2;
// 	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x51, 1, 0x7f, buf_if, 1);
// 	memset(buf_if, 0, 10);
// 	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x51, 1, 0x9b, buf_if, 1);
// #endif
// 	off_t pos=0, begin=0 ;

// 	/* need to add the debug information */
// 	index += sprintf(buf+ index, "Temperature code: %x\n", temperature_code);		//Temperature code by HC
// 	index += sprintf(buf+ index, "VCC code: %x\n", vcc_code);						//VCC code by HC
// 	index += sprintf(buf+ index, "Bias Current: %x\n", bias_current);				//I_bias by HC
// 	index += sprintf(buf+ index, "Modulation Current: %x\n", modulation_current);	//I_mod by HC
// 	index += sprintf(buf+ index, "RSSI Current: %x\n", rssi_current);				//RSSI by HC
// 	index += sprintf(buf+ index, "MPD Current: %x\n", mpd_current); 				//MPD current by HC
// 	index += sprintf(buf+ index, "phy_i2c_div_clock: 0x%x\n", gpPhyPriv->i2c_u2_clk_div);
// 	CHK_BUF() ;

// 	*eof = 1 ;

// done:
// 	*start = buf + (off - begin) ;
// 	index -= (off - begin) ;
// 	if(index<0) 	index = 0 ;
// 	if(index>count) 	index = count ;

// 	return index ;
// }

/*****************************************************************************
******************************************************************************/
int xpon_phy_print_open=0;

// static int phy_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
// {
// 	char val_string[64], cmd[64] ,subcmd[64];
// 	uint value1, value2=0, value3 ,action;
// 	uint read_data = 0;
// 	memset(val_string,0,(sizeof(char)*64));
// 	memset(cmd,0,(sizeof(char)*64));
// 	memset(subcmd,0,(sizeof(char)*64));

// 	if (count > sizeof(val_string) - 1)
// 		return -EINVAL ;

// 	if (copy_from_user(val_string, buffer, count))
// 		return -EFAULT ;

// 	sscanf(val_string, "%s %x %x %x", cmd, &value1, &value2, &value3) ;
// 	sscanf(val_string, "%s %s %x", cmd, subcmd, &action) ;
// 	if(!strcmp(cmd, "Phy_Int_Config"))
// 	{
// 		phy_int_config(value1);
// 	}
// 	/*
// 	else if(!strcmp(cmd, "IO_GTransREG"))
// 	{
// 		io_trans_reg(value1,value2);
// 		read_data = IO_GPHYREG(PHY_CSR_I2C_DATA_PORT);
// 		PON_PHY_MSG(PHY_MSG_DBG, "transmitter Register Verify:%.8x\n", read_data);
// 	}
// 	*/
// 	else if(!strcmp(cmd, "show"))
// 	{
// 		if(!strcmp(subcmd, "xpon_print")) {
// 			xpon_phy_print_open = action;
// 		}
//         if(!strcmp(subcmd, "traffic_status")) {
// 			printk("========== now traffic status is %s ==============\r\n",(TRAFFIC_STATUS_DOWN == gpPhyPriv->phyCfg.flags.trafficStatus)? "TRAFFIC DOWN":"TRAFFIC UP");
// 		}
//     }
// 	else if(!strcmp(cmd, "Phy_Mode_Config"))
// 	{
// 		phy_mode_config(value1, PHY_ENABLE);
// 	}
// 	else if(!strcmp(cmd, "isPhyReady"))
// 	{
// 		phy_ready_status();
// 	}
// 	else if(!strcmp(cmd, "isPhyLOS"))
// 	{
// 		phy_los_status();
// 	}
// 	else if(!strcmp(cmd, "isPhyLOF"))
// 	{
// 		phy_lof_status();
// 	}
// 	else if(!strcmp(cmd, "Phy_Reset_Counter"))
// 	{
// 		phy_reset_counter();
// 	}
// 	else if(!strcmp(cmd, "Phy_ErrCnt_Enable"))
// 	{
// 		phy_cnt_enable(value1,value2,value3);
// 	}
// 	else if(!strcmp(cmd, "Phy_Bit_Delay"))
// 	{
// 		phy_bit_delay(value1);
// 	}
// 	else if(!strcmp(cmd, "Phy_TX_FEC_Status"))
// 	{
// 		phy_tx_fec_status();
// 	}
// 	else if(!strcmp(cmd, "Phy_Gpon_Delimiter_Guard"))
// 	{
// 		phy_gpon_delimiter_guard(value1, value2);
// 	}
// 	else if(!strcmp(cmd, "Phy_Tx_Burst_Config"))
// 	{
// 		phy_tx_burst_config(value1);
// 		}
//     else if(!strcmp(cmd, "Phy_Rogue_PRBS"))
//     {
//         phy_rogue_prbs_config(value1);
// 	}
// 	else if(!strcmp(cmd, "Phy_Tx_Amp_Setting"))
// 	{
// 		phy_tx_amp_setting(value1);
// 	}
// 	else if(!strcmp(cmd, "Phy_Trans_Tx_Setting"))
// 	{
// 		PHY_TransConfig_T para_test;
// 		PPHY_TransConfig_T p_para_test = & para_test;
// 		p_para_test->trans_tx_sd_inv_status = value1;
// 		p_para_test->trans_burst_en_inv_status = value2;
// 		p_para_test->trans_tx_fault_inv_status = value3;
// 		phy_trans_tx_setting(p_para_test);
// 	}
// 	else if(!strcmp(cmd, "Phy_Bip_Counter"))
// 	{
// 		phy_bip_counter();
// 	}
// 	else if(!strcmp(cmd, "Phy_RX_FEC_Setting"))
// 	{
// 		phy_rx_fec_setting(value1);
// 	}
// 	else if(!strcmp(cmd, "Phy_RX_FEC_Status"))
// 	{
// 		phy_rx_fec_status();
// 	}
// 	else if(!strcmp(cmd, "Phy_Trans_RX_Setting"))
// 	{
// 		phy_trans_rx_setting(value1);
// 	}
// 	else if(!strcmp(cmd, "Phy_Gpon_Preamble"))
// 	{
// 		PHY_GponPreb_T pon_preb;
// 		PPHY_GponPreb_T p_pon_preb = & pon_preb;
// 		p_pon_preb->mask = value1;
// 		p_pon_preb->guard_bit_num = 0x10;
// 		p_pon_preb->preamble_t1_num = 0x11;
// 		p_pon_preb->preamble_t2_num = 0x12;
// 		p_pon_preb->preamble_t3_pat = 0x13;
// 		phy_gpon_preamble(p_pon_preb);

// 	}
// 	else if(!strcmp(cmd, "Phy_Gpon_Extend_Preamble"))
// 	{
// 		PHY_GponPreb_T pon_preb;
// 		PPHY_GponPreb_T p_pon_preb = & pon_preb;
// 		p_pon_preb->mask = value1;
// 		p_pon_preb->t3_O4_preamble = 0x14;
// 		p_pon_preb->t3_O5_preamble = 0x15;
// 		p_pon_preb->extend_burst_mode = 0x01;
// 		p_pon_preb->oper_ranged_st = 0x01;
// 		p_pon_preb->dis_scramble = 0x01;
// 		phy_gpon_extend_preamble(p_pon_preb);

// 	}
// 	else if(!strcmp(cmd, "Phy_Fec_Counter"))
// 	{
// 		PHY_FecCount_T fec_count;
// 		PPHY_FecCount_T p_fec_count = & fec_count;
// 		phy_rx_fec_counter(p_fec_count);

// 	}
// 	else if(!strcmp(cmd, "Phy_Frame_Counter"))
// 	{
// 		PHY_FrameCount_T frame_cnt;
// 		PPHY_FrameCount_T p_frame_cnt = & frame_cnt;
// 		phy_rx_frame_counter(p_frame_cnt);

// 	}
// 	else if(!strcmp(cmd, "Phy_Trans_Alarm"))
// 	{
// 		phy_rx_power_alarm();
// 		phy_tx_alarm();
// 	}
// #if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
// 	else if(!strcmp(cmd, "Phy_Tx_Frame_Counter"))
// 	{
// 		phy_tx_frame_counter();
// 		printk("just test \n");
// 	}
// 	else if(!strcmp(cmd, "Phy_Tx_Burst_Counter"))
// 	{
// 		phy_tx_burst_counter();
// 		printk("just test \n");
// 	}
// #if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
// 	else if(!strcmp(cmd, "Phy_Epon_Frame_Err_Cnt"))
// 	{
// 		phy_epon_frame_err_cnt();
// 		printk("just test \n");
// 	}
// #endif
// 	else if(!strcmp(cmd, "Phy_Fw_Ready"))
// 	{
// 		phy_fw_ready(value1);
// 		printk("just test \n");
// 	}
// 	else if(!strcmp(cmd, "Phy_Freq_Meter"))
// 	{
// 		phy_freq_meter(value1,value2);
// 	}
// 	else if(!strcmp(cmd, "Phy_Byte_Delay_Manual"))
// 	{
// 		phy_byte_delay_manual(value1,value2);
// 	}
// 	else if(!strcmp(cmd, "Phy_Tx_Fec_Manual"))
// 	{
// 		phy_tx_fec_manual(value1);
// 	}
// 	else if(!strcmp(cmd, "eye_monitor"))
// 	{
// 		phy_eye_monitor(value1, value2, value3);
// 	}
// 	else if(!strcmp(cmd, "calibration"))
// 	{
// 		phy_calibration_test();
// 	}
// 	else if(!strcmp(cmd, "PBUS_test"))
// 	{
// 		phy_pbus_stress_test();
// 	}
// 	else if(!strcmp(cmd, "power_on"))
// 	{
// 		if(PHY_DISABLE == value1)
// 		{
// 			REPORT_EVENT_TO_MAC(PHY_EVENT_TX_POWER_OFF);
// 		}
// 		else if(PHY_ENABLE == value1)
// 		{
// 			REPORT_EVENT_TO_MAC(PHY_EVENT_TX_POWER_ON);
// 		}
// 		phy_tx_power_config(value1);
// 		phy_trans_power_switch(value1);
// 	}
// 	else if(!strcmp(cmd, "Phy_Tx_Test_Pattern"))
// 	{
// 		phy_tx_test_pattern(value1);
// 	}else if (!strcmp(cmd, "Phy_Rx_Sens_Test")){
// 		phy_rx_sensitivity_test();
// 	}
// 	else if (!strcmp(cmd, "7570_voltage")){
// 		mt7570_supply_voltage_get_8472();
// 	}
// 	else if (!strcmp(cmd, "7570_current")){
// 		mt7570_bias_current_get_8472();
// 	}
// 	else if (!strcmp(cmd, "7570_Vtemp_ADC")){		//by YM 20150714
// 		mt7570_Vtemp_ADC_get();
// 	}
// 	else if (!strcmp(cmd, "EnvTemp_cal")){			// by HC 20151006
// 		set_flash_register_EnvTemp(value1);
// 	}
// 	else if (!strcmp(cmd, "set_flash_register_Tx_data")){		// by HC 20151015
// 		if(en7571_select == 1)
// 			en7571_save_flash_Tx_data();
// 		else if(mt7570_select == 1)
// 		set_flash_register_Tx_data();
// 	}
// 	else if (!strcmp(cmd, "LUT")){						// by HC 20151014
// 		mt7570_show_LUT();
// 	}
// 	else if (!strcmp(cmd, "BiasTracking")){			// by HC 20161128
// 		mt7570_BiasTracking();
// 	}
// 	else if (!strcmp(cmd, "7570_temperature")){
// 		mt7570_temperature_get_8472();
// 	}
// 	else if (!strcmp(cmd, "set_flash_register")){
// 		set_flash_register(value1,value2);
// 	}
// 	else if (!strcmp(cmd, "save_flash_matrix")){
// 		save_flash_matrix();
// 	}
// 	else if (!strcmp(cmd, "get_flash_matrix")){
// 		get_flash_matrix();
// 	}
// 	else if (!strcmp(cmd, "get_flash_register")){
// 		get_flash_register(value1);
// 	}
// 	else if (!strcmp(cmd, "set_flash_register_default")){
// 		set_flash_register_default();
// 	}
// 	else if (!strcmp(cmd, "mt7570_information_output")){
// 		mt7570_information_output(value1);
// 	}
// 	else if (!strcmp(cmd, "mt7570_register_dump")){
// 		mt7570_register_dump(value1);
// 	}
// 	else if (!strcmp(cmd, "mt7570_MPD_current")){
// 		mt7570_MPD_current();
// 	}
// 	else if (!strcmp(cmd, "mt7570_RSSI_current")){
// 		mt7570_RSSI_current();
// 	}
// 	else if (!strcmp(cmd, "dark_current")){
// 		if(en7571_select == 1)
// 		printk("RSSI voltage = %x\n", en7571_dark_current() );
// 		else if(mt7570_select == 1)
// 		mt7570_dark_current();
// 	}
// 	else if (!strcmp(cmd, "flash_dump")){				//by HC 20150128
// 		flash_dump();
// 	}
// 	else if (!strcmp(cmd, "DOL_flash_dump")){			//by HC 20150128
// 		DOL_flash_dump();
// 	}
// 	else if (!strcmp(cmd, "GPON_Tx_calibration")){		//by HC 20150209
// 		GPON_Tx_calibration();
// 	}
// 	else if (!strcmp(cmd, "EPON_Tx_calibration")){		//by HC 20150226
// 		EPON_Tx_calibration();
// 	}
// 	else if (!strcmp(cmd, "GPON_Tx_cal_init")){		//by HC 20161206
// 		en7571_xPON_Tx_calibration(PHY_GPON_CONFIG);
// 	}
// 	else if (!strcmp(cmd, "EPON_Tx_cal_init")){		//by HC 20161206
// 		en7571_xPON_Tx_calibration(PHY_EPON_CONFIG);
// 	}
// 	else if (!strcmp(cmd, "restart_ERC")){				//by HC 20150210
// 		mt7570_restart_ERC();
// 	}
// 	else if (!strcmp(cmd, "LOS_calibration")){			//by HC 20150303
// 		if(en7571_select == 1)
// 			printk("LOS = %x\n", en7571_LOS_calibration(value1,value2) );
// 		else if(mt7570_select == 1)
// 		LOS_calibration(value1,value2);
// 	}
// 	else if (!strcmp(cmd, "set_flash_register_BiasModCurrent")){			//by HC 20150309
// 		set_flash_register_BiasModCurrent();
// 	}
// 	else if (!strcmp(cmd, "set_flash_register_P0P1")){						//by HC 20150309
// 		set_flash_register_P0P1();
// 	}
// 	else if (!strcmp(cmd, "set_flash_register_APD")){						//by HC 20150309
// 		if(en7571_select == 1)
// 			en7571_save_flash_APD(value1,value2,value3);
// 		else if(mt7570_select == 1)
// 		set_flash_register_APD(value1,value2,value3);
// 	}
// 	else if (!strcmp(cmd, "set_flash_register_LOS")){						//by HC 20150309
// 		if(en7571_select == 1)
// 			en7571_save_flash_LOS();
// 		else if(mt7570_select == 1)
// 		set_flash_register_LOS();
// 	}
// 	else if (!strcmp(cmd, "set_flash_register_DDMI_TxPower")){				//by HC 20150309
// 		if(en7571_select == 1)
// 			en7571_save_flash_DDMI_TxPower(value1,value2);
// 		else if(mt7570_select == 1)
// 		set_flash_register_DDMI_TxPower(value1,value2);
// 	}
// 	else if (!strcmp(cmd, "set_flash_register_DDMI_RxPower")){				//by HC 20150309
// 		if(en7571_select == 1)
// 			en7571_save_flash_DDMI_RxPower(value1,value2);
// 		else if(mt7570_select == 1)
// 		set_flash_register_DDMI_RxPower(value1,value2);
// 	}
// 	else if (!strcmp(cmd, "DDMI_check_8472")){								//by HC 20150311
// 		if(en7571_select == 1)
// 			en7571_DDMI_check_8472();
// 		else if(mt7570_select == 1)
// 		DDMI_check_8472();
// 	}
// 	else if (!strcmp(cmd, "set_flash_register_TIAGAIN")){					//by HC 20150320
// 		set_flash_register_TIAGAIN();
// 	}
// 	else if (!strcmp(cmd, "temperature_calibration")){						//by Biker 20150714
// 		set_flash_register_temperature_offset(value1,value2);
// 	}
// 	else if (!strcmp(cmd, "GPON_BER")){										//by HC 20150324
// 		GPON_BER(value1);
// 	}
// 	else if (!strcmp(cmd, "EPON_BER")){										//by HC 20150324
// 		EPON_BER(value1);
// 	}
// 	else if (!strcmp(cmd, "show_BoB_information")){							//by HC 20150325
// 		if(en7571_select == 1)
// 			en7571_BoB_info();
// 		else if(mt7570_select == 1)
// 		show_BoB_information();
// 	}
// 	else if (!strcmp(cmd, "GPON_pattern")){									//by HC 20150518
// 		if(en7571_select == 1)
// 			en7571_calibration_pattern(PHY_GPON_CONFIG);
// 		else if(mt7570_select == 1)
// 		mt7570_calibration_pattern(PHY_GPON_CONFIG);
// 	}
// 	else if (!strcmp(cmd, "EPON_pattern")){									//by HC 20150518
// 		if(en7571_select == 1)
// 			en7571_calibration_pattern(PHY_EPON_CONFIG);
// 		else if(mt7570_select == 1)
// 		mt7570_calibration_pattern(PHY_EPON_CONFIG);
// 	}
// 	else if (!strcmp(cmd, "dis_pattern")){									//by HC 20150518
// 		if(en7571_select == 1)
// 			en7571_disable_pattern();
// 		else if(mt7570_select == 1)
// 		mt7570_disable_pattern();
// 	}
// 	else if (!strcmp(cmd, "pon_phy_status")){ 								//by Jay 20150529
// 		pon_phy_status();
// 	}
// 	else if (!strcmp(cmd, "open_loop")){									//by HC 20150602
// 		mt7570_OpenLoopMode();
// 	}
// 	else if (!strcmp(cmd, "closed_loop")){									//by HC 20150602
// 		mt7570_DualClosedLoopMode();
// 	}
// 	else if (!strcmp(cmd, "sg_closed_loop")){								//by HC 20160325
// 		mt7570_SingleClosedLoopMode();
// 	}
// 	else if (!strcmp(cmd, "pon_register")){									//by Jay 20150826
// 		pon_register(value1);
// 	}
// 	else if (!strcmp(cmd, "err_cnt")){										//by HC 20161206
// 		pon_register(0xbfaf02c0);
// 	}
// 	else if (!strcmp(cmd, "Ibias_temperature_set")){								//by Jay 20150826
// 		Ibias_temperature_set();
// 	}
// 	else if (!strcmp(cmd, "restart_ERC_P0")){					//by Jayyc 20150902
// 		mt7570_restart_ERC_P0();
// 	}
// 	else if (!strcmp(cmd, "ADC_calibration")){					// by HC 20151027
// 		mt7570_ADC_calibration();
// 	}
// 	else if (!strcmp(cmd, "dis_ddmi")){							// by HC 20160517
// 		if(en7571_select == 1)
// 			en7571_internal_DDMI = 0;
// 		else if(mt7570_select == 1)
// 		internal_DDMI = 0;
// 		printk("DDMI disabled\n");
// 	}
// 	else if (!strcmp(cmd, "rebuild_LUT")){						// by HC 20160517
// 		mt7570_LUT_recover();
// 	}
// 	else if (!strcmp(cmd, "IAV")){									// by HC 20160705
// 		en7571_cal_Iav_Imod(value1,1);
// 	}
// 	else if (!strcmp(cmd, "IMOD")){									// by HC 20160705
// 		en7571_cal_Iav_Imod(value1,0);
// 	}
// 	else if (!strcmp(cmd, "PWR")){									// by HC 20160705
// 		en7571_cal_Pav_P1(value1,1);
// 	}
// 	else if (!strcmp(cmd, "ER")){									// by HC 20160705
// 		en7571_cal_Pav_P1(value1,0);
// 	}
// 	else if (!strcmp(cmd, "pwradc")){								// by HC 20160705
// 		printk("PWRADC = 0x%x\n", en7571_PWRADC_get());
// 	}
// 	else if (!strcmp(cmd, "PWR_up")){								//by HC 20160823
// 		en7571_fine_tune_PWR_ER(1,1);
// 	}
// 	else if (!strcmp(cmd, "PWR_dn")){								//by HC 20160823
// 		en7571_fine_tune_PWR_ER(1,0);
// 	}
// 	else if (!strcmp(cmd, "ER_up")){								//by HC 20160823
// 		en7571_fine_tune_PWR_ER(0,1);
// 	}
// 	else if (!strcmp(cmd, "ER_dn")){								//by HC 20160823
// 		en7571_fine_tune_PWR_ER(0,0);
// 	}
// 	else if (!strcmp(cmd, "APD_DAC")){							//by HC 20161208
// 		en7571_APD_DAC(value1);
// 	}
// 	else if (!strcmp(cmd, "VBR_find")){							//by YW 20161214
// 		en7571_VBR_find(value1,value2);
// 	}
// 	else if (!strcmp(cmd, "cal_LOS")){							//by YW 20161214
// 		en7571_LOS_find(0);
// 	}
// 	else if (!strcmp(cmd, "cal_SD")){								//by YW 20161214
// 		en7571_LOS_find(1);
// 	}
// 	else if (!strcmp(cmd, "ERC")){									//by HC 20170106
// 		en7571_show_ERC_ratio();
// 	}
// 	else if (!strcmp(cmd, "CDR")){									//by HC 20170203
// 		en7571_CDR(ENABLE);
// 	}
// 	else if (!strcmp(cmd, "KT")){									//by HC 20170306
// 		en7571_tune_KT(value1);
// 	}
// 	else if (!strcmp(cmd, "drop_test")){							//by HC 20170306
// 		en7571_cross_Imod(value1);
// 	}
// 	else if (!strcmp(cmd, "crs")){									//by HC 20170531
// 		en7571_cross = value1;
// 	}
// 	else if (!strcmp(cmd, "crs_xy")){									//by HC 20170531
// 		en7571_delay1 = value1;
// 		en7571_delay2 = value2;
// 	}
// 	else if (!strcmp(cmd, "show_crs")){							//by HC 20170531
// 		printk("crs enable = %d\n", en7571_cross);
// 		printk("crs cnt = %d\n", en7571_cross_cnt);
// 		printk("crs x = %d\n", en7571_delay1);
// 		printk("crs y = %d\n", en7571_delay2);
// 	}
// 	else if (!strcmp(cmd, "rssi")){								// by HC 20170607
// 		printk("RSSI = 0x%x\n", en7571_RSSI_get());
// 	}
// 	else if (!strcmp(cmd, "apd_ctrl")){								// by HC 20170609
// 		en7571_APD_control();
// 	}
// 	else if (!strcmp(cmd, "msg")) {											//by Wei.Sun 20150727
// 		int level = gpPhyPriv->debugLevel ;

// 		if(!strcmp(subcmd, "act")) {
// 			gpPhyPriv->debugLevel = (action) ? (level|PHY_MSG_ACT) : (level&~PHY_MSG_ACT) ;
// 		} else if(!strcmp(subcmd, "int")) {
// 			gpPhyPriv->debugLevel = (action) ? (level|PHY_MSG_INT) : (level&~PHY_MSG_INT) ;
// 		} else if(!strcmp(subcmd, "trace")) {
// 			gpPhyPriv->debugLevel = (action) ? (level|PHY_MSG_TRACE) : (level&~PHY_MSG_TRACE) ;
// 		} else if(!strcmp(subcmd, "debug")) {
// 			gpPhyPriv->debugLevel = (action) ? (level|PHY_MSG_DBG) : (level&~PHY_MSG_DBG) ;
// 		} else if(!strcmp(subcmd, "err")) {
// 			gpPhyPriv->debugLevel = (action) ? (level|PHY_MSG_ERR) : (level&~PHY_MSG_ERR) ;
// 		}
// 		printk("PHY Debug Level: %x\n", gpPhyPriv->debugLevel) ;
// 	}
// #endif /* CONFIG_USE_MT7520_ASIC */
// 	else if (!strcmp(cmd, "phy_i2c_div_clock")) {
// 		gpPhyPriv->i2c_u2_clk_div = value1;
// 	}
// 	else if (!strcmp(cmd, "a_tdc_setting")) {
// 		a_tdc_setting= value1;
// 		IO_SPHYREG(PHY_CSR_TDCSET2, a_tdc_setting);
// 		printk("a_tdc_setting=%x",a_tdc_setting);
// 	}
// 	else if(!strcmp(cmd, "Usage"))
// 	{
// 		printk("IO_GTransREG (device_addr) (sub_addr) \n");
// 		printk("Phy_Int_Config (interrupt) \n");
// 		printk("Phy_Mode_Config (XPON_mode) \n");
// 		printk("isPhyReady \n");
// 		printk("isPhyLOS \n");
// 		printk("isPhyLOF \n");
// 		printk("Phy_Reset_Counter \n");
// 		printk("Phy_ErrCnt_Enable (enable) (enable) (enable)\n");
// 		printk("Phy_Bit_Delay (delay_value) \n");
// 		printk("Phy_TX_FEC_Status \n");
// 		printk("Phy_Gpon_Delimiter_Guard (delimiter_pat) (guard_time_pat) \n");
// 		printk("Phy_Tx_Burst_Config (burst_mode) \n");
//         printk("Phy_Rogue_PRBS (rogue_mode) \n");
// 		printk("Phy_Tx_Amp_Setting (amp_level) \n");
// 		printk("Phy_Trans_Tx_Setting (enable) (enable) (enable)\n");
// 		printk("Phy_Bip_Counter \n");
// 		printk("Phy_RX_FEC_Setting (fec_control) \n");
// 		printk("Phy_RX_FEC_Status \n");
// 		printk("Phy_Trans_RX_Setting (enable) \n");
// 		printk("Trans_Param_Status \n");
// 		printk("Phy_Gpon_Preamble (maks) \n");
// 		printk("Phy_Gpon_Extend_Preamble (mask)\n");
// 		printk("Phy_Fec_Counter \n");
// 		printk("Phy_Frame_Counter \n");
// 		printk("Phy_Optical_LOS_Time \n");
// 		printk("Phy_Trans_Alarm \n");
// 		printk("Phy_Mode_Select \n");
// 		printk("Phy_Init_Polling \n");
// 		printk("Phy_Tx_Test_Pattern (pattern) \n");
// 		printk("msg (act|int|trace|debug|err) (enable) \n");
// #if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
// 		printk("Phy_Tx_Frame_Counter \n");
// 		printk("Phy_Tx_Burst_Counter \n");
// 		printk("Phy_Epon_Frame_Err_Cnt \n");
// 		printk("Phy_Fw_Ready (fw_rdy_en) \n");
// 		printk("Phy_Freq_Meter (freq_en) (freq_sele) \n");
// 		printk("Phy_Byte_Delay_Manual (byte_dly_en) (byte_dly_value) \n");
// 		printk("Phy_Tx_Fec_Manual (tx_fec_en) \n");
// 		printk("eye_monitor \n");
// 		printk("calibration \n");
// 		printk("PBUS_test \n");
// #endif /* CONFIG_USE_MT7520_ASIC */
// 		printk("phy_i2c_div_clock \n");
// 	}

// 	return count ;
// }


/*****************************************************************************
//function :
//		phy_proc_ddmi_read_proc
//description :
//		This function is to output EN7570 ADC codes for external DDMI usage
//		Type "cat /proc/pon_phy/ddmi" to get these codes
//input :
//		N/A
//output :
//		EN7570 ADC codes
******************************************************************************/
static int phy_proc_ddmi_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int index=0 ;
	char buf_if[10] = {0};

	int temperature_code   = 0x0;
	int vcc_code           = 0x0;
	int bias_current       = 0x0;
	int modulation_current = 0x0;
	int rssi_current       = 0x0;
	int mpd_current        = 0x0;
	temperature_code   = mt7570_ADC_temperature_get();//Temperature code by HC
	vcc_code		   = mt7570_ADC_voltage_get();	//VCC code by HC
	bias_current	   = mt7570_information_output(Ibias);		//I_bias by HC
	modulation_current = mt7570_information_output(Imod);		//I_mod by HC
	rssi_current       = mt7570_RSSI_current();						//RSSI by HC
	mpd_current        = mt7570_MPD_current();						//MPD current by HC

	off_t pos=0, begin=0 ;

	/*output format:TemperatureCode VCCCode BiasCurrent ModulationCurrent RSSICurrent MPDCurrent*/
	index += sprintf(buf+ index, "%x %x %x %x %x %x\n", temperature_code,vcc_code,bias_current,modulation_current,rssi_current,mpd_current);

	CHK_BUF() ;

	*eof = 1 ;

done:
	*start = buf + (off - begin) ;
	index -= (off - begin) ;
	if(index<0) 	index = 0 ;
	if(index>count) 	index = count ;

	return index ;
}

/*****************************************************************************
//function :
//		phy_proc_temperature_read_proc
//description :
//		This function is to output EN7570 temperature ADC code for external DDMI usage
//		Type "cat /proc/pon_phy/Temperature" to get this code
//input :
//		N/A
//output :
//		EN7570 ADC codes
******************************************************************************/
static int phy_proc_temperature_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int index=0 ;
	char buf_if[10] = {0};

	int temperature_code   = 0x0;
	temperature_code   = mt7570_ADC_temperature_get();//Temperature code by HC

	off_t pos=0, begin=0 ;

	index += sprintf(buf+ index, "%x\n", temperature_code);

	CHK_BUF() ;

	*eof = 1 ;

done:
	*start = buf + (off - begin) ;
	index -= (off - begin) ;
	if(index<0) 	index = 0 ;
	if(index>count) 	index = count ;

	return index ;
}


/*****************************************************************************
//function :
//		phy_proc_vcc_read_proc
//description :
//		This function is to output EN7570 VCC ADC code for external DDMI usage
//		Type "cat /proc/pon_phy/VCC" to get this code
//input :
//		N/A
//output :
//		EN7570 ADC codes
******************************************************************************/
static int phy_proc_vcc_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int index=0 ;
	char buf_if[10] = {0};

	int vcc_code           = 0x0;
	vcc_code		   = mt7570_ADC_voltage_get();	//VCC code by HC

	off_t pos=0, begin=0 ;

	index += sprintf(buf+ index, "%x\n", vcc_code);

	CHK_BUF() ;

	*eof = 1 ;

done:
	*start = buf + (off - begin) ;
	index -= (off - begin) ;
	if(index<0) 	index = 0 ;
	if(index>count) 	index = count ;

	return index ;
}


/*****************************************************************************
//function :
//		phy_proc_bias_read_proc
//description :
//		This function is to output EN7570 bias current ADC code for external DDMI usage
//		Type "cat /proc/pon_phy/BiasCurrent" to get this code
//input :
//		N/A
//output :
//		EN7570 ADC codes
******************************************************************************/
static int phy_proc_bias_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int index=0 ;
	char buf_if[10] = {0};

	int bias_current       = 0x0;
	bias_current	   = mt7570_information_output(Ibias);		//I_bias by HC

	off_t pos=0, begin=0 ;

	index += sprintf(buf+ index, "%x\n", bias_current);

	CHK_BUF() ;

	*eof = 1 ;

done:
	*start = buf + (off - begin) ;
	index -= (off - begin) ;
	if(index<0) 	index = 0 ;
	if(index>count) 	index = count ;

	return index ;
}


/*****************************************************************************
//function :
//		phy_proc_modulation_read_proc
//description :
//		This function is to output EN7570 modulation current ADC code for external DDMI usage
//		Type "cat /proc/pon_phy/ModulationCurrent" to get this code
//input :
//		N/A
//output :
//		EN7570 ADC codes
******************************************************************************/
static int phy_proc_modulation_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int index=0 ;
	char buf_if[10] = {0};

	int modulation_current = 0x0;
	modulation_current = mt7570_information_output(Imod);		//I_mod by HC

	off_t pos=0, begin=0 ;

	index += sprintf(buf+ index, "%x\n", modulation_current);

	CHK_BUF() ;

	*eof = 1 ;

done:
	*start = buf + (off - begin) ;
	index -= (off - begin) ;
	if(index<0) 	index = 0 ;
	if(index>count) 	index = count ;

	return index ;
}

/*****************************************************************************
//function :
//		phy_proc_rssi_read_proc
//description :
//		This function is to output EN7570 RSSI current ADC code for external DDMI usage
//		Type "cat /proc/pon_phy/RSSICurrent" to get this code
//input :
//		N/A
//output :
//		EN7570 ADC codes
******************************************************************************/
static int phy_proc_rssi_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int index=0 ;
	char buf_if[10] = {0};

	int rssi_current       = 0x0;
	rssi_current       = mt7570_RSSI_current();						//RSSI by HC

	off_t pos=0, begin=0 ;

	index += sprintf(buf+ index, "%x\n", rssi_current);

	CHK_BUF() ;

	*eof = 1 ;

done:
	*start = buf + (off - begin) ;
	index -= (off - begin) ;
	if(index<0) 	index = 0 ;
	if(index>count) 	index = count ;

	return index ;
}



/*****************************************************************************
//function :
//		phy_proc_mpd_read_proc
//description :
//		This function is to output EN7570 MPD current ADC code for external DDMI usage
//		Type "cat /proc/pon_phy/MPDCurrent" to get this code
//input :
//		N/A
//output :
//		EN7570 ADC codes
******************************************************************************/
static int phy_proc_mpd_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int index=0 ;
	char buf_if[10] = {0};

	int mpd_current        = 0x0;
	mpd_current        = mt7570_MPD_current();						//MPD current by HC

	off_t pos=0, begin=0 ;

	index += sprintf(buf+ index, "%x\n", mpd_current);

	CHK_BUF() ;

	*eof = 1 ;

done:
	*start = buf + (off - begin) ;
	index -= (off - begin) ;
	if(index<0) 	index = 0 ;
	if(index>count) 	index = count ;

	return index ;
}

static int phy_proc_apd_control_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)	//For ASB 20150310
{
	char val_string[64] ;
	unchar ptr[1];
	int apd_value;

	if (count > sizeof(val_string) - 1)
		return -EINVAL ;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT ;

	sscanf(val_string, "%x", &apd_value) ;
	ptr[0] = apd_value;
	mt7570_APD_I2C_write(ptr);
	return count ;
}



static struct proc_dir_entry *phy_proc_dir=NULL, *phy_proc=NULL, *phy_rogue=NULL, *phy_info=NULL;
static struct proc_dir_entry *phy_proc_ddmi=NULL;
static struct proc_dir_entry *phy_proc_temperature=NULL;
static struct proc_dir_entry *phy_proc_vcc=NULL;
static struct proc_dir_entry *phy_proc_bias=NULL;
static struct proc_dir_entry *phy_proc_modulation=NULL;
static struct proc_dir_entry *phy_proc_rssi=NULL;
static struct proc_dir_entry *phy_proc_mpd=NULL;
static struct proc_dir_entry *phy_proc_apd_control=NULL;


/*****************************************************************************
******************************************************************************/
int phy_debug_init(void)
{

	/* create proc node */
	phy_proc_dir = proc_mkdir("pon_phy", NULL);
	if(phy_proc_dir){
#ifdef CONFIG_DEBUG
		phy_proc = create_proc_entry("debug", 0, phy_proc_dir);
	    if(phy_proc) {
		    phy_proc->write_proc = phy_write_proc;
		    phy_proc->read_proc = phy_read_proc;
	    }
#endif /* CONFIG_DEBUG */

		phy_info = create_proc_entry("info", 0, phy_proc_dir);
	    if(phy_info) {
            phy_info->write_proc= NULL;
		    phy_info->read_proc = phy_info_read_proc;
	    }

		phy_proc_ddmi = create_proc_entry("ddmi", 0, phy_proc_dir);
		if(phy_proc_ddmi) {
			phy_proc_ddmi->read_proc =phy_proc_ddmi_read_proc;
		}
		phy_proc_temperature = create_proc_entry("Temperature", 0, phy_proc_dir);
		if(phy_proc_temperature) {
			phy_proc_temperature->read_proc =phy_proc_temperature_read_proc;
		}
		phy_proc_vcc = create_proc_entry("VCC", 0, phy_proc_dir);
		if(phy_proc_vcc) {
			phy_proc_vcc->read_proc =phy_proc_vcc_read_proc;
		}
		phy_proc_bias = create_proc_entry("BiasCurrent", 0, phy_proc_dir);
		if(phy_proc_bias) {
			phy_proc_bias->read_proc =phy_proc_bias_read_proc;
		}
		phy_proc_modulation = create_proc_entry("ModulationCurrent", 0, phy_proc_dir);
		if(phy_proc_modulation) {
			phy_proc_modulation->read_proc =phy_proc_modulation_read_proc;
		}
		phy_proc_rssi = create_proc_entry("RSSICurrent", 0, phy_proc_dir);
		if(phy_proc_rssi) {
			phy_proc_rssi->read_proc =phy_proc_rssi_read_proc;
		}
		phy_proc_mpd = create_proc_entry("MPDCurrent", 0, phy_proc_dir);
		if(phy_proc_mpd) {
			phy_proc_mpd->read_proc =phy_proc_mpd_read_proc;
		}
		phy_proc_apd_control = create_proc_entry("APDVolControl", 0, phy_proc_dir);
		if(phy_proc_apd_control) {
			phy_proc_apd_control->write_proc = phy_proc_apd_control_write_proc;
		}
        phy_rogue = create_proc_entry("rogue", 0, phy_proc_dir);
	    if(phy_rogue) {
		    phy_rogue->read_proc = phy_rogue_read_proc;
	    }
	}

	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int phy_power(unchar direction)
{
	return 0;
}

int
phy_debug_deinit(void){
	if(phy_rogue){
		remove_proc_entry("rogue", phy_proc_dir);
	}
#ifdef CONFIG_DEBUG
	if(phy_proc){
		remove_proc_entry("debug", phy_proc_dir);
	}

#endif

	if (phy_info)
	{
		remove_proc_entry("info", phy_proc_dir);
	}

	if (phy_proc_ddmi)
	{
		 remove_proc_entry("ddmi", phy_proc_dir);
	}

	if (phy_proc_temperature)
	{
		remove_proc_entry("Temperature",phy_proc_dir);
	}

	if (phy_proc_vcc)
	{
		remove_proc_entry("VCC",phy_proc_dir);
	}

	if (phy_proc_bias)
	{
		remove_proc_entry("BiasCurrent",phy_proc_dir);
	}

	if (phy_proc_modulation)
	{
		remove_proc_entry("ModulationCurrent",phy_proc_dir);
	}

	if (phy_proc_rssi)
	{
		remove_proc_entry("RSSICurrent",phy_proc_dir);
	}

	if (phy_proc_mpd)
	{
		remove_proc_entry("MPDCurrent",phy_proc_dir);
	}

	if (phy_proc_apd_control)
	{
		remove_proc_entry("APDVolControl",phy_proc_dir);
	}

    remove_proc_entry("pon_phy",NULL);
	return 0 ;
}

void phy_tx_power_config(unchar enable){
	gpPhyPriv->phyCfg.flags.txPowerEnFlag = enable;
	PON_PHY_MSG(PHY_MSG_DBG, "PON PHY set txPowerEnFlag to %d.\n", enable);
}


/*****************************************************************************
 to determine if there is optical signal
******************************************************************************/


/*****************************************************************************
//Function :
//		phy_tx_fault_reset
//Description :
//		This function is to reset Tx_fault
//Input :
//		N/A
//Output :
//		N/A
//Date:
//		2015/02/03 by YMC
//
******************************************************************************/

void phy_tx_fault_reset(void)
{
	if(en7571_select==1)
		en7571_safe_circuit_reset();
	else if(mt7570_select==1)
		mt7570_safe_circuit_reset();
}

void phy_event_poll(ulong data)
{
    ulong flags;
    PON_PHY_Event_data_t phy_event = {.id = 0, .src = PON_PHY_EVENT_SOURCE_SW_POLL};

    if(FALSE == gpPhyPriv->is_phy_start){
        return;
    }

    spin_lock_irqsave(&gpPhyPriv->event_handle_lock, flags);
    if(PHY_LINK_STATUS_READY == gpPhyPriv->phy_status)
    {
        if(!is_phy_sync())  /* Ready to Los */
        {
            XPON_DPRINT_MSG("los detected by polling\n");
            gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
            XPON_DPRINT_MSG("[%s] event los polled\n", __FUNCTION__);
            phy_event.id = PHY_EVENT_TRANS_LOS_INT;
            phy_event_handler(&phy_event);
			PHY_START_TIMER(gpPhyPriv->phy_ready_timer) ;
        }
    }
	else
    {
        if(is_phy_sync())  /* Unknown or Los to Ready */
        {
            XPON_DPRINT_MSG("ready detected by polling\n");
            gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
            XPON_DPRINT_MSG("[%s] event ready polled\n", __FUNCTION__);
            phy_event.id = PHY_EVENT_PHYRDY_INT;
            phy_event_handler(&phy_event);
			PHY_STOP_TIMER(gpPhyPriv->phy_ready_timer) ;
        }
    }

    spin_unlock_irqrestore(&gpPhyPriv->event_handle_lock, flags);

    PHY_START_TIMER(gpPhyPriv->event_poll_timer) ;
}

void phy_event_handler(PON_PHY_Event_data_t * pEvent_data)
{
    XPON_DPRINT_MSG("[%s] event:%d, src:%d\n", __FUNCTION__, pEvent_data->id, pEvent_data->src);

    switch(pEvent_data->id)
    {
        case PHY_EVENT_TRANS_LOS_INT      :
        case PHY_EVENT_PHY_ILLG_INT       :
        case PHY_EVENT_TRANS_LOS_ILLG_INT :
            phy_los_handler() ;
            break;

        case PHY_EVENT_PHY_LOF_INT:
            break;

        case PHY_EVENT_TF_INT:
            /*phy_trans_power_switch(PHY_DISABLE);*/
            break;

        case PHY_EVENT_TRANS_INT:
            break;

        case PHY_EVENT_TRANS_SD_FAIL_INT:
            break;

        case PHY_EVENT_PHYRDY_INT:
            phy_ready_handler() ;
            break;

        case PHY_EVENT_I2CM_INT:
            break;

        default:
            printk("[%s] event %d still left for ISR to handle\n", __FUNCTION__, pEvent_data->id );
            break;
	}

    XPON_MAC_EVENT_HANDLER(pEvent_data);
}


