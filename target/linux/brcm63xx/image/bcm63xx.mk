
#
# BCM33XX/BCM63XX Profiles
#

define Device/bcm33xx
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | loader-lzma bin | hcs-initramfs
  IMAGES :=
  HCS_MAGIC_BYTES :=
  HCS_REV_MIN :=
  HCS_REV_MAJ :=
endef
DEVICE_VARS += HCS_MAGIC_BYTES HCS_REV_MIN HCS_REV_MAJ

define Device/bcm63xx
  FILESYSTEMS := squashfs jffs2-64k jffs2-128k
  KERNEL := kernel-bin | append-dtb | relocate-kernel | lzma | lzma-cfe
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
  CFE_CHIP_ID :=
  CFE_EXTRAS = --block-size $$(BLOCK_SIZE) --image-offset $$(if $$(IMAGE_OFFSET),$$(IMAGE_OFFSET),$$(BLOCK_SIZE))
endef
DEVICE_VARS += BLOCK_SIZE FLASH_MB IMAGE_OFFSET
DEVICE_VARS += CFE_BOARD_ID CFE_CHIP_ID CFE_EXTRAS

define Device/bcm63xx_netgear
  $(Device/bcm63xx)
  DEVICE_VENDOR := NETGEAR
  IMAGES := factory.chk sysupgrade.bin
  IMAGE/factory.chk := cfe-bin | netgear-chk
  NETGEAR_BOARD_ID :=
  NETGEAR_REGION :=
endef
DEVICE_VARS += NETGEAR_BOARD_ID NETGEAR_REGION

define Device/bcm63xx_redboot
  FILESYSTEMS := squashfs
  KERNEL := kernel-bin | append-dtb | relocate-kernel | gzip
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | loader-lzma elf
  IMAGES := redboot.bin
  IMAGE/redboot.bin := redboot-bin
  REDBOOT_PREFIX := $$(IMAGE_PREFIX)
endef
DEVICE_VARS += REDBOOT_PREFIX

### Generic ###
define Device/963281TAN-generic
  $(Device/bcm63xx)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 963281TAN
  IMAGES := cfe-4M.bin cfe-8M.bin cfe-16M.bin
  DEVICE_DTS := bcm963281TAN
  CFE_BOARD_ID := 963281TAN
  CFE_CHIP_ID := 6328
endef
TARGET_DEVICES += 963281TAN-generic

define Device/96328avng-generic
  $(Device/bcm63xx)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96328avng
  IMAGES := cfe-4M.bin cfe-8M.bin cfe-16M.bin
  DEVICE_DTS := bcm96328avng
  CFE_BOARD_ID := 96328avng
  CFE_CHIP_ID := 6328
endef
TARGET_DEVICES += 96328avng-generic

define Device/96338GW-generic
  $(Device/bcm63xx)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96338GW
  DEVICE_DTS := bcm96338GW
  CFE_BOARD_ID := 6338GW
  CFE_CHIP_ID := 6338
endef
TARGET_DEVICES += 96338GW-generic

define Device/96338W-generic
  $(Device/bcm63xx)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96338W
  DEVICE_DTS := bcm96338W
  CFE_BOARD_ID := 6338W
  CFE_CHIP_ID := 6338
endef
TARGET_DEVICES += 96338W-generic

define Device/96345GW2-generic
  $(Device/bcm63xx)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96345GW2
  IMAGES += cfe-bc221.bin
  DEVICE_DTS := bcm96345GW2
  CFE_BOARD_ID := 96345GW2
  CFE_CHIP_ID := 6345
  DEFAULT := n
endef
TARGET_DEVICES += 96345GW2-generic

define Device/96348GW-generic
  $(Device/bcm63xx)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96348GW
  IMAGES += cfe-bc221.bin
  DEVICE_DTS := bcm96348GW
  CFE_BOARD_ID := 96348GW
  CFE_CHIP_ID := 6348
endef
TARGET_DEVICES += 96348GW-generic

define Device/96348GW-10-generic
  $(Device/bcm63xx)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96348GW-10
  DEVICE_DTS := bcm96348GW-10
  CFE_BOARD_ID := 96348GW-10
  CFE_CHIP_ID := 6348
endef
TARGET_DEVICES += 96348GW-10-generic

define Device/96348GW-11-generic
  $(Device/bcm63xx)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96348GW-11
  DEVICE_DTS := bcm96348GW-11
  CFE_BOARD_ID := 96348GW-11
  CFE_CHIP_ID := 6348
endef
TARGET_DEVICES += 96348GW-11-generic

define Device/96348R-generic
  $(Device/bcm63xx)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96348R
  DEVICE_DTS := bcm96348R
  CFE_BOARD_ID := 96348R
  CFE_CHIP_ID := 6348
endef
TARGET_DEVICES += 96348R-generic

define Device/96358VW-generic
  $(Device/bcm63xx)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96358VW
  DEVICE_DTS := bcm96358VW
  CFE_BOARD_ID := 96358VW
  CFE_CHIP_ID := 6358
endef
TARGET_DEVICES += 96358VW-generic

define Device/96358VW2-generic
  $(Device/bcm63xx)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96358VW2
  DEVICE_DTS := bcm96358VW2
  CFE_BOARD_ID := 96358VW2
  CFE_CHIP_ID := 6358
endef
TARGET_DEVICES += 96358VW2-generic

define Device/96368MVNgr-generic
  $(Device/bcm63xx)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96368MVNgr
  DEVICE_DTS := bcm96368MVNgr
  CFE_BOARD_ID := 96368MVNgr
  CFE_CHIP_ID := 6368
endef
TARGET_DEVICES += 96368MVNgr-generic

define Device/96368MVWG-generic
  $(Device/bcm63xx)
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := 96368MVWG
  DEVICE_DTS := bcm96368MVWG
  CFE_BOARD_ID := 96368MVWG
  CFE_CHIP_ID := 6368
endef
TARGET_DEVICES += 96368MVWG-generic

### Actiontec ###
define Device/R1000H
  $(Device/bcm63xx)
  DEVICE_VENDOR := Actiontec
  DEVICE_MODEL := R1000H
  FILESYSTEMS := squashfs
  DEVICE_DTS := r1000h
  CFE_BOARD_ID := 96368MVWG
  CFE_CHIP_ID := 6368
  FLASH_MB := 32
  IMAGE_OFFSET := 0x20000
  DEVICE_PACKAGES := \
    $(USB2_PACKAGES) $(BRCMWL_PACKAGES)
endef
TARGET_DEVICES += R1000H

### ADB ###
define Device/A4001N
  $(Device/bcm63xx)
  DEVICE_VENDOR := ADB
  DEVICE_MODEL := P.DG A4001N
  DEVICE_DTS := a4001n
  CFE_BOARD_ID := 96328dg2x2
  CFE_CHIP_ID := 6328
  FLASH_MB := 8
  DEVICE_PACKAGES := \
    $(USB2_PACKAGES) $(B43_PACKAGES)
endef
TARGET_DEVICES += A4001N

define Device/A4001N1
  $(Device/bcm63xx)
  DEVICE_VENDOR := ADB
  DEVICE_MODEL := P.DG A4001N1
  IMAGES += sysupgrade.bin
  DEVICE_DTS := a4001n1
  CFE_BOARD_ID := 963281T_TEF
  CFE_CHIP_ID := 6328
  FLASH_MB := 16
  DEVICE_PACKAGES := \
    $(USB2_PACKAGES) $(B43_PACKAGES)
endef
TARGET_DEVICES += A4001N1

define Device/AV4202N
  $(Device/bcm63xx)
  DEVICE_VENDOR := ADB
  DEVICE_MODEL := P.DG AV4202N
  IMAGE_OFFSET := 0x20000
  DEVICE_DTS := av4202n
  CFE_BOARD_ID := 96368_Swiss_S1
  CFE_CHIP_ID := 6368
  DEVICE_PACKAGES := \
    $(USB2_PACKAGES) $(B43_PACKAGES)
endef
TARGET_DEVICES += AV4202N

### Alcatel ###
define Device/RG100A
  $(Device/bcm63xx)
  DEVICE_VENDOR := Alcatel
  DEVICE_MODEL := RG100A
  DEVICE_DTS := rg100a
  CFE_BOARD_ID := 96358VW2
  CFE_CHIP_ID := 6358
  BLOCK_SIZE := 0x20000
  DEVICE_PACKAGES := \
    $(USB2_PACKAGES) $(B43_PACKAGES)
endef
TARGET_DEVICES += RG100A

### Asmax ###
define Device/AR1004G
  $(Device/bcm63xx)
  DEVICE_VENDOR := Asmax
  DEVICE_MODEL := AR 1004g
  DEVICE_DTS := ar1004g
  CFE_BOARD_ID := 96348GW-10
  CFE_CHIP_ID := 6348
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += AR1004G

### Belkin ###
define Device/F5D7633
  $(Device/bcm63xx)
  DEVICE_VENDOR := Belkin
  DEVICE_MODEL := F5D7633
  DEVICE_DTS := f5d7633
  CFE_BOARD_ID := 96348GW-10
  CFE_CHIP_ID := 6348
  BLOCK_SIZE := 0x20000
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += F5D7633

### Broadcom ###
define Device/BCM96318REF
  $(Device/bcm63xx)
  DEVICE_VENDOR := Broadcom
  DEVICE_MODEL := BCM96318REF reference board
  IMAGES :=
  DEVICE_DTS := bcm96318ref
  CFE_BOARD_ID := 96318REF
  CFE_CHIP_ID := 6318
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES) \
    kmod-bcm63xx-udc
endef
TARGET_DEVICES += BCM96318REF

define Device/BCM96318REF_P300
  $(Device/bcm63xx)
  DEVICE_VENDOR := Broadcom
  DEVICE_MODEL := BCM96318REF_P300 reference board
  IMAGES :=
  DEVICE_DTS := bcm96318ref_p300
  CFE_BOARD_ID := 96318REF_P300
  CFE_CHIP_ID := 6318
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES) \
    kmod-bcm63xx-udc
endef
TARGET_DEVICES += BCM96318REF_P300

define Device/BCM963268BU_P300
  $(Device/bcm63xx)
  DEVICE_VENDOR := Broadcom
  DEVICE_MODEL := BCM963268BU_P300 reference board
  IMAGES :=
  DEVICE_DTS := bcm963268bu_p300
  CFE_BOARD_ID := 963268BU_P300
  CFE_CHIP_ID := 63268
  DEVICE_PACKAGES := \
    $(USB2_PACKAGES) \
    kmod-bcm63xx-udc
endef
TARGET_DEVICES += BCM963268BU_P300

define Device/BCM963269BHR
  $(Device/bcm63xx)
  DEVICE_VENDOR := Broadcom
  DEVICE_MODEL := BCM963269BHR reference board
  IMAGES :=
  DEVICE_DTS := bcm963269bhr
  CFE_BOARD_ID := 963269BHR
  CFE_CHIP_ID := 63268
  DEVICE_PACKAGES := \
    $(USB2_PACKAGES) \
    kmod-bcm63xx-udc
endef
TARGET_DEVICES += BCM963269BHR

### BT ###
define Device/HomeHub2A
  $(Device/bcm63xx)
  DEVICE_VENDOR := BT
  DEVICE_MODEL := Home Hub 2.0
  DEVICE_VARIANT := A
  DEVICE_DTS := homehub2a
  CFE_BOARD_ID := HOMEHUB2A
  CFE_CHIP_ID := 6358
  BLOCK_SIZE := 0x20000
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += HomeHub2A

define Device/BTV2110
  $(Device/bcm63xx)
  DEVICE_VENDOR := BT
  DEVICE_MODEL := Voyager V2110
  DEVICE_DTS := v2110
  CFE_BOARD_ID := V2110
  CFE_CHIP_ID := 6348
  CFE_EXTRAS += --layoutver 5
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += BTV2110

define Device/BTV2500V
  $(Device/bcm63xx)
  DEVICE_VENDOR := BT
  DEVICE_MODEL := Voyager V2500V
  DEVICE_DTS := v2500v-bb
  CFE_BOARD_ID := V2500V_BB
  CFE_CHIP_ID := 6348
  CFE_EXTRAS += --layoutver 5
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += BTV2500V

### Comtrend ###
define Device/AR5315u
  $(Device/bcm63xx)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := AR-5315u
  IMAGES += sysupgrade.bin
  DEVICE_DTS := ar-5315u
  CFE_BOARD_ID := 96318A-1441N1
  CFE_CHIP_ID := 6318
  FLASH_MB := 16
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += AR5315u

define Device/AR5381u
  $(Device/bcm63xx)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := AR-5381u
  IMAGES += sysupgrade.bin
  DEVICE_DTS := ar-5381u
  CFE_BOARD_ID := 96328A-1241N
  CFE_CHIP_ID := 6328
  FLASH_MB := 16
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += AR5381u

define Device/AR5387un
  $(Device/bcm63xx)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := AR-5387un
  IMAGES += sysupgrade.bin
  DEVICE_DTS := ar-5387un
  CFE_BOARD_ID := 96328A-1441N1
  CFE_CHIP_ID := 6328
  FLASH_MB := 16
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += AR5387un

define Device/CT-536_CT-5621
  $(Device/bcm63xx)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := CT-536+
  DEVICE_ALT0_VENDOR := Comtrend
  DEVICE_ALT0_MODEL := CT-5621
  DEVICE_DTS := ct536plus
  CFE_BOARD_ID := 96348GW-11
  CFE_CHIP_ID := 6348
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += CT-536_CT-5621

define Device/CT-5365
  $(Device/bcm63xx)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := CT-5365
  DEVICE_DTS := ct-5365
  CFE_BOARD_ID := 96348A-122
  CFE_CHIP_ID := 6348
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += CT-5365

define Device/CT-6373
  $(Device/bcm63xx)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := CT-6373
  DEVICE_DTS := ct-6373
  CFE_BOARD_ID := CT6373-1
  CFE_CHIP_ID := 6358
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += CT-6373

define Device/VR-3025u
  $(Device/bcm63xx)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := VR-3025u
  IMAGES += sysupgrade.bin
  DEVICE_DTS := vr-3025u
  CFE_BOARD_ID := 96368M-1541N
  CFE_CHIP_ID := 6368
  BLOCK_SIZE := 0x20000
  FLASH_MB := 32
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += VR-3025u

define Device/VR-3025un
  $(Device/bcm63xx)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := VR-3025un
  DEVICE_DTS := vr-3025un
  CFE_BOARD_ID := 96368M-1341N
  CFE_CHIP_ID := 6368
  FLASH_MB := 8
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += VR-3025un

define Device/VR-3026e
  $(Device/bcm63xx)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := VR-3026e
  DEVICE_DTS := vr-3026e
  CFE_BOARD_ID := 96368MT-1341N1
  CFE_CHIP_ID := 6368
  FLASH_MB := 8
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
endef
TARGET_DEVICES += VR-3026e

define Device/WAP-5813n
  $(Device/bcm63xx)
  DEVICE_VENDOR := Comtrend
  DEVICE_MODEL := WAP-5813n
  DEVICE_DTS := wap-5813n
  CFE_BOARD_ID := 96369R-1231N
  CFE_CHIP_ID := 6368
  FLASH_MB := 8
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += WAP-5813n

### D-Link ###
define Device/DSL2640B-B
  $(Device/bcm63xx)
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DSL-2640B
  DEVICE_VARIANT := B2
  DEVICE_DTS := dsl-2640b-b
  CFE_BOARD_ID := D-4P-W
  CFE_CHIP_ID := 6348
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += DSL2640B-B

define Device/DSL2640U
  $(Device/bcm63xx)
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DSL-2640U
  DEVICE_VARIANT := C1
  DEVICE_ALT0_VENDOR := D-Link
  DEVICE_ALT0_MODEL := DSL-2640U/BRU/C
  DEVICE_DTS := dsl-2640u
  CFE_BOARD_ID := 96338W2_E7T
  CFE_CHIP_ID := 6338
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += DSL2640U

define Device/DSL2650U
  $(Device/bcm63xx)
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DSL-2650U
  DEVICE_DTS := dsl-2650u
  CFE_BOARD_ID := 96358VW2
  CFE_CHIP_ID := 6358
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += DSL2650U

define Device/DSL274XB-C2
  $(Device/bcm63xx)
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DSL-2740B
  DEVICE_VARIANT := C2
  DEVICE_ALT0_VENDOR := D-Link
  DEVICE_ALT0_MODEL := DSL-2741B
  DEVICE_ALT0_VARIANT := C2
  DEVICE_DTS := dsl-274xb-c
  CFE_BOARD_ID := 96358GW
  CFE_CHIP_ID := 6358
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
endef
TARGET_DEVICES += DSL274XB-C2

define Device/DSL274XB-C3
  $(Device/bcm63xx)
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DSL-2740B
  DEVICE_VARIANT := C3
  DEVICE_ALT0_VENDOR := D-Link
  DEVICE_ALT0_MODEL := DSL-2741B
  DEVICE_ALT0_VARIANT := C3
  DEVICE_DTS := dsl-274xb-c
  CFE_BOARD_ID := AW4139
  CFE_CHIP_ID := 6358
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
endef
TARGET_DEVICES += DSL274XB-C3

define Device/DSL274XB-F1
  $(Device/bcm63xx)
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DSL-2740B
  DEVICE_VARIANT := F1
  DEVICE_ALT0_VENDOR := D-Link
  DEVICE_ALT0_MODEL := DSL-2741B
  DEVICE_ALT0_VARIANT := F1
  DEVICE_DTS := dsl-274xb-f
  CFE_BOARD_ID := AW4339U
  CFE_CHIP_ID := 6328
  IMAGES := cfe-EU.bin cfe-AU.bin
  IMAGE/cfe-AU.bin := cfe-bin --signature2 "4.06.01.AUF1" --pad 4
  IMAGE/cfe-EU.bin := cfe-bin --signature2 "4.06.01.EUF1" --pad 4
  DEVICE_PACKAGES := \
    $(ATH9K_PACKAGES)
endef
TARGET_DEVICES += DSL274XB-F1

define Device/DSL275XB-D1
  $(Device/bcm63xx)
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DSL-2750B
  DEVICE_VARIANT := D1
  DEVICE_ALT0_VENDOR := D-Link
  DEVICE_ALT0_MODEL := DSL-2751
  DEVICE_ALT0_VARIANT := D1
  DEVICE_DTS := dsl-275xb-d
  CFE_BOARD_ID := AW5200B
  CFE_CHIP_ID := 6318
  FLASH_MB := 8
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += DSL275XB-D1

define Device/DVAG3810BN
  $(Device/bcm63xx)
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DVA-G3810BN/TL
  DEVICE_DTS := dva-g3810bn_tl
  CFE_BOARD_ID := 96358VW
  CFE_CHIP_ID := 6358
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += DVAG3810BN

### Davolink ###
define Device/DV-201AMR
  $(Device/bcm63xx)
  DEVICE_VENDOR := Davolink
  DEVICE_MODEL := DV-201AMR
  IMAGES := cfe-old.bin
  DEVICE_DTS := dv-201amr
  CFE_BOARD_ID := DV201AMR
  CFE_CHIP_ID := 6348
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += DV-201AMR

### Dynalink ###
define Device/RTA770BW
  $(Device/bcm63xx)
  DEVICE_VENDOR := Dynalink
  DEVICE_MODEL := RTA770BW
  DEVICE_ALT0_VENDOR := Siemens
  DEVICE_ALT0_MODEL := SE515
  IMAGES =
  DEVICE_DTS := rta770bw
  CFE_BOARD_ID := RTA770BW
  CFE_CHIP_ID := 6345
  CFE_EXTRAS += --layoutver 5
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += RTA770BW

define Device/RTA770W
  $(Device/bcm63xx)
  DEVICE_VENDOR := Dynalink
  DEVICE_MODEL := RTA770W
  IMAGES =
  DEVICE_DTS := rta770w
  CFE_BOARD_ID := RTA770W
  CFE_CHIP_ID := 6345
  CFE_EXTRAS += --layoutver 5
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += RTA770W

define Device/RTA1025W_16
  $(Device/bcm63xx)
  DEVICE_VENDOR := Dynalink
  DEVICE_MODEL := RTA1025W
  DEVICE_DTS := rta1025w
  CFE_BOARD_ID := RTA1025W_16
  CFE_CHIP_ID := 6348
  CFE_EXTRAS += --layoutver 5
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += RTA1025W_16

define Device/RTA1320_16M
  $(Device/bcm63xx)
  DEVICE_VENDOR := Dynalink
  DEVICE_MODEL := RTA1320
  DEVICE_DTS := rta1320
  CFE_BOARD_ID := RTA1320_16M
  CFE_CHIP_ID := 6338
  CFE_EXTRAS += --layoutver 5
  DEFAULT := n
endef
TARGET_DEVICES += RTA1320_16M

### Huawei ###
define Device/HG520v
  $(Device/bcm63xx)
  DEVICE_VENDOR := Huawei
  DEVICE_MODEL := EchoLife HG520v
  DEVICE_DTS := hg520v
  CFE_BOARD_ID := HW6358GW_B
  CFE_CHIP_ID := 6358
  CFE_EXTRAS += --rsa-signature "EchoLife_HG520v"
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
endef
TARGET_DEVICES += HG520v

define Device/HG553
  $(Device/bcm63xx)
  DEVICE_VENDOR := Huawei
  DEVICE_MODEL := EchoLife HG553
  DEVICE_DTS := hg553
  CFE_BOARD_ID := HW553
  CFE_CHIP_ID := 6358
  CFE_EXTRAS += --rsa-signature "EchoLife_HG553" --tag-version 7
  BLOCK_SIZE := 0x20000
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += HG553

define Device/HG556a-A
  $(Device/bcm63xx)
  DEVICE_VENDOR := Huawei
  DEVICE_MODEL := EchoLife HG556a
  DEVICE_VARIANT := A
  DEVICE_DESCRIPTION = Build firmware images for Huawei HG556a version A (Atheros)
  DEVICE_DTS := hg556a-a
  CFE_BOARD_ID := HW556
  CFE_CHIP_ID := 6358
  CFE_EXTRAS += --rsa-signature "EchoLife_HG556a" --tag-version 8
  IMAGE_OFFSET := 0x20000
  DEVICE_PACKAGES := \
    $(ATH9K_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += HG556a-A

define Device/HG556a-B
  $(Device/bcm63xx)
  DEVICE_VENDOR := Huawei
  DEVICE_MODEL := EchoLife HG556a
  DEVICE_VARIANT := B
  DEVICE_DESCRIPTION = Build firmware images for Huawei HG556a version B (Atheros)
  DEVICE_DTS := hg556a-b
  CFE_BOARD_ID := HW556
  CFE_CHIP_ID := 6358
  CFE_EXTRAS += --rsa-signature "EchoLife_HG556a" --tag-version 8
  BLOCK_SIZE := 0x20000
  DEVICE_PACKAGES := \
    $(ATH9K_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += HG556a-B

define Device/HG556a-C
  $(Device/bcm63xx)
  DEVICE_VENDOR := Huawei
  DEVICE_MODEL := EchoLife HG556a
  DEVICE_VARIANT := C
  DEVICE_DESCRIPTION = Build firmware images for Huawei HG556a version C (Ralink)
  DEVICE_DTS := hg556a-c
  CFE_BOARD_ID := HW556
  CFE_CHIP_ID := 6358
  CFE_EXTRAS += --rsa-signature "EchoLife_HG556a" --tag-version 8
  BLOCK_SIZE := 0x20000
  DEVICE_PACKAGES := \
    $(RT28_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += HG556a-C

define Device/HG622
  $(Device/bcm63xx)
  DEVICE_VENDOR := Huawei
  DEVICE_MODEL := EchoLife HG622
  IMAGES += sysupgrade.bin
  DEVICE_DTS := hg622
  CFE_BOARD_ID := 96368MVWG_hg622
  CFE_CHIP_ID := 6368
  CFE_EXTRAS += --tag-version 7
  BLOCK_SIZE := 0x20000
  FLASH_MB := 16
  DEVICE_PACKAGES := \
    $(RT28_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += HG622

define Device/HG655b
  $(Device/bcm63xx)
  DEVICE_VENDOR := Huawei
  DEVICE_MODEL := EchoLife HG655b
  DEVICE_DTS := hg655b
  CFE_BOARD_ID := HW65x
  CFE_CHIP_ID := 6368
  CFE_EXTRAS += --tag-version 7
  IMAGE_OFFSET := 0x20000
  FLASH_MB := 8
  DEVICE_PACKAGES := \
    $(RT28_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += HG655b

### Inteno ###
define Device/VG50
  $(Device/bcm63xx)
  DEVICE_VENDOR := Inteno
  DEVICE_MODEL := VG50 Multi-WAN CPE
  IMAGES :=
  DEVICE_DTS := vg50
  CFE_BOARD_ID := VW6339GU
  CFE_CHIP_ID := 63268
  DEVICE_PACKAGES := \
    $(USB2_PACKAGES)
endef
TARGET_DEVICES += VG50

### Inventel ###
define Device/livebox
  $(Device/bcm63xx_redboot)
  DEVICE_VENDOR := Inventel
  DEVICE_MODEL := Livebox 1
  DEVICE_DTS := livebox-blue-5g
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB1_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += livebox

### Netgear ###
define Device/CVG834G
  $(Device/bcm33xx)
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := CVG834G
  DEVICE_DTS := cvg834g
  HCS_MAGIC_BYTES := 0xa020
  HCS_REV_MIN := 0001
  HCS_REV_MAJ := 0022
endef
TARGET_DEVICES += CVG834G

define Device/DG834GT_PN
  $(Device/bcm63xx)
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := DG834GT
  DEVICE_ALT0_VENDOR := NETGEAR
  DEVICE_ALT0_MODEL := DG834PN
  DEVICE_DTS := dg834gtpn
  CFE_BOARD_ID := 96348GW-10
  CFE_CHIP_ID := 6348
  DEVICE_PACKAGES := \
    $(ATH5K_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += DG834GT_PN

define Device/DG834GTv4
  $(Device/bcm63xx)
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := DG834G
  DEVICE_VARIANT := v4
  IMAGES :=
  DEVICE_DTS := dg834g_v4
  CFE_BOARD_ID := 96348W3
  CFE_CHIP_ID := 6348
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += DG834GTv4

define Device/DGND3700v1
  $(Device/bcm63xx_netgear)
  DEVICE_MODEL := DGND3700
  DEVICE_VARIANT := v1
  DEVICE_DTS := dgnd3700v1
  CFE_BOARD_ID := 96368MVWG
  CFE_CHIP_ID := 6368
  BLOCK_SIZE := 0x20000
  NETGEAR_BOARD_ID := U12L144T01_NETGEAR_NEWLED
  NETGEAR_REGION := 1
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += DGND3700v1

define Device/DGND3800B
  $(Device/bcm63xx_netgear)
  DEVICE_MODEL := DGND3800B
  DEVICE_DTS := dgnd3700v1
  CFE_BOARD_ID := 96368MVWG
  CFE_CHIP_ID := 6368
  BLOCK_SIZE := 0x20000
  NETGEAR_BOARD_ID := U12L144T11_NETGEAR_NEWLED
  NETGEAR_REGION := 1
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += DGND3800B

define Device/EVG2000
  $(Device/bcm63xx_netgear)
  DEVICE_MODEL := EVG2000
  DEVICE_DTS := evg2000
  CFE_BOARD_ID := 96369PVG
  CFE_CHIP_ID := 6368
  BLOCK_SIZE := 0x20000
  NETGEAR_BOARD_ID := U12H154T90_NETGEAR
  NETGEAR_REGION := 1
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += EVG2000

### NuCom ###
define Device/R5010UNv2
  $(Device/bcm63xx)
  DEVICE_VENDOR := NuCom
  DEVICE_MODEL := R5010UN
  DEVICE_VARIANT := v2
  IMAGES += sysupgrade.bin
  DEVICE_DTS := r5010unv2
  CFE_BOARD_ID := 96328ang
  CFE_CHIP_ID := 6328
  FLASH_MB := 16
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
endef
TARGET_DEVICES += R5010UNv2

### Observa ###
define Device/VH4032N
  $(Device/bcm63xx)
  DEVICE_VENDOR := Observa
  DEVICE_MODEL := VH4032N
  IMAGES += sysupgrade.bin
  DEVICE_DTS := vh4032n
  CFE_BOARD_ID := 96368VVW
  CFE_CHIP_ID := 6368
  BLOCK_SIZE := 0x20000
  FLASH_MB := 32
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += VH4032N

### Pirelli ###
define Device/A226G
  $(Device/bcm63xx)
  DEVICE_VENDOR := Pirelli
  DEVICE_MODEL := A226G
  DEVICE_DTS := a226g
  CFE_BOARD_ID := DWV-S0
  CFE_CHIP_ID := 6358
  CFE_EXTRAS += --signature2 IMAGE --tag-version 8
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += A226G

define Device/A226M
  $(Device/bcm63xx)
  DEVICE_VENDOR := Pirelli
  DEVICE_MODEL := A226M
  DEVICE_DTS := a226m
  CFE_BOARD_ID := DWV-S0
  CFE_CHIP_ID := 6358
  CFE_EXTRAS += --signature2 IMAGE --tag-version 8
  DEVICE_PACKAGES := \
    $(USB2_PACKAGES)
endef
TARGET_DEVICES += A226M

define Device/A226M-FWB
  $(Device/bcm63xx)
  DEVICE_VENDOR := Pirelli
  DEVICE_MODEL := A226M-FWB
  DEVICE_DTS := a226m-fwb
  CFE_BOARD_ID := DWV-S0
  CFE_CHIP_ID := 6358
  CFE_EXTRAS += --signature2 IMAGE --tag-version 8
  BLOCK_SIZE := 0x20000
  DEVICE_PACKAGES := \
    $(USB2_PACKAGES)
endef
TARGET_DEVICES += A226M-FWB

define Device/AGPF-S0
  $(Device/bcm63xx)
  DEVICE_VENDOR := Pirelli
  DEVICE_MODEL := Alice Gate VoIP 2 Plus Wi-Fi AGPF-S0
  DEVICE_DTS := agpf-s0
  CFE_BOARD_ID := AGPF-S0
  CFE_CHIP_ID := 6358
  CFE_EXTRAS += --signature2 IMAGE --tag-version 8
  BLOCK_SIZE := 0x20000
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += AGPF-S0

### Sagem ###
define Device/FAST2404
  $(Device/bcm63xx)
  DEVICE_VENDOR := Sagemcom
  DEVICE_MODEL := F@st 2404
  DEVICE_DTS := fast2404
  CFE_BOARD_ID := F@ST2404
  CFE_CHIP_ID := 6348
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += FAST2404

define Device/FAST2504n
  $(Device/bcm63xx)
  DEVICE_VENDOR := Sagemcom
  DEVICE_MODEL := F@st 2504N
  DEVICE_DTS := fast2504n
  CFE_BOARD_ID := F@ST2504n
  CFE_CHIP_ID := 6362
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
endef
TARGET_DEVICES += FAST2504n

define Device/FAST2604
  $(Device/bcm63xx)
  DEVICE_VENDOR := Sagemcom
  DEVICE_MODEL := F@st 2604
  DEVICE_DTS := fast2604
  CFE_BOARD_ID := F@ST2604
  CFE_CHIP_ID := 6348
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += FAST2604

define Device/FAST2704N
  $(Device/bcm63xx)
  DEVICE_VENDOR := Sagemcom
  DEVICE_MODEL := F@st 2704N
  DEVICE_DTS := fast2704n
  CFE_BOARD_ID := F@ST2704N
  CFE_CHIP_ID := 6318
  FLASH_MB := 8
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += FAST2704N

define Device/FAST2704V2
  $(Device/bcm63xx)
  DEVICE_VENDOR := Sagemcom
  DEVICE_MODEL := F@st 2704 V2
  DEVICE_DTS := fast2704v2
  CFE_BOARD_ID := F@ST2704V2
  CFE_CHIP_ID := 6328
  FLASH_MB := 8
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += FAST2704V2

### Sercomm ###
define Device/AD1018-SPI_flash
  $(Device/bcm63xx)
  DEVICE_VENDOR := Sercomm
  DEVICE_MODEL := AD1018
  DEVICE_VARIANT := SPI flash mod
  DEVICE_DTS := ad1018-nor
  CFE_BOARD_ID := 96328avngr
  CFE_CHIP_ID := 6328
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += AD1018-SPI_flash

### SFR ###
define Device/NEUFBOX4-SER
  $(Device/bcm63xx)
  DEVICE_VENDOR := SFR
  DEVICE_MODEL := Neufbox4
  DEVICE_VARIANT := Sercomm
  DEVICE_DTS := nb4-ser-r0
  CFE_BOARD_ID := 96358VW
  CFE_CHIP_ID := 6358
  CFE_EXTRAS += --rsa-signature "$(VERSION_DIST)-$(firstword $(subst -,$(space),$(REVISION)))"
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += NEUFBOX4-SER

define Device/NEUFBOX4-FXC
  $(Device/bcm63xx)
  DEVICE_VENDOR := SFR
  DEVICE_MODEL := Neufbox4
  DEVICE_VARIANT := Foxconn
  DEVICE_DTS := nb4-fxc-r1
  CFE_BOARD_ID := 96358VW
  CFE_CHIP_ID := 6358
  CFE_EXTRAS += --rsa-signature "$(VERSION_DIST)-$(firstword $(subst -,$(space),$(REVISION)))"
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += NEUFBOX4-FXC

define Device/NEUFBOX6
  $(Device/bcm63xx)
  DEVICE_VENDOR := SFR
  DEVICE_MODEL := Neufbox6
  DEVICE_DTS := nb6-ser-r0
  CFE_BOARD_ID := NB6-SER-r0
  CFE_CHIP_ID := 6362
  CFE_EXTRAS += --rsa-signature "$(VERSION_DIST)-$(firstword $(subst -,$(space),$(REVISION)))"
  DEVICE_PACKAGES := \
    $(USB2_PACKAGES)
endef
TARGET_DEVICES += NEUFBOX6

define Device/SR102
  $(Device/bcm63xx)
  DEVICE_VENDOR := SKY
  DEVICE_MODEL := SR102
  DEVICE_DTS := sr102
  CFE_BOARD_ID := BSKYB_63168
  CFE_CHIP_ID := 63268
  CFE_EXTRAS += --rsa-signature "$(VERSION_DIST)-$(firstword $(subst -,$(space),$(REVISION)))"
  DEVICE_PACKAGES := \
    $(USB2_PACKAGES)
endef
TARGET_DEVICES += SR102

### T-Com ###
define Device/SPW303V
  $(Device/bcm63xx)
  DEVICE_VENDOR := T-Com
  DEVICE_MODEL := Speedport W 303V
  IMAGES := factory.bin sysupgrade.bin
  IMAGE/factory.bin := cfe-spw303v-bin --pad 4 | spw303v-bin | xor-image
  IMAGE/sysupgrade.bin := cfe-spw303v-bin | spw303v-bin
  DEVICE_DTS := spw303v
  CFE_BOARD_ID := 96358-502V
  CFE_CHIP_ID := 6358
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
endef
TARGET_DEVICES += SPW303V

define Device/SPW500V
  $(Device/bcm63xx)
  DEVICE_VENDOR := T-Com
  DEVICE_MODEL := Speedport W 500V
  DEVICE_DTS := spw500v
  CFE_BOARD_ID := 96348GW
  CFE_CHIP_ID := 6348
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += SPW500V

### Tecom ###
define Device/GW6000
  $(Device/bcm63xx)
  DEVICE_VENDOR := Tecom
  DEVICE_MODEL := GW6000
  DEVICE_DTS := gw6000
  CFE_BOARD_ID := 96348GW
  CFE_CHIP_ID := 6348
  DEVICE_PACKAGES := \
    $(BRCMWL_PACKAGES) $(USB1_PACKAGES)
endef
TARGET_DEVICES += GW6000

define Device/GW6200
  $(Device/bcm63xx)
  DEVICE_VENDOR := Tecom
  DEVICE_MODEL := GW6200
  DEVICE_DTS := gw6200
  CFE_BOARD_ID := 96348GW
  CFE_CHIP_ID := 6348
  CFE_EXTRAS += --rsa-signature "$(shell printf '\x99')"
  DEVICE_PACKAGES := \
    $(BRCMWL_PACKAGES) $(USB1_PACKAGES)
endef
TARGET_DEVICES += GW6200

### Telsey ###
define Device/CPVA502PLUS
  $(Device/bcm63xx)
  DEVICE_VENDOR := Telsey
  DEVICE_MODEL := CPVA502+
  DEVICE_DTS := cpva502plus
  CFE_BOARD_ID := CPVA502+
  CFE_CHIP_ID := 6348
  CFE_EXTRAS += --signature "Telsey Tlc" --signature2 "99.99.999"
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += CPVA502PLUS

define Device/CPA-ZNTE60T
  $(Device/bcm63xx)
  DEVICE_VENDOR := Telsey
  DEVICE_MODEL := CPVA642-type (CPA-ZNTE60T)
  DEVICE_DTS := cpva642
  CFE_BOARD_ID := CPVA642
  CFE_CHIP_ID := 6358
  CFE_EXTRAS += --signature "Telsey Tlc" --signature2 "99.99.999" --second-image-flag "0"
  FLASH_MB := 8
  DEVICE_PACKAGES := \
    $(RT63_PACKAGES) $(USB2_PACKAGES)
endef
TARGET_DEVICES += CPA-ZNTE60T

define Device/MAGIC
  $(Device/bcm63xx)
  DEVICE_VENDOR := Alice
  DEVICE_MODEL := W-Gate
  DEVICE_ALT0_VENDOR := Telsey
  DEVICE_ALT0_MODEL := MAGIC
  IMAGES :=
  DEVICE_DTS := magic
  CFE_BOARD_ID := MAGIC
  CFE_CHIP_ID := 6348
  DEVICE_PACKAGES := \
    $(RT63_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += MAGIC

### TP-Link ###
define Device/TD-W8900GB
  $(Device/bcm63xx)
  DEVICE_VENDOR := TP-Link
  DEVICE_MODEL := TD-W8900GB
  DEVICE_DTS := td-w8900gb
  CFE_BOARD_ID := 96348GW-11
  CFE_CHIP_ID := 6348
  CFE_EXTRAS += --rsa-signature "$(shell printf 'PRID\x89\x10\x00\x02')"
  IMAGE_OFFSET := 0x20000
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += TD-W8900GB

### USRobotics ###
define Device/USR9108
  $(Device/bcm63xx)
  DEVICE_VENDOR := USRobotics
  DEVICE_MODEL := 9108
  DEVICE_DTS := usr9108
  CFE_BOARD_ID := 96348GW-A
  CFE_CHIP_ID := 6348
  DEVICE_PACKAGES := \
    $(B43_PACKAGES) $(USB1_PACKAGES)
  DEFAULT := n
endef
TARGET_DEVICES += USR9108

### ZyXEL ###
define Device/P870HW-51a_v2
  $(Device/bcm63xx)
  DEVICE_VENDOR := ZyXEL
  DEVICE_MODEL := P870HW-51a
  DEVICE_VARIANT := v2
  IMAGES := factory.bin
  IMAGE/factory.bin := cfe-bin | zyxel-bin
  DEVICE_DTS := p870hw-51a-v2
  CFE_BOARD_ID := 96368VVW
  CFE_CHIP_ID := 6368
  CFE_EXTRAS += --rsa-signature "ZyXEL" --signature "ZyXEL_0001"
  DEVICE_PACKAGES := \
    $(B43_PACKAGES)
endef
TARGET_DEVICES += P870HW-51a_v2
