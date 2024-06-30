include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2024.01
PKG_HASH:=b99611f1ed237bf3541bdc8434b68c96a6e05967061f992443cb30aabebef5b3
PKG_BUILD_DEPENDS:=!(TARGET_ramips||TARGET_mediatek_mt7623):arm-trusted-firmware-tools/host

UBOOT_USE_INTREE_DTC:=1

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

MT7621_LOWLEVEL_PRELOADER_URL:=https://raw.githubusercontent.com/mtk-openwrt/mt7621-lowlevel-preloader/a03b07c60bf1ba4add9b671d32caa102fe948180/

define Download/mt7621-stage-sram
  FILE:=mt7621_stage_sram.bin
  URL:=$(MT7621_LOWLEVEL_PRELOADER_URL)
  HASH:=1dda68aa089f0ff262e01539b990dea478952e9fb68bcc0a8cd6f76f0135c62e
endef

define Download/mt7621-stage-sram-noprint
  FILE:=mt7621_stage_sram_noprint.bin
  URL:=$(MT7621_LOWLEVEL_PRELOADER_URL)
  HASH:=8ee419275144fc298e9444d413d98e965a55d283152a74ea6a1f8de79eb516b6
endef

ifdef CONFIG_TARGET_ramips_mt7621
ifdef CONFIG_DEBUG
$(eval $(call Download,mt7621-stage-sram))
else
$(eval $(call Download,mt7621-stage-sram-noprint))
endif
endif

define U-Boot/Default
  BUILD_TARGET:=mediatek
  UBOOT_IMAGE:=u-boot-mtk.bin
  HIDDEN:=1
endef

define U-Boot/mt7620_rfb
  NAME:=MT7620 Reference Board
  UBOOT_CONFIG:=mt7620_rfb
  BUILD_DEVICES:=ralink_mt7620a-evb
  BUILD_TARGET:=ramips
  BUILD_SUBTARGET:=mt7620
  UBOOT_IMAGE:=u-boot-with-spl.bin
endef

define U-Boot/mt7620_mt7530_rfb
  NAME:=MT7620+MT7530 Reference Board
  UBOOT_CONFIG:=mt7620_mt7530_rfb
  BUILD_DEVICES:=ralink_mt7620a-mt7530-evb
  BUILD_TARGET:=ramips
  BUILD_SUBTARGET:=mt7620
  UBOOT_IMAGE:=u-boot-with-spl.bin
endef

define U-Boot/mt7621_rfb
  NAME:=MT7621 Reference Board
  UBOOT_CONFIG:=mt7621_rfb
  BUILD_DEVICES:=mediatek_mt7621-eval-board
  BUILD_TARGET:=ramips
  BUILD_SUBTARGET:=mt7621
  UBOOT_IMAGE:=u-boot-mt7621.bin
endef

define U-Boot/mt7621_nand_rfb
  NAME:=MT7621 Reference Board (NAND)
  UBOOT_CONFIG:=mt7621_nand_rfb
  BUILD_DEVICES:=mediatek_mt7621-eval-board
  BUILD_TARGET:=ramips
  BUILD_SUBTARGET:=mt7621
  UBOOT_IMAGE:=u-boot-mt7621.bin
endef

define U-Boot/mt7621_zbtlink_zbt-wg3526-16m
  NAME:=Zbtlink ZBT-WG3526-16m
  UBOOT_CONFIG:=mt7621_zbtlink_zbt-wg3526-16m
  BUILD_DEVICES:=zbtlink_zbt-wg3526-16m
  BUILD_TARGET:=ramips
  BUILD_SUBTARGET:=mt7621
  UBOOT_IMAGE:=u-boot-mt7621.bin
endef

define U-Boot/mt7622_rfb1
  NAME:=MT7622 Reference Board 1
  UBOOT_CONFIG:=mt7622_rfb
  BUILD_DEVICES:=mediatek_mt7622-rfb1 mediatek_mt7622-rfb1-ubi
  BUILD_SUBTARGET:=mt7622
endef

define U-Boot/mt7622_linksys_e8450
  NAME:=Linksys E8450
  UBOOT_CONFIG:=mt7622_linksys_e8450
  BUILD_DEVICES:=linksys_e8450-ubi
  BUILD_SUBTARGET:=mt7622
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=snand-ubi
  BL2_DDRBLOB:=1
  DEPENDS:=+trusted-firmware-a-mt7622-snand-ubi-1ddr
endef

define U-Boot/mt7622_bananapi_bpi-r64-emmc
  NAME:=BananaPi R64 (eMMC)
  UBOOT_CONFIG:=mt7622_bananapi_bpi-r64-emmc
  BUILD_DEVICES:=bananapi_bpi-r64
  BUILD_SUBTARGET:=mt7622
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=emmc
  BL2_DDRBLOB:=2
  DEPENDS:=+trusted-firmware-a-mt7622-emmc-2ddr
endef

define U-Boot/mt7622_bananapi_bpi-r64-sdmmc
  NAME:=BananaPi R64 (SDMMC)
  UBOOT_CONFIG:=mt7622_bananapi_bpi-r64-sdmmc
  BUILD_DEVICES:=bananapi_bpi-r64
  BUILD_SUBTARGET:=mt7622
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=sdmmc
  BL2_DDRBLOB:=2
  DEPENDS:=+trusted-firmware-a-mt7622-sdmmc-2ddr
endef

define U-Boot/mt7622_bananapi_bpi-r64-snand
  NAME:=BananaPi R64 (SNAND)
  UBOOT_CONFIG:=mt7622_bananapi_bpi-r64-snand
  BUILD_DEVICES:=bananapi_bpi-r64
  BUILD_SUBTARGET:=mt7622
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=snand-ubi
  BL2_DDRBLOB:=2
  DEPENDS:=+trusted-firmware-a-mt7622-snand-ubi-2ddr
endef

define U-Boot/mt7622_ubnt_unifi-6-lr-v1
  NAME:=Ubiquiti UniFi 6 LR
  UBOOT_CONFIG:=mt7622_ubnt_unifi-6-lr-v1
  BUILD_DEVICES:=ubnt_unifi-6-lr-v1-ubootmod
  BUILD_SUBTARGET:=mt7622
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=nor
  BL2_DDRBLOB:=2
  DEPENDS:=+trusted-firmware-a-mt7622-nor-2ddr
  FIP_COMPRESS:=1
endef

define U-Boot/mt7622_ubnt_unifi-6-lr-v2
  NAME:=Ubiquiti UniFi 6 LR v2
  UBOOT_CONFIG:=mt7622_ubnt_unifi-6-lr-v2
  BUILD_DEVICES:=ubnt_unifi-6-lr-v2-ubootmod
  BUILD_SUBTARGET:=mt7622
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=nor
  BL2_DDRBLOB:=2
  DEPENDS:=+trusted-firmware-a-mt7622-nor-2ddr
  FIP_COMPRESS:=1
endef

define U-Boot/mt7622_ubnt_unifi-6-lr-v3
  NAME:=Ubiquiti UniFi 6 LR v3
  UBOOT_CONFIG:=mt7622_ubnt_unifi-6-lr-v3
  BUILD_DEVICES:=ubnt_unifi-6-lr-v3-ubootmod
  BUILD_SUBTARGET:=mt7622
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=nor
  BL2_DDRBLOB:=2
  DEPENDS:=+trusted-firmware-a-mt7622-nor-2ddr
  FIP_COMPRESS:=1
endef

define U-Boot/mt7622_xiaomi_redmi-router-ax6s-ubi-loader
  NAME:=Xiaomi Redmi Router AX6S (as UBI loader)
  UBOOT_CONFIG:=mt7622_xiaomi_redmi-router-ax6s-ubi-loader
  BUILD_DEVICES:=xiaomi_redmi-router-ax6s
  BUILD_SUBTARGET:=mt7622
  UBOOT_IMAGE:=u-boot.bin
endef

define U-Boot/mt7623a_unielec_u7623
  NAME:=UniElec U7623 (mt7623)
  BUILD_DEVICES:=unielec_u7623-02
  BUILD_SUBTARGET:=mt7623
  UBOOT_CONFIG:=mt7623a_unielec_u7623_02
endef

define U-Boot/mt7623n_bpir2
  NAME:=Banana Pi R2 (mt7623)
  BUILD_DEVICES:=bananapi_bpi-r2
  BUILD_SUBTARGET:=mt7623
  UBOOT_IMAGE:=u-boot.bin
  UBOOT_CONFIG:=mt7623n_bpir2
endef

define U-Boot/mt7628_rfb
  NAME:=MT7628 Reference Board
  BUILD_DEVICES:=mediatek_mt7628an-eval-board
  BUILD_TARGET:=ramips
  BUILD_SUBTARGET:=mt76x8
  UBOOT_CONFIG:=mt7628_rfb
  UBOOT_IMAGE:=u-boot-with-spl.bin
endef

define U-Boot/mt7628_ravpower_rp-wd009
  NAME:=RAVPower RP-WD009
  BUILD_TARGET:=ramips
  BUILD_DEVICES:=ravpower_rp-wd009
  BUILD_SUBTARGET:=mt76x8
  UBOOT_CONFIG:=ravpower-rp-wd009-ram
  UBOOT_IMAGE:=u-boot.bin
endef

define U-Boot/mt7629_rfb
  NAME:=MT7629 Reference Board
  BUILD_SUBTARGET:=mt7629
  BUILD_DEVICES:=mediatek_mt7629-rfb
  UBOOT_CONFIG:=mt7629_rfb
endef

define U-Boot/mt7981_cmcc_rax3000m-emmc
  NAME:=CMCC RAX3000M
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=cmcc_rax3000m
  UBOOT_CONFIG:=mt7981_cmcc_rax3000m-emmc
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=emmc
  BL2_SOC:=mt7981
  BL2_DDRTYPE:=ddr4
  DEPENDS:=+trusted-firmware-a-mt7981-emmc-ddr4
endef

define U-Boot/mt7981_cmcc_rax3000m-nand
  NAME:=CMCC RAX3000M
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=cmcc_rax3000m
  UBOOT_CONFIG:=mt7981_cmcc_rax3000m-nand
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=spim-nand
  BL2_SOC:=mt7981
  BL2_DDRTYPE:=ddr4
  DEPENDS:=+trusted-firmware-a-mt7981-spim-nand-ddr4
endef

define U-Boot/mt7981_h3c_magic-nx30-pro
  NAME:=H3C Magic NX30 Pro
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=h3c_magic-nx30-pro
  UBOOT_CONFIG:=mt7981_h3c_magic-nx30-pro
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=spim-nand
  BL2_SOC:=mt7981
  BL2_DDRTYPE:=ddr3
  DEPENDS:=+trusted-firmware-a-mt7981-spim-nand-ddr3
endef

define U-Boot/mt7981_jcg_q30-pro
  NAME:=JCG Q30 PRO
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=jcg_q30-pro
  UBOOT_CONFIG:=mt7981_jcg_q30-pro
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=spim-nand
  BL2_SOC:=mt7981
  BL2_DDRTYPE:=ddr3
  DEPENDS:=+trusted-firmware-a-mt7981-spim-nand-ddr3
endef

define U-Boot/mt7981_nokia_ea0326gmp
  NAME:=Nokia EA0326GMP
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=nokia_ea0326gmp
  UBOOT_CONFIG:=mt7981_nokia_ea0326gmp
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=spim-nand
  BL2_SOC:=mt7981
  BL2_DDRTYPE:=ddr3
  DEPENDS:=+trusted-firmware-a-mt7981-spim-nand-ddr3
endef

define U-Boot/mt7981_openwrt_one-snand
  NAME:=OpenWrt One NAND
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=openwrt_one
  UBOOT_CONFIG:=mt7981_openwrt-one-spi-nand
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=spim-nand-ubi
  BL2_SOC:=mt7981
  BL2_DDRTYPE:=ddr4
  DEPENDS:=+trusted-firmware-a-mt7981-spim-nand-ubi-ddr4
endef

define U-Boot/mt7981_openwrt_one-nor
  NAME:=OpenWrt One NOR
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=openwrt_one
  UBOOT_CONFIG:=mt7981_openwrt-one-nor
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=nor
  BL2_SOC:=mt7981
  BL2_DDRTYPE:=ddr4
  FIP_COMPRESS:=1
  DEPENDS:=+trusted-firmware-a-mt7981-nor-ddr4
endef

define U-Boot/mt7981_rfb-spim-nand
  NAME:=MT7981 Reference Board
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=mediatek_mt7981-rfb
  UBOOT_CONFIG:=mt7981_rfb
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=spim-nand
  BL2_SOC:=mt7981
  BL2_DDRTYPE:=ddr3
  DEPENDS:=+trusted-firmware-a-mt7981-spim-nand-ddr3
endef

define U-Boot/mt7981_rfb-emmc
  NAME:=MT7981 Reference Board
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=mediatek_mt7981-rfb
  UBOOT_CONFIG:=mt7981_emmc_rfb
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=emmc
  BL2_SOC:=mt7981
  BL2_DDRTYPE:=ddr3
  DEPENDS:=+trusted-firmware-a-mt7981-emmc-ddr3
endef

define U-Boot/mt7981_rfb-nor
  NAME:=MT7981 Reference Board
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=mediatek_mt7981-rfb
  UBOOT_CONFIG:=mt7981_nor_rfb
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=spim-nand
  BL2_SOC:=mt7981
  BL2_DDRTYPE:=ddr3
  DEPENDS:=+trusted-firmware-a-mt7981-nor-ddr3
endef

define U-Boot/mt7981_rfb-sd
  NAME:=MT7981 Reference Board
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=mediatek_mt7981-rfb
  UBOOT_CONFIG:=mt7981_sd_rfb
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=sdmmc
  BL2_SOC:=mt7981
  BL2_DDRTYPE:=ddr3
  DEPENDS:=+trusted-firmware-a-mt7981-sdmmc-ddr3
endef

define U-Boot/mt7981_rfb-snfi
  NAME:=MT7981 Reference Board
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=mediatek_mt7981-rfb
  UBOOT_CONFIG:=mt7981_snfi_nand_rfb
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=snand
  BL2_SOC:=mt7981
  BL2_DDRTYPE:=ddr3
  DEPENDS:=+trusted-firmware-a-mt7981-snand-ddr3
endef

define U-Boot/mt7981_qihoo_360t7
  NAME:=Qihoo 360T7
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=qihoo_360t7
  UBOOT_CONFIG:=mt7981_qihoo-360t7
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=spim-nand
  BL2_SOC:=mt7981
  BL2_DDRTYPE:=ddr3
  DEPENDS:=+trusted-firmware-a-mt7981-spim-nand-ddr3
endef

define U-Boot/mt7981_xiaomi_mi-router-ax3000t
  NAME:=Xiaomi Router AX3000T
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=xiaomi_mi-router-ax3000t-ubootmod
  UBOOT_CONFIG:=mt7981_xiaomi_mi-router-ax3000t
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=spim-nand
  BL2_SOC:=mt7981
  BL2_DDRTYPE:=ddr3
  DEPENDS:=+trusted-firmware-a-mt7981-spim-nand-ddr3
endef

define U-Boot/mt7981_xiaomi_mi-router-wr30u
  NAME:=Xiaomi Router WR30U
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=xiaomi_mi-router-wr30u-ubootmod
  UBOOT_CONFIG:=mt7981_xiaomi_mi-router-wr30u
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=spim-nand
  BL2_SOC:=mt7981
  BL2_DDRTYPE:=ddr3
  DEPENDS:=+trusted-firmware-a-mt7981-spim-nand-ddr3
endef

define U-Boot/mt7986_rfb
  NAME:=MT7986 Reference Board
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=mediatek_mt7986-rfb
  UBOOT_CONFIG:=mt7986_rfb
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=sdmmc
  BL2_SOC:=mt7986
  BL2_DDRTYPE:=ddr4
  DEPENDS:=+trusted-firmware-a-mt7986-sdmmc-ddr4
endef

define U-Boot/mt7986_bananapi_bpi-r3-emmc
  NAME:=BananaPi BPi-R3
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=bananapi_bpi-r3
  UBOOT_CONFIG:=mt7986a_bpi-r3-emmc
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=emmc
  BL2_SOC:=mt7986
  BL2_DDRTYPE:=ddr4
  DEPENDS:=+trusted-firmware-a-mt7986-emmc-ddr4
endef

define U-Boot/mt7986_bananapi_bpi-r3-sdmmc
  NAME:=BananaPi BPi-R3
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=bananapi_bpi-r3
  UBOOT_CONFIG:=mt7986a_bpi-r3-sd
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=sdmmc
  BL2_SOC:=mt7986
  BL2_DDRTYPE:=ddr4
  DEPENDS:=+trusted-firmware-a-mt7986-sdmmc-ddr4
endef

define U-Boot/mt7986_bananapi_bpi-r3-snand
  NAME:=BananaPi BPi-R3
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=bananapi_bpi-r3
  UBOOT_CONFIG:=mt7986a_bpi-r3-snand
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=spim-nand-ubi
  BL2_SOC:=mt7986
  BL2_DDRTYPE:=ddr4
  DEPENDS:=+trusted-firmware-a-mt7986-spim-nand-ubi-ddr4
endef

define U-Boot/mt7986_bananapi_bpi-r3-nor
  NAME:=BananaPi BPi-R3
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=bananapi_bpi-r3
  UBOOT_CONFIG:=mt7986a_bpi-r3-nor
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=nor
  BL2_SOC:=mt7986
  BL2_DDRTYPE:=ddr4
  DEPENDS:=+trusted-firmware-a-mt7986-nor-ddr4
  FIP_COMPRESS:=1
endef

define U-Boot/mt7986_bananapi_bpi-r3-mini-emmc
  NAME:=BananaPi BPi-R3 Mini
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=bananapi_bpi-r3-mini
  UBOOT_CONFIG:=mt7986a_bpi-r3-mini-emmc
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=emmc
  BL2_SOC:=mt7986
  BL2_DDRTYPE:=ddr4
  DEPENDS:=+trusted-firmware-a-mt7986-emmc-ddr4
endef

define U-Boot/mt7986_bananapi_bpi-r3-mini-snand
  NAME:=BananaPi BPi-R3 Mini
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=bananapi_bpi-r3-mini
  UBOOT_CONFIG:=mt7986a_bpi-r3-mini-snand
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=spim-nand-ubi
  BL2_SOC:=mt7986
  BL2_DDRTYPE:=ddr4
  DEPENDS:=+trusted-firmware-a-mt7986-spim-nand-ubi-ddr4
endef

define U-Boot/mt7986_glinet_gl-mt6000
  NAME:=GL.iNet GL-MT6000
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=glinet_gl-mt6000
  UBOOT_CONFIG:=mt7986a_glinet_gl-mt6000
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=emmc
  BL2_SOC:=mt7986
  BL2_DDRTYPE:=ddr4
  DEPENDS:=+trusted-firmware-a-mt7986-emmc-ddr4
endef

define U-Boot/mt7986_jdcloud_re-cp-03
  NAME:=JDCloud RE-CP-03
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=jdcloud_re-cp-03
  UBOOT_CONFIG:=mt7986a_jdcloud_re-cp-03
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=emmc
  BL2_SOC:=mt7986
  BL2_DDRTYPE:=ddr4
  DEPENDS:=+trusted-firmware-a-mt7986-emmc-ddr4
endef

define U-Boot/mt7986_netcore_n60
  NAME:=Netcore N60
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=netcore_n60
  UBOOT_CONFIG:=mt7986_netcore_n60
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=spim-nand
  BL2_SOC:=mt7986
  BL2_DDRTYPE:=ddr3
  DEPENDS:=+trusted-firmware-a-mt7986-spim-nand-ddr3
endef

define U-Boot/mt7986_tplink_tl-xdr4288
  NAME:=TP-LINK TL-XDR4288
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=tplink_tl-xdr4288
  UBOOT_CONFIG:=mt7986_tplink_tl-xdr4288
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=spim-nand
  BL2_SOC:=mt7986
  BL2_DDRTYPE:=ddr3
  DEPENDS:=+trusted-firmware-a-mt7986-spim-nand-ddr3
endef

define U-Boot/mt7986_tplink_tl-xdr6086
  NAME:=TP-LINK TL-XDR6086
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=tplink_tl-xdr6086
  UBOOT_CONFIG:=mt7986_tplink_tl-xdr6086
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=spim-nand
  BL2_SOC:=mt7986
  BL2_DDRTYPE:=ddr3
  DEPENDS:=+trusted-firmware-a-mt7986-spim-nand-ddr3
endef

define U-Boot/mt7986_tplink_tl-xdr6088
  NAME:=TP-LINK TL-XDR6088
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=tplink_tl-xdr6088
  UBOOT_CONFIG:=mt7986_tplink_tl-xdr6088
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=spim-nand
  BL2_SOC:=mt7986
  BL2_DDRTYPE:=ddr3
  DEPENDS:=+trusted-firmware-a-mt7986-spim-nand-ddr3
endef

define U-Boot/mt7986_xiaomi_redmi-router-ax6000
  NAME:=Xiaomi Redmi AX6000
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=xiaomi_redmi-router-ax6000-ubootmod
  UBOOT_CONFIG:=mt7986_xiaomi_redmi-ax6000
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=spim-nand
  BL2_SOC:=mt7986
  BL2_DDRTYPE:=ddr4
  DEPENDS:=+trusted-firmware-a-mt7986-spim-nand-ddr4
endef

define U-Boot/mt7986_zyxel_ex5601-t0
  NAME:=Zyxel EX5601-T0
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=zyxel_ex5601-t0-ubootmod
  UBOOT_CONFIG:=mt7986_zyxel_ex5601-t0
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=spim-nand-4k
  BL2_SOC:=mt7986
  BL2_DDRTYPE:=ddr4
  DEPENDS:=+trusted-firmware-a-mt7986-spim-nand-4k-ddr4
endef

define U-Boot/mt7988_bananapi_bpi-r4-emmc
  NAME:=BananaPi BPi-R4
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=bananapi_bpi-r4
  UBOOT_CONFIG:=mt7988a_bananapi_bpi-r4-emmc
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=emmc
  BL2_SOC:=mt7988
  BL2_DDRTYPE:=comb
  DEPENDS:=+trusted-firmware-a-mt7988-emmc-comb
endef

define U-Boot/mt7988_bananapi_bpi-r4-sdmmc
  NAME:=BananaPi BPi-R4
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=bananapi_bpi-r4
  UBOOT_CONFIG:=mt7988a_bananapi_bpi-r4-sdmmc
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=sdmmc
  BL2_SOC:=mt7988
  BL2_DDRTYPE:=comb
  DEPENDS:=+trusted-firmware-a-mt7988-sdmmc-comb
endef

define U-Boot/mt7988_bananapi_bpi-r4-snand
  NAME:=BananaPi BPi-R4
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=bananapi_bpi-r4
  UBOOT_CONFIG:=mt7988a_bananapi_bpi-r4-snand
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=spim-nand-ubi
  BL2_SOC:=mt7988
  BL2_DDRTYPE:=comb
  DEPENDS:=+trusted-firmware-a-mt7988-spim-nand-ubi-comb
endef

define U-Boot/mt7988_bananapi_bpi-r4-poe-emmc
  NAME:=BananaPi BPi-R4 2.5GE
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=bananapi_bpi-r4-poe
  UBOOT_CONFIG:=mt7988a_bananapi_bpi-r4-poe-emmc
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=emmc
  BL2_SOC:=mt7988
  BL2_DDRTYPE:=comb
  DEPENDS:=+trusted-firmware-a-mt7988-emmc-comb
endef

define U-Boot/mt7988_bananapi_bpi-r4-poe-sdmmc
  NAME:=BananaPi BPi-R4 2.5GE
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=bananapi_bpi-r4-poe
  UBOOT_CONFIG:=mt7988a_bananapi_bpi-r4-poe-sdmmc
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=sdmmc
  BL2_SOC:=mt7988
  BL2_DDRTYPE:=comb
  DEPENDS:=+trusted-firmware-a-mt7988-sdmmc-comb
endef

define U-Boot/mt7988_bananapi_bpi-r4-poe-snand
  NAME:=BananaPi BPi-R4 2.5GE
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=bananapi_bpi-r4-poe
  UBOOT_CONFIG:=mt7988a_bananapi_bpi-r4-poe-snand
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=spim-nand-ubi
  BL2_SOC:=mt7988
  BL2_DDRTYPE:=comb
  DEPENDS:=+trusted-firmware-a-mt7988-spim-nand-ubi-comb
endef

define U-Boot/mt7988_rfb-spim-nand
  NAME:=MT7988 Reference Board
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=mediatek_mt7988a-rfb
  UBOOT_CONFIG:=mt7988_rfb
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=spim-nand
  BL2_SOC:=mt7988
  BL2_DDRTYPE:=comb
  DEPENDS:=+trusted-firmware-a-mt7988-spim-nand-comb
endef

define U-Boot/mt7988_rfb-snand
  NAME:=MT7988 Reference Board
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=mediatek_mt7988a-rfb
  UBOOT_CONFIG:=mt7988_rfb
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=snand
  BL2_SOC:=mt7988
  BL2_DDRTYPE:=comb
  DEPENDS:=+trusted-firmware-a-mt7988-snand-comb
endef

define U-Boot/mt7988_rfb-nor
  NAME:=MT7988 Reference Board
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=mediatek_mt7988a-rfb
  UBOOT_CONFIG:=mt7988_rfb
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=nor
  BL2_SOC:=mt7988
  BL2_DDRTYPE:=comb
  DEPENDS:=+trusted-firmware-a-mt7988-nor-comb
  FIP_COMPRESS:=1
endef

define U-Boot/mt7988_rfb-emmc
  NAME:=MT7988 Reference Board
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=mediatek_mt7988a-rfb
  UBOOT_CONFIG:=mt7988_rfb
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=emmc
  BL2_SOC:=mt7988
  BL2_DDRTYPE:=comb
  DEPENDS:=+trusted-firmware-a-mt7988-emmc-comb
endef

define U-Boot/mt7988_rfb-sd
  NAME:=MT7988 Reference Board
  BUILD_SUBTARGET:=filogic
  BUILD_DEVICES:=mediatek_mt7988a-rfb
  UBOOT_CONFIG:=mt7988_sd_rfb
  UBOOT_IMAGE:=u-boot.fip
  BL2_BOOTDEV:=sdmmc
  BL2_SOC:=mt7988
  BL2_DDRTYPE:=comb
  DEPENDS:=+trusted-firmware-a-mt7988-sdmmc-comb
endef

UBOOT_TARGETS := \
	mt7620_mt7530_rfb \
	mt7620_rfb \
	mt7621_nand_rfb \
	mt7621_rfb \
	mt7621_zbtlink_zbt-wg3526-16m \
	mt7622_bananapi_bpi-r64-emmc \
	mt7622_bananapi_bpi-r64-sdmmc \
	mt7622_bananapi_bpi-r64-snand \
	mt7622_linksys_e8450 \
	mt7622_rfb1 \
	mt7622_ubnt_unifi-6-lr-v1 \
	mt7622_ubnt_unifi-6-lr-v2 \
	mt7622_ubnt_unifi-6-lr-v3 \
	mt7622_xiaomi_redmi-router-ax6s-ubi-loader \
	mt7623n_bpir2 \
	mt7623a_unielec_u7623 \
	mt7628_rfb \
	mt7628_ravpower_rp-wd009 \
	mt7629_rfb \
	mt7981_cmcc_rax3000m-emmc \
	mt7981_cmcc_rax3000m-nand \
	mt7981_h3c_magic-nx30-pro \
	mt7981_jcg_q30-pro \
	mt7981_nokia_ea0326gmp \
	mt7981_openwrt_one-snand \
	mt7981_openwrt_one-nor \
	mt7981_rfb-spim-nand \
	mt7981_rfb-emmc \
	mt7981_rfb-nor \
	mt7981_rfb-sd \
	mt7981_rfb-snfi \
	mt7981_qihoo_360t7 \
	mt7981_xiaomi_mi-router-ax3000t \
	mt7981_xiaomi_mi-router-wr30u \
	mt7986_bananapi_bpi-r3-emmc \
	mt7986_bananapi_bpi-r3-sdmmc \
	mt7986_bananapi_bpi-r3-snand \
	mt7986_bananapi_bpi-r3-nor \
	mt7986_bananapi_bpi-r3-mini-emmc \
	mt7986_bananapi_bpi-r3-mini-snand \
	mt7986_glinet_gl-mt6000 \
	mt7986_jdcloud_re-cp-03 \
	mt7986_netcore_n60 \
	mt7986_tplink_tl-xdr4288 \
	mt7986_tplink_tl-xdr6086 \
	mt7986_tplink_tl-xdr6088 \
	mt7986_xiaomi_redmi-router-ax6000 \
	mt7986_zyxel_ex5601-t0 \
	mt7986_rfb \
	mt7988_bananapi_bpi-r4-emmc \
	mt7988_bananapi_bpi-r4-sdmmc \
	mt7988_bananapi_bpi-r4-snand \
	mt7988_bananapi_bpi-r4-poe-emmc \
	mt7988_bananapi_bpi-r4-poe-sdmmc \
	mt7988_bananapi_bpi-r4-poe-snand \
	mt7988_rfb-spim-nand \
	mt7988_rfb-snand \
	mt7988_rfb-nor \
	mt7988_rfb-emmc \
	mt7988_rfb-sd

ifdef CONFIG_TARGET_mediatek
UBOOT_MAKE_FLAGS += $(UBOOT_IMAGE:.fip=.bin)
endif

define Build/fip-image
	$(if $(FIP_COMPRESS),\
		xz -f -e -k -9 -C crc32 $(STAGING_DIR_IMAGE)/$(if $(BL2_SOC),$(BL2_SOC),$(BUILD_SUBTARGET))-$(BL2_BOOTDEV)-$(if $(BL2_DDRTYPE),$(BL2_DDRTYPE)-)$(if $(BL2_DDRBLOB),$(BL2_DDRBLOB)ddr-)bl31.bin ;\
		xz -f -e -k -9 -C crc32 $(PKG_BUILD_DIR)/u-boot.bin \
	)
	$(STAGING_DIR_HOST)/bin/fiptool create \
		--soc-fw $(STAGING_DIR_IMAGE)/$(if $(BL2_SOC),$(BL2_SOC),$(BUILD_SUBTARGET))-$(BL2_BOOTDEV)-$(if $(BL2_DDRTYPE),$(BL2_DDRTYPE)-)$(if $(BL2_DDRBLOB),$(BL2_DDRBLOB)ddr-)bl31.bin$(if $(FIP_COMPRESS),.xz) \
		--nt-fw $(PKG_BUILD_DIR)/u-boot.bin$(if $(FIP_COMPRESS),.xz) \
		$(PKG_BUILD_DIR)/u-boot.fip
endef

ifdef CONFIG_TARGET_ramips_mt7621
define Build/Prepare
	$(call Build/Prepare/Default)
ifdef CONFIG_DEBUG
	$(CP) $(DL_DIR)/mt7621_stage_sram.bin $(PKG_BUILD_DIR)/
else
	$(CP) $(DL_DIR)/mt7621_stage_sram_noprint.bin $(PKG_BUILD_DIR)/mt7621_stage_sram.bin
endif
endef
endif

define Build/Configure
	$(call Build/Configure/U-Boot)
	sed -i 's/CONFIG_TOOLS_LIBCRYPTO=y/# CONFIG_TOOLS_LIBCRYPTO is not set/' $(PKG_BUILD_DIR)/.config
endef

define Build/Compile
	$(call Build/Compile/U-Boot)
ifeq ($(UBOOT_IMAGE),u-boot.fip)
	$(call Build/fip-image)
endif
endef

# don't stage files to bindir, let target/linux/mediatek/image/*.mk do that
ifdef CONFIG_TARGET_mediatek
define Package/u-boot/install
endef
endif

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(UBOOT_IMAGE) $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-$(UBOOT_IMAGE)
endef

$(eval $(call BuildPackage/U-Boot))
