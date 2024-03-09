#
# Copyright (c) 2015,2016 The Linux Foundation. All rights reserved.
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that the
# above copyright notice and this permission notice appear in all copies.
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
# OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=shortcut-fe-simulated-driver
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://git.codelinaro.org/clo/qsdk/oss/lklm/shortcut-fe.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2021-03-17
PKG_SOURCE_VERSION:=697977d8d0ccf0ab596e5692d08608a75dd7f33d
PKG_MIRROR_HASH:=659fa82a431e15af797a6c7069faeee02810453ad8b576c51c29f95a1761a045

include $(INCLUDE_DIR)/package.mk

define KernelPackage/shortcut-fe-drv
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Support
  DEPENDS:=@TARGET_qualcommax +kmod-shortcut-fe
  KCONFIG:= \
	   CONFIG_NET_CLS_ACT=y \
	   CONFIG_XFRM=y
  TITLE:=Simulated sfe driver for ECM
  FILES:=$(PKG_BUILD_DIR)/simulated-driver/shortcut-fe-drv.ko
endef

define KernelPackage/shortcut-fe-drv/Description
Simulated sfe driver which act as an adapter to convert message
between a connection manager and the SFE core engine.
endef

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		$(PKG_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)/simulated-driver" \
		EXTRA_CFLAGS="-DSFE_SUPPORT_IPV6" \
		modules
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/shortcut-fe
	$(CP) -rf $(PKG_BUILD_DIR)/simulated-driver/sfe_drv.h $(1)/usr/include/shortcut-fe
endef

$(eval $(call KernelPackage,shortcut-fe-drv))
