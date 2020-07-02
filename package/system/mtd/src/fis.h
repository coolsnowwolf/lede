#ifndef __FIS_H
#define __FIS_H

struct fis_part {
	unsigned char name[16];
	uint32_t offset;
	uint32_t loadaddr;
	uint32_t size;
	uint32_t length;
	uint32_t crc;
};

int fis_validate(struct fis_part *old, int n_old, struct fis_part *new, int n_new);
int fis_remap(struct fis_part *old, int n_old, struct fis_part *new, int n_new);

#endif
