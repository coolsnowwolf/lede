DEVICE_VARS += DAP_SIGNATURE NETGEAR_BOARD_ID NETGEAR_HW_ID NETGEAR_KERNEL_MAGIC ROOTFS_SIZE SEAMA_SIGNATURE

define Build/mkbuffaloimg
	$(STAGING_DIR_HOST)/bin/mkbuffaloimg -B $(BOARDNAME) \
		-R $$(($(subst k, * 1024,$(ROOTFS_SIZE)))) \
		-K $$(($(subst k, * 1024,$(KERNEL_SIZE)))) \
		-i $@ -o $@.new
	mv $@.new $@
endef

define Build/mkwrggimg
	$(STAGING_DIR_HOST)/bin/mkwrggimg -b \
		-i $@ -o $@.imghdr -d /dev/mtdblock/1 \
		-m $(BOARDNAME) -s $(DAP_SIGNATURE) \
		-v LEDE -B $(REVISION)
	mv $@.imghdr $@
endef

define Build/netgear-squashfs
	rm -rf $@.fs $@.squashfs
	mkdir -p $@.fs/image
	cp $@ $@.fs/image/uImage
	$(STAGING_DIR_HOST)/bin/mksquashfs-lzma \
		$@.fs $@.squashfs \
		-noappend -root-owned -be -b 65536 \
		$(if $(SOURCE_DATE_EPOCH),-fixed-time $(SOURCE_DATE_EPOCH))

	dd if=/dev/zero bs=1k count=1 >> $@.squashfs
	mkimage \
		-A mips -O linux -T filesystem -C none \
		-M $(NETGEAR_KERNEL_MAGIC) \
		-a 0xbf070000 -e 0xbf070000 \
		-n 'MIPS OpenWrt Linux-$(LINUX_VERSION)' \
		-d $@.squashfs $@
	rm -rf $@.squashfs $@.fs
endef

define Build/netgear-uImage
	$(call Build/uImage,$(1) -M $(NETGEAR_KERNEL_MAGIC))
endef

define Build/relocate-kernel
	rm -rf $@.relocate
	$(CP) ../../generic/image/relocate $@.relocate
	$(MAKE) -j1 -C $@.relocate KERNEL_ADDR=$(KERNEL_LOADADDR) CROSS_COMPILE=$(TARGET_CROSS)
	( \
		dd if=$@.relocate/loader.bin bs=32 conv=sync && \
		perl -e '@s = stat("$@"); print pack("N", @s[7])' && \
		cat "$@" \
	) > "$@.new"
	mv "$@.new" "$@"
	rm -rf $@.relocate
endef

define Build/seama
	$(STAGING_DIR_HOST)/bin/seama -i $@ $(if $(1),$(1),-m "dev=/dev/mtdblock/1" -m "type=firmware")
	mv $@.seama $@
endef

define Build/seama-seal
	$(call Build/seama,-s $@.seama $(1))
endef

define Build/uImageHiWiFi
	# Field ih_name needs to start with "tw150v1"
	mkimage -A $(LINUX_KARCH) \
		-O linux -T kernel \
		-C $(1) -a $(KERNEL_LOADADDR) -e $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) \
		-n 'tw150v1 $(call toupper,$(LINUX_KARCH)) LEDE Linux-$(LINUX_VERSION)' -d $@ $@.new
	@mv $@.new $@
endef

define Build/wrgg-pad-rootfs
	$(STAGING_DIR_HOST)/bin/padjffs2 $(IMAGE_ROOTFS) -c 64 >>$@
endef


define Device/ap121f
  DEVICE_TITLE := ALFA Network AP121F
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-storage -swconfig
  BOARDNAME := AP121F
  IMAGE_SIZE := 16064k
  CONSOLE := ttyATH0,115200
  MTDPARTS := spi0.0:192k(u-boot)ro,64k(u-boot-env),64k(art)ro,-(firmware)
  SUPPORTED_DEVICES := ap121f
  IMAGE/sysupgrade.bin = append-kernel | pad-to $$$$(BLOCKSIZE) | \
	append-rootfs | pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += ap121f

define Device/ap531b0
  DEVICE_TITLE := Rockeetech AP531B0
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := AP531B0
  IMAGE_SIZE := 16000k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,16000k(firmware),64k(art)ro
endef
TARGET_DEVICES += ap531b0

define Device/ap90q
  DEVICE_TITLE := YunCore AP90Q
  BOARDNAME := AP90Q
  IMAGE_SIZE := 16000k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env),16000k(firmware),64k(art)ro
endef
TARGET_DEVICES += ap90q

define Device/arduino-yun
  DEVICE_TITLE := Arduino Yun
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := Yun
  IMAGE_SIZE := 15936k
  CONSOLE = ttyATH0,250000
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env),15936k(firmware),64k(nvram),64k(art)ro
endef
TARGET_DEVICES += arduino-yun

define Device/bsb
  DEVICE_TITLE := Smart Electronics Black Swift board
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := BSB
  IMAGE_SIZE := 16000k
  CONSOLE := ttyATH0,115200
  MTDPARTS := spi0.0:128k(u-boot)ro,64k(u-boot-env)ro,16128k(firmware),64k(art)ro
endef
TARGET_DEVICES += bsb

define Device/carambola2
  DEVICE_TITLE := 8devices Carambola2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := CARAMBOLA2
  IMAGE_SIZE := 16000k
  CONSOLE := ttyATH0,115200
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,16000k(firmware),64k(art)ro
endef
TARGET_DEVICES += carambola2

define Device/cf-e316n-v2
  DEVICE_TITLE := COMFAST CF-E316N v2
  BOARDNAME := CF-E316N-V2
  IMAGE_SIZE := 16192k
  MTDPARTS := spi0.0:64k(u-boot)ro,64k(art)ro,16192k(firmware),64k(art-backup)ro
endef
TARGET_DEVICES += cf-e316n-v2

define Device/cf-e320n-v2
  $(Device/cf-e316n-v2)
  DEVICE_TITLE := COMFAST CF-E320N v2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := CF-E320N-V2
endef
TARGET_DEVICES += cf-e320n-v2

define Device/cf-e355ac
  DEVICE_TITLE := COMFAST CF-E355AC
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := CF-E355AC
  IMAGE_SIZE := 16192k
  MTDPARTS := spi0.0:64k(u-boot)ro,64k(art)ro,16192k(firmware),64k(art-backup)ro
endef
TARGET_DEVICES += cf-e355ac

define Device/cf-e380ac-v1
  DEVICE_TITLE := COMFAST CF-E380AC v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := CF-E380AC-V1
  IMAGE_SIZE := 16128k
  MTDPARTS := spi0.0:128k(u-boot)ro,64k(art)ro,16128k(firmware),64k(art-backup)ro
endef
TARGET_DEVICES += cf-e380ac-v1

define Device/cf-e380ac-v2
  $(Device/cf-e380ac-v1)
  DEVICE_TITLE := COMFAST CF-E380AC v2
  BOARDNAME := CF-E380AC-V2
  IMAGE_SIZE := 16000k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(art)ro,16000k(firmware),64k(art-backup)ro
endef
TARGET_DEVICES += cf-e380ac-v2

define Device/cf-e520n
  DEVICE_TITLE := COMFAST CF-E520N
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := CF-E520N
  IMAGE_SIZE := 8000k
  MTDPARTS := spi0.0:64k(u-boot)ro,64k(art)ro,8000k(firmware),64k(art-backup)ro
endef
TARGET_DEVICES += cf-e520n

define Device/cf-e530n
  $(Device/cf-e520n)
  DEVICE_TITLE := COMFAST CF-E530N
  BOARDNAME := CF-E530N
endef
TARGET_DEVICES += cf-e530n

define Device/cpe505n
  DEVICE_TITLE := P&W CPE505N
  BOARDNAME := CPE505N
  IMAGE_SIZE := 16000k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,16000k(firmware),64k(art)ro
endef
TARGET_DEVICES += cpe505n

define Device/cpe830
  $(Device/ap90q)
  DEVICE_TITLE := YunCore CPE830
  DEVICE_PACKAGES := rssileds
  BOARDNAME := CPE830
endef
TARGET_DEVICES += cpe830

define Device/cpe870
  DEVICE_TITLE := YunCore CPE870
  DEVICE_PACKAGES := rssileds
  BOARDNAME := CPE870
  IMAGE_SIZE := 7936k
  MTDPARTS := spi0.0:64k(u-boot)ro,64k(u-boot-env),7936k(firmware),64k(config)ro,64k(art)ro
endef
TARGET_DEVICES += cpe870

define Device/dragino2
  BOARDNAME := DRAGINO2
  CONSOLE := ttyATH0,115200
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  DEVICE_TITLE := Dragino 2 (MS14)
  IMAGE_SIZE := 16000k
  MTDPARTS := spi0.0:256k(u-boot)ro,16000k(firmware),64k(config)ro,64k(art)ro
endef
TARGET_DEVICES += dragino2

define Device/ew-dorin
  DEVICE_TITLE := Embedded Wireless Dorin Platform
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-chipidea 
  BOARDNAME = EW-DORIN
  CONSOLE := ttyATH0,115200
  IMAGE_SIZE = 16000k
  MTDPARTS = spi0.0:256k(u-boot)ro,64k(u-boot-env),16000k(firmware),64k(art)ro
endef
TARGET_DEVICES += ew-dorin

define Device/ew-dorin-router
  DEVICE_TITLE := Embedded Wireless Dorin Router Platform
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-chipidea 
  BOARDNAME = EW-DORIN-ROUTER
  CONSOLE := ttyATH0,115200
  IMAGE_SIZE = 16000k
  MTDPARTS = spi0.0:256k(u-boot)ro,64k(u-boot-env),16000k(firmware),64k(art)ro
endef
TARGET_DEVICES += ew-dorin-router

define Device/weio
  DEVICE_TITLE := WeIO
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := WEIO
  IMAGE_SIZE := 16000k
  CONSOLE := ttyATH0,115200
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,16000k(firmware),64k(art)ro
endef
TARGET_DEVICES += weio

define Device/gl-ar150
  DEVICE_TITLE := GL.iNet GL-AR150
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := GL-AR150
  IMAGE_SIZE := 16000k
  CONSOLE := ttyATH0,115200
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,16000k(firmware),64k(art)ro
endef
TARGET_DEVICES += gl-ar150

define Device/gl-ar300
  DEVICE_TITLE := GL.iNet GL-AR300
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := GL-AR300
  IMAGE_SIZE := 16000k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,16000k(firmware),64k(art)ro
endef
TARGET_DEVICES += gl-ar300

define Device/gl-ar300m
  DEVICE_TITLE := GL.iNet GL-AR300M
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := GL-AR300M
  IMAGE_SIZE := 16000k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env),16000k(firmware),64k(art)ro
endef
TARGET_DEVICES += gl-ar300m

define Device/gl-domino
  DEVICE_TITLE := GL.iNet Domino Pi
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := DOMINO
  IMAGE_SIZE := 16000k
  CONSOLE := ttyATH0,115200
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,16000k(firmware),64k(art)ro
endef
TARGET_DEVICES += gl-domino

define Device/gl-mifi
  DEVICE_TITLE := GL.iNet GL-MiFi
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := GL-MIFI
  IMAGE_SIZE := 16000k
  CONSOLE := ttyATH0,115200
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,16000k(firmware),64k(art)ro
endef
TARGET_DEVICES += gl-mifi

define Device/gl-usb150
  DEVICE_TITLE := GL.iNet GL-USB150
  DEVICE_PACKAGES := -swconfig
  BOARDNAME := GL-USB150
  IMAGE_SIZE := 16000k
  CONSOLE := ttyATH0,115200
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,16000k(firmware),64k(art)ro
  SUPPORTED_DEVICES := gl-usb150
  IMAGE/sysupgrade.bin = append-kernel | pad-to $$$$(BLOCKSIZE) | \
	append-rootfs | pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += gl-usb150

define Device/lima
  DEVICE_TITLE := 8devices Lima
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := LIMA
  IMAGE_SIZE := 15616k
  MTDPARTS := spi0.0:256k(u-boot)ro,256k(u-boot-env)ro,256k(art)ro,-(firmware)
endef
TARGET_DEVICES += lima

define Device/mr12
  DEVICE_TITLE := Meraki MR12
  DEVICE_PACKAGES := kmod-spi-gpio
  BOARDNAME := MR12
  ROOTFS_SIZE := 13440k
  IMAGE_SIZE := 15680k
  MTDPARTS := spi0.0:256k(u-boot)ro,256k(u-boot-env)ro,13440k(rootfs),2240k(kernel),64k(mac),128k(art)ro,15680k@0x80000(firmware)
  IMAGE/kernel.bin := append-kernel
  IMAGE/rootfs.bin := append-rootfs | pad-rootfs
  IMAGE/sysupgrade.bin := append-rootfs | pad-rootfs | pad-to $$$$(ROOTFS_SIZE) | append-kernel | check-size $$$$(IMAGE_SIZE)
  IMAGES := kernel.bin rootfs.bin sysupgrade.bin
endef

define Device/mr16
  $(Device/mr12)
  DEVICE_TITLE := Meraki MR16
  BOARDNAME := MR16
endef
TARGET_DEVICES += mr12 mr16

define Device/dr344
  DEVICE_TITLE := Wallys DR344
  BOARDNAME := DR344
  KERNEL_SIZE := 1408k
  ROOTFS_SIZE := 6336k
  IMAGE_SIZE := 7744k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,6336k(rootfs),1408k(kernel),64k(nvram),64k(art)ro,7744k@0x50000(firmware)
  IMAGE/sysupgrade.bin := append-rootfs | pad-rootfs | pad-to $$$$(ROOTFS_SIZE) | append-kernel | check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += dr344

define Device/dr531
  DEVICE_TITLE := Wallys DR531
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := DR531
  IMAGE_SIZE := 7808k
  MTDPARTS := spi0.0:192k(u-boot)ro,64k(u-boot-env),64k(partition-table)ro,7808k(firmware),64k(art)ro
endef
TARGET_DEVICES += dr531

define Device/wndr3700
  DEVICE_TITLE := NETGEAR WNDR3700
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport kmod-leds-wndr3700-usb
  BOARDNAME := WNDR3700
  NETGEAR_KERNEL_MAGIC := 0x33373030
  NETGEAR_BOARD_ID := WNDR3700
  IMAGE_SIZE := 7680k
  MTDPARTS := spi0.0:320k(u-boot)ro,128k(u-boot-env)ro,7680k(firmware),64k(art)ro
  IMAGES := sysupgrade.bin factory.img factory-NA.img
  KERNEL := kernel-bin | patch-cmdline | lzma -d20 | netgear-uImage lzma
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | netgear-squashfs | append-rootfs | pad-rootfs
  IMAGE/sysupgrade.bin := $$(IMAGE/default) | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.img := $$(IMAGE/default) | netgear-dni | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory-NA.img := $$(IMAGE/default) | netgear-dni NA | check-size $$$$(IMAGE_SIZE)
endef

define Device/wndr3700v2
  $(Device/wndr3700)
  DEVICE_TITLE := NETGEAR WNDR3700 v2
  NETGEAR_BOARD_ID := WNDR3700v2
  NETGEAR_KERNEL_MAGIC := 0x33373031
  NETGEAR_HW_ID := 29763654+16+64
  IMAGE_SIZE := 15872k
  MTDPARTS := spi0.0:320k(u-boot)ro,128k(u-boot-env)ro,15872k(firmware),64k(art)ro
  IMAGES := sysupgrade.bin factory.img
endef

define Device/wndr3800
  $(Device/wndr3700v2)
  DEVICE_TITLE := NETGEAR WNDR3800
  NETGEAR_BOARD_ID := WNDR3800
  NETGEAR_HW_ID := 29763654+16+128
endef

define Device/wndr3800ch
  $(Device/wndr3800)
  DEVICE_TITLE := NETGEAR WNDR3800 (Ch)
  NETGEAR_BOARD_ID := WNDR3800CH
endef

define Device/wndrmac
  $(Device/wndr3700v2)
  DEVICE_TITLE := NETGEAR WNDRMAC
  NETGEAR_BOARD_ID := WNDRMAC
endef

define Device/wndrmacv2
  $(Device/wndr3800)
  DEVICE_TITLE := NETGEAR WNDRMAC v2
  NETGEAR_BOARD_ID := WNDRMACv2
endef
TARGET_DEVICES += wndr3700 wndr3700v2 wndr3800 wndr3800ch wndrmac wndrmacv2

define Device/cap324
  DEVICE_TITLE := PowerCloud CAP324 Cloud AP
  BOARDNAME := CAP324
  DEVICE_PROFILE := CAP324
  IMAGE_SIZE := 15296k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,15296k(firmware),640k(certs),64k(nvram),64k(art)ro
endef
TARGET_DEVICES += cap324

define Device/cap324-nocloud
  DEVICE_TITLE := PowerCloud CAP324 Cloud AP (No-Cloud)
  BOARDNAME := CAP324
  DEVICE_PROFILE := CAP324
  IMAGE_SIZE := 16000k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,16000k(firmware),64k(art)ro
endef
TARGET_DEVICES += cap324-nocloud

define Device/cr3000
  DEVICE_TITLE := PowerCloud CR3000 Cloud Router
  BOARDNAME := CR3000
  DEVICE_PROFILE := CR3000
  IMAGE_SIZE := 7104k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,7104k(firmware),640k(certs),64k(nvram),64k(art)ro
endef
TARGET_DEVICES += cr3000

define Device/cr3000-nocloud
  DEVICE_TITLE := PowerCloud CR3000 (No-Cloud)
  BOARDNAME := CR3000
  DEVICE_PROFILE := CR3000
  IMAGE_SIZE := 7808k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,7808k(firmware),64k(art)ro
endef
TARGET_DEVICES += cr3000-nocloud

define Device/cr5000
  DEVICE_TITLE := PowerCloud CR5000 Cloud Router
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport kmod-usb-core
  BOARDNAME := CR5000
  DEVICE_PROFILE := CR5000
  IMAGE_SIZE := 7104k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,7104k(firmware),640k(certs),64k(nvram),64k(art)ro
endef
TARGET_DEVICES += cr5000

define Device/cr5000-nocloud
  DEVICE_TITLE := PowerCloud CR5000 (No-Cloud)
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport kmod-usb-core
  BOARDNAME := CR5000
  DEVICE_PROFILE := CR5000
  IMAGE_SIZE := 7808k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,7808k(firmware),64k(art)ro
endef
TARGET_DEVICES += cr5000-nocloud

define Device/pqi-air-pen
  DEVICE_TITLE := PQI Air Pen
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-storage
  BOARDNAME := PQI-AIR-PEN
  IMAGE_SIZE := 7744k
  CONSOLE := ttyATH0,115200
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,64k(art)ro,64k(NVRAM)ro,7680k(firmware),64k(CONF)
endef
TARGET_DEVICES += pqi-air-pen

define Device/antminer-s1
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := Antminer-S1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-crypto-manager kmod-i2c-gpio-custom kmod-usb-hid
  BOARDNAME := ANTMINER-S1
  DEVICE_PROFILE := ANTMINERS1
  TPLINK_HWID := 0x04440101
  CONSOLE := ttyATH0,115200
endef

define Device/antminer-s3
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := Antminer-S3
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-crypto-manager kmod-i2c-gpio-custom kmod-usb-hid
  BOARDNAME := ANTMINER-S3
  DEVICE_PROFILE := ANTMINERS3
  TPLINK_HWID := 0x04440301
  CONSOLE := ttyATH0,115200
endef

define Device/antrouter-r1
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := Antrouter-R1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := ANTROUTER-R1
  DEVICE_PROFILE := ANTROUTERR1
  TPLINK_HWID := 0x44440101
  CONSOLE := ttyATH0,115200
endef

define Device/el-m150
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := EasyLink EL-M150
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := EL-M150
  DEVICE_PROFILE := ELM150
  TPLINK_HWID := 0x01500101
  CONSOLE := ttyATH0,115200
endef

define Device/el-mini
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := EasyLink EL-MINI
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := EL-MINI
  DEVICE_PROFILE := ELMINI
  TPLINK_HWID := 0x01530001
  CONSOLE := ttyATH0,115200
endef
TARGET_DEVICES += antminer-s1 antminer-s3 antrouter-r1 el-m150 el-mini

define Device/gl-inet-6408A-v1
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := GL.iNet 6408
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := GL-INET
  DEVICE_PROFILE := GLINET
  TPLINK_HWID := 0x08000001
  CONSOLE := ttyATH0,115200
endef

define Device/gl-inet-6416A-v1
  $(Device/tplink-16mlzma)
  DEVICE_TITLE := GL.iNet 6416
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := GL-INET
  DEVICE_PROFILE := GLINET
  TPLINK_HWID := 0x08000001
  CONSOLE := ttyATH0,115200
endef
TARGET_DEVICES += gl-inet-6408A-v1 gl-inet-6416A-v1

define Device/jwap230
  DEVICE_TITLE := jjPlus JWAP230
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := JWAP230
  IMAGE_SIZE := 16000k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env),16000k(firmware),64k(art)ro
endef
TARGET_DEVICES += jwap230

define Device/r602n
  DEVICE_TITLE := P&W R602N
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := R602N
  IMAGE_SIZE := 16000k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,16000k(firmware),64k(art)ro
endef
TARGET_DEVICES += r602n

define Device/rnx-n360rt
  $(Device/tplink-4m)
  DEVICE_TITLE := Rosewill RNX-N360RT
  BOARDNAME := TL-WR941ND
  DEVICE_PROFILE := RNXN360RT
  TPLINK_HWID := 0x09410002
  TPLINK_HWREV := 0x00420001
endef
TARGET_DEVICES += rnx-n360rt

define Device/mc-mac1200r
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := Mercury MAC1200R
  DEVICE_PACKAGES := kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := MC-MAC1200R
  DEVICE_PROFILE := MAC1200R
  TPLINK_HWID := 0x12000001
endef
TARGET_DEVICES += mc-mac1200r

define Device/minibox-v1
  $(Device/tplink-16mlzma)
  DEVICE_TITLE := Gainstrong MiniBox V1.0
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2  kmod-usb-ledtrig-usbport
  BOARDNAME := MINIBOX-V1
  DEVICE_PROFILE := MINIBOXV1
  TPLINK_HWID := 0x3C000201
  CONSOLE := ttyATH0,115200
endef
TARGET_DEVICES += minibox-v1

define Device/omy-g1
  $(Device/tplink-16mlzma)
  DEVICE_TITLE := OMYlink OMY-G1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := OMY-G1
  DEVICE_PROFILE := OMYG1
  TPLINK_HWID := 0x06660101
endef

define Device/omy-x1
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := OMYlink OMY-X1
  BOARDNAME := OMY-X1
  DEVICE_PROFILE := OMYX1
  TPLINK_HWID := 0x06660201
endef
TARGET_DEVICES += omy-g1 omy-x1

define Device/onion-omega
  $(Device/tplink-16mlzma)
  DEVICE_TITLE := Onion Omega
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-storage kmod-i2c-core kmod-i2c-gpio-custom kmod-spi-bitbang kmod-spi-dev kmod-spi-gpio kmod-spi-gpio-custom kmod-usb-serial
  BOARDNAME := ONION-OMEGA
  DEVICE_PROFILE := OMEGA
  TPLINK_HWID := 0x04700001
  CONSOLE := ttyATH0,115200
endef
TARGET_DEVICES += onion-omega

define Device/sc1750
  DEVICE_TITLE := Abicom SC1750
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := SC1750
  IMAGE_SIZE := 15744k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env),15744k(firmware),128k(APConfig),128k(kplog),64k(ART)
endef
TARGET_DEVICES += sc1750

define Device/sc300m
  DEVICE_TITLE := Abicom SC300M
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := SC300M
  IMAGE_SIZE := 15744k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env),15744k(firmware),128k(APConfig),128k(kplog),64k(ART)
endef
TARGET_DEVICES += sc300m

define Device/sc450
  DEVICE_TITLE := Abicom SC450
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := SC450
  IMAGE_SIZE := 15744k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env),15744k(firmware),128k(APConfig),128k(kplog),64k(ART)
endef
TARGET_DEVICES += sc450

define Device/smart-300
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := NC-LINK SMART-300
  BOARDNAME := SMART-300
  DEVICE_PROFILE := SMART-300
  TPLINK_HWID := 0x93410001
endef
TARGET_DEVICES += smart-300

define Device/som9331
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := OpenEmbed SOM9331
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-storage kmod-i2c-core kmod-i2c-gpio-custom kmod-spi-bitbang kmod-spi-dev kmod-spi-gpio kmod-spi-gpio-custom kmod-usb-serial
  BOARDNAME := SOM9331
  DEVICE_PROFILE := SOM9331
  TPLINK_HWID := 0x04800054
  CONSOLE := ttyATH0,115200
endef
TARGET_DEVICES += som9331

define Device/sr3200
  DEVICE_TITLE := YunCore SR3200
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := SR3200
  IMAGE_SIZE := 16000k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env),16000k(firmware),64k(art)ro
endef
TARGET_DEVICES += sr3200

define Device/xd3200
  $(Device/sr3200)
  DEVICE_TITLE := YunCore XD3200
  DEVICE_PACKAGES := kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := XD3200
endef
TARGET_DEVICES += xd3200

define Device/tellstick-znet-lite
  $(Device/tplink-16mlzma)
  DEVICE_TITLE := TellStick ZNet Lite
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-acm kmod-usb-serial kmod-usb-serial-pl2303
  BOARDNAME := TELLSTICK-ZNET-LITE
  DEVICE_PROFILE := TELLSTICKZNETLITE
  TPLINK_HWID := 0x00726001
  CONSOLE := ttyATH0,115200
endef
TARGET_DEVICES += tellstick-znet-lite

define Device/oolite
  $(Device/tplink-16mlzma)
  DEVICE_TITLE := Gainstrong OOLITE
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := GS-OOLITE
  DEVICE_PROFILE := OOLITE
  TPLINK_HWID := 0x3C000101
  CONSOLE := ttyATH0,115200
endef
TARGET_DEVICES += oolite

define Device/NBG6616
  DEVICE_TITLE := ZyXEL NBG6616
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-usb-storage kmod-rtc-pcf8563 kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := NBG6616
  KERNEL_SIZE := 2048k
  IMAGE_SIZE := 15323k
  MTDPARTS := spi0.0:192k(u-boot)ro,64k(env)ro,64k(RFdata)ro,384k(zyxel_rfsd),384k(romd),64k(header),2048k(kernel),13184k(rootfs),15232k@0x120000(firmware)
  CMDLINE += mem=128M
  IMAGES := sysupgrade.bin
  KERNEL := kernel-bin | patch-cmdline | lzma | uImage lzma | jffs2 boot/vmlinux.lzma.uImage
  IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE)
  # We cannot currently build a factory image. It is the sysupgrade image
  # prefixed with a header (which is actually written into the MTD device).
  # The header is 2kiB and is filled with 0xff. The format seems to be:
  #   2 bytes:  0x0000
  #   2 bytes:  checksum of the data partition (big endian)
  #   4 bytes:  length of the contained image file (big endian)
  #  32 bytes:  Firmware Version string (NUL terminated, 0xff padded)
  #   2 bytes:  0x0000
  #   2 bytes:  checksum over the header partition (big endian)
  #  32 bytes:  Model (e.g. "NBG6616", NUL termiated, 0xff padded)
  #      rest: 0xff padding
  #
  # The checksums are calculated by adding up all bytes and if a 16bit
  # overflow occurs, one is added and the sum is masked to 16 bit:
  #   csum = csum + databyte; if (csum > 0xffff) { csum += 1; csum &= 0xffff };
  # Should the file have an odd number of bytes then the byte len-0x800 is
  # used additionally.
  # The checksum for the header is calcualted over the first 2048 bytes with
  # the firmware checksum as the placeholder during calculation.
  #
  # The header is padded with 0xff to the erase block size of the device.
endef
TARGET_DEVICES += NBG6616

define Device/c-55
  DEVICE_TITLE := AirTight Networks C-55
  DEVICE_PACKAGES := kmod-ath9k
  BOARDNAME := C-55
  KERNEL_SIZE := 2048k
  IMAGE_SIZE := 15872k
  MTDPARTS := spi0.0:256k(u-boot)ro,128k(u-boot-env)ro,2048k(kernel),13824k(rootfs),13824k(opt)ro,2624k(failsafe)ro,64k(art)ro,15872k@0x60000(firmware)
  IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += c-55

define Device/hiwifi-hc6361
  DEVICE_TITLE := HiWiFi HC6361
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-storage \
	kmod-fs-ext4 kmod-nls-iso8859-1 e2fsprogs
  BOARDNAME := HiWiFi-HC6361
  DEVICE_PROFILE := HIWIFI_HC6361
  IMAGE_SIZE := 16128k
  KERNEL := kernel-bin | patch-cmdline | lzma | uImageHiWiFi lzma
  CONSOLE := ttyATH0,115200
  MTDPARTS := spi0.0:64k(u-boot)ro,64k(bdinfo)ro,16128k(firmware),64k(backup)ro,64k(art)ro
endef
TARGET_DEVICES += hiwifi-hc6361

define Device/seama
  LOADER_TYPE := bin
  BLOCKSIZE := 64k
  KERNEL := kernel-bin | patch-cmdline | relocate-kernel | lzma
  KERNEL_INITRAMFS := kernel-bin | patch-cmdline | lzma | seama
  KERNEL_INITRAMFS_SUFFIX = $$(KERNEL_SUFFIX).seama
  IMAGES := sysupgrade.bin factory.bin

  # 64 bytes offset:
  # - 28 bytes seama_header
  # - 36 bytes of META data (4-bytes aligned)
  IMAGE/default := append-kernel | pad-offset $$$$(BLOCKSIZE) 64 | append-rootfs
  IMAGE/sysupgrade.bin := \
	$$(IMAGE/default) | seama | pad-rootfs | \
	check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := \
	$$(IMAGE/default) | seama | pad-rootfs | \
	seama-seal -m "signature=$$$$(SEAMA_SIGNATURE)" | \
	check-size $$$$(IMAGE_SIZE)
  SEAMA_SIGNATURE :=
endef

define Device/dir-869-a1
  $(Device/seama)
  DEVICE_TITLE := D-Link DIR-869 rev. A1
  DEVICE_PACKAGES := kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := DIR-869-A1
  IMAGE_SIZE := 15872k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,64k(devdata)ro,64k(devconf)ro,15872k(firmware),64k(radiocfg)ro
  SEAMA_SIGNATURE := wrgac54_dlink.2015_dir869
  IMAGE/factory.bin := \
	$$(IMAGE/default) | pad-rootfs -x 64 | \
	seama | seama-seal -m "signature=$$$$(SEAMA_SIGNATURE)" | \
	check-size $$$$(IMAGE_SIZE)
endef

define Device/mynet-n600
  $(Device/seama)
  DEVICE_TITLE := Western Digital My Net N600
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := MYNET-N600
  IMAGE_SIZE := 15808k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,64k(devdata)ro,64k(devconf)ro,15872k(firmware),64k(radiocfg)ro
  SEAMA_SIGNATURE := wrgnd16_wd_db600
endef

define Device/mynet-n750
  $(Device/seama)
  DEVICE_TITLE := Western Digital My Net N750
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := MYNET-N750
  IMAGE_SIZE := 15808k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,64k(devdata)ro,64k(devconf)ro,15872k(firmware),64k(radiocfg)ro
  SEAMA_SIGNATURE := wrgnd13_wd_av
endef

define Device/qihoo-c301
  $(Device/seama)
  DEVICE_TITLE := Qihoo C301
  DEVICE_PACKAGES :=  kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := QIHOO-C301
  IMAGE_SIZE := 15744k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env),64k(devdata),64k(devconf),15744k(firmware),64k(warm_start),64k(action_image_config),64k(radiocfg)ro;spi0.1:15360k(upgrade2),1024k(privatedata)
  SEAMA_SIGNATURE := wrgac26_qihoo360_360rg
endef
TARGET_DEVICES += dir-869-a1 mynet-n600 mynet-n750 qihoo-c301

define Device/dap-2695-a1
  DEVICE_TITLE := D-Link DAP-2695 rev. A1
  DEVICE_PACKAGES := ath10k-firmware-qca988x kmod-ath10k
  BOARDNAME := DAP-2695-A1
  IMAGES := factory.img sysupgrade.bin
  IMAGE_SIZE := 15360k
  IMAGE/factory.img := append-kernel | pad-offset 65536 160 | append-rootfs | wrgg-pad-rootfs | mkwrggimg | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := append-kernel | pad-offset 65536 160 | mkwrggimg | append-rootfs | wrgg-pad-rootfs | check-size $$$$(IMAGE_SIZE)
  KERNEL := kernel-bin | patch-cmdline | relocate-kernel | lzma
  KERNEL_INITRAMFS := $$(KERNEL) | mkwrggimg
  MTDPARTS := spi0.0:256k(bootloader)ro,64k(bdcfg)ro,64k(rgdb)ro,64k(langpack)ro,15360k(firmware),448k(captival)ro,64k(certificate)ro,64k(radiocfg)ro
  DAP_SIGNATURE := wapac02_dkbs_dap2695
endef
TARGET_DEVICES += dap-2695-a1

define Device/bhr-4grv2
  DEVICE_TITLE := Buffalo BHR-4GRV2
  BOARDNAME := BHR-4GRV2
  ROOTFS_SIZE := 14528k
  KERNEL_SIZE := 1472k
  IMAGE_SIZE := 16000k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,14528k(rootfs),1472k(kernel),64k(art)ro,16000k@0x50000(firmware)
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/sysupgrade.bin := append-rootfs | pad-rootfs | pad-to $$$$(ROOTFS_SIZE) | append-kernel | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | mkbuffaloimg
endef
TARGET_DEVICES += bhr-4grv2

define Device/wpj-16m
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  MTDPARTS := spi0.0:192k(u-boot)ro,16128k(firmware),64k(art)ro
  IMAGE_SIZE := 16128k
endef

define Device/wpj342
  $(Device/wpj-16m)
  DEVICE_TITLE := Compex WPJ342 (16MB flash)
  BOARDNAME := WPJ342
endef

define Device/wpj344
  $(Device/wpj-16m)
  DEVICE_TITLE := Compex WPJ344 (16MB flash)
  BOARDNAME := WPJ344
  SUPPORTED_DEVICES := wpj344
  IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(BLOCKSIZE) | \
	append-rootfs | pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
endef

define Device/wpj531
  $(Device/wpj-16m)
  DEVICE_TITLE := Compex WPJ531 (16MB flash)
  BOARDNAME := WPJ531
endef

define Device/wpj558
  $(Device/wpj-16m)
  DEVICE_TITLE := Compex WPJ558 (16MB flash)
  BOARDNAME := WPJ558
  SUPPORTED_DEVICES := wpj558
  IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(BLOCKSIZE) | \
	append-rootfs | pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
endef

define Device/wpj563
  $(Device/wpj-16m)
  DEVICE_TITLE := Compex WPJ563 (16MB flash)
  BOARDNAME := WPJ563
endef
TARGET_DEVICES += wpj342 wpj344 wpj531 wpj558 wpj563

define Device/wrtnode2q
  DEVICE_TITLE := WRTnode2Q
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-storage
  BOARDNAME := WRTNODE2Q
  IMAGE_SIZE := 16064k
  MTDPARTS := spi0.0:192k(u-boot)ro,64k(u-boot-env),64k(art)ro,16064k(firmware),16384k@0x0(fullflash)
endef
TARGET_DEVICES += wrtnode2q

define Device/zbt-we1526
  DEVICE_TITLE := Zbtlink ZBT-WE1526
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := ZBT-WE1526
  IMAGE_SIZE := 16000k
  KERNEL_SIZE := 1472k
  ROOTFS_SIZE := 14528k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,14528k(rootfs),1472k(kernel),64k(art)ro,16000k@0x50000(firmware)
  IMAGE/sysupgrade.bin := append-rootfs | pad-rootfs | pad-to $$$$(ROOTFS_SIZE) | append-kernel | check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += zbt-we1526

define Device/fritz300e
  DEVICE_TITLE := AVM FRITZ!WLAN Repeater 300E
  DEVICE_PACKAGES := fritz-tffs rssileds -swconfig -uboot-envtools
  BOARDNAME := FRITZ300E
  SUPPORTED_DEVICES := fritz300e
  IMAGE_SIZE := 15232k
  KERNEL := kernel-bin | patch-cmdline | lzma | eva-image
  IMAGE/sysupgrade.bin := append-kernel | pad-to 64k | \
	append-squashfs-fakeroot-be | pad-to 256 | \
	append-rootfs | pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += fritz300e
