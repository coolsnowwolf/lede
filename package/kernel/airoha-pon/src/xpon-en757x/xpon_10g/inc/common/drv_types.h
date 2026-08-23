/***************************************************************
Copyright Statement:

This software/firmware and related documentation (¡°EcoNet Software¡±) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (¡°EcoNet¡±) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (¡°ECONET SOFTWARE¡±) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ¡°AS IS¡± 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER¡¯S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER¡¯S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/
#ifndef _DRV_TYPES_H_
#define _DRV_TYPES_H_

#include <linux/types.h>

#ifdef __KERNEL__
#include <asm/io.h>
#include <asm/tc3162/tc3162.h>


#ifdef TCSUPPORT_CPU_ARMV8_64
typedef u64 regAddr_t;
#else
typedef u32 regAddr_t;
#endif


#ifdef TCSUPPORT_CPU_ARMV8_64
extern u32 get_xpon_data(u32 reg);
extern void set_xpon_data(u32 reg, u32 val);
u32 get_frame_engine_data(u32 reg);
void set_frame_engine_data(u32 reg, u32 val);
#endif

#define IO_GPHYREG(reg)						ioReadPhyReg32((uint)(reg))
#define IO_SPHYREG(reg, val)				ioWritePhyReg32((uint)(reg), val)

#ifdef TCSUPPORT_CPU_ARMV8_64
#define INREG32(ptr)						(regAddr_t)(ptr)
#else
#define INREG32(ptr)						(ptr)
#endif

#define REG_FLD(bits, shift)				(0xFFFFFFFF>>(32-bits)), shift
#define REG_FLD_GET(FLD, reg)				GetRegMask((uint)(reg), FLD)
#define REG_FLD_SET(FLD, reg, val)			SetRegMask((uint)(reg), FLD, val)

#ifdef TCSUPPORT_CPU_ARMV8_64
#define IO_GREG(reg)                  get_xpon_data(reg)  
#define IO_SREG(reg, val)             set_xpon_data(reg, val)
#define READ_FE_REG(reg)              get_frame_engine_data(reg)
#define WRITE_FE_REG(reg, val)        set_frame_engine_data(reg,val)
#else
#define IO_GREG(reg)						ioread32((void __iomem *)(reg)) /* GetReg((uint)reg) */
#define IO_SREG(reg, val)					iowrite32(val, (void __iomem *)(reg)) /* SetReg((uint)reg, val) */
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
#ifdef TCSUPPORT_CPU_ARMV8_64
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
#ifdef TCSUPPORT_CPU_ARMV8_64
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
#ifdef TCSUPPORT_CPU_ARMV8_64
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
#ifdef TCSUPPORT_CPU_ARMV8_64
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
#ifdef TCSUPPORT_CPU_ARMV8_64
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
#ifdef TCSUPPORT_CPU_ARMV8_64
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

#ifndef TRUE
#define    TRUE    1
#endif
#ifndef FALSE
#define    FALSE   0
#endif

#ifndef NACK
#define NACK 	0
#endif
#ifndef ACK
#define ACK 1
#endif

#define XGPON_SUCCESS 0
#define XGPON_FAIL -1

#define XPON_SUCCESS 0
#define XPON_FAIL -1

#endif /* _DRV_TYPES_H_ */

