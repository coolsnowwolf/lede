DEVICE_VARS += ROOTFS_SIZE

define Build/buffalo-tftp-header
	( \
		echo -n -e "# Airstation Public Fmt1" | dd bs=32 count=1 conv=sync; \
		dd if=$@; \
	) > $@.new
  mv $@.new $@
endef

define Build/buffalo-tag
	$(eval product=$(word 1,$(1)))
	$(STAGING_DIR_HOST)/bin/buffalo-tag \
		-c 0x80041000 -d 0x801e8000 -w 3 \
		-a ath -v 1.99 -m 1.01 -f 1 \
		-b $(product) -p $(product) \
		-r M_ -l mlang8 \
		-i $@ -o $@.new
	mv $@.new $@
endef

define Device/buffalo_bhr-4grv2
  ATH_SOC := qca9558
  DEVICE_TITLE := Buffalo BHR-4GRV2
  BOARDNAME := BHR-4GRV2
  ROOTFS_SIZE := 14528k
  KERNEL_SIZE := 1472k
  IMAGE_SIZE := 16000k
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := \
    append-rootfs | pad-rootfs | pad-to $$$$(ROOTFS_SIZE) | \
    append-kernel | append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := append-kernel | \
    pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | mkbuffaloimg
  SUPPORTED_DEVICES += bhr-4grv2
endef
TARGET_DEVICES += buffalo_bhr-4grv2

define Device/buffalo_whr-g301n
  ATH_SOC := ar7240
  DEVICE_TITLE := Buffalo WHR-G301N
  IMAGE_SIZE := 3712k
  IMAGES += factory.bin tftp.bin
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := $$(IMAGE/default) | buffalo-enc WHR-G301N 1.99 | buffalo-tag WHR-G301N
  IMAGE/tftp.bin := $$(IMAGE/default) | buffalo-tftp-header
  SUPPORTED_DEVICES += whr-g301n
endef
TARGET_DEVICES += buffalo_whr-g301n
