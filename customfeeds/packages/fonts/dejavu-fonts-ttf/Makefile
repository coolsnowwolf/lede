#
# Copyright (C) 2008-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=dejavu-fonts-ttf
PKG_VERSION:=2.37
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@SF/dejavu
PKG_HASH:=fa9ca4d13871dd122f61258a80d01751d603b4d3ee14095d65453b4e846e17d7
PKG_LICENSE:=Bitstream-Vera-Fonts-Copyright Arev-Fonts-Copyright Public-Domain 
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Mirko Vogt <mirko-dev@nanl.de>

include $(INCLUDE_DIR)/package.mk

FILES:= \
	DejaVuSerif-Italic \
	DejaVuSerifCondensed-Italic \
	DejaVuSerifCondensed-BoldItalic \
	DejaVuSerifCondensed-Bold \
	DejaVuSerifCondensed \
	DejaVuSerif-BoldItalic \
	DejaVuSerif-Bold \
	DejaVuSerif \
	DejaVuSans-Oblique \
	DejaVuSansMono-Oblique \
	DejaVuSansMono-BoldOblique \
	DejaVuSansMono-Bold \
	DejaVuSansMono \
	DejaVuSans-ExtraLight \
	DejaVuSansCondensed-Oblique \
	DejaVuSansCondensed-BoldOblique \
	DejaVuSansCondensed-Bold \
	DejaVuSansCondensed \
	DejaVuSans-BoldOblique \
	DejaVuSans-Bold \
	DejaVuSans \
	DejaVuMathTeXGyre

define PartGen
define Package/dejavu-fonts-ttf-$(1)
$(call Package/dejavu-fonts-ttf/Default)
  TITLE:=$(1)
endef
endef

define Package/dejavu-fonts-ttf/Default
  TITLE:=dejavu-fonts-ttf
  SECTION:=fonts
  CATEGORY:=Fonts
  SUBMENU:=DejaVu
  URL:=http://dejavu.sourceforge.net/
endef

$(foreach file,$(FILES),$(eval $(call PartGen,$(file))))

define Build/Compile
	true
endef

define Build/Configure
	true
endef

define PartInstall
define Package/dejavu-fonts-ttf-$(1)/install
	$(INSTALL_DIR) \
		$$(1)/usr/share/fonts/ttf-dejavu

	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ttf/$(1).ttf \
		$$(1)/usr/share/fonts/ttf-dejavu/
endef
endef

$(foreach file,$(FILES),$(eval $(call PartInstall,$(file))))

$(foreach file,$(FILES),$(eval $(call BuildPackage,dejavu-fonts-ttf-$(file))))
