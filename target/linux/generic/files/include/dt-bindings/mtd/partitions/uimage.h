/* SPDX-License-Identifier: GPL-2.0-only */
/*
 *    *** IMPORTANT ***
 * This file is not only included from C-code but also from devicetree source
 * files. As such this file MUST only contain comments and defines.
 *
 * Based on image.h from U-Boot which is
 * (C) Copyright 2008 Semihalf
 * (C) Copyright 2000-2005 Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef __UIMAGE_H__
#define __UIMAGE_H__

/*
 * Operating System Codes
 *
 * The following are exposed to uImage header.
 * New IDs *MUST* be appended at the end of the list and *NEVER*
 * inserted for backward compatibility.
 */
#define	IH_OS_INVALID		0	/* Invalid OS	*/
#define	IH_OS_OPENBSD		1	/* OpenBSD	*/
#define	IH_OS_NETBSD		2	/* NetBSD	*/
#define	IH_OS_FREEBSD		3	/* FreeBSD	*/
#define	IH_OS_4_4BSD		4	/* 4.4BSD	*/
#define	IH_OS_LINUX		5	/* Linux	*/
#define	IH_OS_SVR4		6	/* SVR4		*/
#define	IH_OS_ESIX		7	/* Esix		*/
#define	IH_OS_SOLARIS		8	/* Solaris	*/
#define	IH_OS_IRIX		9	/* Irix		*/
#define	IH_OS_SCO	       10	/* SCO		*/
#define	IH_OS_DELL	       11	/* Dell		*/
#define	IH_OS_NCR	       12	/* NCR		*/
#define	IH_OS_LYNXOS	       13	/* LynxOS	*/
#define	IH_OS_VXWORKS	       14	/* VxWorks	*/
#define	IH_OS_PSOS	       15	/* pSOS		*/
#define	IH_OS_QNX	       16	/* QNX		*/
#define	IH_OS_U_BOOT	       17	/* Firmware	*/
#define	IH_OS_RTEMS	       18	/* RTEMS	*/
#define	IH_OS_ARTOS	       19	/* ARTOS	*/
#define	IH_OS_UNITY	       20	/* Unity OS	*/
#define	IH_OS_INTEGRITY	       21	/* INTEGRITY	*/
#define	IH_OS_OSE	       22	/* OSE		*/
#define	IH_OS_PLAN9	       23	/* Plan 9	*/
#define	IH_OS_OPENRTOS	       24	/* OpenRTOS	*/
#define	IH_OS_ARM_TRUSTED_FIRMWARE 25    /* ARM Trusted Firmware */
#define	IH_OS_TEE	       26	/* Trusted Execution Environment */
#define	IH_OS_OPENSBI	       27	/* RISC-V OpenSBI */
#define	IH_OS_EFI	       28	/* EFI Firmware (e.g. GRUB2) */

/*
 * CPU Architecture Codes (supported by Linux)
 *
 * The following are exposed to uImage header.
 * New IDs *MUST* be appended at the end of the list and *NEVER*
 * inserted for backward compatibility.
 */
#define	IH_ARCH_INVALID		0	/* Invalid CPU	*/
#define	IH_ARCH_ALPHA		1	/* Alpha	*/
#define	IH_ARCH_ARM		2	/* ARM		*/
#define	IH_ARCH_I386		3	/* Intel x86	*/
#define	IH_ARCH_IA64		4	/* IA64		*/
#define	IH_ARCH_MIPS		5	/* MIPS		*/
#define	IH_ARCH_MIPS64		6	/* MIPS	 64 Bit */
#define	IH_ARCH_PPC		7	/* PowerPC	*/
#define	IH_ARCH_S390		8	/* IBM S390	*/
#define	IH_ARCH_SH		9	/* SuperH	*/
#define	IH_ARCH_SPARC	       10	/* Sparc	*/
#define	IH_ARCH_SPARC64	       11	/* Sparc 64 Bit */
#define	IH_ARCH_M68K	       12	/* M68K		*/
#define	IH_ARCH_NIOS	       13	/* Nios-32	*/
#define	IH_ARCH_MICROBLAZE     14	/* MicroBlaze   */
#define	IH_ARCH_NIOS2	       15	/* Nios-II	*/
#define	IH_ARCH_BLACKFIN       16	/* Blackfin	*/
#define	IH_ARCH_AVR32	       17	/* AVR32	*/
#define	IH_ARCH_ST200	       18	/* STMicroelectronics ST200  */
#define	IH_ARCH_SANDBOX	       19	/* Sandbox architecture (test only) */
#define	IH_ARCH_NDS32	       20	/* ANDES Technology - NDS32  */
#define	IH_ARCH_OPENRISC       21	/* OpenRISC 1000  */
#define	IH_ARCH_ARM64	       22	/* ARM64	*/
#define	IH_ARCH_ARC	       23	/* Synopsys DesignWare ARC */
#define	IH_ARCH_X86_64	       24	/* AMD x86_64, Intel and Via */
#define	IH_ARCH_XTENSA	       25	/* Xtensa	*/
#define	IH_ARCH_RISCV	       26	/* RISC-V */

/*
 * Image Types
 *
 * "Standalone Programs" are directly runnable in the environment
 *	provided by U-Boot; it is expected that (if they behave
 *	well) you can continue to work in U-Boot after return from
 *	the Standalone Program.
 * "OS Kernel Images" are usually images of some Embedded OS which
 *	will take over control completely. Usually these programs
 *	will install their own set of exception handlers, device
 *	drivers, set up the MMU, etc. - this means, that you cannot
 *	expect to re-enter U-Boot except by resetting the CPU.
 * "RAMDisk Images" are more or less just data blocks, and their
 *	parameters (address, size) are passed to an OS kernel that is
 *	being started.
 * "Multi-File Images" contain several images, typically an OS
 *	(Linux) kernel image and one or more data images like
 *	RAMDisks. This construct is useful for instance when you want
 *	to boot over the network using BOOTP etc., where the boot
 *	server provides just a single image file, but you want to get
 *	for instance an OS kernel and a RAMDisk image.
 *
 *	"Multi-File Images" start with a list of image sizes, each
 *	image size (in bytes) specified by an "uint32_t" in network
 *	byte order. This list is terminated by an "(uint32_t)0".
 *	Immediately after the terminating 0 follow the images, one by
 *	one, all aligned on "uint32_t" boundaries (size rounded up to
 *	a multiple of 4 bytes - except for the last file).
 *
 * "Firmware Images" are binary images containing firmware (like
 *	U-Boot or FPGA images) which usually will be programmed to
 *	flash memory.
 *
 * "Script files" are command sequences that will be executed by
 *	U-Boot's command interpreter; this feature is especially
 *	useful when you configure U-Boot to use a real shell (hush)
 *	as command interpreter (=> Shell Scripts).
 *
 * The following are exposed to uImage header.
 * New IDs *MUST* be appended at the end of the list and *NEVER*
 * inserted for backward compatibility.
 */
#define	IH_TYPE_INVALID		0	/* Invalid Image		*/
#define	IH_TYPE_STANDALONE	1	/* Standalone Program		*/
#define	IH_TYPE_KERNEL		2	/* OS Kernel Image		*/
#define	IH_TYPE_RAMDISK		3	/* RAMDisk Image		*/
#define	IH_TYPE_MULTI		4	/* Multi-File Image		*/
#define	IH_TYPE_FIRMWARE	5	/* Firmware Image		*/
#define	IH_TYPE_SCRIPT		6	/* Script file			*/
#define	IH_TYPE_FILESYSTEM	7	/* Filesystem Image (any type)	*/
#define	IH_TYPE_FLATDT		8	/* Binary Flat Device Tree Blob	*/
#define	IH_TYPE_KWBIMAGE	9	/* Kirkwood Boot Image		*/
#define	IH_TYPE_IMXIMAGE       10	/* Freescale IMXBoot Image	*/
#define	IH_TYPE_UBLIMAGE       11	/* Davinci UBL Image		*/
#define	IH_TYPE_OMAPIMAGE      12	/* TI OMAP Config Header Image	*/
#define	IH_TYPE_AISIMAGE       13	/* TI Davinci AIS Image		*/
	/* OS Kernel Image, can run from any load address */
#define	IH_TYPE_KERNEL_NOLOAD  14
#define	IH_TYPE_PBLIMAGE       15	/* Freescale PBL Boot Image	*/
#define	IH_TYPE_MXSIMAGE       16	/* Freescale MXSBoot Image	*/
#define	IH_TYPE_GPIMAGE	       17	/* TI Keystone GPHeader Image	*/
#define	IH_TYPE_ATMELIMAGE     18	/* ATMEL ROM bootable Image	*/
#define	IH_TYPE_SOCFPGAIMAGE   19	/* Altera SOCFPGA CV/AV Preloader */
#define	IH_TYPE_X86_SETUP      20	/* x86 setup.bin Image		*/
#define	IH_TYPE_LPC32XXIMAGE   21	/* x86 setup.bin Image		*/
#define	IH_TYPE_LOADABLE       22	/* A list of typeless images	*/
#define	IH_TYPE_RKIMAGE	       23	/* Rockchip Boot Image		*/
#define	IH_TYPE_RKSD	       24	/* Rockchip SD card		*/
#define	IH_TYPE_RKSPI	       25	/* Rockchip SPI image		*/
#define	IH_TYPE_ZYNQIMAGE      26	/* Xilinx Zynq Boot Image */
#define	IH_TYPE_ZYNQMPIMAGE    27	/* Xilinx ZynqMP Boot Image */
#define	IH_TYPE_ZYNQMPBIF      28	/* Xilinx ZynqMP Boot Image (bif) */
#define	IH_TYPE_FPGA	       29	/* FPGA Image */
#define	IH_TYPE_VYBRIDIMAGE    30	/* VYBRID .vyb Image */
#define	IH_TYPE_TEE            31	/* Trusted Execution Environment OS Image */
#define	IH_TYPE_FIRMWARE_IVT   32	/* Firmware Image with HABv4 IVT */
#define	IH_TYPE_PMMC           33	/* TI Power Management Micro-Controller Firmware */
#define	IH_TYPE_STM32IMAGE     34	/* STMicroelectronics STM32 Image */
#define	IH_TYPE_SOCFPGAIMAGE_V1 35	/* Altera SOCFPGA A10 Preloader	*/
#define	IH_TYPE_MTKIMAGE       36	/* MediaTek BootROM loadable Image */
#define	IH_TYPE_IMX8MIMAGE     37	/* Freescale IMX8MBoot Image	*/
#define	IH_TYPE_IMX8IMAGE      38	/* Freescale IMX8Boot Image	*/
#define	IH_TYPE_COPRO	       39	/* Coprocessor Image for remoteproc*/


/*
 * Compression Types
 *
 * The following are exposed to uImage header.
 * New IDs *MUST* be appended at the end of the list and *NEVER*
 * inserted for backward compatibility.
 */
#define	IH_COMP_NONE		0	/*  No	 Compression Used	*/
#define	IH_COMP_GZIP		1	/* gzip	 Compression Used	*/
#define	IH_COMP_BZIP2		2	/* bzip2 Compression Used	*/
#define	IH_COMP_LZMA		3	/* lzma  Compression Used	*/
#define	IH_COMP_LZO		4	/* lzo   Compression Used	*/
#define	IH_COMP_LZ4		5	/* lz4   Compression Used	*/


#define LZ4F_MAGIC	0x184D2204	/* LZ4 Magic Number		*/
#define IH_MAGIC	0x27051956	/* Image Magic Number		*/
#define IH_NMLEN		32	/* Image Name Length		*/

/*
 * Magic values specific to "openwrt,uimage" partitions
 */
#define IH_MAGIC_OKLI	0x4f4b4c49	/* 'OKLI'			*/
#define FW_EDIMAX_OFFSET	20	/* Edimax Firmware Offset	*/
#define FW_MAGIC_EDIMAX	0x43535953	/* Edimax Firmware Magic Number */

#endif	/* __UIMAGE_H__ */
