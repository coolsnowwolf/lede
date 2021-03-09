#
# Copyright (C) 2006-2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME := firmware-utils
PKG_RELEASE := 2

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/kernel.mk

define cc
	$(HOSTCC) \
		$(HOST_CFLAGS) \
		-Wno-unused-parameter \
		-include endian.h $(HOST_LDFLAGS) \
		-o $(HOST_BUILD_DIR)/bin/$(firstword $(1)) \
		$(foreach src,$(1),src/$(src).c) \
		$(2)
endef

define Host/Compile
	mkdir -p $(HOST_BUILD_DIR)/bin
	$(call cc,add_header)
	$(call cc,addpattern)
	$(call cc,asustrx)
	$(call cc,buffalo-enc buffalo-lib,-Wall)
	$(call cc,buffalo-tag buffalo-lib,-Wall)
	$(call cc,buffalo-tftp buffalo-lib,-Wall)
	$(call cc,dgfirmware)
	$(call cc,dgn3500sum,-Wall)
	$(call cc,dns313-header,-Wall)
	$(call cc,edimax_fw_header,-Wall)
	$(call cc,encode_crc)
	$(call cc,fix-u-media-header cyg_crc32,-Wall)
	$(call cc,hcsmakeimage bcmalgo)
	$(call cc,imagetag imagetag_cmdline cyg_crc32)
	$(call cc,jcgimage,-lz -Wall)
	$(call cc,lxlfw)
	$(call cc,lzma2eva,-lz)
	$(call cc,makeamitbin)
	$(call cc,mkbrncmdline)
	$(call cc,mkbrnimg)
	$(call cc,mkbuffaloimg,-Wall)
	$(call cc,mkcameofw,-Wall)
	$(call cc,mkcasfw)
	$(call cc,mkchkimg)
	$(call cc,mkcsysimg)
	$(call cc,mkdapimg)
	$(call cc,mkdapimg2)
	$(call cc,mkdhpimg buffalo-lib,-Wall)
	$(call cc,mkdlinkfw mkdlinkfw-lib,-lz -Wall --std=c99)
	$(call cc,mkdniimg)
	$(call cc,mkedimaximg)
	$(call cc,mkfwimage,-lz -Wall -Werror -Wextra -D_FILE_OFFSET_BITS=64)
	$(call cc,mkfwimage2,-lz)
	$(call cc,mkheader_gemtek,-lz)
	$(call cc,mkhilinkfw,-lcrypto)
	$(call cc,mkmerakifw sha1,-Wall)
	$(call cc,mkmerakifw-old,-Wall)
	$(call cc,mkmylofw)
	$(call cc,mkplanexfw sha1)
	$(call cc,mkporayfw,-Wall)
	$(call cc,mkrasimage,--std=gnu99)
	$(call cc,mkrtn56uimg,-lz)
	$(call cc,mksenaofw md5,-Wall --std=gnu99)
	$(call cc,mksercommfw,-Wall)
	$(call cc,mktitanimg)
	$(call cc,mktplinkfw mktplinkfw-lib md5,-Wall -fgnu89-inline)
	$(call cc,mktplinkfw2 mktplinkfw-lib md5,-fgnu89-inline)
	$(call cc,mkwrggimg md5,-Wall)
	$(call cc,mkwrgimg md5,-Wall)
	$(call cc,mkzcfw cyg_crc32)
	$(call cc,mkzynfw)
	$(call cc,motorola-bin)
	$(call cc,nand_ecc)
	$(call cc,nec-enc,-Wall --std=gnu99)
	$(call cc,osbridge-crc)
	$(call cc,oseama md5,-Wall)
	$(call cc,otrx)
	$(call cc,pc1crypt)
	$(call cc,ptgen cyg_crc32)
	$(call cc,seama md5)
	$(call cc,spw303v)
	$(call cc,srec2bin)
	$(call cc,tplink-safeloader md5,-Wall --std=gnu99)
	$(call cc,trx)
	$(call cc,trx2edips)
	$(call cc,trx2usr)
	$(call cc,uimage_padhdr,-Wall -lz)
	$(call cc,wrt400n cyg_crc32)
	$(call cc,xorimage)
	$(call cc,zyimage,-Wall)
	$(call cc,zyxbcm)
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/bin/* $(STAGING_DIR_HOST)/bin/
endef

$(eval $(call HostBuild))
