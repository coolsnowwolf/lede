#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk

PKG_VERSION:=1.0.31

PKG_NAME:=uClibc-ng
PKG_SOURCE_URL = http://downloads.uclibc-ng.org/releases/$(PKG_VERSION)/
PATCH_DIR:=$(PATH_PREFIX)/patches
CONFIG_DIR:=$(PATH_PREFIX)/config
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
LIBC_SO_VERSION:=$(PKG_VERSION)

PKG_HASH:=2215d7377118434d1697fd575f10d7a6be3f29e460d6b0e1ee9f6f5306288060

HOST_BUILD_DIR:=$(BUILD_DIR_TOOLCHAIN)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/toolchain-build.mk

UCLIBC_TARGET_ARCH:=$(shell echo $(ARCH) | sed -e s'/-.*//' \
		-e 's/arc.*/arc/' \
		-e 's/i.86/i386/' \
		-e 's/sparc.*/sparc/' \
		-e 's/arm.*/arm/g' \
		-e 's/m68k.*/m68k/' \
		-e 's/ppc/powerpc/g' \
		-e 's/v850.*/v850/g' \
		-e 's/sh64/sh/' \
		-e 's/sh[234].*/sh/' \
		-e 's/mips.*/mips/' \
		-e 's/mipsel.*/mips/' \
)

GEN_CONFIG=$(SCRIPT_DIR)/kconfig.pl -n \
	$(if $(wildcard $(CONFIG_DIR)/common),'+' $(CONFIG_DIR)/common) \
	$(if $(CONFIG_UCLIBC_ENABLE_DEBUG),$(if $(wildcard $(CONFIG_DIR)/debug),'+' $(CONFIG_DIR)/debug)) \
	$(CONFIG_DIR)/$(ARCH)$(strip \
		$(if $(wildcard $(CONFIG_DIR)/$(ARCH).$(BOARD)),.$(BOARD), \
			$(if $(filter archs,$(subst ",,$(CONFIG_CPU_TYPE))),hs, \
			$(if $(CONFIG_MIPS64_ABI),.$(subst ",,$(CONFIG_MIPS64_ABI)), \
			$(if $(CONFIG_HAS_SPE_FPU),$(if $(wildcard $(CONFIG_DIR)/$(ARCH).e500),.e500))))))

CPU_CFLAGS = \
	-funsigned-char -fno-builtin -fno-asm \
	--std=gnu99 -ffunction-sections -fdata-sections \
	-Wno-unused-but-set-variable \
	$(TARGET_CFLAGS) -ggdb

UCLIBC_MAKE = PATH='$(TOOLCHAIN_DIR)/initial/bin:$(TARGET_PATH)' $(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR) \
	$(TARGET_CONFIGURE_OPTS) \
	DEVEL_PREFIX=/ \
	RUNTIME_PREFIX=/ \
	HOSTCC="$(HOSTCC)" \
	CPU_CFLAGS="$(CPU_CFLAGS)" \
	ARCH="$(CONFIG_ARCH)" \
	LIBGCC="$(subst libgcc.a,libgcc_initial.a,$(shell $(TARGET_CC) -print-libgcc-file-name))" \
	DOSTRIP=""

define Host/Prepare
	$(call Host/Prepare/Default)
	$(if $(strip $(QUILT)), \
		cd $(HOST_BUILD_DIR); \
		if $(QUILT_CMD) next >/dev/null 2>&1; then \
			$(QUILT_CMD) push -a; \
		fi
	)
	ln -snf $(PKG_NAME)-$(PKG_VERSION) $(BUILD_DIR_TOOLCHAIN)/$(PKG_NAME)
endef

define Host/Configure
	$(GEN_CONFIG) > $(HOST_BUILD_DIR)/.config.new
	$(SED) 's,^KERNEL_HEADERS=.*,KERNEL_HEADERS=\"$(BUILD_DIR_TOOLCHAIN)/linux-dev/include\",g' \
		-e 's,^.*UCLIBC_HAS_FPU.*,UCLIBC_HAS_FPU=$(if $(CONFIG_SOFT_FLOAT),n,y),g' \
		-e 's,^.*UCLIBC_HAS_SOFT_FLOAT.*,UCLIBC_HAS_SOFT_FLOAT=$(if $(CONFIG_SOFT_FLOAT),y,n),g' \
		-e 's,^.*UCLIBC_HAS_LOCALE.*,UCLIBC_HAS_LOCALE=$(if $(CONFIG_BUILD_NLS),y,n),g' \
		-e 's,^.*UCLIBC_BUILD_ALL_LOCALE.*,UCLIBC_BUILD_ALL_LOCALE=$(if $(CONFIG_BUILD_NLS),y,n),g' \
		-e 's,^.*UCLIBC_HAS_SSP[^_].*,UCLIBC_HAS_SSP=$(if $(or $(CONFIG_PKG_CC_STACKPROTECTOR_REGULAR),$(CONFIG_PKG_CC_STACKPROTECTOR_STRONG)),y,n),g' \
		$(HOST_BUILD_DIR)/.config.new
	cmp -s $(HOST_BUILD_DIR)/.config.new $(HOST_BUILD_DIR)/.config.last || { \
		cp $(HOST_BUILD_DIR)/.config.new $(HOST_BUILD_DIR)/.config && \
		$(MAKE) -C $(HOST_BUILD_DIR) olddefconfig KBUILD_HAVE_NLS= HOSTCFLAGS="-DKBUILD_NO_NLS" && \
		$(MAKE) -C $(HOST_BUILD_DIR)/extra/config conf KBUILD_HAVE_NLS= HOSTCFLAGS="-DKBUILD_NO_NLS" && \
		cp $(HOST_BUILD_DIR)/.config.new $(HOST_BUILD_DIR)/.config.last; \
	}
endef

define Host/Clean
	rm -rf \
		$(HOST_BUILD_DIR) \
		$(BUILD_DIR_TOOLCHAIN)/$(PKG_NAME) \
		$(BUILD_DIR_TOOLCHAIN)/$(LIBC)-dev
endef
