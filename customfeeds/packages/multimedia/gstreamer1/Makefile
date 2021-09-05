#
# Copyright (C) 2008-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gstreamer1
PKG_VERSION:=1.18.4
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=gstreamer-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://gstreamer.freedesktop.org/src/gstreamer
PKG_HASH:=9aeec99b38e310817012aa2d1d76573b787af47f8a725a65b833880a094dfbc5
PKG_BUILD_DIR:=$(BUILD_DIR)/gstreamer-$(PKG_VERSION)

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org> \
		Ted Hess <thess@kitschensync.net>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:gstreamer_project:gstreamer

PKG_INSTALL:=1
PKG_BUILD_DEPENDS:=meson/host glib2/host gettext-full/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include ../../devel/meson/meson.mk

define Package/gstreamer1/Default
  CATEGORY:=Multimedia
  SECTION:=multimedia
  TITLE:=GStreamer
  URL:=https://gstreamer.freedesktop.org/
  DEPENDS:= $(ICONV_DEPENDS) $(INTL_DEPENDS)
endef

define Package/gstreamer1/description/Default
 GStreamer open source multimedia framework
endef

GSTREAMER_CORE_LIBS:= \
	libgstreamer1 \
	libgst1check \
	libgst1controller \
	libgst1net

define Package/gstreamer1
  $(call Package/gstreamer1/Default)
  TITLE+= (All libraries)
  DEPENDS:= $(foreach lib,$(GSTREAMER_CORE_LIBS),+$(lib))
  HIDDEN:=1
endef

define Package/gstreamer1-libs/description
 $(call Package/gstreamer1/description/Default)
 .
 This meta package contains only dependencies on the other GStreamer
 componenents.
endef

define Package/gstreamer1-libs
  $(call Package/gstreamer1/Default)
  TITLE+= core libraries
  DEPENDS+=+ALL:gstreamer1
endef

define Package/gstreamer1-libs/config
    menu "Select GStreamer libraries"
      depends on PACKAGE_gstreamer1-libs

    config gstreamer1-all
      bool "Include all GStreamer1 core libraries"
      select PACKAGE_gstreamer1

    comment "Libraries"

   $(foreach lib,$(GSTREAMER_CORE_LIBS), \
    config PACKAGE_$(lib)
      prompt "GStreamer core library $(lib)"

   )

  endmenu

endef

define Package/gstreamer1-utils
  $(call Package/gstreamer1/Default)
  TITLE+= utilities
  DEPENDS+= +libgstreamer1 +gstreamer1-libs
endef

define Package/gstreamer1-utils/description
 $(call Package/gstreamer1/description/Default)
 .
 This package contains the GStreamer utilities.
endef

define Package/libgstreamer1
  $(call Package/gstreamer1/Default)
  TITLE+= library (core)
  DEPENDS+= +glib2 +libpthread +libxml2
  HIDDEN:=1
endef

define Package/libgstreamer1/description
 $(call Package/gstreamer1/description/Default)
 .
 This package contains the GStreamer core library.
endef


GST_VERSION:=1.0

MESON_ARGS += \
	-Dgst_debug=false \
	-Dgst_parse=true \
	-Dregistry=true \
	-Dtracer_hooks=false \
	-Dptp-helper-setuid-user=none \
	-Dptp-helper-setuid-group=none \
	-Doption-parsing=true \
	-Dpoisoning=false \
	-Dmemory-alignment=malloc \
	-Dcheck=enabled \
	-Dlibunwind=disabled \
	-Dlibdw=disabled \
	-Ddbghelp=disabled \
	-Dbash-completion=disabled \
	-Dcoretracers=disabled \
	-Dexamples=disabled \
	-Dbenchmarks=disabled \
	-Dtools=enabled \
	-Dgtk_doc=disabled \
	-Dintrospection=disabled \
	-Dnls=enabled \
	-Dgobject-cast-checks=disabled \
	-Dglib-asserts=disabled \
	-Dextra-checks=disabled \
	-Ddoc=disabled

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/gstreamer-$(GST_VERSION)
	( cd $(PKG_INSTALL_DIR); $(CP) \
		./usr/include/gstreamer-$(GST_VERSION)/* \
		$(1)/usr/include/gstreamer-$(GST_VERSION)/ \
	)
	$(INSTALL_DIR) $(1)/usr/lib
	( cd $(PKG_INSTALL_DIR); $(CP) \
		./usr/lib/libgst*-$(GST_VERSION).so* \
		$(1)/usr/lib/ \
	)
	$(INSTALL_DIR) $(1)/usr/lib/gstreamer-$(GST_VERSION)
	( cd $(PKG_INSTALL_DIR); $(CP) \
		./usr/lib/gstreamer-$(GST_VERSION)/libgst*.so \
		$(1)/usr/lib/gstreamer-$(GST_VERSION)/ \
	)
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	( cd $(PKG_INSTALL_DIR); $(CP) \
		./usr/lib/pkgconfig/gstreamer*-$(GST_VERSION).pc \
		$(1)/usr/lib/pkgconfig/ \
	)
	$(INSTALL_DIR) $(2)/share/aclocal
	( cd $(PKG_INSTALL_DIR); $(CP) \
		./usr/share/aclocal/* \
		$(2)/share/aclocal/ \
	)
endef

define Package/gstreamer1/install
  true
endef

define Package/gstreamer1-libs/install
  true
endef

define Package/gstreamer1-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	( cd $(PKG_INSTALL_DIR); $(CP) \
		./usr/bin/gst-launch* \
		./usr/bin/gst-inspect* \
		./usr/bin/gst-typefind* \
		$(1)/usr/bin/ \
	)
endef

define Package/libgstreamer1/install
	$(INSTALL_DIR) $(1)/usr/lib
	( cd $(PKG_INSTALL_DIR); $(CP) \
		./usr/lib/libgstbase-$(GST_VERSION).so.* \
		./usr/lib/libgstreamer-$(GST_VERSION).so.* \
		$(1)/usr/lib/ \
	)
	$(INSTALL_DIR) $(1)/usr/lib/gstreamer-$(GST_VERSION)
	( cd $(PKG_INSTALL_DIR); $(CP) \
		./usr/lib/gstreamer-$(GST_VERSION)/libgst*.so \
		$(1)/usr/lib/gstreamer-$(GST_VERSION)/ \
	)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/libexec/gstreamer-$(GST_VERSION)/gst-plugin-scanner \
		       $(1)/usr/lib/gstreamer-$(GST_VERSION)
endef


# 1: short name
# 2: description
# 3: dependencies on other gstreamer libraries (short name)
# 4: dependencies on other packages
define GstBuildLibrary

  define Package/libgst1$(1)
    $(call Package/gstreamer1/Default)
    TITLE+= $(2) library (core)
    DEPENDS+= +libgstreamer1 $$(foreach p,$(3),+libgst1$$(p)) $(4)
    HIDDEN:=1
  endef

  define Package/libgst1$(1)/description
   $(call Package/gstreamer1/description/Default)
   .
   This package contains the GStreamer $(2) library.
  endef

  define Package/libgst1$(1)/install
	$(INSTALL_DIR) $$(1)/usr/lib
	( cd $(PKG_INSTALL_DIR); $(CP) \
		./usr/lib/libgst$(1)-$(GST_VERSION).so.* \
		$$(1)/usr/lib/ \
	)
  endef

  $$(eval $$(call BuildPackage,libgst1$(1)))
endef

$(eval $(call GstBuildLibrary,check,check unit testing))
$(eval $(call GstBuildLibrary,controller,dynamic parameter control))
$(eval $(call GstBuildLibrary,net,network classes))

$(eval $(call BuildPackage,libgstreamer1))
$(eval $(call BuildPackage,gstreamer1-libs))
$(eval $(call BuildPackage,gstreamer1-utils))
$(eval $(call BuildPackage,gstreamer1))
