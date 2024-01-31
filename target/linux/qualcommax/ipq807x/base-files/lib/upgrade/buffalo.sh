. /lib/functions.sh

# Prepare UBI devices for OpenWrt installation
# - rootfs (mtd22)
#   - remove "ubi_rootfs" volume (rootfs on stock)
#   - remove "fw_hash" volume (firmware hash)
# - user_property (mtd24)
#   - remove "user_property_ubi" volume (user configuration)
#   - remove "extra_property" volume (gzipped syslog)
buffalo_upgrade_prepare() {
	local ubi_rootdev ubi_propdev

	if ! ubi_rootdev="$(nand_attach_ubi rootfs)" || \
	   ! ubi_propdev="$(nand_attach_ubi user_property)"; then
		echo "failed to attach UBI volume \"rootfs\" or \"user_property\", rebooting..."
		reboot -f
	fi

	ubirmvol /dev/$ubi_rootdev -N ubi_rootfs &> /dev/null || true
	ubirmvol /dev/$ubi_rootdev -N fw_hash &> /dev/null || true

	ubirmvol /dev/$ubi_propdev -N user_property_ubi &> /dev/null || true
	ubirmvol /dev/$ubi_propdev -N extra_property &> /dev/null || true
}

# Re-create small dummy ubi_rootfs volume and update
# fw_hash volume to pass the checking by U-Boot
# - rootfs (mtd22)
#   - re-create "ubi_rootfs" volume
#   - re-create and update "fw_hash" volume
# - rootfs_recover (mtd23)
#   - update "fw_hash" volume
buffalo_upgrade_optvol() {
	local ubi_rootdev ubi_rcvrdev
	local hashvol_root hashvol_rcvr

	if ! ubi_rootdev="$(nand_attach_ubi rootfs)" || \
	   ! ubi_rcvrdev="$(nand_attach_ubi rootfs_recover)"; then
		echo "failed to attach UBI volume \"rootfs\" or \"rootfs_recover\", rebooting..."
		reboot -f
	fi

	ubimkvol /dev/$ubi_rootdev -N ubi_rootfs -S 1
	ubimkvol /dev/$ubi_rootdev -N fw_hash -S 1 -t static

	if ! hashvol_root="$(nand_find_volume $ubi_rootdev fw_hash)" || \
	   ! hashvol_rcvr="$(nand_find_volume $ubi_rcvrdev fw_hash)"; then
		echo "\"fw_hash\" volume in \"rootfs\" or \"rootfs_recover\" not found, rebooting..."
		reboot -f
	fi

	echo -n "00000000000000000000000000000000" > /tmp/dummyhash.txt
	ubiupdatevol /dev/$hashvol_root /tmp/dummyhash.txt
	ubiupdatevol /dev/$hashvol_rcvr /tmp/dummyhash.txt
}
