DEVICE_VARS += SENAO_IMGNAME

# This needs to make OEM config archive 'sysupgrade.tgz' an empty file prior to OEM
# sysupgrade, as otherwise it will implant the old configuration from
# OEM firmware when writing rootfs from factory.bin
# rootfs size and checksum is taken from a squashfs header
# the header does not exist, therefore, supply the size and md5
define Build/senao-tar-gz
	-[ -f "$@" ] && \
	mkdir -p $@.tmp && \
	touch $@.tmp/failsafe.bin && \
	touch $@.tmp/FWINFO-$(word 1,$(1))-$(REVISION) && \
	echo '#!/bin/sh' > $@.tmp/before-upgrade.sh && \
	echo ': > /tmp/sysupgrade.tgz' >> $@.tmp/before-upgrade.sh && \
	echo ': > /tmp/_sys/sysupgrade.tgz' >> $@.tmp/before-upgrade.sh && \
	echo -n $$(( $$(cat $@ | wc -c) / 4096 * 4096 )) > $@.len && \
	dd if=$@ bs=$$(cat $@.len) count=1 | md5sum - | cut -d ' ' -f 1 > $@.md5 && \
	echo '#!/bin/sh' > $@.tmp/after-upgrade.sh && \
	printf 'fw_setenv rootfs_size 0x%08x\n' $$(cat $@.len) >> $@.tmp/after-upgrade.sh && \
	printf 'fw_setenv rootfs_checksum %s\n' $$(cat $@.md5) >> $@.tmp/after-upgrade.sh && \
	$(CP) $(KDIR)/loader-$(DEVICE_NAME).uImage \
		$@.tmp/openwrt-$(word 1,$(1))-uImage-lzma.bin && \
	$(CP) $@ $@.tmp/openwrt-$(word 1,$(1))-root.squashfs && \
	$(TAR) -cp --numeric-owner --owner=0 --group=0 --mode=a-s --sort=name \
		$(if $(SOURCE_DATE_EPOCH),--mtime="@$(SOURCE_DATE_EPOCH)") \
		-C $@.tmp . | gzip -9n > $@ && \
	rm -rf $@.tmp $@.len $@.md5
endef

define Device/senao_loader_okli
  $(Device/loader-okli-uimage)
  KERNEL := kernel-bin | append-dtb | lzma | uImage lzma -M 0x73714f4b
  LOADER_KERNEL_MAGIC := 0x73714f4b
  IMAGES += factory.bin
  IMAGE/factory.bin := append-kernel | pad-to $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | \
	check-size | senao-tar-gz $$$$(SENAO_IMGNAME)
  IMAGE/sysupgrade.bin := $$(IMAGE/factory.bin) | append-metadata
endef
