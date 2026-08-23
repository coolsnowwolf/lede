#ifndef __MTD_RT_FLASH_H__
#define __MTD_RT_FLASH_H__

#include <asm/tc3162/tc3162.h>

#ifdef TCSUPPORT_CPU_ARMV8
#include <modules/spi/spi_controller.h>
#endif

#ifdef TCSUPPORT_NEW_SPIFLASH
extern unsigned char ReadSPIByte(unsigned long index);
extern unsigned long ReadSPIDWord(unsigned long index);
#else
#ifdef TCSUPPORT_MT7510_E1
#define ReadSPIByte(i) (((*((unsigned char*)i))==0) ? (*((unsigned char*)i)): (*((unsigned char*)i)))
#define ReadSPIDWord(i) (((*((unsigned int*)i))==0) ? (*((unsigned int*)i)): (*((unsigned int*)i)))
#else
#define ReadSPIByte(i) (*((unsigned char*)i))
#define ReadSPIDWord(i) (*((unsigned int*)i))
#endif
#endif

#define READ_FLASH_BYTE(i)  	( (IS_NANDFLASH) ? \
								((ranand_read_byte != NULL) ? ranand_read_byte((i)) : -1) \
								: (ReadSPIByte(i)) )


/* for read flash, frankliao added 20101216 */
#define READ_FLASH_DWORD(i)  ( (IS_NANDFLASH) ? \
								((ranand_read_dword != NULL) ? ranand_read_dword((i)) : -1) \
								: (ReadSPIDWord(i)) )
								
/* frankliao added 20101215 */
extern unsigned long flash_base;
extern unsigned int (*ranand_read_byte)(unsigned long long);
extern unsigned int (*ranand_read_dword)(unsigned long long);

#define TCLINUX_INFO_MASTER_REAL_SIZE		(0)
#define TCLINUX_INFO_MASTER_KERNEL_OFFSET	(1)
#define TCLINUX_INFO_MASTER_KERNEL_SIZE		(2)
#define TCLINUX_INFO_MASTER_ROOTFS_OFFSET	(3)
#define TCLINUX_INFO_MASTER_ROOTFS_SIZE		(4)
#define TCLINUX_INFO_SLAVE_REAL_SIZE		(5)
#define TCLINUX_INFO_SLAVE_KERNEL_OFFSET	(6)
#define TCLINUX_INFO_SLAVE_KERNEL_SIZE		(7)
#define TCLINUX_INFO_SLAVE_ROOTFS_OFFSET	(8)
#define TCLINUX_INFO_SLAVE_ROOTFS_SIZE		(9)
#define TCLINUX_INFO_UNKNOW_VAL				(0xFFFFFFFFFFFFFFFF)

#endif /* __MTD_MTD_RT_FLASH_H__ */
