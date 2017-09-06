#
# Copyright (C) 2013-2016 OpenWrt.org
# Copyright (C) 2017 Yousong Zhou
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2016.03

PKG_HASH:=e49337262ecac44dbdeac140f2c6ebd1eba345e0162b0464172e7f05583ed7bb

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
  NAME:=A10 OLinuXino LIME
  BUILD_DEVICES:=sun4i-a10-olinuxino-lime
endef

define U-Boot/A13-OLinuXino
  NAME:=A13 OlinuXino
  BUILD_DEVICES:=sun5i-a13-olinuxino
endef

define U-Boot/A20-OLinuXino-Lime
  NAME:=A20 OLinuXino LIME
  BUILD_DEVICES:=sun7i-a20-olinuxino-lime
endef

define U-Boot/A20-OLinuXino_MICRO
  NAME:=A20 OLinuXino MICRO
  BUILD_DEVICES:=sun7i-a20-olinuxino-micro
endef

define U-Boot/Bananapi
  NAME:=Bananapi
  BUILD_DEVICES:=sun7i-a20-bananapi
endef

define U-Boot/Bananapro
  NAME:=Bananapro
  BUILD_DEVICES:=sun7i-a20-bananapro
endef

define U-Boot/Cubieboard
  NAME:=Cubieboard
  BUILD_DEVICES:=sun4i-a10-cubieboard
endef

define U-Boot/Cubieboard2
  NAME:=Cubieboard2
  BUILD_DEVICES:=sun7i-a20-cubieboard2
endef

define U-Boot/Cubietruck
  NAME:=Cubietruck
  BUILD_DEVICES:=sun7i-a20-cubietruck
endef

define U-Boot/Hummingbird_A31
  NAME:=Hummingbird A31 board
endef

define U-Boot/Mele_M9
  NAME:=Mele M9 (A31)
  BUILD_DEVICES:=sun6i-a31-m9
endef

define U-Boot/OLIMEX_A13_SOM
  NAME:=Olimex A13 SOM
  BUILD_DEVICES:=sun5i-a13-olimex-som
endef

define U-Boot/Linksprite_pcDuino
  NAME:=Linksprite pcDuino
  BUILD_DEVICES:=sun4i-a10-pcduino
endef

define U-Boot/Linksprite_pcDuino3
  NAME:=Linksprite pcDuino3
  BUILD_DEVICES:=sun7i-a20-pcduino3
endef

define U-Boot/Lamobo_R1
  NAME:=Lamobo R1
  BUILD_DEVICES:=sun7i-a20-lamobo-r1
endef

define U-Boot/orangepi_plus
  NAME:=Orange Pi Plus (H3)
  BUILD_DEVICES:=sun8i-h3-orangepi-plus
endef

define U-Boot/pangolin
  NAME:=Theobroma A31-yQ7 devboard
  UENV:=pangolin
endef

UBOOT_TARGETS := \
	A10-OLinuXino-Lime \
	A13-OLinuXino \
	A20-OLinuXino-Lime \
	A20-OLinuXino_MICRO \
	Bananapi \
	Bananapro \
	Cubieboard \
	Cubieboard2 \
	Cubietruck \
	Hummingbird_A31 \
	Mele_M9 \
	OLIMEX_A13_SOM \
	Linksprite_pcDuino \
	Linksprite_pcDuino3 \
	Lamobo_R1 \
	orangepi_plus \
	pangolin

UBOOT_CONFIGURE_VARS += USE_PRIVATE_LIBGCC=yes

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/$(UBOOT_IMAGE) $(STAGING_DIR_IMAGE)/$(BUILD_DEVICES)-u-boot-with-spl.bin
	mkimage -C none -A arm -T script -d uEnv-$(UENV).txt \
		$(STAGING_DIR_IMAGE)/$(BUILD_DEVICES)-boot.scr
endef

define Package/u-boot/install/default
endef

$(eval $(call BuildPackage/U-Boot))
