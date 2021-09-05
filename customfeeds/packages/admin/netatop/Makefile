#
# This is free software, licensed under the GNU General Public License v2.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=netatop
PKG_RELEASE:=1
PKG_VERSION:=3.1
PKG_LICENSE:=GPL-2.0
PKG_SOURCE_URL:=https://www.atoptool.nl/download/
PKG_HASH:=736f43572c31a90748f023f0a5a814bff58d44c0c3f060d776cfd6e6e8435c62

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_MAINTAINER:=Toni Uhlig <matzeton@googlemail.com>

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define KernelPackage/netatop
  URL:=https://www.atoptool.nl/
  CATEGORY:=Kernel modules
  SUBMENU:=Netfilter Extensions
  TITLE:=netatop netfilter module
  FILES:= \
		$(PKG_BUILD_DIR)/module/netatop.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoProbe,netatop)
endef

define KernelPackage/netatop/description
  The optional kernel module netatop can be loaded to gather statistics about
  the TCP and UDP packets that have been transmitted/received per process and
  per thread. As soon as atop discovers that this module is active, it shows
  the columns SNET and RNET in the generic screen for the number of transmitted
  and received packets per process. When the 'n' key is pressed, it shows
  detailed counters about the number packets transmitted/received via TCP and
  UDP, the average sizes of these packets, and the total bandwidth consumed
  for input and output per process/thread.
endef

define Package/netatop
  SECTION:=admin
  CATEGORY:=Administration
  TITLE:=network counter for atop
  DEPENDS:=+zlib +kmod-netatop
  URL:=https://www.atoptool.nl/
endef

define Package/netatop/description
  The daemon netatopd is packaged with the netatop kernel module. This
  daemon takes care that information is gathered about processes that are
  finished. For every finished process that has transferred network packets,
  a binary record is written to a dedicated logfile. The added records in the
  logfile are read by atop with every sample to show information about the
  network activity of finished processes as well.
endef

NETATOP_KMOD_MAKEOPTS= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	KERNDIR="$(LINUX_DIR)"
NETATOP_DAEMON_MAKEOPTS= \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)"

define Build/Compile/netatop
	$(MAKE) -C $(PKG_BUILD_DIR)/module \
		$(NETATOP_KMOD_MAKEOPTS) \
		netatop.$(LINUX_KMOD_SUFFIX)
	$(MAKE) -C $(PKG_BUILD_DIR)/daemon \
		$(NETATOP_DAEMON_MAKEOPTS) \
		all
endef

define Build/Compile
	$(call Build/Compile/netatop)
endef

define Package/netatop/install
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/daemon/netatopd $(1)/usr/sbin/
endef

$(eval $(call KernelPackage,netatop))
$(eval $(call BuildPackage,netatop))
