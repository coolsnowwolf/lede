#
# Copyright (C) 2020-2021 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ooniprobe
PKG_VERSION:=3.9.2
PKG_RELEASE:=1

PKG_SOURCE:=probe-cli-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/ooni/probe-cli/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=d34dc096dfdebceaa027716fdf675eb9ab7f0085defb4235f52685d064bd5afa

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE.md

PKG_BUILD_DIR:=$(BUILD_DIR)/probe-cli-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/ooni/probe-cli
GO_PKG_TAGS:=PSIPHON_DISABLE_QUIC

include $(INCLUDE_DIR)/package.mk
include ../../lang/golang/golang-package.mk

define Package/ooniprobe
  SECTION:=net
  CATEGORY:=Network
  TITLE:=OONI probe-cli
  URL:=https://ooni.org
  DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Package/ooniprobe/description
  The next generation of  OONI(Open Observatory of Network Interference)
  Probe Command Line Interface.
endef

$(eval $(call GoBinPackage,ooniprobe))
$(eval $(call BuildPackage,ooniprobe))
