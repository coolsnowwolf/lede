
#
# BCM33XX/BCM63XX Profiles
#

DEVICE_VARS += HCS_MAGIC_BYTES HCS_REV_MIN HCS_REV_MAJ
DEVICE_VARS += BLOCK_SIZE FLASH_MB IMAGE_OFFSET
DEVICE_VARS += CFE_BOARD_ID CFE_EXTRAS
DEVICE_VARS += NETGEAR_BOARD_ID NETGEAR_REGION
DEVICE_VARS += REDBOOT_PREFIX

define Device/bcm33xx
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | loader-lzma bin | hcs-initramfs
  IMAGES :=
  HCS_MAGIC_BYTES :=
  HCS_REV_MIN :=
  HCS_REV_MAJ :=
endef

define Device/bcm63xx
  FILESYSTEMS := squashfs jffs2-64k jffs2-128k
  KERNEL := kernel-bin | append-dtb | relocate-kernel | lzma
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | loader-lzma elf
  IMAGES := cfe.bin
  IMAGE/cfe.bin := cfe-bin --pad $$$$(shell expr $$$$(FLASH_MB) / 2)
  IMAGE/cfe-4M.bin := cfe-bin --pad 2
  IMAGE/cfe-8M.bin := cfe-bin --pad 4
  IMAGE/cfe-16M.bin := cfe-bin --pad 8
  IMAGE/cfe-bc221.bin := cfe-bin --layoutver 5
  IMAGE/cfe-old.bin := cfe-old-bin
  IMAGE/sysupgrade.bin := cfe-bin
  BLOCK_SIZE := 0x10000
  IMAGE_OFFSET :=
  FLASH_MB := 4
  CFE_BOARD_ID :=
  CFE_EXTRAS = --block-size $$(BLOCK_SIZE) --image-offset $$(if $$(IMAGE_OFFSET),$$(IMAGE_OFFSET),$$(BLOCK_SIZE))
endef

define Device/bcm63xx-legacy
  $(Device/bcm63xx)
  KERNEL := kernel-bin | append-dtb | relocate-kernel | lzma-cfe
endef

define Device/bcm63xx_netgear
  $(Device/bcm63xx)
  DEVICE_VENDOR := NETGEAR
  IMAGES := factory.chk sysupgrade.bin
  IMAGE/factory.chk := cfe-bin | netgear-chk
  NETGEAR_BOARD_ID :=
  NETGEAR_REGION :=
endef

define Device/bcm63xx_redboot
  FILESYSTEMS := squashfs
  KERNEL := kernel-bin | append-dtb | relocate-kernel | gzip
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | loader-lzma elf
  IMAGES := redboot.bin
  IMAGE/redboot.bin := redboot-bin
  REDBOOT_PREFIX := $$(IMAGE_PREFIX)
endef

### Generic ###
define Device/brcm_bcm963281tan
  $(Device/bcm63xx)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 963281TAN
  IMAGES := cfe-4M.bin cfe-8M.bin cfe-16M.bin
  CFE_BOARD_ID := 963281TAN
  CHIP_ID := 6328
endef
TARGET_DEVICES += brcm_bcm963281tan

define Device/brcm_bcm96328avng
  $(Device/bcm63xx)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96328avng
  IMAGES := cfe-4M.bin cfe-8M.bin cfe-16M.bin
  CFE_BOARD_ID := 96328avng
  CHIP_ID := 6328
endef
TARGET_DEVICES += brcm_bcm96328avng

define Device/brcm_bcm96338gw
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96338GW
  CFE_BOARD_ID := 6338GW
  CHIP_ID := 6338
endef
TARGET_DEVICES += brcm_bcm96338gw

define Device/brcm_bcm96338w
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96338W
  CFE_BOARD_ID := 6338W
  CHIP_ID := 6338
  DEFAULT := n
endef
TARGET_DEVICES += brcm_bcm96338w

define Device/brcm_bcm96345gw2
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96345GW2
  IMAGES += cfe-bc221.bin
  CFE_BOARD_ID := 96345GW2
  CHIP_ID := 6345
  DEFAULT := n
endef
TARGET_DEVICES += brcm_bcm96345gw2

define Device/brcm_bcm96348gw
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96348GW
  IMAGES += cfe-bc221.bin
  CFE_BOARD_ID := 96348GW
  CHIP_ID := 6348
  DEFAULT := n
endef
TARGET_DEVICES += brcm_bcm96348gw

define Device/brcm_bcm96348gw-10
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96348GW-10
  CFE_BOARD_ID := 96348GW-10
  CHIP_ID := 6348
  DEFAULT := n
endef
TARGET_DEVICES += brcm_bcm96348gw-10

define Device/brcm_bcm96348gw-11
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96348GW-11
  CFE_BOARD_ID := 96348GW-11
  CHIP_ID := 6348
  DEFAULT := n
endef
TARGET_DEVICES += brcm_bcm96348gw-11

define Device/brcm_bcm96348r
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96348R
  CFE_BOARD_ID := 96348R
  CHIP_ID := 6348
  DEFAULT := n
endef
TARGET_DEVICES += brcm_bcm96348r

define Device/brcm_bcm96358vw
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96358VW
  CFE_BOARD_ID := 96358VW
  CHIP_ID := 6358
endef
TARGET_DEVICES += brcm_bcm96358vw

define Device/brcm_bcm96358vw2
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96358VW2
  CFE_BOARD_ID := 96358VW2
  CHIP_ID := 6358
endef
TARGET_DEVICES += brcm_bcm96358vw2

define Device/brcm_bcm96368mvngr
  $(Device/bcm63xx)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96368MVNgr
  CFE_BOARD_ID := 96368MVNgr
  CHIP_ID := 6368
endef
TARGET_DEVICES += brcm_bcm96368mvngr

define Device/brcm_bcm96368mvwg
  $(Device/bcm63xx)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96368MVWG
  CFE_BOARD_ID := 96368MVWG
  CHIP_ID := 6368
endef
TARGET_DEVICES += brcm_bcm96368mvwg

### Actiontec ###
define Device/actiontec_r1000h
  $(Device/bcm63xx)
  DEVICE_VENDOR := Actiontec
  DEVICE_MODEL := R1000H
  FILESYSTEMS := squashfs
  CFE_BOARD_ID := 96368MVWG
  CHIP_ID := 6368
  FLASH_MB := 32
  IMAGE_OFFSET := 0x20000
  DEVICE_PACKAGES := $(USB2_PACKAGES) $(BRCMWL_PACKAGES)
endef
TARGET_DEVICES += actiontec_r1000h

### ADB ###
define Device/adb_a4001n
  $(Device/bcm63xx)
  DEVICE_VENDOR := ADB
  DEVICE_MODEL := P.DG A4001N
  CFE_BOARD_ID := 96328dg2x2
  CHIP_ID := 6328
  FLASH_MB := 8
  DEVICE_PACKAGES := $(USB2_PACKAGES) $(B43_PACKAGES)
endef
TARGET_DEVICES += adb_a4001n

define Device/adb_a4001n1
  $(Device/bcm63xx)
  DEVICE_VENDOR := ADB
  DEVICE_MODEL := P.DG A4001N1
  IMAGES += sysupgrade.bin
  CFE_BOARD_ID := 963281T_TEF
  CHIP_ID := 6328
  FLASH_MB := 16
  DEVICE_PACKAGES := $(USB2_PACKAGES) $(B43_PACKAGES)
endef
TARGET_DEVICES += adb_a4001n1

define Device/adb_pdg-a4001n-a-000-1a1-ax
  $(Device/bcm63xx)
  DEVICE_VENDOR := ADB
  DEVICE_MODEL := P.DG A4001N A-000-1A1-AX
  IMAGES += sysupgrade.bin
  CFE_BOARD_ID := 96328avng
  CHIP_ID := 6328
  FLASH_MB := 16
  DEVICE_PACKAGES := $(USB2_PACKAGES) $(B43_PACKAGES)
endef
TARGET_DEVICES += adb_pdg-a4001n-a-000-1a1-ax

define Device/adb_pdg-a4101n-a-000-1a1-ae
  $(Device/bcm63xx)
  DEVICE_VENDOR := ADB
  DEVICE_MODEL := P.DG A4101N A-000-1A1-AE
  IMAGES += sysupgrade.bin
  CFE_BOARD_ID := 96328avngv
  CHIP_ID := 6328
  FLASH_MB := 16
  DEVICE_PACKAGES := $(USB2_PACKAGES) $(B43_PACKAGES)
endef
TARGET_DEVICES += adb_pdg-a4101n-a-000-1a1-ae

define Device/adb_av4202n
  $(Device/bcm63xx)
  DEVICE_VENDOR := ADB
  DEVICE_MODEL := P.DG AV4202N
  IMAGE_OFFSET := 0x20000
  CFE_BOARD_ID := 96368_Swiss_S1
  CHIP_ID := 6368
  DEVICE_PACKAGES := $(USB2_PACKAGES) $(B43_PACKAGES)
endef
TARGET_DEVICES += adb_av4202n

### Alcatel ###
define Device/alcatel_rg100a
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Alcatel
  DEVICE_MODEL := RG100A
  CFE_BOARD_ID := 96358VW2
  CHIP_ID := 6358
  BLOCK_SIZE := 0x20000
  DEVICE_PACKAGES := $(USB2_PACKAGES) $(B43_PACKAGES)
endef
TARGET_DEVICES += alcatel_rg100a

### Asmax ###
define Device/asmax_ar-1004g
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Asmax
  DEVICE_MODEL := AR 1004g
  CFE_BOARD_ID := 96348GW-10
  CHIP_ID := 6348
  DEVICE_PACKAGES := $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += asmax_ar-1004g

### Belkin ###
define Device/belkin_f5d7633
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Belkin
  DEVICE_MODEL := F5D7633
  CFE_BOARD_ID := 96348GW-10
  CHIP_ID := 6348
  BLOCK_SIZE := 0x20000
  DEVICE_PACKAGES := $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += belkin_f5d7633

### Broadcom ###
define Device/brcm_bcm96318ref
  $(Device/bcm63xx)
  DEVICE_VENDOR := Broadcom
  DEVICE_MODEL := BCM96318REF reference board
  IMAGES :=
  CFE_BOARD_ID := 96318REF
  CHIP_ID := 6318
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES) kmod-bcm63xx-udc
endef
TARGET_DEVICES += brcm_bcm96318ref

define Device/brcm_bcm96318ref-p300
  $(Device/bcm63xx)
  DEVICE_VENDOR := Broadcom
  DEVICE_MODEL := BCM96318REF_P300 reference board
  IMAGES :=
  CFE_BOARD_ID := 96318REF_P300
  CHIP_ID := 6318
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES) kmod-bcm63xx-udc
endef
TARGET_DEVICES += brcm_bcm96318ref-p300

define Device/brcm_bcm963268bu-p300
  $(Device/bcm63xx)
  DEVICE_VENDOR := Broadcom
  DEVICE_MODEL := BCM963268BU_P300 reference board
  IMAGES :=
  CFE_BOARD_ID := 963268BU_P300
  CHIP_ID := 63268
  DEVICE_PACKAGES := $(USB2_PACKAGES) kmod-bcm63xx-udc
endef
TARGET_DEVICES += brcm_bcm963268bu-p300

define Device/brcm_bcm963269bhr
  $(Device/bcm63xx)
  DEVICE_VENDOR := Broadcom
  DEVICE_MODEL := BCM963269BHR reference board
  IMAGES :=
  CFE_BOARD_ID := 963269BHR
  CHIP_ID := 63268
  SOC := bcm63269
  DEVICE_PACKAGES := $(USB2_PACKAGES) kmod-bcm63xx-udc
endef
TARGET_DEVICES += brcm_bcm963269bhr

### BT ###
define Device/bt_home-hub-2-a
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := BT
  DEVICE_MODEL := Home Hub 2.0
  DEVICE_VARIANT := A
  CFE_BOARD_ID := HOMEHUB2A
  CHIP_ID := 6358
  BLOCK_SIZE := 0x20000
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += bt_home-hub-2-a

define Device/bt_voyager-2110
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := BT
  DEVICE_MODEL := Voyager 2110
  CFE_BOARD_ID := V2110
  CHIP_ID := 6348
  CFE_EXTRAS += --layoutver 5
  DEVICE_PACKAGES := $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += bt_voyager-2110

define Device/bt_voyager-2500v-bb
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := BT
  DEVICE_MODEL := Voyager 2500V
  CFE_BOARD_ID := V2500V_BB
  CHIP_ID := 6348
  CFE_EXTRAS += --layoutver 5
  DEVICE_PACKAGES := $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += bt_voyager-2500v-bb

### Comtrend ###
define Device/comtrend_ar-5315u
  $(Device/bcm63xx)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := AR-5315u
  IMAGES += sysupgrade.bin
  CFE_BOARD_ID := 96318A-1441N1
  CHIP_ID := 6318
  FLASH_MB := 16
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += comtrend_ar-5315u

define Device/comtrend_ar-5381u
  $(Device/bcm63xx)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := AR-5381u
  IMAGES += sysupgrade.bin
  CFE_BOARD_ID := 96328A-1241N
  CHIP_ID := 6328
  FLASH_MB := 16
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += comtrend_ar-5381u

define Device/comtrend_ar-5387un
  $(Device/bcm63xx)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := AR-5387un
  IMAGES += sysupgrade.bin
  CFE_BOARD_ID := 96328A-1441N1
  CHIP_ID := 6328
  FLASH_MB := 16
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += comtrend_ar-5387un

define Device/comtrend_ct-536plus
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := CT-536+
  DEVICE_ALT0_VENDOR := Comtrend
  DEVICE_ALT0_MODEL := CT-5621
  CFE_BOARD_ID := 96348GW-11
  CHIP_ID := 6348
  DEVICE_PACKAGES := $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += comtrend_ct-536plus

define Device/comtrend_ct-5365
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := CT-5365
  CFE_BOARD_ID := 96348A-122
  CHIP_ID := 6348
  DEVICE_PACKAGES := $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += comtrend_ct-5365

define Device/comtrend_ct-6373
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := CT-6373
  CFE_BOARD_ID := CT6373-1
  CHIP_ID := 6358
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += comtrend_ct-6373

define Device/comtrend_vr-3025u
  $(Device/bcm63xx)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := VR-3025u
  IMAGES += sysupgrade.bin
  CFE_BOARD_ID := 96368M-1541N
  CHIP_ID := 6368
  BLOCK_SIZE := 0x20000
  FLASH_MB := 32
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += comtrend_vr-3025u

define Device/comtrend_vr-3025un
  $(Device/bcm63xx)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := VR-3025un
  CFE_BOARD_ID := 96368M-1341N
  CHIP_ID := 6368
  FLASH_MB := 8
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += comtrend_vr-3025un

define Device/comtrend_vr-3026e
  $(Device/bcm63xx)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := VR-3026e
  CFE_BOARD_ID := 96368MT-1341N1
  CHIP_ID := 6368
  FLASH_MB := 8
  DEVICE_PACKAGES := $(B43_PACKAGES)
endef
TARGET_DEVICES += comtrend_vr-3026e

define Device/comtrend_wap-5813n
  $(Device/bcm63xx)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := WAP-5813n
  CFE_BOARD_ID := 96369R-1231N
  CHIP_ID := 6368
  FLASH_MB := 8
  SOC := bcm6369
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += comtrend_wap-5813n

### D-Link ###
define Device/d-link_dsl-2640b-b
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DSL-2640B
  DEVICE_VARIANT := B2
  CFE_BOARD_ID := D-4P-W
  CHIP_ID := 6348
  DEVICE_PACKAGES := $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += d-link_dsl-2640b-b

define Device/d-link_dsl-2640u
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DSL-2640U
  DEVICE_VARIANT := C1
  DEVICE_ALT0_VENDOR := D-Link
  DEVICE_ALT0_MODEL := DSL-2640U/BRU/C
  CFE_BOARD_ID := 96338W2_E7T
  CHIP_ID := 6338
  DEVICE_PACKAGES := $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += d-link_dsl-2640u

define Device/d-link_dsl-2650u
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DSL-2650U
  CFE_BOARD_ID := 96358VW2
  CHIP_ID := 6358
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += d-link_dsl-2650u

define Device/d-link_dsl-274xb-c2
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DSL-2740B
  DEVICE_VARIANT := C2
  DEVICE_ALT0_VENDOR := D-Link
  DEVICE_ALT0_MODEL := DSL-2741B
  DEVICE_ALT0_VARIANT := C2
  CFE_BOARD_ID := 96358GW
  CHIP_ID := 6358
  DEVICE_PACKAGES := $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += d-link_dsl-274xb-c2

define Device/d-link_dsl-274xb-c3
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DSL-2740B
  DEVICE_VARIANT := C3
  DEVICE_ALT0_VENDOR := D-Link
  DEVICE_ALT0_MODEL := DSL-2741B
  DEVICE_ALT0_VARIANT := C3
  DEVICE_DTS := bcm6358-d-link-dsl-274xb-c2
  CFE_BOARD_ID := AW4139
  CHIP_ID := 6358
  DEVICE_PACKAGES := $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += d-link_dsl-274xb-c3

define Device/d-link_dsl-274xb-f1
  $(Device/bcm63xx)
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DSL-2740B
  DEVICE_VARIANT := F1
  DEVICE_ALT0_VENDOR := D-Link
  DEVICE_ALT0_MODEL := DSL-2741B
  DEVICE_ALT0_VARIANT := F1
  CFE_BOARD_ID := AW4339U
  CHIP_ID := 6328
  IMAGES := cfe-EU.bin cfe-AU.bin
  IMAGE/cfe-AU.bin := cfe-bin --signature2 "4.06.01.AUF1" --pad 4
  IMAGE/cfe-EU.bin := cfe-bin --signature2 "4.06.01.EUF1" --pad 4
  DEVICE_PACKAGES := $(ATH9K_PACKAGES)
endef
TARGET_DEVICES += d-link_dsl-274xb-f1

define Device/d-link_dsl-2750u-c1
  $(Device/bcm63xx)
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DSL-2750U
  DEVICE_VARIANT := C1
  IMAGES += sysupgrade.bin
  CFE_BOARD_ID := 963281TAVNG
  CHIP_ID := 6328
  FLASH_MB := 8
  DEVICE_PACKAGES := $(USB2_PACKAGES) $(B43_PACKAGES)
endef
TARGET_DEVICES += d-link_dsl-2750u-c1

define Device/d-link_dsl-275xb-d1
  $(Device/bcm63xx)
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DSL-2750B
  DEVICE_VARIANT := D1
  DEVICE_ALT0_VENDOR := D-Link
  DEVICE_ALT0_MODEL := DSL-2751
  DEVICE_ALT0_VARIANT := D1
  CFE_BOARD_ID := AW5200B
  CHIP_ID := 6318
  FLASH_MB := 8
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += d-link_dsl-275xb-d1

define Device/d-link_dva-g3810bn-tl
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DVA-G3810BN/TL
  CFE_BOARD_ID := 96358VW
  CHIP_ID := 6358
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += d-link_dva-g3810bn-tl

### Davolink ###
define Device/davolink_dv-201amr
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Davolink
  DEVICE_MODEL := DV-201AMR
  IMAGES := cfe-old.bin
  CFE_BOARD_ID := DV201AMR
  CHIP_ID := 6348
  DEVICE_PACKAGES := $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += davolink_dv-201amr

### Dynalink ###
define Device/dynalink_rta770bw
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Dynalink
  DEVICE_MODEL := RTA770BW
  DEVICE_ALT0_VENDOR := Siemens
  DEVICE_ALT0_MODEL := SE515
  IMAGES =
  CFE_BOARD_ID := RTA770BW
  CHIP_ID := 6345
  CFE_EXTRAS += --layoutver 5
  DEVICE_PACKAGES := $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += dynalink_rta770bw

define Device/dynalink_rta770w
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Dynalink
  DEVICE_MODEL := RTA770W
  IMAGES =
  CFE_BOARD_ID := RTA770W
  CHIP_ID := 6345
  CFE_EXTRAS += --layoutver 5
  DEVICE_PACKAGES := $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += dynalink_rta770w

define Device/dynalink_rta1025w
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Dynalink
  DEVICE_MODEL := RTA1025W
  CFE_BOARD_ID := RTA1025W_16
  CHIP_ID := 6348
  CFE_EXTRAS += --layoutver 5
  DEVICE_PACKAGES := $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += dynalink_rta1025w

define Device/dynalink_rta1320
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Dynalink
  DEVICE_MODEL := RTA1320
  CFE_BOARD_ID := RTA1320_16M
  CHIP_ID := 6338
  CFE_EXTRAS += --layoutver 5
  DEFAULT := n
endef
TARGET_DEVICES += dynalink_rta1320

### Huawei ###
define Device/huawei_echolife-hg520v
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Huawei
  DEVICE_MODEL := EchoLife HG520v
  CFE_BOARD_ID := HW6358GW_B
  CHIP_ID := 6358
  CFE_EXTRAS += --rsa-signature "EchoLife_HG520v"
  SOC := bcm6359
  DEVICE_PACKAGES := $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += huawei_echolife-hg520v

define Device/huawei_echolife-hg553
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Huawei
  DEVICE_MODEL := EchoLife HG553
  CFE_BOARD_ID := HW553
  CHIP_ID := 6358
  CFE_EXTRAS += --rsa-signature "EchoLife_HG553" --tag-version 7
  BLOCK_SIZE := 0x20000
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += huawei_echolife-hg553

define Device/huawei_echolife-hg556a-a
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Huawei
  DEVICE_MODEL := EchoLife HG556a
  DEVICE_VARIANT := A
  DEVICE_DESCRIPTION = Build firmware images for Huawei HG556a version A (Atheros)
  CFE_BOARD_ID := HW556
  CHIP_ID := 6358
  CFE_EXTRAS += --rsa-signature "EchoLife_HG556a" --tag-version 8
  IMAGE_OFFSET := 0x20000
  DEVICE_PACKAGES := $(ATH9K_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += huawei_echolife-hg556a-a

define Device/huawei_echolife-hg556a-b
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Huawei
  DEVICE_MODEL := EchoLife HG556a
  DEVICE_VARIANT := B
  DEVICE_DESCRIPTION = Build firmware images for Huawei HG556a version B (Atheros)
  CFE_BOARD_ID := HW556
  CHIP_ID := 6358
  CFE_EXTRAS += --rsa-signature "EchoLife_HG556a" --tag-version 8
  BLOCK_SIZE := 0x20000
  DEVICE_PACKAGES := $(ATH9K_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += huawei_echolife-hg556a-b

define Device/huawei_echolife-hg556a-c
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Huawei
  DEVICE_MODEL := EchoLife HG556a
  DEVICE_VARIANT := C
  DEVICE_DESCRIPTION = Build firmware images for Huawei HG556a version C (Ralink)
  CFE_BOARD_ID := HW556
  CHIP_ID := 6358
  CFE_EXTRAS += --rsa-signature "EchoLife_HG556a" --tag-version 8
  BLOCK_SIZE := 0x20000
  DEVICE_PACKAGES := $(RT28_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += huawei_echolife-hg556a-c

define Device/huawei_echolife-hg622
  $(Device/bcm63xx)
  DEVICE_VENDOR := Huawei
  DEVICE_MODEL := EchoLife HG622
  IMAGES += sysupgrade.bin
  CFE_BOARD_ID := 96368MVWG_hg622
  CHIP_ID := 6368
  CFE_EXTRAS += --tag-version 7
  BLOCK_SIZE := 0x20000
  FLASH_MB := 16
  DEVICE_PACKAGES := $(RT28_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += huawei_echolife-hg622

define Device/huawei_echolife-hg655b
  $(Device/bcm63xx)
  DEVICE_VENDOR := Huawei
  DEVICE_MODEL := EchoLife HG655b
  CFE_BOARD_ID := HW65x
  CHIP_ID := 6368
  CFE_EXTRAS += --tag-version 7
  IMAGE_OFFSET := 0x20000
  FLASH_MB := 8
  DEVICE_PACKAGES := $(RT28_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += huawei_echolife-hg655b

### Innacomm ###
define Device/innacomm_w3400v6
  $(Device/bcm63xx)
  DEVICE_VENDOR := Innacomm
  DEVICE_MODEL := W3400V6
  CFE_BOARD_ID := 96328ang
  CHIP_ID := 6328
  FLASH_MB := 8
  DEVICE_PACKAGES := $(B43_PACKAGES)
endef
TARGET_DEVICES += innacomm_w3400v6

### Inteno ###
define Device/inteno_vg50
  $(Device/bcm63xx)
  DEVICE_VENDOR := Inteno
  DEVICE_MODEL := VG50 Multi-WAN CPE
  IMAGES :=
  CFE_BOARD_ID := VW6339GU
  CHIP_ID := 63268
  DEVICE_PACKAGES := $(USB2_PACKAGES)
endef
TARGET_DEVICES += inteno_vg50

### Inventel ###
define Device/inventel_livebox-1
  $(Device/bcm63xx_redboot)
  DEVICE_VENDOR := Inventel
  DEVICE_MODEL := Livebox 1
  SOC := bcm6348
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB1_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += inventel_livebox-1

### Netgear ###
define Device/netgear_cvg834g
  $(Device/bcm33xx)
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := CVG834G
  CHIP_ID := 3368
  HCS_MAGIC_BYTES := 0xa020
  HCS_REV_MIN := 0001
  HCS_REV_MAJ := 0022
endef
TARGET_DEVICES += netgear_cvg834g

define Device/netgear_dg834gt-pn
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := DG834GT
  DEVICE_ALT0_VENDOR := NETGEAR
  DEVICE_ALT0_MODEL := DG834PN
  CFE_BOARD_ID := 96348GW-10
  CHIP_ID := 6348
  DEVICE_PACKAGES := $(ATH5K_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += netgear_dg834gt-pn

define Device/netgear_dg834g-v4
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := DG834G
  DEVICE_VARIANT := v4
  IMAGES :=
  CFE_BOARD_ID := 96348W3
  CHIP_ID := 6348
  DEVICE_PACKAGES := $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += netgear_dg834g-v4

define Device/netgear_dgnd3700-v1
  $(Device/bcm63xx_netgear)
  DEVICE_MODEL := DGND3700
  DEVICE_VARIANT := v1
  CFE_BOARD_ID := 96368MVWG
  CHIP_ID := 6368
  BLOCK_SIZE := 0x20000
  NETGEAR_BOARD_ID := U12L144T01_NETGEAR_NEWLED
  NETGEAR_REGION := 1
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += netgear_dgnd3700-v1

define Device/netgear_dgnd3800b
  $(Device/bcm63xx_netgear)
  DEVICE_MODEL := DGND3800B
  DEVICE_DTS := bcm6368-netgear-dgnd3700-v1
  CFE_BOARD_ID := 96368MVWG
  CHIP_ID := 6368
  BLOCK_SIZE := 0x20000
  NETGEAR_BOARD_ID := U12L144T11_NETGEAR_NEWLED
  NETGEAR_REGION := 1
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += netgear_dgnd3800b

define Device/netgear_evg2000
  $(Device/bcm63xx_netgear)
  DEVICE_MODEL := EVG2000
  CFE_BOARD_ID := 96369PVG
  CHIP_ID := 6368
  BLOCK_SIZE := 0x20000
  NETGEAR_BOARD_ID := U12H154T90_NETGEAR
  NETGEAR_REGION := 1
  SOC := bcm6369
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += netgear_evg2000

### NuCom ###
define Device/nucom_r5010un-v2
  $(Device/bcm63xx)
  DEVICE_VENDOR := NuCom
  DEVICE_MODEL := R5010UN
  DEVICE_VARIANT := v2
  IMAGES += sysupgrade.bin
  CFE_BOARD_ID := 96328ang
  CHIP_ID := 6328
  FLASH_MB := 16
  DEVICE_PACKAGES := $(B43_PACKAGES)
endef
TARGET_DEVICES += nucom_r5010un-v2

### Observa ###
define Device/observa_vh4032n
  $(Device/bcm63xx)
  DEVICE_VENDOR := Observa
  DEVICE_MODEL := VH4032N
  IMAGES += sysupgrade.bin
  CFE_BOARD_ID := 96368VVW
  CHIP_ID := 6368
  BLOCK_SIZE := 0x20000
  FLASH_MB := 32
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += observa_vh4032n

### Pirelli ###
define Device/pirelli_a226g
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Pirelli
  DEVICE_MODEL := A226G
  CFE_BOARD_ID := DWV-S0
  CHIP_ID := 6358
  CFE_EXTRAS += --signature2 IMAGE --tag-version 8
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += pirelli_a226g

define Device/pirelli_a226m
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Pirelli
  DEVICE_MODEL := A226M
  CFE_BOARD_ID := DWV-S0
  CHIP_ID := 6358
  CFE_EXTRAS += --signature2 IMAGE --tag-version 8
  DEVICE_PACKAGES := $(USB2_PACKAGES)
endef
TARGET_DEVICES += pirelli_a226m

define Device/pirelli_a226m-fwb
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Pirelli
  DEVICE_MODEL := A226M-FWB
  CFE_BOARD_ID := DWV-S0
  CHIP_ID := 6358
  CFE_EXTRAS += --signature2 IMAGE --tag-version 8
  BLOCK_SIZE := 0x20000
  DEVICE_PACKAGES := $(USB2_PACKAGES)
endef
TARGET_DEVICES += pirelli_a226m-fwb

define Device/pirelli_agpf-s0
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Pirelli
  DEVICE_MODEL := Alice Gate VoIP 2 Plus Wi-Fi AGPF-S0
  CFE_BOARD_ID := AGPF-S0
  CHIP_ID := 6358
  CFE_EXTRAS += --signature2 IMAGE --tag-version 8
  BLOCK_SIZE := 0x20000
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += pirelli_agpf-s0

### Sagem ###
define Device/sagem_fast-2404
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Sagemcom
  DEVICE_MODEL := F@st 2404
  CFE_BOARD_ID := F@ST2404
  CHIP_ID := 6348
  DEVICE_PACKAGES := $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += sagem_fast-2404

define Device/sagem_fast-2504n
  $(Device/bcm63xx)
  DEVICE_VENDOR := Sagemcom
  DEVICE_MODEL := F@st 2504N
  CFE_BOARD_ID := F@ST2504n
  CHIP_ID := 6362
  DEVICE_PACKAGES := $(B43_PACKAGES)
endef
TARGET_DEVICES += sagem_fast-2504n

define Device/sagem_fast-2604
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Sagemcom
  DEVICE_MODEL := F@st 2604
  CFE_BOARD_ID := F@ST2604
  CHIP_ID := 6348
  DEVICE_PACKAGES := $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += sagem_fast-2604

define Device/sagem_fast-2704n
  $(Device/bcm63xx)
  DEVICE_VENDOR := Sagemcom
  DEVICE_MODEL := F@st 2704N
  CFE_BOARD_ID := F@ST2704N
  CHIP_ID := 6318
  FLASH_MB := 8
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += sagem_fast-2704n

define Device/sagem_fast-2704-v2
  $(Device/bcm63xx)
  DEVICE_VENDOR := Sagemcom
  DEVICE_MODEL := F@st 2704
  DEVICE_VARIANT := V2
  CFE_BOARD_ID := F@ST2704V2
  CHIP_ID := 6328
  FLASH_MB := 8
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += sagem_fast-2704-v2

### Sercomm ###
define Device/sercomm_ad1018-nor
  $(Device/bcm63xx)
  DEVICE_VENDOR := Sercomm
  DEVICE_MODEL := AD1018
  DEVICE_VARIANT := SPI flash mod
  CFE_BOARD_ID := 96328avngr
  CHIP_ID := 6328
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += sercomm_ad1018-nor

### SFR ###
define Device/sfr_neufbox-4-sercomm-r0
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := SFR
  DEVICE_MODEL := Neufbox 4
  DEVICE_VARIANT := Sercomm
  CFE_BOARD_ID := 96358VW
  CHIP_ID := 6358
  CFE_EXTRAS += --rsa-signature "$(VERSION_DIST)-$(firstword $(subst -,$(space),$(REVISION)))"
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += sfr_neufbox-4-sercomm-r0

define Device/sfr_neufbox-4-foxconn-r1
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := SFR
  DEVICE_MODEL := Neufbox 4
  DEVICE_VARIANT := Foxconn
  CFE_BOARD_ID := 96358VW
  CHIP_ID := 6358
  CFE_EXTRAS += --rsa-signature "$(VERSION_DIST)-$(firstword $(subst -,$(space),$(REVISION)))"
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += sfr_neufbox-4-foxconn-r1

define Device/sfr_neufbox-6-sercomm-r0
  $(Device/bcm63xx)
  DEVICE_VENDOR := SFR
  DEVICE_MODEL := Neufbox 6
  CFE_BOARD_ID := NB6-SER-r0
  CHIP_ID := 6362
  CFE_EXTRAS += --rsa-signature "$(VERSION_DIST)-$(firstword $(subst -,$(space),$(REVISION)))"
  SOC := bcm6361
  DEVICE_PACKAGES := $(USB2_PACKAGES)
endef
TARGET_DEVICES += sfr_neufbox-6-sercomm-r0

define Device/sky_sr102
  $(Device/bcm63xx)
  DEVICE_VENDOR := SKY
  DEVICE_MODEL := SR102
  CFE_BOARD_ID := BSKYB_63168
  CHIP_ID := 63268
  CFE_EXTRAS += --rsa-signature "$(VERSION_DIST)-$(firstword $(subst -,$(space),$(REVISION)))"
  SOC := bcm63168
  DEVICE_PACKAGES := $(USB2_PACKAGES)
endef
TARGET_DEVICES += sky_sr102

### T-Com ###
define Device/t-com_speedport-w-303v
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := T-Com
  DEVICE_MODEL := Speedport W 303V
  IMAGES := factory.bin sysupgrade.bin
  IMAGE/factory.bin := cfe-spw303v-bin --pad 4 | spw303v-bin | xor-image
  IMAGE/sysupgrade.bin := cfe-spw303v-bin | spw303v-bin
  CFE_BOARD_ID := 96358-502V
  CHIP_ID := 6358
  DEVICE_PACKAGES := $(B43_PACKAGES)
endef
TARGET_DEVICES += t-com_speedport-w-303v

define Device/t-com_speedport-w-500v
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := T-Com
  DEVICE_MODEL := Speedport W 500V
  CFE_BOARD_ID := 96348GW
  CHIP_ID := 6348
  DEVICE_PACKAGES := $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += t-com_speedport-w-500v

### Technicolor ###
define Device/technicolor_tg582n
  $(Device/bcm63xx)
  DEVICE_VENDOR := Technicolor
  DEVICE_MODEL := TG582n
  IMAGES += sysupgrade.bin
  CFE_BOARD_ID := DANT-1
  CHIP_ID := 6328
  FLASH_MB := 16
  DEVICE_PACKAGES := $(USB2_PACKAGES) $(B43_PACKAGES)
endef
TARGET_DEVICES += technicolor_tg582n

define Device/technicolor_tg582n-telecom-italia
  $(Device/bcm63xx)
  DEVICE_VENDOR := Technicolor
  DEVICE_MODEL := TG582n
  DEVICE_VARIANT := Telecom Italia
  IMAGES += sysupgrade.bin
  CFE_BOARD_ID := DANT-V
  CHIP_ID := 6328
  FLASH_MB := 16
  DEVICE_PACKAGES := $(USB2_PACKAGES) $(B43_PACKAGES)
endef
TARGET_DEVICES += technicolor_tg582n-telecom-italia

### Tecom ###
define Device/tecom_gw6000
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Tecom
  DEVICE_MODEL := GW6000
  CFE_BOARD_ID := 96348GW
  CHIP_ID := 6348
  DEVICE_PACKAGES := $(BRCMWL_PACKAGES) $(USB1_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += tecom_gw6000

define Device/tecom_gw6200
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Tecom
  DEVICE_MODEL := GW6200
  CFE_BOARD_ID := 96348GW
  CHIP_ID := 6348
  CFE_EXTRAS += --rsa-signature "$(shell printf '\x99')"
  DEVICE_PACKAGES := $(BRCMWL_PACKAGES) $(USB1_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += tecom_gw6200

### Telsey ###
define Device/telsey_cpva502plus
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Telsey
  DEVICE_MODEL := CPVA502+
  CFE_BOARD_ID := CPVA502+
  CHIP_ID := 6348
  CFE_EXTRAS += --signature "Telsey Tlc" --signature2 "99.99.999"
  DEVICE_PACKAGES := $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += telsey_cpva502plus

define Device/telsey_cpva642
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Telsey
  DEVICE_MODEL := CPVA642-type (CPA-ZNTE60T)
  CFE_BOARD_ID := CPVA642
  CHIP_ID := 6358
  CFE_EXTRAS += --signature "Telsey Tlc" --signature2 "99.99.999" --second-image-flag "0"
  FLASH_MB := 8
  DEVICE_PACKAGES := $(RT63_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += telsey_cpva642

define Device/telsey_magic
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := Alice
  DEVICE_MODEL := W-Gate
  DEVICE_ALT0_VENDOR := Telsey
  DEVICE_ALT0_MODEL := MAGIC
  IMAGES :=
  CFE_BOARD_ID := MAGIC
  CHIP_ID := 6348
  DEVICE_PACKAGES := $(RT63_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += telsey_magic

### TP-Link ###
define Device/tp-link_td-w8900gb
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := TP-Link
  DEVICE_MODEL := TD-W8900GB
  CFE_BOARD_ID := 96348GW-11
  CHIP_ID := 6348
  CFE_EXTRAS += --rsa-signature "$(shell printf 'PRID\x89\x10\x00\x02')"
  IMAGE_OFFSET := 0x20000
  DEVICE_PACKAGES := $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += tp-link_td-w8900gb

### USRobotics ###
define Device/usrobotics_usr9108
  $(Device/bcm63xx-legacy)
  DEVICE_VENDOR := USRobotics
  DEVICE_MODEL := USR9108
  CFE_BOARD_ID := 96348GW-A
  CHIP_ID := 6348
  DEVICE_PACKAGES := $(B43_PACKAGES) $(USB1_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += usrobotics_usr9108

### ZyXEL ###
define Device/zyxel_p870hw-51a-v2
  $(Device/bcm63xx)
  DEVICE_VENDOR := ZyXEL
  DEVICE_MODEL := P870HW-51a
  DEVICE_VARIANT := v2
  IMAGES := factory.bin
  IMAGE/factory.bin := cfe-bin | zyxel-bin
  CFE_BOARD_ID := 96368VVW
  CHIP_ID := 6368
  CFE_EXTRAS += --rsa-signature "ZyXEL" --signature "ZyXEL_0001"
  DEVICE_PACKAGES := $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += zyxel_p870hw-51a-v2
