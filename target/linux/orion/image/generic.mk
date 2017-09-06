#
# Copyright (C) 2008-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

### DO NOT INDENT LINES CONTAINING $(call xyz) AS THIS MAY CHANGE THE CONTEXT
### OF THE FIRST LINE IN THE CALLED VARIABLE (NOTE: variable!)
### see http://www.gnu.org/software/make/manual/html_node/Call-Function.html#Call-Function
### ACTUALLY IT IS A SIMPLE MACRO EXPANSION

### use round brackets for make variables, and curly brackets for shell variables


## Kernel mtd partition size in KiB
KERNEL_MTD_SIZE:=1280

# Netgear WNR854T: erase size is 128KiB = 0x00020000 = 131072
ERASE_SIZE_128K:=128

# Linksys WRT350N v2: erase size is 64KiB = 0x00010000 = 65536
ERASE_SIZE_64K:=64

# define JFFS2 sizes for include/image.mk
JFFS2_BLOCKSIZE:=64k 128k


###
### Image/BuildKernel
###

define Image/BuildKernel
### Dummy comment for indented calls of Image/BuildKernel

 ## Netgear WN802T: mach id 3306 (0x0cea)
$(call Image/BuildKernel/ARM/zImage,wn802t,"\x0c\x1c\xa0\xe3\xea\x10\x81\xe3")
$(call Image/BuildKernel/ARM/uImage,wn802t)
ifeq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),y)
$(call Image/BuildKernel/ARM/zImage,wn802t,"\x0c\x1c\xa0\xe3\xea\x10\x81\xe3",-initramfs)
$(call Image/BuildKernel/ARM/uImage,wn802t,-initramfs)
endif
 ifneq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),y)  # nothing more to do for a ramdisk build
$(call Image/BuildKernel/JFFS2uImage,wn802t,$(ERASE_SIZE_64K),uImage)
$(call Image/Default/FileSizeCheck,$(KDIR)/wn802t-uImage.jffs2,$(shell expr $(KERNEL_MTD_SIZE) \* 1024))
 endif

 ## Netgear WNR854T: mach id 1801 (0x0709)
$(call Image/BuildKernel/ARM/zImage,wnr854t,"\x07\x1c\xa0\xe3\x09\x10\x81\xe3")
$(call Image/BuildKernel/ARM/uImage,wnr854t)
ifeq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),y)
$(call Image/BuildKernel/ARM/zImage,wnr854t,"\x07\x1c\xa0\xe3\x09\x10\x81\xe3",-initramfs)
$(call Image/BuildKernel/ARM/uImage,wnr854t,-initramfs)
endif
 ifneq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),y)  # nothing more to do for a ramdisk build
$(call Image/BuildKernel/JFFS2uImage,wnr854t,$(ERASE_SIZE_128K),uImage)
$(call Image/Default/FileSizeCheck,$(KDIR)/wnr854t-uImage.jffs2,$(shell expr $(KERNEL_MTD_SIZE) \* 1024))
 endif

 ## Linksys WRT350N v2: mach id 1633 (0x0661)
$(call Image/BuildKernel/ARM/zImage,wrt350nv2,"\x06\x1c\xa0\xe3\x61\x10\x81\xe3")
$(call Image/BuildKernel/ARM/uImage,wrt350nv2)
ifeq ($($CONFIG_TARGET_ROOTFS_INITRAMFS),y)
$(call Image/BuildKernel/ARM/zImage,wrt350nv2,"\x06\x1c\xa0\xe3\x61\x10\x81\xe3",-initramfs)
$(call Image/BuildKernel/ARM/uImage,wrt350nv2-initramfs)
endif
 ifneq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),y)  # nothing more to do for a ramdisk build
$(call Image/Default/FileSizeCheck,$(KDIR)/wrt350nv2-uImage,$(shell expr $(KERNEL_MTD_SIZE) \* 1024))
 endif

 ## Buffalo Terastation Pro II/Live: mach id 1584 (0x0630)
$(call Image/BuildKernel/ARM/zImage,terastation-pro2,"\x06\x1c\xa0\xe3\x30\x10\x81\xe3")
$(call Image/BuildKernel/ARM/uImage,terastation-pro2)
ifeq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),y)
$(call Image/BuildKernel/ARM/zImage,terastation-pro2,"\x06\x1c\xa0\xe3\x30\x10\x81\xe3",-initramfs)
$(call Image/BuildKernel/ARM/uImage,terastation-pro2,-initramfs)
endif
endef

define Image/BuildKernel/ARM/zImage
 # merge machine id and regular zImage into one file
 # parameters: 1 = machine name, 2 = machine id as string in quotes
	# $(BOARD) kernel zImage for $(1)
	echo -en $(2) > '$(KDIR)/$(1)-zImage$(3)'
	cat '$(KDIR)/zImage$(3)' >> '$(KDIR)/$(1)-zImage$(3)'
endef

define Image/BuildKernel/ARM/uImage
 # create uImage from zImage
 # parameters: 1 = machine name
	# $(BOARD) kernel uImage for $(1)
	'$(STAGING_DIR_HOST)/bin/mkimage' -A arm -O linux -T kernel \
	-C none -a 0x00008000 -e 0x00008000 -n 'Linux-$(LINUX_VERSION)' \
	-d '$(KDIR)/$(1)-zImage$(2)' '$(KDIR)/$(1)-uImage$(2)'
 ifeq ($(2),-initramfs) # only copy uImage for ramdisk build
	cp '$(KDIR)/$(1)-uImage-initramfs' '$(BIN_DIR)/openwrt-$(1)-uImage-initramfs'
 endif
endef

define Image/BuildKernel/JFFS2uImage
 # create JFFS2 partition with uImage file (result is already padded to erase size)
 # parameters: 1 = machine name, 2 = erase size in KiB, 3 = uImage file name
	# $(BOARD) kernel uImage for $(1) in JFFS2-$(2)k partition
	rm -rf '$(TMP_DIR)/$(1)_jffs2_uimage'
	mkdir '$(TMP_DIR)/$(1)_jffs2_uimage'
	cp '$(KDIR)/$(1)-uImage' '$(TMP_DIR)/$(1)_jffs2_uimage/$(3)'
	$(STAGING_DIR_HOST)/bin/mkfs.jffs2 --compression-mode=none --pad --little-endian --squash -e $(2)KiB -o '$(KDIR)/$(1)-uImage.jffs2' -d '$(TMP_DIR)/$(1)_jffs2_uimage'
	rm -rf '$(TMP_DIR)/$(1)_jffs2_uimage'
endef

define Image/Default/FileSizeCheck
 # parameters: 1 = file path, 2 = maximum size in bytes
	[ `stat -c %s '$(1)'` -le $(2) ] || { echo '   ERROR: $(1) too big (> $(2) bytes)'; rm -f $(1); }
endef


###
### Image/Build
###

define Image/Build
### Dummy comment for indented calls of Image/Build with $(1)

 ## Prepare rootfs
$(call Image/Build/$(1),$(1))

 ## Netgear WN802T
$(call Image/Build/Default,$(1),wn802t,$(ERASE_SIZE_64K),$(KERNEL_MTD_SIZE),.jffs2,NG_WN802T)

 ## Netgear WNR854T
$(call Image/Build/Default,$(1),wnr854t,$(ERASE_SIZE_128K),$(KERNEL_MTD_SIZE),.jffs2,NG_WNR854T)

 ## Linksys WRT350N v2
$(call Image/Build/Linksys/wrt350nv2,$(1),wrt350nv2,$(ERASE_SIZE_64K),$(KERNEL_MTD_SIZE),)

 ## Buffalo Terastation Pro II/Live
$(call Image/Build/Default,$(1),terastation-pro2,$(ERASE_SIZE_128K),$(KERNEL_MTD_SIZE),.jffs2,TERASTATION_PRO2)
endef

define Image/Build/squashfs
$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
endef

## generate defines for all JFFS2 block sizes
define Image/Build/jffs2/sub
 $(eval define Image/Build/jffs2-$(1)
	cp '$$(KDIR)/root.jffs2-$(1)' '$$(BIN_DIR)/$$(IMG_PREFIX)-root.jffs2-$(1)'
 endef)
endef

$(foreach SZ,$(JFFS2_BLOCKSIZE),$(call Image/Build/jffs2/sub,$(SZ)))

define Image/Build/Default
 # parameters: 1 = rootfs type, 2 = machine name, 3 = erase size in KiB, 4 = kernel mtd size in KiB, 5 = kernel file suffix, 6 = header
 ifeq ($(findstring jffs2-,$(1)),jffs2-)  # JFFS2: build only image fitting to erase size
  ifeq ($(1),jffs2-$(3)k)
$(call Image/Build/Default/sysupgrade,$(1),$(2),$(4),$(5))
$(call Image/Build/Default/factory,$(1),$(2),$(6))
  endif
 else
  ifeq ($(1),ext4)  # EXT4: ignore
   # do nothing
  else  # do all other images
$(call Image/Build/Default/sysupgrade,$(1),$(2),$(4),$(5))
$(call Image/Build/Default/factory,$(1),$(2),$(6))
  endif
 endif
endef

define Image/Build/Default/sysupgrade
 # parameters: 1 = rootfs type, 2 = machine name, 3 = pad size in KiB (kernel mtd size or erase size), 4 = kernel file suffix
	# $(BOARD) $(1) sysupgrade image for $(2)
	( \
		dd if='$(KDIR)/$(2)-uImage$(4)' bs=$(3)k conv=sync; \
		dd if='$(KDIR)/root.$(1)'; \
	) > '$(BIN_DIR)/openwrt-$(2)-$(1)-sysupgrade.img'
endef

define Image/Build/Default/factory
 # parameters: 1 = rootfs type, 2 = machine name, 3 = header
	# $(BOARD) $(1) factory upgrade image for $(2)
	'$(STAGING_DIR_HOST)/bin/add_header' $(3) '$(BIN_DIR)/openwrt-$(2)-$(1)-sysupgrade.img' '$(BIN_DIR)/openwrt-$(2)-$(1)-factory.img'
endef

##
## Image/Build/Linksys
##

define Image/Build/Linksys/wrt350nv2
 # parameters: 1 = rootfs type, 2 = machine name, 3 = erase size in KiB, 4 = kernel mtd size in KiB, 5 = kernel file suffix
 ifeq ($(findstring jffs2-,$(1)),jffs2-)  # JFFS2: build only image fitting to erase size
  ifeq ($(1),jffs2-$(3)k)
$(call Image/Build/Default/sysupgrade,$(1),$(2),$(4),$(5))
$(call Image/Build/Linksys/wrt350nv2-builder,$(1),$(2))
  endif
 else
  ifeq ($(1),ext4)  # EXT4: ignore
   # do nothing
  else  # do all other images
$(call Image/Build/Default/sysupgrade,$(1),$(2),$(4),$(5))
$(call Image/Build/Linksys/wrt350nv2-builder,$(1),$(2))
  endif
 endif
endef

define Image/Build/Linksys/wrt350nv2-builder
 # parameters: 1 = rootfs type, 2 = machine name
	# $(BOARD) $(1) factory and recovery image for $(2) via wrt350nv2-builder
	rm -rf '$(TMP_DIR)/$(2)_factory'
	mkdir '$(TMP_DIR)/$(2)_factory'
 # create parameter file
	echo ':image 0 $(BIN_DIR)/openwrt-$(2)-$(1)-sysupgrade.img' > '$(TMP_DIR)/$(2)_factory/$(2).par'
	[ ! -f '$(STAGING_DIR_HOST)/share/wrt350nv2-builder/u-boot.bin' ] || ( \
		echo ':u-boot 0 $(STAGING_DIR_HOST)/share/wrt350nv2-builder/u-boot.bin' >> '$(TMP_DIR)/$(2)_factory/$(2).par'; \
	)
	echo '#version 0x2020' >> '$(TMP_DIR)/$(2)_factory/$(2).par'
 # create bin file for recovery and factory image
	-( \
		cd '$(TMP_DIR)/$(2)_factory'; \
		'$(STAGING_DIR_HOST)/bin/wrt350nv2-builder' -b '$(TMP_DIR)/$(2)_factory/$(2).par'; \
	) && $(CP) '$(TMP_DIR)/$(2)_factory/wrt350n.bin' '$(BIN_DIR)/openwrt-$(2)-$(1)-recovery.bin' && \
	( \
		cd '$(TMP_DIR)/$(2)_factory'; \
		zip 'wrt350n.zip' 'wrt350n.bin'; \
	) && '$(STAGING_DIR_HOST)/bin/wrt350nv2-builder' -z '$(TMP_DIR)/$(2)_factory/wrt350n.zip' '$(BIN_DIR)/openwrt-$(2)-$(1)-factory.img'
	rm -rf '$(TMP_DIR)/$(2)_factory'
endef


###
### Image/PreReq
###

## Dependency for WRT350N v2 factory image
$(eval $(call RequireCommand,zip, \
	Please install zip. \
))
