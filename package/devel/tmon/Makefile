include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=tmon
PKG_VERSION:=$(LINUX_VERSION)
PKG_RELEASE:=1

PKG_MAINTAINER:=Florian Eckert <fe@dev.tdt.de>
PKG_LICENSE:=GPL-2.0-only

include $(INCLUDE_DIR)/package.mk

define Package/tmon
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=Thermal monitoring and testing tool
  VERSION:=$(LINUX_VERSION)-$(PKG_RELEASE)
  URL:=http://www.kernel.org
  DEPENDS:=+libncursesw
endef

define Package/tmon/description
  As hardware vendors cope with the thermal constraints on their products,
  more and more sensors are added, new cooling capabilities are introduced.
  To expose such relationship to the userspace, Linux generic thermal layer
  introduced sysfs entry at /sys/class/thermal with a matrix of symbolic
  links, trip point bindings, and device instances. To traverse such
  matrix by hand is not a trivial task.
  'TMON' is conceived as a tool to help visualize, tune, and test the
  complex thermal subsystem.
endef

MAKE_FLAGS = \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)"

define Build/Compile
	-$(MAKE) clean \
		-C $(LINUX_DIR)/tools/thermal/tmon
	+$(MAKE_FLAGS) $(MAKE) \
		-C $(LINUX_DIR)/tools/thermal/tmon
endef

define Package/tmon/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(LINUX_DIR)/tools/thermal/tmon/tmon \
		$(1)/usr/bin/
endef

$(eval $(call BuildPackage,tmon))
