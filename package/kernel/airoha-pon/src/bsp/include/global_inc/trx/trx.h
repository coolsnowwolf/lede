/*
 * trx.h
 *
 *  Created on: Mar 19, 2009
 *      Author: pork
 */

#ifndef TRX_H_
#define TRX_H_

#if defined(TCSUPPORT_SECURE_BOOT)
#include <boot/sHeader.h>
#endif

struct f_header_t {
	unsigned int bl22_length;
	unsigned int bl23_length;
	unsigned int flash_table_length;
	unsigned int lzma_src;
	unsigned int lzma_des;
	unsigned int lzma_length;
	unsigned int lzma_cmd;
	unsigned int fw_ver;
	unsigned int reserved;
};

struct multi_bl2_h {
	unsigned int magic;
	unsigned int bl2_crc;
	unsigned int version;
	char reserved[16];
};


struct trx_header {
	unsigned int magic;			/* "HDR0" */
	unsigned int header_len;    /*Length of trx header*/
	unsigned int len;			/* Length of file including header */
	unsigned int crc32;			/* 32-bit CRC from flag_version to end of file */
	unsigned char version[32];  /*firmware version number*/
	unsigned char customerversion[32];  /*firmware version number*/
//	unsigned int flag_version;	/* 0:15 flags, 16:31 version */
#if 0
	unsigned int reserved[44];	/* Reserved field of header */
#else
	unsigned int kernel_len;	//kernel length
	unsigned int rootfs_len;	//rootfs length
	unsigned int romfile_len;	//romfile length
	#if 0
	unsigned int reserved[42];  /* Reserved field of header */
	#else
	unsigned char Model[32];
	unsigned int decompAddr;//kernel decompress address
	unsigned int saflag;
	unsigned int saflen;
	unsigned int linux_7z_pad;	/* for 4byte alignment */
	unsigned int reserved[29];  /* Reserved field of header */
	#endif
#if defined(TCSUPPORT_SECURE_BOOT_V2)
	SECURE_HEADER_V2 sHeader;
#elif defined(TCSUPPORT_SECURE_BOOT_V1) || defined(TCSUPPORT_SECURE_BOOT_FLASH_OTP)
	SECURE_HEADER_V1 sHeader;
#endif
#endif	
};
#define TRX_MAGIC	0x30524448	/* "HDR0" */
#define TRX_MAGIC1	0x31524448	/* "HDR1" */
#define TRX_MAGIC2	0x32524448	/* "for tclinux" */
#define TRX_MAGIC3		0x33524448	/* "for romfile" */
#define DEFAULT_CRC	0xFFFFFFFF
#endif /* TRX_H_ */
