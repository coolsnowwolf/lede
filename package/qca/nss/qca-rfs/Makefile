include $(TOPDIR)/rules.mk

PKG_NAME:=qca-rfs
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://source.codeaurora.org/quic/qsdk/oss/lklm/qca-rfs
PKG_SOURCE_DATE:=2021-03-17
PKG_SOURCE_VERSION:=75197c386f477c7b3a6f02489d9903a9409fd5cc
PKG_MIRROR_HASH:=90f1c3ec2e984cf8efa79c85d715ebd8a21e347ab57adbd9695de23e64eea1ec

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/qca-rfs
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Support
  DEPENDS:=@TARGET_ipq40xx +kmod-ipt-conntrack
  TITLE:=Kernel module for QCA Receiving Flow Steering
  FILES:=$(PKG_BUILD_DIR)/qrfs.ko
  KCONFIG:=\
  	CONFIG_NF_CONNTRACK_EVENTS=y \
	CONFIG_NF_CONNTRACK_CHAIN_EVENTS=y
  AUTOLOAD:=$(call AutoLoad,29,qrfs)
endef

define KernelPackage/qca-rfs/Description
QCA-RFS is a kernel module for ESS Receive Flow Steering.
endef

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		$(PKG_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		modules
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/qca-rfs
	$(CP) -rf $(PKG_BUILD_DIR)/rfs_dev.h $(1)/usr/include/qca-rfs
endef

define KernelPackage/qca-rfs/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/qrfs.init $(1)/etc/init.d/qrfs
endef

$(eval $(call KernelPackage,qca-rfs))
