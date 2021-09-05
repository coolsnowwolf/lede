#
# Copyright (C) 2020-2021 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gitlab-runner
PKG_VERSION:=13.12.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-v$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://gitlab.com/gitlab-org/gitlab-runner/-/archive/v$(PKG_VERSION)
PKG_HASH:=5cf7c2af3a3682f1044ef374dfe7f014febba6e5a53fa442b7f114b10811831e

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DIR:=$(BUILD_DIR)/gitlab-runner-v$(PKG_VERSION)
PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0
GO_PKG:=gitlab.com/gitlab-org/gitlab-runner

include $(INCLUDE_DIR)/package.mk
include ../../lang/golang/golang-package.mk

define Package/gitlab-runner
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=Runner for CI/CD
  URL:=https://docs.gitlab.com/runner
  DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Package/gitlab-runner/description
  GitLab Runner is an application that works with
  GitLab CI/CD to run jobs in a pipeline.
endef

$(eval $(call GoBinPackage,gitlab-runner))
$(eval $(call BuildPackage,gitlab-runner))
