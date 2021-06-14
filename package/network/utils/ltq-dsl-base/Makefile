#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ltq-dsl-base
PKG_RELEASE:=3

PKG_FLAGS:=nonshared

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/ltq-dsl-base
  SECTION:=net
  CATEGORY:=Network
  TITLE:=DSL related files for Intel/Lantiq DSL Chipsets
  URL:=http://openwrt.org/
  DEPENDS:=@TARGET_lantiq +jshn
endef

define Package/ltq-dsl-base/description
  This package contains DSL related files for Intel/Lantiq DSL Chipsets.
endef

define Build/Compile
endef

define Package/ltq-dsl-base/install
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,ltq-dsl-base))
