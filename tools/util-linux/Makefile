#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=util-linux
PKG_VERSION:=2.39.3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/linux/utils/$(PKG_NAME)/v2.39
PKG_HASH:=7b6605e48d1a49f43cc4b4cfc59f313d0dd5402fa40b96810bd572e167dfed0f
PKG_CPE_ID:=cpe:/a:kernel:util-linux

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += \
	--disable-poman \
	--disable-nls \
	--disable-asciidoc \
	--disable-poman \
	--disable-libuuid \
	--disable-libblkid \
	--disable-libmount \
	--disable-libsmartcols \
	--disable-libfdisk \
	--disable-fdisks \
	--disable-mount \
	--disable-losetup \
	--disable-zramctl \
	--disable-fsck \
	--disable-partx \
	--disable-uuidd \
	--disable-uuidgen \
	--disable-blkid \
	--disable-wipefs \
	--disable-mountpoint \
	--disable-fallocate \
	--disable-unshare \
	--disable-nsenter \
	--disable-setpriv \
	--disable-hardlink \
	--disable-eject \
	--disable-agetty \
	--disable-cramfs \
	--disable-bfs \
	--disable-minix \
	--disable-hwclock \
	--disable-mkfs \
	--disable-fstrim \
	--disable-swapon \
	--disable-lscpu \
	--disable-lsfd \
	--disable-lslogins \
	--disable-wdctl \
	--disable-cal \
	--disable-logger \
	--disable-whereis \
	--disable-pipesz \
	--disable-waitpid \
	--disable-switch_root \
	--disable-pivot_root \
	--disable-lsmem \
	--disable-chmem \
	--disable-ipcmk \
	--disable-ipcrm \
	--disable-ipcs \
	--disable-irqtop \
	--disable-lsirq \
	--disable-lsns \
	--disable-rfkill \
	--disable-scriptutils \
	--disable-tunelp \
	--disable-kill \
	--disable-last \
	--disable-utmpdump \
	--disable-line \
	--disable-mesg \
	--disable-raw \
	--disable-rename \
	--disable-vipw \
	--disable-newgrp \
	--disable-chfn-chsh \
	--disable-login \
	--disable-nologin \
	--disable-sulogin \
	--disable-su \
	--disable-runuser \
	--disable-ul \
	--disable-more \
	--disable-pg \
	--disable-setterm \
	--disable-schedutils \
	--disable-wall \
	--disable-write \
	--disable-bash-completion \
	--disable-pylibmount \
	--disable-pg-bell \
	--without-util \
	--without-selinux \
	--without-audit \
	--without-udev \
	--without-ncursesw \
	--without-ncurses \
	--without-slang \
	--without-tinfo \
	--without-readline \
	--without-utempter \
	--without-cap-ng \
	--without-libz \
	--without-libmagic \
	--without-user \
	--without-btrfs \
	--without-systemd \
	--without-smack \
	--without-econf \
	--without-python \
	--without-cryptsetup

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/hexdump $(STAGING_DIR_HOST)/bin/
endef

define Host/Uninstall
	rm -f $(STAGING_DIR_HOST)/bin/hexdump
endef

$(eval $(call HostBuild))
