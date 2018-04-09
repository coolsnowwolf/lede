#
# RT305X Profiles
#
define Build/buffalo-tftp-header
  ( \
    echo -n -e "# Airstation FirmWare\nrun u_fw\nreset\n\n" | \
      dd bs=512 count=1 conv=sync; \
    dd if=$@; \
  ) > $@.tmp && \
  $(STAGING_DIR_HOST)/bin/buffalo-tftp -i $@.tmp -o $@.new
  mv $@.new $@
endef

define Build/dap-header
	$(STAGING_DIR_HOST)/bin/mkdapimg $(1) -i $@ -o $@.new
	mv $@.new $@
endef

define Build/hilink-header
	$(STAGING_DIR_HOST)/bin/mkhilinkfw -e -i $@ -o $@.new
	mv $@.new $@
endef

define Build/jcg-header
	$(STAGING_DIR_HOST)/bin/jcgimage -v $(1) -u $@ -o $@.new
	mv $@.new $@
endef


define Device/3g150b
  DTS := 3G150B
  BLOCKSIZE := 4k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  UIMAGE_NAME:= Linux Kernel Image
  DEVICE_TITLE := Tenda 3G150B
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-dwc2 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += 3g150b

define Device/3g300m
  DTS := 3G300M
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  UIMAGE_NAME := 3G150M_SPI Kernel Image
  DEVICE_TITLE := Tenda 3G300M
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-dwc2 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += 3g300m

define Device/3g-6200n
  DTS := 3G-6200N
  IMAGE_SIZE := 3648k
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | \
	edimax-header -s CSYS -m 3G62 -f 0x50000 -S 0x01100000 | pad-rootfs | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_TITLE := Edimax 3g-6200n
endef
TARGET_DEVICES += 3g-6200n

define Device/3g-6200nl
  DTS := 3G-6200NL
  IMAGE_SIZE := 3648k
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | \
	edimax-header -s CSYS -m 3G62 -f 0x50000 -S 0x01100000 | pad-rootfs | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_TITLE := Edimax 3g-6200nl
endef
TARGET_DEVICES += 3g-6200nl

define Device/a5-v11
  DTS := A5-V11
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | poray-header -B A5-V11 -F 4M
  DEVICE_TITLE := A5-V11
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2
endef
TARGET_DEVICES += a5-v11

define Device/air3gii
  DTS := AIR3GII
  BLOCKSIZE := 64k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := AirLive Air3GII
endef
TARGET_DEVICES += air3gii

define Device/all0256n-4M
  DTS := ALL0256N-4M
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := Allnet ALL0256N (4MB)
  DEVICE_PACKAGES := rssileds
endef
TARGET_DEVICES += all0256n-4M

define Device/all0256n-8M
  DTS := ALL0256N-8M
  DEVICE_TITLE := Allnet ALL0256N (8MB)
  DEVICE_PACKAGES := rssileds
endef
TARGET_DEVICES += all0256n-8M

define Device/all5002
  DTS := ALL5002
  IMAGE_SIZE := 32448k
  DEVICE_TITLE := Allnet ALL5002
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport \
          kmod-i2c-core kmod-i2c-gpio kmod-hwmon-lm92 kmod-gpio-pcf857x
endef
TARGET_DEVICES += all5002

define Device/all5003
  DTS := ALL5003
  IMAGE_SIZE := 32448k
  DEVICE_TITLE := Allnet ALL5003
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport \
          kmod-i2c-core kmod-i2c-gpio kmod-hwmon-lm92 kmod-gpio-pcf857x
endef
TARGET_DEVICES += all5003

define Device/asl26555-8M
  DTS := ASL26555-8M
  IMAGE_SIZE := 7744k
  SUPPORTED_DEVICES += asl26555
  DEVICE_TITLE := Alpha ASL26555 
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-dwc2 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += asl26555-8M

define Device/asl26555-16M
  DTS := ASL26555-16M
  IMAGE_SIZE := 15872k
  SUPPORTED_DEVICES += asl26555
  DEVICE_TITLE := Alpha ASL26555 16M
  DEVICE_PACKAGES :=
endef
TARGET_DEVICES += asl26555-16M

define Device/atp-52b
  DTS := ATP-52B
  IMAGE_SIZE := 7808k
  DEVICE_TITLE := Argus ATP-52B
endef
TARGET_DEVICES += atp-52b

define Device/awm002-evb-4M
  DTS := AWM002-EVB-4M
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := AsiaRF AWM002-EVB (4M)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 \
		kmod-i2c-core kmod-i2c-gpio
endef
TARGET_DEVICES += awm002-evb-4M

define Device/awm002-evb-8M
  DTS := AWM002-EVB-8M
  DEVICE_TITLE := AsiaRF AWM002-EVB (8M)/AsiaRF AWM003 EVB
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 \
		kmod-i2c-core kmod-i2c-gpio
endef
TARGET_DEVICES += awm002-evb-8M

define Device/awapn2403
  DTS := AWAPN2403
  BLOCKSIZE := 4k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := AsiaRF AWAPN2403
endef
TARGET_DEVICES += awapn2403

define Device/bc2
  DTS := BC2
  DEVICE_TITLE := NexAira BC2
endef
TARGET_DEVICES += bc2

define Device/broadway
  DTS := BROADWAY
  IMAGE_SIZE := 7744k
  UIMAGE_NAME:= Broadway Kernel Image
  DEVICE_TITLE := Hauppauge Broadway
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-dwc2 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += broadway

define Device/carambola
  DTS := CARAMBOLA
  DEVICE_TITLE := 8devices Carambola
  DEVICE_PACKAGES :=
endef
TARGET_DEVICES += carambola

define Device/d105
  DTS := D105
  BLOCKSIZE := 64k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := Huawei D105
endef
TARGET_DEVICES += d105

define Device/dap-1350
  DTS := DAP-1350
  IMAGES += factory.bin factory-NA.bin
  IMAGE_SIZE := 7488k
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	dap-header -s RT3052-AP-DAP1350WW-3
  IMAGE/factory-NA.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	dap-header -s RT3052-AP-DAP1350-3
  DEVICE_TITLE := D-Link DAP-1350
endef
TARGET_DEVICES += dap-1350

define Device/dir-300-b1
  DTS := DIR-300-B1
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | wrg-header wrgn23_dlwbr_dir300b
  DEVICE_TITLE := D-Link DIR-300 B1
endef
TARGET_DEVICES += dir-300-b1

define Device/dir-300-b7
  DTS := DIR-300-B7
  BLOCKSIZE := 4k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := D-Link DIR-300 B7
endef
TARGET_DEVICES += dir-300-b7

define Device/dir-320-b1
  DTS := DIR-320-B1
  DEVICE_TITLE := D-Link DIR-320 B1
endef
TARGET_DEVICES += dir-320-b1

define Device/dir-600-b1
  DTS := DIR-600-B1
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  SUPPORTED_DEVICES := dir-600-b1 dir-600-b2
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | wrg-header wrgn23_dlwbr_dir600b
  DEVICE_TITLE := D-Link DIR-600 B1/B2
endef
TARGET_DEVICES += dir-600-b1

define Device/dir-610-a1
  DTS := DIR-610-A1
  BLOCKSIZE := 4k
  IMAGES += factory.bin
  KERNEL := $(KERNEL_DTB)
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  IMAGE/sysupgrade.bin := \
	append-kernel | pad-offset $$$$(BLOCKSIZE) 64 | append-rootfs | \
	seama -m "dev=/dev/mtdblock/2" -m "type=firmware" | \
	pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := \
	append-kernel | pad-offset $$$$(BLOCKSIZE) 64 | \
	append-rootfs | pad-rootfs -x 64 | \
	seama -m "dev=/dev/mtdblock/2" -m "type=firmware" | \
	seama-seal -m "signature=wrgn59_dlob.hans_dir610" | \
	check-size $$$$(IMAGE_SIZE)
  DEVICE_TITLE := D-Link DIR-610 A1 
  DEVICE_PACKAGES := kmod-ledtrig-netdev kmod-ledtrig-timer
endef
TARGET_DEVICES += dir-610-a1

define Device/dir-615-d
  DTS := DIR-615-D
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | wrg-header wrgn23_dlwbr_dir615d
  DEVICE_TITLE := D-Link DIR-615 D
endef
TARGET_DEVICES += dir-615-d


define Device/dir-615-h1
  DTS := DIR-615-H1
  BLOCKSIZE := 4k
  IMAGES += factory.bin
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | senao-header -r 0x218 -p 0x30 -t 3
  DEVICE_TITLE := D-Link DIR-615 H1
endef
TARGET_DEVICES += dir-615-h1

define Device/dir-620-a1
  DTS := DIR-620-A1
  DEVICE_TITLE := D-Link DIR-620 A1
endef
TARGET_DEVICES += dir-620-a1

define Device/dir-620-d1
  DTS := DIR-620-D1
  DEVICE_TITLE := D-Link DIR-620 D1
endef
TARGET_DEVICES += dir-620-d1

define Device/dwr-512-b
  DTS := DWR-512-B
  IMAGE_SIZE := 7800k
  DEVICE_TITLE := D-Link DWR-512 B
  DEVICE_PACKAGES := jboot-tools kmod-usb2 kmod-spi-dev kmod-usb-serial \
			kmod-usb-serial-option kmod-usb-net kmod-usb-net-cdc-ether \
			comgt-ncm
  DLINK_ROM_ID := DLK6E2412001
  DLINK_FAMILY_MEMBER := 0x6E24
  DLINK_FIRMWARE_SIZE := 0x7E0000
  KERNEL := $(KERNEL_DTB)
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := mkdlinkfw | pad-rootfs | append-metadata
  IMAGE/factory.bin := mkdlinkfw | pad-rootfs | mkdlinkfw-factory
endef
TARGET_DEVICES += dwr-512-b

define Device/esr-9753
  DTS := ESR-9753
  BLOCKSIZE := 64k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := EnGenius ESR-9753
endef
TARGET_DEVICES += esr-9753

define Device/f5d8235-v2
  DTS := F5D8235_V2
  IMAGE_SIZE := 7744k
  DEVICE_TITLE := Belkin F5D8235 v2
endef
TARGET_DEVICES += f5d8235-v2

define Device/f7c027
  DTS := F7C027
  IMAGE_SIZE := 7616k
  DEVICE_TITLE := Belkin F7C027
  DEVICE_PACKAGES := -kmod-usb-core -kmod-usb-dwc2 -kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += f7c027

define Device/fonera20n
  DTS := FONERA20N
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | \
	edimax-header -s RSDK -m NL1T -f 0x50000 -S 0xc0000
  DEVICE_TITLE := Fonera 2.0N
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-dwc2 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += fonera20n

define Device/freestation5
  DTS := FREESTATION5
  DEVICE_TITLE := ARC Wireless FreeStation
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-rt2500-usb kmod-rt2800-usb kmod-rt2x00-usb
endef
TARGET_DEVICES += freestation5

define Device/hg255d
  DTS := HG255D
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := HuaWei HG255D
endef
TARGET_DEVICES += hg255d

define Device/hlk-rm04
  DTS := HLKRM04
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | hilink-header
  DEVICE_TITLE := Hi-Link HLK-RM04
endef
TARGET_DEVICES += hlk-rm04

define Device/ht-tm02
  DTS := HT-TM02
  DEVICE_TITLE := HooToo HT-TM02
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += ht-tm02

define Device/hw550-3g
  DTS := HW550-3G
  DEVICE_TITLE := Aztech HW550-3G
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-dwc2 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += hw550-3g

define Device/ip2202
  DTS := IP2202
  DEVICE_TITLE := Poray IP2202
endef
TARGET_DEVICES += ip2202

define Device/jhr-n805r
  DTS := JHR-N805R
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | jcg-header 29.24
  DEVICE_TITLE := JCG JHR-N805R
endef
TARGET_DEVICES += jhr-n805r

define Device/jhr-n825r
  DTS := JHR-N825R
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | jcg-header 23.24
  DEVICE_TITLE := JCG JHR-N825R
endef
TARGET_DEVICES += jhr-n825r

define Device/jhr-n926r
  DTS := JHR-N926R
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | jcg-header 25.24
  DEVICE_TITLE := JCG JHR-N926R
endef
TARGET_DEVICES += jhr-n926r

define Device/m2m
  DTS := M2M
  UIMAGE_NAME:= Linux Kernel Image
  DEVICE_TITLE := Intenso Memory 2 Move
  DEVICE_PACKAGES := kmod-ledtrig-netdev kmod-ledtrig-timer \
		kmod-usb-core kmod-usb2 kmod-usb-storage kmod-scsi-core \
		kmod-fs-ext4 kmod-fs-vfat block-mount
endef
TARGET_DEVICES += m2m

define Device/m3
  DTS := M3
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | poray-header -B M3 -F 4M
  DEVICE_TITLE := Poray M3
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-ledtrig-netdev \
	kmod-ledtrig-timer
endef
TARGET_DEVICES += m3

define Device/m4-4M
  DTS := M4-4M
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | poray-header -B M4 -F 4M
  DEVICE_TITLE := Poray M4 (4MB)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-ledtrig-netdev \
	kmod-ledtrig-timer
endef
TARGET_DEVICES += m4-4M

define Device/m4-8M
  DTS := M4-8M
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | poray-header -B M4 -F 8M
  DEVICE_TITLE := Poray M4 (8MB)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-ledtrig-netdev kmod-ledtrig-timer
endef
TARGET_DEVICES += m4-8M

define Device/miniembplug
  DTS := MINIEMBPLUG
  DEVICE_TITLE := Omnima MiniEMBPlug
endef
TARGET_DEVICES += miniembplug

define Device/miniembwifi
  DTS := MINIEMBWIFI
  DEVICE_TITLE := Omnima MiniEMBWiFi
endef
TARGET_DEVICES += miniembwifi

define Device/mofi3500-3gn
  DTS := MOFI3500-3GN
  DEVICE_TITLE := MoFi Network MOFI3500-3GN
endef
TARGET_DEVICES += mofi3500-3gn

define Device/mpr-a1
  DTS := MPRA1
  BLOCKSIZE := 4k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  UIMAGE_NAME:= Linux Kernel Image
  DEVICE_TITLE := HAME MPR-A1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-ledtrig-netdev
endef
TARGET_DEVICES += mpr-a1

define Device/mpr-a2
  DTS := MPRA2
  UIMAGE_NAME:= Linux Kernel Image
  DEVICE_TITLE := HAME MPR-A2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-ledtrig-netdev
endef
TARGET_DEVICES += mpr-a2

define Device/mr-102n
  DTS := MR-102N
  DEVICE_TITLE := AXIMCom MR-102N
endef
TARGET_DEVICES += mr-102n

define Device/mzk-dp150n
  DTS := MZK-DP150N
  BLOCKSIZE := 64k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := Planex MZK-DP150N
  DEVICE_PACKAGES := kmod-spi-dev
endef
TARGET_DEVICES += mzk-dp150n

define Device/mzk-w300nh2
  DTS := MZK-W300NH2
  IMAGE_SIZE := 3648k
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | \
	edimax-header -s CSYS -m RN52 -f 0x50000 -S 0xc0000
  DEVICE_TITLE := Planex MZK-W300NH2
endef
TARGET_DEVICES += mzk-w300nh2

define Device/mzk-wdpr
  DTS := MZK-WDPR
  DEVICE_TITLE := Planex MZK-WDPR
endef
TARGET_DEVICES += mzk-wdpr

define Device/nbg-419n
  DTS := NBG-419N
  BLOCKSIZE := 64k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := ZyXEL NBG-419N
endef
TARGET_DEVICES += nbg-419n

define Device/nbg-419n2
  DTS := NBG-419N2
  IMAGE_SIZE := $(ralink_default_fw_size_8M)
  DEVICE_TITLE := ZyXEL NBG-419N2
endef
TARGET_DEVICES += nbg-419n2

define Device/ncs601w
  DTS := NCS601W
  DEVICE_TITLE := Wansview NCS601W
  DEVICE_PACKAGES := kmod-video-core kmod-video-uvc \
		kmod-usb-core kmod-usb-ohci
endef
TARGET_DEVICES += ncs601w

define Device/nixcore-x1-8M
  DTS := NIXCORE-8M
  IMAGE_SIZE := 7872k
  SUPPORTED_DEVICES += nixcore-x1
  DEVICE_TITLE := NixcoreX1 (8M)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-i2c-core kmod-i2c-ralink kmod-spi-dev
endef
TARGET_DEVICES += nixcore-x1-8M

define Device/nixcore-x1-16M
  DTS := NIXCORE-16M
  IMAGE_SIZE := 16064k
  SUPPORTED_DEVICES += nixcore-x1
  DEVICE_TITLE := NixcoreX1 (16M)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-i2c-core kmod-i2c-ralink kmod-spi-dev
endef
TARGET_DEVICES += nixcore-x1-16M

define Device/nw718
  DTS := NW718
  IMAGE_SIZE := 3712k
  UIMAGE_NAME:= ARA1B4NCRNW718;1
  DEVICE_TITLE := Netcore NW718
endef
TARGET_DEVICES += nw718

define Device/psr-680w
  DTS := PSR-680W
  BLOCKSIZE := 64k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := Petatel PSR-680W Wireless 3G Router
endef
TARGET_DEVICES += psr-680w

define Device/pwh2004
  DTS := PWH2004
  DEVICE_TITLE := Prolink PWH2004
  DEVICE_PACKAGES :=
endef
TARGET_DEVICES += pwh2004

define Device/px-4885-4M
  DTS := PX-4885-4M
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := 7Links PX-4885 (4M)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-dwc2 kmod-usb2 kmod-usb-ohci \
	kmod-usb-ledtrig-usbport kmod-leds-gpio
endef
TARGET_DEVICES += px-4885-4M

define Device/px-4885-8M
  DTS := PX-4885-8M
  DEVICE_TITLE := 7Links PX-4885 (8M)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-dwc2 kmod-usb2 kmod-usb-ohci \
	kmod-usb-ledtrig-usbport kmod-leds-gpio
endef
TARGET_DEVICES += px-4885-8M

define Device/rt5350f-olinuxino
  DTS := RT5350F-OLINUXINO
  DEVICE_TITLE := RT5350F-OLinuXino
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 \
		kmod-i2c-core kmod-i2c-ralink \
		kmod-spi-dev
endef
TARGET_DEVICES += rt5350f-olinuxino

define Device/rt5350f-olinuxino-evb
  DTS := RT5350F-OLINUXINO-EVB
  DEVICE_TITLE := RT5350F-OLinuXino-EVB
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 \
		kmod-i2c-core kmod-i2c-ralink \
		kmod-spi-dev
endef
TARGET_DEVICES += rt5350f-olinuxino-evb

define Device/rt-g32-b1
  DTS := RT-G32-B1
  BLOCKSIZE := 4k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := Asus RT-G32 B1
endef
TARGET_DEVICES += rt-g32-b1

define Device/rt-n10-plus
  DTS := RT-N10-PLUS
  BLOCKSIZE := 64k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := Asus RT-N10+
endef
TARGET_DEVICES += rt-n10-plus

define Device/rt-n13u
  DTS := RT-N13U
  DEVICE_TITLE := Asus RT-N13U
  DEVICE_PACKAGES := kmod-leds-gpio kmod-rt2800-pci kmod-usb-dwc2
endef
TARGET_DEVICES += rt-n13u

define Device/rut5xx
  DTS := RUT5XX
  DEVICE_TITLE := Teltonika RUT5XX
  DEVICE_PACKAGES := om-watchdog
endef
TARGET_DEVICES += rut5xx

define Device/sl-r7205
  DTS := SL-R7205
  BLOCKSIZE := 64k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := Skyline SL-R7205 Wireless 3G Router
endef
TARGET_DEVICES += sl-r7205

define Device/tew-638apb-v2
  DTS := TEW-638APB-V2
  BLOCKSIZE := 64k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  IMAGE/sysupgrade.bin := $$(sysupgrade_bin) | umedia-header 0x026382 | \
        append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_TITLE := TRENDnet TEW-638APB v2
endef
TARGET_DEVICES += tew-638apb-v2

define Device/tew-714tru
  DTS := TEW-714TRU
  DEVICE_TITLE := TRENDnet TEW-714TRU
endef
TARGET_DEVICES += tew-714tru

define Device/ur-326n4g
  DTS := UR-326N4G
  BLOCKSIZE := 64k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := UPVEL UR-326N4G
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-dwc2 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += ur-326n4g

define Device/ur-336un
  DTS := UR-336UN
  DEVICE_TITLE := UPVEL UR-336UN
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-dwc2 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += ur-336un

define Device/v22rw-2x2
  DTS := V22RW-2X2
  BLOCKSIZE := 64k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := Ralink AP-RT3052-V22RW-2X2
endef
TARGET_DEVICES += v22rw-2x2

define Device/vocore-8M
  DTS := VOCORE-8M
  IMAGE_SIZE := 7872k
  SUPPORTED_DEVICES += vocore
  DEVICE_TITLE := VoCore (8M)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 \
		kmod-i2c-core kmod-i2c-ralink \
		kmod-spi-dev
endef
TARGET_DEVICES += vocore-8M

define Device/vocore-16M
  DTS := VOCORE-16M
  IMAGE_SIZE := 16064k
  SUPPORTED_DEVICES += vocore
  DEVICE_TITLE := VoCore (16M)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 \
		kmod-i2c-core kmod-i2c-ralink \
		kmod-spi-dev
endef
TARGET_DEVICES += vocore-16M

define Device/w150m
  DTS := W150M
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  UIMAGE_NAME:= W150M Kernel Image
  DEVICE_TITLE := Tenda W150M
endef
TARGET_DEVICES += w150m

define Device/w306r-v20
  DTS := W306R_V20
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  UIMAGE_NAME:= linkn Kernel Image
  DEVICE_TITLE := Tenda W306R V2.0
endef
TARGET_DEVICES += w306r-v20

define Device/w502u
  DTS := W502U
  DEVICE_TITLE := ALFA Networks W502U
endef
TARGET_DEVICES += w502u

define Device/wcr-150gn
  DTS := WCR150GN
  BLOCKSIZE := 64k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := Sparklan WCR-150GN
endef
TARGET_DEVICES += wcr-150gn

define Device/whr-g300n
  DTS := WHR-G300N
  BLOCKSIZE := 64k
  IMAGE_SIZE := 3801088
  DEVICE_TITLE := Buffalo WHR-G300N
  IMAGES += tftp.bin
  IMAGE/tftp.bin := $$(sysupgrade_bin) | \
    check-size $$$$(IMAGE_SIZE) | buffalo-tftp-header
endef
TARGET_DEVICES += whr-g300n

define Device/wizard8800
  DTS := WIZARD8800
  UIMAGE_NAME:= Linux Kernel Image
  DEVICE_TITLE := EasyAcc WIZARD 8800
endef
TARGET_DEVICES += wizard8800

define Device/wizfi630a
  DTS := WIZFI630A
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := WIZnet WizFi630A
endef
TARGET_DEVICES += wizfi630a

define Device/wl-330n
  DTS := WL-330N
  BLOCKSIZE := 4k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := Asus WL-330N
endef
TARGET_DEVICES += wl-330n

define Device/wl-330n3g
  DTS := WL-330N3G
  BLOCKSIZE := 4k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := Asus WL-330N3G
  DEVICE_PACKAGES :=
endef
TARGET_DEVICES += wl-330n3g

define Device/wl-351
  DTS := WL-351
  BLOCKSIZE := 64k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := Sitecom WL-351 v1
  DEVICE_PACKAGES := kmod-switch-rtl8366rb kmod-swconfig swconfig
endef
TARGET_DEVICES += wl-351

define Device/wnce2001
  DTS := WNCE2001
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  IMAGES += factory.bin factory-NA.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	dap-header -s RT3052-AP-WNCE2001-3 -r WW -v 1.0.0.99
  IMAGE/factory-NA.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	dap-header -s RT3052-AP-WNCE2001-3 -r NA -v 1.0.0.99
  DEVICE_TITLE := Netgear WNCE2001
endef
TARGET_DEVICES += wnce2001

define Device/wr512-3gn-4M
  DTS := WR512-3GN-4M
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := WR512-3GN (4M)
endef
TARGET_DEVICES += wr512-3gn-4M

define Device/wr512-3gn-8M
  DTS := WR512-3GN-8M
  DEVICE_TITLE := WR512-3GN (8M)
endef
TARGET_DEVICES += wr512-3gn-8M

define Device/wr6202
  DTS := WR6202
  DEVICE_TITLE := AWB WR6202
endef
TARGET_DEVICES += wr6202

define Device/wt1520-4M
  DTS := WT1520-4M
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | poray-header -B WT1520 -F 4M
  DEVICE_TITLE := Nexx WT1520 (4MB)
endef
TARGET_DEVICES += wt1520-4M

define Device/wt1520-8M
  DTS := WT1520-8M
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | poray-header -B WT1520 -F 8M
  DEVICE_TITLE := Nexx WT1520 (8MB)
endef
TARGET_DEVICES += wt1520-8M

define Device/x5
  DTS := X5
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | poray-header -B X5 -F 8M
  DEVICE_TITLE := Poray X5/X6
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-ledtrig-netdev kmod-ledtrig-timer
endef
TARGET_DEVICES += x5


define Device/x8
  DTS := X8
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | poray-header -B X8 -F 8M
  DEVICE_TITLE := Poray X8
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-ledtrig-netdev kmod-ledtrig-timer
endef
TARGET_DEVICES += x8

define Device/xdxrn502j
  DTS := XDXRN502J
  BLOCKSIZE := 64k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := XDX RN502J
endef
TARGET_DEVICES += xdxrn502j

define Device/kn
  DTS := kn
  BLOCKSIZE := 64k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := ZyXEL Keenetic
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ehci kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += kn

define Device/zorlik_zl5900v2
  DTS := ZL5900V2
  DEVICE_TITLE := Zorlik ZL5900V2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-ledtrig-netdev
endef
TARGET_DEVICES += zorlik_zl5900v2
