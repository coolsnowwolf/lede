include $(TOPDIR)/rules.mk

PKG_NAME:=ksmbd
PKG_VERSION:=3.4.8
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/cifsd-team/ksmbd/releases/download/$(PKG_VERSION)
PKG_HASH:=209bfba4dc5c50007942a3ec24f86a7f7c628618ac07f79c62e3404dc247d495

PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

TAR_OPTIONS+= --strip-components 1
TAR_CMD=$(HOST_TAR) -C $(1) $(TAR_OPTIONS)

define KernelPackage/fs-ksmbd
	SUBMENU:=Filesystems
	TITLE:=SMB kernel server support
	URL:=https://github.com/cifsd-team/cifsd
	FILES:=$(PKG_BUILD_DIR)/ksmbd.ko
	DEPENDS:= \
		+kmod-nls-base \
		+kmod-nls-utf8 \
		+kmod-crypto-md4 \
		+kmod-crypto-md5 \
		+kmod-crypto-hmac \
		+kmod-crypto-ecb \
		+kmod-crypto-des \
		+kmod-crypto-sha256 \
		+kmod-crypto-cmac \
		+kmod-crypto-sha512 \
		+kmod-crypto-aead \
		+kmod-crypto-ccm \
		+kmod-crypto-gcm \
		+kmod-asn1-decoder \
		+kmod-oid-registry
endef

# The last two DEPENDS are hacks in order to get CONFIG_ASN1 and CONFIG_OID_REGISTRY
# which it seems can't be selected independently. Some bug in either base or upstream.

define KernelPackage/fs-ksmbd/description
  Ksmbd is an In-kernel SMBv(1)2/3 fileserver.
  It's an implementation of the SMB protocol in kernel space for sharing files and IPC services over network.
endef

define KernelPackage/fs-ksmbd/config
config KSMBD_SMB_INSECURE_SERVER
	bool "Support for insecure SMB1/CIFS and SMB2.0 protocols"
	depends on PACKAGE_kmod-fs-ksmbd
	help
		This enables deprecated insecure protocols dialects: SMB1/CIFS and SMB2.0.
	default y
endef

ifeq ($(CONFIG_KSMBD_SMB_INSECURE_SERVER),y)
PKG_EXTRA_KCONFIG:=CONFIG_SMB_INSECURE_SERVER=y
EXTRA_CFLAGS += -DCONFIG_SMB_INSECURE_SERVER=1
endif

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(PKG_EXTRA_KCONFIG) \
	CONFIG_SMB_SERVER=m \
	modules
endef

$(eval $(call KernelPackage,fs-ksmbd))
