#ifndef __wrgg_h
#define __wrgg_h

#define WRGG03_MAGIC	0x20080321

struct wrgg03_header {
	char		signature[32];
	uint32_t	magic1;
	uint32_t	magic2;
	char		version[16];
	char		model[16];
	uint32_t	flag[2];
	uint32_t	reserve[2];
	char		buildno[16];
	uint32_t	size;
	uint32_t	offset;
	char		dev_name[32];
	char		digest[16];
} __attribute__ ((packed));
#endif /* __wrgg_h */
