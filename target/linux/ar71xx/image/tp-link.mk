DEVICE_VARS += LOADER_FLASH_OFFS TPLINK_BOARD_ID TPLINK_FLASHLAYOUT TPLINK_HEADER_VERSION TPLINK_HWID TPLINK_HWREV

define Build/copy-file
	cat "$(1)" > "$@"
endef

define Build/loader-okli
	dd if=$(KDIR)/loader-$(1).gz bs=7680 conv=sync of="$@.new"
	cat "$@" >> "$@.new"
	mv "$@.new" "$@"
endef

define Build/loader-okli-compile
	$(call Build/loader-common,FLASH_OFFS=$(LOADER_FLASH_OFFS) FLASH_MAX=0 KERNEL_CMDLINE="$(CMDLINE)")
endef

# combine kernel and rootfs into one image
# mktplinkfw <type> <optional extra arguments to mktplinkfw binary>
# <type> is "sysupgrade" or "factory"
#
# -a align the rootfs start on an <align> bytes boundary
# -j add jffs2 end-of-filesystem markers
# -s strip padding from end of the image
# -X reserve <size> bytes in the firmware image (hexval prefixed with 0x)
define Build/mktplinkfw
	-$(STAGING_DIR_HOST)/bin/mktplinkfw \
		-H $(TPLINK_HWID) -W $(TPLINK_HWREV) -F $(TPLINK_FLASHLAYOUT) -N OpenWrt -V $(REVISION) \
		-m $(TPLINK_HEADER_VERSION) \
		-k $(IMAGE_KERNEL) \
		-r $@ \
		-o $@.new \
		-j -X 0x40000 \
		-a $(call rootfs_align,$(FILESYSTEM)) \
		$(wordlist 2,$(words $(1)),$(1)) \
		$(if $(findstring sysupgrade,$(word 1,$(1))),-s) && mv $@.new $@ || rm -f $@
endef

define Build/uImageArcher
	mkimage -A $(LINUX_KARCH) \
		-O linux -T kernel \
		-C $(1) -a $(KERNEL_LOADADDR) -e $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) \
		-n '$(call toupper,$(LINUX_KARCH)) LEDE Linux-$(LINUX_VERSION)' -d $@ $@.new
	@mv $@.new $@
endef


define Device/tplink
  TPLINK_HWREV := 0x1
  TPLINK_HEADER_VERSION := 1
  LOADER_TYPE := gz
  KERNEL := kernel-bin | patch-cmdline | lzma
  KERNEL_INITRAMFS := kernel-bin | patch-cmdline | lzma | tplink-v1-header
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/sysupgrade.bin := append-rootfs | mktplinkfw sysupgrade
  IMAGE/factory.bin := append-rootfs | mktplinkfw factory
endef

define Device/tplink-nolzma
  $(Device/tplink)
  LOADER_FLASH_OFFS := 0x22000
  COMPILE := loader-$(1).gz
  COMPILE/loader-$(1).gz := loader-okli-compile
  KERNEL := copy-file $(KDIR)/vmlinux.bin.lzma | uImage lzma -M 0x4f4b4c49 | loader-okli $(1)
  KERNEL_INITRAMFS := copy-file $(KDIR)/vmlinux-initramfs.bin.lzma | loader-kernel-cmdline | tplink-v1-header
endef

define Device/tplink-4m
  $(Device/tplink-nolzma)
  TPLINK_FLASHLAYOUT := 4M
  IMAGE_SIZE := 3904k
endef

define Device/tplink-8m
  $(Device/tplink-nolzma)
  TPLINK_FLASHLAYOUT := 8M
  IMAGE_SIZE := 7936k
endef

define Device/tplink-4mlzma
  $(Device/tplink)
  TPLINK_FLASHLAYOUT := 4Mlzma
  IMAGE_SIZE := 3904k
endef

define Device/tplink-8mlzma
  $(Device/tplink)
  TPLINK_FLASHLAYOUT := 8Mlzma
  IMAGE_SIZE := 7936k
endef

define Device/tplink-16mlzma
  $(Device/tplink)
  TPLINK_FLASHLAYOUT := 16Mlzma
  IMAGE_SIZE := 15872k
endef

define Device/archer-cxx
  KERNEL := kernel-bin | patch-cmdline | lzma | uImageArcher lzma
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
endef

define Device/archer-c25-v1
  $(Device/archer-cxx)
  DEVICE_TITLE := TP-LINK Archer C25 v1
  DEVICE_PACKAGES := kmod-ath10k ath10k-firmware-qca9887
  BOARDNAME := ARCHER-C25-V1
  TPLINK_BOARD_ID := ARCHER-C25-V1
  DEVICE_PROFILE := ARCHERC25V1
  IMAGE_SIZE := 7808k
  LOADER_TYPE := elf
  MTDPARTS := spi0.0:128k(factory-uboot)ro,64k(u-boot)ro,1536k(kernel),6272k(rootfs),128k(config)ro,64k(art)ro,7808k@0x30000(firmware)
  SUPPORTED_DEVICES := archer-c25-v1
endef

define Device/archer-c58-v1
  $(Device/archer-cxx)
  DEVICE_TITLE := TP-LINK Archer C58 v1
  DEVICE_PACKAGES := kmod-ath10k
  BOARDNAME := ARCHER-C58-V1
  TPLINK_BOARD_ID := ARCHER-C58-V1
  DEVICE_PROFILE := ARCHERC58V1
  IMAGE_SIZE := 7936k
  MTDPARTS := spi0.0:64k(u-boot)ro,64k(mac)ro,1344k(kernel),6592k(rootfs),64k(tplink)ro,64k(art)ro,7936k@0x20000(firmware)
  SUPPORTED_DEVICES := archer-c58-v1
endef

define Device/archer-c59-v1
  $(Device/archer-cxx)
  DEVICE_TITLE := TP-LINK Archer C59 v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := ARCHER-C59-V1
  TPLINK_BOARD_ID := ARCHER-C59-V1
  DEVICE_PROFILE := ARCHERC59V1
  IMAGE_SIZE := 14528k
  MTDPARTS := spi0.0:64k(u-boot)ro,64k(mac)ro,1536k(kernel),12992k(rootfs),1664k(tplink)ro,64k(art)ro,14528k@0x20000(firmware)
  SUPPORTED_DEVICES := archer-c59-v1
endef

define Device/archer-c60-v1
  $(Device/archer-cxx)
  DEVICE_TITLE := TP-LINK Archer C60 v1
  DEVICE_PACKAGES := kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := ARCHER-C60-V1
  TPLINK_BOARD_ID := ARCHER-C60-V1
  DEVICE_PROFILE := ARCHERC60V1
  IMAGE_SIZE := 7936k
  MTDPARTS := spi0.0:64k(u-boot)ro,64k(mac)ro,1344k(kernel),6592k(rootfs),64k(tplink)ro,64k(art)ro,7936k@0x20000(firmware)
  SUPPORTED_DEVICES := archer-c60-v1
endef
TARGET_DEVICES += archer-c25-v1 archer-c58-v1 archer-c59-v1 archer-c60-v1

define Device/archer-c5-v1
  $(Device/tplink-16mlzma)
  DEVICE_TITLE := TP-LINK Archer C5 v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := ARCHER-C5
  DEVICE_PROFILE := ARCHERC7
  TPLINK_HWID := 0xc5000001
endef

define Device/archer-c7-v1
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK Archer C7 v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := ARCHER-C7
  DEVICE_PROFILE := ARCHERC7
  TPLINK_HWID := 0x75000001
endef

define Device/archer-c7-v2
  $(Device/tplink-16mlzma)
  DEVICE_TITLE := TP-LINK Archer C7 v2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := ARCHER-C7-V2
  DEVICE_PROFILE := ARCHERC7
  TPLINK_HWID := 0xc7000002
  IMAGES := sysupgrade.bin factory.bin factory-us.bin factory-eu.bin
  IMAGE/factory-us.bin := append-rootfs | mktplinkfw factory -C US
  IMAGE/factory-eu.bin := append-rootfs | mktplinkfw factory -C EU
endef

define Device/archer-c7-v2-il
  $(Device/tplink-16mlzma)
  DEVICE_TITLE := TP-LINK Archer C7 v2 (IL)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := ARCHER-C7-V2
  DEVICE_PROFILE := ARCHERC7
  TPLINK_HWID := 0xc7000002
  TPLINK_HWREV := 0x494c0001
endef

define Device/tl-wdr7500-v3
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK Archer C7 v3
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := ARCHER-C7
  DEVICE_PROFILE := ARCHERC7
  TPLINK_HWID := 0x75000003
endef
TARGET_DEVICES += archer-c5-v1 archer-c7-v1 archer-c7-v2 archer-c7-v2-il tl-wdr7500-v3

define Device/cpe510-520-v1
  DEVICE_TITLE := TP-LINK CPE510/520 v1
  DEVICE_PACKAGES := rssileds
  MTDPARTS := spi0.0:128k(u-boot)ro,64k(pation-table)ro,64k(product-info)ro,1536k(kernel),6144k(rootfs),192k(config)ro,64k(ART)ro,7680k@0x40000(firmware)
  IMAGE_SIZE := 7680k
  BOARDNAME := CPE510
  TPLINK_BOARD_ID := CPE510
  DEVICE_PROFILE := CPE510
  LOADER_TYPE := elf
  KERNEL := kernel-bin | patch-cmdline | lzma | loader-kernel
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
endef

define Device/cpe210-220-v1
  $(Device/cpe510-520-v1)
  DEVICE_TITLE := TP-LINK CPE210/220 v1
  BOARDNAME := CPE210
  TPLINK_BOARD_ID := CPE210
endef

define Device/wbs210-v1
  $(Device/cpe510-520-v1)
  DEVICE_TITLE := TP-LINK WBS210 v1
  BOARDNAME := WBS210
  TPLINK_BOARD_ID := WBS210
endef

define Device/wbs510-v1
  $(Device/cpe510-520-v1)
  DEVICE_TITLE := TP-LINK WBS510 v1
  BOARDNAME := WBS510
  TPLINK_BOARD_ID := WBS510
endef
TARGET_DEVICES += cpe210-220-v1 cpe510-520-v1 wbs210-v1 wbs510-v1

define Device/eap120-v1
  DEVICE_TITLE := TP-LINK EAP120 v1
  MTDPARTS := spi0.0:128k(u-boot)ro,64k(pation-table)ro,64k(product-info)ro,1536k(kernel),14336k(rootfs),192k(config)ro,64k(ART)ro,15872k@0x40000(firmware)
  IMAGE_SIZE := 15872k
  BOARDNAME := EAP120
  TPLINK_BOARD_ID := EAP120
  DEVICE_PROFILE := EAP120
  LOADER_TYPE := elf
  KERNEL := kernel-bin | patch-cmdline | lzma | loader-kernel
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
endef
TARGET_DEVICES += eap120-v1

define Device/re450-v1
  DEVICE_TITLE := TP-LINK RE450 v1
  DEVICE_PACKAGES := kmod-ath10k ath10k-firmware-qca988x
  MTDPARTS := spi0.0:128k(u-boot)ro,1344k(kernel),4672k(rootfs),64k(pation-table)ro,64k(product-info)ro,1856k(config)ro,64k(art)ro,6016k@0x20000(firmware)
  IMAGE_SIZE := 7936k
  BOARDNAME := RE450
  TPLINK_BOARD_ID := RE450
  DEVICE_PROFILE := RE450
  LOADER_TYPE := elf
  TPLINK_HWID := 0x0
  TPLINK_HWREV := 0
  TPLINK_HEADER_VERSION := 1
  KERNEL := kernel-bin | patch-cmdline | lzma | tplink-v1-header
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
endef
TARGET_DEVICES += re450-v1

define Device/tl-mr10u-v1
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-MR10U v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := TL-MR10U
  DEVICE_PROFILE := TLMR10U
  TPLINK_HWID := 0x00100101
  CONSOLE := ttyATH0,115200
endef

define Device/tl-mr11u-v1
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-MR11U v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-MR11U
  DEVICE_PROFILE := TLMR11U
  TPLINK_HWID := 0x00110101
  CONSOLE := ttyATH0,115200
endef

define Device/tl-mr11u-v2
  $(Device/tl-mr11u-v1)
  DEVICE_TITLE := TP-LINK TL-MR11U v2
  TPLINK_HWID := 0x00110102
endef

define Device/tl-mr12u-v1
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-MR12U v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-MR13U
  DEVICE_PROFILE := TLMR12U
  TPLINK_HWID := 0x00120101
  CONSOLE := ttyATH0,115200
endef

define Device/tl-mr13u-v1
  $(Device/tl-mr12u-v1)
  DEVICE_TITLE := TP-LINK TL-MR13U v1
  DEVICE_PROFILE := TLMR13U
  TPLINK_HWID := 0x00130101
endef
TARGET_DEVICES += tl-mr10u-v1 tl-mr11u-v1 tl-mr11u-v2 tl-mr12u-v1 tl-mr13u-v1

define Device/tl-mr3020-v1
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-MR3020 v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-MR3020
  DEVICE_PROFILE := TLMR3020
  TPLINK_HWID := 0x30200001
  CONSOLE := ttyATH0,115200
endef

define Device/tl-mr3040-v1
  $(Device/tl-mr3020-v1)
  DEVICE_TITLE := TP-LINK TL-MR3040 v1
  BOARDNAME := TL-MR3040
  DEVICE_PROFILE := TLMR3040
  TPLINK_HWID := 0x30400001
endef

define Device/tl-mr3040-v2
  $(Device/tl-mr3040-v1)
  DEVICE_TITLE := TP-LINK TL-MR3040 v2
  BOARDNAME := TL-MR3040-v2
  TPLINK_HWID := 0x30400002
endef

define Device/tl-mr3220-v1
  $(Device/tplink-4m)
  DEVICE_TITLE := TP-LINK TL-MR3220 v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-MR3220
  DEVICE_PROFILE := TLMR3220
  TPLINK_HWID := 0x32200001
endef

define Device/tl-mr3220-v2
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-MR3220 v2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-MR3220-v2
  DEVICE_PROFILE := TLMR3220
  TPLINK_HWID := 0x32200002
  CONSOLE := ttyATH0,115200
endef

define Device/tl-mr3420-v1
  $(Device/tplink-4m)
  DEVICE_TITLE := TP-LINK TL-MR3420 v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-MR3420
  DEVICE_PROFILE := TLMR3420
  TPLINK_HWID := 0x34200001
endef

define Device/tl-mr3420-v2
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-MR3420 v2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-MR3420-v2
  DEVICE_PROFILE := TLMR3420
  TPLINK_HWID := 0x34200002
endef
TARGET_DEVICES += tl-mr3020-v1 tl-mr3040-v1 tl-mr3040-v2 tl-mr3220-v1 tl-mr3220-v2 tl-mr3420-v1 tl-mr3420-v2

define Device/tl-mr6400-v1
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK TL-MR6400 v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-net kmod-usb-net-rndis kmod-usb-serial kmod-usb-serial-option adb-enablemodem
  BOARDNAME := TL-MR6400
  DEVICE_PROFILE := TLMR6400
  TPLINK_HWID := 0x64000001
endef
TARGET_DEVICES += tl-mr6400-v1

define Device/tl-wa701nd-v1
  $(Device/tplink-4m)
  DEVICE_TITLE := TP-LINK TL-WA701N/ND v1
  BOARDNAME := TL-WA901ND
  DEVICE_PROFILE := TLWA701
  TPLINK_HWID := 0x07010001
endef

define Device/tl-wa701nd-v2
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WA701N/ND v2
  BOARDNAME := TL-WA701ND-v2
  DEVICE_PROFILE := TLWA701
  TPLINK_HWID := 0x07010002
  CONSOLE := ttyATH0,115200
endef

define Device/tl-wa7210n-v2
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WA7210N v2
  DEVICE_PACKAGES := rssileds kmod-ledtrig-netdev
  BOARDNAME := TL-WA7210N-v2
  DEVICE_PROFILE := TLWA7210
  TPLINK_HWID := 0x72100002
  CONSOLE := ttyATH0,115200
endef

define Device/tl-wa730re-v1
  $(Device/tplink-4m)
  DEVICE_TITLE := TP-LINK TL-WA730RE v1
  BOARDNAME := TL-WA901ND
  DEVICE_PROFILE := TLWA730RE
  TPLINK_HWID := 0x07300001
endef

define Device/tl-wa750re-v1
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WA750RE v1
  DEVICE_PACKAGES := rssileds
  BOARDNAME := TL-WA750RE
  DEVICE_PROFILE := TLWA750
  TPLINK_HWID := 0x07500001
endef

define Device/tl-wa7510n-v1
  $(Device/tplink-4m)
  DEVICE_TITLE := TP-LINK TL-WA7510N v1
  BOARDNAME := TL-WA7510N
  DEVICE_PROFILE := TLWA7510
  TPLINK_HWID := 0x75100001
endef
TARGET_DEVICES += tl-wa701nd-v1 tl-wa701nd-v2 tl-wa7210n-v2 tl-wa730re-v1 tl-wa750re-v1 tl-wa7510n-v1

define Device/tl-wa801nd-v1
  $(Device/tplink-4m)
  DEVICE_TITLE := TP-LINK TL-WA801N/ND v1
  BOARDNAME := TL-WA901ND
  DEVICE_PROFILE := TLWA801
  TPLINK_HWID := 0x08010001
endef

define Device/tl-wa801nd-v2
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WA801N/ND v2
  BOARDNAME := TL-WA801ND-v2
  DEVICE_PROFILE := TLWA801
  TPLINK_HWID := 0x08010002
endef

define Device/tl-wa801nd-v3
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WA801N/ND v3
  BOARDNAME := TL-WA801ND-v3
  DEVICE_PROFILE := TLWA801
  TPLINK_HWID := 0x08010003
endef

define Device/tl-wa830re-v1
  $(Device/tplink-4m)
  DEVICE_TITLE := TP-LINK TL-WA830RE v1
  BOARDNAME := TL-WA901ND
  DEVICE_PROFILE := TLWA830
  TPLINK_HWID := 0x08300010
endef

define Device/tl-wa830re-v2
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WA830RE v2
  BOARDNAME := TL-WA830RE-v2
  DEVICE_PROFILE := TLWA830
  TPLINK_HWID := 0x08300002
endef

define Device/tl-wa850re-v1
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WA850RE v1
  BOARDNAME := TL-WA850RE
  DEVICE_PROFILE := TLWA850
  TPLINK_HWID := 0x08500001
endef

define Device/tl-wa85xre
  $(Device/tplink)
  TPLINK_HWREV := 0
  KERNEL := kernel-bin | patch-cmdline | lzma | tplink-v1-header
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
  MTDPARTS := spi0.0:128k(u-boot)ro,1344k(kernel),2304k(rootfs),256k(config)ro,64k(art)ro,3648k@0x20000(firmware)
endef

define Device/tl-wa850re-v2
  $(Device/tl-wa85xre)
  DEVICE_TITLE := TP-LINK TL-WA850RE v2
  DEVICE_PACKAGES := rssileds
  BOARDNAME := TL-WA850RE-V2
  DEVICE_PROFILE := TLWA850
  TPLINK_BOARD_ID := TLWA850REV2
  TPLINK_HWID := 0x08500002
endef

define Device/tl-wa855re-v1
  $(Device/tl-wa85xre)
  DEVICE_TITLE := TP-LINK TL-WA855RE v1
  BOARDNAME := TL-WA855RE-v1
  DEVICE_PROFILE := TLWA855RE
  TPLINK_HWID := 0x08550001
  TPLINK_BOARD_ID := TLWA855REV1
endef

define Device/tl-wa860re-v1
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WA860RE v1
  BOARDNAME := TL-WA860RE
  DEVICE_PROFILE := TLWA860
  TPLINK_HWID := 0x08600001
endef
TARGET_DEVICES += tl-wa801nd-v1 tl-wa801nd-v2 tl-wa801nd-v3 tl-wa830re-v1 tl-wa830re-v2 tl-wa850re-v1 tl-wa850re-v2 tl-wa855re-v1 tl-wa860re-v1

define Device/tl-wa901nd-v1
  $(Device/tplink-4m)
  DEVICE_TITLE := TP-LINK TL-WA901N/ND v1
  BOARDNAME := TL-WA901ND
  DEVICE_PROFILE := TLWA901
  TPLINK_HWID := 0x09010001
endef

define Device/tl-wa901nd-v2
  $(Device/tplink-4m)
  DEVICE_TITLE := TP-LINK TL-WA901N/ND v2
  BOARDNAME := TL-WA901ND-v2
  DEVICE_PROFILE := TLWA901
  TPLINK_HWID := 0x09010002
endef

define Device/tl-wa901nd-v3
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WA901N/ND v3
  BOARDNAME := TL-WA901ND-v3
  DEVICE_PROFILE := TLWA901
  TPLINK_HWID := 0x09010003
endef

define Device/tl-wa901nd-v4
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WA901N/ND v4
  BOARDNAME := TL-WA901ND-v4
  DEVICE_PROFILE := TLWA901
  TPLINK_HWID := 0x09010004
  IMAGE/factory.bin := append-rootfs | mktplinkfw factory -C EU
endef
TARGET_DEVICES += tl-wa901nd-v1 tl-wa901nd-v2 tl-wa901nd-v3 tl-wa901nd-v4

define Device/tl-wdr3320-v2
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WDR3320 v2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-WDR3320-v2
  DEVICE_PROFILE := TLWDR3320V2
  TPLINK_HWID := 0x33200002
  TPLINK_HEADER_VERSION := 2
endef

define Device/tl-wdr3500-v1
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK TL-WDR3500 v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-WDR3500
  DEVICE_PROFILE := TLWDR4300
  TPLINK_HWID := 0x35000001
endef

define Device/tl-wdr3600-v1
  $(Device/tl-wdr3500-v1)
  DEVICE_TITLE := TP-LINK TL-WDR3600 v1
  BOARDNAME := TL-WDR4300
  TPLINK_HWID := 0x36000001
  IMAGE/factory.bin := append-rootfs | mktplinkfw factory -C US
endef

define Device/tl-wdr4300-v1
  $(Device/tl-wdr3600-v1)
  DEVICE_TITLE := TP-LINK TL-WDR4300 v1
  TPLINK_HWID := 0x43000001
endef

define Device/tl-wdr4300-v1-il
  $(Device/tl-wdr3500-v1)
  DEVICE_TITLE := TP-LINK TL-WDR4300 v1 (IL)
  BOARDNAME := TL-WDR4300
  TPLINK_HWID := 0x43008001
endef

define Device/tl-wdr4310-v1
  $(Device/tl-wdr4300-v1-il)
  DEVICE_TITLE := TP-LINK TL-WDR4310 v1
  TPLINK_HWID := 0x43100001
endef

define Device/tl-wdr4900-v2
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK TL-WDR4900 v2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-WDR4900-v2
  DEVICE_PROFILE := TLWDR4900V2
  TPLINK_HWID := 0x49000002
endef

define Device/tl-wdr6500-v2
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK TL-WDR6500 v2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k ath10k-firmware-qca988x
  KERNEL := kernel-bin | patch-cmdline | lzma | uImage lzma
  KERNEL_INITRAMFS := kernel-bin | patch-cmdline | lzma | uImage lzma | tplink-v1-header
  BOARDNAME := TL-WDR6500-v2
  DEVICE_PROFILE := TLWDR6500V2
  TPLINK_HWID := 0x65000002
  TPLINK_HEADER_VERSION := 2
endef

define Device/mw4530r-v1
  $(Device/tl-wdr4300-v1)
  DEVICE_TITLE := Mercury MW4530R v1
  TPLINK_HWID := 0x45300001
endef
TARGET_DEVICES += tl-wdr3320-v2 tl-wdr3500-v1 tl-wdr3600-v1 tl-wdr4300-v1 tl-wdr4300-v1-il tl-wdr4310-v1 tl-wdr4900-v2 tl-wdr6500-v2 mw4530r-v1

define Device/tl-wpa8630-v1
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK TL-WPA8630 v1
  DEVICE_PACKAGES := kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := TL-WPA8630
  DEVICE_PROFILE := TL-WPA8630
  TPLINK_HWID := 0x86300001
endef
TARGET_DEVICES += tl-wpa8630-v1

define Device/tl-wr1041n-v2
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WR1041N v2
  BOARDNAME := TL-WR1041N-v2
  DEVICE_PROFILE := TLWR1041
  TPLINK_HWID := 0x10410002
endef
TARGET_DEVICES += tl-wr1041n-v2

define Device/tl-wr1043nd-v1
  $(Device/tplink-8m)
  DEVICE_TITLE := TP-LINK TL-WR1043N/ND v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-WR1043ND
  DEVICE_PROFILE := TLWR1043
  TPLINK_HWID := 0x10430001
endef

define Device/tl-wr1043nd-v2
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK TL-WR1043N/ND v2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-WR1043ND-v2
  DEVICE_PROFILE := TLWR1043
  TPLINK_HWID := 0x10430002
endef

define Device/tl-wr1043nd-v3
  $(Device/tl-wr1043nd-v2)
  DEVICE_TITLE := TP-LINK TL-WR1043N/ND v3
  TPLINK_HWID := 0x10430003
endef

define Device/tl-wr1043nd-v4
  $(Device/tplink)
  DEVICE_TITLE := TP-LINK TL-WR1043N/ND v4
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-WR1043ND-v4
  DEVICE_PROFILE := TLWR1043
  TPLINK_HWID :=  0x10430004
  MTDPARTS := spi0.0:128k(u-boot)ro,1536k(kernel),14016k(rootfs),128k(product-info)ro,320k(config)ro,64k(partition-table)ro,128k(logs)ro,64k(ART)ro,15552k@0x20000(firmware)
  IMAGE_SIZE := 15552k
  TPLINK_BOARD_ID := TLWR1043NDV4
  KERNEL := kernel-bin | patch-cmdline | lzma | tplink-v1-header
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
endef

define Device/tl-wr2543-v1
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK TL-WR2543N/ND v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-WR2543N
  DEVICE_PROFILE := TLWR2543
  TPLINK_HWID := 0x25430001
  IMAGE/sysupgrade.bin := append-rootfs | mktplinkfw sysupgrade -v 3.13.99
  IMAGE/factory.bin := append-rootfs | mktplinkfw factory -v 3.13.99
endef
TARGET_DEVICES += tl-wr1043nd-v1 tl-wr1043nd-v2 tl-wr1043nd-v3 tl-wr1043nd-v4 tl-wr2543-v1

define Device/tl-wr703n-v1
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WR703N v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := TL-WR703N
  DEVICE_PROFILE := TLWR703
  TPLINK_HWID := 0x07030101
  CONSOLE := ttyATH0,115200
endef

define Device/tl-wr710n-v1
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK TL-WR710N v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := TL-WR710N
  DEVICE_PROFILE := TLWR710
  TPLINK_HWID := 0x07100001
  CONSOLE := ttyATH0,115200
  IMAGE/factory.bin := append-rootfs | mktplinkfw factory -C US
endef

define Device/tl-wr710n-v2
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WR710N v2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := TL-WR710N
  DEVICE_PROFILE := TLWR710
  TPLINK_HWID := 0x07100002
  CONSOLE := ttyATH0,115200
endef

define Device/tl-wr710n-v2.1
  $(Device/tl-wr710n-v1)
  DEVICE_TITLE := TP-LINK TL-WR710N v2.1
  TPLINK_HWID := 0x07100002
  TPLINK_HWREV := 0x00000002
endef

define Device/tl-wr720n-v3
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WR720N v3
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := TL-WR720N-v3
  DEVICE_PROFILE := TLWR720
  TPLINK_HWID := 0x07200103
  CONSOLE := ttyATH0,115200
endef

define Device/tl-wr720n-v4
  $(Device/tl-wr720n-v3)
  DEVICE_TITLE := TP-LINK TL-WR720N v4
  TPLINK_HWID := 0x07200104
endef
TARGET_DEVICES += tl-wr703n-v1 tl-wr710n-v1 tl-wr710n-v2 tl-wr710n-v2.1 tl-wr720n-v3 tl-wr720n-v4

define Device/tl-wr740n-v1
  $(Device/tplink-4m)
  DEVICE_TITLE := TP-LINK TL-WR740N/ND v1
  BOARDNAME := TL-WR741ND
  DEVICE_PROFILE := TLWR740
  TPLINK_HWID := 0x07400001
endef

define Device/tl-wr740n-v3
  $(Device/tl-wr740n-v1)
  DEVICE_TITLE := TP-LINK TL-WR740N/ND v3
  TPLINK_HWID := 0x07400003
endef

define Device/tl-wr740n-v4
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WR740N/ND v4
  BOARDNAME := TL-WR741ND-v4
  DEVICE_PROFILE := TLWR740
  TPLINK_HWID := 0x07400004
  CONSOLE := ttyATH0,115200
endef

define Device/tl-wr740n-v5
  $(Device/tl-wr740n-v4)
  DEVICE_TITLE := TP-LINK TL-WR740N/ND v5
  TPLINK_HWID := 0x07400005
endef

define Device/tl-wr740n-v6
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WR740N/ND v6
  BOARDNAME := TL-WR740N-v6
  DEVICE_PROFILE := TLWR740
  TPLINK_HWID := 0x07400006
endef
TARGET_DEVICES += tl-wr740n-v1 tl-wr740n-v3 tl-wr740n-v4 tl-wr740n-v5 tl-wr740n-v6

define Device/tl-wr741nd-v1
  $(Device/tplink-4m)
  DEVICE_TITLE := TP-LINK TL-WR741N/ND v1
  BOARDNAME := TL-WR741ND
  DEVICE_PROFILE := TLWR741
  TPLINK_HWID := 0x07410001
endef

define Device/tl-wr741nd-v2
  $(Device/tl-wr741nd-v1)
  DEVICE_TITLE := TP-LINK TL-WR741N/ND v2
endef

define Device/tl-wr741nd-v4
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WR741N/ND v4
  BOARDNAME := TL-WR741ND-v4
  DEVICE_PROFILE := TLWR741
  TPLINK_HWID := 0x07410004
  CONSOLE := ttyATH0,115200
endef

define Device/tl-wr741nd-v5
  $(Device/tl-wr741nd-v4)
  DEVICE_TITLE := TP-LINK TL-WR741N/ND v5
  TPLINK_HWID := 0x07400005
endef

define Device/tl-wr743nd-v1
  $(Device/tplink-4m)
  DEVICE_TITLE := TP-LINK TL-WR743N/ND v1
  BOARDNAME := TL-WR741ND
  DEVICE_PROFILE := TLWR743
  TPLINK_HWID := 0x07430001
endef

define Device/tl-wr743nd-v2
  $(Device/tl-wr741nd-v4)
  DEVICE_TITLE := TP-LINK TL-WR743N/ND v2
  DEVICE_PROFILE := TLWR743
  TPLINK_HWID := 0x07430002
endef
TARGET_DEVICES += tl-wr741nd-v1 tl-wr741nd-v2 tl-wr741nd-v4 tl-wr741nd-v5 tl-wr743nd-v1 tl-wr743nd-v2

define Device/tl-wr802n-v1
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WR802N v1
  BOARDNAME := TL-WR802N-v1
  DEVICE_PROFILE := TLWR802
  TPLINK_HWID := 0x08020001
  TPLINK_HWREV := 1
endef

define Device/tl-wr802n-v2
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WR802N v2
  BOARDNAME := TL-WR802N-v2
  DEVICE_PROFILE := TLWR802
  TPLINK_HWID := 0x08020002
  TPLINK_HWREV := 2
  IMAGES += factory-us.bin factory-eu.bin
  IMAGE/factory-us.bin := append-rootfs | mktplinkfw factory -C US
  IMAGE/factory-eu.bin := append-rootfs | mktplinkfw factory -C EU
endef

define Device/tl-wr810n-v1
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK TL-WR810N v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := TL-WR810N
  DEVICE_PROFILE := TLWR810
  TPLINK_HWID := 0x08100001
endef

define Device/tl-wr840n-v2
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WR840N v2
  BOARDNAME := TL-WR840N-v2
  DEVICE_PROFILE := TLWR840
  TPLINK_HWID := 0x08400002
  IMAGES += factory-eu.bin
  IMAGE/factory-eu.bin := append-rootfs | mktplinkfw factory -C EU
endef

define Device/tl-wr840n-v3
  $(Device/tl-wr840n-v2)
  DEVICE_TITLE := TP-LINK TL-WR840N v3
  BOARDNAME := TL-WR840N-v3
  TPLINK_HWID := 0x08400003
endef
TARGET_DEVICES += tl-wr802n-v1 tl-wr802n-v2 tl-wr810n-v1 tl-wr840n-v2 tl-wr840n-v3

define Device/tl-wr841-v1.5
  $(Device/tplink-4m)
  DEVICE_TITLE := TP-LINK TL-WR841N/ND v1.5
  BOARDNAME := TL-WR841N-v1.5
  DEVICE_PROFILE := TLWR841
  TPLINK_HWID := 0x08410002
  TPLINK_HWREV := 2
endef

define Device/tl-wr841-v3
  $(Device/tplink-4m)
  DEVICE_TITLE := TP-LINK TL-WR841N/ND v3
  BOARDNAME := TL-WR941ND
  DEVICE_PROFILE := TLWR841
  TPLINK_HWID := 0x08410003
  TPLINK_HWREV := 3
endef

define Device/tl-wr841-v5
  $(Device/tplink-4m)
  DEVICE_TITLE := TP-LINK TL-WR841N/ND v5
  BOARDNAME := TL-WR741ND
  DEVICE_PROFILE := TLWR841
  TPLINK_HWID := 0x08410005
endef

define Device/tl-wr841-v7
  $(Device/tplink-4m)
  DEVICE_TITLE := TP-LINK TL-WR841N/ND v7
  BOARDNAME := TL-WR841N-v7
  DEVICE_PROFILE := TLWR841
  TPLINK_HWID := 0x08410007
endef

define Device/tl-wr841-v8
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WR841N/ND v8
  BOARDNAME := TL-WR841N-v8
  DEVICE_PROFILE := TLWR841
  TPLINK_HWID := 0x08410008
endef

define Device/tl-wr841-v9
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WR841N/ND v9
  BOARDNAME := TL-WR841N-v9
  DEVICE_PROFILE := TLWR841
  TPLINK_HWID := 0x08410009
endef

define Device/tl-wr841-v10
  $(Device/tl-wr841-v9)
  DEVICE_TITLE := TP-LINK TL-WR841N/ND v10
  TPLINK_HWID := 0x08410010
endef

define Device/tl-wr841-v11
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WR841N/ND v11
  BOARDNAME := TL-WR841N-v11
  DEVICE_PROFILE := TLWR841
  TPLINK_HWID := 0x08410011
  IMAGES += factory-us.bin factory-eu.bin
  IMAGE/factory-us.bin := append-rootfs | mktplinkfw factory -C US
  IMAGE/factory-eu.bin := append-rootfs | mktplinkfw factory -C EU
endef

define Device/tl-wr841-v12
  $(Device/tl-wr841-v11)
  DEVICE_TITLE := TP-LINK TL-WR841N/ND v12
  TPLINK_HWID := 0x08410012
endef
TARGET_DEVICES += tl-wr841-v1.5 tl-wr841-v3 tl-wr841-v5 tl-wr841-v7 tl-wr841-v8 tl-wr841-v9 tl-wr841-v10 tl-wr841-v11 tl-wr841-v12

define Device/tl-wr842n-v1
  $(Device/tplink-8m)
  DEVICE_TITLE := TP-LINK TL-WR842N/ND v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-MR3420
  DEVICE_PROFILE := TLWR842
  TPLINK_HWID := 0x08420001
endef

define Device/tl-wr842n-v2
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK TL-WR842N/ND v2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-WR842N-v2
  DEVICE_PROFILE := TLWR842
  TPLINK_HWID := 0x8420002
endef

define Device/tl-wr842n-v3
  $(Device/tplink-16mlzma)
  DEVICE_TITLE := TP-LINK TL-WR842N/ND v3
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-WR842N-v3
  DEVICE_PROFILE := TLWR842
  TPLINK_HWID := 0x08420003
endef

define Device/tl-wr843nd-v1
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WR843N/ND v1
  BOARDNAME := TL-WR841N-v8
  DEVICE_PROFILE := TLWR843
  TPLINK_HWID := 0x08430001
endef

define Device/tl-wr847n-v8
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WR847N/ND v8
  BOARDNAME := TL-WR841N-v8
  DEVICE_PROFILE := TLWR841
  TPLINK_HWID := 0x08470008
endef
TARGET_DEVICES += tl-wr842n-v1 tl-wr842n-v2 tl-wr842n-v3 tl-wr843nd-v1 tl-wr847n-v8

define Device/tl-wr902ac-v1
  DEVICE_TITLE := TP-LINK TL-WR902AC v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport \
	kmod-ath10k ath10k-firmware-qca9887 -swconfig -uboot-envtools
  BOARDNAME := TL-WR902AC-V1
  DEVICE_PROFILE := TLWR902
  TPLINK_BOARD_ID := TL-WR902AC-V1
  TPLINK_HWID := 0x0
  TPLINK_HWREV := 0
  TPLINK_HEADER_VERSION := 1
  SUPPORTED_DEVICES := tl-wr902ac-v1
  IMAGE_SIZE := 7360k
  KERNEL := kernel-bin | patch-cmdline | lzma | tplink-v1-header
  IMAGES += factory.bin
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  MTDPARTS := spi0.0:128k(u-boot)ro,7360k(firmware),640k(tplink)ro,64k(art)ro
endef
TARGET_DEVICES += tl-wr902ac-v1

define Device/tl-wr940n-v4
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WR940N v4
  BOARDNAME := TL-WR940N-v4
  DEVICE_PROFILE := TLWR941
  TPLINK_HWID := 0x09400004
  IMAGES += factory-us.bin factory-eu.bin
  IMAGE/factory-us.bin := append-rootfs | mktplinkfw factory -C US
  IMAGE/factory-eu.bin := append-rootfs | mktplinkfw factory -C EU
endef

define Device/tl-wr941nd-v2
  $(Device/tplink-4m)
  DEVICE_TITLE := TP-LINK TL-WR941N/ND v2
  BOARDNAME := TL-WR941ND
  DEVICE_PROFILE := TLWR941
  TPLINK_HWID := 0x09410002
  TPLINK_HWREV := 2
endef

define Device/tl-wr941nd-v3
  $(Device/tl-wr941nd-v2)
  DEVICE_TITLE := TP-LINK TL-WR941N/ND v3
endef

define Device/tl-wr941nd-v4
  $(Device/tplink-4m)
  DEVICE_TITLE := TP-LINK TL-WR941N/ND v4
  BOARDNAME := TL-WR741ND
  DEVICE_PROFILE := TLWR941
  TPLINK_HWID := 0x09410004
endef

define Device/tl-wr941nd-v5
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WR941N/ND v5
  BOARDNAME := TL-WR941ND-v5
  DEVICE_PROFILE := TLWR941
  TPLINK_HWID := 0x09410005
endef

define Device/tl-wr941nd-v6
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WR941N/ND v6
  BOARDNAME := TL-WR941ND-v6
  DEVICE_PROFILE := TLWR941
  TPLINK_HWID := 0x09410006
endef

# Chinese version (unlike European) is similar to the TL-WDR3500
define Device/tl-wr941nd-v6-cn
  $(Device/tplink-4mlzma)
  DEVICE_TITLE := TP-LINK TL-WR941N/ND v6 (CN)
  BOARDNAME := TL-WDR3500
  DEVICE_PROFILE := TLWR941
  TPLINK_HWID := 0x09410006
endef

define Device/tl-wr942n-v1
  DEVICE_TITLE := TP-LINK TL-WR942N v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-WR942N-V1
  TPLINK_BOARD_ID := TLWR942NV1
  DEVICE_PROFILE := TLWR942
  IMAGE_SIZE := 14464k
  KERNEL := kernel-bin | patch-cmdline | lzma | uImageArcher lzma
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
  MTDPARTS := spi0.0:128k(u-boot)ro,1344k(kernel),13120k(rootfs),64k(product-info)ro,64k(partition-table)ro,256k(oem-config)ro,1344k(oem-vars)ro,64k(ART)ro,14464k@0x20000(firmware)
  SUPPORTED_DEVICES := tl-wr942n-v1
endef
TARGET_DEVICES += tl-wr940n-v4 tl-wr941nd-v2 tl-wr941nd-v3 tl-wr941nd-v4 tl-wr941nd-v5 tl-wr941nd-v6 tl-wr941nd-v6-cn tl-wr942n-v1
