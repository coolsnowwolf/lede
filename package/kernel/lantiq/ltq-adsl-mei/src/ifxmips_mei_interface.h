/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef IFXMIPS_MEI_H
#define IFXMIPS_MEI_H

//#define CONFIG_AMAZON_SE 1
//#define CONFIG_DANUBE 1
//#define CONFIG_AR9 1

#if !defined(CONFIG_DANUBE) && !defined(CONFIG_AMAZON_SE) && !defined(CONFIG_AR9) && !defined(CONFIG_VR9)
#error Platform undefined!!!
#endif

#ifdef IFX_MEI_BSP
/** This is the character datatype. */
typedef char            DSL_char_t;
/** This is the unsigned 8-bit datatype. */
typedef unsigned char   DSL_uint8_t;
/** This is the signed 8-bit datatype. */
typedef signed char     DSL_int8_t;
/** This is the unsigned 16-bit datatype. */
typedef unsigned short  DSL_uint16_t;
/** This is the signed 16-bit datatype. */
typedef signed short    DSL_int16_t;
/** This is the unsigned 32-bit datatype. */
typedef unsigned long   DSL_uint32_t;
/** This is the signed 32-bit datatype. */
typedef signed long     DSL_int32_t;
/** This is the float datatype. */
typedef float           DSL_float_t;
/** This is the void datatype. */
typedef void            DSL_void_t;
/** integer type, width is depending on processor arch */
typedef int             DSL_int_t;
/** unsigned integer type, width is depending on processor arch */
typedef unsigned int    DSL_uint_t;
typedef struct file DSL_DRV_file_t;
typedef struct inode DSL_DRV_inode_t;

/**
 *    Defines all possible CMV groups
 *    */
typedef enum {
   DSL_CMV_GROUP_CNTL = 1,
   DSL_CMV_GROUP_STAT = 2,
   DSL_CMV_GROUP_INFO = 3,
   DSL_CMV_GROUP_TEST = 4,
   DSL_CMV_GROUP_OPTN = 5,
   DSL_CMV_GROUP_RATE = 6,
   DSL_CMV_GROUP_PLAM = 7,
   DSL_CMV_GROUP_CNFG = 8
} DSL_CmvGroup_t;
/**
 *    Defines all opcode types
 *    */
typedef enum {
   H2D_CMV_READ = 0x00,
   H2D_CMV_WRITE = 0x04,
   H2D_CMV_INDICATE_REPLY = 0x10,
   H2D_ERROR_OPCODE_UNKNOWN =0x20,
   H2D_ERROR_CMV_UNKNOWN =0x30,

   D2H_CMV_READ_REPLY =0x01,
   D2H_CMV_WRITE_REPLY = 0x05,
   D2H_CMV_INDICATE = 0x11,
   D2H_ERROR_OPCODE_UNKNOWN = 0x21,
   D2H_ERROR_CMV_UNKNOWN = 0x31,
   D2H_ERROR_CMV_READ_NOT_AVAILABLE = 0x41,
   D2H_ERROR_CMV_WRITE_ONLY = 0x51,
   D2H_ERROR_CMV_READ_ONLY = 0x61,

   H2D_DEBUG_READ_DM = 0x02,
   H2D_DEBUG_READ_PM = 0x06,
   H2D_DEBUG_WRITE_DM = 0x0a,
   H2D_DEBUG_WRITE_PM = 0x0e,

   D2H_DEBUG_READ_DM_REPLY = 0x03,
   D2H_DEBUG_READ_FM_REPLY = 0x07,
   D2H_DEBUG_WRITE_DM_REPLY = 0x0b,
   D2H_DEBUG_WRITE_FM_REPLY = 0x0f,
   D2H_ERROR_ADDR_UNKNOWN = 0x33,

   D2H_AUTONOMOUS_MODEM_READY_MSG = 0xf1
} DSL_CmvOpcode_t;

/* mutex macros */
#define MEI_MUTEX_INIT(id,flag) \
        sema_init(&id,flag)
#define MEI_MUTEX_LOCK(id) \
        down_interruptible(&id)
#define MEI_MUTEX_UNLOCK(id) \
        up(&id)
#define MEI_WAIT(ms) \
        {\
                set_current_state(TASK_INTERRUPTIBLE);\
                schedule_timeout(ms);\
        }
#define MEI_INIT_WAKELIST(name,queue) \
        init_waitqueue_head(&queue)

static inline long
ugly_hack_sleep_on_timeout(wait_queue_head_t *q, long timeout)
{
	unsigned long flags;
	wait_queue_entry_t wait;

	init_waitqueue_entry(&wait, current);

	__set_current_state(TASK_INTERRUPTIBLE);
	spin_lock_irqsave(&q->lock, flags);
	__add_wait_queue(q, &wait);
	spin_unlock(&q->lock);

	timeout = schedule_timeout(timeout);

	spin_lock_irq(&q->lock);
	__remove_wait_queue(q, &wait);
	spin_unlock_irqrestore(&q->lock, flags);

	return timeout;
}

/* wait for an event, timeout is measured in ms */
#define MEI_WAIT_EVENT_TIMEOUT(ev,timeout)\
        ugly_hack_sleep_on_timeout(&ev, timeout * HZ / 1000)
#define MEI_WAKEUP_EVENT(ev)\
        wake_up_interruptible(&ev)
#endif /* IFX_MEI_BSP */

/***	Register address offsets, relative to MEI_SPACE_ADDRESS ***/
#define ME_DX_DATA				(0x0000)
#define	ME_VERSION				(0x0004)
#define	ME_ARC_GP_STAT				(0x0008)
#define ME_DX_STAT				(0x000C)
#define	ME_DX_AD				(0x0010)
#define ME_DX_MWS				(0x0014)
#define	ME_ME2ARC_INT				(0x0018)
#define	ME_ARC2ME_STAT				(0x001C)
#define	ME_ARC2ME_MASK 				(0x0020)
#define	ME_DBG_WR_AD				(0x0024)
#define ME_DBG_RD_AD				(0x0028)
#define	ME_DBG_DATA				(0x002C)
#define	ME_DBG_DECODE				(0x0030)
#define ME_CONFIG				(0x0034)
#define	ME_RST_CTRL				(0x0038)
#define	ME_DBG_MASTER				(0x003C)
#define	ME_CLK_CTRL				(0x0040)
#define	ME_BIST_CTRL				(0x0044)
#define	ME_BIST_STAT				(0x0048)
#define ME_XDATA_BASE_SH			(0x004c)
#define ME_XDATA_BASE				(0x0050)
#define ME_XMEM_BAR_BASE			(0x0054)
#define ME_XMEM_BAR0				(0x0054)
#define ME_XMEM_BAR1				(0x0058)
#define ME_XMEM_BAR2				(0x005C)
#define ME_XMEM_BAR3				(0x0060)
#define ME_XMEM_BAR4				(0x0064)
#define ME_XMEM_BAR5				(0x0068)
#define ME_XMEM_BAR6				(0x006C)
#define ME_XMEM_BAR7				(0x0070)
#define ME_XMEM_BAR8				(0x0074)
#define ME_XMEM_BAR9				(0x0078)
#define ME_XMEM_BAR10				(0x007C)
#define ME_XMEM_BAR11				(0x0080)
#define ME_XMEM_BAR12				(0x0084)
#define ME_XMEM_BAR13				(0x0088)
#define ME_XMEM_BAR14				(0x008C)
#define ME_XMEM_BAR15				(0x0090)
#define ME_XMEM_BAR16				(0x0094)

#define WHILE_DELAY 		20000
/*
**	Define where in ME Processor's memory map the Stratify chip lives
*/

#define MAXSWAPSIZE      	(8 * 1024)	//8k *(32bits)

//      Mailboxes
#define MSG_LENGTH		16	// x16 bits
#define YES_REPLY      	 	1
#define NO_REPLY         	0

#define CMV_TIMEOUT		1000	//jiffies

//  Block size per BAR
#define SDRAM_SEGMENT_SIZE	(64*1024)
// Number of Bar registers
#define MAX_BAR_REGISTERS	(17)

#define XDATA_REGISTER		(15)

// ARC register addresss
#define ARC_STATUS		0x0
#define ARC_LP_START		0x2
#define ARC_LP_END		0x3
#define ARC_DEBUG		0x5
#define ARC_INT_MASK		0x10A

#define IRAM0_BASE 		(0x00000)
#define IRAM1_BASE 		(0x04000)
#if defined(CONFIG_DANUBE)
#define BRAM_BASE  		(0x0A000)
#elif defined(CONFIG_AMAZON_SE) || defined(CONFIG_AR9) || defined(CONFIG_VR9)
#define BRAM_BASE               (0x08000)
#endif
#define XRAM_BASE		(0x18000)
#define YRAM_BASE		(0x1A000)
#define EXT_MEM_BASE		(0x80000)
#define ARC_GPIO_CTRL		(0xC030)
#define ARC_GPIO_DATA		(0xC034)

#define IRAM0_SIZE		(16*1024)
#define IRAM1_SIZE		(16*1024)
#define BRAM_SIZE		(12*1024)
#define XRAM_SIZE		(8*1024)
#define YRAM_SIZE		(8*1024)
#define EXT_MEM_SIZE		(1536*1024)

#define ADSL_BASE		(0x20000)
#define CRI_BASE		(ADSL_BASE + 0x11F00)
#define CRI_CCR0		(CRI_BASE + 0x00)
#define CRI_RST			(CRI_BASE + 0x04*4)
#define ADSL_DILV_BASE 		(ADSL_BASE+0x20000)

//
#define IRAM0_ADDR_BIT_MASK	0xFFF
#define IRAM1_ADDR_BIT_MASK	0xFFF
#define BRAM_ADDR_BIT_MASK	0xFFF
#define RX_DILV_ADDR_BIT_MASK	0x1FFF

/***  Bit definitions ***/
#define ARC_AUX_HALT		(1 << 25)
#define ARC_DEBUG_HALT		(1 << 1)
#define FALSE			0
#define TRUE			1
#define BIT0			(1<<0)
#define BIT1			(1<<1)
#define BIT2			(1<<2)
#define BIT3			(1<<3)
#define BIT4			(1<<4)
#define BIT5			(1<<5)
#define BIT6			(1<<6)
#define BIT7			(1<<7)
#define BIT8			(1<<8)
#define BIT9			(1<<9)
#define BIT10 			(1<<10)
#define BIT11			(1<<11)
#define BIT12			(1<<12)
#define BIT13			(1<<13)
#define BIT14			(1<<14)
#define BIT15			(1<<15)
#define BIT16 			(1<<16)
#define BIT17			(1<<17)
#define BIT18			(1<<18)
#define BIT19			(1<<19)
#define BIT20			(1<<20)
#define BIT21			(1<<21)
#define BIT22			(1<<22)
#define BIT23			(1<<23)
#define BIT24			(1<<24)
#define BIT25			(1<<25)
#define BIT26			(1<<26)
#define BIT27			(1<<27)
#define BIT28			(1<<28)
#define BIT29			(1<<29)
#define BIT30			(1<<30)
#define BIT31			(1<<31)

// CRI_CCR0 Register definitions
#define CLK_2M_MODE_ENABLE	BIT6
#define	ACL_CLK_MODE_ENABLE	BIT4
#define FDF_CLK_MODE_ENABLE	BIT2
#define STM_CLK_MODE_ENABLE	BIT0

// CRI_RST Register definitions
#define FDF_SRST		BIT3
#define MTE_SRST		BIT2
#define FCI_SRST		BIT1
#define AAI_SRST		BIT0

//      MEI_TO_ARC_INTERRUPT Register definitions
#define	MEI_TO_ARC_INT1		BIT3
#define	MEI_TO_ARC_INT0		BIT2
#define MEI_TO_ARC_CS_DONE	BIT1	//need to check
#define	MEI_TO_ARC_MSGAV	BIT0

//      ARC_TO_MEI_INTERRUPT Register definitions
#define	ARC_TO_MEI_INT1		BIT8
#define	ARC_TO_MEI_INT0		BIT7
#define	ARC_TO_MEI_CS_REQ	BIT6
#define	ARC_TO_MEI_DBG_DONE	BIT5
#define	ARC_TO_MEI_MSGACK	BIT4
#define	ARC_TO_MEI_NO_ACCESS	BIT3
#define	ARC_TO_MEI_CHECK_AAITX	BIT2
#define	ARC_TO_MEI_CHECK_AAIRX	BIT1
#define	ARC_TO_MEI_MSGAV	BIT0

//      ARC_TO_MEI_INTERRUPT_MASK Register definitions
#define	GP_INT1_EN		BIT8
#define	GP_INT0_EN		BIT7
#define	CS_REQ_EN		BIT6
#define	DBG_DONE_EN		BIT5
#define	MSGACK_EN		BIT4
#define	NO_ACC_EN		BIT3
#define	AAITX_EN		BIT2
#define	AAIRX_EN		BIT1
#define	MSGAV_EN		BIT0

#define	MEI_SOFT_RESET		BIT0

#define	HOST_MSTR		BIT0

#define JTAG_MASTER_MODE	0x0
#define MEI_MASTER_MODE		HOST_MSTR

//      MEI_DEBUG_DECODE Register definitions
#define MEI_DEBUG_DEC_MASK	(0x3)
#define MEI_DEBUG_DEC_AUX_MASK	(0x0)
#define ME_DBG_DECODE_DMP1_MASK	(0x1)
#define MEI_DEBUG_DEC_DMP2_MASK	(0x2)
#define MEI_DEBUG_DEC_CORE_MASK	(0x3)

#define AUX_STATUS		(0x0)
#define AUX_ARC_GPIO_CTRL	(0x10C)
#define AUX_ARC_GPIO_DATA	(0x10D)
//      ARC_TO_MEI_MAILBOX[11] is a special location used to indicate
//      page swap requests.
#if defined(CONFIG_DANUBE)
#define OMBOX_BASE      	0xDF80
#define	ARC_TO_MEI_MAILBOX	0xDFA0
#define IMBOX_BASE      	0xDFC0
#define MEI_TO_ARC_MAILBOX	0xDFD0
#elif defined(CONFIG_AMAZON_SE) || defined(CONFIG_AR9) || defined(CONFIG_VR9)
#define OMBOX_BASE              0xAF80
#define ARC_TO_MEI_MAILBOX      0xAFA0
#define IMBOX_BASE              0xAFC0
#define MEI_TO_ARC_MAILBOX      0xAFD0
#endif

#define MEI_TO_ARC_MAILBOXR	(MEI_TO_ARC_MAILBOX + 0x2C)
#define ARC_MEI_MAILBOXR	(ARC_TO_MEI_MAILBOX + 0x2C)
#define OMBOX1  		(OMBOX_BASE+0x4)

// Codeswap request messages are indicated by setting BIT31
#define OMB_CODESWAP_MESSAGE_MSG_TYPE_MASK	(0x80000000)

// Clear Eoc messages received are indicated by setting BIT17
#define OMB_CLEAREOC_INTERRUPT_CODE		(0x00020000)
#define OMB_REBOOT_INTERRUPT_CODE               (1 << 18)

/*
**	Swap page header
*/
//      Page must be loaded at boot time if size field has BIT31 set
#define BOOT_FLAG		(BIT31)
#define BOOT_FLAG_MASK		~BOOT_FLAG

#define FREE_RELOAD		1
#define FREE_SHOWTIME		2
#define FREE_ALL		3

// marcos
#define	IFX_MEI_WRITE_REGISTER_L(data,addr)	*((volatile u32*)(addr)) = (u32)(data)
#define IFX_MEI_READ_REGISTER_L(addr) 	(*((volatile u32*)(addr)))
#define SET_BIT(reg, mask)			reg |= (mask)
#define CLEAR_BIT(reg, mask)			reg &= (~mask)
#define CLEAR_BITS(reg, mask)			CLEAR_BIT(reg, mask)
//#define SET_BITS(reg, mask)			SET_BIT(reg, mask)
#define SET_BITFIELD(reg, mask, off, val)	{reg &= (~mask); reg |= (val << off);}

#define ALIGN_SIZE				( 1L<<10 )	//1K size align
#define MEM_ALIGN(addr)				(((addr) + ALIGN_SIZE - 1) & ~ (ALIGN_SIZE -1) )

// swap marco
#define MEI_HALF_WORD_SWAP(data)		{data = ((data & 0xffff)<<16) + ((data & 0xffff0000)>>16);}
#define MEI_BYTE_SWAP(data)			{data = ((data & 0xff)<<24) + ((data & 0xff00)<<8)+ ((data & 0xff0000)>>8)+ ((data & 0xff000000)>>24);}


#ifdef CONFIG_PROC_FS
typedef struct reg_entry
{
   int *flag;
   char name[30];               /* big enough to hold names */
   char description[100];       /* big enough to hold description */
   unsigned short low_ino;
} reg_entry_t;
#endif
//      Swap page header describes size in 32-bit words, load location, and image offset
//      for program and/or data segments
typedef struct _arc_swp_page_hdr {
	u32 p_offset;		//Offset bytes of progseg from beginning of image
	u32 p_dest;		//Destination addr of progseg on processor
	u32 p_size;		//Size in 32-bitwords of program segment
	u32 d_offset;		//Offset bytes of dataseg from beginning of image
	u32 d_dest;		//Destination addr of dataseg on processor
	u32 d_size;		//Size in 32-bitwords of data segment
} ARC_SWP_PAGE_HDR;

/*
**	Swap image header
*/
#define GET_PROG	0	//      Flag used for program mem segment
#define GET_DATA	1	//      Flag used for data mem segment

//      Image header contains size of image, checksum for image, and count of
//      page headers. Following that are 'count' page headers followed by
//      the code and/or data segments to be loaded
typedef struct _arc_img_hdr {
	u32 size;		//      Size of binary image in bytes
	u32 checksum;		//      Checksum for image
	u32 count;		//      Count of swp pages in image
	ARC_SWP_PAGE_HDR page[1];	//      Should be "count" pages - '1' to make compiler happy
} ARC_IMG_HDR;

typedef struct smmu_mem_info {
	int type;
	int boot;
	unsigned long nCopy;
	unsigned long size;
	unsigned char *address;
	unsigned char *org_address;
} smmu_mem_info_t;

#ifdef __KERNEL__
typedef struct ifx_mei_device_private {
	int modem_ready;
	int arcmsgav;
	int cmv_reply;
	int cmv_waiting;
	// Mei to ARC CMV count, reply count, ARC Indicator count
	int modem_ready_cnt;
	int cmv_count;
	int reply_count;
	unsigned long image_size;
	int nBar;
	u16 Recent_indicator[MSG_LENGTH];

	u16 CMV_RxMsg[MSG_LENGTH] __attribute__ ((aligned (4)));

	smmu_mem_info_t adsl_mem_info[MAX_BAR_REGISTERS];
	ARC_IMG_HDR *img_hdr;
	//  to wait for arc cmv reply, sleep on wait_queue_arcmsgav;
	wait_queue_head_t wait_queue_arcmsgav;
	wait_queue_head_t wait_queue_modemready;
	struct semaphore mei_cmv_sema;
} ifx_mei_device_private_t;
#endif
typedef struct winhost_message {
	union {
		u16 RxMessage[MSG_LENGTH] __attribute__ ((aligned (4)));
		u16 TxMessage[MSG_LENGTH] __attribute__ ((aligned (4)));
	} msg;
} DSL_DEV_WinHost_Message_t;
/********************************************************************************************************
 * DSL CPE API Driver Stack Interface Definitions
 * *****************************************************************************************************/
/** IOCTL codes for bsp driver */
#define DSL_IOC_MEI_BSP_MAGIC		's'

#define DSL_FIO_BSP_DSL_START		_IO  (DSL_IOC_MEI_BSP_MAGIC, 0)
#define DSL_FIO_BSP_RUN			_IO  (DSL_IOC_MEI_BSP_MAGIC, 1)
#define DSL_FIO_BSP_FREE_RESOURCE	_IO  (DSL_IOC_MEI_BSP_MAGIC, 2)
#define DSL_FIO_BSP_RESET		_IO  (DSL_IOC_MEI_BSP_MAGIC, 3)
#define DSL_FIO_BSP_REBOOT		_IO  (DSL_IOC_MEI_BSP_MAGIC, 4)
#define DSL_FIO_BSP_HALT		_IO  (DSL_IOC_MEI_BSP_MAGIC, 5)
#define DSL_FIO_BSP_BOOTDOWNLOAD	_IO  (DSL_IOC_MEI_BSP_MAGIC, 6)
#define DSL_FIO_BSP_JTAG_ENABLE		_IO  (DSL_IOC_MEI_BSP_MAGIC, 7)
#define DSL_FIO_FREE_RESOURCE		_IO  (DSL_IOC_MEI_BSP_MAGIC, 8)
#define DSL_FIO_ARC_MUX_TEST		_IO  (DSL_IOC_MEI_BSP_MAGIC, 9)
#define DSL_FIO_BSP_REMOTE		_IOW (DSL_IOC_MEI_BSP_MAGIC, 10, u32)
#define DSL_FIO_BSP_GET_BASE_ADDRESS	_IOR (DSL_IOC_MEI_BSP_MAGIC, 11, u32)
#define DSL_FIO_BSP_IS_MODEM_READY	_IOR (DSL_IOC_MEI_BSP_MAGIC, 12, u32)
#define DSL_FIO_BSP_GET_VERSION		_IOR (DSL_IOC_MEI_BSP_MAGIC, 13, DSL_DEV_Version_t)
#define DSL_FIO_BSP_CMV_WINHOST		_IOWR(DSL_IOC_MEI_BSP_MAGIC, 14, DSL_DEV_WinHost_Message_t)
#define DSL_FIO_BSP_CMV_READ		_IOWR(DSL_IOC_MEI_BSP_MAGIC, 15, DSL_DEV_MeiReg_t)
#define DSL_FIO_BSP_CMV_WRITE		_IOW (DSL_IOC_MEI_BSP_MAGIC, 16, DSL_DEV_MeiReg_t)
#define DSL_FIO_BSP_DEBUG_READ		_IOWR(DSL_IOC_MEI_BSP_MAGIC, 17, DSL_DEV_MeiDebug_t)
#define DSL_FIO_BSP_DEBUG_WRITE		_IOWR(DSL_IOC_MEI_BSP_MAGIC, 18, DSL_DEV_MeiDebug_t)
#define DSL_FIO_BSP_GET_CHIP_INFO	_IOR (DSL_IOC_MEI_BSP_MAGIC, 19, DSL_DEV_HwVersion_t)

#define DSL_DEV_MEIDEBUG_BUFFER_SIZES	512

typedef struct DSL_DEV_MeiDebug
{
	DSL_uint32_t iAddress;
	DSL_uint32_t iCount;
	DSL_uint32_t buffer[DSL_DEV_MEIDEBUG_BUFFER_SIZES];
} DSL_DEV_MeiDebug_t;			/* meidebug */

/**
 *    Structure is used for debug access only.
 *       Refer to configure option INCLUDE_ADSL_WINHOST_DEBUG */
typedef struct struct_meireg
{
	/*
	*       Specifies that address for debug access */
	unsigned long iAddress;
	/*
	*       Specifies the pointer to the data that has to be written or returns a
	*             pointer to the data that has been read out*/
	unsigned long iData;
} DSL_DEV_MeiReg_t;					/* meireg */

typedef struct DSL_DEV_Device
{
	DSL_int_t nInUse;                /* modem state, update by bsp driver, */
	DSL_void_t *pPriv;
	DSL_uint32_t base_address;       /* mei base address */
	DSL_int_t nIrq[2];                  /* irq number */
#define IFX_DFEIR		0
#define IFX_DYING_GASP	1
	DSL_DEV_MeiDebug_t lop_debugwr;  /* dying gasp */
	struct module *owner;
} DSL_DEV_Device_t;			/* ifx_adsl_device_t */

#define DSL_DEV_PRIVATE(dev)  ((ifx_mei_device_private_t*)(dev->pPriv))

typedef struct DSL_DEV_Version		/* ifx_adsl_bsp_version */
{
	unsigned long major;
	unsigned long minor;
	unsigned long revision;
} DSL_DEV_Version_t;			/* ifx_adsl_bsp_version_t */

typedef struct DSL_DEV_ChipInfo
{
	unsigned long major;
	unsigned long minor;
} DSL_DEV_HwVersion_t;

typedef struct
{
	DSL_uint8_t dummy;
} DSL_DEV_DeviceConfig_t;

/** error code definitions */
typedef enum DSL_DEV_MeiError
{
	DSL_DEV_MEI_ERR_SUCCESS = 0,
	DSL_DEV_MEI_ERR_FAILURE = -1,
	DSL_DEV_MEI_ERR_MAILBOX_FULL = -2,
	DSL_DEV_MEI_ERR_MAILBOX_EMPTY = -3,
	DSL_DEV_MEI_ERR_MAILBOX_TIMEOUT = -4
} DSL_DEV_MeiError_t;			/* MEI_ERROR */

typedef enum {
	DSL_BSP_MEMORY_READ=0,
	DSL_BSP_MEMORY_WRITE,
} DSL_BSP_MemoryAccessType_t;		/* ifx_adsl_memory_access_type_t */

typedef enum
{
	DSL_LED_LINK_ID=0,
	DSL_LED_DATA_ID
} DSL_DEV_LedId_t;			/* ifx_adsl_led_id_t */

typedef enum
{
	DSL_LED_LINK_TYPE=0,
	DSL_LED_DATA_TYPE
} DSL_DEV_LedType_t;			/* ifx_adsl_led_type_t */

typedef enum
{
	DSL_LED_HD_CPU=0,
	DSL_LED_HD_FW
} DSL_DEV_LedHandler_t;			/* ifx_adsl_led_handler_t */

typedef enum {
	DSL_LED_ON=0,
	DSL_LED_OFF,
	DSL_LED_FLASH,
} DSL_DEV_LedMode_t;			/* ifx_adsl_led_mode_t */

typedef enum {
	DSL_CPU_HALT=0,
	DSL_CPU_RUN,
	DSL_CPU_RESET,
} DSL_DEV_CpuMode_t;			/* ifx_adsl_cpu_mode_t */

#if 0
typedef enum {
	DSL_BSP_EVENT_DYING_GASP = 0,
	DSL_BSP_EVENT_CEOC_IRQ,
} DSL_BSP_Event_id_t;			/* ifx_adsl_event_id_t */

typedef union DSL_BSP_CB_Param
{
	DSL_uint32_t nIrqMessage;
} DSL_BSP_CB_Param_t;			/* ifx_adsl_cbparam_t */

typedef struct DSL_BSP_CB_Event
{
	DSL_BSP_Event_id_t nID;
	DSL_DEV_Device_t *pDev;
	DSL_BSP_CB_Param_t *pParam;
} DSL_BSP_CB_Event_t;			/* ifx_adsl_cb_event_t */
#endif

/* external functions (from the BSP Driver) */
extern DSL_DEV_Device_t* DSL_BSP_DriverHandleGet(int, int);
extern DSL_int_t DSL_BSP_DriverHandleDelete(DSL_DEV_Device_t *);
extern DSL_DEV_MeiError_t DSL_BSP_FWDownload(DSL_DEV_Device_t *, const DSL_char_t *, DSL_uint32_t, DSL_int32_t *, DSL_int32_t *);
extern int DSL_BSP_KernelIoctls(DSL_DEV_Device_t *, unsigned int, unsigned long);
extern DSL_DEV_MeiError_t DSL_BSP_SendCMV(DSL_DEV_Device_t *, DSL_uint16_t *, DSL_int_t, DSL_uint16_t *);
extern DSL_DEV_MeiError_t DSL_BSP_AdslLedInit(DSL_DEV_Device_t *, DSL_DEV_LedId_t, DSL_DEV_LedType_t, DSL_DEV_LedHandler_t);
extern DSL_DEV_MeiError_t DSL_BSP_Showtime(DSL_DEV_Device_t *, DSL_uint32_t, DSL_uint32_t);
extern int DSL_BSP_ATMLedCBRegister( int (*ifx_adsl_ledcallback)(void));
extern DSL_DEV_MeiError_t DSL_BSP_MemoryDebugAccess(DSL_DEV_Device_t *, DSL_BSP_MemoryAccessType_t, DSL_uint32_t, DSL_uint32_t *, DSL_uint32_t);
extern volatile DSL_DEV_Device_t *adsl_dev;

/**
 *    Dummy structure by now to show mechanism of extended data that will be
 *       provided within event callback itself.
 *       */
typedef struct
{
	/**
	*    Dummy value */
	DSL_uint32_t nDummy1;
} DSL_BSP_CB_Event1DataDummy_t;

/**
 *    Dummy structure by now to show mechanism of extended data that will be
 *       provided within event callback itself.
 *       */
typedef struct
{
	/**
	*    Dummy value */
	DSL_uint32_t nDummy2;
} DSL_BSP_CB_Event2DataDummy_t;

/**
 *    encapsulate all data structures that are necessary for status event
 *       callbacks.
 *       */
typedef union
{
	DSL_BSP_CB_Event1DataDummy_t dataEvent1;
	DSL_BSP_CB_Event2DataDummy_t dataEvent2;
} DSL_BSP_CB_DATA_Union_t;


typedef enum
{
	/**
	 *    Informs the upper layer driver (DSL CPE API) about a reboot request from the
	 *       firmware.
	 *          \note This event does NOT include any additional data.
	 *                   More detailed information upon reboot reason has to be requested from
	 *                            upper layer software via CMV (INFO 109) if necessary. */
	DSL_BSP_CB_FIRST = 0,
        DSL_BSP_CB_DYING_GASP,
	DSL_BSP_CB_CEOC_IRQ,
	DSL_BSP_CB_FIRMWARE_REBOOT,
	/**
	 *    Delimiter only */
	DSL_BSP_CB_LAST
} DSL_BSP_CB_Type_t;

/**
 *    Specifies the common event type that has to be used for registering and
 *       signalling of interrupts/autonomous status events from MEI BSP Driver.
 *
 *    \param pDev
 *    Context pointer from MEI BSP Driver.
 *
 *    \param IFX_ADSL_BSP_CallbackType_t
 *    Specifies the event callback type (reason of callback). Regrading to the
 *    setting of this value the data which is included in the following union
 *    might have different meanings.
 *    Please refer to the description of the union to get information about the
 *    meaning of the included data.
 *
 *    \param pData
 *    Data according to \ref DSL_BSP_CB_DATA_Union_t.
 *    If this pointer is NULL there is no additional data available.
 *
 *    \return depending on event
 */
typedef int (*DSL_BSP_EventCallback_t)
(
	DSL_DEV_Device_t *pDev,
	DSL_BSP_CB_Type_t nCallbackType,
	DSL_BSP_CB_DATA_Union_t *pData
);

typedef struct {
        DSL_BSP_EventCallback_t function;
        DSL_BSP_CB_Type_t       event;
        DSL_BSP_CB_DATA_Union_t *pData;
} DSL_BSP_EventCallBack_t;

extern int DSL_BSP_EventCBRegister(DSL_BSP_EventCallBack_t *);
extern int DSL_BSP_EventCBUnregister(DSL_BSP_EventCallBack_t *);

/** Modem states */
#define DSL_DEV_STAT_InitState              0x0000
#define DSL_DEV_STAT_ReadyState             0x0001
#define DSL_DEV_STAT_FailState              0x0002
#define DSL_DEV_STAT_IdleState              0x0003
#define DSL_DEV_STAT_QuietState             0x0004
#define DSL_DEV_STAT_GhsState               0x0005
#define DSL_DEV_STAT_FullInitState          0x0006
#define DSL_DEV_STAT_ShowTimeState          0x0007
#define DSL_DEV_STAT_FastRetrainState       0x0008
#define DSL_DEV_STAT_LoopDiagMode           0x0009
#define DSL_DEV_STAT_ShortInit              0x000A     /* Bis short initialization */

#define DSL_DEV_STAT_CODESWAP_COMPLETE	    0x0002

#endif //IFXMIPS_MEI_H
