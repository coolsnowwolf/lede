#ifndef _DRV_REG_UTIL_H_
#define _DRV_REG_UTIL_H_

#include <asm/io.h>
#include <asm/tc3162/tc3162.h>


#define	U1_DEV_ADDR  	(0x74) //Roger_a60972

#ifdef TCSUPPORT_AUTOBENCH
#define I2C_U2_CLK_DIV	(0xc7)
#else
#define I2C_U2_CLK_DIV	(0xc7)
#endif

#ifdef TCSUPPORT_CPU_ARMV8 //julia_arm
#define IO_GPHYREG(reg)						get_pon_phy_data(reg)//ioread32((void __iomem *)(reg)) /* GetReg((uint)reg) */
#define IO_SPHYREG(reg, val)				set_pon_phy_data(reg, val) /* SetReg((uint)reg, val) */
#else
#define IO_GPHYREG(reg)						regReadPhy32(reg)//ioread32((void __iomem *)(reg)) /* GetReg((uint)reg) */
#define IO_SPHYREG(reg, val)				iowrite32(val, (void __iomem *)(reg)) /* SetReg((uint)reg, val) */
#endif

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


#define GETBIT(data,i)					((data&(UINT32)1<<(i))?1:0)
#define SETBIT(data,i)					((data)|=(UINT32)1<<(i)))
#define CLRBIT(data,i)					((data)&=(~((UINT32)1<<(i))))

#define GETPOSITION(data,i)					((data&(UINT32)(i))?1:0)
#define SETPOSITION(data,i)					((data)|=(UINT32)(i)))
#define CLRPOSITION(data,i)					((data)&=(~((UINT32)(i))))


/************************************************************************
 Description:	get register value
 Input:			arg1: device register
 Output:		return the value of register
************************************************************************/
static uint __inline__ GetReg(uint reg)
{
	return ioread32((void __iomem *)reg) ;
}

/************************************************************************
 Description:	set register
 Input:			arg1: device register
 				arg2: setting value
  Output:		no return
************************************************************************/
static void __inline__ SetReg(uint reg, uint value)
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
static uint __inline__ GetRegMask(uint reg, uint mask, uint shift)
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
static void __inline__ SetRegMask(uint reg, uint mask, uint shift, uint value)
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
static void __inline__ SetBits(uint reg, uint bits)
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
static void __inline__ ClearBits(uint reg, uint bits)
{
    uint data =ioread32((void __iomem *)reg) ;
    
    data &= ~bits ;
	iowrite32(data, (void __iomem *)reg) ;
}

#endif /* _DRV_REG_UTIL_H_ */

