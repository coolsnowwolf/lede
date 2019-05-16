#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

ifdef CONFIG_STRIP_KERNEL_EXPORTS
  KERNEL_MAKEOPTS += \
	EXTRA_LDSFLAGS="-I$(KERNEL_BUILD_DIR) -include symtab.h"
endif

INITRAMFS_EXTRA_FILES ?= $(GENERIC_PLATFORM_DIR)/image/initramfs-base-files.txt

ifneq (,$(KERNEL_CC))
  KERNEL_MAKEOPTS += CC="$(KERNEL_CC)"
endif

export HOST_EXTRACFLAGS=-I$(STAGING_DIR_HOST)/include

# defined in quilt.mk
Kernel/Patch:=$(Kernel/Patch/Default)

ifneq (,$(findstring .xz,$(LINUX_SOURCE)))
  LINUX_CAT:=xzcat
else
  LINUX_CAT:=zcat
endif

ifeq ($(strip $(CONFIG_EXTERNAL_KERNEL_TREE)),"")
  ifeq ($(strip $(CONFIG_KERNEL_GIT_CLONE_URI)),"")
    define Kernel/Prepare/Default
	$(LINUX_CAT) $(DL_DIR)/$(LINUX_SOURCE) | $(TAR) -C $(KERNEL_BUILD_DIR) $(TAR_OPTIONS)
	$(Kernel/Patch)
	$(if $(QUILT),touch $(LINUX_DIR)/.quilt_used)
    endef
  else
    define Kernel/Prepare/Default
	$(LINUX_CAT) $(DL_DIR)/$(LINUX_SOURCE) | $(TAR) -C $(KERNEL_BUILD_DIR) $(TAR_OPTIONS)
    endef
  endif
else
  define Kernel/Prepare/Default
	mkdir -p $(KERNEL_BUILD_DIR)
	if [ -d $(LINUX_DIR) ]; then \
		rmdir $(LINUX_DIR); \
	fi
	ln -s $(CONFIG_EXTERNAL_KERNEL_TREE) $(LINUX_DIR)
  endef
endif

ifeq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),y)
  ifeq ($(strip $(CONFIG_EXTERNAL_CPIO)),"")
    define Kernel/SetInitramfs/PreConfigure
	grep -v -e INITRAMFS -e CONFIG_RD_ -e CONFIG_BLK_DEV_INITRD $(LINUX_DIR)/.config.old > $(LINUX_DIR)/.config
	echo 'CONFIG_BLK_DEV_INITRD=y' >> $(LINUX_DIR)/.config
	echo 'CONFIG_INITRAMFS_SOURCE="$(strip $(TARGET_DIR) $(INITRAMFS_EXTRA_FILES))"' >> $(LINUX_DIR)/.config
    endef
  else
    define Kernel/SetInitramfs/PreConfigure
	grep -v INITRAMFS $(LINUX_DIR)/.config.old > $(LINUX_DIR)/.config
	echo 'CONFIG_INITRAMFS_SOURCE="$(call qstrip,$(CONFIG_EXTERNAL_CPIO))"' >> $(LINUX_DIR)/.config
    endef
  endif

  define Kernel/SetInitramfs
	rm -f $(LINUX_DIR)/.config.prev
	mv $(LINUX_DIR)/.config $(LINUX_DIR)/.config.old
	$(call Kernel/SetInitramfs/PreConfigure)
	echo 'CONFIG_INITRAMFS_ROOT_UID=$(shell id -u)' >> $(LINUX_DIR)/.config
	echo 'CONFIG_INITRAMFS_ROOT_GID=$(shell id -g)' >> $(LINUX_DIR)/.config
	echo "$(if $(CONFIG_TARGET_INITRAMFS_FORCE),CONFIG_INITRAMFS_FORCE=y,# CONFIG_INITRAMFS_FORCE is not set)" >> $(LINUX_DIR)/.config
	echo "$(if $(CONFIG_TARGET_INITRAMFS_COMPRESSION_NONE),CONFIG_INITRAMFS_COMPRESSION_NONE=y,# CONFIG_INITRAMFS_COMPRESSION_NONE is not set)" >> $(LINUX_DIR)/.config
	echo -e "$(if $(CONFIG_TARGET_INITRAMFS_COMPRESSION_GZIP),CONFIG_INITRAMFS_COMPRESSION_GZIP=y\nCONFIG_RD_GZIP=y,# CONFIG_INITRAMFS_COMPRESSION_GZIP is not set\n# CONFIG_RD_GZIP is not set)" >> $(LINUX_DIR)/.config
	echo -e "$(if $(CONFIG_TARGET_INITRAMFS_COMPRESSION_BZIP2),CONFIG_INITRAMFS_COMPRESSION_BZIP2=y\nCONFIG_RD_BZIP2=y,# CONFIG_INITRAMFS_COMPRESSION_BZIP2 is not set\n# CONFIG_RD_BZIP2 is not set)" >> $(LINUX_DIR)/.config
	echo -e "$(if $(CONFIG_TARGET_INITRAMFS_COMPRESSION_LZMA),CONFIG_INITRAMFS_COMPRESSION_LZMA=y\nCONFIG_RD_LZMA=y,# CONFIG_INITRAMFS_COMPRESSION_LZMA is not set\n# CONFIG_RD_LZMA is not set)" >> $(LINUX_DIR)/.config
	echo -e "$(if $(CONFIG_TARGET_INITRAMFS_COMPRESSION_LZO),CONFIG_INITRAMFS_COMPRESSION_LZO=y\nCONFIG_RD_LZO=y,# CONFIG_INITRAMFS_COMPRESSION_LZO is not set\n# CONFIG_RD_LZO is not set)" >> $(LINUX_DIR)/.config
	echo -e "$(if $(CONFIG_TARGET_INITRAMFS_COMPRESSION_XZ),CONFIG_INITRAMFS_COMPRESSION_XZ=y\nCONFIG_RD_XZ=y,# CONFIG_INITRAMFS_COMPRESSION_XZ is not set\n# CONFIG_RD_XZ is not set)" >> $(LINUX_DIR)/.config
	echo -e "$(if $(CONFIG_TARGET_INITRAMFS_COMPRESSION_LZ4),CONFIG_INITRAMFS_COMPRESSION_LZ4=y\nCONFIG_RD_LZ4=y,# CONFIG_INITRAMFS_COMPRESSION_LZ4 is not set\n# CONFIG_RD_LZ4 is not set)" >> $(LINUX_DIR)/.config
  endef
else
endif

define Kernel/SetNoInitramfs
	mv $(LINUX_DIR)/.config.set $(LINUX_DIR)/.config.old
	grep -v INITRAMFS $(LINUX_DIR)/.config.old > $(LINUX_DIR)/.config.set
	echo 'CONFIG_INITRAMFS_SOURCE=""' >> $(LINUX_DIR)/.config.set
	echo '# CONFIG_INITRAMFS_FORCE is not set' >> $(LINUX_DIR)/.config.set
endef

define Kernel/Configure/Default
	rm -f $(LINUX_DIR)/localversion
	$(LINUX_CONF_CMD) > $(LINUX_DIR)/.config.target
# copy CONFIG_KERNEL_* settings over to .config.target
	awk '/^(#[[:space:]]+)?CONFIG_KERNEL/{sub("CONFIG_KERNEL_","CONFIG_");print}' $(TOPDIR)/.config >> $(LINUX_DIR)/.config.target
	echo "# CONFIG_KALLSYMS_EXTRA_PASS is not set" >> $(LINUX_DIR)/.config.target
	echo "# CONFIG_KALLSYMS_ALL is not set" >> $(LINUX_DIR)/.config.target
	echo "CONFIG_KALLSYMS_UNCOMPRESSED=y" >> $(LINUX_DIR)/.config.target
	$(SCRIPT_DIR)/package-metadata.pl kconfig $(TMP_DIR)/.packageinfo $(TOPDIR)/.config $(KERNEL_PATCHVER) > $(LINUX_DIR)/.config.override
	$(SCRIPT_DIR)/kconfig.pl 'm+' '+' $(LINUX_DIR)/.config.target /dev/null $(LINUX_DIR)/.config.override > $(LINUX_DIR)/.config.set
	$(call Kernel/SetNoInitramfs)
	rm -rf $(KERNEL_BUILD_DIR)/modules
	cmp -s $(LINUX_DIR)/.config.set $(LINUX_DIR)/.config.prev || { \
		cp $(LINUX_DIR)/.config.set $(LINUX_DIR)/.config; \
		cp $(LINUX_DIR)/.config.set $(LINUX_DIR)/.config.prev; \
	}
	$(_SINGLE) [ -d $(LINUX_DIR)/user_headers ] || $(KERNEL_MAKE) INSTALL_HDR_PATH=$(LINUX_DIR)/user_headers headers_install
	grep '=[ym]' $(LINUX_DIR)/.config.set | LC_ALL=C sort | mkhash md5 > $(LINUX_DIR)/.vermagic
endef

define Kernel/Configure/Initramfs
	$(call Kernel/SetInitramfs)
endef

define Kernel/CompileModules/Default
	rm -f $(LINUX_DIR)/vmlinux $(LINUX_DIR)/System.map
	+$(KERNEL_MAKE) modules
endef

OBJCOPY_STRIP = -R .reginfo -R .notes -R .note -R .comment -R .mdebug -R .note.gnu.build-id

# AMD64 shares the location with x86
ifeq ($(LINUX_KARCH),x86_64)
IMAGES_DIR:=../../x86/boot
endif

define Kernel/CopyImage
	cmp -s $(LINUX_DIR)/vmlinux $(KERNEL_BUILD_DIR)/vmlinux$(1).debug || { \
		$(KERNEL_CROSS)objcopy -O binary $(OBJCOPY_STRIP) -S $(LINUX_DIR)/vmlinux $(LINUX_KERNEL)$(1); \
		$(KERNEL_CROSS)objcopy $(OBJCOPY_STRIP) -S $(LINUX_DIR)/vmlinux $(KERNEL_BUILD_DIR)/vmlinux$(1).elf; \
		$(CP) $(LINUX_DIR)/vmlinux $(KERNEL_BUILD_DIR)/vmlinux$(1).debug; \
		$(foreach k, \
			$(if $(KERNEL_IMAGES),$(KERNEL_IMAGES),$(filter-out vmlinux dtbs,$(KERNELNAME))), \
			$(CP) $(LINUX_DIR)/arch/$(LINUX_KARCH)/boot/$(IMAGES_DIR)/$(k) $(KERNEL_BUILD_DIR)/$(k)$(1); \
		) \
	}
endef

define Kernel/CompileImage/Default
	rm -f $(TARGET_DIR)/init
	+$(KERNEL_MAKE) $(if $(KERNELNAME),$(KERNELNAME),all) modules
	$(call Kernel/CopyImage)
endef

ifneq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),)
define Kernel/CompileImage/Initramfs
	$(call Kernel/Configure/Initramfs)
	$(CP) $(GENERIC_PLATFORM_DIR)/other-files/init $(TARGET_DIR)/init
	rm -rf $(KERNEL_BUILD_DIR)/linux-$(LINUX_VERSION)/usr/initramfs_data.cpio*
	+$(KERNEL_MAKE) $(if $(KERNELNAME),$(KERNELNAME),all) modules
	$(call Kernel/CopyImage,-initramfs)
endef
else
define Kernel/CompileImage/Initramfs
endef
endif

define Kernel/Clean/Default
	rm -f $(KERNEL_BUILD_DIR)/linux-$(LINUX_VERSION)/.configured
	rm -f $(LINUX_KERNEL)
	$(_SINGLE)$(MAKE) -C $(KERNEL_BUILD_DIR)/linux-$(LINUX_VERSION) clean
endef


