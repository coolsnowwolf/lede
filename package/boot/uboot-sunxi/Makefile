#
# Copyright (C) 2013-2016 OpenWrt.org
# Copyright (C) 2017 Yousong Zhou
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=u-boot
PKG_VERSION:=2016.03
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=ftp://ftp.denx.de/pub/u-boot
PKG_HASH:=e49337262ecac44dbdeac140f2c6ebd1eba345e0162b0464172e7f05583ed7bb
PKG_MAINTAINER:=Zoltan HERPAI <wigyori@uid0.hu>
PKG_LICENSE:=GPL-2.0 GPL-2.0+
PKG_LICENSE_FILES:=Licenses/README

PKG_BUILD_PARALLEL:=1

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/uboot/template
define Package/uboot-sunxi-$(1)
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_sunxi
  TITLE:=U-Boot for $(2)
  URL:=http://www.denx.de/wiki/U-Boot
  VARIANT:=$(1)
endef

define Package/uboot-sunxi-$(1)/install
	$(CP) $(PKG_BUILD_DIR)/u-boot-sunxi-with-spl.bin \
		$(KERNEL_BUILD_DIR)/uboot-$(BOARD)-$(1)-u-boot-with-spl.bin
	mkimage -C none -A arm -T script -d uEnv-$(if $(3),$(3),default).txt \
		$(KERNEL_BUILD_DIR)/uboot-$(BOARD)-$(1)-boot.scr
endef

UBOOT_SUNXI_PACKAGES += uboot-sunxi-$(1)

endef

$(eval $(call Package/uboot/template,A10-OLinuXino-Lime,the A10 OLinuXino LIME))
$(eval $(call Package/uboot/template,A13-OLinuXino,the A13 OlinuXino))
$(eval $(call Package/uboot/template,A20-OLinuXino-Lime,the A20 OLinuXino LIME))
$(eval $(call Package/uboot/template,A20-OLinuXino_MICRO,A20 OLinuXino MICRO))
$(eval $(call Package/uboot/template,Bananapi,Bananapi))
$(eval $(call Package/uboot/template,Bananapro,Bananapro))
$(eval $(call Package/uboot/template,Cubieboard,Cubieboard))
$(eval $(call Package/uboot/template,Cubieboard2,Cubieboard2))
$(eval $(call Package/uboot/template,Cubietruck,Cubietruck))
$(eval $(call Package/uboot/template,Hummingbird_A31,the Hummingbird A31 board))
$(eval $(call Package/uboot/template,Mele_M9,the Mele M9 (A31)))
$(eval $(call Package/uboot/template,OLIMEX_A13_SOM,the Olimex A13 SOM))
$(eval $(call Package/uboot/template,Linksprite_pcDuino,Linksprite pcDuino))
$(eval $(call Package/uboot/template,Linksprite_pcDuino3,Linksprite pcDuino3))
$(eval $(call Package/uboot/template,Lamobo_R1,Lamobo R1))
$(eval $(call Package/uboot/template,orangepi_plus,Orange Pi Plus (H3)))
$(eval $(call Package/uboot/template,pangolin,Theobroma A31-yQ7 devboard,pangolin))

define Package/uboot-sunxi-all
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_sunxi
  TITLE:=Meta package for selecting all variants of uboot-sunxi
  URL:=http://www.denx.de/wiki/U-Boot
  DEPENDS:=$(patsubst %,+%,$(UBOOT_SUNXI_PACKAGES))
endef

ifdef BUILD_VARIANT
define Build/Configure
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		USE_PRIVATE_LIBGCC=yes $(BUILD_VARIANT)_defconfig
endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		CROSS_COMPILE=$(TARGET_CROSS) \
		DTCDIR=$(LINUX_DIR)/scripts/dtc/
endef
else	# BUILD_VARIANT
define Build/Prepare
endef
define Build/Configure
endef
define Build/Compile
endef
endif	# BUILD_VARIANT

$(foreach pkg,$(UBOOT_SUNXI_PACKAGES), \
  $(eval $(call BuildPackage,$(pkg))) \
)
$(eval $(call BuildPackage,uboot-sunxi-all))
