#
# Copyright (C) 2007-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define trxedimax/jffs2-128k
-a 0x20000 -f $(KDIR)/root.jffs2-128k
endef

define trxedimax/jffs2-64k
-a 0x10000 -f $(KDIR)/root.jffs2-64k
endef

define trxedimax/squashfs
-a 1024 -f $(KDIR)/root.squashfs
endef

define Image/Build/TRXEdimax
	$(STAGING_DIR_HOST)/bin/trx -o $(1) -f $(KDIR)/vmlinux.lzma \
		$(call trxedimax/$(2))
endef

define Image/Build/Compex
	$(call Image/Build/Loader,$(2),gz,0x80500000,0,y,$(2))
	$(call Image/Build/TRX,$(call imgname,$(1),$(2)).trx,$(1),$(KDIR)/loader-$(2).gz)
endef

define Image/Build/Edimax
	$(call Image/Build/Loader,$(2),gz,0x80500000,0x6D8,y,$(2))
	$(call Image/Build/TRXEdimax,$(call imgname,$(1),$(2)).trx,$(1))
	$(STAGING_DIR_HOST)/bin/mkcsysimg -B $(2) -d -w \
		-r $(KDIR)/loader-$(2).gz::0x1000 \
		-x $(call imgname,$(1),$(2)).trx:0x10000 \
		-x $(JFFS2MARK):0x10000 \
		$(call imgname,$(1),$(2))-webui.bin
	$(STAGING_DIR_HOST)/bin/mkcsysimg -B $(2) -d \
		-r $(KDIR)/loader-$(2).gz::0x1000 \
		-x $(call imgname,$(1),$(2)).trx:0x10000 \
		-x $(JFFS2MARK):0x10000 \
		$(call imgname,$(1),$(2))-xmodem.bin
	rm -f $(call imgname,$(1),$(2)).trx
endef

define Image/Build/Osbridge
	$(call Image/Build/Loader,$(2),gz,0x80500000,0x6D8,y,$(2))
	$(call Image/Build/TRXEdimax,$(call imgname,$(1),$(2)).trx,$(1))
	$(STAGING_DIR_HOST)/bin/mkcsysimg -B $(2) -d \
		-r $(KDIR)/loader-$(2).gz::0x1000 \
		-x $(call imgname,$(1),$(2)).trx:0x10000 \
		-x $(JFFS2MARK):0x10000 \
		$(call imgname,$(1),$(2))-firmware.bin
	$(STAGING_DIR_HOST)/bin/osbridge-crc \
		-i $(call imgname,$(1),$(2))-firmware.bin \
		-o $(call imgname,$(1),$(2))-temp.bin
	$(STAGING_DIR_HOST)/bin/pc1crypt \
		-i $(call imgname,$(1),$(2))-temp.bin \
		-o $(call imgname,$(1),$(2))-webui.bin
	rm -f $(call imgname,$(1),$(2))-temp.bin
	rm -f $(call imgname,$(1),$(2)).trx
endef

define Image/Build/Infineon
	$(call Image/Build/Loader,$(2),gz,0x80500000,0x6D8,y,$(2))
	$(call Image/Build/TRXNoloader,$(call imgname,$(1),$(2)).trx,$(1))
	dd if=$(KDIR)/loader-$(2).gz of=$(call imgname,$(1),$(2)).img bs=64k conv=sync
	cat $(call imgname,$(1),$(2)).trx >> $(call imgname,$(1),$(2)).img
endef

define Image/Build/Cellvision
	$(call Image/Build/Loader,$(2),bin,0x80500000,0x6D8,y,$(3))
	mkdir -p $(BIN_DIR)/tmp
	cp $(KDIR)/loader-$(2).bin $(BIN_DIR)/tmp/vmlinux.bin
	gzip -9n $(BIN_DIR)/tmp/vmlinux.bin
	dd if=$(BIN_DIR)/tmp/vmlinux.bin.gz of=$(call imgname,$(1),$(2))-xmodem.bin bs=64k conv=sync
	rm -rf $(BIN_DIR)/tmp
	$(call Image/Build/TRXNoloader,$(call imgname,$(1),$(2)).trx,$(1))
	cat $(call imgname,$(1),$(2)).trx >> $(call imgname,$(1),$(2))-xmodem.bin
	$(STAGING_DIR_HOST)/bin/mkcasfw -B $(2) -d \
		-K $(call imgname,$(1),$(2))-xmodem.bin \
		$(call imgname,$(1),$(2))-webui.bin
endef

define Image/Build/Cellvision2
	# only for CAS-700/771/790/861
	$(call Image/Build/Loader,$(2),gz,0x80500000,0x6D8,y,$(3))
	$(call Image/Build/TRXNoloader,$(call imgname,$(1),$(2)).trx,$(1))
	dd if=$(KDIR)/loader-$(2).gz of=$(call imgname,$(1),$(2)).bin bs=64k conv=sync
	cat $(call imgname,$(1),$(2)).trx >> $(call imgname,$(1),$(2)).bin
	echo -ne '\x14\x07\x24\x06$(2)' | dd bs=14 count=1 conv=sync >> $(call imgname,$(1),$(2)).bin
	echo -ne 'OpenWrt\x00\x00\x00' >> $(call imgname,$(1),$(2)).bin
endef

define Image/Build/MyLoader
	$(call Image/Build/Loader,$(2),gz,0x80500000,0)
	$(call Image/Build/TRXNoloader,$(call imgname,$(1),$(2)).trx,$(1))
	$(STAGING_DIR_HOST)/bin/mkmylofw -B $(2) \
		-p0x20000:0x10000:ahp:0x80001000 \
		-p0x30000:0 \
		-b0x20000:0x10000:h:$(KDIR)/loader-$(2).gz \
		-b0x30000:0::$(call imgname,$(1),$(2)).trx \
		$(call imgname,$(1),$(2)).bin
endef

#
# Cellvision CAS-630/630W, CAS-670/670W, NFS-101U/101WU, NFS-202U/202WU
#
define Image/Build/Template/Cellvision
	$(call Image/Build/Cellvision,$(1),$(2),$(3))
endef

define Image/Build/Template/Cellvision/squashfs
	$(call Image/Build/Template/Cellvision,squashfs,$(1),$(2))
endef

define Image/Build/Template/Cellvision/jffs2-64k
	$(call Image/Build/Template/Cellvision,jffs2-64k,$(1),$(2))
endef

#
# Cellvision CAS-700/700W, CAS-771/771W, CAS-790, CAS-861/861W
#
define Image/Build/Template/Cellvision2
	$(call Image/Build/Cellvision2,$(1),$(2),$(3))
endef

define Image/Build/Template/Cellvision2/squashfs
	$(call Image/Build/Template/Cellvision2,squashfs,$(1),$(2))
endef

define Image/Build/Template/Cellvision2/jffs2-64k
	$(call Image/Build/Template/Cellvision2,jffs2-64k,$(1),$(2))
endef

define Image/Build/Template/Cellvision2/Initramfs
	$(call Image/Build/LZMAKernel/Cellvision,$(1),$(2),gz)
endef

#
# Compex NP27G, NP28G, WP54G, WP54AG, WPP54G, WPP54AG
#
define Image/Build/Template/Compex
	$(call Image/Build/MyLoader,$(1),$(2))
endef

define Image/Build/Template/Compex/squashfs
	$(call Image/Build/Template/Compex,squashfs,$(1))
endef

define Image/Build/Template/Compex/jffs2-64k
	$(call Image/Build/Template/Compex,jffs2-64k,$(1))
endef

define Image/Build/Template/Compex/Initramfs
	$(call Image/Build/LZMAKernel/Generic,$(1),bin)
endef

#
# Compex WP54G-WRT
#
define Image/Build/Template/WP54GWRT
	$(call Image/Build/Compex,$(1),wp54g-wrt)
endef

define Image/Build/Template/WP54GWRT/squashfs
	$(call Image/Build/Template/WP54GWRT,squashfs)
endef

define Image/Build/Template/WP54GWRT/jffs2-64k
	$(call Image/Build/Template/WP54GWRT,jffs2-64k)
endef

define Image/Build/Template/WP54GWRT/Initramfs
	$(call Image/Build/LZMAKernel/KArgs,wp54g-wrt,bin)
endef

#
# Edimax BR-6104K, BR-6104KP, BR-6104Wg, BR-6114WG
#
define Image/Build/Template/Edimax
	$(call Image/Build/Edimax,$(1),$(2))
endef

define Image/Build/Template/Edimax/squashfs
	$(call Image/Build/Template/Edimax,squashfs,$(1))
endef

define Image/Build/Template/Edimax/Initramfs
	$(call Image/Build/LZMAKernel/Admboot,$(1),gz)
endef

#
# Infineon EASY 5120, EASY 83000
#
define Image/Build/Template/Infineon
	$(call Image/Build/Infineon,$(1),$(2))
endef

define Image/Build/Template/Infineon/squashfs
	$(call Image/Build/Template/Infineon,squashfs,$(1))
endef

define Image/Build/Template/Infineon/jffs2-64k
	$(call Image/Build/Template/Infineon,jffs2-64k,$(1))
endef

define Image/Build/Template/Infineon/Initramfs
	$(call Image/Build/LZMAKernel/Admboot,$(1),gz)
endef

#
# Generic EB-214A
#
define Image/Build/Template/Edimax/Initramfs
	$(call Image/Build/LZMAKernel/Admboot,eb-214a,bin)
endef


#
# Mikrotik RouterBOARD 1xx
#
define Image/Build/Template/Mikrotik/Initramfs
	$(CP) $(KDIR)/vmlinux.elf $(call imgname,netboot,rb1xx)
endef

#
# OSBRiDGE 5GXi/5XLi
#
define Image/Build/Template/Osbridge
	$(call Image/Build/Osbridge,$(1),$(2))
endef

define Image/Build/Template/Osbridge/squashfs
	$(call Image/Build/Template/Osbridge,squashfs,$(1))
endef

define Image/Build/Template/Osbridge/Initramfs
	$(call Image/Build/LZMAKernel/Admboot,$(1),gz)
endef

#
# Profiles
#
define Image/Build/Profile/CAS630
	$(call Image/Build/Template/Cellvision/$(1),cas-630,cas-630)
endef

define Image/Build/Profile/CAS630W
	$(call Image/Build/Template/Cellvision/$(1),cas-630w,cas-630)
endef

define Image/Build/Profile/CAS670
	$(call Image/Build/Template/Cellvision/$(1),cas-670,cas-670)
endef

define Image/Build/Profile/CAS670W
	$(call Image/Build/Template/Cellvision/$(1),cas-670w,cas-670)
endef

define Image/Build/Profile/NFS101U
	$(call Image/Build/Template/Cellvision/$(1),nfs-101u,nfs-101u)
	$(call Image/Build/Template/Cellvision/$(1),dn-7013,nfs-101u)
	$(call Image/Build/Template/Cellvision/$(1),dns-120,nfs-101u)
	$(call Image/Build/Template/Cellvision/$(1),mu-5000fs,nfs-101u)
	$(call Image/Build/Template/Cellvision/$(1),tn-u100,nfs-101u)
	$(call Image/Build/Template/Cellvision/$(1),cg-nsadp,nfs-101u)
endef

define Image/Build/Profile/NFS101WU
	$(call Image/Build/Template/Cellvision/$(1),nfs-101wu,nfs-101u)
	$(call Image/Build/Template/Cellvision/$(1),dns-g120,nfs-101u)
endef

define Image/Build/Profile/CAS700
	$(call Image/Build/Template/Cellvision2/$(1),cas-700,cas-700)
endef

define Image/Build/Profile/CAS700W
	$(call Image/Build/Template/Cellvision2/$(1),cas-700w,cas-700)
endef

define Image/Build/Profile/CAS771
	$(call Image/Build/Template/Cellvision2/$(1),cas-771,cas-771)
endef

define Image/Build/Profile/CAS771W
	$(call Image/Build/Template/Cellvision2/$(1),cas-771w,cas-771)
endef

define Image/Build/Profile/CAS790
	$(call Image/Build/Template/Cellvision2/$(1),cas-790,cas-790)
endef

define Image/Build/Profile/CAS861
	$(call Image/Build/Template/Cellvision2/$(1),cas-861,cas-861)
endef

define Image/Build/Profile/CAS861W
	$(call Image/Build/Template/Cellvision2/$(1),cas-861w,cas-861)
endef

define Image/Build/Profile/NP27G
	$(call Image/Build/Template/Compex/$(1),np27g)
endef

define Image/Build/Profile/NP28G
	$(call Image/Build/Template/Compex/$(1),np28g)
endef

define Image/Build/Profile/WP54
	$(call Image/Build/Template/Compex/$(1),wp54g)
	$(call Image/Build/Template/Compex/$(1),wp54ag)
	$(call Image/Build/Template/Compex/$(1),wpp54g)
	$(call Image/Build/Template/Compex/$(1),wpp54ag)
	$(call Image/Build/Template/WP54GWRT/$(1))
endef

define Image/Build/Profile/BR6104K
	$(call Image/Build/Template/Edimax/$(1),br-6104k)
endef

define Image/Build/Profile/BR6104KP
	$(call Image/Build/Template/Edimax/$(1),br-6104kp)
endef

define Image/Build/Profile/BR6104WG
	$(call Image/Build/Template/Edimax/$(1),br-6104wg)
endef

define Image/Build/Profile/BR6114WG
	$(call Image/Build/Template/Edimax/$(1),br-6114wg)
endef

define Image/Build/Profile/EASY83000
	$(call Image/Build/Template/Infineon/$(1),easy-83000)
endef

define Image/Build/Profile/EASY5120RT
	$(call Image/Build/Template/Infineon/$(1),easy-5120-rt)
endef

define Image/Build/Profile/EASY5120PATA
	$(call Image/Build/Template/Infineon/$(1),easy-5120p-ata)
endef

define Image/Build/Profile/PMUGW
	$(call Image/Build/Template/Infineon/$(1),powerline-mugw)
endef

define Image/Build/Profile/5GXI
	$(call Image/Build/Template/Osbridge/$(1),5gxi)
endef

define Image/Build/Profile/RouterBoard
	$(call Image/Build/Template/Mikrotik/$(1))
endef

ifeq ($(CONFIG_BROKEN),y)
  define Image/Build/Experimental
	# Cellvison
	$(call Image/Build/Profile/CAS630,$(1))
	$(call Image/Build/Profile/CAS630W,$(1))
	$(call Image/Build/Profile/CAS670,$(1))
	$(call Image/Build/Profile/CAS670W,$(1))
	$(call Image/Build/Profile/CAS700,$(1))
	$(call Image/Build/Profile/CAS700W,$(1))
	$(call Image/Build/Profile/CAS771,$(1))
	$(call Image/Build/Profile/CAS771W,$(1))
	$(call Image/Build/Profile/CAS861,$(1))
	$(call Image/Build/Profile/CAS861W,$(1))
	# Motorola
	$(call Image/Build/Profile/PMUGW,$(1))
	# OSBRiDGE
	$(call Image/Build/Profile/5GXI,$(1))
  endef
endif

define Image/Build/Profile/Generic
	# Cellvision
	$(call Image/Build/Profile/NFS101U,$(1))
	$(call Image/Build/Profile/NFS101WU,$(1))
	# Compex
	$(call Image/Build/Profile/WP54,$(1))
	$(call Image/Build/Profile/NP27G,$(1))
	$(call Image/Build/Profile/NP28G,$(1))
	# Edimax
	$(call Image/Build/Profile/BR6104K,$(1))
	$(call Image/Build/Profile/BR6104KP,$(1))
	$(call Image/Build/Profile/BR6104WG,$(1))
	$(call Image/Build/Profile/BR6114WG,$(1))
	$(call Image/Build/Profile/EB214A,$(1))
	# Infineon
	$(call Image/Build/Profile/EASY83000,$(1))
	$(call Image/Build/Profile/EASY5120RT,$(1))
	$(call Image/Build/Profile/EASY5120PATA,$(1))
	# Mikrotik
	$(call Image/Build/Profile/RB1xx/$(1))

	$(call Image/Build/Experimental,$(1))
endef
