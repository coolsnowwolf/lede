include $(TOPDIR)/rules.mk

PKG_NAME:=ksmbd
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2025-04-04
PKG_SOURCE_URL:=https://github.com/namjaejeon/ksmbd.git
PKG_SOURCE_VERSION:=5ce517e5d08fa748074ba1422592da739da31e07
PKG_MIRROR_HASH:=c088ea320fe7ff15f940774884b7c3a70178f58a8e9ddc425c8ddb37df67ab2a

PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

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
	default n
	help
		This enables deprecated insecure protocols dialects: SMB1/CIFS and SMB2.0.
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
