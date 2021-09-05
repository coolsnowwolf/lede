# Copyright (C) 2020 Yousong Zhou <yszhou4tech@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

# Versions

ovs_version:=2.15.0
ovs_builddir=$(KERNEL_BUILD_DIR)/openvswitch-$(ovs_version)

# Shared vars, macros

ovs_packages:=

ovs_package_name=$(if $(filter openvswitch,$(1)),openvswitch,openvswitch-$(1))
define OvsPackageTemplate
  define Package/$(call ovs_package_name,$(1))
     SECTION:=net
     SUBMENU:=Open vSwitch
     CATEGORY:=Network
     URL:=https://www.openvswitch.org
     TITLE:=$(ovs_$(1)_title)
     HIDDEN:=$(ovs_$(1)_hidden)
     DEPENDS:=$(ovs_$(1)_depends)
  endef

  define Package/$(call ovs_package_name,$(1))/install
	$(foreach f,$(ovs_$(1)_files),
		$(INSTALL_DIR) $$(1)/$(dir $(f))
		$(CP) $(PKG_INSTALL_DIR)/$(f) $$(1)/$(dir $(f))
	)
	$(ovs_$(1)_install)
  endef

  ovs_packages+=$(call ovs_package_name,$(1))
endef
