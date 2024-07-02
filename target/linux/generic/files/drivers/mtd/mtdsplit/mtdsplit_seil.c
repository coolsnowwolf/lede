// SPDX-License-Identifier: GPL-2.0-or-later
/* a mtdsplit driver for IIJ SEIL devices */

#define pr_fmt(fmt)	KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/of.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/byteorder/generic.h>

#include "mtdsplit.h"

#define NR_PARTS		2
#define SEIL_VFMT		1
#define LDR_ENV_PART_NAME	"bootloader-env"
#define LDR_ENV_KEY_BOOTDEV	"BOOTDEV"

struct seil_header {
	uint64_t	id;		/* Identifier */
	uint8_t		copy[80];	/* Copyright */
	uint32_t	dcrc;		/* Data CRC Checksum */
	uint32_t	vfmt;		/* Image Version Format */
	uint32_t	vmjr;		/* Image Version Major */
	uint32_t	vmnr;		/* Image Version Minor */
	uint8_t		vrel[32];	/* Image Version Release */
	uint32_t	dxor;		/* xor value for Data? */
	uint32_t	dlen;		/* Data Length */
};

/*
 * check whether the current mtd device is active or not
 *
 * example of BOOTDEV value (IIJ SA-W2):
 *   - "flash"   : primary image on flash
 *   - "rescue"  : secondary image on flash
 *   - "usb"     : usb storage
 *   - "lan0/1"  : network
 */
static bool seil_bootdev_is_active(struct device_node *np)
{
	struct mtd_info *env_mtd;
	char *buf, *var, *value, *eq;
	const char *devnm;
	size_t rdlen;
	int ret;

	/*
	 * read bootdev name of the partition
	 * if doesn't exist, return true and skip checking of active device
	 */
	ret = of_property_read_string(np, "iij,bootdev-name", &devnm);
	if (ret == -EINVAL)
		return true;
	else if (ret < 0)
		return false;

	env_mtd = get_mtd_device_nm(LDR_ENV_PART_NAME);
	if (IS_ERR(env_mtd)) {
		pr_err("failed to get mtd device \"%s\"", LDR_ENV_PART_NAME);
		return false;
	}

	buf = vmalloc(env_mtd->size);
	if (!buf)
		return false;

	ret = mtd_read(env_mtd, 0, env_mtd->size, &rdlen, buf);
	if (ret || rdlen != env_mtd->size) {
		pr_err("failed to read from mtd (%d)\n", ret);
		ret = 0;
		goto exit_vfree;
	}

	for (var = buf, ret = false;
	     var < buf + env_mtd->size && *var;
	     var = value + strlen(value) + 1) {
		eq = strchr(var, '=');
		if (!eq)
			break;
		*eq = '\0';
		value = eq + 1;

		pr_debug("ENV: %s=%s\n", var, value);
		if (!strcmp(var, LDR_ENV_KEY_BOOTDEV)) {
			ret = !strcmp(devnm, value);
			break;
		}
	}

exit_vfree:
	vfree(buf);

	return ret;
}

static int mtdsplit_parse_seil_fw(struct mtd_info *master,
				  const struct mtd_partition **pparts,
				  struct mtd_part_parser_data *data)
{
	struct device_node *np = mtd_get_of_node(master);
	struct mtd_partition *parts;
	struct seil_header header;
	size_t image_size = 0;
	size_t rootfs_offset;
	size_t hdrlen = sizeof(header);
	size_t retlen;
	int ret;
	u64 id;

	if (!seil_bootdev_is_active(np))
		return -ENODEV;

	ret = of_property_read_u64(np, "iij,seil-id", &id);
	if (ret) {
		pr_err("failed to get iij,seil-id from dt\n");
		return ret;
	}
	pr_debug("got seil-id=0x%016llx from dt\n", id);

	parts = kcalloc(NR_PARTS, sizeof(*parts), GFP_KERNEL);
	if (!parts)
		return -ENOMEM;

	ret = mtd_read(master, 0, hdrlen, &retlen, (void *)&header);
	if (ret)
		goto err_free_parts;

	if (retlen != hdrlen) {
		ret = -EIO;
		goto err_free_parts;
	}

	if (be64_to_cpu(header.id) != id ||
	    be32_to_cpu(header.vfmt) != SEIL_VFMT) {
		pr_debug("no valid seil image found in \"%s\"\n", master->name);
		ret = -ENODEV;
		goto err_free_parts;
	}

	image_size = hdrlen + be32_to_cpu(header.dlen);
	if (image_size > master->size) {
		pr_err("seil image exceeds MTD device \"%s\"\n", master->name);
		ret = -EINVAL;
		goto err_free_parts;
	}

	/* find the roots after the seil image */
	ret = mtd_find_rootfs_from(master, image_size,
				   master->size, &rootfs_offset, NULL);
	if (ret || (master->size - rootfs_offset) == 0) {
		pr_debug("no rootfs after seil image in \"%s\"\n",
			 master->name);
		ret = -ENODEV;
		goto err_free_parts;
	}

	parts[0].name = KERNEL_PART_NAME;
	parts[0].offset = 0;
	parts[0].size = rootfs_offset;

	parts[1].name = ROOTFS_PART_NAME;
	parts[1].offset = rootfs_offset;
	parts[1].size = master->size - rootfs_offset;

	*pparts = parts;
	return NR_PARTS;

err_free_parts:
	kfree(parts);
	return ret;
}

static const struct of_device_id mtdsplit_seil_fw_of_match_table[] = {
	{ .compatible = "iij,seil-firmware" },
	{},
};
MODULE_DEVICE_TABLE(of, mtdsplit_seil_fw_of_match_table);

static struct mtd_part_parser mtdsplit_seil_fw_parser = {
	.owner = THIS_MODULE,
	.name = "seil-fw",
	.of_match_table = mtdsplit_seil_fw_of_match_table,
	.parse_fn = mtdsplit_parse_seil_fw,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

module_mtd_part_parser(mtdsplit_seil_fw_parser);
