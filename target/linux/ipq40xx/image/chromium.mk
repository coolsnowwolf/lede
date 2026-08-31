DTS_DIR := $(DTS_DIR)/qcom

define Build/cros-gpt
	cp $@ $@.tmp 2>/dev/null || true
	ptgen -o $@.tmp -g \
		-T cros_kernel	-N kernel -p $(CONFIG_TARGET_KERNEL_PARTSIZE)m \
				-N rootfs -p $(CONFIG_TARGET_ROOTFS_PARTSIZE)m
	cat $@.tmp >> $@
	rm $@.tmp
endef

define Build/append-kernel-part
	dd if=$(IMAGE_KERNEL) bs=$(CONFIG_TARGET_KERNEL_PARTSIZE)M conv=sync >> $@
endef

# NB: Chrome OS bootloaders replace the '%U' in command lines with the UUID of
# the kernel partition it chooses to boot from. This gives a flexible way to
# consistently build and sign kernels that always use the subsequent
# (PARTNROFF=1) partition as their rootfs.
define Build/cros-vboot
	$(STAGING_DIR_HOST)/bin/cros-vbutil \
		-k $@ -c "root=PARTUUID=%U/PARTNROFF=1" -o $@.new
	@mv $@.new $@
endef

define Device/google_wifi
	DEVICE_VENDOR := Google
	DEVICE_MODEL := WiFi (Gale)
	SOC := qcom-ipq4019
	KERNEL_SUFFIX := -fit-zImage.itb.vboot
	KERNEL = kernel-bin | fit none $$(KDIR)/image-$$(DEVICE_DTS).dtb | cros-vboot
	KERNEL_NAME := zImage
	IMAGES += factory.bin
	IMAGE/factory.bin := cros-gpt | append-kernel-part | append-rootfs
	# Note: Chromium/Depthcharge-based bootloaders insert a reserved-memory
	# ramoops node into the Device Tree automatically, so we can use
	# kmod-ramoops.
	DEVICE_PACKAGES := partx-utils mkf2fs e2fsprogs \
			   kmod-fs-ext4 kmod-fs-f2fs kmod-google-firmware \
			   kmod-leds-lp5523 kmod-ramoops
endef
TARGET_DEVICES += google_wifi
