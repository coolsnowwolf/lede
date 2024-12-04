include $(TOPDIR)/rules.mk

PKG_NAME:=ksmbd
PKG_VERSION:=2024.10.14
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/namjaejeon/ksmbd.git
PKG_SOURCE_VERSION:=6a0efcd1301d72ccd76b2f457c1d0b56702f4908
PKG_MIRROR_HASH:=02ba5828aae66d4fac202b8c4dcb1f45beb255655927741eb293d4a257822624

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

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" \
	CONFIG_SMB_SERVER=m \
	modules
endef

$(eval $(call KernelPackage,fs-ksmbd))
