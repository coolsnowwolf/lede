include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=gl-mifi-mcu
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_MAINTAINER:=Nuno Goncalves <nunojpg@gmail.com>
PKG_LICENSE:=GPL-3.0-or-later

include $(INCLUDE_DIR)/package.mk

define KernelPackage/gl-mifi-mcu
  SUBMENU:=Hardware Monitoring Support
  TITLE:=GL.iNet GL-MiFI Power monitoring support
  AUTOLOAD:=$(call AutoLoad,60,gl-mifi-mcu)
  FILES:=$(PKG_BUILD_DIR)/gl-mifi-mcu.ko
endef

define KernelPackage/gl-mifi-mcu/description
  Interfaces with GL-MiFI Power monitoring MCU with a soft UART
  and provides Battery SOC, Temperature and charging data at
  /proc/gl_mifi_mcu.
  This feature is supported from GL-MiFi PCB revision v2.6.2.
  The content of /proc/gl_mifi_mcu is JSON as received from the
  UART and will frequenty contain corrupted data due to soft UART
  unreliability. User application must validate the data.
endef

define Build/Compile
	$(KERNEL_MAKE) \
		M="$(PKG_BUILD_DIR)" \
		modules
endef

$(eval $(call KernelPackage,gl-mifi-mcu))
