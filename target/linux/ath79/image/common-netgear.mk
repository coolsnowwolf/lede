define Build/netgear-rootfs
	mkimage \
		-A mips -O linux -T filesystem -C none \
		$(if $(UIMAGE_MAGIC),-M $(UIMAGE_MAGIC)) \
		-n '$(VERSION_DIST) filesystem' \
		-d $(IMAGE_ROOTFS) $@.fs
	cat $@.fs >> $@
	rm -rf $@.fs
endef

define Build/netgear-squashfs
	rm -rf $@.fs $@.squashfs
	mkdir -p $@.fs/image
	cp $@ $@.fs/image/uImage
	$(STAGING_DIR_HOST)/bin/mksquashfs3-lzma  \
		$@.fs $@.squashfs -be \
		-noappend -root-owned -b 65536 \
		$(if $(SOURCE_DATE_EPOCH),-fixed-time $(SOURCE_DATE_EPOCH))

	dd if=/dev/zero bs=1k count=1 >> $@.squashfs
	mkimage \
		-A mips -O linux -T filesystem -C none \
		$(if $(UIMAGE_MAGIC),-M $(UIMAGE_MAGIC)) \
		-a 0xbf070000 -e 0xbf070000 \
		-n 'MIPS $(VERSION_DIST) Linux-$(LINUX_VERSION)' \
		-d $@.squashfs $@
	rm -rf $@.squashfs $@.fs
endef

define Device/netgear_generic
  DEVICE_VENDOR := NETGEAR
  KERNEL := kernel-bin | append-dtb | lzma -d20 | uImage lzma
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma -d20 | uImage lzma
  IMAGES += factory.img
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | \
	netgear-squashfs | append-rootfs | pad-rootfs
  IMAGE/sysupgrade.bin := $$(IMAGE/default) | check-size | append-metadata
  IMAGE/factory.img := $$(IMAGE/default) | netgear-dni | check-size
endef
