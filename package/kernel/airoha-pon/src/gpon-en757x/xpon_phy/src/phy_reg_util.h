#ifndef _DRV_REG_UTIL_H_
#define _DRV_REG_UTIL_H_

#include <asm/io.h>
#include "i2c.h"
#include <asm/tc3162/tc3162.h>

#ifdef TCSUPPORT_CPU_EN7521
#ifdef TCSUPPORT_AUTOBENCH
#define I2C_U2_CLK_DIV	(0xc7)
#else
#define I2C_U2_CLK_DIV	(0x60)
#endif
#else
#define I2C_U2_CLK_DIV	(0xc7)
#endif
#ifdef CONFIG_USE_MT7520_ASIC
// #define IO_GPHYREG(reg)						regReadPhy32(reg)//ioread32((void __iomem *)(reg)) /* GetReg((uint)reg) */
// #define IO_SPHYREG(reg, val)				iowrite32(val, (void __iomem *)(reg)) /* SetReg((uint)reg, val) */

extern u32 get_xpon_data(u32 reg);
extern void set_xpon_data(u32 reg, u32 val);
#define IO_GPHYREG(reg)						get_xpon_data(reg)
#define IO_SPHYREG(reg, val)			set_xpon_data(reg, val)

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

#define INREG32(ptr)						(ptr)
#define REG_FLD(bits, shift)				(0xFFFFFFFF>>(32-bits)), shift
#define REG_FLD_GET(FLD, reg)				GetRegMask((uint)(reg), FLD)
#define REG_FLD_SET(FLD, reg, val)			SetRegMask((uint)(reg), FLD, val)

#define IO_GREG(reg)						ioread32((void __iomem *)(reg)) /* GetReg((uint)reg) */
#define IO_SREG(reg, val)					iowrite32(val, (void __iomem *)(reg)) /* SetReg((uint)reg, val) */
#define IO_SBITS(reg, bits)					SetBits((uint)(reg), bits)
#define IO_CBITS(reg, bits)					ClearBits((uint)(reg), bits)
#define IO_GMASK(reg, mask, shift)			GetRegMask((uint)(reg), mask, shift)
#define IO_SMASK(reg, mask, shift, val)		SetRegMask((uint)(reg), mask, shift, val)

/************************************************************************
 Description:	get register value
 Input:			arg1: device register
 Output:		return the value of register
************************************************************************/
static __inline__ uint GetReg(uint reg)
{
	return ioread32((void __iomem *)reg) ;
}

/************************************************************************
 Description:	set register
 Input:			arg1: device register
 				arg2: setting value
  Output:		no return
************************************************************************/
static __inline__ void SetReg(uint reg, uint value)
{
	iowrite32(value, (void __iomem *)reg) ;
}


/************************************************************************
 Description:	get mask value of register
 Input:			arg1: device register
 				arg2: mask bits to get
 				arg3: offset of the mask
 Output:		return the mask value of register
************************************************************************/
static __inline__ uint GetRegMask(uint reg, uint mask, uint shift)
{
	return (((ioread32((void __iomem *)reg))>>shift) & mask) ;
}

/************************************************************************
 Description:	set mask value of register
 Input:			arg1: device register
 				arg2: mask bits to get
 				arg3: offset of the mask
 				arg4: setting value
 Output:		no return
************************************************************************/
static __inline__ void SetRegMask(uint reg, uint mask, uint shift, uint value)
{
    uint data = ioread32((void __iomem *)reg) ;

    data = (data & ~(mask<<shift)) | ((value&mask)<<shift) ;
    iowrite32(data, (void __iomem *)reg) ;
}

/************************************************************************
 Description:	set multiple bits to 1
 Input:			arg1: device register
 				arg2: multiple bits
 Output:		no return
************************************************************************/
static __inline__ void SetBits(uint reg, uint bits)
{
    uint data = ioread32((void __iomem *)reg) ;

    data |= bits ;
	iowrite32(data, (void __iomem *)reg) ;
}

/************************************************************************
 Description:	set multiple bits to 0
 Input:			arg1: device register
 				arg2: multiple bits
 Output:		no return
************************************************************************/
static __inline__ void ClearBits(uint reg, uint bits)
{
    uint data =ioread32((void __iomem *)reg) ;

    data &= ~bits ;
	iowrite32(data, (void __iomem *)reg) ;
}


#endif /* _DRV_REG_UTIL_H_ */

