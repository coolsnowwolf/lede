#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=selinux-policy
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://git.defensec.nl/selinux-policy.git
PKG_VERSION:=2.8.2
PKG_MIRROR_HASH:=7e81e6e9e933e6409b7f7bf2d5639960c440c82589c99b199b3540676f88eb8a
PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_BUILD_DEPENDS:=secilc/host policycoreutils/host

PKG_MAINTAINER:=Dominick Grift <dominick.grift@defensec.nl>
PKG_CPE_ID:=cpe:/a:defensec:selinux-policy
PKG_LICENSE:=Unlicense
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/selinux-policy
	SECTION:=system
	CATEGORY:=Base system
	TITLE:=SELinux security policy for OpenWrt
	URL:=https://git.defensec.nl/?p=selinux-policy.git;a=summary
	PKGARCH:=all
endef

define Package/selinux-policy/description
	Basic SELinux Security Policy designed specifically for
	OpenWrt and written in Common Intermediate Language.
endef

define Build/Compile
	$(call Build/Compile/Default,policy)
endef

define Package/selinux-policy/conffiles
/etc/selinux/config
endef

define Package/selinux-policy/install
	$(INSTALL_DIR) $(1)/etc/selinux/$(PKG_NAME)/contexts/files/
	$(INSTALL_DIR) $(1)/etc/selinux/$(PKG_NAME)/policy/
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/policy.* $(1)/etc/selinux/$(PKG_NAME)/policy/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/customizable_types $(1)/etc/selinux/$(PKG_NAME)/contexts/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/default_contexts $(1)/etc/selinux/$(PKG_NAME)/contexts/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/default_type $(1)/etc/selinux/$(PKG_NAME)/contexts/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/failsafe_context $(1)/etc/selinux/$(PKG_NAME)/contexts/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/file_contexts $(1)/etc/selinux/$(PKG_NAME)/contexts/files/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/file_contexts.subs_dist $(1)/etc/selinux/$(PKG_NAME)/contexts/files/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/seusers $(1)/etc/selinux/$(PKG_NAME)/
	$(INSTALL_DATA) ./files/selinux-config $(1)/etc/selinux/config
endef

$(eval $(call BuildPackage,selinux-policy))
