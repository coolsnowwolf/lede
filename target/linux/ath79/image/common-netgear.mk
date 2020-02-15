DEVICE_VARS += NETGEAR_KERNEL_MAGIC NETGEAR_BOARD_ID NETGEAR_HW_ID

define Build/netgear-rootfs
	mkimage \
		-A mips -O linux -T filesystem -C none \
		-M $(NETGEAR_KERNEL_MAGIC) \
		-n '$(VERSION_DIST) filesystem' \
		-d $(IMAGE_ROOTFS) $@.fs
	cat $@.fs >> $@
	rm -rf $@.fs
endef

define Build/netgear-squashfs
	rm -rf $@.fs $@.squashfs
	mkdir -p $@.fs/image
	cp $@ $@.fs/image/uImage
	$(STAGING_DIR_HOST)/bin/mksquashfs-lzma  \
		$@.fs $@.squashfs -be \
		-noappend -root-owned -b 65536 \
		$(if $(SOURCE_DATE_EPOCH),-fixed-time $(SOURCE_DATE_EPOCH))

	dd if=/dev/zero bs=1k count=1 >> $@.squashfs
	mkimage \
		-A mips -O linux -T filesystem -C none \
		-M $(NETGEAR_KERNEL_MAGIC) \
		-a 0xbf070000 -e 0xbf070000 \
		-n 'MIPS $(VERSION_DIST) Linux-$(LINUX_VERSION)' \
		-d $@.squashfs $@
	rm -rf $@.squashfs $@.fs
endef

define Build/netgear-uImage
	$(call Build/uImage,$(1) -M $(NETGEAR_KERNEL_MAGIC))
endef

define Device/netgear_ath79
  DEVICE_VENDOR := NETGEAR
  KERNEL := kernel-bin | append-dtb | lzma -d20 | netgear-uImage lzma
  IMAGES += factory.img
  IMAGE/sysupgrade.bin := $$(IMAGE/default) | append-metadata | \
	check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.img := $$(IMAGE/default) | netgear-dni | \
	check-size $$$$(IMAGE_SIZE)
endef
