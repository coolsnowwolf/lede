#ifndef _DRV_TYPES_H_
#define _DRV_TYPES_H_

#include <linux/types.h>

#ifdef __KERNEL__
#include <asm/io.h>
#include <asm/tc3162/tc3162.h>
#include <xmcs/xmcs_const.h>

#ifdef TCSUPPORT_CPU_ARMV8
#ifdef TCSUPPORT_CPU_ARMV8_64
typedef u64 regAddr_t;
#else
typedef u32 regAddr_t;
#endif
#endif

#ifdef TCSUPPORT_CPU_ARMV8
extern u32 GET_HIR(void);
extern u32 get_xpon_data(u32 reg);
extern void set_xpon_data(u32 reg, u32 val);
u32 get_frame_engine_data(u32 reg);
void set_frame_engine_data(u32 reg, u32 val);
#endif

#define I2C_U2_CLK_DIV	(0xc7)
#ifdef CONFIG_USE_MT7520_ASIC
#define IO_GPHYREG(reg)						regReadPhy32(reg)//ioread32((void __iomem *)(reg)) /* GetReg((uint)reg) */
#define IO_SPHYREG(reg, val)				iowrite32(val, (void __iomem *)(reg)) /* SetReg((uint)reg, val) */
#elif defined(CONFIG_USE_A60901) || defined(CONFIG_USE_A60928)
	#ifdef CONFIG_USE_A60901
	#define	U1_DEV_ADDR  	(0x60)
	#elif CONFIG_USE_A60928
	#define	U1_DEV_ADDR	 	(0x50)
	#endif
	#define IO_GPHYREG(sub_addr)				phy_I2C_read_translet(sub_addr)
	#define IO_SPHYREG(sub_addr, val)			phy_I2C_write_translet(sub_addr, val)
#else
#define IO_GPHYREG(reg)						ioReadPhyReg32((uint)(reg))
#define IO_SPHYREG(reg, val)				ioWritePhyReg32((uint)(reg), val)
#endif /* CONFIG_USE_MT7520_ASIC */

#ifdef TCSUPPORT_CPU_ARMV8
#define INREG32(ptr)						(regAddr_t)(ptr)
#else
#define INREG32(ptr)						(ptr)
#endif
#define REG_FLD(bits, shift)				(0xFFFFFFFF>>(32-bits)), shift
#define REG_FLD_GET(FLD, reg)				GetRegMask((uint)(reg), FLD)
#define REG_FLD_SET(FLD, reg, val)			SetRegMask((uint)(reg), FLD, val)

#ifdef TCSUPPORT_CPU_ARMV8
#define IO_GREG(reg)                  get_xpon_data(reg)  
#define IO_SREG(reg, val)             set_xpon_data(reg, val)	
#define READ_FE_REG(reg)              get_frame_engine_data(reg)
#define WRITE_FE_REG(reg, val)        set_frame_engine_data(reg,val)
#else
#define IO_GREG(reg)                  ioread32((void __iomem *)(reg))
#define IO_SREG(reg, val)             iowrite32(val, (void __iomem *)(reg)) 
#define READ_FE_REG(reg)              ioread32((void __iomem *)(reg))
#define WRITE_FE_REG(reg, val)        iowrite32(val, (void __iomem *)(reg))
#endif

#define IO_SBITS(reg, bits)					SetBits((uint)(reg), bits)
#define IO_CBITS(reg, bits)					ClearBits((uint)(reg), bits)
#define IO_GMASK(reg, mask, shift)			GetRegMask((uint)(reg), mask, shift)
#define IO_SMASK(reg, mask, shift, val)		SetRegMask((uint)(reg), mask, shift, val)

/************************************************************************
 Description:	get register value
 Input:			arg1: device register
 Output:		return the value of register
************************************************************************/
static uint __inline__ GetReg(uint reg)
{
#ifdef TCSUPPORT_CPU_ARMV8
	return get_xpon_data(reg); 
#else	
	return ioread32((void __iomem *)reg) ;
#endif
}

/************************************************************************
 Description:	set register
 Input:			arg1: device register
 				arg2: setting value
  Output:		no return
************************************************************************/
static void __inline__ SetReg(uint reg, uint value)
{
#ifdef TCSUPPORT_CPU_ARMV8
	set_xpon_data(reg,value);
#else
	iowrite32(value, (void __iomem *)reg) ;
#endif
}


/************************************************************************
 Description:	get mask value of register
 Input:			arg1: device register
 				arg2: mask bits to get
 				arg3: offset of the mask
 Output:		return the mask value of register
************************************************************************/
static uint __inline__ GetRegMask(uint reg, uint mask, uint shift)
{
#ifdef TCSUPPORT_CPU_ARMV8
	return ((get_xpon_data(reg)>>shift) & mask) ;
#else
	return (((ioread32((void __iomem *)reg))>>shift) & mask) ;
#endif
}

/************************************************************************
 Description:	set mask value of register
 Input:			arg1: device register
 				arg2: mask bits to get
 				arg3: offset of the mask
 				arg4: setting value
 Output:		no return
************************************************************************/
static void __inline__ SetRegMask(uint reg, uint mask, uint shift, uint value)
{
#ifdef TCSUPPORT_CPU_ARMV8
	uint data = get_xpon_data(reg) ;
    
    data = (data & ~(mask<<shift)) | ((value&mask)<<shift) ;
    set_xpon_data(reg,data);  
#else
    uint data = ioread32((void __iomem *)reg) ;
    
    data = (data & ~(mask<<shift)) | ((value&mask)<<shift) ;
    iowrite32(data, (void __iomem *)reg) ;
#endif
}

/************************************************************************
 Description:	set multiple bits to 1
 Input:			arg1: device register
 				arg2: multiple bits
 Output:		no return
************************************************************************/
static void __inline__ SetBits(uint reg, uint bits)
{
#ifdef TCSUPPORT_CPU_ARMV8
	uint data = get_xpon_data(reg) ;
	
	data |= bits ;
	set_xpon_data(reg,data); 
#else
    uint data = ioread32((void __iomem *)reg) ;
    
    data |= bits ;
	iowrite32(data, (void __iomem *)reg) ;
#endif
}

/************************************************************************
 Description:	set multiple bits to 0
 Input:			arg1: device register
 				arg2: multiple bits
 Output:		no return
************************************************************************/
static void __inline__ ClearBits(uint reg, uint bits)
{
#ifdef TCSUPPORT_CPU_ARMV8
	uint data = get_xpon_data(reg) ;
	
	data &= ~bits ;
	set_xpon_data(reg,data);
#else
    uint data =ioread32((void __iomem *)reg) ;
    
    data &= ~bits ;
	iowrite32(data, (void __iomem *)reg) ;
#endif
}

#endif

#ifdef UINT32
#undef UINT32
#endif
#ifdef UINT16
#undef UINT16
#endif
#ifdef UINT8
#undef UINT8
#endif

typedef unsigned int		UINT32 ;
typedef unsigned short		UINT16 ;
typedef unsigned char   	UINT8 ;
#define PACKING
typedef unsigned int FIELD;

#ifndef VPint
#define VPint			*(volatile unsigned int *)
#endif /* VPint */

#ifndef TRUE
#define    TRUE    1
#endif
#ifndef FALSE
#define    FALSE   0
#endif

//#define ENABLE	1
//#define DISABLE	0

#ifndef NACK
#define NACK 	0
#endif
#ifndef ACK
#define ACK 1
#endif

#define XPON_SUCCESS 0
#define XPON_FAIL -1


#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)
typedef enum {
	PHY_XPON_STATE_NOTIFY_LOS = 0,	    /* G/EPON los happen */
	PHY_XPON_STATE_NOTIFY_SD, 	        /* G/EPON los dismiss */
	PHY_XPON_STATE_NOTIFY_DYINGGASP, 	/* G/EPON dying gasp happen */
} PHY_XponStateNotifyType_t ;
#endif/*TCSUPPORT_COMPILE*/

#endif /* _DRV_TYPES_H_ */

