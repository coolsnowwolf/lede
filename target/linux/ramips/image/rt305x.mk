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

define Device/7links_px-4885-4m
  MTK_SOC := rt5350
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := 7Links
  DEVICE_MODEL := PX-4885
  DEVICE_VARIANT := 4M
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb2 kmod-usb-ohci \
	kmod-usb-ledtrig-usbport kmod-leds-gpio
  SUPPORTED_DEVICES += px-4885-4M
endef
TARGET_DEVICES += 7links_px-4885-4m

define Device/7links_px-4885-8m
  MTK_SOC := rt5350
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := 7Links
  DEVICE_MODEL := PX-4885
  DEVICE_VARIANT := 8M
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb2 kmod-usb-ohci \
	kmod-usb-ledtrig-usbport kmod-leds-gpio
  SUPPORTED_DEVICES += px-4885-8M
endef
TARGET_DEVICES += 7links_px-4885-8m

define Device/8devices_carambola
  MTK_SOC := rt3050
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := 8devices
  DEVICE_MODEL := Carambola
  DEVICE_PACKAGES :=
  SUPPORTED_DEVICES += carambola
endef
TARGET_DEVICES += 8devices_carambola

define Device/accton_wr6202
  MTK_SOC := rt3052
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Accton
  DEVICE_MODEL := WR6202
  SUPPORTED_DEVICES += wr6202
endef
TARGET_DEVICES += accton_wr6202

define Device/airlive_air3gii
  MTK_SOC := rt5350
  BLOCKSIZE := 64k
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := AirLive
  DEVICE_MODEL := Air3GII
  SUPPORTED_DEVICES += air3gii
endef
TARGET_DEVICES += airlive_air3gii

define Device/alfa-network_w502u
  MTK_SOC := rt3052
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := ALFA
  DEVICE_MODEL := Networks W502U
  SUPPORTED_DEVICES += w502u
endef
TARGET_DEVICES += alfa-network_w502u

define Device/allnet_all0256n-4m
  MTK_SOC := rt3050
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := Allnet
  DEVICE_MODEL := ALL0256N
  DEVICE_VARIANT := 4M
  DEVICE_PACKAGES := rssileds
  SUPPORTED_DEVICES += all0256n-4M
endef
TARGET_DEVICES += allnet_all0256n-4m

define Device/allnet_all0256n-8m
  MTK_SOC := rt3050
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Allnet
  DEVICE_MODEL := ALL0256N
  DEVICE_VARIANT := 8M
  DEVICE_PACKAGES := rssileds
  SUPPORTED_DEVICES += all0256n-8M
endef
TARGET_DEVICES += allnet_all0256n-8m

define Device/allnet_all5002
  MTK_SOC := rt3352
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := Allnet
  DEVICE_MODEL := ALL5002
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport \
          kmod-i2c-core kmod-i2c-gpio kmod-hwmon-lm92 kmod-gpio-pcf857x
  SUPPORTED_DEVICES += all5002
endef
TARGET_DEVICES += allnet_all5002

define Device/allnet_all5003
  MTK_SOC := rt5350
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := Allnet
  DEVICE_MODEL := ALL5003
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport \
          kmod-i2c-core kmod-i2c-gpio kmod-hwmon-lm92 kmod-gpio-pcf857x
  SUPPORTED_DEVICES += all5003
endef
TARGET_DEVICES += allnet_all5003

define Device/alphanetworks_asl26555-16m
  MTK_SOC := rt3050
  IMAGE_SIZE := 15872k
  DEVICE_VENDOR := Alpha
  DEVICE_MODEL := ASL26555
  DEVICE_VARIANT := 16M
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += asl26555 asl26555-16M
endef
TARGET_DEVICES += alphanetworks_asl26555-16m

define Device/alphanetworks_asl26555-8m
  MTK_SOC := rt3050
  IMAGE_SIZE := 7744k
  DEVICE_VENDOR := Alpha
  DEVICE_MODEL := ASL26555
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += asl26555 asl26555-8M
endef
TARGET_DEVICES += alphanetworks_asl26555-8m

define Device/arcwireless_freestation5
  MTK_SOC := rt3050
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := ARC Wireless
  DEVICE_MODEL := FreeStation
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-rt2500-usb kmod-rt2800-usb kmod-rt2x00-usb
  SUPPORTED_DEVICES += freestation5
endef
TARGET_DEVICES += arcwireless_freestation5

define Device/argus_atp-52b
  MTK_SOC := rt3052
  IMAGE_SIZE := 7808k
  DEVICE_VENDOR := Argus
  DEVICE_MODEL := ATP-52B
  SUPPORTED_DEVICES += atp-52b
endef
TARGET_DEVICES += argus_atp-52b

define Device/asiarf_awapn2403
  MTK_SOC := rt3052
  BLOCKSIZE := 4k
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := AsiaRF
  DEVICE_MODEL := AWAPN2403
  SUPPORTED_DEVICES += awapn2403
endef
TARGET_DEVICES += asiarf_awapn2403

define Device/asiarf_awm002-evb-4m
  MTK_SOC := rt5350
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := AsiaRF
  DEVICE_MODEL := AWM002-EVB
  DEVICE_VARIANT := 4M
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 \
		kmod-i2c-core kmod-i2c-gpio
  SUPPORTED_DEVICES += awm002-evb-4M
endef
TARGET_DEVICES += asiarf_awm002-evb-4m

define Device/asiarf_awm002-evb-8m
  MTK_SOC := rt5350
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := AsiaRF
  DEVICE_MODEL := AWM002-EVB/AWM003-EVB
  DEVICE_VARIANT := 8M
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 \
		kmod-i2c-core kmod-i2c-gpio
  SUPPORTED_DEVICES += awm002-evb-8M
endef
TARGET_DEVICES += asiarf_awm002-evb-8m

define Device/asus_rt-g32-b1
  MTK_SOC := rt3050
  BLOCKSIZE := 4k
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := Asus
  DEVICE_MODEL := RT-G32
  DEVICE_VARIANT := B1
  SUPPORTED_DEVICES += rt-g32-b1
endef
TARGET_DEVICES += asus_rt-g32-b1

define Device/asus_rt-n10-plus
  MTK_SOC := rt3050
  BLOCKSIZE := 64k
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := Asus
  DEVICE_MODEL := RT-N10+
  SUPPORTED_DEVICES += rt-n10-plus
endef
TARGET_DEVICES += asus_rt-n10-plus

define Device/asus_rt-n13u
  MTK_SOC := rt3052
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Asus
  DEVICE_MODEL := RT-N13U
  DEVICE_PACKAGES := kmod-leds-gpio kmod-rt2800-pci kmod-usb-dwc2
  SUPPORTED_DEVICES += rt-n13u
endef
TARGET_DEVICES += asus_rt-n13u

define Device/asus_wl-330n
  MTK_SOC := rt3050
  BLOCKSIZE := 4k
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := Asus
  DEVICE_MODEL := WL-330N
  SUPPORTED_DEVICES += wl-330n
endef
TARGET_DEVICES += asus_wl-330n

define Device/asus_wl-330n3g
  MTK_SOC := rt3050
  BLOCKSIZE := 4k
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := Asus
  DEVICE_MODEL := WL-330N3G
  DEVICE_PACKAGES :=
  SUPPORTED_DEVICES += wl-330n3g
endef
TARGET_DEVICES += asus_wl-330n3g

define Device/aximcom_mr-102n
  MTK_SOC := rt3052
  IMAGE_SIZE := 7744k
  DEVICE_VENDOR := AXIMCom
  DEVICE_MODEL := MR-102N
  SUPPORTED_DEVICES += mr-102n
endef
TARGET_DEVICES += aximcom_mr-102n

define Device/aztech_hw550-3g
  MTK_SOC := rt3052
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Aztech
  DEVICE_MODEL := HW550-3G
  DEVICE_ALT0_VENDOR := Allnet
  DEVICE_ALT0_MODEL := ALL0239-3G
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += hw550-3g
endef
TARGET_DEVICES += aztech_hw550-3g

define Device/belkin_f5d8235-v2
  MTK_SOC := rt3052
  IMAGE_SIZE := 7744k
  DEVICE_VENDOR := Belkin
  DEVICE_MODEL := F5D8235
  DEVICE_VARIANT := v2
  DEVICE_PACKAGES := kmod-switch-rtl8366rb
  SUPPORTED_DEVICES += f5d8235-v2
endef
TARGET_DEVICES += belkin_f5d8235-v2

define Device/belkin_f7c027
  MTK_SOC := rt5350
  IMAGE_SIZE := 7616k
  DEVICE_VENDOR := Belkin
  DEVICE_MODEL := F7C027
  DEVICE_PACKAGES := --kmod-usb-dwc2 -kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += f7c027
endef
TARGET_DEVICES += belkin_f7c027

define Device/buffalo_whr-g300n
  MTK_SOC := rt3052
  BLOCKSIZE := 64k
  IMAGE_SIZE := 3712k
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WHR-G300N
  IMAGES += tftp.bin
  IMAGE/tftp.bin := $$(sysupgrade_bin) | \
    check-size $$$$(IMAGE_SIZE) | buffalo-tftp-header
  SUPPORTED_DEVICES += whr-g300n
endef
TARGET_DEVICES += buffalo_whr-g300n

define Device/dlink_dap-1350
  MTK_SOC := rt3052
  IMAGES += factory.bin factory-NA.bin
  IMAGE_SIZE := 7488k
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	dap-header -s RT3052-AP-DAP1350WW-3
  IMAGE/factory-NA.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	dap-header -s RT3052-AP-DAP1350-3
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DAP-1350
  SUPPORTED_DEVICES += dap-1350
endef
TARGET_DEVICES += dlink_dap-1350

define Device/dlink_dcs-930
  MTK_SOC := rt3050
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DCS-930
  DEVICE_PACKAGES := kmod-video-core kmod-video-uvc kmod-sound-core kmod-usb-audio kmod-usb-dwc2
  SUPPORTED_DEVICES += dcs-930
endef
TARGET_DEVICES += dlink_dcs-930

define Device/dlink_dcs-930l-b1
  MTK_SOC := rt5350
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DCS-930L
  DEVICE_VARIANT := B1
  DEVICE_PACKAGES := kmod-video-core kmod-video-uvc kmod-sound-core kmod-usb-audio kmod-usb-ohci kmod-usb2
  SUPPORTED_DEVICES += dcs-930l-b1
endef
TARGET_DEVICES += dlink_dcs-930l-b1

define Device/dlink_dir-300-b1
  MTK_SOC := rt3050
  IMAGE_SIZE := 3776k
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | wrg-header wrgn23_dlwbr_dir300b
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DIR-300
  DEVICE_VARIANT := B1
  SUPPORTED_DEVICES += dir-300-b1
endef
TARGET_DEVICES += dlink_dir-300-b1

define Device/dlink_dir-300-b7
  MTK_SOC := rt5350
  BLOCKSIZE := 4k
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DIR-300
  DEVICE_VARIANT := B7
  SUPPORTED_DEVICES += dir-300-b7
endef
TARGET_DEVICES += dlink_dir-300-b7

define Device/dlink_dir-320-b1
  MTK_SOC := rt5350
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DIR-320
  DEVICE_VARIANT := B1
  SUPPORTED_DEVICES += dir-320-b1
endef
TARGET_DEVICES += dlink_dir-320-b1

define Device/dlink_dir-600-b1
  MTK_SOC := rt3050
  IMAGE_SIZE := 3776k
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | wrg-header wrgn23_dlwbr_dir600b
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DIR-600
  DEVICE_VARIANT := B1/B2
  SUPPORTED_DEVICES += dir-600-b1 dir-600-b2
endef
TARGET_DEVICES += dlink_dir-600-b1

define Device/dlink_dir-610-a1
  $(Device/seama)
  MTK_SOC := rt5350
  BLOCKSIZE := 4k
  SEAMA_SIGNATURE := wrgn59_dlob.hans_dir610
  KERNEL := $(KERNEL_DTB)
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DIR-610
  DEVICE_VARIANT := A1
  DEVICE_PACKAGES := kmod-ledtrig-netdev kmod-ledtrig-timer
  SUPPORTED_DEVICES += dir-610-a1
endef
TARGET_DEVICES += dlink_dir-610-a1

define Device/dlink_dir-615-d
  MTK_SOC := rt3050
  IMAGE_SIZE := 3776k
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | wrg-header wrgn23_dlwbr_dir615d
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DIR-615
  DEVICE_VARIANT := D
  SUPPORTED_DEVICES += dir-615-d
endef
TARGET_DEVICES += dlink_dir-615-d

define Device/dlink_dir-615-h1
  MTK_SOC := rt3352
  BLOCKSIZE := 4k
  IMAGES += factory.bin
  IMAGE_SIZE := 3776k
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | senao-header -r 0x218 -p 0x30 -t 3
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DIR-615
  DEVICE_VARIANT := H1
  SUPPORTED_DEVICES += dir-615-h1
endef
TARGET_DEVICES += dlink_dir-615-h1

define Device/dlink_dir-620-a1
  MTK_SOC := rt3050
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DIR-620
  DEVICE_VARIANT := A1
  SUPPORTED_DEVICES += dir-620-a1
endef
TARGET_DEVICES += dlink_dir-620-a1

define Device/dlink_dir-620-d1
  MTK_SOC := rt3352
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DIR-620
  DEVICE_VARIANT := D1
  SUPPORTED_DEVICES += dir-620-d1
endef
TARGET_DEVICES += dlink_dir-620-d1

define Device/dlink_dwr-512-b
  MTK_SOC := rt5350
  IMAGE_SIZE := 8064k
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DWR-512
  DEVICE_VARIANT := B
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
  SUPPORTED_DEVICES += dwr-512-b
endef
TARGET_DEVICES += dlink_dwr-512-b

define Device/easyacc_wizard-8800
  MTK_SOC := rt5350
  IMAGE_SIZE := 7872k
  UIMAGE_NAME:= Linux Kernel Image
  DEVICE_VENDOR := EasyAcc
  DEVICE_MODEL := WIZARD 8800
  SUPPORTED_DEVICES += wizard8800
endef
TARGET_DEVICES += easyacc_wizard-8800

define Device/edimax_3g-6200n
  MTK_SOC := rt3050
  IMAGE_SIZE := 3648k
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | \
	edimax-header -s CSYS -m 3G62 -f 0x50000 -S 0x01100000 | pad-rootfs | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_VENDOR := Edimax
  DEVICE_MODEL := 3g-6200n
  SUPPORTED_DEVICES += 3g-6200n
endef
TARGET_DEVICES += edimax_3g-6200n

define Device/edimax_3g-6200nl
  MTK_SOC := rt3050
  IMAGE_SIZE := 3648k
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | \
	edimax-header -s CSYS -m 3G62 -f 0x50000 -S 0x01100000 | pad-rootfs | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_VENDOR := Edimax
  DEVICE_MODEL := 3g-6200nl
  SUPPORTED_DEVICES += 3g-6200nl
endef
TARGET_DEVICES += edimax_3g-6200nl

define Device/engenius_esr-9753
  MTK_SOC := rt3052
  BLOCKSIZE := 64k
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := EnGenius
  DEVICE_MODEL := ESR-9753
  SUPPORTED_DEVICES += esr-9753
endef
TARGET_DEVICES += engenius_esr-9753

define Device/fon_fonera-20n
  MTK_SOC := rt3052
  IMAGE_SIZE := 7872k
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | \
	edimax-header -s RSDK -m NL1T -f 0x50000 -S 0xc0000
  DEVICE_VENDOR := Fon
  DEVICE_MODEL := Fonera 2.0N
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += fonera20n
endef
TARGET_DEVICES += fon_fonera-20n

define Device/hame_mpr-a1
  MTK_SOC := rt5350
  BLOCKSIZE := 4k
  IMAGE_SIZE := 3776k
  UIMAGE_NAME:= Linux Kernel Image
  DEVICE_VENDOR := HAME
  DEVICE_MODEL := MPR
  DEVICE_VARIANT := A1
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 kmod-ledtrig-netdev
  SUPPORTED_DEVICES += mpr-a1
endef
TARGET_DEVICES += hame_mpr-a1

define Device/hame_mpr-a2
  MTK_SOC := rt5350
  IMAGE_SIZE := 7872k
  UIMAGE_NAME:= Linux Kernel Image
  DEVICE_VENDOR := HAME
  DEVICE_MODEL := MPR
  DEVICE_VARIANT := A2
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 kmod-ledtrig-netdev
  SUPPORTED_DEVICES += mpr-a2
endef
TARGET_DEVICES += hame_mpr-a2

define Device/hauppauge_broadway
  MTK_SOC := rt3052
  IMAGE_SIZE := 7744k
  UIMAGE_NAME:= Broadway Kernel Image
  DEVICE_VENDOR := Hauppauge
  DEVICE_MODEL := Broadway
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += broadway
endef
TARGET_DEVICES += hauppauge_broadway

define Device/hilink_hlk-rm04
  MTK_SOC := rt5350
  IMAGE_SIZE := 3776k
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | hilink-header
  DEVICE_VENDOR := Hi-Link
  DEVICE_MODEL := HLK-RM04
  SUPPORTED_DEVICES += hlk-rm04
endef
TARGET_DEVICES += hilink_hlk-rm04

define Device/hootoo_ht-tm02
  MTK_SOC := rt5350
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := HooToo
  DEVICE_MODEL := HT-TM02
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += ht-tm02
endef
TARGET_DEVICES += hootoo_ht-tm02

define Device/huawei_d105
  MTK_SOC := rt3050
  BLOCKSIZE := 64k
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := Huawei
  DEVICE_MODEL := D105
  SUPPORTED_DEVICES += d105
endef
TARGET_DEVICES += huawei_d105

define Device/huawei_hg255d
  MTK_SOC := rt3052
  IMAGE_SIZE := 15744k
  DEVICE_VENDOR := HuaWei
  DEVICE_MODEL := HG255D
  SUPPORTED_DEVICES += hg255d
endef
TARGET_DEVICES += huawei_hg255d

define Device/intenso_memory2move
  MTK_SOC := rt5350
  IMAGE_SIZE := 7872k
  UIMAGE_NAME:= Linux Kernel Image
  DEVICE_VENDOR := Intenso
  DEVICE_MODEL := Memory 2 Move
  DEVICE_PACKAGES := kmod-ledtrig-netdev kmod-ledtrig-timer \
		kmod-usb2 kmod-usb-storage kmod-scsi-core \
		kmod-fs-ext4 kmod-fs-vfat block-mount
  SUPPORTED_DEVICES += m2m
endef
TARGET_DEVICES += intenso_memory2move

define Device/jcg_jhr-n805r
  MTK_SOC := rt3050
  IMAGE_SIZE := 3776k
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | jcg-header 29.24
  DEVICE_VENDOR := JCG
  DEVICE_MODEL := JHR-N805R
  SUPPORTED_DEVICES += jhr-n805r
endef
TARGET_DEVICES += jcg_jhr-n805r

define Device/jcg_jhr-n825r
  MTK_SOC := rt3052
  IMAGE_SIZE := 3776k
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | jcg-header 23.24
  DEVICE_VENDOR := JCG
  DEVICE_MODEL := JHR-N825R
  SUPPORTED_DEVICES += jhr-n825r
endef
TARGET_DEVICES += jcg_jhr-n825r

define Device/jcg_jhr-n926r
  MTK_SOC := rt3052
  IMAGE_SIZE := 3776k
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | jcg-header 25.24
  DEVICE_VENDOR := JCG
  DEVICE_MODEL := JHR-N926R
  SUPPORTED_DEVICES += jhr-n926r
endef
TARGET_DEVICES += jcg_jhr-n926r

define Device/mofinetwork_mofi3500-3gn
  MTK_SOC := rt3052
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := MoFi Network
  DEVICE_MODEL := MOFI3500-3GN
  SUPPORTED_DEVICES += mofi3500-3gn
endef
TARGET_DEVICES += mofinetwork_mofi3500-3gn

define Device/netcore_nw718
  MTK_SOC := rt3050
  IMAGE_SIZE := 3712k
  UIMAGE_NAME:= ARA1B4NCRNW718;1
  DEVICE_VENDOR := Netcore
  DEVICE_MODEL := NW718
  SUPPORTED_DEVICES += nw718
endef
TARGET_DEVICES += netcore_nw718

define Device/netgear_wnce2001
  MTK_SOC := rt3052
  IMAGE_SIZE := 3392k
  IMAGES += factory.bin factory-NA.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	dap-header -s RT3052-AP-WNCE2001-3 -r WW -v 1.0.0.99
  IMAGE/factory-NA.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	dap-header -s RT3052-AP-WNCE2001-3 -r NA -v 1.0.0.99
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := WNCE2001
  SUPPORTED_DEVICES += wnce2001
endef
TARGET_DEVICES += netgear_wnce2001

define Device/nexaira_bc2
  MTK_SOC := rt3052
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := NexAira
  DEVICE_MODEL := BC2
  SUPPORTED_DEVICES += bc2
endef
TARGET_DEVICES += nexaira_bc2

define Device/nexx_wt1520-4m
  MTK_SOC := rt5350
  IMAGE_SIZE := 3776k
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | poray-header -B WT1520 -F 4M
  DEVICE_VENDOR := Nexx
  DEVICE_MODEL := WT1520
  DEVICE_VARIANT := 4M
  SUPPORTED_DEVICES += wt1520-4M
endef
TARGET_DEVICES += nexx_wt1520-4m

define Device/nexx_wt1520-8m
  MTK_SOC := rt5350
  IMAGE_SIZE := 7872k
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | poray-header -B WT1520 -F 8M
  DEVICE_VENDOR := Nexx
  DEVICE_MODEL := WT1520
  DEVICE_VARIANT := 8M
  SUPPORTED_DEVICES += wt1520-8M
endef
TARGET_DEVICES += nexx_wt1520-8m

define Device/nixcore_x1-16m
  MTK_SOC := rt5350
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Nixcore
  DEVICE_MODEL := X1
  DEVICE_VARIANT := 16M
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 kmod-i2c-core kmod-i2c-ralink kmod-spi-dev
  SUPPORTED_DEVICES += nixcore-x1 nixcore-x1-16M
endef
TARGET_DEVICES += nixcore_x1-16m

define Device/nixcore_x1-8m
  MTK_SOC := rt5350
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Nixcore
  DEVICE_MODEL := X1
  DEVICE_VARIANT := 8M
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 kmod-i2c-core kmod-i2c-ralink kmod-spi-dev
  SUPPORTED_DEVICES += nixcore-x1 nixcore-x1-8M
endef
TARGET_DEVICES += nixcore_x1-8m

define Device/olimex_rt5350f-olinuxino
  MTK_SOC := rt5350
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := OLIMEX
  DEVICE_MODEL := RT5350F-OLinuXino
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 \
		kmod-i2c-core kmod-i2c-ralink \
		kmod-spi-dev
  SUPPORTED_DEVICES += rt5350f-olinuxino
endef
TARGET_DEVICES += olimex_rt5350f-olinuxino

define Device/olimex_rt5350f-olinuxino-evb
  MTK_SOC := rt5350
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := OLIMEX
  DEVICE_MODEL := RT5350F-OLinuXino-EVB
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 \
		kmod-i2c-core kmod-i2c-ralink \
		kmod-spi-dev
  SUPPORTED_DEVICES += rt5350f-olinuxino-evb
endef
TARGET_DEVICES += olimex_rt5350f-olinuxino-evb

define Device/omnima_miniembplug
  MTK_SOC := rt5350
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Omnima
  DEVICE_MODEL := MiniEMBPlug
  SUPPORTED_DEVICES += miniembplug
endef
TARGET_DEVICES += omnima_miniembplug

define Device/omnima_miniembwifi
  MTK_SOC := rt3052
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Omnima
  DEVICE_MODEL := MiniEMBWiFi
  SUPPORTED_DEVICES += miniembwifi
endef
TARGET_DEVICES += omnima_miniembwifi

define Device/petatel_psr-680w
  MTK_SOC := rt3052
  BLOCKSIZE := 64k
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := Petatel
  DEVICE_MODEL := PSR-680W Wireless 3G Router
  SUPPORTED_DEVICES += psr-680w
endef
TARGET_DEVICES += petatel_psr-680w

define Device/planex_mzk-dp150n
  MTK_SOC := rt5350
  BLOCKSIZE := 64k
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := Planex
  DEVICE_MODEL := MZK-DP150N
  DEVICE_PACKAGES := kmod-spi-dev
  SUPPORTED_DEVICES += mzk-dp150n
endef
TARGET_DEVICES += planex_mzk-dp150n

define Device/planex_mzk-w300nh2
  MTK_SOC := rt3052
  IMAGE_SIZE := 3648k
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | \
	edimax-header -s CSYS -m RN52 -f 0x50000 -S 0xc0000
  DEVICE_VENDOR := Planex
  DEVICE_MODEL := MZK-W300NH2
  SUPPORTED_DEVICES += mzk-w300nh2
endef
TARGET_DEVICES += planex_mzk-w300nh2

define Device/planex_mzk-wdpr
  MTK_SOC := rt3052
  IMAGE_SIZE := 6656k
  DEVICE_VENDOR := Planex
  DEVICE_MODEL := MZK-WDPR
  SUPPORTED_DEVICES += mzk-wdpr
endef
TARGET_DEVICES += planex_mzk-wdpr

define Device/poray_ip2202
  MTK_SOC := rt3052
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Poray
  DEVICE_MODEL := IP2202
  SUPPORTED_DEVICES += ip2202
endef
TARGET_DEVICES += poray_ip2202

define Device/poray_m3
  MTK_SOC := rt5350
  IMAGE_SIZE := 3776k
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | poray-header -B M3 -F 4M
  DEVICE_VENDOR := Poray
  DEVICE_MODEL := M3
  DEVICE_PACKAGES := kmod-usb2 kmod-ledtrig-netdev \
	kmod-ledtrig-timer
  SUPPORTED_DEVICES += m3
endef
TARGET_DEVICES += poray_m3

define Device/poray_m4-4m
  MTK_SOC := rt5350
  IMAGE_SIZE := 3776k
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | poray-header -B M4 -F 4M
  DEVICE_VENDOR := Poray
  DEVICE_MODEL := M4
  DEVICE_VARIANT := 4M
  DEVICE_PACKAGES := kmod-usb2 kmod-ledtrig-netdev \
	kmod-ledtrig-timer
  SUPPORTED_DEVICES += m4-4M
endef
TARGET_DEVICES += poray_m4-4m

define Device/poray_m4-8m
  MTK_SOC := rt5350
  IMAGE_SIZE := 7872k
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | poray-header -B M4 -F 8M
  DEVICE_VENDOR := Poray
  DEVICE_MODEL := M4
  DEVICE_VARIANT := 8M
  DEVICE_PACKAGES := kmod-usb2 kmod-ledtrig-netdev kmod-ledtrig-timer
  SUPPORTED_DEVICES += m4-8M
endef
TARGET_DEVICES += poray_m4-8m

define Device/poray_x5
  MTK_SOC := rt5350
  IMAGE_SIZE := 7872k
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | poray-header -B X5 -F 8M
  DEVICE_VENDOR := Poray
  DEVICE_MODEL := X5/X6
  DEVICE_PACKAGES := kmod-usb2 kmod-ledtrig-netdev kmod-ledtrig-timer
  SUPPORTED_DEVICES += x5
endef
TARGET_DEVICES += poray_x5

define Device/poray_x8
  MTK_SOC := rt5350
  IMAGE_SIZE := 7872k
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | poray-header -B X8 -F 8M
  DEVICE_VENDOR := Poray
  DEVICE_MODEL := X8
  DEVICE_PACKAGES := kmod-usb2 kmod-ledtrig-netdev kmod-ledtrig-timer
  SUPPORTED_DEVICES += x8
endef
TARGET_DEVICES += poray_x8

define Device/prolink_pwh2004
  MTK_SOC := rt3052
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Prolink
  DEVICE_MODEL := PWH2004
  DEVICE_PACKAGES :=
  SUPPORTED_DEVICES += pwh2004
endef
TARGET_DEVICES += prolink_pwh2004

define Device/ralink_v22rw-2x2
  MTK_SOC := rt3052
  BLOCKSIZE := 64k
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := Ralink
  DEVICE_MODEL := AP-RT3052-V22RW-2X2
  SUPPORTED_DEVICES += v22rw-2x2
endef
TARGET_DEVICES += ralink_v22rw-2x2

define Device/sitecom_wl-351
  MTK_SOC := rt3052
  BLOCKSIZE := 64k
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := Sitecom
  DEVICE_MODEL := WL-351 v1
  DEVICE_PACKAGES := kmod-switch-rtl8366rb kmod-swconfig swconfig
  SUPPORTED_DEVICES += wl-351
endef
TARGET_DEVICES += sitecom_wl-351

define Device/skyline_sl-r7205
  MTK_SOC := rt3052
  BLOCKSIZE := 64k
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := Skyline
  DEVICE_MODEL := SL-R7205 Wireless 3G Router
  SUPPORTED_DEVICES += sl-r7205
endef
TARGET_DEVICES += skyline_sl-r7205

define Device/sparklan_wcr-150gn
  MTK_SOC := rt3050
  BLOCKSIZE := 64k
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := Sparklan
  DEVICE_MODEL := WCR-150GN
  SUPPORTED_DEVICES += wcr-150gn
endef
TARGET_DEVICES += sparklan_wcr-150gn

define Device/teltonika_rut5xx
  MTK_SOC := rt3050
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Teltonika
  DEVICE_MODEL := RUT5XX
  DEVICE_PACKAGES := om-watchdog
  SUPPORTED_DEVICES += rut5xx
endef
TARGET_DEVICES += teltonika_rut5xx

define Device/tenda_3g150b
  MTK_SOC := rt5350
  BLOCKSIZE := 4k
  IMAGE_SIZE := 3776k
  UIMAGE_NAME:= Linux Kernel Image
  DEVICE_VENDOR := Tenda
  DEVICE_MODEL := 3G150B
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += 3g150b
endef
TARGET_DEVICES += tenda_3g150b

define Device/tenda_3g300m
  MTK_SOC := rt3052
  IMAGE_SIZE := 3776k
  UIMAGE_NAME := 3G150M_SPI Kernel Image
  DEVICE_VENDOR := Tenda
  DEVICE_MODEL := 3G300M
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += 3g300m
endef
TARGET_DEVICES += tenda_3g300m

define Device/tenda_w150m
  MTK_SOC := rt3050
  IMAGE_SIZE := 3776k
  UIMAGE_NAME:= W150M Kernel Image
  DEVICE_VENDOR := Tenda
  DEVICE_MODEL := W150M
  SUPPORTED_DEVICES += w150m
endef
TARGET_DEVICES += tenda_w150m

define Device/tenda_w306r-v2
  MTK_SOC := rt3052
  IMAGE_SIZE := 3776k
  UIMAGE_NAME:= linkn Kernel Image
  DEVICE_VENDOR := Tenda
  DEVICE_MODEL := W306R
  DEVICE_VARIANT := V2.0
  SUPPORTED_DEVICES += w306r-v20
endef
TARGET_DEVICES += tenda_w306r-v2

define Device/trendnet_tew-638apb-v2
  MTK_SOC := rt3050
  BLOCKSIZE := 64k
  IMAGE_SIZE := 3776k
  IMAGE/sysupgrade.bin := $$(sysupgrade_bin) | umedia-header 0x026382 | \
        append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_VENDOR := TRENDnet
  DEVICE_MODEL := TEW-638APB
  DEVICE_VARIANT := v2
  SUPPORTED_DEVICES += tew-638apb-v2
endef
TARGET_DEVICES += trendnet_tew-638apb-v2

define Device/trendnet_tew-714tru
  MTK_SOC := rt5350
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := TRENDnet
  DEVICE_MODEL := TEW-714TRU
  SUPPORTED_DEVICES += tew-714tru
endef
TARGET_DEVICES += trendnet_tew-714tru

define Device/unbranded_a5-v11
  MTK_SOC := rt5350
  IMAGE_SIZE := 3776k
  IMAGES += factory.bin
  IMAGE/factory.bin := \
	$$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | poray-header -B A5-V11 -F 4M
  DEVICE_VENDOR := 
  DEVICE_MODEL := A5-V11
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2
  SUPPORTED_DEVICES += a5-v11
endef
TARGET_DEVICES += unbranded_a5-v11

define Device/unbranded_wr512-3gn-4m
  MTK_SOC := rt3052
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := Ralink
  DEVICE_MODEL := WR512-3GN
  DEVICE_VARIANT := 4M
  SUPPORTED_DEVICES += wr512-3gn-4M
endef
TARGET_DEVICES += unbranded_wr512-3gn-4m

define Device/unbranded_wr512-3gn-8m
  MTK_SOC := rt3052
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Ralink
  DEVICE_MODEL := WR512-3GN
  DEVICE_VARIANT := 8M
  SUPPORTED_DEVICES += wr512-3gn-8M
endef
TARGET_DEVICES += unbranded_wr512-3gn-8m

define Device/unbranded_xdx-rn502j
  MTK_SOC := rt3052
  BLOCKSIZE := 64k
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := XDX
  DEVICE_MODEL := RN502J
  SUPPORTED_DEVICES += xdxrn502j
endef
TARGET_DEVICES += unbranded_xdx-rn502j

define Device/upvel_ur-326n4g
  MTK_SOC := rt3052
  BLOCKSIZE := 64k
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := UPVEL
  DEVICE_MODEL := UR-326N4G
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += ur-326n4g
endef
TARGET_DEVICES += upvel_ur-326n4g

define Device/upvel_ur-336un
  MTK_SOC := rt3052
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := UPVEL
  DEVICE_MODEL := UR-336UN
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += ur-336un
endef
TARGET_DEVICES += upvel_ur-336un

define Device/vocore_vocore-16m
  MTK_SOC := rt5350
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := VoCore
  DEVICE_MODEL := VoCore
  DEVICE_VARIANT := 16M
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 \
		kmod-i2c-core kmod-i2c-ralink \
		kmod-spi-dev
  SUPPORTED_DEVICES += vocore vocore-16M
endef
TARGET_DEVICES += vocore_vocore-16m

define Device/vocore_vocore-8m
  MTK_SOC := rt5350
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := VoCore
  DEVICE_MODEL := VoCore
  DEVICE_VARIANT := 8M
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 \
		kmod-i2c-core kmod-i2c-ralink \
		kmod-spi-dev
  SUPPORTED_DEVICES += vocore vocore-8M
endef
TARGET_DEVICES += vocore_vocore-8m

define Device/wansview_ncs601w
  MTK_SOC := rt5350
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Wansview
  DEVICE_MODEL := NCS601W
  DEVICE_PACKAGES := kmod-video-core kmod-video-uvc \
		kmod-usb-ohci
  SUPPORTED_DEVICES += ncs601w
endef
TARGET_DEVICES += wansview_ncs601w

define Device/wiznet_wizfi630a
  MTK_SOC := rt5350
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := WIZnet
  DEVICE_MODEL := WizFi630A
  SUPPORTED_DEVICES += wizfi630a
endef
TARGET_DEVICES += wiznet_wizfi630a

define Device/zorlik_zl5900v2
  MTK_SOC := rt5350
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Zorlik
  DEVICE_MODEL := ZL5900V2
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 kmod-ledtrig-netdev
endef
TARGET_DEVICES += zorlik_zl5900v2

define Device/zyxel_keenetic
  MTK_SOC := rt3052
  BLOCKSIZE := 64k
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := ZyXEL
  DEVICE_MODEL := Keenetic
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ehci kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += kn
endef
TARGET_DEVICES += zyxel_keenetic

define Device/zyxel_keenetic-start
  MTK_SOC := rt5350
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := ZyXEL
  DEVICE_MODEL := Keenetic Start
endef
TARGET_DEVICES += zyxel_keenetic-start

define Device/zyxel_nbg-419n
  MTK_SOC := rt3052
  BLOCKSIZE := 64k
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := ZyXEL
  DEVICE_MODEL := NBG-419N
  SUPPORTED_DEVICES += nbg-419n
endef
TARGET_DEVICES += zyxel_nbg-419n

define Device/zyxel_nbg-419n-v2
  MTK_SOC := rt3352
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := ZyXEL
  DEVICE_MODEL := NBG-419N
  DEVICE_VARIANT := v2
  SUPPORTED_DEVICES += nbg-419n2
endef
TARGET_DEVICES += zyxel_nbg-419n-v2
