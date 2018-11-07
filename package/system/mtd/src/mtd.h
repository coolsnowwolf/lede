#ifndef __mtd_h
#define __mtd_h

#include <stdbool.h>

#if defined(target_brcm47xx) || defined(target_bcm53xx)
#define target_brcm 1
#endif

#define JFFS2_EOF "\xde\xad\xc0\xde"

extern int quiet;
extern int mtdsize;
extern int erasesize;

extern int mtd_open(const char *mtd, bool block);
extern int mtd_check_open(const char *mtd);
extern int mtd_block_is_bad(int fd, int offset);
extern int mtd_erase_block(int fd, int offset);
extern int mtd_write_buffer(int fd, const char *buf, int offset, int length);
extern int mtd_write_jffs2(const char *mtd, const char *filename, const char *dir);
extern int mtd_replace_jffs2(const char *mtd, int fd, int ofs, const char *filename);
extern void mtd_parse_jffs2data(const char *buf, const char *dir);

/* target specific functions */
extern int trx_fixup(int fd, const char *name)  __attribute__ ((weak));
extern int trx_check(int imagefd, const char *mtd, char *buf, int *len) __attribute__ ((weak));
extern int mtd_fixtrx(const char *mtd, size_t offset, size_t data_size) __attribute__ ((weak));
extern int mtd_fixseama(const char *mtd, size_t offset, size_t data_size) __attribute__ ((weak));
extern int mtd_fixwrg(const char *mtd, size_t offset, size_t data_size) __attribute__ ((weak));
extern int mtd_fixwrgg(const char *mtd, size_t offset, size_t data_size) __attribute__ ((weak));
extern int mtd_resetbc(const char *mtd) __attribute__ ((weak));
#endif /* __mtd_h */
