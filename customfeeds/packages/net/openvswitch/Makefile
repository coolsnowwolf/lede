#
# Copyright (C) 2013 Julius Schulz-Zander <julius@net.t-labs.tu-berlin.de>
# Copyright (C) 2014-2017 OpenWrt.org
# Copyright (C) 2018-2020 Yousong Zhou <yszhou4tech@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk
include ./openvswitch.mk

# Checklist on version bump
#
#  - Check acinclude.m4 for range of supported kernel versions: "but version newer than .* is not supported"
#  - Check and update kmod dependencies when necessary (runtime module load check in the least)
#
PKG_NAME:=openvswitch
PKG_VERSION:=$(ovs_version)
PKG_RELEASE:=1
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.openvswitch.org/releases/
PKG_HASH:=7d5797f2bf2449c6a266149e88f72123540f7fe7f31ad52902057ae8d8f88c38

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:openvswitch:openvswitch

PKG_BUILD_DIR:=$(ovs_builddir)
PKG_BUILD_DEPENDS+=python3/host python-six/host
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PYTHON3_PKG_BUILD:=0

PKG_MAINTAINER:=Yousong Zhou <yszhou4tech@gmail.com>

include $(INCLUDE_DIR)/package.mk
include ../../lang/python/python3-host.mk
include ../../lang/python/python3-package.mk


ovs_kmod_packages:=
ovs_kmod_intree_kernel_patchver_min:=3.10
ovs_kmod_intree_kernel_patchver_max:=5.5
ovs_kmod_intree_not_supported:=$(strip $(call kernel_patchver_lt,$(ovs_kmod_intree_kernel_patchver_min))$(call kernel_patchver_gt,$(ovs_kmod_intree_kernel_patchver_max)))
ovs_kmod_intree_dir:=$(PKG_BUILD_DIR)/datapath/linux
ovs_kmod_upstream_dir:=$(LINUX_DIR)/net/openvswitch
ovs_kmod_package_name=$(if $(filter openvswitch,$(1)),openvswitch,$(1))
ovs_kmod_is_intree=$(filter %-intree,$(1))
ovs_kmod_upstream_name=kmod-$(call ovs_kmod_package_name,$(patsubst %-intree,%,$(1)))
ovs_kmod_package_provides=$(call ovs_kmod_upstream_name,$(1))
define OvsKmodPackageTemplate
ifeq ($(if $(call ovs_kmod_is_intree,$(1)),$(ovs_kmod_intree_not_supported)),)
  define KernelPackage/$(call ovs_kmod_package_name,$(1))
     SECTION:=kernel
     CATEGORY:=Kernel modules
     SUBMENU:=Network Support
     TITLE:=$(ovs_kmod_$(1)_title)
     DEPENDS:=$(ovs_kmod_$(1)_depends) $(if $(call ovs_kmod_is_intree,$(1)),@IPV6 @DEVEL)
     PROVIDES:=$(call ovs_kmod_package_provides,$(1))
     KCONFIG:=$(ovs_kmod_$(1)_kconfig)
     FILES:=$(ovs_kmod_$(1)_files)
     AUTOLOAD:=$(call AutoProbe,$(foreach m,$(ovs_kmod_$(1)_files),$(notdir $(patsubst %.ko,%,$(basename $(m))))))
  endef

  ovs_kmod_packages+=$(call ovs_kmod_package_name,$(1))
endif
endef

ovs_kmod_openvswitch_title:=Open vSwitch kernel datapath (upstream)
ovs_kmod_openvswitch_kconfig:=CONFIG_OPENVSWITCH
ovs_kmod_openvswitch_depends:=\
	  +kmod-lib-crc32c \
	  +kmod-nf-nat \
	  +IPV6:kmod-nf-nat6 \
	  +kmod-nf-conntrack \
	  +IPV6:kmod-nf-conntrack6 \
	  +kmod-nsh \
	  +kmod-ipt-conntrack-extra \

ovs_kmod_openvswitch_files:=$(ovs_kmod_upstream_dir)/openvswitch.ko
$(eval $(call OvsKmodPackageTemplate,openvswitch))

ovs_kmod_openvswitch-gre_title:=Open vSwitch GRE tunneling support (upstream)
ovs_kmod_openvswitch-gre_kconfig:= CONFIG_OPENVSWITCH_GRE
ovs_kmod_openvswitch-gre_depends:= +kmod-openvswitch +kmod-gre
ovs_kmod_openvswitch-gre_files:= $(ovs_kmod_upstream_dir)/vport-gre.ko
$(eval $(call OvsKmodPackageTemplate,openvswitch-gre))

ovs_kmod_openvswitch-vxlan_title:=Open vSwitch VXLAN tunneling support (upstream)
ovs_kmod_openvswitch-vxlan_kconfig:= CONFIG_OPENVSWITCH_VXLAN
ovs_kmod_openvswitch-vxlan_depends:= +kmod-openvswitch +kmod-vxlan
ovs_kmod_openvswitch-vxlan_files:= $(ovs_kmod_upstream_dir)/vport-vxlan.ko
$(eval $(call OvsKmodPackageTemplate,openvswitch-vxlan))

ovs_kmod_openvswitch-geneve_title:=Open vSwitch Geneve tunneling support (upstream)
ovs_kmod_openvswitch-geneve_kconfig:= CONFIG_OPENVSWITCH_GENEVE
ovs_kmod_openvswitch-geneve_depends:= +kmod-openvswitch +kmod-geneve
ovs_kmod_openvswitch-geneve_files:= $(ovs_kmod_upstream_dir)/vport-geneve.ko
$(eval $(call OvsKmodPackageTemplate,openvswitch-geneve))

# NOTE depends
#
#  - kmod-ipt-conntrack-extra: required for nf_conncount.ko
#
ovs_kmod_openvswitch-intree_title:=Open vSwitch kernel datapath (in tree)
ovs_kmod_openvswitch-intree_depends:=\
	  +kmod-lib-crc32c \
	  +kmod-nf-nat \
	  +IPV6:kmod-nf-nat6 \
	  +kmod-nf-conntrack \
	  +kmod-udptunnel4 \
	  +kmod-ipt-conntrack-extra \

ovs_kmod_openvswitch-intree_files:= $(ovs_kmod_intree_dir)/openvswitch.ko
$(eval $(call OvsKmodPackageTemplate,openvswitch-intree))

ovs_kmod_openvswitch-gre-intree_title:=Open vSwitch GRE tunneling support (in tree)
ovs_kmod_openvswitch-gre-intree_depends:= +kmod-openvswitch-intree +kmod-gre
ovs_kmod_openvswitch-gre-intree_files:= $(ovs_kmod_intree_dir)/vport-gre.ko
$(eval $(call OvsKmodPackageTemplate,openvswitch-gre-intree))

ovs_kmod_openvswitch-vxlan-intree_title:=Open vSwitch VXLAN tunneling support (in tree)
ovs_kmod_openvswitch-vxlan-intree_depends:= +kmod-openvswitch-intree +kmod-vxlan
ovs_kmod_openvswitch-vxlan-intree_files:= $(ovs_kmod_intree_dir)/vport-vxlan.ko
$(eval $(call OvsKmodPackageTemplate,openvswitch-vxlan-intree))

ovs_kmod_openvswitch-geneve-intree_title:=Open vSwitch Geneve tunneling support (in tree)
ovs_kmod_openvswitch-geneve-intree_depends:= +kmod-openvswitch-intree +kmod-geneve
ovs_kmod_openvswitch-geneve-intree_files:= $(ovs_kmod_intree_dir)/vport-geneve.ko
$(eval $(call OvsKmodPackageTemplate,openvswitch-geneve-intree))

ovs_kmod_openvswitch-stt-intree_title:=Open vSwitch STT tunneling support (in tree)
ovs_kmod_openvswitch-stt-intree_depends:= +kmod-openvswitch-intree
ovs_kmod_openvswitch-stt-intree_files:= $(ovs_kmod_intree_dir)/vport-stt.ko
$(eval $(call OvsKmodPackageTemplate,openvswitch-stt-intree))

ovs_kmod_openvswitch-lisp-intree_title:=Open vSwitch LISP tunneling support (in tree)
ovs_kmod_openvswitch-lisp-intree_depends:= +kmod-openvswitch-intree
ovs_kmod_openvswitch-lisp-intree_files:= $(ovs_kmod_intree_dir)/vport-lisp.ko
$(eval $(call OvsKmodPackageTemplate,openvswitch-lisp-intree))

# Dependency review
#
#	for f in sbin/*;   do echo $f; readelf -d $f | grep -i shared; done
#	for f in bin/*;    do echo $f; readelf -d $f | grep -i shared; done
#	for f in lib/*.so; do echo $f; readelf -d $f | grep -i shared; done
#
ovs_libopenvswitch_title:=Open vSwitch (libopenvswitch.so)
ovs_libopenvswitch_hidden:=1
ovs_libopenvswitch_depends:=+libopenssl +libunbound +!(arc||arceb):libunwind
ovs_libopenvswitch_depends+=+libatomic
ovs_libopenvswitch_files:=usr/lib/libopenvswitch*.so*
$(eval $(call OvsPackageTemplate,libopenvswitch))


ovs_libofproto_title:=Open vSwitch (libofproto.so libsflow.so)
ovs_libofproto_hidden:=1
ovs_libofproto_depends+=+libatomic
ovs_libofproto_files:=usr/lib/libofproto*.so* usr/lib/libsflow*.so*
$(eval $(call OvsPackageTemplate,libofproto))


ovs_libovsdb_title:=Open vSwitch (libovsdb.so)
ovs_libovsdb_hidden:=1
ovs_libovsdb_depends+=+libatomic
ovs_libovsdb_files:=usr/lib/libovsdb*.so*
$(eval $(call OvsPackageTemplate,libovsdb))


ovs_vswitchd_title:=Open vSwitch (ovs-vswitchd)
ovs_vswitchd_hidden:=1
ovs_vswitchd_depends:=+openvswitch-libopenvswitch +openvswitch-libofproto
ovs_vswitchd_depends+=+libatomic
ovs_vswitchd_files:=usr/sbin/ovs-vswitchd
$(eval $(call OvsPackageTemplate,vswitchd))


ovs_ovsdb_title:=Open vSwitch (ovsdb-server)
ovs_ovsdb_hidden:=1
ovs_ovsdb_depends:=+openvswitch-libopenvswitch +openvswitch-libovsdb
ovs_ovsdb_depends+=+libatomic
ovs_ovsdb_files:=usr/sbin/ovsdb-server
$(eval $(call OvsPackageTemplate,ovsdb))


ovs_common_title:=Open vSwitch (common files)
ovs_common_hidden:=1
ovs_common_depends:=+openvswitch-libopenvswitch +openvswitch-libofproto +openvswitch-libovsdb
ovs_common_depends+=+libatomic
ovs_common_files:= \
	usr/share/openvswitch/scripts/ovs-lib \
	usr/share/openvswitch/scripts/ovs-ctl \
	usr/share/openvswitch/scripts/ovs-kmod-ctl \
	usr/share/openvswitch/scripts/ovs-save \
	$(foreach b,ovs-appctl ovs-dpctl ovs-ofctl ovs-vsctl ovsdb-client ovsdb-tool,usr/bin/$(b))
define ovs_common_install
	$$(INSTALL_DIR) $$(1)/etc/openvswitch
	$$(INSTALL_DIR) $$(1)/etc/init.d
	$$(INSTALL_BIN) ./files/openvswitch.init $$(1)/etc/init.d/openvswitch
	$$(INSTALL_DIR) $$(1)/etc/config
	$$(INSTALL_DATA) ./files/openvswitch.config $$(1)/etc/config/openvswitch
	$$(INSTALL_DIR) $$(1)/usr/share/openvswitch/scripts
	$$(INSTALL_BIN) ./files/ovs-ctl-wrapper $$(1)/usr/share/openvswitch/scripts/
	$$(LN) /usr/share/openvswitch/scripts/ovs-ctl-wrapper $$(1)/usr/bin/ovs-ctl
	$$(LN) /usr/share/openvswitch/scripts/ovs-ctl-wrapper $$(1)/usr/bin/ovs-kmod-ctl
endef
define Package/openvswitch-common/conffiles
/etc/config/openvswitch
/etc/openvswitch
endef
$(eval $(call OvsPackageTemplate,common))


# coreutils-sleep is required by ovs-lib for sleeping a fraction of second
#
# uuidgen is required for generating system-id
ovs_openvswitch_title:=Open vSwitch
ovs_openvswitch_hidden:=
ovs_openvswitch_depends:=+coreutils +coreutils-sleep +uuidgen \
	+openvswitch-common +openvswitch-vswitchd +openvswitch-ovsdb +kmod-openvswitch
ovs_openvswitch_depends+=+libatomic
ovs_openvswitch_files:= usr/share/openvswitch/vswitch.ovsschema
$(eval $(call OvsPackageTemplate,openvswitch))


ovs_python3_title:=Open vSwitch (Python3 library)
ovs_python3_hidden:=
ovs_python3_depends:=+PACKAGE_openvswitch-python3:python3 +PACKAGE_openvswitch-python3:python3-six
define ovs_python3_install
	$$(INSTALL_DIR) $$(1)$$(PYTHON3_PKG_DIR)
	$$(CP) $$(PKG_INSTALL_DIR)/usr/share/openvswitch/python/ovs $$(1)$$(PYTHON3_PKG_DIR)
endef
$(eval $(call OvsPackageTemplate,python3))


CONFIGURE_ARGS+= \
	--enable-ndebug \
	--enable-shared \
	--disable-libcapng \
	--disable-silent-rules \

CONFIGURE_VARS += \
	ovs_cv_flake8=no \
	ovs_cv_python3=$(PYTHON3) \
	ovs_cv_python3_host=$(HOST_PYTHON3_BIN) \
	SPHINXBUILD=none \
	KARCH=$(LINUX_KARCH) \

ovs_intree_kmod_configs:=CONFIG_PACKAGE_kmod-openvswitch-intree
ovs_intree_kmod_enabled:=$(strip $(foreach c,$(ovs_intree_kmod_configs),$($(c))))
PKG_CONFIG_DEPENDS+=$(ovs_intree_kmod_configs)
ifneq ($(ovs_intree_kmod_enabled),)
  ifeq ($(ovs_kmod_intree_not_supported),)
    CONFIGURE_ARGS += --with-linux=$(LINUX_DIR)
  else
    $(warning XXX: openvswitch: intree kmods selected but not supported)
  endif
endif

TARGET_CFLAGS += -flto -std=gnu99
MAKE_VARS += PYTHONPATH="$(HOST_PYTHON3PATH)"

export OVS_KERNEL_MAKE_FLAGS=$(KERNEL_MAKE_FLAGS)

override CONFIG_AUTOREMOVE=

$(foreach p,$(ovs_kmod_packages),\
  $(eval $(call KernelPackage,$(p)))\
)
$(foreach p,$(ovs_packages),\
  $(eval $(call BuildPackage,$(p)))\
)
