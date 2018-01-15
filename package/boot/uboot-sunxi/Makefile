#
# Copyright (C) 2013-2016 OpenWrt.org
# Copyright (C) 2017 Yousong Zhou
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2017.07

PKG_HASH:=5374bfdc8acb9a38c025371b1ff20f45e7533668e84e685d0df5d9e7c0e4feff

PKG_MAINTAINER:=Zoltan HERPAI <wigyori@uid0.hu>

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=sunxi
  UBOOT_IMAGE:=u-boot-sunxi-with-spl.bin
  UENV:=default
  HIDDEN:=1
endef

define U-Boot/A10-OLinuXino-Lime
  BUILD_SUBTARGET:=cortexa8
  NAME:=A10 OLinuXino LIME
  BUILD_DEVICES:=sun4i-a10-olinuxino-lime
endef

define U-Boot/A13-OLinuXino
  BUILD_SUBTARGET:=cortexa8
  NAME:=A13 OlinuXino
  BUILD_DEVICES:=sun5i-a13-olinuxino
endef

define U-Boot/A20-OLinuXino-Lime
  BUILD_SUBTARGET:=cortexa7
  NAME:=A20 OLinuXino LIME
  BUILD_DEVICES:=sun7i-a20-olinuxino-lime
endef

define U-Boot/A20-OLinuXino-Lime2
  BUILD_SUBTARGET:=cortexa7
  NAME:=A20 OLinuXino LIME2
  BUILD_DEVICES:=sun7i-a20-olinuxino-lime2
endef

define U-Boot/A20-OLinuXino-Lime2-eMMC
  BUILD_SUBTARGET:=cortexa7
  NAME:=A20 OLinuXino LIME2 eMMC
  BUILD_DEVICES:=sun7i-a20-olinuxino-lime2-emmc
endef

define U-Boot/A20-OLinuXino_MICRO
  BUILD_SUBTARGET:=cortexa7
  NAME:=A20 OLinuXino MICRO
  BUILD_DEVICES:=sun7i-a20-olinuxino-micro
endef

define U-Boot/Bananapi
  BUILD_SUBTARGET:=cortexa7
  NAME:=Bananapi
  BUILD_DEVICES:=sun7i-a20-bananapi
endef

define U-Boot/Bananapro
  BUILD_SUBTARGET:=cortexa7
  NAME:=Bananapro
  BUILD_DEVICES:=sun7i-a20-bananapro
endef

define U-Boot/Cubieboard
  BUILD_SUBTARGET:=cortexa8
  NAME:=Cubieboard
  BUILD_DEVICES:=sun4i-a10-cubieboard
endef

define U-Boot/Cubieboard2
  BUILD_SUBTARGET:=cortexa7
  NAME:=Cubieboard2
  BUILD_DEVICES:=sun7i-a20-cubieboard2
endef

define U-Boot/Cubietruck
  BUILD_SUBTARGET:=cortexa7
  NAME:=Cubietruck
  BUILD_DEVICES:=sun7i-a20-cubietruck
endef

define U-Boot/Hummingbird_A31
  BUILD_SUBTARGET:=cortexa7
  NAME:=Hummingbird A31 board
endef

define U-Boot/Marsboard_A10
  BUILD_SUBTARGET:=cortexa8
  NAME:=Marsboard
  BUILD_DEVICES:=sun4i-a10-marsboard
endef

define U-Boot/Mele_M9
  BUILD_SUBTARGET:=cortexa7
  NAME:=Mele M9 (A31)
  BUILD_DEVICES:=sun6i-a31-m9
endef

define U-Boot/OLIMEX_A13_SOM
  BUILD_SUBTARGET:=cortexa8
  NAME:=Olimex A13 SOM
  BUILD_DEVICES:=sun5i-a13-olimex-som
endef

define U-Boot/Linksprite_pcDuino
  BUILD_SUBTARGET:=cortexa8
  NAME:=Linksprite pcDuino
  BUILD_DEVICES:=sun4i-a10-pcduino
endef

define U-Boot/Linksprite_pcDuino3
  BUILD_SUBTARGET:=cortexa7
  NAME:=Linksprite pcDuino3
  BUILD_DEVICES:=sun7i-a20-pcduino3
endef

define U-Boot/Lamobo_R1
  BUILD_SUBTARGET:=cortexa7
  NAME:=Lamobo R1
  BUILD_DEVICES:=sun7i-a20-lamobo-r1
endef

define U-Boot/nanopi_m1_plus
  BUILD_SUBTARGET:=cortexa7
  NAME:=NanoPi M1 Plus (H3)
  BUILD_DEVICES:=sun8i-h3-nanopi-m1-plus
endef

define U-Boot/nanopi_neo
  BUILD_SUBTARGET:=cortexa7
  NAME:=U-Boot for NanoPi NEO (H3)
  BUILD_DEVICES:=sun8i-h3-nanopi-neo
endef

define U-Boot/orangepi_r1
  BUILD_SUBTARGET:=cortexa7
  NAME:=Orange Pi R1 (H2+)
  BUILD_DEVICES:=sun8i-h2-plus-orangepi-r1
endef

define U-Boot/orangepi_plus
  BUILD_SUBTARGET:=cortexa7
  NAME:=Orange Pi Plus (H3)
  BUILD_DEVICES:=sun8i-h3-orangepi-plus
endef

define U-Boot/orangepi_2
  BUILD_SUBTARGET:=cortexa7
  NAME:=Orange Pi 2 (H3)
  BUILD_DEVICES:=sun8i-h3-orangepi-2
endef

define U-Boot/pangolin
  BUILD_SUBTARGET:=cortexa7
  NAME:=Theobroma A31-yQ7 devboard
  UENV:=pangolin
endef

define U-Boot/pine64_plus
  BUILD_SUBTARGET:=cortexa53
  NAME:=Pine64 Plus A64
  BUILD_DEVICES:=sun50i-a64-pine64-plus
  DEPENDS:=+PACKAGE_u-boot-pine64_plus:arm-trusted-firmware-sunxi
  UENV:=a64
endef

UBOOT_TARGETS := \
	A10-OLinuXino-Lime \
	A13-OLinuXino \
	A20-OLinuXino-Lime \
	A20-OLinuXino-Lime2 \
	A20-OLinuXino-Lime2-eMMC \
	A20-OLinuXino_MICRO \
	Bananapi \
	Bananapro \
	Cubieboard \
	Cubieboard2 \
	Cubietruck \
	Hummingbird_A31 \
	Marsboard_A10 \
	Mele_M9 \
	OLIMEX_A13_SOM \
	Linksprite_pcDuino \
	Linksprite_pcDuino3 \
	Lamobo_R1 \
	nanopi_m1_plus \
	nanopi_neo \
	orangepi_r1 \
	orangepi_plus \
	orangepi_2 \
	pangolin \
	pine64_plus

UBOOT_CONFIGURE_VARS += USE_PRIVATE_LIBGCC=yes

UBOOT_MAKE_FLAGS += \
	BL31=$(STAGING_DIR_IMAGE)/bl31.bin

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
ifeq ($(SUBTARGET),cortexa53)
	cat $(PKG_BUILD_DIR)/spl/sunxi-spl.bin $(PKG_BUILD_DIR)/u-boot.itb > $(STAGING_DIR_IMAGE)/$(BUILD_DEVICES)-u-boot-with-spl.bin
else
	$(CP) $(PKG_BUILD_DIR)/$(UBOOT_IMAGE) $(STAGING_DIR_IMAGE)/$(BUILD_DEVICES)-u-boot-with-spl.bin
endif
	mkimage -C none -A arm -T script -d uEnv-$(UENV).txt \
		$(STAGING_DIR_IMAGE)/$(BUILD_DEVICES)-boot.scr
endef

define Package/u-boot/install/default
endef

$(eval $(call BuildPackage/U-Boot))
