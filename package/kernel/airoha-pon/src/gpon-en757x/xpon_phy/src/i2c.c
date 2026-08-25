#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <asm/io.h>

#include "i2c.h"

static uint i2cMasterBaseAddr = 0 ;

static int i2c_write_read_data(unchar addr, uint wlen, uint rlen, unchar *buff) ;
static int i2c_write_data(unchar addr, uint wlen, unchar *buff) ;
/******************************************************************************
******************************************************************************/
int ioWritePhyReg8(ushort phyReg, unchar value)
{
	unchar buff[6] ;

	buff[0] = (unchar)(phyReg>>8) ;
	buff[1] = (unchar)(phyReg) ;
	buff[2] = (unchar)(value) ;

	return i2c_write_data(0x60, 3, buff) ;
}

/******************************************************************************
******************************************************************************/
uint ioReadPhyReg32(ushort phyReg)
{
	unchar buff[4] ;
	uint data = 0 ;
	int ret, i ;

	buff[0] = (unchar)(phyReg>>8) ;
	buff[1] = (unchar)(phyReg) ;

	ret = i2c_write_read_data(0x60, 2, 4, buff) ;
	if(!ret) {
		for(i=0 ; i<4 ; i++) {
			data |= (buff[i]<<(i*8)) ;
		}
	}

	return data ;
}

/******************************************************************************
******************************************************************************/
int ioWritePhyReg32(ushort phyReg, uint value)
{
	unchar buff[6] ;
	int i ;

	buff[0] = (unchar)(phyReg>>8) ;
	buff[1] = (unchar)(phyReg) ;
	for(i=0 ; i<4 ; i++) {
		buff[i+2] = (unchar)(value>>(i*8)) ;
	}

	return i2c_write_data(0x60, 6, buff) ;
}

/******************************************************************************
******************************************************************************/
// static int __print_csr(void)
// {
// 	uint	base = i2cMasterBaseAddr ;

// 	printk("CSR: 0004 Slave Address Register: 		%.4x\n", ioread32((void __iomem *)I2C_CSR_SLAVE_ADDR)) ;
// 	printk("CSR: 0010 Control Register: 			%.4x\n", ioread32((void __iomem *)I2C_CSR_CONTROL)) ;
// 	printk("CSR: 0014 Transfer Length Register: 	%.4x\n", ioread32((void __iomem *)I2C_CSR_TRANSFER_LEN)) ;
// 	printk("CSR: 0018 Translation Length Register: 	%.4x\n", ioread32((void __iomem *)I2C_CSR_TRANSAC_LEN)) ;
// 	printk("CSR: 001C Delay Register: 				%.4x\n", ioread32((void __iomem *)I2C_CSR_DELAY_LEN)) ;
// 	printk("CSR: 0020 Timeing Control Register: 	%.4x\n", ioread32((void __iomem *)I2C_CSR_TIMING)) ;
// 	printk("CSR: 0030 FIFO Status Register: 		%.4x\n", ioread32((void __iomem *)I2C_CSR_FIFO_STAT)) ;
// 	printk("CSR: 0040 IO Config Register:			%.4x\n", ioread32((void __iomem *)I2C_CSR_IO_CONFIG)) ;
// 	printk("CSR: 0048 High Speed Mode Register:		%.4x\n", ioread32((void __iomem *)I2C_CSR_HS)) ;
// 	printk("CSR: 0064 Debug Status Register:		%.4x\n", ioread32((void __iomem *)I2C_CSR_DEBUGSTAT)) ;
// 	printk("CSR: 0064 Debug Control Register:		%.4x\n", ioread32((void __iomem *)I2C_CSR_DEBUGCTRL)) ;

// 	return 0 ;
// }

/******************************************************************************
******************************************************************************/
static int i2c_write_read_data(unchar addr, uint wlen, uint rlen, unchar *buff)
{
	uint base = i2cMasterBaseAddr ;
	int	ret = 0, i ;
	int	timer = 0xffff ;
	ushort status ;


	/* bit 0 is to indicate read REQ or write REQ */
	addr = (addr<<1) ;

	/* control registers */
	I2C_SET_SLAVE_ADDR(addr) ;
	I2C_SET_TRANS_AUX_LEN(rlen) ;
	I2C_SET_TRANS_LEN(wlen) ;
	I2C_SET_TRANSAC_LEN(2) ;
	I2C_FIFO_CLR_ADDR ;
	I2C_SET_TRANS_CTRL(CONTROL_ACKERR_DET_EN | CONTROL_DIR_CHANGE | CONTROL_CLK_EXT | CONTROL_RS_FLAG) ;

	/* start to write data */
	for(i=0 ; i<wlen ; i++) {
		I2C_WRITE_BYTE(buff[i]) ;
	}

	I2C_START_TRANSAC ;

	/* see if transaction complete */
	while(1) {
		status = I2C_INTR_STATUS ;

	 	if(status & I2C_INT_COMPLETE) {
			break ;
	 	} else if(status & I2C_INT_HS_NACKERR) {
	 		ret = -EFAULT ;
	 		break ;
	 	} else if(status & I2C_INT_ACKERR) {
	 		ret = -EFAULT ;
			break ;
		} else if (timer == 0) {
	 		ret = -EFAULT ;
			break ;
		}
		timer-- ;
	}

	I2C_CLR_INTR_STATUS(I2C_INT_HS_NACKERR | I2C_INT_ACKERR | I2C_INT_COMPLETE);

	if(!ret) {
		for(i=0 ; i<4 ; i++) {
			buff[i] = (unchar)I2C_READ_BYTE() ;
		}
	}

	return ret ;
}

/******************************************************************************
******************************************************************************/
static int i2c_write_data(unchar addr, uint wlen, unchar *buff)
{
	uint base = i2cMasterBaseAddr ;
	int	ret, i ;
	int	timer = 0xffff ;
	ushort status ;


	/* bit 0 is to indicate read REQ or write REQ */
	addr = (addr<<1) ;

	/* control registers */
	I2C_SET_SLAVE_ADDR(addr) ;
	I2C_SET_TRANS_AUX_LEN(1) ;
	I2C_SET_TRANS_LEN(wlen) ;
	I2C_SET_TRANSAC_LEN(1) ;
	I2C_FIFO_CLR_ADDR ;
	I2C_SET_TRANS_CTRL((CONTROL_ACKERR_DET_EN | CONTROL_CLK_EXT) & ~CONTROL_RS_FLAG) ;

	for(i=0 ; i<wlen ; i++) {
		I2C_WRITE_BYTE(buff[i]) ;
	}

	I2C_START_TRANSAC ;

	while(1) {
		status = I2C_INTR_STATUS ;

	 	if(status & I2C_INT_COMPLETE) {
			break ;
	 	} else if(status & I2C_INT_HS_NACKERR) {
	 		ret = -EFAULT ;
	 		break ;
	 	} else if(status & I2C_INT_ACKERR) {
	 		ret = -EFAULT ;
			break ;
		} else if (timer == 0) {
	 		ret = -EFAULT ;
			break ;
		}
		timer-- ;
	}

	I2C_CLR_INTR_STATUS(I2C_INT_HS_NACKERR | I2C_INT_ACKERR | I2C_INT_COMPLETE);
	return ret ;
}


/******************************************************************************
******************************************************************************/
// static int i2c_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
// {
// 	char val_string[64], cmd[32] ;
// 	uint reg, value ;

// 	if (count > sizeof(val_string) - 1)
// 		return -EINVAL ;

// 	if (copy_from_user(val_string, buffer, count))
// 		return -EFAULT ;

// 	sscanf(val_string, "%s %x %x", cmd, &reg, &value) ;

// 	if(!strcmp(cmd, "read")) {
// 		printk("Read REG:%.4x: %.8x\n", reg, ioReadPhyReg32(reg)) ;
// 	} else if(!strcmp(cmd, "write")) {
// 		ioWritePhyReg32(reg, value) ;
// 	} else if(!strcmp(cmd, "show")) {
// 		__print_csr() ;
// 	}

// 	return count ;
// }

/******************************************************************************
******************************************************************************/
int i2c_init(void)
{
    int ret = 0 ;
	// struct proc_dir_entry *i2c_proc ;

	i2cMasterBaseAddr = (uint)(ioremap(CONFIG_I2C_BASE_ADDR, 0xFF)) ;
	if(!i2cMasterBaseAddr) {
		printk("ioremap the I2C base address failed.\n") ;
		return -EFAULT ;
	}

	// i2c_proc = create_proc_entry("i2c", 0, NULL) ;
	// if(i2c_proc) {
	// 	i2c_proc->write_proc = i2c_write_proc ;
	// }
    return ret;
}

/******************************************************************************
******************************************************************************/
void i2c_exit(void)
{
	remove_proc_entry("i2c", NULL) ;
	iounmap((uint *)i2cMasterBaseAddr) ;
}

