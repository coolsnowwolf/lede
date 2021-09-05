include $(TOPDIR)/rules.mk

PKG_NAME:=dockerd
PKG_VERSION:=20.10.8
PKG_RELEASE:=1
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_GIT_URL:=github.com/moby/moby
PKG_GIT_REF:=v$(PKG_VERSION)
PKG_SOURCE_URL:=https://codeload.$(PKG_GIT_URL)/tar.gz/$(PKG_GIT_REF)?
PKG_HASH:=2505d00032f5d40ead5ac779c2840303dcead04713c93ba974be4c19b3ab8d0a
PKG_GIT_SHORT_COMMIT:=75249d8 # SHA1 used within the docker executables

PKG_MAINTAINER:=Gerard Ryan <G.M0N3Y.2503@gmail.com>

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/docker/docker

include $(INCLUDE_DIR)/package.mk
include ../../lang/golang/golang-package.mk

define Package/dockerd/config
  source "$(SOURCE)/Config.in"
endef

define Package/dockerd
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Docker Community Edition Daemon
  URL:=https://www.docker.com/
  DEPENDS:=$(GO_ARCH_DEPENDS) +btrfs-progs +ca-certificates +containerd +libdevmapper +libnetwork +tini \
           +KERNEL_SECCOMP:libseccomp +iptables-mod-extra +kmod-br-netfilter +kmod-ikconfig +kmod-nf-conntrack-netlink +kmod-nf-ipvs \
           +kmod-nf-nat +kmod-veth
  USERID:=docker:docker
  MENU:=1
endef

define Package/dockerd/conffiles
/etc/config/dockerd
endef

define Package/dockerd/description
The Docker CE Engine.
endef

GO_PKG_BUILD_VARS += GO111MODULE=auto
TAR_OPTIONS:=--strip-components 1 $(TAR_OPTIONS)
TAR_CMD=$(HOST_TAR) -C $(1) $(TAR_OPTIONS)

# $(1) = path to dependent package 'Makefile'
# $(2) = relevant dependency '.installer' file
define EnsureVendoredVersion
	( \
		DEP_VER=$$$$( grep --only-matching --perl-regexp '(?<=PKG_SOURCE_VERSION:=)(.*)' "$(1)" ); \
		VEN_VER=$$$$( grep --only-matching --perl-regexp '(?<=_COMMIT:=)(.*)(?=})' "$(PKG_BUILD_DIR)/hack/dockerfile/install/$(2)" ); \
		if [ "$$$${VEN_VER}" != "$$$${DEP_VER}" ]; then \
			echo "ERROR: Expected 'PKG_SOURCE_VERSION:=$$$${VEN_VER}' in '$(1)', found 'PKG_SOURCE_VERSION:=$$$${DEP_VER}'"; \
			exit 1; \
		fi \
	)
endef

define Build/Prepare
	$(Build/Prepare/Default)

	# Verify dependencies are the vendored version
	$(call EnsureVendoredVersion,../containerd/Makefile,containerd.installer)
	$(call EnsureVendoredVersion,../libnetwork/Makefile,proxy.installer)
	$(call EnsureVendoredVersion,../runc/Makefile,runc.installer)
	$(call EnsureVendoredVersion,../tini/Makefile,tini.installer)

	# Verify CLI is the same version
	( \
		CLI_MAKEFILE="../docker/Makefile"; \
		CLI_VERSION=$$$$( grep --only-matching --perl-regexp '(?<=PKG_VERSION:=)(.*)' "$$$${CLI_MAKEFILE}" ); \
		if [ "$$$${CLI_VERSION}" != "$(PKG_VERSION)" ]; then \
			echo "ERROR: Expected 'PKG_VERSION:=$(PKG_VERSION)' in '$$$${CLI_MAKEFILE}', found 'PKG_VERSION:=$$$${CLI_VERSION}'"; \
			exit 1; \
		fi \
	)

	# Verify PKG_GIT_SHORT_COMMIT
	( \
		EXPECTED_PKG_GIT_SHORT_COMMIT=$$$$( $(CURDIR)/git-short-commit.sh '$(PKG_GIT_URL)' '$(PKG_GIT_REF)' '$(TMP_DIR)/git-short-commit/$(PKG_NAME)-$(PKG_VERSION)' ); \
		if [ "$$$${EXPECTED_PKG_GIT_SHORT_COMMIT}" != "$(strip $(PKG_GIT_SHORT_COMMIT))" ]; then \
			echo "ERROR: Expected 'PKG_GIT_SHORT_COMMIT:=$$$${EXPECTED_PKG_GIT_SHORT_COMMIT}', found 'PKG_GIT_SHORT_COMMIT:=$(strip $(PKG_GIT_SHORT_COMMIT))'"; \
			exit 1; \
		fi \
	)
endef

BUILDTAGS:=
ifeq ($(KERNEL_SECCOMP),y)
BUILDTAGS += seccomp
endif
ifeq ($(CONFIG_SELINUX),y)
BUILDTAGS += selinux
endif

define Build/Compile
	( \
		cd $(PKG_BUILD_DIR); \
		$(GO_PKG_VARS) \
		DOCKER_GITCOMMIT=$(PKG_GIT_SHORT_COMMIT) \
		DOCKER_BUILDTAGS='$(BUILDTAGS)' \
		VERSION=$(PKG_VERSION) \
		./hack/make.sh binary; \
	)
endef

define Package/dockerd/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bundles/binary-daemon/dockerd $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/opt/docker/
	$(INSTALL_DIR) $(1)/usr/share/docker/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/contrib/check-config.sh $(1)/usr/share/docker/

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/dockerd.init $(1)/etc/init.d/dockerd

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/etc/config/dockerd $(1)/etc/config/dockerd

	# Must be after systcl 11-br-netfilter.conf from kmod-br-netfilter
	$(INSTALL_DIR) $(1)/etc/sysctl.d
	$(INSTALL_DATA) ./files/etc/sysctl.d/sysctl-br-netfilter-ip.conf \
		$(1)/etc/sysctl.d/12-br-netfilter-ip.conf
endef

define Package/dockerd/postinst
#!/bin/sh
[ -n "$$IPKG_INSTROOT" ] || {
	/etc/init.d/dockerd enable
	/etc/init.d/dockerd uciadd
	/etc/init.d/dockerd start
}
endef

define Package/dockerd/prerm
#!/bin/sh
[ -n "$$IPKG_INSTROOT" ] || {
	/etc/init.d/dockerd disable
	/etc/init.d/dockerd stop
	/etc/init.d/dockerd ucidel
}
endef

$(eval $(call BuildPackage,dockerd))
