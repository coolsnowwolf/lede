#!/bin/sh

BLKSZ=65536

[ -f "$1" -a -f "$2" ] || {
	echo "Usage: $0 <kernel image> <rootfs image> [output file]"
	exit 1
}

IMAGE=${3:-openwrt-combined.img}

# Make sure provided images are 64k aligned.
kern="${IMAGE}.kernel"
root="${IMAGE}.rootfs"
dd if="$1" of="$kern" bs=$BLKSZ conv=sync 2>/dev/null
dd if="$2" of="$root" bs=$BLKSZ conv=sync 2>/dev/null

# Calculate md5sum over combined kernel and rootfs image.
md5=$(cat "$kern" "$root" | $MKHASH md5)

# Write image header followed by kernel and rootfs image.
# The header is padded to 64k, format is:
#  CI               magic word ("Combined Image")
#  <kernel length>  length of kernel encoded as zero padded 8 digit hex
#  <rootfs length>  length of rootfs encoded as zero padded 8 digit hex
#  <md5sum>         checksum of the combined kernel and rootfs image
( printf "CI%08x%08x%32s" \
	$(stat -c "%s" "$kern") $(stat -c "%s" "$root") "${md5%% *}" | \
	dd bs=$BLKSZ conv=sync;
  cat "$kern" "$root"
) > ${IMAGE} 2>/dev/null

# Clean up.
rm -f "$kern" "$root"
