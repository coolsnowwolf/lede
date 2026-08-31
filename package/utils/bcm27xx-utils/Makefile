# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk

PKG_NAME:=bcm27xx-utils
PKG_VERSION:=2026.06.21
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/raspberrypi/utils.git
PKG_SOURCE_VERSION:=61371fa6d93463c5451131f7bb68ae145aaf1e7a
PKG_MIRROR_HASH:=412b9faad4d37580a8316e967da65dea8f85d1622ed588bed3f2080325d9b6c9

PKG_FLAGS:=nonshared
PKG_BUILD_FLAGS:=no-lto

PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENCE

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/bcm27xx-utils
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=@TARGET_bcm27xx +dtc +fdt-utils +gcc +git +libfdt +libncurses \
	+perl +perlbase-file +perlbase-getopt +perlbase-posix +python3-light
  TITLE:=BCM27xx scripts and simple applications
  PROVIDES:=bcm27xx-userland
endef

define Package/bcm27xx-utils/description
  BCM27xx scripts and simple applications.
  Replaces bcm27xx-userland scripts and applications.
endef

define Package/bcm27xx-utils-rpi-fw-crypto
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=bcm27xx-utils +libopenssl
  TITLE:=BCM27xx rpi-fw-crypto application
  PROVIDES:=rpi-fw-crypto
  DEFAULT:=m if PACKAGE_bcm27xx-utils
endef

define Package/bcm27xx-utils-rpi-fw-crypto/description
  A command line application and shared library for the firmware
  cryptography service. Intended for use with Raspberry Pi Connect
  and secure-boot provisioner.
endef

define Package/bcm27xx-utils-libpiolib
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=@TARGET_bcm27xx bcm27xx-utils
  TITLE:=Raspberry Pi PIO userspace library (libpiolib)
  PROVIDES:=libpiolib
  DEFAULT:=n
endef

define Package/bcm27xx-utils-libpiolib/description
  PIOlib/libPIO is a user-space API to the rp1-pio driver,
  which gives access to the PIO hardware of RP1.
endef

CMAKE_OPTIONS += \
	-DBUILD_SHARED_LIBS=0 \
	-DCMAKE_INSTALL_PREFIX=/usr

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/piolib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/piolib/*.h \
		$(1)/usr/include/piolib/

	$(INSTALL_DIR) $(1)/usr/include/piolib/hardware
	$(CP) $(PKG_INSTALL_DIR)/usr/include/piolib/hardware/*.h \
		$(1)/usr/include/piolib/hardware/

	$(INSTALL_DIR) $(1)/usr/include/piolib/pico
	$(CP) $(PKG_INSTALL_DIR)/usr/include/piolib/pico/*.h \
		$(1)/usr/include/piolib/pico/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpio.so \
		$(1)/usr/lib/
endef

define Package/bcm27xx-utils/install
	$(INSTALL_DIR) $(1)/usr/bin

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dtmerge $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dtoverlay $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dtparam $(1)/usr/bin

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/eepdump $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/eepflash.sh $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/eepmake $(1)/usr/bin

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/kdtc $(1)/usr/bin
	$(SED) '1s|^#!/bin/env perl|#!/usr/bin/env perl|' $(1)/usr/bin/kdtc

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/otpset $(1)/usr/bin

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/overlaycheck $(1)/usr/bin
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/bin/overlaycheck_exclusions.txt $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ovmerge $(1)/usr/bin

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/pinctrl $(1)/usr/bin

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/raspinfo $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/rpi-eeprom-ab $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/rpi-gpu-usage $(1)/usr/bin

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/vcgencmd $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/vclog $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/vcmailbox $(1)/usr/bin
endef

define Package/bcm27xx-utils-rpi-fw-crypto/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/rpi-fw-crypto $(1)/usr/bin
endef

define Package/bcm27xx-utils-libpiolib/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpio.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,bcm27xx-utils))
$(eval $(call BuildPackage,bcm27xx-utils-rpi-fw-crypto))
$(eval $(call BuildPackage,bcm27xx-utils-libpiolib))
