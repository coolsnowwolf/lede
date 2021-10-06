#
# Copyright (C) 2006-2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME := firmware-utils
PKG_RELEASE := 7

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
	$(call cc,add_header,-Wall)
	$(call cc,addfwhdr cyg_crc32,-Wall)
	$(call cc,addpattern,-Wall)
	$(call cc,asustrx,-Wall)
	$(call cc,bcm4908asus,-Wall)
	$(call cc,bcm4908kernel,-Wall)
	$(call cc,buffalo-enc buffalo-lib,-Wall)
	$(call cc,buffalo-tag buffalo-lib,-Wall)
	$(call cc,buffalo-tftp buffalo-lib,-Wall)
	$(call cc,dgfirmware,-Wall)
	$(call cc,dgn3500sum,-Wall)
	$(call cc,dns313-header,-Wall)
	$(call cc,edimax_fw_header,-Wall)
	$(call cc,encode_crc,-Wall)
	$(call cc,fix-u-media-header cyg_crc32,-Wall)
	$(call cc,hcsmakeimage bcmalgo,-Wall)
	$(call cc,imagetag imagetag_cmdline cyg_crc32,-Wall)
	$(call cc,jcgimage,-lz -Wall)
	$(call cc,lxlfw,-Wall)
	$(call cc,lzma2eva,-lz -Wall)
	$(call cc,makeamitbin,-Wall)
	$(call cc,mkbrncmdline,-Wall)
	$(call cc,mkbrnimg,-Wall)
	$(call cc,mkbuffaloimg,-Wall)
	$(call cc,mkcameofw,-Wall)
	$(call cc,mkcasfw,-Wall)
	$(call cc,mkchkimg,-Wall)
	$(call cc,mkcsysimg,-Wall)
	$(call cc,mkdapimg,-Wall)
	$(call cc,mkdapimg2,-Wall)
	$(call cc,mkdhpimg buffalo-lib,-Wall)
	$(call cc,mkdlinkfw mkdlinkfw-lib,-lz -Wall --std=c99)
	$(call cc,mkdniimg,-Wall)
	$(call cc,mkedimaximg,-Wall)
	$(call cc,mkfwimage,-lz -Wall -Werror -Wextra -D_FILE_OFFSET_BITS=64)
	$(call cc,mkfwimage2,-lz -Wall)
	$(call cc,mkheader_gemtek,-lz -Wall)
	$(call cc,mkhilinkfw,-lcrypto -Wall)
	$(call cc,mkmerakifw sha1,-Wall)
	$(call cc,mkmerakifw-old,-Wall)
	$(call cc,mkmylofw,-Wall)
	$(call cc,mkplanexfw sha1,-Wall)
	$(call cc,mkporayfw,-Wall)
	$(call cc,mkrasimage,--std=gnu99 -Wall)
	$(call cc,mkrtn56uimg,-lz -Wall)
	$(call cc,mksenaofw md5,-Wall --std=gnu99)
	$(call cc,mksercommfw,-Wall)
	$(call cc,mktitanimg,-Wall)
	$(call cc,mktplinkfw mktplinkfw-lib md5,-Wall -fgnu89-inline)
	$(call cc,mktplinkfw2 mktplinkfw-lib md5,-Wall -fgnu89-inline)
	$(call cc,mkwrggimg md5,-Wall)
	$(call cc,mkwrgimg md5,-Wall)
	$(call cc,mkzcfw cyg_crc32,-Wall)
	$(call cc,mkzynfw,-Wall)
	$(call cc,motorola-bin,-Wall)
	$(call cc,nand_ecc,-Wall)
	$(call cc,nec-enc,-Wall --std=gnu99)
	$(call cc,osbridge-crc,-Wall)
	$(call cc,oseama md5,-Wall)
	$(call cc,otrx,-Wall)
	$(call cc,pc1crypt)
	$(call cc,ptgen cyg_crc32,-Wall)
	$(call cc,seama md5,-Wall)
	$(call cc,sign_dlink_ru md5,-Wall)
	$(call cc,spw303v,-Wall)
	$(call cc,srec2bin)
	$(call cc,tplink-safeloader md5,-Wall --std=gnu99)
	$(call cc,trx,-Wall)
	$(call cc,trx2edips,-Wall)
	$(call cc,trx2usr,-Wall)
	$(call cc,uimage_padhdr,-Wall -lz)
	$(call cc,wrt400n cyg_crc32,-Wall)
	$(call cc,xorimage,-Wall)
	$(call cc,zyimage,-Wall)
	$(call cc,zytrx,-Wall)
	$(call cc,zyxbcm,-Wall)
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/bin/* $(STAGING_DIR_HOST)/bin/
endef

$(eval $(call HostBuild))
