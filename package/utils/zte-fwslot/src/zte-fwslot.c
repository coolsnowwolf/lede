// SPDX-License-Identifier: GPL-2.0-only
/*
 * zte-fwslot - flash and select the SR1010 firmware slots from OpenWrt.
 *
 * The SR1010 keeps two firmware slots in flash. The vendor bootloader starts
 * the valid slot carrying the highest serial number. Images use the vendor
 * upgrade package format, so one file installs from either the vendor web UI
 * or OpenWrt, letting the two systems swap in place on this device.
 *
 * Flashing mirrors the vendor fw_flashing program: verify the package CRCs,
 * erase the inactive slot, write the package body to good erase blocks only,
 * then write the 512-byte version header to the next good block with a serial
 * larger than any other slot. "switch" checks the other slot against the CRCs of
 * its header and then only rewrites that header with a higher serial, so the
 * next boot selects it.
 *
 * This is a read/modify tool for the two firmware-slot MTD partitions of the
 * device it runs on. It never touches bootloader, tags or config partitions.
 */
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <mtd/mtd-user.h>

#define VER_HDR_SIZE	0x200
#define UBOOT_SIZE	0x180000
#define OUTER_HDR_SIZE	0x14

#define KERNEL_MAGIC_SIZE 32	/* in front of the kernel, behind the U-Boot */

#define H_VERSION	0x10
#define H_FW_SIZE	0x30
#define H_KERNEL_SIZE	0x34
#define H_KERNEL_FILEOFS 0x38
#define H_KERNEL_CRC	0x3c
#define H_FS_SIZE	0x40
#define H_FS_FILEOFS	0x44
#define H_FS_CRC	0x48
#define H_PRODUCT	0x6c
#define H_UBOOT_CRC	0x98
#define H_CRC		0xa4
#define H_BUILD		0xa8
#define H_INNER_MAGIC	0xf4
#define H_SLOT_START	0x1f0
#define H_SERIAL	0x1f4
#define H_NEW		0x1f8
#define H_CRC2		0x1fc

#define PRODUCT		"ZXSLC SR1010"

static const uint8_t outer_magic[16] = {
	0x99,0x99,0x99,0x99, 0x44,0x44,0x44,0x44,
	0x55,0x55,0x55,0x55, 0xaa,0xaa,0xaa,0xaa,
};
static const uint8_t inner_magic[16] = {
	0x33,0x33,0x33,0x33, 0x66,0x66,0x66,0x66,
	0x99,0x99,0x99,0x99, 0xcc,0xcc,0xcc,0xcc,
};

/* The two firmware slots, by absolute offset in the whole flash. */
static const uint32_t slot_start[2] = { 0x600000, 0x2f00000 };

static uint32_t crc_table[256];

static void crc_init(void)
{
	for (uint32_t i = 0; i < 256; i++) {
		uint32_t c = i;
		for (int k = 0; k < 8; k++)
			c = (c & 1) ? 0xedb88320u ^ (c >> 1) : c >> 1;
		crc_table[i] = c;
	}
}

/* Running CRC: start from 0xffffffff and invert the result at the end. */
static uint32_t crc32_update(uint32_t c, const void *buf, size_t len)
{
	const uint8_t *p = buf;

	while (len--)
		c = crc_table[(c ^ *p++) & 0xff] ^ (c >> 8);
	return c;
}

static uint32_t crc32(const void *buf, size_t len)
{
	return crc32_update(0xffffffffu, buf, len) ^ 0xffffffffu;
}

static uint32_t rd32(const uint8_t *p, size_t ofs)
{
	return p[ofs] | p[ofs + 1] << 8 | p[ofs + 2] << 16 |
	       (uint32_t)p[ofs + 3] << 24;
}

static void wr32(uint8_t *p, size_t ofs, uint32_t v)
{
	p[ofs] = v; p[ofs + 1] = v >> 8;
	p[ofs + 2] = v >> 16; p[ofs + 3] = v >> 24;
}

static void die(const char *msg)
{
	fprintf(stderr, "zte-fwslot: %s: %s\n", msg, strerror(errno));
	exit(1);
}

/* Read a sysfs attribute of an MTD device as an unsigned value. */
static long mtd_sysfs_u(int index, const char *attr)
{
	char path[64], buf[32];
	int fd, n;

	snprintf(path, sizeof(path), "/sys/class/mtd/mtd%d/%s", index, attr);
	fd = open(path, O_RDONLY);
	if (fd < 0)
		return -1;
	n = read(fd, buf, sizeof(buf) - 1);
	close(fd);
	if (n <= 0)
		return -1;
	buf[n] = 0;
	return strtol(buf, NULL, 0);
}

/* Find the MTD partition that starts at the given absolute flash offset. */
static int mtd_find_by_offset(uint32_t offset)
{
	DIR *d = opendir("/sys/class/mtd");
	struct dirent *e;
	int found = -1;

	if (!d)
		die("open /sys/class/mtd");
	while ((e = readdir(d))) {
		int idx;

		if (sscanf(e->d_name, "mtd%d", &idx) != 1)
			continue;
		if (strchr(e->d_name, 'r'))	/* skip mtdNro aliases */
			continue;
		if (mtd_sysfs_u(idx, "offset") == (long)offset) {
			found = idx;
			break;
		}
	}
	closedir(d);
	return found;
}

struct mtd {
	int fd;
	struct mtd_info_user info;
};

static void mtd_open(struct mtd *m, int index, bool writable)
{
	char path[32];

	snprintf(path, sizeof(path), "/dev/mtd%d", index);
	m->fd = open(path, writable ? O_RDWR : O_RDONLY);
	if (m->fd < 0)
		die(path);
	if (ioctl(m->fd, MEMGETINFO, &m->info) < 0)
		die("MEMGETINFO");
}

static bool mtd_bad(struct mtd *m, uint64_t ofs)
{
	long long o = ofs;

	return ioctl(m->fd, MEMGETBADBLOCK, &o) > 0;
}

/*
 * Both the vendor U-Boot and the kernel locate the payload by skipping the
 * blocks the bad block table lists, so a block that cannot be written must be
 * listed there, or everything behind it would be read from the wrong place.
 */
static void mtd_mark_bad(struct mtd *m, uint64_t ofs)
{
	long long o = ofs;

	if (ioctl(m->fd, MEMSETBADBLOCK, &o) < 0)
		perror("  could not mark the block bad");
}

static int mtd_erase(struct mtd *m, uint64_t ofs)
{
	struct erase_info_user e = { .start = ofs, .length = m->info.erasesize };

	ioctl(m->fd, MEMUNLOCK, &e);
	return ioctl(m->fd, MEMERASE, &e);
}

static void mtd_read(struct mtd *m, uint64_t ofs, void *buf, size_t len)
{
	if (lseek(m->fd, ofs, SEEK_SET) < 0 || read(m->fd, buf, len) != (ssize_t)len)
		die("mtd read");
}

/*
 * Scan a slot for its version header. The header is a single block carrying
 * the inner magic at +0xf4 and a CRC over [0:0xa4] at +0xa4. Returns the
 * block offset within the slot, or -1, and fills *serial when found.
 */
static long slot_scan(struct mtd *m, uint32_t *serial)
{
	uint8_t hdr[VER_HDR_SIZE];
	uint32_t bs = m->info.erasesize;

	for (uint64_t ofs = 0; ofs + VER_HDR_SIZE <= m->info.size; ofs += bs) {
		if (mtd_bad(m, ofs))
			continue;
		mtd_read(m, ofs, hdr, sizeof(hdr));
		if (memcmp(hdr + H_INNER_MAGIC, inner_magic, 16))
			continue;
		if (crc32(hdr, H_CRC) != rd32(hdr, H_CRC))
			continue;
		if (serial)
			*serial = rd32(hdr, H_SERIAL);
		return ofs;
	}
	return -1;
}

/* Highest serial across both slots, 0 if neither has a valid header. */
static uint32_t max_serial(void)
{
	uint32_t best = 0;

	for (int s = 0; s < 2; s++) {
		int idx = mtd_find_by_offset(slot_start[s]);
		struct mtd m;
		uint32_t serial;

		if (idx < 0)
			continue;
		mtd_open(&m, idx, false);
		if (slot_scan(&m, &serial) >= 0 && serial > best)
			best = serial;
		close(m.fd);
	}
	return best;
}

/* Offset of the slot the system booted from, -1 when the kernel does not say. */
static long running_slot_offset(void)
{
	FILE *f = fopen("/sys/firmware/zx279133/slot_offset", "r");
	long so = -1;

	if (f) {
		if (fscanf(f, "%li", &so) != 1)
			so = -1;
		fclose(f);
	}
	return so;
}

static int slot_index(long offset)
{
	return offset == (long)slot_start[0] ? 0 :
	       offset == (long)slot_start[1] ? 1 : -1;
}

/* A part of the payload and the CRC its version header promises. */
struct region {
	const char *fail;
	uint64_t start, len;
	uint32_t want, crc;
};

static void region_feed(struct region *r, uint64_t pos, const uint8_t *buf,
			size_t n)
{
	uint64_t lo = pos > r->start ? pos : r->start;
	uint64_t end = r->start + r->len;
	uint64_t hi = pos + n < end ? pos + n : end;

	if (lo < hi)
		r->crc = crc32_update(r->crc, buf + (lo - pos), hi - lo);
}

/*
 * Check a slot against the version header the bootloader goes by: the header
 * itself, then the U-Boot, the kernel and the root filesystem. The payload
 * starts at the beginning of the slot and sits in the good blocks, in order,
 * with the U-Boot first, then the kernel behind its magic, then the root
 * filesystem at the distance the header records between the two.
 *
 * Returns NULL for an intact slot, otherwise what is wrong with it.
 */
static const char *slot_verify(struct mtd *m, const uint8_t *hdr)
{
	uint32_t bs = m->info.erasesize;
	uint64_t body_len = rd32(hdr, H_FW_SIZE);
	uint32_t koff = rd32(hdr, H_KERNEL_FILEOFS);
	uint32_t foff = rd32(hdr, H_FS_FILEOFS);
	uint64_t kstart = UBOOT_SIZE + KERNEL_MAGIC_SIZE;
	uint64_t pos = 0, ofs = 0;
	const char *bad = NULL;
	struct region r[3];
	uint8_t *buf;

	if (crc32(hdr, H_CRC) != rd32(hdr, H_CRC) ||
	    crc32(hdr, H_CRC2) != rd32(hdr, H_CRC2))
		return "version header CRC mismatch";
	if (foff < koff)
		return "bad firmware layout";

	r[0] = (struct region){ "U-Boot CRC mismatch", 0, UBOOT_SIZE,
				rd32(hdr, H_UBOOT_CRC), 0xffffffffu };
	r[1] = (struct region){ "kernel CRC mismatch", kstart,
				rd32(hdr, H_KERNEL_SIZE),
				rd32(hdr, H_KERNEL_CRC), 0xffffffffu };
	r[2] = (struct region){ "root filesystem CRC mismatch",
				kstart + (foff - koff), rd32(hdr, H_FS_SIZE),
				rd32(hdr, H_FS_CRC), 0xffffffffu };
	if (body_len > m->info.size)
		return "bad firmware layout";
	for (int i = 0; i < 3; i++)
		if (r[i].start + r[i].len > body_len)
			return "bad firmware layout";

	buf = malloc(bs);
	if (!buf)
		return "out of memory";
	while (pos < body_len) {
		size_t n = body_len - pos < bs ? body_len - pos : bs;

		if (ofs + bs > m->info.size) {
			bad = "payload runs past the end of the slot";
			break;
		}
		if (mtd_bad(m, ofs)) {
			ofs += bs;
			continue;
		}
		if (pread(m->fd, buf, n, ofs) != (ssize_t)n) {
			bad = "read error";
			break;
		}
		for (int i = 0; i < 3; i++)
			region_feed(&r[i], pos, buf, n);
		pos += n;
		ofs += bs;
	}
	free(buf);
	if (bad)
		return bad;
	for (int i = 0; i < 3; i++)
		if ((r[i].crc ^ 0xffffffffu) != r[i].want)
			return r[i].fail;
	return NULL;
}

/* Map the package file and verify the CRCs the bootloader checks. */
struct package {
	uint8_t *data;
	size_t size;
	const uint8_t *hdr;	/* version header */
	const uint8_t *body;
	uint32_t body_len;
};

static void package_load(struct package *p, const char *path)
{
	FILE *f = fopen(path, "rb");
	long size;
	uint32_t hdr_ofs, body_ofs;

	if (!f)
		die(path);
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (size < OUTER_HDR_SIZE + VER_HDR_SIZE)
		errno = EINVAL, die("package too small");
	p->data = malloc(size);
	if (!p->data || fread(p->data, 1, size, f) != (size_t)size)
		die("read package");
	fclose(f);
	p->size = size;

	if (memcmp(p->data, outer_magic, 16))
		errno = EINVAL, die("not an SR1010 upgrade package");
	hdr_ofs = 0x14 + rd32(p->data, 0x10);
	body_ofs = hdr_ofs + VER_HDR_SIZE;
	if (body_ofs > p->size)
		errno = EINVAL, die("truncated header");
	p->hdr = p->data + hdr_ofs;
	p->body = p->data + body_ofs;
	p->body_len = p->size - body_ofs;

	if (memcmp(p->hdr + H_INNER_MAGIC, inner_magic, 16))
		errno = EINVAL, die("bad version header");
	if (crc32(p->hdr, H_CRC) != rd32(p->hdr, H_CRC) ||
	    crc32(p->hdr, H_CRC2) != rd32(p->hdr, H_CRC2))
		errno = EINVAL, die("header CRC mismatch");
	if (strncmp((const char *)p->hdr + H_PRODUCT, PRODUCT, sizeof(PRODUCT) - 1))
		errno = EINVAL, die("package is not for this product");
	if (rd32(p->hdr, H_FW_SIZE) != p->body_len)
		errno = EINVAL, die("body size mismatch");

	uint32_t koff = rd32(p->hdr, H_KERNEL_FILEOFS);
	uint32_t ksz = rd32(p->hdr, H_KERNEL_SIZE);
	uint32_t foff = rd32(p->hdr, H_FS_FILEOFS);
	uint32_t fsz = rd32(p->hdr, H_FS_SIZE);

	if ((uint64_t)koff + ksz > p->size || (uint64_t)foff + fsz > p->size)
		errno = EINVAL, die("region outside package");
	if (crc32(p->body, UBOOT_SIZE) != rd32(p->hdr, H_UBOOT_CRC))
		errno = EINVAL, die("U-Boot CRC mismatch");
	if (crc32(p->data + koff, ksz) != rd32(p->hdr, H_KERNEL_CRC))
		errno = EINVAL, die("kernel CRC mismatch");
	if (crc32(p->data + foff, fsz) != rd32(p->hdr, H_FS_CRC))
		errno = EINVAL, die("rootfs CRC mismatch");
}

/*
 * Write one full erase block at or after @ofs, skipping bad blocks, and return
 * the offset after it. The slot is erased beforehand and @buf must hold a full
 * erase block, padded with 0xff by the caller.
 */
static uint64_t write_block(struct mtd *m, uint64_t ofs, const uint8_t *buf)
{
	uint32_t bs = m->info.erasesize;

	while (ofs < m->info.size) {
		if (mtd_bad(m, ofs)) {
			fprintf(stderr, "  skip bad block at %#llx\n",
				(unsigned long long)ofs);
			ofs += bs;
			continue;
		}
		if (lseek(m->fd, ofs, SEEK_SET) < 0 ||
		    write(m->fd, buf, bs) != (ssize_t)bs) {
			fprintf(stderr, "  write failed at %#llx, marking the block bad\n",
				(unsigned long long)ofs);
			mtd_mark_bad(m, ofs);
			ofs += bs;
			continue;
		}
		return ofs + bs;
	}
	errno = ENOSPC;
	die("ran out of good blocks");
	return 0;
}

/*
 * Erase every good block of the slot, as the vendor flasher does. A header left
 * behind by an older firmware would otherwise still look valid to the
 * bootloader and take precedence over the new one.
 */
static void erase_slot(struct mtd *m)
{
	uint32_t bs = m->info.erasesize;

	for (uint64_t ofs = 0; ofs < m->info.size; ofs += bs) {
		if (mtd_bad(m, ofs))
			continue;
		if (mtd_erase(m, ofs) < 0)
			fprintf(stderr, "  erase failed at %#llx\n",
				(unsigned long long)ofs);
	}
}

static int cmd_check(const char *path)
{
	struct package p;

	package_load(&p, path);
	free(p.data);
	fprintf(stderr, "ok: valid SR1010 package for %s\n", PRODUCT);
	return 0;
}

static int cmd_flash(const char *path)
{
	struct package p;
	int target_slot, idx;
	struct mtd m;
	uint8_t hdr[VER_HDR_SIZE];
	uint32_t bs, serial;
	uint64_t ofs, left;
	const uint8_t *src;
	int running;

	package_load(&p, path);

	/* The running slot comes from the bootinfo driver; flash the other one. */
	running = slot_index(running_slot_offset());
	if (running < 0)
		errno = ENODEV, die("cannot tell which slot is running");
	target_slot = running ^ 1;

	idx = mtd_find_by_offset(slot_start[target_slot]);
	if (idx < 0)
		errno = ENODEV, die("target slot partition not found");
	mtd_open(&m, idx, true);
	bs = m.info.erasesize;
	if (p.body_len > m.info.size - bs)
		errno = ENOSPC, die("package larger than slot");

	serial = max_serial() + 1;
	memcpy(hdr, p.hdr, VER_HDR_SIZE);
	wr32(hdr, H_SLOT_START, slot_start[target_slot]);
	wr32(hdr, H_SERIAL, serial);
	wr32(hdr, H_NEW, 1);
	/* H_CRC covers [0:0xa4] and is unaffected; H_CRC2 covers the serial. */
	wr32(hdr, H_CRC2, crc32(hdr, H_CRC2));

	fprintf(stderr, "flashing slot %d (mtd%d), serial %u, body %#x\n",
		target_slot, idx, serial, p.body_len);

	erase_slot(&m);

	ofs = 0;
	left = p.body_len;
	src = p.body;
	while (left) {
		uint8_t pad[0x20000];
		const uint8_t *chunk = src;

		if (left < bs) {
			memset(pad, 0xff, bs);
			memcpy(pad, src, left);
			chunk = pad;
		}
		ofs = write_block(&m, ofs, chunk);
		if (left < bs)
			left = 0;
		else
			left -= bs;
		src += bs;
	}
	/* Version header goes in the next good block after the body. */
	{
		uint8_t hdrblk[0x20000];

		memset(hdrblk, 0xff, sizeof(hdrblk));
		memcpy(hdrblk, hdr, VER_HDR_SIZE);
		ofs = write_block(&m, ofs, hdrblk);
	}

	/* Verify the header reads back with a valid CRC. */
	if (slot_scan(&m, &serial) < 0)
		errno = EIO, die("header verify failed after flash");

	close(m.fd);
	free(p.data);
	fprintf(stderr, "ok: slot %d now holds serial %u; reboot to run it\n",
		target_slot, serial);
	return 0;
}

static int cmd_switch(void)
{
	int running = slot_index(running_slot_offset()), other, idx;
	struct mtd m;
	long hpos;
	uint32_t serial;
	uint8_t hdr[VER_HDR_SIZE];
	const char *why;

	if (running < 0)
		errno = ENODEV, die("cannot tell which slot is running");
	other = running ^ 1;

	idx = mtd_find_by_offset(slot_start[other]);
	if (idx < 0)
		errno = ENODEV, die("other slot partition not found");
	mtd_open(&m, idx, true);

	hpos = slot_scan(&m, &serial);
	if (hpos < 0) {
		fprintf(stderr, "zte-fwslot: slot %d holds no firmware\n", other);
		return 1;
	}
	mtd_read(&m, hpos, hdr, sizeof(hdr));

	/* The bootloader does not fall back: never point it at a broken slot. */
	why = slot_verify(&m, hdr);
	if (why) {
		fprintf(stderr, "zte-fwslot: slot %d is damaged (%s), not switching\n",
			other, why);
		return 1;
	}

	wr32(hdr, H_SERIAL, max_serial() + 1);
	wr32(hdr, H_CRC2, crc32(hdr, H_CRC2));

	/* NAND takes whole pages only: rewrite the full block, padded with 0xff. */
	{
		uint8_t blk[0x20000];

		memset(blk, 0xff, sizeof(blk));
		memcpy(blk, hdr, VER_HDR_SIZE);
		if (mtd_erase(&m, hpos) < 0)
			die("erase header block");
		if (lseek(m.fd, hpos, SEEK_SET) < 0 ||
		    write(m.fd, blk, m.info.erasesize) != (ssize_t)m.info.erasesize)
			die("write header");
	}

	close(m.fd);
	fprintf(stderr, "ok: next boot selects slot %d (serial %u)\n",
		other, rd32(hdr, H_SERIAL));
	return 0;
}

struct slot_info {
	int mtd;		/* -1: no such partition */
	bool valid;		/* a version header was found */
	long hpos;
	uint8_t hdr[VER_HDR_SIZE];
};

static void slot_info_read(int s, struct slot_info *si)
{
	struct mtd m;
	uint32_t serial;

	memset(si, 0, sizeof(*si));
	si->mtd = mtd_find_by_offset(slot_start[s]);
	if (si->mtd < 0)
		return;
	mtd_open(&m, si->mtd, false);
	si->hpos = slot_scan(&m, &serial);
	if (si->hpos >= 0) {
		si->valid = true;
		mtd_read(&m, si->hpos, si->hdr, sizeof(si->hdr));
	}
	close(m.fd);
}

/* The slot the bootloader starts next: valid, with the highest serial. */
static int next_slot(const struct slot_info *si)
{
	int best = -1;

	for (int s = 0; s < 2; s++)
		if (si[s].valid && (best < 0 ||
		    rd32(si[s].hdr, H_SERIAL) > rd32(si[best].hdr, H_SERIAL)))
			best = s;
	return best;
}

/* Print a header string field as a JSON string. */
static void json_str(const uint8_t *s, size_t max)
{
	putchar('"');
	for (size_t i = 0; i < max && s[i]; i++) {
		if (s[i] == '"' || s[i] == '\\')
			printf("\\%c", s[i]);
		else if (s[i] < 0x20 || s[i] >= 0x7f)
			printf("\\u%04x", s[i]);
		else
			putchar(s[i]);
	}
	putchar('"');
}

static int cmd_info(bool json)
{
	int running = slot_index(running_slot_offset());
	struct slot_info si[2];
	int next;

	for (int s = 0; s < 2; s++)
		slot_info_read(s, &si[s]);
	next = next_slot(si);

	if (json) {
		printf("{\"running\":%d,\"next\":%d,\"slots\":[", running, next);
		for (int s = 0; s < 2; s++) {
			printf("%s{\"slot\":%d,\"mtd\":%d,\"valid\":%s",
			       s ? "," : "", s, si[s].mtd, si[s].valid ? "true" : "false");
			if (si[s].valid) {
				printf(",\"serial\":%u,\"version\":", rd32(si[s].hdr, H_SERIAL));
				json_str(si[s].hdr + H_VERSION, 32);
				printf(",\"build\":");
				json_str(si[s].hdr + H_BUILD, 16);
				printf(",\"product\":");
				json_str(si[s].hdr + H_PRODUCT, 52);
			}
			printf("}");
		}
		printf("]}\n");
		return 0;
	}

	printf("running slot: %d\n", running);
	for (int s = 0; s < 2; s++) {
		printf("slot %d: start %#x mtd%d", s, slot_start[s], si[s].mtd);
		if (si[s].mtd < 0)
			printf(" (missing)\n");
		else if (!si[s].valid)
			printf(" (no valid firmware)\n");
		else
			printf(" header@%#lx serial=%u version=\"%.32s\"%s%s\n",
			       si[s].hpos, rd32(si[s].hdr, H_SERIAL),
			       (const char *)si[s].hdr + H_VERSION,
			       s == running ? " [running]" : "",
			       s == next ? " [next boot]" : "");
	}
	return 0;
}

int main(int argc, char **argv)
{
	crc_init();
	if (argc == 3 && !strcmp(argv[1], "flash"))
		return cmd_flash(argv[2]);
	if (argc == 3 && !strcmp(argv[1], "check"))
		return cmd_check(argv[2]);
	if (argc == 2 && !strcmp(argv[1], "switch"))
		return cmd_switch();
	if (argc == 2 && !strcmp(argv[1], "info"))
		return cmd_info(false);
	if (argc == 3 && !strcmp(argv[1], "info") && !strcmp(argv[2], "-j"))
		return cmd_info(true);

	fprintf(stderr,
		"usage: zte-fwslot check <package.bin>  verify a package without writing\n"
		"       zte-fwslot flash <package.bin>  write package to the inactive slot\n"
		"       zte-fwslot switch              check the other slot and make it the next boot\n"
		"       zte-fwslot info [-j]           show both slots, -j as JSON\n");
	return 1;
}
