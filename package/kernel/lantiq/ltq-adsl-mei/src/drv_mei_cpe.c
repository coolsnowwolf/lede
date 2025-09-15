/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/*!
  \defgroup AMAZON_S_MEI Amazon-S MEI Driver Module
  \brief Amazon-S MEI driver module
 */

/*!
  \defgroup Internal Compile Parametere
  \ingroup AMAZON_S_MEI
  \brief exported functions for other driver use
 */

/*!
  \file amazon_s_mei_bsp.c
  \ingroup AMAZON_S_MEI
  \brief Amazon-S MEI driver file
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <generated/utsrelease.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <asm/hardirq.h>

#include "lantiq_atm.h"
#include <lantiq_soc.h>
//#include "ifxmips_atm.h"
#define IFX_MEI_BSP
#include "ifxmips_mei_interface.h"

/*#define LTQ_RCU_RST                   IFX_RCU_RST_REQ
#define LTQ_RCU_RST_REQ_ARC_JTAG      IFX_RCU_RST_REQ_ARC_JTAG
#define LTQ_RCU_RST_REQ_DFE		  IFX_RCU_RST_REQ_DFE
#define LTQ_RCU_RST_REQ_AFE		  IFX_RCU_RST_REQ_AFE
#define IFXMIPS_FUSE_BASE_ADDR            IFX_FUSE_BASE_ADDR
#define IFXMIPS_ICU_IM0_IER               IFX_ICU_IM0_IER
#define IFXMIPS_ICU_IM2_IER               IFX_ICU_IM2_IER
#define LTQ_MEI_INT                   IFX_MEI_INT
#define LTQ_MEI_DYING_GASP_INT        IFX_MEI_DYING_GASP_INT
#define LTQ_MEI_BASE_ADDR  		  IFX_MEI_SPACE_ACCESS
#define IFXMIPS_PMU_PWDCR		  IFX_PMU_PWDCR
#define IFXMIPS_MPS_CHIPID                IFX_MPS_CHIPID

#define ifxmips_port_reserve_pin 	  ifx_gpio_pin_reserve
#define ifxmips_port_set_dir_in		  ifx_gpio_dir_in_set
#define ifxmips_port_clear_altsel0        ifx_gpio_altsel0_set
#define ifxmips_port_clear_altsel1 	  ifx_gpio_altsel1_clear
#define ifxmips_port_set_open_drain       ifx_gpio_open_drain_clear
#define ifxmips_port_free_pin		  ifx_gpio_pin_free
#define ifxmips_mask_and_ack_irq	  bsp_mask_and_ack_irq
#define IFXMIPS_MPS_CHIPID_VERSION_GET    IFX_MCD_CHIPID_VERSION_GET
#define ltq_r32(reg)                        __raw_readl(reg)
#define ltq_w32(val, reg)                   __raw_writel(val, reg)
#define ltq_w32_mask(clear, set, reg)       ltq_w32((ltq_r32(reg) & ~clear) | set, reg)
*/

#define LTQ_RCU_BASE_ADDR       0x1F203000
#define LTQ_ICU_BASE_ADDR       0x1F880200
#define LTQ_MEI_BASE_ADDR       0x1E116000
#define LTQ_PMU_BASE_ADDR       0x1F102000


#ifdef CONFIG_DANUBE
# define LTQ_MEI_INT             (INT_NUM_IM1_IRL0 + 23)
# define LTQ_MEI_DYING_GASP_INT  (INT_NUM_IM1_IRL0 + 21)
# define LTQ_USB_OC_INT          (INT_NUM_IM4_IRL0 + 23)
#endif

#ifdef CONFIG_AMAZON_SE
# define LTQ_MEI_INT             (INT_NUM_IM2_IRL0 + 9)
# define LTQ_MEI_DYING_GASP_INT  (INT_NUM_IM2_IRL0 + 11)
# define LTQ_USB_OC_INT          (INT_NUM_IM2_IRL0 + 20)
#endif

#ifdef CONFIG_AR9
# define LTQ_MEI_INT             (INT_NUM_IM1_IRL0 + 23)
# define LTQ_MEI_DYING_GASP_INT  (INT_NUM_IM1_IRL0 + 21)
# define LTQ_USB_OC_INT          (INT_NUM_IM1_IRL0 + 28)
#endif

#ifndef LTQ_MEI_INT
#error "Unknown Lantiq ARCH!"
#endif

#define LTQ_RCU_RST_REQ_DFE		(1 << 7)
#define LTQ_RCU_RST_REQ_AFE		(1 << 11)

#define LTQ_PMU_BASE		(KSEG1 + LTQ_PMU_BASE_ADDR)
#define LTQ_RCU_BASE		(KSEG1 + LTQ_RCU_BASE_ADDR)
#define LTQ_ICU_BASE		(KSEG1 + LTQ_ICU_BASE_ADDR)

#define LTQ_PMU_PWDCR        ((u32 *)(LTQ_PMU_BASE + 0x001C))
#define LTQ_PMU_PWDSR        ((u32 *)(LTQ_PMU_BASE + 0x0020))
#define LTQ_RCU_RST          ((u32 *)(LTQ_RCU_BASE + 0x0010))
#define LTQ_RCU_RST_ALL      0x40000000

#define LTQ_ICU_IM0_ISR      ((u32 *)(LTQ_ICU_BASE + 0x0000))
#define LTQ_ICU_IM0_IER      ((u32 *)(LTQ_ICU_BASE + 0x0008))
#define LTQ_ICU_IM0_IOSR     ((u32 *)(LTQ_ICU_BASE + 0x0010))
#define LTQ_ICU_IM0_IRSR     ((u32 *)(LTQ_ICU_BASE + 0x0018))
#define LTQ_ICU_IM0_IMR      ((u32 *)(LTQ_ICU_BASE + 0x0020))


#define LTQ_ICU_IM1_ISR      ((u32 *)(LTQ_ICU_BASE + 0x0028))
#define LTQ_ICU_IM2_ISR      ((u32 *)(LTQ_ICU_BASE + 0x0050))
#define LTQ_ICU_IM3_ISR      ((u32 *)(LTQ_ICU_BASE + 0x0078))
#define LTQ_ICU_IM4_ISR      ((u32 *)(LTQ_ICU_BASE + 0x00A0))

#define LTQ_ICU_OFFSET       (LTQ_ICU_IM1_ISR - LTQ_ICU_IM0_ISR)
#define LTQ_ICU_IM2_IER		(LTQ_ICU_IM0_IER + LTQ_ICU_OFFSET)

#define IFX_MEI_EMSG(fmt, args...) pr_err("[%s %d]: " fmt,__FUNCTION__, __LINE__, ## args)
#define IFX_MEI_DMSG(fmt, args...) pr_debug("[%s %d]: " fmt,__FUNCTION__, __LINE__, ## args)

#define LTQ_FUSE_BASE          (KSEG1 + 0x1F107354)

#ifdef CONFIG_LTQ_MEI_FW_LOOPBACK
//#define DFE_MEM_TEST
//#define DFE_PING_TEST
#define DFE_ATM_LOOPBACK


#ifdef DFE_ATM_LOOPBACK
#include <asm/ifxmips/ifxmips_mei_fw_loopback.h>
#endif

void dfe_loopback_irq_handler (DSL_DEV_Device_t *pDev);

#endif //CONFIG_AMAZON_S_MEI_FW_LOOPBACK

DSL_DEV_Version_t bsp_mei_version = {
	major:	5,
	minor:	0,
	revision:0
};
DSL_DEV_HwVersion_t bsp_chip_info;

#define IFX_MEI_DEVNAME "ifx_mei"
#define BSP_MAX_DEVICES 1
#define MEI_DIRNAME "ifxmips_mei"

DSL_DEV_MeiError_t DSL_BSP_FWDownload (DSL_DEV_Device_t *, const char *, unsigned long, long *, long *);
DSL_DEV_MeiError_t DSL_BSP_Showtime (DSL_DEV_Device_t *, DSL_uint32_t, DSL_uint32_t);
DSL_DEV_MeiError_t DSL_BSP_AdslLedInit (DSL_DEV_Device_t *, DSL_DEV_LedId_t, DSL_DEV_LedType_t, DSL_DEV_LedHandler_t);
//DSL_DEV_MeiError_t DSL_BSP_AdslLedSet (DSL_DEV_Device_t *, DSL_DEV_LedId_t, DSL_DEV_LedMode_t);
DSL_DEV_MeiError_t DSL_BSP_MemoryDebugAccess (DSL_DEV_Device_t *, DSL_BSP_MemoryAccessType_t, DSL_uint32_t, DSL_uint32_t*, DSL_uint32_t);
DSL_DEV_MeiError_t DSL_BSP_SendCMV (DSL_DEV_Device_t *, u16 *, int, u16 *);

int DSL_BSP_KernelIoctls (DSL_DEV_Device_t *, unsigned int, unsigned long);

static DSL_DEV_MeiError_t IFX_MEI_RunAdslModem (DSL_DEV_Device_t *);
static DSL_DEV_MeiError_t IFX_MEI_CpuModeSet (DSL_DEV_Device_t *, DSL_DEV_CpuMode_t);
static DSL_DEV_MeiError_t IFX_MEI_DownloadBootCode (DSL_DEV_Device_t *);
static DSL_DEV_MeiError_t IFX_MEI_ArcJtagEnable (DSL_DEV_Device_t *, int);
static DSL_DEV_MeiError_t IFX_MEI_AdslMailboxIRQEnable (DSL_DEV_Device_t *, int);

static int IFX_MEI_GetPage (DSL_DEV_Device_t *, u32, u32, u32, u32 *, u32 *);
static int IFX_MEI_BarUpdate (DSL_DEV_Device_t *, int);

static ssize_t IFX_MEI_Write (DSL_DRV_file_t *, const char *, size_t, loff_t *);
static long IFX_MEI_UserIoctls (DSL_DRV_file_t *, unsigned int, unsigned long);
static int IFX_MEI_Open (DSL_DRV_inode_t *, DSL_DRV_file_t *);
static int IFX_MEI_Release (DSL_DRV_inode_t *, DSL_DRV_file_t *);

void AMAZON_SE_MEI_ARC_MUX_Test(void);

void IFX_MEI_ARC_MUX_Test(void);

static int adsl_dummy_ledcallback(void);

int (*ifx_mei_atm_showtime_enter)(struct port_cell_info *, void *) = NULL;
EXPORT_SYMBOL(ifx_mei_atm_showtime_enter);

int (*ifx_mei_atm_showtime_exit)(void) = NULL;
EXPORT_SYMBOL(ifx_mei_atm_showtime_exit);

static int (*g_adsl_ledcallback)(void) = adsl_dummy_ledcallback;

static unsigned int g_tx_link_rate[2] = {0};

static void *g_xdata_addr = NULL;

static u32 *mei_arc_swap_buff = NULL;	//  holding swap pages

extern void ltq_mask_and_ack_irq(struct irq_data *d);
static void inline MEI_MASK_AND_ACK_IRQ(int x)
{
	struct irq_data d;
	d.hwirq = x;
	ltq_mask_and_ack_irq(&d);
}
#define MEI_MAJOR	105
static int dev_major = MEI_MAJOR;

static struct file_operations bsp_mei_operations = {
      owner:THIS_MODULE,
      open:IFX_MEI_Open,
      release:IFX_MEI_Release,
      write:IFX_MEI_Write,
      unlocked_ioctl:IFX_MEI_UserIoctls,
};

static DSL_DEV_Device_t dsl_devices[BSP_MAX_DEVICES];

static ifx_mei_device_private_t
	sDanube_Mei_Private[BSP_MAX_DEVICES];

static DSL_BSP_EventCallBack_t dsl_bsp_event_callback[DSL_BSP_CB_LAST + 1];

/**
 * Write a value to register
 * This function writes a value to danube register
 *
 * \param  	ul_address	The address to write
 * \param  	ul_data		The value to write
 * \ingroup	Internal
 */
static void
IFX_MEI_LongWordWrite (u32 ul_address, u32 ul_data)
{
	IFX_MEI_WRITE_REGISTER_L (ul_data, ul_address);
	wmb();
	return;
}

/**
 * Write a value to register
 * This function writes a value to danube register
 *
 * \param 	pDev		the device pointer
 * \param  	ul_address	The address to write
 * \param  	ul_data		The value to write
 * \ingroup	Internal
 */
static void
IFX_MEI_LongWordWriteOffset (DSL_DEV_Device_t * pDev, u32 ul_address,
				   u32 ul_data)
{
	IFX_MEI_WRITE_REGISTER_L (ul_data, pDev->base_address + ul_address);
	wmb();
	return;
}

/**
 * Read the danube register
 * This function read the value from danube register
 *
 * \param  	ul_address	The address to write
 * \param  	pul_data	Pointer to the data
 * \ingroup	Internal
 */
static void
IFX_MEI_LongWordRead (u32 ul_address, u32 * pul_data)
{
	*pul_data = IFX_MEI_READ_REGISTER_L (ul_address);
	rmb();
	return;
}

/**
 * Read the danube register
 * This function read the value from danube register
 *
 * \param 	pDev		the device pointer
 * \param  	ul_address	The address to write
 * \param  	pul_data	Pointer to the data
 * \ingroup	Internal
 */
static void
IFX_MEI_LongWordReadOffset (DSL_DEV_Device_t * pDev, u32 ul_address,
				  u32 * pul_data)
{
	*pul_data = IFX_MEI_READ_REGISTER_L (pDev->base_address + ul_address);
	rmb();
	return;
}

/**
 * Write several DWORD datas to ARC memory via ARC DMA interface
 * This function writes several DWORD datas to ARC memory via DMA interface.
 *
 * \param 	pDev		the device pointer
 * \param  	destaddr	The address to write
 * \param  	databuff	Pointer to the data buffer
 * \param  	databuffsize	Number of DWORDs to write
 * \return	DSL_DEV_MEI_ERR_SUCCESS or DSL_DEV_MEI_ERR_FAILURE
 * \ingroup	Internal
 */
static DSL_DEV_MeiError_t
IFX_MEI_DMAWrite (DSL_DEV_Device_t * pDev, u32 destaddr,
			u32 * databuff, u32 databuffsize)
{
	u32 *p = databuff;
	u32 temp;

	if (destaddr & 3)
		return DSL_DEV_MEI_ERR_FAILURE;

	//      Set the write transfer address
	IFX_MEI_LongWordWriteOffset (pDev, ME_DX_AD, destaddr);

	//      Write the data pushed across DMA
	while (databuffsize--) {
		temp = *p;
		if (destaddr == MEI_TO_ARC_MAILBOX)
			MEI_HALF_WORD_SWAP (temp);
		IFX_MEI_LongWordWriteOffset (pDev, (u32) ME_DX_DATA, temp);
		p++;
	}

	return DSL_DEV_MEI_ERR_SUCCESS;

}

/**
 * Read several DWORD datas from ARC memory via ARC DMA interface
 * This function reads several DWORD datas from ARC memory via DMA interface.
 *
 * \param 	pDev		the device pointer
 * \param  	srcaddr		The address to read
 * \param  	databuff	Pointer to the data buffer
 * \param  	databuffsize	Number of DWORDs to read
 * \return	DSL_DEV_MEI_ERR_SUCCESS or DSL_DEV_MEI_ERR_FAILURE
 * \ingroup	Internal
 */
static DSL_DEV_MeiError_t
IFX_MEI_DMARead (DSL_DEV_Device_t * pDev, u32 srcaddr, u32 * databuff,
		       u32 databuffsize)
{
	u32 *p = databuff;
	u32 temp;

	if (srcaddr & 3)
		return DSL_DEV_MEI_ERR_FAILURE;

	//      Set the read transfer address
	IFX_MEI_LongWordWriteOffset (pDev, (u32) ME_DX_AD, srcaddr);

	//      Read the data popped across DMA
	while (databuffsize--) {
		IFX_MEI_LongWordReadOffset (pDev, (u32) ME_DX_DATA, &temp);
		if (databuff == (u32 *) DSL_DEV_PRIVATE(pDev)->CMV_RxMsg)	// swap half word
			MEI_HALF_WORD_SWAP (temp);
		*p = temp;
		p++;
	}

	return DSL_DEV_MEI_ERR_SUCCESS;

}

/**
 * Switch the ARC control mode
 * This function switchs the ARC control mode to JTAG mode or MEI mode
 *
 * \param 	pDev		the device pointer
 * \param  	mode		The mode want to switch: JTAG_MASTER_MODE or MEI_MASTER_MODE.
 * \ingroup	Internal
 */
static void
IFX_MEI_ControlModeSet (DSL_DEV_Device_t * pDev, int mode)
{
	u32 temp = 0x0;

	IFX_MEI_LongWordReadOffset (pDev, (u32) ME_DBG_MASTER, &temp);
	switch (mode) {
	case JTAG_MASTER_MODE:
		temp &= ~(HOST_MSTR);
		break;
	case MEI_MASTER_MODE:
		temp |= (HOST_MSTR);
		break;
	default:
		IFX_MEI_EMSG ("IFX_MEI_ControlModeSet: unkonwn mode [%d]\n", mode);
		return;
	}
	IFX_MEI_LongWordWriteOffset (pDev, (u32) ME_DBG_MASTER, temp);
}

/**
 * Disable ARC to MEI interrupt
 *
 * \param 	pDev		the device pointer
 * \ingroup	Internal
 */
static void
IFX_MEI_IRQDisable (DSL_DEV_Device_t * pDev)
{
	IFX_MEI_LongWordWriteOffset (pDev, (u32) ME_ARC2ME_MASK,  0x0);
}

/**
 * Eable ARC to MEI interrupt
 *
 * \param 	pDev		the device pointer
 * \ingroup	Internal
 */
static void
IFX_MEI_IRQEnable (DSL_DEV_Device_t * pDev)
{
	IFX_MEI_LongWordWriteOffset (pDev, (u32) ME_ARC2ME_MASK, MSGAV_EN);
}

/**
 * Poll for transaction complete signal
 * This function polls and waits for transaction complete signal.
 *
 * \param 	pDev		the device pointer
 * \ingroup	Internal
 */
static void
meiPollForDbgDone (DSL_DEV_Device_t * pDev)
{
	u32 query = 0;
	int i = 0;

	while (i < WHILE_DELAY) {
		IFX_MEI_LongWordReadOffset (pDev, (u32) ME_ARC2ME_STAT,  &query);
		query &= (ARC_TO_MEI_DBG_DONE);
		if (query)
			break;
		i++;
		if (i == WHILE_DELAY) {
			IFX_MEI_EMSG ("PollforDbg fail!\n");
		}
	}
	IFX_MEI_LongWordWriteOffset (pDev, (u32) ME_ARC2ME_STAT, ARC_TO_MEI_DBG_DONE);	// to clear this interrupt
}

/**
 * ARC Debug Memory Access for a single DWORD reading.
 * This function used for direct, address-based access to ARC memory.
 *
 * \param 	pDev		the device pointer
 * \param  	DEC_mode	ARC memory space to used
 * \param  	address	  	Address to read
 * \param  	data	  	Pointer to data
 * \return	DSL_DEV_MEI_ERR_SUCCESS or DSL_DEV_MEI_ERR_FAILURE
 * \ingroup	Internal
 */
static DSL_DEV_MeiError_t
_IFX_MEI_DBGLongWordRead (DSL_DEV_Device_t * pDev, u32 DEC_mode,
				u32 address, u32 * data)
{
	IFX_MEI_LongWordWriteOffset (pDev, (u32) ME_DBG_DECODE, DEC_mode);
	IFX_MEI_LongWordWriteOffset (pDev, (u32) ME_DBG_RD_AD, address);
	meiPollForDbgDone (pDev);
	IFX_MEI_LongWordReadOffset (pDev, (u32) ME_DBG_DATA, data);
	return DSL_DEV_MEI_ERR_SUCCESS;
}

/**
 * ARC Debug Memory Access for a single DWORD writing.
 * This function used for direct, address-based access to ARC memory.
 *
 * \param 	pDev		the device pointer
 * \param  	DEC_mode	ARC memory space to used
 * \param  	address	  	The address to write
 * \param  	data	  	The data to write
 * \return	DSL_DEV_MEI_ERR_SUCCESS or DSL_DEV_MEI_ERR_FAILURE
 * \ingroup	Internal
 */
static DSL_DEV_MeiError_t
_IFX_MEI_DBGLongWordWrite (DSL_DEV_Device_t * pDev, u32 DEC_mode,
				 u32 address, u32 data)
{
	IFX_MEI_LongWordWriteOffset (pDev, (u32) ME_DBG_DECODE, DEC_mode);
	IFX_MEI_LongWordWriteOffset (pDev, (u32) ME_DBG_WR_AD, address);
	IFX_MEI_LongWordWriteOffset (pDev, (u32) ME_DBG_DATA, data);
	meiPollForDbgDone (pDev);
	return DSL_DEV_MEI_ERR_SUCCESS;
}

/**
 * ARC Debug Memory Access for writing.
 * This function used for direct, address-based access to ARC memory.
 *
 * \param 	pDev		the device pointer
 * \param  	destaddr	The address to read
 * \param  	databuffer  	Pointer to data
 * \param	databuffsize	The number of DWORDs to read
 * \return	DSL_DEV_MEI_ERR_SUCCESS or DSL_DEV_MEI_ERR_FAILURE
 * \ingroup	Internal
 */

static DSL_DEV_MeiError_t
IFX_MEI_DebugWrite (DSL_DEV_Device_t * pDev, u32 destaddr,
			  u32 * databuff, u32 databuffsize)
{
	u32 i;
	u32 temp = 0x0;
	u32 address = 0x0;
	u32 *buffer = 0x0;

	//      Open the debug port before DMP memory write
	IFX_MEI_ControlModeSet (pDev, MEI_MASTER_MODE);

	//      For the requested length, write the address and write the data
	address = destaddr;
	buffer = databuff;
	for (i = 0; i < databuffsize; i++) {
		temp = *buffer;
		_IFX_MEI_DBGLongWordWrite (pDev, ME_DBG_DECODE_DMP1_MASK, address, temp);
		address += 4;
		buffer++;
	}

	//      Close the debug port after DMP memory write
	IFX_MEI_ControlModeSet (pDev, JTAG_MASTER_MODE);

	return DSL_DEV_MEI_ERR_SUCCESS;
}

/**
 * ARC Debug Memory Access for reading.
 * This function used for direct, address-based access to ARC memory.
 *
 * \param 	pDev		the device pointer
 * \param  	srcaddr	  	The address to read
 * \param  	databuffer  	Pointer to data
 * \param	databuffsize	The number of DWORDs to read
 * \return	DSL_DEV_MEI_ERR_SUCCESS or DSL_DEV_MEI_ERR_FAILURE
 * \ingroup	Internal
 */
static DSL_DEV_MeiError_t
IFX_MEI_DebugRead (DSL_DEV_Device_t * pDev, u32 srcaddr, u32 * databuff, u32 databuffsize)
{
	u32 i;
	u32 temp = 0x0;
	u32 address = 0x0;
	u32 *buffer = 0x0;

	//      Open the debug port before DMP memory read
	IFX_MEI_ControlModeSet (pDev, MEI_MASTER_MODE);

	//      For the requested length, write the address and read the data
	address = srcaddr;
	buffer = databuff;
	for (i = 0; i < databuffsize; i++) {
		_IFX_MEI_DBGLongWordRead (pDev, ME_DBG_DECODE_DMP1_MASK, address, &temp);
		*buffer = temp;
		address += 4;
		buffer++;
	}

	//      Close the debug port after DMP memory read
	IFX_MEI_ControlModeSet (pDev, JTAG_MASTER_MODE);

	return DSL_DEV_MEI_ERR_SUCCESS;
}

/**
 * Send a message to ARC MailBox.
 * This function sends a message to ARC Mailbox via ARC DMA interface.
 *
 * \param 	pDev		the device pointer
 * \param  	msgsrcbuffer  	Pointer to message.
 * \param	msgsize		The number of words to write.
 * \return	DSL_DEV_MEI_ERR_SUCCESS or DSL_DEV_MEI_ERR_FAILURE
 * \ingroup	Internal
 */
static DSL_DEV_MeiError_t
IFX_MEI_MailboxWrite (DSL_DEV_Device_t * pDev, u16 * msgsrcbuffer,
			    u16 msgsize)
{
	int i;
	u32 arc_mailbox_status = 0x0;
	u32 temp = 0;
	DSL_DEV_MeiError_t meiMailboxError = DSL_DEV_MEI_ERR_SUCCESS;

	//      Write to mailbox
	meiMailboxError =
		IFX_MEI_DMAWrite (pDev, MEI_TO_ARC_MAILBOX, (u32 *) msgsrcbuffer, msgsize / 2);
	meiMailboxError =
		IFX_MEI_DMAWrite (pDev, MEI_TO_ARC_MAILBOXR, (u32 *) (&temp), 1);

	//      Notify arc that mailbox write completed
	DSL_DEV_PRIVATE(pDev)->cmv_waiting = 1;
	IFX_MEI_LongWordWriteOffset (pDev, (u32) ME_ME2ARC_INT, MEI_TO_ARC_MSGAV);

	i = 0;
	while (i < WHILE_DELAY) {	// wait for ARC to clear the bit
		IFX_MEI_LongWordReadOffset (pDev, (u32) ME_ME2ARC_INT, &arc_mailbox_status);
		if ((arc_mailbox_status & MEI_TO_ARC_MSGAV) != MEI_TO_ARC_MSGAV)
			break;
		i++;
		if (i == WHILE_DELAY) {
			IFX_MEI_EMSG (">>> Timeout waiting for ARC to clear MEI_TO_ARC_MSGAV!!!"
			      " MEI_TO_ARC message size = %d DWORDs <<<\n", msgsize/2);
			meiMailboxError = DSL_DEV_MEI_ERR_FAILURE;
		}
	}

	return meiMailboxError;
}

/**
 * Read a message from ARC MailBox.
 * This function reads a message from ARC Mailbox via ARC DMA interface.
 *
 * \param 	pDev		the device pointer
 * \param  	msgsrcbuffer  	Pointer to message.
 * \param	msgsize		The number of words to read
 * \return	DSL_DEV_MEI_ERR_SUCCESS or DSL_DEV_MEI_ERR_FAILURE
 * \ingroup	Internal
 */
static DSL_DEV_MeiError_t
IFX_MEI_MailboxRead (DSL_DEV_Device_t * pDev, u16 * msgdestbuffer,
			   u16 msgsize)
{
	DSL_DEV_MeiError_t meiMailboxError = DSL_DEV_MEI_ERR_SUCCESS;
	//      Read from mailbox
	meiMailboxError =
		IFX_MEI_DMARead (pDev, ARC_TO_MEI_MAILBOX, (u32 *) msgdestbuffer, msgsize / 2);

	//      Notify arc that mailbox read completed
	IFX_MEI_LongWordWriteOffset (pDev, (u32) ME_ARC2ME_STAT, ARC_TO_MEI_MSGAV);

	return meiMailboxError;
}

/**
 * Download boot pages to ARC.
 * This function downloads boot pages to ARC.
 *
 * \param 	pDev		the device pointer
 * \return	DSL_DEV_MEI_ERR_SUCCESS or DSL_DEV_MEI_ERR_FAILURE
 * \ingroup	Internal
 */
static DSL_DEV_MeiError_t
IFX_MEI_DownloadBootPages (DSL_DEV_Device_t * pDev)
{
	int boot_loop;
	int page_size;
	u32 dest_addr;

	/*
	 **     DMA the boot code page(s)
	 */

	for (boot_loop = 1;
	     boot_loop <
	     (DSL_DEV_PRIVATE(pDev)->img_hdr-> count); boot_loop++) {
		if ((DSL_DEV_PRIVATE(pDev)-> img_hdr->page[boot_loop].p_size) & BOOT_FLAG) {
			page_size = IFX_MEI_GetPage (pDev, boot_loop,
						       GET_PROG, MAXSWAPSIZE,
						       mei_arc_swap_buff,
						       &dest_addr);
			if (page_size > 0) {
				IFX_MEI_DMAWrite (pDev, dest_addr,
							mei_arc_swap_buff,
							page_size);
			}
		}
		if ((DSL_DEV_PRIVATE(pDev)-> img_hdr->page[boot_loop].d_size) & BOOT_FLAG) {
			page_size = IFX_MEI_GetPage (pDev, boot_loop,
						       GET_DATA, MAXSWAPSIZE,
						       mei_arc_swap_buff,
						       &dest_addr);
			if (page_size > 0) {
				IFX_MEI_DMAWrite (pDev, dest_addr,
							mei_arc_swap_buff,
							page_size);
			}
		}
	}
	return DSL_DEV_MEI_ERR_SUCCESS;
}

/**
 * Initial efuse rar.
 **/
static void
IFX_MEI_FuseInit (DSL_DEV_Device_t * pDev)
{
	u32 data = 0;
	IFX_MEI_DMAWrite (pDev, IRAM0_BASE, &data, 1);
	IFX_MEI_DMAWrite (pDev, IRAM0_BASE + 4, &data, 1);
	IFX_MEI_DMAWrite (pDev, IRAM1_BASE, &data, 1);
	IFX_MEI_DMAWrite (pDev, IRAM1_BASE + 4, &data, 1);
	IFX_MEI_DMAWrite (pDev, BRAM_BASE, &data, 1);
	IFX_MEI_DMAWrite (pDev, BRAM_BASE + 4, &data, 1);
	IFX_MEI_DMAWrite (pDev, ADSL_DILV_BASE, &data, 1);
	IFX_MEI_DMAWrite (pDev, ADSL_DILV_BASE + 4, &data, 1);
}

/**
 * efuse rar program
 **/
static void
IFX_MEI_FuseProg (DSL_DEV_Device_t * pDev)
{
	u32 reg_data, fuse_value;
	int i = 0;

	IFX_MEI_LongWordRead ((u32) LTQ_RCU_RST, &reg_data);
	while ((reg_data & 0x10000000) == 0) {
		IFX_MEI_LongWordRead ((u32) LTQ_RCU_RST,  &reg_data);
		i++;
		/* 0x4000 translate to  about 16 ms@111M, so should be enough */
		if (i == 0x4000)
			return;
	}
	// STEP a: Prepare memory for external accesses
	// Write fuse_en bit24
	IFX_MEI_LongWordRead ((u32) LTQ_RCU_RST, &reg_data);
	IFX_MEI_LongWordWrite ((u32) LTQ_RCU_RST, reg_data | (1 << 24));

	IFX_MEI_FuseInit (pDev);
	for (i = 0; i < 4; i++) {
		IFX_MEI_LongWordRead ((u32) (LTQ_FUSE_BASE) + i * 4, &fuse_value);
		switch (fuse_value & 0xF0000) {
		case 0x80000:
			reg_data = ((fuse_value & RX_DILV_ADDR_BIT_MASK) |
				 (RX_DILV_ADDR_BIT_MASK + 0x1));
			IFX_MEI_DMAWrite (pDev, ADSL_DILV_BASE, &reg_data, 1);
			break;
		case 0x90000:
			reg_data = ((fuse_value & RX_DILV_ADDR_BIT_MASK) |
				 (RX_DILV_ADDR_BIT_MASK + 0x1));
			IFX_MEI_DMAWrite (pDev, ADSL_DILV_BASE + 4, &reg_data, 1);
			break;
		case 0xA0000:
			reg_data = ((fuse_value & IRAM0_ADDR_BIT_MASK) |
				 (IRAM0_ADDR_BIT_MASK + 0x1));
			IFX_MEI_DMAWrite (pDev, IRAM0_BASE, &reg_data, 1);
			break;
		case 0xB0000:
			reg_data = ((fuse_value & IRAM0_ADDR_BIT_MASK) |
				 (IRAM0_ADDR_BIT_MASK + 0x1));
			IFX_MEI_DMAWrite (pDev, IRAM0_BASE + 4, &reg_data, 1);
			break;
		case 0xC0000:
			reg_data = ((fuse_value & IRAM1_ADDR_BIT_MASK) |
				 (IRAM1_ADDR_BIT_MASK + 0x1));
			IFX_MEI_DMAWrite (pDev, IRAM1_BASE, &reg_data, 1);
			break;
		case 0xD0000:
			reg_data = ((fuse_value & IRAM1_ADDR_BIT_MASK) |
				 (IRAM1_ADDR_BIT_MASK + 0x1));
			IFX_MEI_DMAWrite (pDev, IRAM1_BASE + 4, &reg_data, 1);
			break;
		case 0xE0000:
			reg_data = ((fuse_value & BRAM_ADDR_BIT_MASK) |
				 (BRAM_ADDR_BIT_MASK + 0x1));
			IFX_MEI_DMAWrite (pDev, BRAM_BASE, &reg_data, 1);
			break;
		case 0xF0000:
			reg_data = ((fuse_value & BRAM_ADDR_BIT_MASK) |
				 (BRAM_ADDR_BIT_MASK + 0x1));
			IFX_MEI_DMAWrite (pDev, BRAM_BASE + 4, &reg_data, 1);
			break;
		default:	// PPE efuse
			break;
		}
	}
	IFX_MEI_LongWordRead ((u32) LTQ_RCU_RST, &reg_data);
	IFX_MEI_LongWordWrite ((u32) LTQ_RCU_RST, reg_data & ~(1 << 24));
	IFX_MEI_LongWordRead ((u32) LTQ_RCU_RST, &reg_data);
}

/**
 * Enable DFE Clock
 * This function enables DFE Clock
 *
 * \param 	pDev		the device pointer
 * \return	DSL_DEV_MEI_ERR_SUCCESS or DSL_DEV_MEI_ERR_FAILURE
 * \ingroup	Internal
 */
static DSL_DEV_MeiError_t
IFX_MEI_EnableCLK (DSL_DEV_Device_t * pDev)
{
	u32 arc_debug_data = 0;
	IFX_MEI_ControlModeSet (pDev, MEI_MASTER_MODE);
	//enable ac_clk signal
	_IFX_MEI_DBGLongWordRead (pDev, ME_DBG_DECODE_DMP1_MASK,
					CRI_CCR0, &arc_debug_data);
	arc_debug_data |= ACL_CLK_MODE_ENABLE;
	_IFX_MEI_DBGLongWordWrite (pDev, ME_DBG_DECODE_DMP1_MASK,
					 CRI_CCR0, arc_debug_data);
	IFX_MEI_ControlModeSet (pDev, JTAG_MASTER_MODE);
	return DSL_DEV_MEI_ERR_SUCCESS;
}

/**
 * Halt the ARC.
 * This function halts the ARC.
 *
 * \param 	pDev		the device pointer
 * \return	DSL_DEV_MEI_ERR_SUCCESS or DSL_DEV_MEI_ERR_FAILURE
 * \ingroup	Internal
 */
static DSL_DEV_MeiError_t
IFX_MEI_HaltArc (DSL_DEV_Device_t * pDev)
{
	u32 arc_debug_data = 0x0;

	//      Switch arc control from JTAG mode to MEI mode
	IFX_MEI_ControlModeSet (pDev, MEI_MASTER_MODE);
	_IFX_MEI_DBGLongWordRead (pDev, MEI_DEBUG_DEC_AUX_MASK,
					ARC_DEBUG, &arc_debug_data);
	arc_debug_data |= ARC_DEBUG_HALT;
	_IFX_MEI_DBGLongWordWrite (pDev, MEI_DEBUG_DEC_AUX_MASK,
					 ARC_DEBUG, arc_debug_data);
	//      Switch arc control from MEI mode to JTAG mode
	IFX_MEI_ControlModeSet (pDev, JTAG_MASTER_MODE);

	MEI_WAIT (10);

	return DSL_DEV_MEI_ERR_SUCCESS;
}

/**
 * Run the ARC.
 * This function runs the ARC.
 *
 * \param 	pDev		the device pointer
 * \return	DSL_DEV_MEI_ERR_SUCCESS or DSL_DEV_MEI_ERR_FAILURE
 * \ingroup	Internal
 */
static DSL_DEV_MeiError_t
IFX_MEI_RunArc (DSL_DEV_Device_t * pDev)
{
	u32 arc_debug_data = 0x0;

	//      Switch arc control from JTAG mode to MEI mode- write '1' to bit0
	IFX_MEI_ControlModeSet (pDev, MEI_MASTER_MODE);
	_IFX_MEI_DBGLongWordRead (pDev, MEI_DEBUG_DEC_AUX_MASK,
					AUX_STATUS, &arc_debug_data);

	//      Write debug data reg with content ANDd with 0xFDFFFFFF (halt bit cleared)
	arc_debug_data &= ~ARC_AUX_HALT;
	_IFX_MEI_DBGLongWordWrite (pDev, MEI_DEBUG_DEC_AUX_MASK,
					 AUX_STATUS, arc_debug_data);

	//      Switch arc control from MEI mode to JTAG mode- write '0' to bit0
	IFX_MEI_ControlModeSet (pDev, JTAG_MASTER_MODE);
	//      Enable mask for arc codeswap interrupts
	IFX_MEI_IRQEnable (pDev);

	return DSL_DEV_MEI_ERR_SUCCESS;

}

/**
 * Reset the ARC.
 * This function resets the ARC.
 *
 * \param 	pDev		the device pointer
 * \return	DSL_DEV_MEI_ERR_SUCCESS or DSL_DEV_MEI_ERR_FAILURE
 * \ingroup	Internal
 */
static DSL_DEV_MeiError_t
IFX_MEI_ResetARC (DSL_DEV_Device_t * pDev)
{
	u32 arc_debug_data = 0;

	IFX_MEI_HaltArc (pDev);

	IFX_MEI_LongWordRead ((u32) LTQ_RCU_RST, &arc_debug_data);
	IFX_MEI_LongWordWrite ((u32) LTQ_RCU_RST,
		arc_debug_data | LTQ_RCU_RST_REQ_DFE | LTQ_RCU_RST_REQ_AFE);

	// reset ARC
	IFX_MEI_LongWordWriteOffset (pDev, (u32) ME_RST_CTRL, MEI_SOFT_RESET);
	IFX_MEI_LongWordWriteOffset (pDev, (u32) ME_RST_CTRL, 0);

	IFX_MEI_IRQDisable (pDev);

	IFX_MEI_EnableCLK (pDev);

#if 0
	// reset part of PPE
	*(unsigned long *) (BSP_PPE32_SRST) = 0xC30;
	*(unsigned long *) (BSP_PPE32_SRST) = 0xFFF;
#endif

	DSL_DEV_PRIVATE(pDev)->modem_ready = 0;

	return DSL_DEV_MEI_ERR_SUCCESS;
}

DSL_DEV_MeiError_t
DSL_BSP_Showtime (DSL_DEV_Device_t * dev, DSL_uint32_t rate_fast, DSL_uint32_t rate_intl)
{
    struct port_cell_info port_cell = {0};

	IFX_MEI_EMSG ("Datarate US intl = %d, fast = %d\n", (int)rate_intl,
			    (int)rate_fast);

    if ( rate_fast )
        g_tx_link_rate[0] = rate_fast / (53 * 8);
    if ( rate_intl )
        g_tx_link_rate[1] = rate_intl / (53 * 8);

    if ( g_tx_link_rate[0] == 0 && g_tx_link_rate[1] == 0 ) {
        IFX_MEI_EMSG ("Got rate fail.\n");
    }

	if ( ifx_mei_atm_showtime_enter )
	{
	    port_cell.port_num = 2;
	    port_cell.tx_link_rate[0] = g_tx_link_rate[0];
	    port_cell.tx_link_rate[1] = g_tx_link_rate[1];
        ifx_mei_atm_showtime_enter(&port_cell, g_xdata_addr);
	}
	else
	{
		IFX_MEI_EMSG("no hookup from ATM driver to set cell rate\n");
	}

	return DSL_DEV_MEI_ERR_SUCCESS;
};

/**
 * Reset/halt/run the DFE.
 * This function provide operations to reset/halt/run the DFE.
 *
 * \param 	pDev		the device pointer
 * \param	mode		which operation want to do
 * \return	DSL_DEV_MEI_ERR_SUCCESS or DSL_DEV_MEI_ERR_FAILURE
 * \ingroup	Internal
 */
static DSL_DEV_MeiError_t
IFX_MEI_CpuModeSet (DSL_DEV_Device_t *pDev,
			  DSL_DEV_CpuMode_t mode)
{
	DSL_DEV_MeiError_t err_ret = DSL_DEV_MEI_ERR_FAILURE;
	switch (mode) {
	case DSL_CPU_HALT:
		err_ret = IFX_MEI_HaltArc (pDev);
		break;
	case DSL_CPU_RUN:
		err_ret = IFX_MEI_RunArc (pDev);
		break;
	case DSL_CPU_RESET:
		err_ret = IFX_MEI_ResetARC (pDev);
		break;
	default:
		break;
	}
	return err_ret;
}

/**
 * Accress DFE memory.
 * This function provide a way to access DFE memory;
 *
 * \param 	pDev		the device pointer
 * \param	type		read or write
 * \param	destaddr	destination address
 * \param	databuff	pointer to hold data
 * \param	databuffsize	size want to read/write
 * \return	DSL_DEV_MEI_ERR_SUCCESS or DSL_DEV_MEI_ERR_FAILURE
 * \ingroup	Internal
 */
DSL_DEV_MeiError_t
DSL_BSP_MemoryDebugAccess (DSL_DEV_Device_t * pDev,
				DSL_BSP_MemoryAccessType_t type,
				DSL_uint32_t destaddr, DSL_uint32_t *databuff,
				DSL_uint32_t databuffsize)
{
	DSL_DEV_MeiError_t meierr = DSL_DEV_MEI_ERR_SUCCESS;
	switch (type) {
	case DSL_BSP_MEMORY_READ:
		meierr = IFX_MEI_DebugRead (pDev, (u32)destaddr, (u32*)databuff, (u32)databuffsize);
		break;
	case DSL_BSP_MEMORY_WRITE:
		meierr = IFX_MEI_DebugWrite (pDev, (u32)destaddr, (u32*)databuff, (u32)databuffsize);
		break;
	}
	return DSL_DEV_MEI_ERR_SUCCESS;
};

/**
 * Download boot code to ARC.
 * This function downloads boot code to ARC.
 *
 * \param 	pDev		the device pointer
 * \return	DSL_DEV_MEI_ERR_SUCCESS or DSL_DEV_MEI_ERR_FAILURE
 * \ingroup	Internal
 */
static DSL_DEV_MeiError_t
IFX_MEI_DownloadBootCode (DSL_DEV_Device_t *pDev)
{
	IFX_MEI_IRQDisable (pDev);

	IFX_MEI_EnableCLK (pDev);

	IFX_MEI_FuseProg (pDev);	//program fuse rar

	IFX_MEI_DownloadBootPages (pDev);

	return DSL_DEV_MEI_ERR_SUCCESS;
};

/**
 * Enable Jtag debugger interface
 * This function setups mips gpio to enable jtag debugger
 *
 * \param 	pDev		the device pointer
 * \param 	enable		enable or disable
 * \return	DSL_DEV_MEI_ERR_SUCCESS or DSL_DEV_MEI_ERR_FAILURE
 * \ingroup	Internal
 */
static DSL_DEV_MeiError_t
IFX_MEI_ArcJtagEnable (DSL_DEV_Device_t *dev, int enable)
{
	/*
	int meierr=0;
	u32 reg_data;
	switch (enable) {
	case 1:
                //reserve gpio 9, 10, 11, 14, 19 for ARC JTAG
		ifxmips_port_reserve_pin (0, 9);
		ifxmips_port_reserve_pin (0, 10);
		ifxmips_port_reserve_pin (0, 11);
		ifxmips_port_reserve_pin (0, 14);
		ifxmips_port_reserve_pin (1, 3);

		ifxmips_port_set_dir_in(0, 11);
		ifxmips_port_clear_altsel0(0, 11);
		ifxmips_port_clear_altsel1(0, 11);
		ifxmips_port_set_open_drain(0, 11);
        //enable ARC JTAG
        IFX_MEI_LongWordRead ((u32) LTQ_RCU_RST, &reg_data);
        IFX_MEI_LongWordWrite ((u32) LTQ_RCU_RST, reg_data | LTQ_RCU_RST_REQ_ARC_JTAG);
		break;
	case 0:
	default:
		break;
	}
jtag_end:
	if (meierr)
		return DSL_DEV_MEI_ERR_FAILURE;
*/

	return DSL_DEV_MEI_ERR_SUCCESS;
};

/**
 * Enable DFE to MIPS interrupt
 * This function enable DFE to MIPS interrupt
 *
 * \param 	pDev		the device pointer
 * \param 	enable		enable or disable
 * \return	DSL_DEV_MEI_ERR_SUCCESS or DSL_DEV_MEI_ERR_FAILURE
 * \ingroup	Internal
 */
static DSL_DEV_MeiError_t
IFX_MEI_AdslMailboxIRQEnable (DSL_DEV_Device_t *pDev, int enable)
{
	DSL_DEV_MeiError_t meierr;
	switch (enable) {
	case 0:
		meierr = DSL_DEV_MEI_ERR_SUCCESS;
		IFX_MEI_IRQDisable (pDev);
		break;
	case 1:
		IFX_MEI_IRQEnable (pDev);
		meierr = DSL_DEV_MEI_ERR_SUCCESS;
		break;
	default:
		meierr = DSL_DEV_MEI_ERR_FAILURE;
		break;

	}
	return meierr;
}

/**
 * Get the modem status
 * This function return the modem status
 *
 * \param 	pDev		the device pointer
 * \return	1: modem ready 0: not ready
 * \ingroup	Internal
 */
static int
IFX_MEI_IsModemReady (DSL_DEV_Device_t * pDev)
{
	return DSL_DEV_PRIVATE(pDev)->modem_ready;
}

DSL_DEV_MeiError_t
DSL_BSP_AdslLedInit (DSL_DEV_Device_t * dev,
			  DSL_DEV_LedId_t led_number,
			  DSL_DEV_LedType_t type,
			  DSL_DEV_LedHandler_t handler)
{
#if 0
        struct led_config_param param;
        if (led_number == DSL_LED_LINK_ID && type == DSL_LED_LINK_TYPE && handler == /*DSL_LED_HD_CPU*/DSL_LED_HD_FW) {
                param.operation_mask = CONFIG_OPERATION_UPDATE_SOURCE;
                param.led = 0x01;
                param.source = 0x01;
//                bsp_led_config (&param);

        } else if (led_number == DSL_LED_DATA_ID && type == DSL_LED_DATA_TYPE && (handler == DSL_LED_HD_FW)) {
                param.operation_mask = CONFIG_OPERATION_UPDATE_SOURCE;
                param.led = 0x02;
                param.source = 0x02;
//                bsp_led_config (&param);
        }
#endif
        return DSL_DEV_MEI_ERR_SUCCESS;
};
#if 0
DSL_DEV_MeiError_t
DSL_BSP_AdslLedSet (DSL_DEV_Device_t * dev, DSL_DEV_LedId_t led_number, DSL_DEV_LedMode_t mode)
{
	printk(KERN_INFO "[%s %d]: mode = %#x, led_number = %d\n", __func__, __LINE__, mode, led_number);
	switch (mode) {
	case DSL_LED_OFF:
		switch (led_number) {
		case DSL_LED_LINK_ID:
#ifdef CONFIG_BSP_LED
			bsp_led_set_blink (1, 0);
			bsp_led_set_data (1, 0);
#endif
			break;
		case DSL_LED_DATA_ID:
#ifdef CONFIG_BSP_LED
			bsp_led_set_blink (0, 0);
			bsp_led_set_data (0, 0);
#endif
			break;
		}
		break;
	case DSL_LED_FLASH:
		switch (led_number) {
		case DSL_LED_LINK_ID:
#ifdef CONFIG_BSP_LED
			bsp_led_set_blink (1, 1);	// data
#endif
			break;
		case DSL_LED_DATA_ID:
#ifdef CONFIG_BSP_LED
			bsp_led_set_blink (0, 1);	// data
#endif
			break;
		}
		break;
	case DSL_LED_ON:
		switch (led_number) {
		case DSL_LED_LINK_ID:
#ifdef CONFIG_BSP_LED
			bsp_led_set_blink (1, 0);
			bsp_led_set_data (1, 1);
#endif
			break;
		case DSL_LED_DATA_ID:
#ifdef CONFIG_BSP_LED
			bsp_led_set_blink (0, 0);
			bsp_led_set_data (0, 1);
#endif
			break;
		}
		break;
	}
	return DSL_DEV_MEI_ERR_SUCCESS;
};

#endif

/**
* Compose a message.
* This function compose a message from opcode, group, address, index, size, and data
*
* \param       opcode          The message opcode
* \param       group           The message group number
* \param       address         The message address.
* \param       index           The message index.
* \param       size            The number of words to read/write.
* \param       data            The pointer to data.
* \param       CMVMSG          The pointer to message buffer.
* \ingroup     Internal
*/
void
makeCMV (u8 opcode, u8 group, u16 address, u16 index, int size, u16 * data, u16 *CMVMSG)
{
        memset (CMVMSG, 0, MSG_LENGTH * 2);
        CMVMSG[0] = (opcode << 4) + (size & 0xf);
        CMVMSG[1] = (((index == 0) ? 0 : 1) << 7) + (group & 0x7f);
        CMVMSG[2] = address;
        CMVMSG[3] = index;
        if (opcode == H2D_CMV_WRITE)
                memcpy (CMVMSG + 4, data, size * 2);
        return;
}

/**
 * Send a message to ARC and read the response
 * This function sends a message to arc, waits the response, and reads the responses.
 *
 * \param 	pDev		the device pointer
 * \param	request		Pointer to the request
 * \param	reply		Wait reply or not.
 * \param	response	Pointer to the response
 * \return	DSL_DEV_MEI_ERR_SUCCESS or DSL_DEV_MEI_ERR_FAILURE
 * \ingroup	Internal
 */
DSL_DEV_MeiError_t
DSL_BSP_SendCMV (DSL_DEV_Device_t * pDev, u16 * request, int reply, u16 * response)	// write cmv to arc, if reply needed, wait for reply
{
	DSL_DEV_MeiError_t meierror;
#if defined(BSP_PORT_RTEMS)
	int delay_counter = 0;
#endif

	if (MEI_MUTEX_LOCK (DSL_DEV_PRIVATE(pDev)->mei_cmv_sema))
		return -ERESTARTSYS;

	DSL_DEV_PRIVATE(pDev)->cmv_reply = reply;
	memset (DSL_DEV_PRIVATE(pDev)->CMV_RxMsg, 0,
		sizeof (DSL_DEV_PRIVATE(pDev)->
			CMV_RxMsg));
	DSL_DEV_PRIVATE(pDev)->arcmsgav = 0;

	meierror = IFX_MEI_MailboxWrite (pDev, request, MSG_LENGTH);

	if (meierror != DSL_DEV_MEI_ERR_SUCCESS) {
		DSL_DEV_PRIVATE(pDev)->cmv_waiting = 0;
		DSL_DEV_PRIVATE(pDev)->arcmsgav = 0;
		IFX_MEI_EMSG ("MailboxWrite Fail!\n");
		IFX_MEI_EMSG ("Resetting ARC...\n");
		IFX_MEI_ResetARC(pDev);
		MEI_MUTEX_UNLOCK (DSL_DEV_PRIVATE(pDev)->mei_cmv_sema);
		return meierror;
	}
	else {
		DSL_DEV_PRIVATE(pDev)->cmv_count++;
	}

	if (DSL_DEV_PRIVATE(pDev)->cmv_reply ==
	    NO_REPLY) {
		MEI_MUTEX_UNLOCK (DSL_DEV_PRIVATE(pDev)->mei_cmv_sema);
		return DSL_DEV_MEI_ERR_SUCCESS;
	}

#if !defined(BSP_PORT_RTEMS)
	if (DSL_DEV_PRIVATE(pDev)->arcmsgav == 0)
		MEI_WAIT_EVENT_TIMEOUT (DSL_DEV_PRIVATE(pDev)->wait_queue_arcmsgav, CMV_TIMEOUT);
#else
	while (DSL_DEV_PRIVATE(pDev)->arcmsgav == 0 && delay_counter < CMV_TIMEOUT / 5) {
		MEI_WAIT (5);
		delay_counter++;
	}
#endif

	DSL_DEV_PRIVATE(pDev)->cmv_waiting = 0;
	if (DSL_DEV_PRIVATE(pDev)->arcmsgav == 0) {	//CMV_timeout
		DSL_DEV_PRIVATE(pDev)->arcmsgav = 0;
		IFX_MEI_EMSG ("\%s: DSL_DEV_MEI_ERR_MAILBOX_TIMEOUT\n",
				    __FUNCTION__);
		MEI_MUTEX_UNLOCK (DSL_DEV_PRIVATE(pDev)->mei_cmv_sema);
		return DSL_DEV_MEI_ERR_MAILBOX_TIMEOUT;
	}
	else {
		DSL_DEV_PRIVATE(pDev)->arcmsgav = 0;
		DSL_DEV_PRIVATE(pDev)->
			reply_count++;
		memcpy (response, DSL_DEV_PRIVATE(pDev)->CMV_RxMsg, MSG_LENGTH * 2);
		MEI_MUTEX_UNLOCK (DSL_DEV_PRIVATE(pDev)->mei_cmv_sema);
		return DSL_DEV_MEI_ERR_SUCCESS;
	}
	MEI_MUTEX_UNLOCK (DSL_DEV_PRIVATE(pDev)->mei_cmv_sema);
	return DSL_DEV_MEI_ERR_SUCCESS;
}

/**
 * Reset the ARC, download boot codes, and run the ARC.
 * This function resets the ARC, downloads boot codes to ARC, and runs the ARC.
 *
 * \param 	pDev		the device pointer
 * \return	DSL_DEV_MEI_ERR_SUCCESS or DSL_DEV_MEI_ERR_FAILURE
 * \ingroup	Internal
 */
static DSL_DEV_MeiError_t
IFX_MEI_RunAdslModem (DSL_DEV_Device_t *pDev)
{
	int nSize = 0, idx = 0;
	uint32_t im0_register, im2_register;
//	DSL_DEV_WinHost_Message_t m;

	if (mei_arc_swap_buff == NULL) {
		mei_arc_swap_buff =
			(u32 *) kmalloc (MAXSWAPSIZE * 4, GFP_KERNEL);
		if (mei_arc_swap_buff == NULL) {
			IFX_MEI_EMSG (">>> malloc fail for codeswap buff!!! <<<\n");
			return DSL_DEV_MEI_ERR_FAILURE;
		}
                IFX_MEI_DMSG("allocate %dKB swap buff memory at: 0x%p\n", ksize(mei_arc_swap_buff)/1024, mei_arc_swap_buff);
	}

	DSL_DEV_PRIVATE(pDev)->img_hdr =
		(ARC_IMG_HDR *) DSL_DEV_PRIVATE(pDev)->adsl_mem_info[0].address;
	if ((DSL_DEV_PRIVATE(pDev)->img_hdr->
	     count) * sizeof (ARC_SWP_PAGE_HDR) > SDRAM_SEGMENT_SIZE) {
		IFX_MEI_EMSG ("firmware header size is bigger than 64K segment size\n");
		return DSL_DEV_MEI_ERR_FAILURE;
	}
	// check image size
	for (idx = 0; idx < MAX_BAR_REGISTERS; idx++) {
		nSize += DSL_DEV_PRIVATE(pDev)->adsl_mem_info[idx].nCopy;
	}
	if (nSize !=
	    DSL_DEV_PRIVATE(pDev)->image_size) {
		IFX_MEI_EMSG ("Firmware download is not completed. Please download firmware again!\n");
		return DSL_DEV_MEI_ERR_FAILURE;
	}
	// TODO: check crc
	///

	IFX_MEI_ResetARC (pDev);
	IFX_MEI_HaltArc (pDev);
	IFX_MEI_BarUpdate (pDev, DSL_DEV_PRIVATE(pDev)->nBar);

	//IFX_MEI_DMSG("Starting to meiDownloadBootCode\n");

	IFX_MEI_DownloadBootCode (pDev);

	im0_register = (*LTQ_ICU_IM0_IER) & (1 << 20);
	im2_register = (*LTQ_ICU_IM2_IER) & (1 << 20);

	/* Turn off irq */
	disable_irq (LTQ_USB_OC_INT);
	disable_irq (pDev->nIrq[IFX_DYING_GASP]);

	IFX_MEI_RunArc (pDev);

	MEI_WAIT_EVENT_TIMEOUT (DSL_DEV_PRIVATE(pDev)->wait_queue_modemready, 1000);

	MEI_MASK_AND_ACK_IRQ (LTQ_USB_OC_INT);
	MEI_MASK_AND_ACK_IRQ (pDev->nIrq[IFX_DYING_GASP]);

	/* Re-enable irq */
	enable_irq(pDev->nIrq[IFX_DYING_GASP]);
	*LTQ_ICU_IM0_IER |= im0_register;
	*LTQ_ICU_IM2_IER |= im2_register;

	if (DSL_DEV_PRIVATE(pDev)->modem_ready != 1) {
		IFX_MEI_EMSG ("Modem failed to be ready!\n");
		return DSL_DEV_MEI_ERR_FAILURE;
	} else {
		IFX_MEI_DMSG("Modem is ready.\n");
		return DSL_DEV_MEI_ERR_SUCCESS;
	}
}

/**
 * Get the page's data pointer
 * This function caculats the data address from the firmware header.
 *
 * \param 	pDev		the device pointer
 * \param	Page		The page number.
 * \param	data		Data page or program page.
 * \param	MaxSize		The maximum size to read.
 * \param	Buffer		Pointer to data.
 * \param	Dest		Pointer to the destination address.
 * \return	The number of bytes to read.
 * \ingroup	Internal
 */
static int
IFX_MEI_GetPage (DSL_DEV_Device_t * pDev, u32 Page, u32 data,
		       u32 MaxSize, u32 * Buffer, u32 * Dest)
{
	u32 size;
	u32 i;
	u32 *p;
	u32 idx, offset, nBar = 0;

	if (Page > DSL_DEV_PRIVATE(pDev)->img_hdr->count)
		return -2;
	/*
	 **     Get program or data size, depending on "data" flag
	 */
	size = (data == GET_DATA) ? (DSL_DEV_PRIVATE(pDev)->img_hdr->page[Page].d_size) :
			     (DSL_DEV_PRIVATE(pDev)->img_hdr->page[Page].p_size);
	size &= BOOT_FLAG_MASK;	//      Clear boot bit!
	if (size > MaxSize)
		return -1;

	if (size == 0)
		return 0;
	/*
	 **     Get program or data offset, depending on "data" flag
	 */
	i = data ? (DSL_DEV_PRIVATE(pDev)->img_hdr->page[Page].d_offset) :
			(DSL_DEV_PRIVATE(pDev)->img_hdr->page[Page].p_offset);

	/*
	 **     Copy data/program to buffer
	 */

	idx = i / SDRAM_SEGMENT_SIZE;
	offset = i % SDRAM_SEGMENT_SIZE;
	p = (u32 *) ((u8 *) DSL_DEV_PRIVATE(pDev)->adsl_mem_info[idx].address + offset);

	for (i = 0; i < size; i++) {
		if (offset + i * 4 - (nBar * SDRAM_SEGMENT_SIZE) >= SDRAM_SEGMENT_SIZE) {
			idx++;
			nBar++;
			p = (u32 *) ((u8 *) KSEG1ADDR ((u32)DSL_DEV_PRIVATE(pDev)->adsl_mem_info[idx].address));
		}
		Buffer[i] = *p++;
	}

	/*
	 **     Pass back data/program destination address
	 */
	*Dest = data ? (DSL_DEV_PRIVATE(pDev)-> img_hdr->page[Page].d_dest) :
				(DSL_DEV_PRIVATE(pDev)->img_hdr->page[Page].p_dest);

	return size;
}

/**
 * Free the memory for ARC firmware
 *
 * \param 	pDev		the device pointer
 * \param	type	Free all memory or free the unused memory after showtime
 * \ingroup	Internal
 */
const char *free_str[4] = {"Invalid", "Free_Reload", "Free_Showtime", "Free_All"};
static int
IFX_MEI_DFEMemoryFree (DSL_DEV_Device_t * pDev, int type)
{
        int idx = 0;
        smmu_mem_info_t *adsl_mem_info =
                DSL_DEV_PRIVATE(pDev)->adsl_mem_info;

        for (idx = 0; idx < MAX_BAR_REGISTERS; idx++) {
                if (type == FREE_ALL ||adsl_mem_info[idx].type == type) {
                        if (adsl_mem_info[idx].size > 0) {
                                IFX_MEI_DMSG ("Freeing memory %p (%s)\n", adsl_mem_info[idx].org_address, free_str[adsl_mem_info[idx].type]);
                                if ( idx == XDATA_REGISTER ) {
                                    g_xdata_addr = NULL;
                                    if ( ifx_mei_atm_showtime_exit )
                                        ifx_mei_atm_showtime_exit();
                                }
				kfree (adsl_mem_info[idx].org_address);
                                adsl_mem_info[idx].org_address = 0;
                                adsl_mem_info[idx].address = 0;
                                adsl_mem_info[idx].size = 0;
                                adsl_mem_info[idx].type = 0;
                                adsl_mem_info[idx].nCopy = 0;
                        }
                }
        }

	if(mei_arc_swap_buff != NULL){
                IFX_MEI_DMSG("free %dKB swap buff memory at: 0x%p\n", ksize(mei_arc_swap_buff)/1024, mei_arc_swap_buff);
		kfree(mei_arc_swap_buff);
		mei_arc_swap_buff=NULL;
	}

        return 0;
}
static int
IFX_MEI_DFEMemoryAlloc (DSL_DEV_Device_t * pDev, long size)
{
	unsigned long mem_ptr;
	char *org_mem_ptr = NULL;
	int idx = 0;
	long total_size = 0;
	int err = 0;
	smmu_mem_info_t *adsl_mem_info =
		((ifx_mei_device_private_t *) pDev->pPriv)->adsl_mem_info;
//		DSL_DEV_PRIVATE(pDev)->adsl_mem_info;
	int allocate_size = SDRAM_SEGMENT_SIZE;

	IFX_MEI_DMSG("image_size = %ld\n", size);
	// Alloc Swap Pages
	for (idx = 0; size > 0 && idx < MAX_BAR_REGISTERS; idx++) {
		// skip bar15 for XDATA usage.
		if (idx == XDATA_REGISTER)
			continue;
#if 0
                if (size < SDRAM_SEGMENT_SIZE) {
                        allocate_size = size;
                        if (allocate_size < 1024)
                                allocate_size = 1024;
                }
#endif
                if (idx == (MAX_BAR_REGISTERS - 1))
                        allocate_size = size;
                else
                        allocate_size = SDRAM_SEGMENT_SIZE;
        
		org_mem_ptr = kmalloc (allocate_size, GFP_KERNEL);
		if (org_mem_ptr == NULL) {
                        IFX_MEI_EMSG ("%d: kmalloc %d bytes memory fail!\n", idx, allocate_size);
			err = -ENOMEM;
			goto allocate_error;
		}
		
		if (((unsigned long)org_mem_ptr) & (1023)) {
			/* Pointer not 1k aligned, so free it and allocate a larger chunk
			 * for further alignment.
			 */
			kfree(org_mem_ptr);
			org_mem_ptr = kmalloc (allocate_size + 1024, GFP_KERNEL);
			if (org_mem_ptr == NULL) {
				IFX_MEI_EMSG ("%d: kmalloc %d bytes memory fail!\n",
				              idx, allocate_size + 1024);
				err = -ENOMEM;
				goto allocate_error;
			}
			mem_ptr = (unsigned long) (org_mem_ptr + 1023) & ~(1024 -1);
		} else {
			mem_ptr = (unsigned long) org_mem_ptr;
		}

                adsl_mem_info[idx].address = (char *) mem_ptr;
                adsl_mem_info[idx].org_address = org_mem_ptr;
                adsl_mem_info[idx].size = allocate_size;
                size -= allocate_size;
                total_size += allocate_size;
	}
	if (size > 0) {
		IFX_MEI_EMSG ("Image size is too large!\n");
		err = -EFBIG;
		goto allocate_error;
	}
	err = idx;
	return err;

      allocate_error:
	IFX_MEI_DFEMemoryFree (pDev, FREE_ALL);
	return err;
}

/**
 * Program the BAR registers
 *
 * \param 	pDev		the device pointer
 * \param	nTotalBar	The number of bar to program.
 * \ingroup	Internal
 */
static int
IFX_MEI_BarUpdate (DSL_DEV_Device_t * pDev, int nTotalBar)
{
	int idx = 0;
	smmu_mem_info_t *adsl_mem_info =
		DSL_DEV_PRIVATE(pDev)->adsl_mem_info;

	for (idx = 0; idx < nTotalBar; idx++) {
		//skip XDATA register
		if (idx == XDATA_REGISTER)
			continue;
		IFX_MEI_LongWordWriteOffset (pDev, (u32) ME_XMEM_BAR_BASE + idx * 4,
			(((uint32_t) adsl_mem_info[idx].address) & 0x0FFFFFFF));
	}
	for (idx = nTotalBar; idx < MAX_BAR_REGISTERS; idx++) {
		if (idx == XDATA_REGISTER)
			continue;
		IFX_MEI_LongWordWriteOffset (pDev,  (u32) ME_XMEM_BAR_BASE  + idx * 4,
			 (((uint32_t)adsl_mem_info[nTotalBar - 1].address) & 0x0FFFFFFF));
		/* These are for /proc/danube_mei/meminfo purpose */
		adsl_mem_info[idx].address = adsl_mem_info[nTotalBar - 1].address;
		adsl_mem_info[idx].org_address = adsl_mem_info[nTotalBar - 1].org_address;
		adsl_mem_info[idx].size = 0; /* Prevent it from being freed */
	}

    g_xdata_addr = adsl_mem_info[XDATA_REGISTER].address;
	IFX_MEI_LongWordWriteOffset (pDev,  (u32) ME_XMEM_BAR_BASE  + XDATA_REGISTER * 4,
		(((uint32_t) adsl_mem_info [XDATA_REGISTER].address) & 0x0FFFFFFF));
	// update MEI_XDATA_BASE_SH
	IFX_MEI_LongWordWriteOffset (pDev, (u32) ME_XDATA_BASE_SH,
		 ((unsigned long)adsl_mem_info[XDATA_REGISTER].address) & 0x0FFFFFFF);

	return DSL_DEV_MEI_ERR_SUCCESS;
}

/* This copies the firmware from secondary storage to 64k memory segment in SDRAM */
DSL_DEV_MeiError_t
DSL_BSP_FWDownload (DSL_DEV_Device_t * pDev, const char *buf,
			 unsigned long size, long *loff, long *current_offset)
{
	ARC_IMG_HDR img_hdr_tmp;
	smmu_mem_info_t *adsl_mem_info = DSL_DEV_PRIVATE(pDev)->adsl_mem_info;

	size_t nRead = 0, nCopy = 0;
	char *mem_ptr;
	char *org_mem_ptr = NULL;
	ssize_t retval = -ENOMEM;
	int idx = 0;

        IFX_MEI_DMSG("\n");

	if (*loff == 0) {
		if (size < sizeof (img_hdr_tmp)) {
			IFX_MEI_EMSG ("Firmware size is too small!\n");
			return retval;
		}
		copy_from_user ((char *) &img_hdr_tmp, buf, sizeof (img_hdr_tmp));
		// header of image_size and crc are not included.
		DSL_DEV_PRIVATE(pDev)->image_size = le32_to_cpu (img_hdr_tmp.size) + 8;

		if (DSL_DEV_PRIVATE(pDev)->image_size > 1024 * 1024) {
			IFX_MEI_EMSG ("Firmware size is too large!\n");
			return retval;
		}
		// check if arc is halt
		IFX_MEI_ResetARC (pDev);
		IFX_MEI_HaltArc (pDev);

		IFX_MEI_DFEMemoryFree (pDev, FREE_ALL);	//free all

		retval = IFX_MEI_DFEMemoryAlloc (pDev,  DSL_DEV_PRIVATE(pDev)->image_size);
		if (retval < 0) {
			IFX_MEI_EMSG ("Error: No memory space left.\n");
			goto error;
		}
		for (idx = 0; idx < retval; idx++) {
			//skip XDATA register
			if (idx == XDATA_REGISTER)
				continue;
			if (idx * SDRAM_SEGMENT_SIZE < le32_to_cpu (img_hdr_tmp.page[0].p_offset))
				adsl_mem_info[idx].type = FREE_RELOAD;
			else
				adsl_mem_info[idx].type = FREE_SHOWTIME;
		}
		DSL_DEV_PRIVATE(pDev)->nBar = retval;

		DSL_DEV_PRIVATE(pDev)->img_hdr =
			(ARC_IMG_HDR *) adsl_mem_info[0].address;

		org_mem_ptr = kmalloc (SDRAM_SEGMENT_SIZE, GFP_KERNEL);
		if (org_mem_ptr == NULL) {
			IFX_MEI_EMSG ("kmalloc memory fail!\n");
			retval = -ENOMEM;
			goto error;
		}
		
		if (((unsigned long)org_mem_ptr) & (1023)) {
			/* Pointer not 1k aligned, so free it and allocate a larger chunk
			 * for further alignment.
			 */
			kfree(org_mem_ptr);
			org_mem_ptr = kmalloc (SDRAM_SEGMENT_SIZE + 1024, GFP_KERNEL);
			if (org_mem_ptr == NULL) {
				IFX_MEI_EMSG ("kmalloc memory fail!\n");
				retval = -ENOMEM;
				goto error;
			}
			adsl_mem_info[XDATA_REGISTER].address =
				(char *) ((unsigned long) (org_mem_ptr + 1023) & ~(1024 -1));
		} else {
			adsl_mem_info[XDATA_REGISTER].address = org_mem_ptr;
		}
		
		adsl_mem_info[XDATA_REGISTER].org_address = org_mem_ptr;
		adsl_mem_info[XDATA_REGISTER].size = SDRAM_SEGMENT_SIZE;

		adsl_mem_info[XDATA_REGISTER].type = FREE_RELOAD;
		IFX_MEI_DMSG("-> IFX_MEI_BarUpdate()\n");
		IFX_MEI_BarUpdate (pDev, (DSL_DEV_PRIVATE(pDev)->nBar));
	}
	else if (DSL_DEV_PRIVATE(pDev)-> image_size == 0) {
		IFX_MEI_EMSG ("Error: Firmware size=0! \n");
		goto error;
	}

	nRead = 0;
	while (nRead < size) {
		long offset = ((long) (*loff) + nRead) % SDRAM_SEGMENT_SIZE;
		idx = (((long) (*loff)) + nRead) / SDRAM_SEGMENT_SIZE;
		mem_ptr = (char *) KSEG1ADDR ((unsigned long) (adsl_mem_info[idx].address) + offset);
		if ((size - nRead + offset) > SDRAM_SEGMENT_SIZE)
			nCopy = SDRAM_SEGMENT_SIZE - offset;
		else
			nCopy = size - nRead;
		copy_from_user (mem_ptr, buf + nRead, nCopy);
		for (offset = 0; offset < (nCopy / 4); offset++) {
			((unsigned long *) mem_ptr)[offset] = le32_to_cpu (((unsigned long *) mem_ptr)[offset]);
		}
		nRead += nCopy;
		adsl_mem_info[idx].nCopy += nCopy;
	}

	*loff += size;
	*current_offset = size;
	return DSL_DEV_MEI_ERR_SUCCESS;
error:
	IFX_MEI_DFEMemoryFree (pDev, FREE_ALL);
	return DSL_DEV_MEI_ERR_FAILURE;
}
/*
 * Register a callback event.
 * Return:
 * -1 if the event already has a callback function registered.
 *  0 success
 */
int DSL_BSP_EventCBRegister(DSL_BSP_EventCallBack_t *p)
{
	if (!p) {
                IFX_MEI_EMSG("Invalid parameter!\n");
                return -EINVAL;
	}
        if (p->event > DSL_BSP_CB_LAST || p->event < DSL_BSP_CB_FIRST) {
                IFX_MEI_EMSG("Invalid Event %d\n", p->event);
                return -EINVAL;
        }
        if (dsl_bsp_event_callback[p->event].function) {
                IFX_MEI_EMSG("Event %d already has a callback function registered!\n", p->event);
                return -1;
        } else {
                dsl_bsp_event_callback[p->event].function = p->function;
                dsl_bsp_event_callback[p->event].event    = p->event;
                dsl_bsp_event_callback[p->event].pData    = p->pData;
        }
        return 0;
}
int DSL_BSP_EventCBUnregister(DSL_BSP_EventCallBack_t *p)
{
	if (!p) {
                IFX_MEI_EMSG("Invalid parameter!\n");
                return -EINVAL;
	}
        if (p->event > DSL_BSP_CB_LAST || p->event < DSL_BSP_CB_FIRST) {
                IFX_MEI_EMSG("Invalid Event %d\n", p->event);
                return -EINVAL;
        }
        if (dsl_bsp_event_callback[p->event].function) {
                IFX_MEI_EMSG("Unregistering Event %d...\n", p->event);
                dsl_bsp_event_callback[p->event].function = NULL;
                dsl_bsp_event_callback[p->event].pData    = NULL;
        } else {
                IFX_MEI_EMSG("Event %d is not registered!\n", p->event);
                return -1;
        }
        return 0;
}

/**
 * MEI Dying Gasp interrupt handler
 *
 * \param int1
 * \param void0
 * \param regs	Pointer to the structure of danube mips registers
 * \ingroup	Internal
 */
/*static irqreturn_t IFX_MEI_Dying_Gasp_IrqHandle (int int1, void *void0)
{
	DSL_DEV_Device_t *pDev = (DSL_DEV_Device_t *) void0;
        DSL_BSP_CB_Type_t event;

	if (pDev == NULL)
		IFX_MEI_EMSG("Error: Got Interrupt but pDev is NULL!!!!\n");

#ifndef CONFIG_SMP
	disable_irq (pDev->nIrq[IFX_DYING_GASP]);
#else
	disable_irq_nosync(pDev->nIrq[IFX_DYING_GASP]);
#endif
	event = DSL_BSP_CB_DYING_GASP;

	if (dsl_bsp_event_callback[event].function)
		(*dsl_bsp_event_callback[event].function)(pDev, event, dsl_bsp_event_callback[event].pData);

#ifdef CONFIG_USE_EMULATOR
    IFX_MEI_EMSG("Dying Gasp! Shutting Down... (Work around for Amazon-S Venus emulator)\n");
#else
	IFX_MEI_EMSG("Dying Gasp! Shutting Down...\n");
//	kill_proc (1, SIGINT, 1);   
#endif
        return IRQ_HANDLED;
}
*/
extern void ifx_usb_enable_afe_oc(void);

/**
 * MEI interrupt handler
 *
 * \param int1
 * \param void0
 * \param regs	Pointer to the structure of danube mips registers
 * \ingroup	Internal
 */
static irqreturn_t IFX_MEI_IrqHandle (int int1, void *void0)
{
	u32 scratch;
	DSL_DEV_Device_t *pDev = (DSL_DEV_Device_t *) void0;
#if defined(CONFIG_LTQ_MEI_FW_LOOPBACK) && defined(DFE_PING_TEST)
	dfe_loopback_irq_handler (pDev);
	return IRQ_HANDLED;
#endif //CONFIG_AMAZON_S_MEI_FW_LOOPBACK
        DSL_BSP_CB_Type_t event;

	if (pDev == NULL)
		IFX_MEI_EMSG("Error: Got Interrupt but pDev is NULL!!!!\n");

	IFX_MEI_DebugRead (pDev, ARC_MEI_MAILBOXR, &scratch, 1);
	if (scratch & OMB_CODESWAP_MESSAGE_MSG_TYPE_MASK) {
		IFX_MEI_EMSG("Receive Code Swap Request interrupt!!!\n");
		return IRQ_HANDLED;
	}
	else if (scratch & OMB_CLEAREOC_INTERRUPT_CODE)	 {
		// clear eoc message interrupt
		IFX_MEI_DMSG("OMB_CLEAREOC_INTERRUPT_CODE\n");
                event = DSL_BSP_CB_CEOC_IRQ;
		IFX_MEI_LongWordWriteOffset (pDev, (u32) ME_ARC2ME_STAT, ARC_TO_MEI_MSGAV);
                if (dsl_bsp_event_callback[event].function)
			(*dsl_bsp_event_callback[event].function)(pDev, event, dsl_bsp_event_callback[event].pData);
        } else if (scratch & OMB_REBOOT_INTERRUPT_CODE) {
                // Reboot
                IFX_MEI_DMSG("OMB_REBOOT_INTERRUPT_CODE\n");
                event = DSL_BSP_CB_FIRMWARE_REBOOT;

		IFX_MEI_LongWordWriteOffset (pDev, (u32) ME_ARC2ME_STAT, ARC_TO_MEI_MSGAV);

                if (dsl_bsp_event_callback[event].function)
                        (*dsl_bsp_event_callback[event].function)(pDev, event, dsl_bsp_event_callback[event].pData);
        } else { // normal message
                IFX_MEI_MailboxRead (pDev, DSL_DEV_PRIVATE(pDev)->CMV_RxMsg, MSG_LENGTH);
                if (DSL_DEV_PRIVATE(pDev)-> cmv_waiting == 1) {
                        DSL_DEV_PRIVATE(pDev)-> arcmsgav = 1;
                        DSL_DEV_PRIVATE(pDev)-> cmv_waiting = 0;
#if !defined(BSP_PORT_RTEMS)
                        MEI_WAKEUP_EVENT (DSL_DEV_PRIVATE(pDev)->wait_queue_arcmsgav);
#endif
                }
		else {
			DSL_DEV_PRIVATE(pDev)-> modem_ready_cnt++;
			memcpy ((char *) DSL_DEV_PRIVATE(pDev)->Recent_indicator,
				(char *) DSL_DEV_PRIVATE(pDev)->CMV_RxMsg, MSG_LENGTH * 2);
			if (((DSL_DEV_PRIVATE(pDev)->CMV_RxMsg[0] & 0xff0) >> 4) == D2H_AUTONOMOUS_MODEM_READY_MSG) {
				//check ARC ready message
				IFX_MEI_DMSG ("Got MODEM_READY_MSG\n");
				DSL_DEV_PRIVATE(pDev)->modem_ready = 1;
				MEI_WAKEUP_EVENT (DSL_DEV_PRIVATE(pDev)->wait_queue_modemready);
			}
		}
	}

	return IRQ_HANDLED;
}

int
DSL_BSP_ATMLedCBRegister (int (*ifx_adsl_ledcallback) (void))
{
    g_adsl_ledcallback = ifx_adsl_ledcallback;
    return 0;
}

int
DSL_BSP_ATMLedCBUnregister (int (*ifx_adsl_ledcallback) (void))
{
    g_adsl_ledcallback = adsl_dummy_ledcallback;
    return 0;
}

#if 0
int
DSL_BSP_EventCBRegister (int (*ifx_adsl_callback)
			        (DSL_BSP_CB_Event_t * param))
{
	int error = 0;

	if (DSL_EventCB == NULL) {
		DSL_EventCB = ifx_adsl_callback;
	}
	else {
		error = -EIO;
	}
	return error;
}

int
DSL_BSP_EventCBUnregister (int (*ifx_adsl_callback)
				  (DSL_BSP_CB_Event_t * param))
{
	int error = 0;

	if (DSL_EventCB == ifx_adsl_callback) {
		DSL_EventCB = NULL;
	}
	else {
		error = -EIO;
	}
	return error;
}

static int
DSL_BSP_GetEventCB (int (**ifx_adsl_callback)
			   (DSL_BSP_CB_Event_t * param))
{
	*ifx_adsl_callback = DSL_EventCB;
	return 0;
}
#endif

#ifdef CONFIG_LTQ_MEI_FW_LOOPBACK
#define mte_reg_base	(0x4800*4+0x20000)

/* Iridia Registers Address Constants */
#define MTE_Reg(r)    	(int)(mte_reg_base + (r*4))

#define IT_AMODE       	MTE_Reg(0x0004)

#define TIMER_DELAY   	(1024)
#define BC0_BYTES     	(32)
#define BC1_BYTES     	(30)
#define NUM_MB        	(12)
#define TIMEOUT_VALUE 	2000

static void
BFMWait (u32 cycle)
{
	u32 i;
	for (i = 0; i < cycle; i++);
}

static void
WriteRegLong (u32 addr, u32 data)
{
	//*((volatile u32 *)(addr)) =  data;
	IFX_MEI_WRITE_REGISTER_L (data, addr);
}

static u32
ReadRegLong (u32 addr)
{
	// u32  rd_val;
	//rd_val = *((volatile u32 *)(addr));
	// return rd_val;
	return IFX_MEI_READ_REGISTER_L (addr);
}

/* This routine writes the mailbox with the data in an input array */
static void
WriteMbox (u32 * mboxarray, u32 size)
{
	IFX_MEI_DebugWrite (&dsl_devices[0], IMBOX_BASE, mboxarray, size);
	IFX_MEI_DMSG("write to %X\n", IMBOX_BASE);
	IFX_MEI_LongWordWriteOffset (&dsl_devices[0], (u32) ME_ME2ARC_INT, MEI_TO_ARC_MSGAV);
}

/* This routine reads the output mailbox and places the results into an array */
static void
ReadMbox (u32 * mboxarray, u32 size)
{
	IFX_MEI_DebugRead (&dsl_devices[0], OMBOX_BASE, mboxarray, size);
	IFX_MEI_DMSG("read from %X\n", OMBOX_BASE);
}

static void
MEIWriteARCValue (u32 address, u32 value)
{
	u32 i, check = 0;

	/* Write address register */
	IFX_MEI_WRITE_REGISTER_L (address,  ME_DBG_WR_AD + LTQ_MEI_BASE_ADDR);

	/* Write data register */
	IFX_MEI_WRITE_REGISTER_L (value, ME_DBG_DATA + LTQ_MEI_BASE_ADDR);

	/* wait until complete - timeout at 40 */
	for (i = 0; i < 40; i++) {
		check = IFX_MEI_READ_REGISTER_L (ME_ARC2ME_STAT + LTQ_MEI_BASE_ADDR);

		if ((check & ARC_TO_MEI_DBG_DONE))
			break;
	}
	/* clear the flag */
	IFX_MEI_WRITE_REGISTER_L (ARC_TO_MEI_DBG_DONE, ME_ARC2ME_STAT + LTQ_MEI_BASE_ADDR);
}

void
arc_code_page_download (uint32_t arc_code_length, uint32_t * start_address)
{
	int count;

	IFX_MEI_DMSG("try to download pages,size=%d\n", arc_code_length);
	IFX_MEI_ControlModeSet (&dsl_devices[0], MEI_MASTER_MODE);
	IFX_MEI_HaltArc (&dsl_devices[0]);
	IFX_MEI_LongWordWriteOffset (&dsl_devices[0], (u32) ME_DX_AD, 0);
	for (count = 0; count < arc_code_length; count++) {
		IFX_MEI_LongWordWriteOffset (&dsl_devices[0], (u32) ME_DX_DATA,
						   *(start_address + count));
	}
	IFX_MEI_ControlModeSet (&dsl_devices[0], JTAG_MASTER_MODE);
}
static int
load_jump_table (unsigned long addr)
{
	int i;
	uint32_t addr_le, addr_be;
	uint32_t jump_table[32];

	for (i = 0; i < 16; i++) {
		addr_le = i * 8 + addr;
		addr_be = ((addr_le >> 16) & 0xffff);
		addr_be |= ((addr_le & 0xffff) << 16);
		jump_table[i * 2 + 0] = 0x0f802020;
		jump_table[i * 2 + 1] = addr_be;
		//printk("jt %X %08X %08X\n",i,jump_table[i*2+0],jump_table[i*2+1]);
	}
	arc_code_page_download (32, &jump_table[0]);
return 0;
}

int got_int = 0;

void
dfe_loopback_irq_handler (DSL_DEV_Device_t *pDev)
{
	uint32_t rd_mbox[10];

	memset (&rd_mbox[0], 0, 10 * 4);
	ReadMbox (&rd_mbox[0], 6);
	if (rd_mbox[0] == 0x0) {
		FX_MEI_DMSG("Get ARC_ACK\n");
		got_int = 1;
	}
	else if (rd_mbox[0] == 0x5) {
		IFX_MEI_DMSG("Get ARC_BUSY\n");
		got_int = 2;
	}
	else if (rd_mbox[0] == 0x3) {
		IFX_MEI_DMSG("Get ARC_EDONE\n");
		if (rd_mbox[1] == 0x0) {
			got_int = 3;
			IFX_MEI_DMSG("Get E_MEMTEST\n");
			if (rd_mbox[2] != 0x1) {
				got_int = 4;
				IFX_MEI_DMSG("Get Result %X\n", rd_mbox[2]);
			}
		}
	}
	IFX_MEI_LongWordWriteOffset (&dsl_devices[0], (u32) ME_ARC2ME_STAT,
		ARC_TO_MEI_DBG_DONE);
	MEI_MASK_AND_ACK_IRQ (pDev->nIrq[IFX_DFEIR]);
	disable_irq (pDev->nIrq[IFX_DFEIR]);
	//got_int = 1;
	return;
}

static void
wait_mem_test_result (void)
{
	uint32_t mbox[5];
	mbox[0] = 0;

	IFX_MEI_DMSG("Waiting Starting\n");
	while (mbox[0] == 0) {
		ReadMbox (&mbox[0], 5);
	}
	IFX_MEI_DMSG("Try to get mem test result.\n");
	ReadMbox (&mbox[0], 5);
	if (mbox[0] == 0xA) {
		IFX_MEI_DMSG("Success.\n");
	}
	else if (mbox[0] == 0xA) {
		IFX_MEI_EMSG("Fail,address %X,except data %X,receive data %X\n",
			mbox[1], mbox[2], mbox[3]);
	}
	else {
		IFX_MEI_EMSG("Fail\n");
	}
}

static int
arc_ping_testing (DSL_DEV_Device_t *pDev)
{
#define MEI_PING 0x00000001
	uint32_t wr_mbox[10], rd_mbox[10];
	int i;

	for (i = 0; i < 10; i++) {
		wr_mbox[i] = 0;
		rd_mbox[i] = 0;
	}

	FX_MEI_DMSG("send ping msg\n");
	wr_mbox[0] = MEI_PING;
	WriteMbox (&wr_mbox[0], 10);

	while (got_int == 0) {
		MEI_WAIT (100);
	}

	IFX_MEI_DMSG("send start event\n");
	got_int = 0;

	wr_mbox[0] = 0x4;
	wr_mbox[1] = 0;
	wr_mbox[2] = 0;
	wr_mbox[3] = (uint32_t) 0xf5acc307e;
	wr_mbox[4] = 5;
	wr_mbox[5] = 2;
	wr_mbox[6] = 0x1c000;
	wr_mbox[7] = 64;
	wr_mbox[8] = 0;
	wr_mbox[9] = 0;
	WriteMbox (&wr_mbox[0], 10);
	DSL_ENABLE_IRQ (pDev->nIrq[IFX_DFEIR]);
	//printk("IFX_MEI_MailboxWrite ret=%d\n",i);
	IFX_MEI_LongWordWriteOffset (&dsl_devices[0],
					   (u32) ME_ME2ARC_INT,
					   MEI_TO_ARC_MSGAV);
	IFX_MEI_DMSG("sleeping\n");
	while (1) {
		if (got_int > 0) {

			if (got_int > 3)
				IFX_MEI_DMSG("got_int >>>> 3\n");
			else
				IFX_MEI_DMSG("got int = %d\n", got_int);
			got_int = 0;
			//schedule();
			DSL_ENABLE_IRQ (pDev->nIrq[IFX_DFEIR]);
		}
		//mbox_read(&rd_mbox[0],6);
		MEI_WAIT (100);
	}
	return 0;
}

static DSL_DEV_MeiError_t
DFE_Loopback_Test (void)
{
	int i = 0;
	u32 arc_debug_data = 0, temp;
	DSL_DEV_Device_t *pDev = &dsl_devices[0];
	uint32_t wr_mbox[10];

	IFX_MEI_ResetARC (pDev);
	// start the clock
	arc_debug_data = ACL_CLK_MODE_ENABLE;
	IFX_MEI_DebugWrite (pDev, CRI_CCR0, &arc_debug_data, 1);

#if defined( DFE_PING_TEST )|| defined( DFE_ATM_LOOPBACK)
	// WriteARCreg(AUX_XMEM_LTEST,0);
	IFX_MEI_ControlModeSet (pDev, MEI_MASTER_MODE);
#define AUX_XMEM_LTEST 0x128
	_IFX_MEI_DBGLongWordWrite (pDev, MEI_DEBUG_DEC_AUX_MASK,  AUX_XMEM_LTEST, 0);
	IFX_MEI_ControlModeSet (pDev, JTAG_MASTER_MODE);

	// WriteARCreg(AUX_XDMA_GAP,0);
	IFX_MEI_ControlModeSet (pDev, MEI_MASTER_MODE);
#define AUX_XDMA_GAP 0x114
	_IFX_MEI_DBGLongWordWrite (pDev, MEI_DEBUG_DEC_AUX_MASK, AUX_XDMA_GAP, 0);
	IFX_MEI_ControlModeSet (pDev, JTAG_MASTER_MODE);

	IFX_MEI_ControlModeSet (pDev, MEI_MASTER_MODE);
	temp = 0;
	_IFX_MEI_DBGLongWordWrite (pDev, MEI_DEBUG_DEC_AUX_MASK,
		(u32) ME_XDATA_BASE_SH +  LTQ_MEI_BASE_ADDR, temp);
	IFX_MEI_ControlModeSet (pDev, JTAG_MASTER_MODE);

	i = IFX_MEI_DFEMemoryAlloc (pDev, SDRAM_SEGMENT_SIZE * 16);
	if (i >= 0) {
		int idx;

		for (idx = 0; idx < i; idx++) {
			DSL_DEV_PRIVATE(pDev)->adsl_mem_info[idx].type = FREE_RELOAD;
			IFX_MEI_WRITE_REGISTER_L ((((uint32_t) DSL_DEV_PRIVATE(pDev)->adsl_mem_info[idx].address) & 0x0fffffff),
							LTQ_MEI_BASE_ADDR + ME_XMEM_BAR_BASE  + idx * 4);
			IFX_MEI_DMSG("bar%d(%X)=%X\n", idx,
				LTQ_MEI_BASE_ADDR + ME_XMEM_BAR_BASE  +
				idx * 4, (((uint32_t)
					   ((ifx_mei_device_private_t *)
					    pDev->pPriv)->adsl_mem_info[idx].
					   address) & 0x0fffffff));
			memset ((u8 *) DSL_DEV_PRIVATE(pDev)->adsl_mem_info[idx].address, 0, SDRAM_SEGMENT_SIZE);
		}

		IFX_MEI_LongWordWriteOffset (pDev, (u32) ME_XDATA_BASE_SH,
					   ((unsigned long) DSL_DEV_PRIVATE(pDev)->adsl_mem_info[XDATA_REGISTER].address) & 0x0FFFFFFF);
	}
	else {
		IFX_MEI_EMSG ("cannot load image: no memory\n");
		return DSL_DEV_MEI_ERR_FAILURE;
	}
	//WriteARCreg(AUX_IC_CTRL,2);
	IFX_MEI_DMSG("Setting MEI_MASTER_MODE..\n");
	IFX_MEI_ControlModeSet (pDev, MEI_MASTER_MODE);
#define AUX_IC_CTRL 0x11
	_IFX_MEI_DBGLongWordWrite (pDev, MEI_DEBUG_DEC_AUX_MASK,
					 AUX_IC_CTRL, 2);
	IFX_MEI_DMSG("Setting JTAG_MASTER_MODE..\n");
	IFX_MEI_ControlModeSet (pDev, JTAG_MASTER_MODE);

	IFX_MEI_DMSG("Halting ARC...\n");
	IFX_MEI_HaltArc (&dsl_devices[0]);

#ifdef DFE_PING_TEST

	IFX_MEI_DMSG("ping test image size=%d\n", sizeof (arc_ahb_access_code));
	memcpy ((u8 *) (DSL_DEV_PRIVATE(pDev)->
			adsl_mem_info[0].address + 0x1004),
		&arc_ahb_access_code[0], sizeof (arc_ahb_access_code));
	load_jump_table (0x80000 + 0x1004);

#endif //DFE_PING_TEST

	IFX_MEI_DMSG("ARC ping test code download complete\n");
#endif //defined( DFE_PING_TEST )|| defined( DFE_ATM_LOOPBACK)
#ifdef DFE_MEM_TEST
	IFX_MEI_LongWordWriteOffset (&dsl_devices[0], (u32) ME_ARC2ME_MASK, MSGAV_EN);

	arc_code_page_download (1537, &code_array[0]);
	IFX_MEI_DMSG("ARC mem test code download complete\n");
#endif //DFE_MEM_TEST
#ifdef DFE_ATM_LOOPBACK
	arc_debug_data = 0xf;
	arc_code_page_download (sizeof(code_array) / sizeof(*code_array), &code_array[0]);
	wr_mbox[0] = 0;	    //TIMER_DELAY   - org: 1024
	wr_mbox[1] = 0;		//TXFB_START0
	wr_mbox[2] = 0x7f;	//TXFB_END0     - org: 49
	wr_mbox[3] = 0x80;	//TXFB_START1   - org: 80
	wr_mbox[4] = 0xff;	//TXFB_END1     - org: 109
	wr_mbox[5] = 0x100;	//RXFB_START0   - org: 0
	wr_mbox[6] = 0x17f;	//RXFB_END0     - org: 49
	wr_mbox[7] = 0x180;	//RXFB_START1   - org: 256
	wr_mbox[8] = 0x1ff;	//RXFB_END1     - org: 315
	WriteMbox (&wr_mbox[0], 9);
	// Start Iridia IT_AMODE (in dmp access) why is it required?
	IFX_MEI_DebugWrite (&dsl_devices[0], 0x32010, &arc_debug_data, 1);
#endif //DFE_ATM_LOOPBACK
	IFX_MEI_IRQEnable (pDev);
	IFX_MEI_DMSG("run ARC...\n");
	IFX_MEI_RunArc (&dsl_devices[0]);

#ifdef DFE_PING_TEST
	arc_ping_testing (pDev);
#endif //DFE_PING_TEST
#ifdef DFE_MEM_TEST
	wait_mem_test_result ();
#endif //DFE_MEM_TEST

	IFX_MEI_DFEMemoryFree (pDev, FREE_ALL);
	return DSL_DEV_MEI_ERR_SUCCESS;
}

#endif //CONFIG_AMAZON_S_MEI_FW_LOOPBACK

static int
IFX_MEI_InitDevNode (int num)
{
	if (num == 0) {
		if ((dev_major = register_chrdev (dev_major, IFX_MEI_DEVNAME, &bsp_mei_operations)) < 0) {
			IFX_MEI_EMSG ("register_chrdev(%d %s) failed!\n", dev_major, IFX_MEI_DEVNAME);
			return -ENODEV;
		}
	}
	return 0;
}

static int
IFX_MEI_CleanUpDevNode (int num)
{
	if (num == 0)
		unregister_chrdev (dev_major, MEI_DIRNAME);
	return 0;
}

static int
IFX_MEI_InitDevice (int num)
{
	DSL_DEV_Device_t *pDev;
        u32 temp;
	pDev = &dsl_devices[num];
	if (pDev == NULL)
		return -ENOMEM;
	pDev->pPriv = &sDanube_Mei_Private[num];
	memset (pDev->pPriv, 0, sizeof (ifx_mei_device_private_t));

	memset (&DSL_DEV_PRIVATE(pDev)->
		adsl_mem_info[0], 0,
		sizeof (smmu_mem_info_t) * MAX_BAR_REGISTERS);

	if (num == 0) {
		pDev->nIrq[IFX_DFEIR]      = LTQ_MEI_INT;
		pDev->nIrq[IFX_DYING_GASP] = LTQ_MEI_DYING_GASP_INT;
		pDev->base_address = KSEG1 + LTQ_MEI_BASE_ADDR;

                /* Power up MEI */
#ifdef CONFIG_LANTIQ_AMAZON_SE
		*LTQ_PMU_PWDCR &= ~(1 << 9);  // enable dsl
		*LTQ_PMU_PWDCR &= ~(1 << 15); // enable AHB base
#endif
		temp = ltq_r32(LTQ_PMU_PWDCR);
		temp &= 0xffff7dbe;
		ltq_w32(temp, LTQ_PMU_PWDCR);
	}
	pDev->nInUse = 0;
	DSL_DEV_PRIVATE(pDev)->modem_ready = 0;
	DSL_DEV_PRIVATE(pDev)->arcmsgav = 0;

	MEI_INIT_WAKELIST ("arcq", DSL_DEV_PRIVATE(pDev)->wait_queue_arcmsgav);	// for ARCMSGAV
	MEI_INIT_WAKELIST ("arcr", DSL_DEV_PRIVATE(pDev)->wait_queue_modemready);	// for arc modem ready

	MEI_MUTEX_INIT (DSL_DEV_PRIVATE(pDev)->mei_cmv_sema, 1);	// semaphore initialization, mutex
#if 0
	MEI_MASK_AND_ACK_IRQ (pDev->nIrq[IFX_DFEIR]);
	MEI_MASK_AND_ACK_IRQ (pDev->nIrq[IFX_DYING_GASP]);
#endif
	if (request_irq (pDev->nIrq[IFX_DFEIR], IFX_MEI_IrqHandle, 0, "DFEIR", pDev) != 0) {
		IFX_MEI_EMSG ("request_irq %d failed!\n", pDev->nIrq[IFX_DFEIR]);
		return -1;
	}
	/*if (request_irq (pDev->nIrq[IFX_DYING_GASP], IFX_MEI_Dying_Gasp_IrqHandle, 0, "DYING_GASP", pDev) != 0) {
		IFX_MEI_EMSG ("request_irq %d failed!\n", pDev->nIrq[IFX_DYING_GASP]);
		return -1;
	}*/
//	IFX_MEI_DMSG("Device %d initialized. IER %#x\n", num, bsp_get_irq_ier(pDev->nIrq[IFX_DYING_GASP]));
	return 0;
}

static int
IFX_MEI_ExitDevice (int num)
{
	DSL_DEV_Device_t *pDev;
	pDev = &dsl_devices[num];

	if (pDev == NULL)
		return -EIO;

	disable_irq (pDev->nIrq[IFX_DFEIR]);
	disable_irq (pDev->nIrq[IFX_DYING_GASP]);

	free_irq(pDev->nIrq[IFX_DFEIR], pDev);
	free_irq(pDev->nIrq[IFX_DYING_GASP], pDev);

	return 0;
}

static DSL_DEV_Device_t *
IFX_BSP_HandleGet (int maj, int num)
{
	if (num > BSP_MAX_DEVICES)
		return NULL;
	return &dsl_devices[num];
}

DSL_DEV_Device_t *
DSL_BSP_DriverHandleGet (int maj, int num)
{
	DSL_DEV_Device_t *pDev;

	if (num > BSP_MAX_DEVICES)
		return NULL;

	pDev = &dsl_devices[num];
	if (!try_module_get(pDev->owner))
		return NULL;

	pDev->nInUse++;
	return pDev;
}

int
DSL_BSP_DriverHandleDelete (DSL_DEV_Device_t * nHandle)
{
	DSL_DEV_Device_t *pDev = (DSL_DEV_Device_t *) nHandle;
	if (pDev->nInUse)
		pDev->nInUse--;
        module_put(pDev->owner);
	return 0;
}

static int
IFX_MEI_Open (DSL_DRV_inode_t * ino, DSL_DRV_file_t * fil)
{
	int maj = MAJOR (ino->i_rdev);
	int num = MINOR (ino->i_rdev);

	DSL_DEV_Device_t *pDev = NULL;
	if ((pDev = DSL_BSP_DriverHandleGet (maj, num)) == NULL) {
		IFX_MEI_EMSG("open(%d:%d) fail!\n", maj, num);
		return -EIO;
	}
	fil->private_data = pDev;
	return 0;
}

static int
IFX_MEI_Release (DSL_DRV_inode_t * ino, DSL_DRV_file_t * fil)
{
	//int maj = MAJOR(ino->i_rdev);
	int num = MINOR (ino->i_rdev);
	DSL_DEV_Device_t *pDev;

	pDev = &dsl_devices[num];
	if (pDev == NULL)
		return -EIO;
	DSL_BSP_DriverHandleDelete (pDev);
	return 0;
}

/**
 * Callback function for linux userspace program writing
 */
static ssize_t
IFX_MEI_Write (DSL_DRV_file_t * filp, const char *buf, size_t size, loff_t * loff)
{
	DSL_DEV_MeiError_t mei_error = DSL_DEV_MEI_ERR_FAILURE;
	long offset = 0;
	DSL_DEV_Device_t *pDev = (DSL_DEV_Device_t *) filp->private_data;

	if (pDev == NULL)
		return -EIO;

	mei_error =
		DSL_BSP_FWDownload (pDev, buf, size, (long *) loff,  &offset);

	if (mei_error == DSL_DEV_MEI_ERR_FAILURE)
		return -EIO;
	return (ssize_t) offset;
}

/**
 * Callback function for linux userspace program ioctling
 */
static int
IFX_MEI_IoctlCopyFrom (int from_kernel, char *dest, char *from, int size)
{
	int ret = 0;

	if (!from_kernel)
		ret = copy_from_user ((char *) dest, (char *) from, size);
	else
		ret = (int)memcpy ((char *) dest, (char *) from, size);
	return ret;
}

static int
IFX_MEI_IoctlCopyTo (int from_kernel, char *dest, char *from, int size)
{
	int ret = 0;

	if (!from_kernel)
		ret = copy_to_user ((char *) dest, (char *) from, size);
	else
		ret = (int)memcpy ((char *) dest, (char *) from, size);
	return ret;
}

int
IFX_MEI_Ioctls (DSL_DEV_Device_t * pDev, int from_kernel, unsigned int command, unsigned long lon)
{
	int i = 0;
	int meierr = DSL_DEV_MEI_ERR_SUCCESS;
	u32 base_address = LTQ_MEI_BASE_ADDR;
	DSL_DEV_WinHost_Message_t winhost_msg, m;
//	DSL_DEV_MeiDebug_t debugrdwr;
	DSL_DEV_MeiReg_t regrdwr;

	switch (command) {

	case DSL_FIO_BSP_CMV_WINHOST:
		IFX_MEI_IoctlCopyFrom (from_kernel, (char *) winhost_msg.msg.TxMessage,
					     (char *) lon, MSG_LENGTH * 2);

		if ((meierr = DSL_BSP_SendCMV (pDev, winhost_msg.msg.TxMessage, YES_REPLY,
					   winhost_msg.msg.RxMessage)) != DSL_DEV_MEI_ERR_SUCCESS) {
			IFX_MEI_EMSG ("WINHOST CMV fail :TxMessage:%X %X %X %X, RxMessage:%X %X %X %X %X\n",
				 winhost_msg.msg.TxMessage[0], winhost_msg.msg.TxMessage[1], winhost_msg.msg.TxMessage[2], winhost_msg.msg.TxMessage[3],
				 winhost_msg.msg.RxMessage[0], winhost_msg.msg.RxMessage[1], winhost_msg.msg.RxMessage[2], winhost_msg.msg.RxMessage[3],
				 winhost_msg.msg.RxMessage[4]);
			meierr = DSL_DEV_MEI_ERR_FAILURE;
		}
		else {
			IFX_MEI_IoctlCopyTo (from_kernel, (char *) lon,
						   (char *) winhost_msg.msg.RxMessage,
						   MSG_LENGTH * 2);
		}
		break;

	case DSL_FIO_BSP_CMV_READ:
		IFX_MEI_IoctlCopyFrom (from_kernel, (char *) (&regrdwr),
					     (char *) lon, sizeof (DSL_DEV_MeiReg_t));

		IFX_MEI_LongWordRead ((u32) regrdwr.iAddress,
					    (u32 *) & (regrdwr.iData));

		IFX_MEI_IoctlCopyTo (from_kernel, (char *) lon,
					   (char *) (&regrdwr),
					   sizeof (DSL_DEV_MeiReg_t));

		break;

	case DSL_FIO_BSP_CMV_WRITE:
		IFX_MEI_IoctlCopyFrom (from_kernel, (char *) (&regrdwr),
					     (char *) lon, sizeof (DSL_DEV_MeiReg_t));

		IFX_MEI_LongWordWrite ((u32) regrdwr.iAddress,
					     regrdwr.iData);
		break;

	case DSL_FIO_BSP_GET_BASE_ADDRESS:
		IFX_MEI_IoctlCopyTo (from_kernel, (char *) lon,
					   (char *) (&base_address),
					   sizeof (base_address));
		break;

	case DSL_FIO_BSP_IS_MODEM_READY:
		i = IFX_MEI_IsModemReady (pDev);
		IFX_MEI_IoctlCopyTo (from_kernel, (char *) lon,
					   (char *) (&i), sizeof (int));
		meierr = DSL_DEV_MEI_ERR_SUCCESS;
		break;
	case DSL_FIO_BSP_RESET:
	case DSL_FIO_BSP_REBOOT:
		meierr = IFX_MEI_CpuModeSet (pDev, DSL_CPU_RESET);
		meierr = IFX_MEI_CpuModeSet (pDev, DSL_CPU_HALT);
		break;

	case DSL_FIO_BSP_HALT:
		meierr = IFX_MEI_CpuModeSet (pDev, DSL_CPU_HALT);
		break;

	case DSL_FIO_BSP_RUN:
		meierr = IFX_MEI_CpuModeSet (pDev, DSL_CPU_RUN);
		break;
	case DSL_FIO_BSP_BOOTDOWNLOAD:
		meierr = IFX_MEI_DownloadBootCode (pDev);
		break;
	case DSL_FIO_BSP_JTAG_ENABLE:
		meierr = IFX_MEI_ArcJtagEnable (pDev, 1);
		break;

	case DSL_FIO_BSP_REMOTE:
		IFX_MEI_IoctlCopyFrom (from_kernel, (char *) (&i),
					     (char *) lon, sizeof (int));

		meierr = IFX_MEI_AdslMailboxIRQEnable (pDev, i);
		break;

	case DSL_FIO_BSP_DSL_START:
		IFX_MEI_DMSG("DSL_FIO_BSP_DSL_START\n");
		if ((meierr = IFX_MEI_RunAdslModem (pDev)) != DSL_DEV_MEI_ERR_SUCCESS) {
			IFX_MEI_EMSG ("IFX_MEI_RunAdslModem() error...");
			meierr = DSL_DEV_MEI_ERR_FAILURE;
		}
		break;

/*	case DSL_FIO_BSP_DEBUG_READ:
	case DSL_FIO_BSP_DEBUG_WRITE:
		IFX_MEI_IoctlCopyFrom (from_kernel,
					     (char *) (&debugrdwr),
					     (char *) lon,
					     sizeof (debugrdwr));

		if (command == DSL_FIO_BSP_DEBUG_READ)
			meierr = DSL_BSP_MemoryDebugAccess (pDev,
								 DSL_BSP_MEMORY_READ,
								 debugrdwr.
								 iAddress,
								 debugrdwr.
								 buffer,
								 debugrdwr.
								 iCount);
		else
			meierr = DSL_BSP_MemoryDebugAccess (pDev,
								 DSL_BSP_MEMORY_WRITE,
								 debugrdwr.
								 iAddress,
								 debugrdwr.
								 buffer,
								 debugrdwr.
								 iCount);

		IFX_MEI_IoctlCopyTo (from_kernel, (char *) lon, (char *) (&debugrdwr), sizeof (debugrdwr));
		break;*/
	case DSL_FIO_BSP_GET_VERSION:
		IFX_MEI_IoctlCopyTo (from_kernel, (char *) lon, (char *) (&bsp_mei_version), sizeof (DSL_DEV_Version_t));
		break;

#define LTQ_MPS_CHIPID_VERSION_GET(value)  (((value) >> 28) & ((1 << 4) - 1))
	case DSL_FIO_BSP_GET_CHIP_INFO:
                bsp_chip_info.major = 1;
                bsp_chip_info.minor = LTQ_MPS_CHIPID_VERSION_GET(*LTQ_MPS_CHIPID);
                IFX_MEI_IoctlCopyTo (from_kernel, (char *) lon, (char *) (&bsp_chip_info), sizeof (DSL_DEV_HwVersion_t));
                meierr = DSL_DEV_MEI_ERR_SUCCESS;
		break;

        case DSL_FIO_BSP_FREE_RESOURCE:
                makeCMV (H2D_CMV_READ, DSL_CMV_GROUP_STAT, 4, 0, 1, NULL, m.msg.TxMessage);
                if (DSL_BSP_SendCMV (pDev, m.msg.TxMessage, YES_REPLY, m.msg.RxMessage) != DSL_DEV_MEI_ERR_SUCCESS) {
                        meierr = DSL_DEV_MEI_ERR_FAILURE;
                        return -EIO;
                }
                IFX_MEI_DMSG("RxMessage[4] = %#x\n", m.msg.RxMessage[4]);
                if (!(m.msg.RxMessage[4] & DSL_DEV_STAT_CODESWAP_COMPLETE)) {
                        meierr = DSL_DEV_MEI_ERR_FAILURE;
                        return -EAGAIN;
                }
                IFX_MEI_DMSG("Freeing all memories marked FREE_SHOWTIME\n");
                IFX_MEI_DFEMemoryFree (pDev, FREE_SHOWTIME);
                meierr = DSL_DEV_MEI_ERR_SUCCESS;
		break;
#ifdef CONFIG_IFXMIPS_AMAZON_SE
	case DSL_FIO_ARC_MUX_TEST:
		AMAZON_SE_MEI_ARC_MUX_Test();
		break;
#endif
	default:
//		IFX_MEI_EMSG("Invalid IOCTL command: %d\n");
		break;
	}
	return meierr;
}

#ifdef CONFIG_IFXMIPS_AMAZON_SE
void AMAZON_SE_MEI_ARC_MUX_Test(void)
{
	u32 *p, i;
	*LTQ_RCU_RST |= LTQ_RCU_RST_REQ_MUX_ARC;

	p = (u32*)(DFE_LDST_BASE_ADDR + IRAM0_BASE);
	IFX_MEI_EMSG("Writing to IRAM0(%p)...\n", p);
	for (i = 0; i < IRAM0_SIZE/sizeof(u32); i++, p++) {
		*p = 0xdeadbeef;
		if (*p != 0xdeadbeef)
			IFX_MEI_EMSG("%p: %#x\n", p, *p);
	}

	p = (u32*)(DFE_LDST_BASE_ADDR + IRAM1_BASE);
	IFX_MEI_EMSG("Writing to IRAM1(%p)...\n", p);
	for (i = 0; i < IRAM1_SIZE/sizeof(u32); i++, p++) {
		*p = 0xdeadbeef;
		if (*p != 0xdeadbeef)
			IFX_MEI_EMSG("%p: %#x\n", p, *p);
	}

	p = (u32*)(DFE_LDST_BASE_ADDR + BRAM_BASE);
	IFX_MEI_EMSG("Writing to BRAM(%p)...\n", p);
	for (i = 0; i < BRAM_SIZE/sizeof(u32); i++, p++) {
		*p = 0xdeadbeef;
		if (*p != 0xdeadbeef)
			IFX_MEI_EMSG("%p: %#x\n", p, *p);
	}

	p = (u32*)(DFE_LDST_BASE_ADDR + XRAM_BASE);
	IFX_MEI_EMSG("Writing to XRAM(%p)...\n", p);
	for (i = 0; i < XRAM_SIZE/sizeof(u32); i++, p++) {
		*p = 0xdeadbeef;
		if (*p != 0xdeadbeef)
			IFX_MEI_EMSG("%p: %#x\n", p, *p);
	}

	p = (u32*)(DFE_LDST_BASE_ADDR + YRAM_BASE);
	IFX_MEI_EMSG("Writing to YRAM(%p)...\n", p);
	for (i = 0; i < YRAM_SIZE/sizeof(u32); i++, p++) {
		*p = 0xdeadbeef;
		if (*p != 0xdeadbeef)
			IFX_MEI_EMSG("%p: %#x\n", p, *p);
	}

	p = (u32*)(DFE_LDST_BASE_ADDR + EXT_MEM_BASE);
	IFX_MEI_EMSG("Writing to EXT_MEM(%p)...\n", p);
	for (i = 0; i < EXT_MEM_SIZE/sizeof(u32); i++, p++) {
		*p = 0xdeadbeef;
		if (*p != 0xdeadbeef)
			IFX_MEI_EMSG("%p: %#x\n", p, *p);
	}
	*LTQ_RCU_RST &= ~LTQ_RCU_RST_REQ_MUX_ARC;
}
#endif
int
DSL_BSP_KernelIoctls (DSL_DEV_Device_t * pDev, unsigned int command,
			   unsigned long lon)
{
	int error = 0;

	error = IFX_MEI_Ioctls (pDev, 1, command, lon);
	return error;
}

static long
IFX_MEI_UserIoctls (DSL_DRV_file_t * fil,
			  unsigned int command, unsigned long lon)
{
	int error = 0;
	DSL_DEV_Device_t *pDev;

	pDev = IFX_BSP_HandleGet (0, 0);
	if (pDev == NULL)
		return -EIO;

	error = IFX_MEI_Ioctls (pDev, 0, command, lon);
	return error;
}

static int adsl_dummy_ledcallback(void)
{
    return 0;
}

int ifx_mei_atm_led_blink(void)
{
    return g_adsl_ledcallback();
}
EXPORT_SYMBOL(ifx_mei_atm_led_blink);

int ifx_mei_atm_showtime_check(int *is_showtime, struct port_cell_info *port_cell, void **xdata_addr)
{
    int i;

    if ( is_showtime ) {
        *is_showtime = g_tx_link_rate[0] == 0 && g_tx_link_rate[1] == 0 ? 0 : 1;
    }

    if ( port_cell ) {
        for ( i = 0; i < port_cell->port_num && i < 2; i++ )
            port_cell->tx_link_rate[i] = g_tx_link_rate[i];
    }

    if ( xdata_addr ) {
        if ( g_tx_link_rate[0] == 0 && g_tx_link_rate[1] == 0 )
            *xdata_addr = NULL;
        else
            *xdata_addr = g_xdata_addr;
    }

    return 0;
}
EXPORT_SYMBOL(ifx_mei_atm_showtime_check);

/*
 * Writing function for linux proc filesystem
 */
static int ltq_mei_probe(struct platform_device *pdev)
{
	int i = 0;
	static struct class *dsl_class;

	pr_info("IFX MEI Version %ld.%02ld.%02ld\n", bsp_mei_version.major, bsp_mei_version.minor, bsp_mei_version.revision);

	for (i = 0; i < BSP_MAX_DEVICES; i++) {
		if (IFX_MEI_InitDevice (i) != 0) {
			IFX_MEI_EMSG("Init device fail!\n");
			return -EIO;
		}
		IFX_MEI_InitDevNode (i);
	}
		for (i = 0; i <= DSL_BSP_CB_LAST ; i++)
		dsl_bsp_event_callback[i].function = NULL;

#ifdef CONFIG_LTQ_MEI_FW_LOOPBACK
	IFX_MEI_DMSG("Start loopback test...\n");
	DFE_Loopback_Test ();
#endif
	dsl_class = class_create(THIS_MODULE, "ifx_mei");
	device_create(dsl_class, NULL, MKDEV(MEI_MAJOR, 0), NULL, "ifx_mei");
	return 0;
}

static int ltq_mei_remove(struct platform_device *pdev)
{
	int i = 0;
	int num;

	for (num = 0; num < BSP_MAX_DEVICES; num++) {
		IFX_MEI_CleanUpDevNode (num);
	}

	for (i = 0; i < BSP_MAX_DEVICES; i++) {
		for (i = 0; i < BSP_MAX_DEVICES; i++) {
			IFX_MEI_ExitDevice (i);
		}
	}
	return 0;
}

static const struct of_device_id ltq_mei_match[] = {
	{ .compatible = "lantiq,mei-xway"},
	{},
};

static struct platform_driver ltq_mei_driver = {
	.probe = ltq_mei_probe,
	.remove = ltq_mei_remove,
	.driver = {
		.name = "lantiq,mei-xway",
		.owner = THIS_MODULE,
		.of_match_table = ltq_mei_match,
	},
};

module_platform_driver(ltq_mei_driver);

/* export function for DSL Driver */

/* The functions of MEI_DriverHandleGet and MEI_DriverHandleDelete are
something like open/close in kernel space , where the open could be used
to register a callback for autonomous messages and returns a mei driver context pointer (comparable to the file descriptor in user space)
   The context will be required for the multi line chips future! */

EXPORT_SYMBOL (DSL_BSP_DriverHandleGet);
EXPORT_SYMBOL (DSL_BSP_DriverHandleDelete);

EXPORT_SYMBOL (DSL_BSP_ATMLedCBRegister);
EXPORT_SYMBOL (DSL_BSP_ATMLedCBUnregister);
EXPORT_SYMBOL (DSL_BSP_KernelIoctls);
EXPORT_SYMBOL (DSL_BSP_AdslLedInit);
//EXPORT_SYMBOL (DSL_BSP_AdslLedSet);
EXPORT_SYMBOL (DSL_BSP_FWDownload);
EXPORT_SYMBOL (DSL_BSP_Showtime);

EXPORT_SYMBOL (DSL_BSP_MemoryDebugAccess);
EXPORT_SYMBOL (DSL_BSP_SendCMV);

// provide a register/unregister function for DSL driver to register a event callback function
EXPORT_SYMBOL (DSL_BSP_EventCBRegister);
EXPORT_SYMBOL (DSL_BSP_EventCBUnregister);

MODULE_LICENSE("Dual BSD/GPL");
