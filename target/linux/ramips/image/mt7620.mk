#
# MT7620A Profiles
#

DEVICE_VARS += TPLINK_FLASHLAYOUT TPLINK_HWID TPLINK_HWREV TPLINK_HWREVADD TPLINK_HVERSION \
	DLINK_ROM_ID DLINK_FAMILY_MEMBER DLINK_FIRMWARE_SIZE DLINK_IMAGE_OFFSET

define Build/elecom-header
	cp $@ $(KDIR)/v_0.0.0.bin
	( \
		mkhash md5 $(KDIR)/v_0.0.0.bin && \
		echo 458 \
	) | mkhash md5 > $(KDIR)/v_0.0.0.md5
	$(STAGING_DIR_HOST)/bin/tar -c \
		$(if $(SOURCE_DATE_EPOCH),--mtime=@$(SOURCE_DATE_EPOCH)) \
		--owner=0 --group=0 -f $@ -C $(KDIR) v_0.0.0.bin v_0.0.0.md5
endef

define Device/aigale_ai-br100
  MTK_SOC := mt7620a
  IMAGE_SIZE := 7936k
  DEVICE_VENDOR := Aigale
  DEVICE_MODEL := Ai-BR100
  DEVICE_PACKAGES:= kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += ai-br100
endef
TARGET_DEVICES += aigale_ai-br100

define Device/alfa-network_ac1200rm
  MTK_SOC := mt7620a
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := ALFA Network
  DEVICE_MODEL := AC1200RM
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci uboot-envtools
endef
TARGET_DEVICES += alfa-network_ac1200rm

define Device/alfa-network_r36m-e4g
  MTK_SOC := mt7620a
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := ALFA Network
  DEVICE_MODEL := R36M-E4G
  DEVICE_PACKAGES := kmod-i2c-ralink kmod-usb2 kmod-usb-ohci uboot-envtools uqmi
endef
TARGET_DEVICES += alfa-network_r36m-e4g

define Device/alfa-network_tube-e4g
  MTK_SOC := mt7620a
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := ALFA Network
  DEVICE_MODEL := Tube-E4G
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci uboot-envtools uqmi \
	-iwinfo -kmod-rt2800-soc -wpad-openssl
endef
TARGET_DEVICES += alfa-network_tube-e4g

define Device/amit_jboot
  DLINK_IMAGE_OFFSET := 0x10000
  KERNEL := $(KERNEL_DTB)
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := mkdlinkfw | pad-rootfs | append-metadata
  IMAGE/factory.bin := mkdlinkfw | pad-rootfs | mkdlinkfw-factory
  DEVICE_PACKAGES := jboot-tools kmod-usb2 kmod-usb-ohci
endef

define Device/Archer
  MTK_SOC := mt7620a
  DEVICE_VENDOR := TP-Link
  TPLINK_HWREVADD := 0
  TPLINK_HVERSION := 3
  KERNEL := $(KERNEL_DTB)
  KERNEL_INITRAMFS := $(KERNEL_DTB) | tplink-v2-header -e
  IMAGE/factory.bin := tplink-v2-image -e
  IMAGE/sysupgrade.bin := tplink-v2-image -s -e | append-metadata
endef

define Device/asus_rp-n53
  MTK_SOC := mt7620a
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Asus
  DEVICE_MODEL := RP-N53
  DEVICE_PACKAGES := kmod-rt2800-pci
  SUPPORTED_DEVICES += rp-n53
endef
TARGET_DEVICES += asus_rp-n53

define Device/asus_rt-ac51u
  MTK_SOC := mt7620a
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Asus
  DEVICE_MODEL := RT-AC51U
  DEVICE_PACKAGES := kmod-mt76x0e kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += rt-ac51u
endef
TARGET_DEVICES += asus_rt-ac51u

define Device/asus_rt-n12p
  MTK_SOC := mt7620n
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Asus
  DEVICE_MODEL := RT-N11P/RT-N12+/RT-N12Eb1
  SUPPORTED_DEVICES += rt-n12p
endef
TARGET_DEVICES += asus_rt-n12p

define Device/asus_rt-n14u
  MTK_SOC := mt7620n
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Asus
  DEVICE_MODEL := RT-N14u
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += rt-n14u
endef
TARGET_DEVICES += asus_rt-n14u

define Device/bdcom_wap2100-sk
  MTK_SOC := mt7620a
  IMAGE_SIZE := 15808k
  DEVICE_VENDOR := BDCOM
  DEVICE_MODEL := WAP2100-SK (ZTE ZXECS EBG3130)
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-mt76x2 kmod-mt76x0e kmod-sdhci-mt7620 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += bdcom_wap2100-sk

define Device/buffalo_whr-1166d
  MTK_SOC := mt7620a
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WHR-1166D
  DEVICE_PACKAGES := kmod-mt76x2
  SUPPORTED_DEVICES += whr-1166d
endef
TARGET_DEVICES += buffalo_whr-1166d

define Device/buffalo_whr-300hp2
  MTK_SOC := mt7620a
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WHR-300HP2
  SUPPORTED_DEVICES += whr-300hp2
endef
TARGET_DEVICES += buffalo_whr-300hp2

define Device/buffalo_whr-600d
  MTK_SOC := mt7620a
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WHR-600D
  DEVICE_PACKAGES := kmod-rt2800-pci
  SUPPORTED_DEVICES += whr-600d
endef
TARGET_DEVICES += buffalo_whr-600d

define Device/buffalo_wmr-300
  MTK_SOC := mt7620n
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WMR-300
  SUPPORTED_DEVICES += wmr-300
endef
TARGET_DEVICES += buffalo_wmr-300

define Device/comfast_cf-wr800n
  MTK_SOC := mt7620n
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Comfast
  DEVICE_MODEL := CF-WR800N
  SUPPORTED_DEVICES += cf-wr800n
endef
TARGET_DEVICES += comfast_cf-wr800n

define Device/dlink_dch-m225
  $(Device/seama)
  MTK_SOC := mt7620a
  BLOCKSIZE := 4k
  SEAMA_SIGNATURE := wapn22_dlink.2013gui_dap1320b
  IMAGE_SIZE := 6848k
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DCH-M225
  DEVICE_PACKAGES := kmod-sound-core kmod-sound-mt7620 kmod-i2c-ralink
  SUPPORTED_DEVICES += dch-m225
endef
TARGET_DEVICES += dlink_dch-m225

define Device/dlink_dir-510l
  $(Device/amit_jboot)
  MTK_SOC := mt7620a
  IMAGE_SIZE := 14208k
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DIR-510L
  DEVICE_PACKAGES += kmod-mt76x0e
  DLINK_ROM_ID := DLK6E3805001
  DLINK_FAMILY_MEMBER := 0x6E38
  DLINK_FIRMWARE_SIZE := 0xDE0000
  DLINK_IMAGE_OFFSET := 0x210000
endef
TARGET_DEVICES += dlink_dir-510l

define Device/dlink_dir-810l
  MTK_SOC := mt7620a
  DEVICE_PACKAGES := kmod-mt76x0e
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DIR-810L
  IMAGE_SIZE := 6720k
  SUPPORTED_DEVICES += dir-810l
endef
TARGET_DEVICES += dlink_dir-810l

define Device/dlink_dwr-116-a1
  $(Device/amit_jboot)
  MTK_SOC := mt7620n
  IMAGE_SIZE := 8064k
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DWR-116
  DEVICE_VARIANT := A1/A2
  DLINK_ROM_ID := DLK6E3803001
  DLINK_FAMILY_MEMBER := 0x6E38
  DLINK_FIRMWARE_SIZE := 0x7E0000
endef
TARGET_DEVICES += dlink_dwr-116-a1

define Device/dlink_dwr-118-a1
  $(Device/amit_jboot)
  MTK_SOC := mt7620a
  IMAGE_SIZE := 16256k
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DWR-118
  DEVICE_VARIANT := A1
  DEVICE_PACKAGES += kmod-mt76x0e
  DLINK_ROM_ID := DLK6E3811001
  DLINK_FAMILY_MEMBER := 0x6E38
  DLINK_FIRMWARE_SIZE := 0xFE0000
endef
TARGET_DEVICES += dlink_dwr-118-a1

define Device/dlink_dwr-118-a2
  $(Device/amit_jboot)
  MTK_SOC := mt7620a
  IMAGE_SIZE := 16256k
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DWR-118
  DEVICE_VARIANT := A2
  DEVICE_PACKAGES += kmod-mt76x2
  DLINK_ROM_ID := DLK6E3814001
  DLINK_FAMILY_MEMBER := 0x6E38
  DLINK_FIRMWARE_SIZE := 0xFE0000
endef
TARGET_DEVICES += dlink_dwr-118-a2

define Device/dlink_dwr-921-c1
  $(Device/amit_jboot)
  MTK_SOC := mt7620n
  IMAGE_SIZE := 16256k
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DWR-921
  DEVICE_VARIANT := C1
  DLINK_ROM_ID := DLK6E2414001
  DLINK_FAMILY_MEMBER := 0x6E24
  DLINK_FIRMWARE_SIZE := 0xFE0000
  DEVICE_PACKAGES += kmod-usb-net-qmi-wwan kmod-usb-serial-option uqmi
endef
TARGET_DEVICES += dlink_dwr-921-c1

define Device/dlink_dwr-921-c3
  $(Device/dlink_dwr-921-c1)
  DEVICE_DTS := mt7620n_dlink_dwr-921-c1
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DWR-921
  DEVICE_VARIANT := C3
  DLINK_ROM_ID := DLK6E2414009
  SUPPORTED_DEVICES := dlink,dwr-921-c1
endef
TARGET_DEVICES += dlink_dwr-921-c3

define Device/dlink_dwr-922-e2
  $(Device/amit_jboot)
  MTK_SOC := mt7620n
  IMAGE_SIZE := 16256k
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DWR-922
  DEVICE_VARIANT := E2
  DLINK_ROM_ID := DLK6E2414005
  DLINK_FAMILY_MEMBER := 0x6E24
  DLINK_FIRMWARE_SIZE := 0xFE0000
  DEVICE_PACKAGES += kmod-usb-net-qmi-wwan kmod-usb-serial-option uqmi
endef
TARGET_DEVICES += dlink_dwr-922-e2

define Device/dovado_tiny-ac
  MTK_SOC := mt7620a
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Dovado
  DEVICE_MODEL := Tiny AC
  DEVICE_PACKAGES := kmod-mt76x0e kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += tiny-ac
endef
TARGET_DEVICES += dovado_tiny-ac

define Device/edimax_br-6478ac-v2
  MTK_SOC := mt7620a
  DEVICE_VENDOR := Edimax
  DEVICE_MODEL := BR-6478AC
  DEVICE_VARIANT := V2
  BLOCKSIZE := 64k
  IMAGE_SIZE := 7744k
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | \
	edimax-header -s CSYS -m RN68 -f 0x70000 -S 0x01100000 | pad-rootfs | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += edimax_br-6478ac-v2

define Device/edimax_ew-7476rpc
  MTK_SOC := mt7620a
  DEVICE_VENDOR := Edimax
  DEVICE_MODEL := EW-7476RPC
  BLOCKSIZE := 4k
  IMAGE_SIZE := 7744k
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | \
        edimax-header -s CSYS -m RN79 -f 0x70000 -S 0x01100000 | pad-rootfs | \
        append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_PACKAGES := kmod-mt76x2 kmod-phy-realtek
endef
TARGET_DEVICES += edimax_ew-7476rpc

define Device/edimax_ew-7478ac
  MTK_SOC := mt7620a
  DEVICE_VENDOR := Edimax
  DEVICE_MODEL := EW-7478AC
  BLOCKSIZE := 4k
  IMAGE_SIZE := 7744k
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | \
        edimax-header -s CSYS -m RN70 -f 0x70000 -S 0x01100000 | pad-rootfs | \
        append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_PACKAGES := kmod-mt76x2 kmod-phy-realtek
endef
TARGET_DEVICES += edimax_ew-7478ac

define Device/edimax_ew-7478apc
  MTK_SOC := mt7620a
  DEVICE_VENDOR := Edimax
  DEVICE_MODEL := EW-7478APC
  BLOCKSIZE := 4k
  IMAGE_SIZE := 7744k
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | \
        edimax-header -s CSYS -m RN75 -f 0x70000 -S 0x01100000 | pad-rootfs | \
        append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += edimax_ew-7478apc

define Device/elecom_wrh-300cr
  MTK_SOC := mt7620n
  IMAGE_SIZE := 14272k
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	elecom-header
  DEVICE_VENDOR := Elecom
  DEVICE_MODEL := WRH-300CR
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += wrh-300cr
endef
TARGET_DEVICES += elecom_wrh-300cr

define Device/engenius_esr600
  MTK_SOC := mt7620a
  BLOCKSIZE := 64k
  IMAGE_SIZE := 15616k
  IMAGES += factory.dlf
  IMAGE/factory.dlf := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
       senao-header -r 0x101 -p 0x57 -t 2
  DEVICE_VENDOR := EnGenius
  DEVICE_MODEL := ESR600
  DEVICE_PACKAGES += kmod-rt2800-pci kmod-usb-storage kmod-usb-ohci kmod-usb-ehci
endef
TARGET_DEVICES += engenius_esr600

define Device/fon_fon2601
  MTK_SOC := mt7620a
  IMAGE_SIZE := 15936k
  DEVICE_VENDOR := Fon
  DEVICE_MODEL := FON2601
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci
  KERNEL_INITRAMFS := $$(KERNEL) | fonfxcimage
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs |\
			fonfxcimage |\
			pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += fon_fon2601

define Device/glinet_gl-mt300a
  MTK_SOC := mt7620a
  IMAGE_SIZE := 15872k
  DEVICE_VENDOR := GL.iNet
  DEVICE_MODEL := GL-MT300A
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += gl-mt300a
endef
TARGET_DEVICES += glinet_gl-mt300a

define Device/glinet_gl-mt300n
  MTK_SOC := mt7620a
  IMAGE_SIZE := 15872k
  DEVICE_VENDOR := GL.iNet
  DEVICE_MODEL := GL-MT300N
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += gl-mt300n
endef
TARGET_DEVICES += glinet_gl-mt300n

define Device/glinet_gl-mt750
  MTK_SOC := mt7620a
  IMAGE_SIZE := 15872k
  DEVICE_VENDOR := GL.iNet
  DEVICE_MODEL := GL-MT750
  DEVICE_PACKAGES := kmod-mt76x0e kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += gl-mt750
endef
TARGET_DEVICES += glinet_gl-mt750

define Device/head-weblink_hdrm200
  MTK_SOC := mt7620a
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Head Weblink
  DEVICE_MODEL := HDRM2000
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci kmod-sdhci-mt7620 \
		     uqmi kmod-usb-serial kmod-usb-serial-option
endef
TARGET_DEVICES += head-weblink_hdrm200

define Device/hiwifi_hc5661
  MTK_SOC := mt7620a
  IMAGE_SIZE := 15872k
  DEVICE_VENDOR := HiWiFi
  DEVICE_MODEL := HC5661
  DEVICE_PACKAGES := kmod-sdhci-mt7620
  SUPPORTED_DEVICES += hc5661
endef
TARGET_DEVICES += hiwifi_hc5661

define Device/hiwifi_hc5761
  MTK_SOC := mt7620a
  IMAGE_SIZE := 15872k
  DEVICE_VENDOR := HiWiFi
  DEVICE_MODEL := HC5761
  DEVICE_PACKAGES := kmod-mt76x0e kmod-usb2 kmod-usb-ohci kmod-sdhci-mt7620 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += hc5761
endef
TARGET_DEVICES += hiwifi_hc5761

define Device/hiwifi_hc5861
  MTK_SOC := mt7620a
  IMAGE_SIZE := 15872k
  DEVICE_VENDOR := HiWiFi
  DEVICE_MODEL := HC5861
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci kmod-sdhci-mt7620 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += hc5861
endef
TARGET_DEVICES += hiwifi_hc5861

define Device/hnet_c108
  MTK_SOC := mt7620a
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := HNET
  DEVICE_MODEL := C108
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-sdhci-mt7620
  SUPPORTED_DEVICES += c108
endef
TARGET_DEVICES += hnet_c108

define Device/iodata_wn-ac1167gr
  MTK_SOC := mt7620a
  DEVICE_VENDOR := I-O DATA
  DEVICE_MODEL := WN-AC1167GR
  IMAGE_SIZE := 6864k
  IMAGES += factory.bin
  IMAGE/factory.bin := \
    $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
    elx-header 01040016 8844A2D168B45A2D
  DEVICE_PACKAGES := kmod-mt76x2
endef
TARGET_DEVICES += iodata_wn-ac1167gr

define Device/iodata_wn-ac733gr3
  MTK_SOC := mt7620a
  DEVICE_VENDOR := I-O DATA
  DEVICE_MODEL := WN-AC733GR3
  IMAGE_SIZE := 6992k
  IMAGES += factory.bin
  IMAGE/factory.bin := \
    $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
    elx-header 01040006 8844A2D168B45A2D
  DEVICE_PACKAGES := kmod-mt76x0e kmod-switch-rtl8367b
endef
TARGET_DEVICES += iodata_wn-ac733gr3

define Device/kimax_u25awf-h1
  MTK_SOC := mt7620a
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Kimax
  DEVICE_MODEL := U25AWF
  DEVICE_VARIANT := H1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-storage kmod-scsi-core \
		     kmod-fs-ext4 kmod-fs-vfat block-mount
  SUPPORTED_DEVICES += u25awf-h1
endef
TARGET_DEVICES += kimax_u25awf-h1

define Device/kimax_u35wf
  MTK_SOC := mt7620n
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Kimax
  DEVICE_MODEL := U35WF
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-storage kmod-scsi-core \
		     kmod-fs-ext4 kmod-fs-vfat block-mount
endef
TARGET_DEVICES += kimax_u35wf

define Device/kingston_mlw221
  MTK_SOC := mt7620n
  IMAGE_SIZE := 15744k
  DEVICE_VENDOR := Kingston
  DEVICE_MODEL := MLW221
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += mlw221
endef
TARGET_DEVICES += kingston_mlw221

define Device/kingston_mlwg2
  MTK_SOC := mt7620n
  IMAGE_SIZE := 15744k
  DEVICE_VENDOR := Kingston
  DEVICE_MODEL := MLWG2
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += mlwg2
endef
TARGET_DEVICES += kingston_mlwg2

define Device/lava_lr-25g001
  $(Device/amit_jboot)
  MTK_SOC := mt7620a
  IMAGE_SIZE := 16256k
  DEVICE_VENDOR := LAVA
  DEVICE_MODEL := LR-25G001
  DLINK_ROM_ID := LVA6E3804001
  DLINK_FAMILY_MEMBER := 0x6E38
  DLINK_FIRMWARE_SIZE := 0xFE0000
  DEVICE_PACKAGES += kmod-mt76x0e
endef
TARGET_DEVICES += lava_lr-25g001

define Device/lenovo_newifi-y1
  MTK_SOC := mt7620a
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Lenovo
  DEVICE_MODEL := Y1
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += y1
endef
TARGET_DEVICES += lenovo_newifi-y1

define Device/lenovo_newifi-y1s
  MTK_SOC := mt7620a
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Lenovo
  DEVICE_MODEL := Y1S
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += y1s
endef
TARGET_DEVICES += lenovo_newifi-y1s

define Device/linksys_e1700
  MTK_SOC := mt7620a
  IMAGE_SIZE := 7872k
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	umedia-header 0x013326
  DEVICE_VENDOR := Linksys
  DEVICE_MODEL := E1700
  SUPPORTED_DEVICES += e1700
endef
TARGET_DEVICES += linksys_e1700

define Device/microduino_microwrt
  MTK_SOC := mt7620a
  IMAGE_SIZE := 16128k
  DEVICE_VENDOR := Microduino
  DEVICE_MODEL := MicroWRT
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += microwrt
endef
TARGET_DEVICES += microduino_microwrt

define Device/netgear_ex2700
  MTK_SOC := mt7620a
  NETGEAR_HW_ID := 29764623+4+0+32+2x2+0
  NETGEAR_BOARD_ID := EX2700
  BLOCKSIZE := 4k
  IMAGE_SIZE := 3776k
  IMAGES += factory.bin
  KERNEL := $(KERNEL_DTB) | uImage lzma | pad-offset 64k 64 | append-uImage-fakehdr filesystem
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	netgear-dni
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := EX2700
  SUPPORTED_DEVICES += ex2700
endef
TARGET_DEVICES += netgear_ex2700

define Device/netgear_ex3700
  MTK_SOC := mt7620a
  NETGEAR_BOARD_ID := U12H319T00_NETGEAR
  BLOCKSIZE := 4k
  IMAGE_SIZE := 7744k
  IMAGES += factory.chk
  IMAGE/factory.chk := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | netgear-chk
  DEVICE_PACKAGES := kmod-mt76x2
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := EX3700/EX3800
  SUPPORTED_DEVICES += ex3700
endef
TARGET_DEVICES += netgear_ex3700

define Device/netgear_ex6130
  MTK_SOC := mt7620a
  NETGEAR_BOARD_ID := U12H319T50_NETGEAR
  BLOCKSIZE := 4k
  IMAGE_SIZE := 7744k
  IMAGES += factory.chk
  IMAGE/factory.chk := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | netgear-chk
  DEVICE_PACKAGES := kmod-mt76x2
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := EX6130
endef
TARGET_DEVICES += netgear_ex6130

define Device/netgear_wn3000rp-v3
  MTK_SOC := mt7620a
  IMAGE_SIZE := 7872k
  NETGEAR_HW_ID := 29764836+8+0+32+2x2+0
  NETGEAR_BOARD_ID := WN3000RPv3
  BLOCKSIZE := 4k
  IMAGES += factory.bin
  KERNEL := $(KERNEL_DTB) | uImage lzma | pad-offset 64k 64 | append-uImage-fakehdr filesystem
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	netgear-dni
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := WN3000RP
  DEVICE_VARIANT := v3
  SUPPORTED_DEVICES += wn3000rpv3
endef
TARGET_DEVICES += netgear_wn3000rp-v3

define Device/nexx_wt3020-4m
  MTK_SOC := mt7620n
  BLOCKSIZE := 4k
  IMAGE_SIZE := 3776k
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	poray-header -B WT3020 -F 4M
  DEVICE_VENDOR := Nexx
  DEVICE_MODEL := WT3020
  DEVICE_VARIANT := 4M
  SUPPORTED_DEVICES += wt3020 wt3020-4M
endef
TARGET_DEVICES += nexx_wt3020-4m

define Device/nexx_wt3020-8m
  MTK_SOC := mt7620n
  IMAGE_SIZE := 7872k
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	poray-header -B WT3020 -F 8M
  DEVICE_VENDOR := Nexx
  DEVICE_MODEL := WT3020
  DEVICE_VARIANT := 8M
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += wt3020 wt3020-8M
endef
TARGET_DEVICES += nexx_wt3020-8m

define Device/ohyeah_oy-0001
  MTK_SOC := mt7620a
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Oh Yeah
  DEVICE_MODEL := OY-0001
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += oy-0001
endef
TARGET_DEVICES += ohyeah_oy-0001

define Device/phicomm_k2g
  MTK_SOC := mt7620a
  IMAGE_SIZE := 7552k
  DEVICE_VENDOR := Phicomm
  DEVICE_MODEL := K2G
  DEVICE_PACKAGES := kmod-mt76x2
endef
TARGET_DEVICES += phicomm_k2g

define Device/phicomm_psg1208
  MTK_SOC := mt7620a
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Phicomm
  DEVICE_MODEL := PSG1208
  DEVICE_PACKAGES := kmod-mt76x2
  SUPPORTED_DEVICES += psg1208
endef
TARGET_DEVICES += phicomm_psg1208

define Device/phicomm_psg1218a
  MTK_SOC := mt7620a
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Phicomm
  DEVICE_MODEL := PSG1218
  DEVICE_VARIANT:= Ax
  DEVICE_PACKAGES := kmod-mt76x2
  SUPPORTED_DEVICES += psg1218 psg1218a
endef
TARGET_DEVICES += phicomm_psg1218a

define Device/phicomm_psg1218b
  MTK_SOC := mt7620a
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Phicomm
  DEVICE_MODEL := PSG1218
  DEVICE_VARIANT := Bx
  DEVICE_PACKAGES := kmod-mt76x2
  SUPPORTED_DEVICES += psg1218 psg1218b
endef
TARGET_DEVICES += phicomm_psg1218b

define Device/planex_cs-qr10
  MTK_SOC := mt7620a
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Planex
  DEVICE_MODEL := CS-QR10
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci \
	kmod-sound-core kmod-sound-mt7620 \
	kmod-i2c-ralink kmod-sdhci-mt7620
  SUPPORTED_DEVICES += cs-qr10
endef
TARGET_DEVICES += planex_cs-qr10

define Device/planex_db-wrt01
  MTK_SOC := mt7620a
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Planex
  DEVICE_MODEL := DB-WRT01
  SUPPORTED_DEVICES += db-wrt01
endef
TARGET_DEVICES += planex_db-wrt01

define Device/planex_mzk-750dhp
  MTK_SOC := mt7620a
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Planex
  DEVICE_MODEL := MZK-750DHP
  DEVICE_PACKAGES := kmod-mt76x0e
  SUPPORTED_DEVICES += mzk-750dhp
endef
TARGET_DEVICES += planex_mzk-750dhp

define Device/planex_mzk-ex300np
  MTK_SOC := mt7620a
  IMAGE_SIZE := 7360k
  DEVICE_VENDOR := Planex
  DEVICE_MODEL := MZK-EX300NP
  SUPPORTED_DEVICES += mzk-ex300np
endef
TARGET_DEVICES += planex_mzk-ex300np

define Device/planex_mzk-ex750np
  MTK_SOC := mt7620a
  IMAGE_SIZE := 7360k
  DEVICE_VENDOR := Planex
  DEVICE_MODEL := MZK-EX750NP
  DEVICE_PACKAGES := kmod-mt76x2
  SUPPORTED_DEVICES += mzk-ex750np
endef
TARGET_DEVICES += planex_mzk-ex750np

define Device/ralink_mt7620a-evb
  MTK_SOC := mt7620a
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MT7620a EVB
endef
TARGET_DEVICES += ralink_mt7620a-evb

define Device/ralink_mt7620a-mt7530-evb
  MTK_SOC := mt7620a
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MT7620a + MT7530 EVB
  SUPPORTED_DEVICES += mt7620a_mt7530
endef
TARGET_DEVICES += ralink_mt7620a-mt7530-evb

define Device/ralink_mt7620a-mt7610e-evb
  MTK_SOC := mt7620a
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MT7620a + MT7610e EVB
  DEVICE_PACKAGES := kmod-mt76x0e
  SUPPORTED_DEVICES += mt7620a_mt7610e
endef
TARGET_DEVICES += ralink_mt7620a-mt7610e-evb

define Device/ralink_mt7620a-v22sg-evb
  MTK_SOC := mt7620a
  IMAGE_SIZE := 130560k
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MT7620a V22SG
  SUPPORTED_DEVICES += mt7620a_v22sg
endef
TARGET_DEVICES += ralink_mt7620a-v22sg-evb

define Device/ravpower_wd03
  MTK_SOC := mt7620n
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Ravpower
  DEVICE_MODEL := WD03
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += ravpower_wd03

define Device/sanlinking_d240
  MTK_SOC := mt7620a
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Sanlinking Technologies
  DEVICE_MODEL := D240
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci kmod-sdhci-mt7620
  SUPPORTED_DEVICES += d240
endef
TARGET_DEVICES += sanlinking_d240

define Device/sercomm_na930
  MTK_SOC := mt7620a
  IMAGE_SIZE := 20480k
  DEVICE_VENDOR := Sercomm
  DEVICE_MODEL := NA930
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += na930
endef
TARGET_DEVICES += sercomm_na930

define Device/tplink_archer-c20i
  $(Device/Archer)
  IMAGE_SIZE := 7808k
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0xc2000001
  TPLINK_HWREV := 58
  IMAGES += factory.bin
  DEVICE_PACKAGES := kmod-mt76x0e
  DEVICE_MODEL := Archer C20i
  DEVICE_PACKAGES := kmod-mt76x0e kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += c20i
endef
TARGET_DEVICES += tplink_archer-c20i

define Device/tplink_archer-c20-v1
  $(Device/Archer)
  IMAGE_SIZE := 7808k
  SUPPORTED_DEVICES += tplink,c20-v1
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0xc2000001
  TPLINK_HWREV := 0x44
  TPLINK_HWREVADD := 0x1
  IMAGES += factory.bin
  DEVICE_MODEL := Archer C20
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-mt76x0e kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += tplink_archer-c20-v1

define Device/tplink_archer-c2-v1
  $(Device/Archer)
  IMAGE_SIZE := 7808k
  SUPPORTED_DEVICES += tplink,c2-v1
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0xc7500001
  TPLINK_HWREV := 50
  IMAGES += factory.bin
  DEVICE_MODEL := Archer C2
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-mt76x0e kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport kmod-switch-rtl8366-smi kmod-switch-rtl8367b
endef
TARGET_DEVICES += tplink_archer-c2-v1

define Device/tplink_archer-c50-v1
  $(Device/Archer)
  IMAGE_SIZE := 7808k
  TPLINK_FLASHLAYOUT := 8Mmtk
  TPLINK_HWID := 0xc7500001
  TPLINK_HWREV := 69
  IMAGES += factory-us.bin factory-eu.bin
  IMAGE/factory-us.bin := tplink-v2-image -e -w 0
  IMAGE/factory-eu.bin := tplink-v2-image -e -w 2
  DEVICE_MODEL := Archer C50
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += c50
endef
TARGET_DEVICES += tplink_archer-c50-v1

define Device/tplink_archer-mr200
  $(Device/Archer)
  IMAGE_SIZE := 7872k
  TPLINK_FLASHLAYOUT := 8MLmtk
  TPLINK_HWID := 0xd7500001
  TPLINK_HWREV := 0x4a
  DEVICE_PACKAGES := kmod-mt76x0e kmod-usb2 kmod-usb-net kmod-usb-net-rndis kmod-usb-serial kmod-usb-serial-option adb-enablemodem
  DEVICE_MODEL := Archer MR200
  SUPPORTED_DEVICES += mr200
endef
TARGET_DEVICES += tplink_archer-mr200

define Device/vonets_var11n-300
  MTK_SOC := mt7620n
  IMAGE_SIZE := 3776k
  BLOCKSIZE := 4k
  DEVICE_VENDOR := Vonets
  DEVICE_MODEL := VAR11N-300
endef
TARGET_DEVICES += vonets_var11n-300

define Device/wrtnode_wrtnode
  MTK_SOC := mt7620n
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := WRTNode
  DEVICE_MODEL := WRTNode
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += wrtnode
endef
TARGET_DEVICES += wrtnode_wrtnode

define Device/xiaomi_miwifi-mini
  MTK_SOC := mt7620a
  IMAGE_SIZE := 15872k
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := MiWiFi Mini
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += miwifi-mini
endef
TARGET_DEVICES += xiaomi_miwifi-mini

define Device/youku_yk1
  MTK_SOC := mt7620a
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := YOUKU
  DEVICE_MODEL := YK1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-sdhci-mt7620 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += youku-yk1
endef
TARGET_DEVICES += youku_yk1

define Device/yukai_bocco
  MTK_SOC := mt7620a
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := YUKAI Engineering
  DEVICE_MODEL := BOCCO
  DEVICE_PACKAGES := kmod-sound-core kmod-sound-mt7620 kmod-i2c-ralink
  SUPPORTED_DEVICES += bocco
endef
TARGET_DEVICES += yukai_bocco

define Device/zbtlink_zbt-ape522ii
  MTK_SOC := mt7620a
  IMAGE_SIZE := 15872k
  DEVICE_VENDOR := Zbtlink
  DEVICE_MODEL := ZBT-APE522II
  DEVICE_PACKAGES := kmod-mt76x2
  SUPPORTED_DEVICES += zbt-ape522ii
endef
TARGET_DEVICES += zbtlink_zbt-ape522ii

define Device/zbtlink_zbt-cpe102
  MTK_SOC := mt7620n
  IMAGE_SIZE := 7552k
  DEVICE_VENDOR := Zbtlink
  DEVICE_MODEL := ZBT-CPE102
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += zbt-cpe102
endef
TARGET_DEVICES += zbtlink_zbt-cpe102

define Device/zbtlink_zbt-wa05
  MTK_SOC := mt7620n
  IMAGE_SIZE := 7552k
  DEVICE_VENDOR := Zbtlink
  DEVICE_MODEL := ZBT-WA05
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += zbt-wa05
endef
TARGET_DEVICES += zbtlink_zbt-wa05

define Device/zbtlink_zbt-we1026-5g-16m
  MTK_SOC := mt7620a
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Zbtlink
  DEVICE_MODEL := ZBT-WE1026-5G
  DEVICE_VARIANT := 16M
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci kmod-sdhci-mt7620
  SUPPORTED_DEVICES += we1026-5g-16m zbtlink,we1026-5g-16m
endef
TARGET_DEVICES += zbtlink_zbt-we1026-5g-16m

define Device/zbtlink_zbt-we1026-h-32m
  MTK_SOC := mt7620a
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := Zbtlink
  DEVICE_MODEL := ZBT-WE1026-H
  DEVICE_VARIANT := 32M
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-sdhci-mt7620 \
	kmod-ledtrig-netdev
endef
TARGET_DEVICES += zbtlink_zbt-we1026-h-32m

define Device/zbtlink_zbt-we2026
  MTK_SOC := mt7620n
  IMAGE_SIZE := 7552k
  DEVICE_VENDOR := Zbtlink
  DEVICE_MODEL := ZBT-WE2026
  SUPPORTED_DEVICES += zbt-we2026
endef
TARGET_DEVICES += zbtlink_zbt-we2026

define Device/zbtlink_zbt-we826-16m
  MTK_SOC := mt7620a
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Zbtlink
  DEVICE_MODEL := ZBT-WE826
  DEVICE_VARIANT := 16M
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci kmod-sdhci-mt7620
  SUPPORTED_DEVICES += zbt-we826 zbt-we826-16M
endef
TARGET_DEVICES += zbtlink_zbt-we826-16m

define Device/zbtlink_zbt-we826-32m
  MTK_SOC := mt7620a
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := Zbtlink
  DEVICE_MODEL := ZBT-WE826
  DEVICE_VARIANT := 32M
  DEVICE_PACKAGES := kmod-mt76x2 kmod-usb2 kmod-usb-ohci kmod-sdhci-mt7620
  SUPPORTED_DEVICES += zbt-we826-32M
endef
TARGET_DEVICES += zbtlink_zbt-we826-32m

define Device/zbtlink_zbt-we826-e
  MTK_SOC := mt7620a
  IMAGE_SIZE := 32448k
  DEVICE_VENDOR := Zbtlink
  DEVICE_MODEL := ZBT-WE826-E
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-sdhci-mt7620 uqmi \
		     kmod-usb-serial kmod-usb-serial-option
endef
TARGET_DEVICES += zbtlink_zbt-we826-e

define Device/zbtlink_zbt-wr8305rt
  MTK_SOC := mt7620n
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Zbtlink
  DEVICE_MODEL := ZBT-WR8305RT
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += zbt-wr8305rt
endef
TARGET_DEVICES += zbtlink_zbt-wr8305rt

define Device/zte_q7
  MTK_SOC := mt7620a
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := ZTE
  DEVICE_MODEL := Q7
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
  SUPPORTED_DEVICES += zte-q7
endef
TARGET_DEVICES += zte_q7

define Device/zyxel_keenetic-omni
  MTK_SOC := mt7620n
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := ZyXEL
  DEVICE_MODEL := Keenetic Omni
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(IMAGE/sysupgrade.bin) | pad-to 64k | check-size $$$$(IMAGE_SIZE) | \
	zyimage -d 4882 -v "ZyXEL Keenetic Omni"
  SUPPORTED_DEVICES += kn_rc
endef
TARGET_DEVICES += zyxel_keenetic-omni

define Device/zyxel_keenetic-omni-ii
  MTK_SOC := mt7620n
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := ZyXEL
  DEVICE_MODEL := Keenetic Omni II
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(IMAGE/sysupgrade.bin) | pad-to 64k | check-size $$$$(IMAGE_SIZE) | \
	zyimage -d 2102034 -v "ZyXEL Keenetic Omni II"
  SUPPORTED_DEVICES += kn_rf
endef
TARGET_DEVICES += zyxel_keenetic-omni-ii

define Device/zyxel_keenetic-viva
  MTK_SOC := mt7620a
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := ZyXEL
  DEVICE_MODEL := Keenetic Viva
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport kmod-switch-rtl8366-smi kmod-switch-rtl8367b
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | pad-to 64k | check-size $$$$(IMAGE_SIZE) | \
	zyimage -d 8997 -v "ZyXEL Keenetic Viva"
  SUPPORTED_DEVICES += kng_rc
endef
TARGET_DEVICES += zyxel_keenetic-viva
