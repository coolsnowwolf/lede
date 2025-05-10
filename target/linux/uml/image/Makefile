# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2010 OpenWrt.org

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

define Image/Prepare
	cp $(LINUX_DIR)/linux $(KDIR)/vmlinux.elf
endef

define Image/Build/squashfs
	dd if=/dev/zero of=$(KDIR)/root.squashfs bs=1024k count=0 seek=$(CONFIG_TARGET_ROOTFS_PARTSIZE)
endef

define Image/Build
	$(call Image/Build/$(1))
	cp $(KDIR)/root.$(1) $(BIN_DIR)/$(IMG_PREFIX)-$(1).img
	cp $(KDIR)/vmlinux.elf $(BIN_DIR)/$(IMG_PREFIX)-vmlinux
endef

$(eval $(call BuildImage))

# UML requires linking against several glibc static libraries: libutil, librt
# and libpthread, check that here. We do not check against libpthread
# specifically because getting something to build that references a libpthread
# symbol is pretty involved and Linux distributions package these 3 libraries
# in the same package.

$(eval $(call TestHostCommand,glibc-static, \
	Please install a static glibc package. (Missing libutil.a, librt.a or libpthread.a), \
	echo 'int main(int argc, char **argv) { login(0); timer_gettime(0, 0); return 0; }' | \
		gcc -include utmp.h -include time.h -x c -o $(TMP_DIR)/a.out - -static -lutil -lrt))
