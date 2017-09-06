#ifndef __MKTITANIMG_H
#define __MKTITANIMG_H

#ifndef CFGMGR_CKSUM_H
#define CFGMGR_CKSUM_H

#define CKSUM_MAGIC_NUMBER 0xC453DE23

#include <inttypes.h>
#include <stdio.h>
#include <errno.h>

int cs_is_tagged(FILE*);
unsigned long cs_read_sum(FILE*);
int cs_calc_sum(FILE*, unsigned long*, int);
int cs_set_sum(FILE*, unsigned long, int);
void cs_get_sum(FILE*, unsigned long*);
unsigned long cs_calc_buf_sum(char*, int);
int cs_validate_file(char*);

#endif
#ifndef ___CMDLINE_H___
#define	___CMDLINE_H___

/* ********* Library Configuration ********* */
typedef	struct CMDLINE_OPT
{
	int	min;					/* Minimum number of arguments this option takes */
	int	max;					/* Maximum number of arguments this option takes */
	int	flags;					/* Controlling flags (whether to accept or not, etc) */
} CMDLINE_OPT;

typedef	struct CMDLINE_CFG
{
	CMDLINE_OPT	opts[26];		/* Options 'a' through 'z' */
	CMDLINE_OPT	global;			/* Global option (outside 'a'..'z') */
} CMDLINE_CFG;
/* ******************************************** */

#define	CMDLINE_OPTFLAG_ALLOW	0x1			/* The option is allowed */
#define	CMDLINE_OPTFLAG_MANDAT	0x2			/* The option is mandatory */

extern	void	cmdline_print(char* argv[]);

extern	int		cmdline_configure(CMDLINE_CFG* p_cfg);
extern	int		cmdline_read(int argc, char* argv[]);

extern	void*	cmdline_getarg_list(char opt);
extern	int		cmdline_getarg_count(void* list);
extern	int		cmdline_getopt_count(char opt);
extern	int		cmdline_getarg(void* list, int num);

extern	char*	cmdline_error(int err);
#endif


#ifndef _NSPIMGHDR_H_
#define _NSPIMGHDR_H_

/* This file describes the header format for the single image. The image is broken
   up into several pieces. The image contains this header plus 1 or more sections.
   Each section contains a binary block that could be a kernel, filesystem, etc. The
   only garentee for this is that the very first section MUST be executable. Meaning
   that the bootloader will be able to take the address of the header start, add the
   header size, and execute that binary block. The header has its own checksum. It
   starts hdr_size-4 bytes from the start of the header.
 */

struct nsp_img_hdr_head
{
	unsigned int	magic;		/* Magic number to identify this image header */
	unsigned int	boot_offset;	/* Offset from start of header to kernel code. */
	unsigned int	flags;		/* Image flags. */
	unsigned int	hdr_version;	/* Version of this header. */
	unsigned int	hdr_size;	/* The complete size of all portions of the header */
	unsigned int	prod_id;	/* This product id */
	unsigned int	rel_id;		/* Which release this is */
	unsigned int	version;	/* name-MMM.nnn.ooo-rxx => 0xMMnnooxx. See comment
					   below */
	unsigned int	image_size;	/* Image size (including header) */
	unsigned int	info_offset;	/* Offset from start of header to info block */
	unsigned int	sect_info_offset;	/* Offset from start of header to section desc */
	unsigned int	chksum_offset;	/* Offset from start of header to chksum block */
//	unsigned int    pad1;
};

/* The patch id is a machine readable value that takes the normal patch level, and encodes
   the correct numbers inside of it. The format of the patches are name-MM.NN.oo-rxx.bin.
   Convert MM, NN, oo, and xx into hex, and encode them as 0xMMNNooxx. Thus:
   att-1.2.18-r14.bin => 0x0102120e */

/* The following are the flag bits for the above flags variable */
/* List of NSP status flags: */
#define NSP_IMG_FLAG_FAILBACK_MASK	0xF8000000

/* NSP Image status flag: Flag indicates individual sections image */
#define NSP_IMG_FLAG_INDIVIDUAL		0x00000001

/* NSP Image status flag 1: Image contains a bootable image when this bit is 0 */
#define NSP_IMG_FLAG_FAILBACK_1		0x08000000

/* NSP Image status flag 2: Image contains a non-bootable image when this bit is 0 */
#define NSP_IMG_FLAG_FAILBACK_2		0x10000000

/* NSP Image status flag 3: PSPBoot has tried the image when this bit is 0 */
#define NSP_IMG_FLAG_FAILBACK_3		0x20000000

/* NSP Image status flag 4: Image is now secondary image when this bit is 0 */
#define NSP_IMG_FLAG_FAILBACK_4		0x40000000

/* NSP Image status flag 5: Image contains a valid image when this bit is 0 */
#define NSP_IMG_FLAG_FAILBACK_5		0x80000000

/* NSP Single image magic number */
#define NSP_IMG_MAGIC_NUMBER		0x4D544443


struct nsp_img_hdr_info
{
	char	release_name[64];	/* Name of release */
	char	image_filename[64];	/* name-mm.nn.oo-rxx.bin format */
};

struct nsp_img_hdr_section_info
{
	unsigned int	num_sects;	/* Number of section (and section desc blocks) in this
					   image */
	unsigned int	sect_size;	/* Size of a SINGLE section_desc block */
	unsigned int	sections_offset;	/* Offset to from start of header to the start of
						   the section blocks */
};

/* There will be one of more of the following stuctures in the image header. Each
   section will have one of these blocks. */
struct nsp_img_hdr_sections
{
	unsigned int	offset;		/* Offset of section from start of NSP_IMG_HDR_HEAD */
	unsigned int	total_size;	/* Size of section (including pad size.) */
	unsigned int	raw_size;	/* Size of section only */
	unsigned int	flags;		/* Section flags */
	unsigned int	chksum;		/* Section checksum */
	unsigned int	type;		/* Section type. What kind of info does this section
					   describe */
	char		name[16];	/* Reference name for this section. */
};
#define NSP_IMG_SECTION_TYPE_KERNEL		(0x01)
#define NSP_IMG_SECTION_TYPE_FILESYSTEM_ROOT	(0x02)
#define NSP_IMG_SECTION_TYPE_FILESYSTEM		(0x03)

struct nsp_img_hdr
{
	struct nsp_img_hdr_head		head;	/* Head portion */
	struct nsp_img_hdr_info		info;	/* Info */
	struct nsp_img_hdr_section_info	sect_info;	/* Section block */
	struct nsp_img_hdr_sections	sections;	/* 1 or more section_description blocks. More
						   section_desc blocks will be appended here
						   for each additional section needed */
};

struct nsp_img_hdr_chksum
{
	unsigned int	hdr_chksum;	/* The checksum for the complete header. Excepting the
					   checksum block */
};

struct nsp_img_hdr_sections *nsp_img_hdr_get_section_ptr_by_name(struct nsp_img_hdr *hdr, char *name);
unsigned int nsp_img_hdr_get_section_offset_by_name(struct nsp_img_hdr *hdr, char *name);
unsigned int nsp_img_hdr_get_section_size_by_name(struct nsp_img_hdr *hdr, char *name);

#endif
#endif /* __MKTITANIMG_H */
