rootfs_type=$(patsubst squashfs-%,squashfs,$(1))

# $(1): rootfs type.
# $(2): board name.
define imgname
$(BIN_DIR)/$(IMG_PREFIX)-$(2)-$(call rootfs_type,$(1))
endef

define rootfs_align
$(patsubst %-256k,0x40000,$(patsubst %-128k,0x20000,$(patsubst %-64k,0x10000,$(patsubst squashfs%,0x4,$(patsubst root.%,%,$(1))))))
endef

define sysupname
$(call imgname,$(1),$(2))-sysupgrade.bin
endef

define factoryname
$(call imgname,$(1),$(2))-factory.bin
endef

COMMA:=,

define mkcmdline
$(if $(1),board=$(1) )$(if $(2),console=$(2)$(COMMA)$(3))
endef

define mtdpartsize
$(shell sz=`echo '$(2)' | sed -ne 's/.*[:$(COMMA)]\([0-9]*\)k[@]*[0-9a-zx]*($(1)).*/\1/p'`; [ -n "$$sz" ] && echo $$(($$sz * 1024)))
endef

# $(1)      : name of image build method to be used, e.g., AthLzma.
# $(2)      : name of the build template to be used, e.g. 64k, 64kraw, 128k, etc.
# $(3)      : name of the profile to be defined.
# $(4)      : board name.
# $(5)~$(7) : arguments for $(mkcmdline)
#		board=$(1) console=$(2),$(3)
# $(8)~$(14): extra arguments.
define SingleProfile
  # $(1): action name, e.g. loader, buildkernel, squashfs, etc.
  define Image/Build/Profile/$(3)
	$$(call Image/Build/Template/$(2)/$$(1),$(1),$(4),$$(call mkcmdline,$(5),$(6),$(7)),$(8),$(9),$(10),$(11),$(12),$(13),$(14))
  endef
endef

LOADER_MAKE := $(NO_TRACE_MAKE) -C lzma-loader KDIR=$(KDIR)

VMLINUX:=$(BIN_DIR)/$(IMG_PREFIX)-vmlinux
UIMAGE:=$(BIN_DIR)/$(IMG_PREFIX)-uImage

# $(1): input file.
# $(2): output file.
# $(3): extra arguments for lzma.
define CompressLzma
  $(STAGING_DIR_HOST)/bin/lzma e $(1) -lc1 -lp2 -pb2 $(3) $(2)
endef

define PatchKernel
	cp $(KDIR)/vmlinux$(3) $(KDIR_TMP)/vmlinux$(3)-$(1)
	$(STAGING_DIR_HOST)/bin/patch-cmdline $(KDIR_TMP)/vmlinux$(3)-$(1) "$(strip $(2))"
endef

define PatchKernel/initramfs
	$(call PatchKernel,$(1),$(2),-initramfs)
	cp $(KDIR_TMP)/vmlinux-initramfs-$(1) $(call imgname,initramfs,$(1)).bin
endef

# $(1): board name.
# $(2): kernel command line.
# $(3): extra argumetns for lzma.
# $(4): name suffix, e.g. "-initramfs".
define PatchKernelLzma
	cp $(KDIR)/vmlinux$(4) $(KDIR_TMP)/vmlinux$(4)-$(1)
	$(STAGING_DIR_HOST)/bin/patch-cmdline $(KDIR_TMP)/vmlinux$(4)-$(1) "$(strip $(2))"
	$(call CompressLzma,$(KDIR_TMP)/vmlinux$(4)-$(1),$(KDIR_TMP)/vmlinux$(4)-$(1).bin.lzma,$(3))
endef

define PatchKernelGzip
	cp $(KDIR)/vmlinux$(3) $(KDIR_TMP)/vmlinux$(3)-$(1)
	$(STAGING_DIR_HOST)/bin/patch-cmdline $(KDIR_TMP)/vmlinux$(3)-$(1) "$(strip $(2))"
	gzip -9n -c $(KDIR_TMP)/vmlinux$(3)-$(1) > $(KDIR_TMP)/vmlinux$(3)-$(1).bin.gz
endef

ifneq ($(SUBTARGET),mikrotik)
# $(1): compression method of the data.
# $(2): extra arguments.
# $(3): input data file.
# $(4): output file.
define MkuImage
	mkimage -A mips -O linux -T kernel -a 0x80060000 -C $(1) $(2) \
		-e 0x80060000 -n 'MIPS $(VERSION_DIST) Linux-$(LINUX_VERSION)' \
		-d $(3) $(4)
endef

# $(1): board name.
# $(2): kernel command line.
# $(3): extra arguments for lzma.
# $(4): name suffix, e.g. "-initramfs".
# $(5): extra arguments for mkimage.
define MkuImageLzma
	$(call PatchKernelLzma,$(1),$(2),$(3),$(4))
	$(call MkuImage,lzma,$(5),$(KDIR_TMP)/vmlinux$(4)-$(1).bin.lzma,$(KDIR_TMP)/vmlinux$(4)-$(1).uImage)
endef

define MkuImageLzma/initramfs
	$(call PatchKernelLzma,$(1),$(2),$(3),-initramfs)
	$(call MkuImage,lzma,$(4),$(KDIR_TMP)/vmlinux-initramfs-$(1).bin.lzma,$(call imgname,initramfs,$(1))-uImage.bin)
endef

define MkuImageGzip
	$(call PatchKernelGzip,$(1),$(2))
	$(call MkuImage,gzip,,$(KDIR_TMP)/vmlinux-$(1).bin.gz,$(KDIR_TMP)/vmlinux-$(1).uImage)
endef

define MkuImageGzip/initramfs
	$(call PatchKernelGzip,$(1),$(2),-initramfs)
	$(call MkuImage,gzip,,$(KDIR_TMP)/vmlinux-initramfs-$(1).bin.gz,$(call imgname,initramfs,$(1))-uImage.bin)
endef

define MkuImageOKLI
	$(call MkuImage,lzma,-M 0x4f4b4c49,$(KDIR)/vmlinux.bin.lzma,$(KDIR_TMP)/vmlinux-$(1).okli)
endef
endif

# $(1): name of the 1st file.
# $(2): size limit of the 1st file if it is greater than 262144, or
#       the erase size of the flash if it is greater than zero and less
#       than 262144
# $(3): name of the 2nd file.
# $(4): size limit of the 2nd file if $(2) is greater than 262144, otherwise
#       it is the size limit of the output file
# $(5): name of the output file.
# $(6): padding size.
define CatFiles
	if [ $(2) -eq 0 ]; then \
		filename="$(3)"; fstype=$${filename##*\.}; \
		case "$${fstype}" in \
		*) bs=`stat -c%s $(1)`;; \
		esac; \
		( dd if=$(1) bs=$${bs} conv=sync;  cat $(3) ) > $(5); \
		if [ -n "$(6)" ]; then \
			case "$${fstype}" in \
			squashfs*) \
				padjffs2 $(5) $(6); \
				;; \
			esac; \
		fi; \
		if [ `stat -c%s $(5)` -gt $(4) ]; then \
			echo "Warning: $(5) is too big (> $(4) bytes)" >&2; \
			rm -f $(5); \
		fi; \
	else if [ $(2) -gt 262144 ]; then \
		if [ `stat -c%s "$(1)"` -gt $(2) ]; then \
			echo "Warning: $(1) is too big (> $(2) bytes)" >&2; \
		else if [ `stat -c%s $(3)` -gt $(4) ]; then \
			echo "Warning: $(3) is too big (> $(4) bytes)" >&2; \
		else \
			( dd if=$(1) bs=$(2) conv=sync; dd if=$(3) ) > $(5); \
		fi; fi; \
	else \
		( dd if=$(1) bs=$(2) conv=sync; dd if=$(3) ) > $(5); \
		if [ `stat -c%s $(5)` -gt $(4) ]; then \
			echo "Warning: $(5) is too big (> $(4) bytes)" >&2; \
			rm -f $(5); \
		fi; \
	fi; fi
endef

# $(1): rootfs type.
# $(2): board name.
# $(3): kernel image size limit.
# $(4): rootfs image size limit.
# $(5): padding argument for padjffs2.
Sysupgrade/KR=$(call CatFiles,$(2),$(3),$(KDIR)/root.$(1),$(4),$(call sysupname,$(1),$(5)))
Sysupgrade/KRuImage=$(call CatFiles,$(KDIR_TMP)/vmlinux-$(2).uImage,$(3),$(KDIR)/root.$(1),$(4),$(call sysupname,$(1),$(2)),$(5))
Sysupgrade/RKuImage=$(call CatFiles,$(KDIR)/root.$(1),$(4),$(KDIR_TMP)/vmlinux-$(2).uImage,$(3),$(call sysupname,$(1),$(2)))

# $(1): ubinize ini file
# $(2): working directory
# $(3): output file
# $(4): physical erase block size
# $(5): minimum I/O unit size
# $(6): custom options
define ubinize
	$(CP) $(1) $(2)
	( cd $(2); $(STAGING_DIR_HOST)/bin/ubinize -o $(3) -p $(4) -m $(5) $(6) $(1))
endef

#
# Embed lzma-compressed kernel inside lzma-loader.
#
# $(1), suffix of output filename, e.g. generic, lowercase board name, etc.
# $(2), suffix of target file to build, e.g. bin, gz, elf
# $(3), kernel command line to pass from lzma-loader to kernel
# $(4), unused here
# $(5), suffix of kernel filename, e.g. -initramfs, or empty
define Image/BuildLoader
	-rm -rf $(KDIR)/lzma-loader
	$(LOADER_MAKE) LOADER=loader-$(1).$(2) KERNEL_CMDLINE="$(3)"\
		LZMA_TEXT_START=0x80a00000 LOADADDR=0x80060000 \
		LOADER_DATA="$(KDIR)/vmlinux$(5).bin.lzma" BOARD="$(1)" \
		compile loader.$(2)
	-$(if $(5),$(CP) $(KDIR)/loader-$(1).$(2) $(KDIR)/loader-$(1)$(5).$(2))
endef

#
# Build lzma-loader alone which will search for lzma-compressed kernel identified by
# uImage header with magic "OKLI" at boot time.
#
# $(4), offset into the flash space to start searching uImage magic "OKLI".
# $(5), size of search range starting at $(4).  With 0 as the value, uImage
#	header is expected to be at precisely $(4)
define Image/BuildLoaderAlone
	-rm -rf $(KDIR)/lzma-loader
	$(LOADER_MAKE) LOADER=loader-$(1).$(2) KERNEL_CMDLINE="$(3)" \
		LZMA_TEXT_START=0x80a00000 LOADADDR=0x80060000 \
		BOARD="$(1)" FLASH_OFFS=$(4) FLASH_MAX=$(5) \
		compile loader.$(2)
endef

define Build/Clean
	$(LOADER_MAKE) clean
endef

alfa_ap120c_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,13312k(rootfs),1536k(kernel),1152k(unknown)ro,64k(art)ro;spi0.1:-(unknown)
alfa_ap96_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,256k(u-boot-env)ro,13312k(rootfs),2048k(kernel),512k(caldata)ro,15360k@0x80000(firmware)
alfa_mtdlayout_8M=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,6144k(rootfs),1600k(kernel),64k(nvram),64k(art)ro,7744k@0x50000(firmware)
alfa_mtdlayout_16M=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,15936k(firmware),64k(nvram),64k(art)ro
all0258n_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env),6272k(firmware),1536k(failsafe),64k(art)ro
all0315n_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,256k(u-boot-env),13568k(firmware),2048k(failsafe),256k(art)ro
ap96_mtdlayout=mtdparts=spi0.0:192k(u-boot)ro,64k(u-boot-env)ro,6144k(rootfs),1728k(kernel),64k(art)ro,7872k@0x40000(firmware)
ap121_mtdlayout_8M=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,6144k(rootfs),1600k(kernel),64k(nvram),64k(art)ro,7744k@0x50000(firmware)
ap121_mtdlayout_16M=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,10944k(rootfs),4992k(kernel),64k(nvram),64k(art)ro,15936k@0x50000(firmware)
ap132_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,1408k(kernel),6400k(rootfs),64k(art)ro,7808k@0x50000(firmware)
ap135_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,14528k(rootfs),1472k(kernel),64k(art)ro,16000k@0x50000(firmware)
ap136_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,6336k(rootfs),1408k(kernel),64k(mib0),64k(art)ro,7744k@0x50000(firmware)
ap143_mtdlayout_8M=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,6336k(rootfs),1472k(kernel),64k(art)ro,7744k@0x50000(firmware)
ap143_mtdlayout_16M=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,14528k(rootfs),1472k(kernel),64k(art)ro,16000k@0x50000(firmware)
ap147_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,14528k(rootfs),1472k(kernel),64k(art)ro,16000k@0x50000(firmware)
ap152_mtdlayout_16M=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,14528k(rootfs),1472k(kernel),64k(art)ro,16000k@0x50000(firmware)
bxu2000n2_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,1408k(kernel),8448k(rootfs),6016k(user),64k(cfg),64k(oem),64k(art)ro
cameo_ap81_mtdlayout=mtdparts=spi0.0:128k(u-boot)ro,64k(config)ro,3840k(firmware),64k(art)ro
cameo_ap91_mtdlayout=mtdparts=spi0.0:192k(u-boot)ro,64k(nvram)ro,3712k(firmware),64k(mac)ro,64k(art)ro
cameo_ap99_mtdlayout=mtdparts=spi0.0:192k(u-boot)ro,64k(nvram)ro,3520k(firmware),64k(mac)ro,192k(lp)ro,64k(art)ro
cameo_ap121_mtdlayout=mtdparts=spi0.0:64k(u-boot)ro,64k(art)ro,64k(mac)ro,64k(nvram)ro,192k(language)ro,3648k(firmware)
cameo_ap121_mtdlayout_8M=mtdparts=spi0.0:64k(u-boot)ro,64k(art)ro,64k(mac)ro,64k(nvram)ro,256k(language)ro,7680k@0x80000(firmware)
cameo_ap123_mtdlayout_4M=mtdparts=spi0.0:64k(u-boot)ro,64k(nvram)ro,3712k(firmware),192k(lang)ro,64k(art)ro
cameo_db120_mtdlayout=mtdparts=spi0.0:64k(uboot)ro,64k(nvram)ro,15936k(firmware),192k(lang)ro,64k(mac)ro,64k(art)ro
cameo_db120_mtdlayout_8M=mtdparts=spi0.0:64k(uboot)ro,64k(nvram)ro,7872k(firmware),128k(lang)ro,64k(art)ro
cap4200ag_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env),320k(custom)ro,1536k(kernel),12096k(rootfs),2048k(failsafe),64k(art)ro,13632k@0xa0000(firmware)
eap300v2_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env),320k(custom),13632k(firmware),2048k(failsafe),64k(art)ro
db120_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,6336k(rootfs),1408k(kernel),64k(nvram),64k(art)ro,7744k@0x50000(firmware)
dgl_5500_mtdlayout=mtdparts=spi0.0:192k(u-boot)ro,64k(nvram)ro,15296k(firmware),192k(lang)ro,512k(my-dlink)ro,64k(mac)ro,64k(art)ro
dlan_hotspot_mtdlayout=mtdparts=spi0.0:128k(u-boot)ro,64k(Config1)ro,64k(Config2)ro,7872k@0x40000(firmware),64k(art)ro
dlan_pro_500_wp_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,64k(Config1)ro,64k(Config2)ro,7680k@0x70000(firmware),64k(art)ro
dlan_pro_1200_ac_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,64k(Config1)ro,64k(Config2)ro,15872k@0x70000(firmware),64k(art)ro
cameo_ap94_mtdlayout=mtdparts=spi0.0:256k(uboot)ro,64k(config)ro,6208k(firmware),64k(caldata)ro,1600k(unknown)ro,64k@0x7f0000(caldata_copy)
cameo_ap94_mtdlayout_fat=mtdparts=spi0.0:256k(uboot)ro,64k(config)ro,7808k(firmware),64k(caldata)ro,64k@0x660000(caldata_orig),6208k@0x50000(firmware_orig)
esr900_mtdlayout=mtdparts=spi0.0:192k(u-boot)ro,64k(u-boot-env)ro,1408k(kernel),13248k(rootfs),1024k(manufacture)ro,64k(backup)ro,320k(storage)ro,64k(caldata)ro,14656k@0x40000(firmware)
esr1750_mtdlayout=mtdparts=spi0.0:192k(u-boot)ro,64k(u-boot-env)ro,1408k(kernel),13248k(rootfs),1024k(manufacture)ro,64k(backup)ro,320k(storage)ro,64k(caldata)ro,14656k@0x40000(firmware)
epg5000_mtdlayout=mtdparts=spi0.0:192k(u-boot)ro,64k(u-boot-env)ro,1408k(kernel),13248k(rootfs),1024k(manufacture)ro,64k(backup)ro,320k(storage)ro,64k(caldata)ro,14656k@0x40000(firmware)
f9k1115v2_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env),14464k(rootfs),1408k(kernel),64k(nvram)ro,64k(envram)ro,64k(art)ro,15872k@0x50000(firmware)
dlrtdev_mtdlayout=mtdparts=spi0.0:256k(uboot)ro,64k(config)ro,6208k(firmware),64k(caldata)ro,640k(certs),960k(unknown)ro,64k@0x7f0000(caldata_copy)
dlrtdev_mtdlayout_fat=mtdparts=spi0.0:256k(uboot)ro,64k(config)ro,7168k(firmware),640k(certs),64k(caldata)ro,64k@0x660000(caldata_orig),6208k@0x50000(firmware_orig)
planex_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,7744k(firmware),128k(art)ro
whrhpg300n_mtdlayout=mtdparts=spi0.0:248k(u-boot)ro,8k(u-boot-env)ro,3712k(firmware),64k(art)ro
wndap360_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,7744k(firmware),64k(nvram)ro,64k(art)ro
wnr2200_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,7808k(firmware),64k(art)ro
wnr2000_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,3712k(firmware),64k(art)ro
wnr2000v3_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,3712k(firmware),64k(art)ro
wnr2000v4_mtdlayout=mtdparts=spi0.0:192k(u-boot)ro,64k(u-boot-env)ro,3776k(firmware),64k(art)ro
r6100_mtdlayout=mtdparts=ar934x-nfc:128k(u-boot)ro,256k(caldata)ro,256k(caldata-backup),512k(config),512k(pot),2048k(kernel),122240k(ubi),25600k@0x1a0000(firmware),2048k(language),3072k(traffic_meter)
sgrw500n85bv2_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,14528k(rootfs),1472k(kernel),64k(art)ro,16000k@0x50000(firmware)
tew823dru_mtdlayout=mtdparts=spi0.0:192k(u-boot)ro,64k(nvram)ro,15296k(firmware),192k(lang)ro,512k(my-dlink)ro,64k(mac)ro,64k(art)ro
wndr4300_mtdlayout=mtdparts=ar934x-nfc:256k(u-boot)ro,256k(u-boot-env)ro,256k(caldata)ro,512k(pot),2048k(language),512k(config),3072k(traffic_meter),2048k(kernel),121856k(ubi),123904k@0x6c0000(firmware),256k(caldata_backup),-(reserved)
zcn1523h_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,6208k(rootfs),1472k(kernel),64k(configure)ro,64k(mfg)ro,64k(art)ro,7680k@0x50000(firmware)
mynet_rext_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,7808k(firmware),64k(nvram)ro,64k(ART)ro
zyx_nbg6716_mtdlayout=mtdparts=spi0.0:256k(u-boot)ro,64k(env)ro,64k(RFdata)ro,-(nbu);ar934x-nfc:2048k(zyxel_rfsd),2048k(romd),1024k(header),2048k(kernel),-(ubi)

define Image/BuildKernel
	cp $(KDIR)/vmlinux.elf $(VMLINUX).elf
	cp $(KDIR)/vmlinux $(VMLINUX).bin
	dd if=$(KDIR)/vmlinux.bin.lzma of=$(VMLINUX).lzma bs=65536 conv=sync
	$(call MkuImage,lzma,,$(KDIR)/vmlinux.bin.lzma,$(UIMAGE)-lzma.bin)
	cp $(KDIR)/loader-generic.elf $(VMLINUX)-lzma.elf
	-mkdir -p $(KDIR_TMP)
endef

define Image/BuildKernel/Initramfs
	cp $(KDIR)/vmlinux-initramfs.elf $(VMLINUX)-initramfs.elf
	cp $(KDIR)/vmlinux-initramfs $(VMLINUX)-initramfs.bin
	dd if=$(KDIR)/vmlinux-initramfs.bin.lzma of=$(VMLINUX)-initramfs.lzma bs=65536 conv=sync
	$(call MkuImage,lzma,,$(KDIR)/vmlinux-initramfs.bin.lzma,$(UIMAGE)-initramfs-lzma.bin)
	cp $(KDIR)/loader-generic-initramfs.elf $(VMLINUX)-initramfs-lzma.elf
	$(call Image/Build/Initramfs)
endef

Image/Build/WRT400N/buildkernel=$(call MkuImageLzma,$(2),$(3))

define Image/Build/WRT400N
	$(call Sysupgrade/KRuImage,$(1),$(2),1310720,6488064)
	if [ -e "$(call sysupname,$(1),$(2))" ]; then \
		wrt400n $(KDIR_TMP)/vmlinux-$(2).uImage $(KDIR)/root.$(1) $(call factoryname,$(1),$(2)); \
	fi
endef


define Image/Build/CameoAP94/buildkernel
	$(call MkuImageLzma,$(2),$(3) $(4))
	$(call MkuImageLzma,$(2)-fat,$(3) $(5))
endef

define Image/Build/CameoAP94
	$(eval fwsize=$(call mtdpartsize,firmware,$(4)))
	$(eval fwsize_fat=$(call mtdpartsize,firmware,$(5)))
	$(call Sysupgrade/KRuImage,$(1),$(2),0,$$(($(fwsize)-4*64*1024)),64)
	if [ -e "$(call sysupname,$(1),$(2))" ]; then \
		( \
			dd if=$(call sysupname,$(1),$(2)); \
			echo -n "$(6)"; \
		) > $(call imgname,$(1),$(2))-backup-loader.bin; \
		if [ `stat -c%s $(call sysupname,$(1),$(2))` -gt 4194304 ]; then \
			echo "Warning: $(call sysupname,$(1),$(2)) is too big" >&2; \
		else \
			( \
				dd if=$(call sysupname,$(1),$(2)) bs=4096k conv=sync; \
				echo -n "$(7)"; \
			) > $(call factoryname,$(1),$(2)); \
		fi; \
	fi
	$(call CatFiles,$(KDIR_TMP)/vmlinux-$(2)-fat.uImage,0,$(KDIR)/root.$(1),$$(($(fwsize_fat)-4*64*1024)),$(KDIR_TMP)/$(2)-fat.bin,64)
	if [ -e "$(KDIR_TMP)/$(2)-fat.bin" ]; then \
		echo -n "" > $(KDIR_TMP)/$(2)-fat.dummy; \
		sh $(TOPDIR)/scripts/combined-image.sh \
			"$(KDIR_TMP)/$(2)-fat.bin" \
			"$(KDIR_TMP)/$(2)-fat.dummy" \
			$(call sysupname,$(1),$(2)-fat); \
	fi
endef

define Image/Build/WZRHP
	$(call Sysupgrade/KRuImage,$(1),$(2),0,$$(($(3)-4*$(4)*1024)),$(4))
	if [ -e "$(call sysupname,$(1),$(2))" ]; then \
		( \
			echo -n -e "# Airstation Public Fmt1\x00\x00\x00\x00\x00\x00\x00\x00"; \
			dd if=$(call sysupname,$(1),$(2)); \
		) > $(call imgname,$(1),$(2))-tftp.bin; \
		buffalo-enc -p $(5) -v 1.99 \
			-i $(call sysupname,$(1),$(2)) \
			-o $(KDIR_TMP)/$(2).enc; \
		buffalo-tag -b $(5) -p $(5) -a ath -v 1.99 -m 1.01 -l mlang8 \
			-w 3 -c 0x80041000 -d 0x801e8000 -f 1 -r M_ \
			-i $(KDIR_TMP)/$(2).enc \
			-o $(call factoryname,$(1),$(2)); \
	fi
endef

Image/Build/WZRHP64K/buildkernel=$(call MkuImageLzma,$(2),$(3))
Image/Build/WZRHP64K/initramfs=$(call MkuImageLzma/initramfs,$(2),$(3) $(4))
Image/Build/WZRHP64K=$(call Image/Build/WZRHP,$(1),$(2),33095680,64,$(4))

Image/Build/WZRHP128K/buildkernel=$(call MkuImageLzma,$(2),$(3))
Image/Build/WZRHP128K/initramfs=$(call MkuImageLzma/initramfs,$(2),$(3) $(4))
Image/Build/WZRHP128K=$(call Image/Build/WZRHP,$(1),$(2),33030144,128,$(4))


Image/Build/WHRHPG300N/buildkernel=$(call MkuImageLzma,$(2),$(3) $(4))
Image/Build/WHRHPG300N/initramfs=$(call MkuImageLzma/initramfs,$(2),$(3) $(4))

define Image/Build/WHRHPG300N
	$(eval fwsize=$(call mtdpartsize,firmware,$(4)))
	$(call Sysupgrade/KRuImage,$(1),$(2),0,$$(($(fwsize)-4*64*1024)),64)
	if [ -e "$(call sysupname,$(1),$(2))" ]; then \
		( \
			echo -n -e "# Airstation Public Fmt1\x00\x00\x00\x00\x00\x00\x00\x00"; \
			dd if=$(call sysupname,$(1),$(2)); \
		) > $(call imgname,$(1),$(2))-tftp.bin; \
		buffalo-enc -p $(5) -v 1.99 \
			-i $(call sysupname,$(1),$(2)) \
			-o $(KDIR_TMP)/$(2).enc; \
		buffalo-tag -b $(5) -p $(5) -a ath -v 1.99 -m 1.01 -l mlang8 \
			-w 3 -c 0x80041000 -d 0x801e8000 -f 1 -r M_ \
			-i $(KDIR_TMP)/$(2).enc \
			-o $(call factoryname,$(1),$(2)); \
	fi
endef


define Image/Build/Cameo
	$(eval fwsize=$(call mtdpartsize,firmware,$(4)))
	$(call Sysupgrade/KRuImage,$(1),$(2),0,$$(($(fwsize)-4*64*1024)),64)
	if [ -e "$(call sysupname,$(1),$(2))" ]; then \
		factory_size=$$(($(fwsize) - $(6))); \
		( \
			dd if=$(call sysupname,$(1),$(2)) bs=$${factory_size} conv=sync; \
			echo -n $(5); \
		) > $(call factoryname,$(1),$(2)); \
	fi
endef

Image/Build/CameoAP81/buildkernel=$(call MkuImageLzma,$(2),$(3) $(cameo_ap81_mtdlayout))
Image/Build/CameoAP81=$(call Image/Build/Cameo,$(1),$(2),$(3),$(cameo_ap81_mtdlayout),$(4),65536)
Image/Build/CameoAP81/initramfs=$(call MkuImageLzma/initramfs,$(2),$(3) $(cameo_ap81_mtdlayout))

Image/Build/CameoAP91/buildkernel=$(call MkuImageLzma,$(2),$(3) $(cameo_ap91_mtdlayout))
Image/Build/CameoAP91=$(call Image/Build/Cameo,$(1),$(2),$(3),$(cameo_ap91_mtdlayout),$(4),65536)
Image/Build/CameoAP91/initramfs=$(call MkuImageLzma/initramfs,$(2),$(3) $(cameo_ap91_mtdlayout))

Image/Build/CameoAP99/buildkernel=$(call MkuImageLzma,$(2),$(3) $(cameo_ap99_mtdlayout))
Image/Build/CameoAP99=$(call Image/Build/Cameo,$(1),$(2),$(3),$(cameo_ap99_mtdlayout),$(4),65536)
Image/Build/CameoAP99/initramfs=$(call MkuImageLzma/initramfs,$(2),$(3) $(cameo_ap99_mtdlayout))

Image/Build/CameoAP123_4M/buildkernel=$(call MkuImageLzma,$(2),$(3) $(cameo_ap123_mtdlayout_4M))
Image/Build/CameoAP123_4M=$(call Image/Build/Cameo,$(1),$(2),$(3),$(cameo_ap123_mtdlayout_4M),$(4),26)
Image/Build/CameoAP123_4M/initramfs=$(call MkuImageLzma/initramfs,$(2),$(3) $(cameo_ap123_mtdlayout_4M))

Image/Build/CameoAP135/buildkernel=$(call MkuImageLzma,$(2),$(3) $(4))
Image/Build/CameoAP135=$(call Image/Build/Cameo,$(1),$(2),$(3),$(4),$(5),26)
Image/Build/CameoAP135/initramfs=$(call MkuImageLzma/initramfs,$(2),$(3) $(4))

Image/Build/CameoDB120/buildkernel=$(call MkuImageLzma,$(2),$(3) $(cameo_db120_mtdlayout))
Image/Build/CameoDB120=$(call Image/Build/Cameo,$(1),$(2),$(3),$(cameo_db120_mtdlayout),$(4),26)
Image/Build/CameoDB120/initramfs=$(call MkuImageLzma/initramfs,$(2),$(3) $(cameo_db120_mtdlayout))

Image/Build/CameoDB120_8M/buildkernel=$(call MkuImageLzma,$(2),$(3) $(cameo_db120_mtdlayout_8M))
Image/Build/CameoDB120_8M=$(call Image/Build/Cameo,$(1),$(2),$(3),$(cameo_db120_mtdlayout_8M),$(4),26)
Image/Build/CameoDB120_8M/initramfs=$(call MkuImageLzma/initramfs,$(2),$(3) $(cameo_db120_mtdlayout_8M))

define Image/Build/CameoHornet
	$(eval fwsize=$(call mtdpartsize,firmware,$(4)))
	$(call Sysupgrade/KRuImage,$(1),$(2),0,$$(($(fwsize)-4*64*1024)),64)
	if [ -e "$(call sysupname,$(1),$(2))" ]; then \
		for r in $(7); do \
			[ -n "$$r" ] && dashr="-$$r" || dashr=; \
			[ -z "$$r" ] && r="DEF"; \
			mkcameofw -M HORNET -R "$$r" -S $(5) -V $(6) -c \
				-K $(8) -I $(fwsize) \
				-k "$(call sysupname,$(1),$(2))" \
				-o $(call imgname,$(1),$(2))-factory$$dashr.bin; \
			true; \
		done; \
	fi
endef

Image/Build/CameoAP121/buildkernel=$(call MkuImageLzma,$(2),$(3) $(cameo_ap121_mtdlayout))
Image/Build/CameoAP121=$(call Image/Build/CameoHornet,$(1),$(2),$(3),$(cameo_ap121_mtdlayout),$(4),$(5),$(6),0xe0000)
Image/Build/CameoAP121/initramfs=$(call MkuImageLzma/initramfs,$(2),$(3) $(cameo_ap121_mtdlayout))

Image/Build/CameoAP121_8M/buildkernel=$(call MkuImageLzma,$(2),$(3) $(cameo_ap121_mtdlayout_8M))
Image/Build/CameoAP121_8M=$(call Image/Build/CameoHornet,$(1),$(2),$(3),$(cameo_ap121_mtdlayout_8M),$(4),$(5),$(6),0x100000)
Image/Build/CameoAP121_8M/initramfs=$(call MkuImageLzma/initramfs,$(2),$(3) $(cameo_ap121_mtdlayout_8M))

define Image/Build/dLAN
	$(eval fwsize=$(call mtdpartsize,firmware,$(4)))
	$(eval rootsize=$(call mtdpartsize,rootfs,$(4)))
	$(eval kernsize=$(call mtdpartsize,kernel,$(4)))
	$(call Sysupgrade/$(5),$(1),$(2),$(if $(6),$(6),$(kernsize)),$(if $(rootsize),$(rootsize),$(fwsize)))
endef

Image/Build/dLANLzma/buildkernel=$(call MkuImageLzma,$(2),$(3) $(4))
Image/Build/dLANLzma=$(call Image/Build/dLAN,$(1),$(2),$(3),$(4),$(5),$(6),$(7))
Image/Build/dLANLzma/initramfs=$(call MkuImageLzma/initramfs,$(2),$(3) $(4))

define Image/Build/Ath
	$(eval fwsize=$(call mtdpartsize,firmware,$(4)))
	$(eval rootsize=$(call mtdpartsize,rootfs,$(4)))
	$(eval kernsize=$(call mtdpartsize,kernel,$(4)))
	$(call Sysupgrade/$(5),$(1),$(2),$(if $(6),$(6),$(kernsize)),$(if $(rootsize),$(rootsize),$(fwsize)))
endef

Image/Build/AthGzip/buildkernel=$(call MkuImageGzip,$(2),$(3) $(4))
Image/Build/AthGzip=$(call Image/Build/Ath,$(1),$(2),$(3),$(4),$(5),$(6),$(7))
Image/Build/AthGzip/initramfs=$(call MkuImageGzip/initramfs,$(2),$(3) $(4))

Image/Build/AthLzma/buildkernel=$(call MkuImageLzma,$(2),$(3) $(4))
Image/Build/AthLzma=$(call Image/Build/Ath,$(1),$(2),$(3),$(4),$(5),$(6),$(7))
Image/Build/AthLzma/initramfs=$(call MkuImageLzma/initramfs,$(2),$(3) $(4))


Image/Build/Belkin/buildkernel=$(call MkuImageLzma,$(2),$(3) $(4))
Image/Build/Belkin/initramfs=$(call MkuImageLzma/initramfs,$(2),$(3) $(4))

define Image/Build/Belkin
	$(eval fwsize=$(call mtdpartsize,firmware,$(4)))
	$(eval kernsize=$(call mtdpartsize,kernel,$(4)))
	$(eval rootsize=$(call mtdpartsize,rootfs,$(4)))
	$(call Sysupgrade/RKuImage,$(1),$(2),$(kernsize),$(rootsize))
	if [ -e "$(call sysupname,$(1),$(2))" ]; then \
		edimax_fw_header -m $(5) -v "$(shell echo -n $(VERSION_DIST)$(REVISION) | cut -c -13)" \
			-n "uImage" \
			-i $(KDIR_TMP)/vmlinux-$(2).uImage \
			-o $(KDIR_TMP)/$(2)-uImage; \
		edimax_fw_header -m $(5) -v "$(shell echo -n $(VERSION_DIST)$(REVISION) | cut -c -13)" \
			-n "rootfs" \
			-i $(KDIR)/root.$(1) \
			-o $(KDIR_TMP)/$(2)-rootfs; \
		( \
			dd if=$(KDIR_TMP)/$(2)-rootfs; \
			dd if=$(KDIR_TMP)/$(2)-uImage; \
		) > "$(call factoryname,$(1),$(2))"; \
	fi
endef

define Image/Build/EnGenius
	$(eval fwsize=$(call mtdpartsize,firmware,$(4)))
	$(eval rootsize=$(call mtdpartsize,rootfs,$(4)))
	$(eval kernsize=$(call mtdpartsize,kernel,$(4)))
	$(call Sysupgrade/$(5),$(1),$(2),$(if $(6),$(6),$(kernsize)),$(if $(rootsize),$(rootsize),$(fwsize)))
	if [ -e "$(call sysupname,$(1),$(2))" ]; then \
		mksenaofw -e $(call sysupname,$(1),$(2)) \
			-o $(call imgname,$(1),$(2))-factory.dlf \
			-r 0x101 -p $(7) -t 2; \
	fi
endef

Image/Build/EnGenius/buildkernel=$(call MkuImageLzma,$(2),$(3) $(4))
Image/Build/EnGenius/initramfs=$(call MkuImageLzma/initramfs,$(2),$(3) $(4))


Image/Build/PB4X/buildkernel=$(call PatchKernelLzma,$(2),$(3))

define Image/Build/PB4X
	dd if=$(KDIR_TMP)/vmlinux-$(2).bin.lzma \
	   of=$(call imgname,kernel,$(2)).bin bs=64k conv=sync
	dd if=$(KDIR)/root.$(1) \
	   of=$(call imgname,$(1),$(2)-rootfs).bin bs=128k conv=sync
	-sh $(TOPDIR)/scripts/combined-image.sh \
		"$(call imgname,kernel,$(2)).bin" \
		"$(call imgname,$(1),$(2)-rootfs).bin" \
		$(call sysupname,$(1),$(2))
endef


Image/Build/MyLoader/buildkernel=$(call PatchKernelLzma,$(2),$(3))
Image/Build/MyLoader/initramfs=$(call PatchKernel/initramfs,$(2),$(3))

define Image/Build/MyLoader
	$(eval fwsize=$(shell echo $$(($(4)-0x30000-4*64*1024))))
	$(eval fwimage=$(KDIR_TMP)/$(2)-$(5)-firmware.bin)
	$(call CatFiles,$(KDIR_TMP)/vmlinux-$(2).bin.lzma,65536,$(KDIR)/root.$(1),$(fwsize),$(fwimage))
	if [ -e "$(fwimage)" ]; then \
		$(STAGING_DIR_HOST)/bin/mkmylofw -B $(2) -s $(4) -v \
			-p0x00030000:0:al:0x80060000:firmware:$(fwimage) \
			$(call imgname,$(1),$(2))-$(5)-factory.img; \
		echo -n "" > $(KDIR_TMP)/empty.bin; \
		sh $(TOPDIR)/scripts/combined-image.sh \
			$(fwimage) $(KDIR_TMP)/empty.bin \
			$(call imgname,$(1),$(2))-$(5)-sysupgrade.bin; \
	fi
endef

Image/Build/Planex/initramfs=$(call MkuImageGzip/initramfs,$(2),$(3) $(planex_mtdlayout))
Image/Build/Planex/loader=$(call Image/BuildLoaderAlone,$(1),gz,$(2) $(planex_mtdlayout),0x52000,0)

define Image/Build/Planex/buildkernel
	[ -e "$(KDIR)/loader-$(2).gz" ]
	$(call MkuImageOKLI,$(2))
	( \
		dd if=$(KDIR)/loader-$(2).gz bs=8128 count=1 conv=sync; \
		dd if=$(KDIR_TMP)/vmlinux-$(2).okli; \
	) > $(KDIR_TMP)/kernel-$(2).bin
	$(call MkuImage,gzip,,$(KDIR_TMP)/kernel-$(2).bin,$(KDIR_TMP)/vmlinux-$(2).uImage)
endef

define Image/Build/Planex
	$(eval fwsize=$(call mtdpartsize,firmware,$(planex_mtdlayout)))
	$(call Sysupgrade/KRuImage,$(1),$(2),0,$$(($(fwsize)-4*64*1024)),64)
	if [ -e "$(call sysupname,$(1),$(2))" ]; then \
		$(STAGING_DIR_HOST)/bin/mkplanexfw \
			-B $(2) \
			-v 2.00.00 \
			-i $(call sysupname,$(1),$(2)) \
			-o $(call factoryname,$(1),$(2)); \
	fi
endef


Image/Build/ALFA/buildkernel=$(call MkuImageLzma,$(2),$(3) $(4))
Image/Build/ALFA/initramfs=$(call MkuImageLzma/initramfs,$(2),$(3) $(4))

define Image/Build/ALFA
	$(call Sysupgrade/RKuImage,$(1),$(2),$(5),$(6))
	if [ -e "$(call sysupname,$(1),$(2))" ]; then \
		rm -rf $(KDIR)/$(1); \
		mkdir -p $(KDIR)/$(1); \
		cd $(KDIR)/$(1); \
		cp $(KDIR_TMP)/vmlinux-$(2).uImage $(KDIR)/$(1)/$(7); \
		cp $(KDIR)/root.$(1) $(KDIR)/$(1)/$(8); \
		$(TAR) c \
			$(if $(SOURCE_DATE_EPOCH),--mtime="@$(SOURCE_DATE_EPOCH)") \
			-C $(KDIR)/$(1) $(7) $(8) \
				| gzip -9nc > $(call factoryname,$(1),$(2)); \
		( \
			echo WRM7222C | dd bs=32 count=1 conv=sync; \
			echo -ne '\xfe'; \
		) >> $(call factoryname,$(1),$(2)); \
	fi
endef


Image/Build/Senao/buildkernel=$(call MkuImageLzma,$(2),$(3) $(4))
Image/Build/Senao/initramfs=$(call MkuImageLzma/initramfs,$(2),$(3) $(4))

define Image/Build/Senao
	mkdir -p $(KDIR_TMP)/$(2)/
	touch $(KDIR_TMP)/$(2)/FWINFO-OpenWrt-$(REVISION)-$(2)
	-$(CP) ./$(2)/* $(KDIR_TMP)/$(2)/
	dd if=$(KDIR_TMP)/vmlinux-$(2).uImage \
		of=$(KDIR_TMP)/$(2)/openwrt-senao-$(2)-uImage-lzma.bin bs=64k conv=sync
	dd if=$(KDIR)/root.$(1) \
		of=$(KDIR_TMP)/$(2)/openwrt-senao-$(2)-root.$(1) bs=64k conv=sync
	( \
		cd $(KDIR_TMP)/$(2)/;  \
		$(TAR) -c \
			$(if $(SOURCE_DATE_EPOCH),--mtime="@$(SOURCE_DATE_EPOCH)") \
			* | gzip -9nc > $(call factoryname,$(1),$(2)) \
	)
	-rm -rf $(KDIR_TMP)/$(2)/
	-sh $(TOPDIR)/scripts/combined-image.sh \
		$(KDIR_TMP)/vmlinux-$(2).uImage \
		$(KDIR)/root.$(1) \
		$(call sysupname,$(1),$(2))
endef

define Image/Build/CyberTAN
	echo -n '' > $(KDIR_TMP)/empty.bin
	-$(STAGING_DIR_HOST)/bin/trx -o $(KDIR)/image.tmp \
		-f $(KDIR_TMP)/vmlinux-$(2).uImage -F $(KDIR_TMP)/empty.bin \
		-x 32 -a 0x10000 -x -32 -f $(KDIR)/root.$(1) && \
	$(STAGING_DIR_HOST)/bin/addpattern -B $(2) -v v$(5) \
		-i $(KDIR)/image.tmp \
		-o $(call sysupname,$(1),$(2))
	-$(STAGING_DIR_HOST)/bin/trx -o $(KDIR)/image.tmp -f $(KDIR_TMP)/vmlinux-$(2).uImage \
		-x 32 -a 0x10000 -x -32 -f $(KDIR)/root.$(1) && \
	$(STAGING_DIR_HOST)/bin/addpattern -B $(2) -v v$(5) -g \
		-i $(KDIR)/image.tmp \
		-o $(call factoryname,$(1),$(2))
	-rm $(KDIR)/image.tmp
endef

Image/Build/CyberTANGZIP/loader=$(call Image/BuildLoader,$(1),gz,$(2),0x80060000)
Image/Build/CyberTANGZIP/buildkernel=$(call MkuImage,gzip,,$(KDIR)/loader-$(2).gz,$(KDIR_TMP)/vmlinux-$(2).uImage)
Image/Build/CyberTANGZIP=$(call Image/Build/CyberTAN,$(1),$(2),$(3),$(4),$(5))

Image/Build/CyberTANLZMA/buildkernel=$(call MkuImageLzma,$(2),$(3) $(4))
Image/Build/CyberTANLZMA=$(call Image/Build/CyberTAN,$(1),$(2),$(3),$(4),$(5))


Image/Build/Netgear/initramfs=$(call MkuImageLzma/initramfs,$(2),$(3) $(4),,-M $(5))

define Image/Build/Netgear/buildkernel
	$(call MkuImageLzma,$(2),$(3) $(4),-d20,,-M $(5))
	-rm -rf $(KDIR_TMP)/$(2)
	mkdir -p $(KDIR_TMP)/$(2)/image
	cat $(KDIR_TMP)/vmlinux-$(2).uImage > $(KDIR_TMP)/$(2)/image/uImage
	$(STAGING_DIR_HOST)/bin/mksquashfs-lzma \
		$(KDIR_TMP)/$(2) $(KDIR_TMP)/vmlinux-$(2).uImage.squashfs.tmp1 \
		-noappend -root-owned -be -b 65536 \
		$(if $(SOURCE_DATE_EPOCH),-fixed-time $(SOURCE_DATE_EPOCH))
	( \
		cat $(KDIR_TMP)/vmlinux-$(2).uImage.squashfs.tmp1; \
		dd if=/dev/zero bs=1k count=1 \
	) > $(KDIR_TMP)/vmlinux-$(2).uImage.squashfs.tmp2
	mkimage -A mips -O linux -T filesystem -C none -M $(5) \
		-a 0xbf070000 -e 0xbf070000 \
		-n 'MIPS $(VERSION_DIST) Linux-$(LINUX_VERSION)' \
		-d $(KDIR_TMP)/vmlinux-$(2).uImage.squashfs.tmp2 \
		$(KDIR_TMP)/vmlinux-$(2).uImage.squashfs
endef

define Image/Build/Netgear
	$(eval fwsize=$(call mtdpartsize,firmware,$(4)))
	$(call CatFiles,$(KDIR_TMP)/vmlinux-$(2).uImage.squashfs,0,$(KDIR)/root.$(1),$(fwsize),$(call sysupname,$(1),$(2)),64)
	if [ -e $(call sysupname,$(1),$(2)) ]; then \
		for r in $(7) ; do \
			[ -n "$$r" ] && dashr="-$$r" || dashr= ; \
			$(STAGING_DIR_HOST)/bin/mkdniimg \
				-B $(6) -v $(VERSION_DIST).$(REVISION) -r "$$r" $(8) \
				-i $(call sysupname,$(1),$(2)) \
				-o $(call imgname,$(1),$(2))-factory$$dashr.img; \
		done; \
	fi
	if [ "$2" = "wnr2000" ]; then \
		dd if=$(KDIR)/root.$(1) \
			of=$(call imgname,$(1),$(2)-rootfs).bin bs=128k conv=sync; \
	fi
endef


Image/Build/NetgearLzma/initramfs=$(call MkuImageLzma/initramfs,$(2),$(3) $(4),,-M $(5))
Image/Build/NetgearLzma/buildkernel=$(call MkuImageLzma,$(2),$(3) $(4),-d20,,-M $(5))

define Image/Build/NetgearLzma
	$(eval fwsize=$(call mtdpartsize,firmware,$(4)))
	$(call CatFiles,$(KDIR_TMP)/vmlinux-$(2).uImage,0,$(KDIR)/root.$(1),$(fwsize),$(call sysupname,$(1),$(2)),64)
endef


Image/Build/NetgearNAND/initramfs=$(call MkuImageLzma/initramfs,$(2),$(3) $(4),,-M $(5))

# $(1): (empty)
# $(2): Board name (small caps)
# $(3): Kernel board specific cmdline
# $(4): Kernel mtdparts definition
# $(5): U-Boot magic
define Image/Build/NetgearNAND/buildkernel
	$(eval kernelsize=$(call mtdpartsize,kernel,$(4)))
	$(call PatchKernelLzma,$(2),$(3) $(4),-d20)
	dd if=$(KDIR_TMP)/vmlinux-$(2).bin.lzma \
		of=$(KDIR_TMP)/vmlinux-$(2).bin.tmp \
		bs=$$(($(kernelsize)-131072-2*64-1)) \
		count=1 conv=sync
	$(call MkuImage,lzma,-M $(5),$(KDIR_TMP)/vmlinux-$(2).bin.tmp,$(KDIR_TMP)/vmlinux-$(2).uImage)
	echo -ne '\xff' >> $(KDIR_TMP)/vmlinux-$(2).uImage
	# create a fake rootfs image
	dd if=/dev/zero of=$(KDIR_TMP)/fakeroot-$(2) bs=131072 count=1
	mkimage -A mips -O linux -T filesystem -C none \
		-a 0xbf070000 -e 0xbf070000 \
		-n 'MIPS $(VERSION_DIST) fakeroot' \
		-d $(KDIR_TMP)/fakeroot-$(2) \
		-M $(5) \
		$(KDIR_TMP)/fakeroot-$(2).uImage
	# append the fake rootfs image to the kernel, it will reside in the last
	# erase block of the kernel partition
	cat $(KDIR_TMP)/fakeroot-$(2).uImage >> $(KDIR_TMP)/vmlinux-$(2).uImage
endef


# $(1): rootfs image suffix
# $(2): Board name (small caps)
# $(3): Kernel board specific cmdline
# $(4): Kernel mtdparts definition
# $(5): U-Boot magic
# $(6): Board name (upper caps)
# $(7): firmware region code (not used yet)
# $(8): DNI Hardware version
# $(9): suffix of the configuration file for ubinize
define Image/Build/NetgearNAND
	$(eval firmwaresize=$(call mtdpartsize,firmware,$(4)))
	$(eval kernelsize=$(call mtdpartsize,kernel,$(4)))
	$(eval imageraw=$(KDIR_TMP)/$(2)-raw.img)
	$(CP) $(KDIR)/root.squashfs-raw $(KDIR_TMP)/root.squashfs
	echo -ne '\xde\xad\xc0\xde' > $(KDIR_TMP)/jffs2.eof
	$(call ubinize,ubinize-$(9).ini,$(KDIR_TMP),$(KDIR_TMP)/$(2)-root.ubi,128KiB,2048,-E 5)
	( \
		dd if=$(KDIR_TMP)/vmlinux-$(2).uImage; \
		dd if=$(KDIR_TMP)/$(2)-root.ubi \
	) > $(imageraw)
	$(STAGING_DIR_HOST)/bin/mkdniimg \
		-B $(6) -v $(VERSION_DIST).$(REVISION) -r "$$r" $(8) \
		-i $(imageraw) \
		-o $(call imgname,ubi,$(2))-factory.img

	$(call Image/Build/SysupgradeNAND,$(2),squashfs,$(KDIR_TMP)/vmlinux-$(2).uImage)
endef

ZYXEL_UBOOT = $(KDIR)/u-boot-nbg460n_550n_550nh.bin
ZYXEL_UBOOT_BIN = $(wildcard $(BIN_DIR)/u-boot-nbg460n_550n_550nh/u-boot.bin)

Image/Build/ZyXEL/buildkernel=$(call MkuImageLzma,$(2),$(3))

define Image/Build/ZyXEL
	$(call Sysupgrade/KRuImage,$(1),$(2),917504,2752512)
	if [ -e "$(call sysupname,$(1),$(2))" ]; then \
		if [ ! -f $(ZYXEL_UBOOT) ]; then \
			echo "Warning: $(ZYXEL_UBOOT) not found" >&2; \
		else \
			$(STAGING_DIR_HOST)/bin/mkzynfw \
				-B $(4) \
				-b $(ZYXEL_UBOOT) \
				-r $(call sysupname,$(1),$(2)):0x10000 \
				-o $(call factoryname,$(1),$(2)); \
	fi; fi
endef

define	Image/Build/ZyXELNAND/buildkernel
	$(eval kernelsize=$(call mtdpartsize,kernel,$(5)))
	$(call MkuImageLzma,$(2),$(3) $(5) $(6))
	mkdir -p $(KDIR_TMP)/$(2)/image/boot
	cp $(KDIR_TMP)/vmlinux-$(2).uImage $(KDIR_TMP)/$(2)/image/boot/vmlinux.lzma.uImage
	$(STAGING_DIR_HOST)/bin/mkfs.jffs2 \
		--pad=$(kernelsize) --big-endian --squash-uids -v -e 128KiB \
		-o $(KDIR_TMP)/$(2)-kernel.jffs2 \
		-d $(KDIR_TMP)/$(2)/image \
		2>&1 1>/dev/null | awk '/^.+$$/'
	-rm -rf $(KDIR_TMP)/$(2)
endef

define Image/Build/ZyXELNAND
	if [ "$(1)" != "squashfs" ]; then \
		echo Only squashfs is supported; \
		return 0; \
	fi
	$(eval firmwaresize=$(call mtdpartsize,firmware,$(4)))
	$(eval kernelsize=$(call mtdpartsize,kernel,$(4)))
	$(eval imageraw=$(KDIR_TMP)/$(2)-raw.img)
	$(CP) $(KDIR)/root.$(1) $(KDIR_TMP)/ubi_root.img
	$(call ubinize,ubinize-$(2).ini,$(KDIR_TMP),$(KDIR_TMP)/$(2)-root.ubi,128KiB,2048,-E 5)
	( \
		dd if=$(KDIR_TMP)/$(2)-kernel.jffs2; \
		dd if=$(KDIR_TMP)/$(2)-root.ubi \
	) > $(imageraw)
	dd if=$(imageraw) of=$(BIN_DIR)/$(IMG_PREFIX)-$(2)-$(1)-factory.bin \
		bs=128k conv=sync
	$(call Image/Build/SysupgradeNAND,$(2),squashfs,$(KDIR_TMP)/$(2)-kernel.jffs2)
endef


Image/Build/OpenMesh/buildkernel=$(call MkuImageLzma,$(2))
Image/Build/OpenMesh/initramfs=$(call MkuImageLzma/initramfs,$(2),)

define Image/Build/OpenMesh
	-sh $(TOPDIR)/scripts/om-fwupgradecfg-gen.sh \
		"$(4)" \
		"$(BUILD_DIR)/fwupgrade.cfg-$(4)" \
		"$(KDIR_TMP)/vmlinux-$(2).uImage" \
		"$(KDIR)/root.$(1)"
	-sh $(TOPDIR)/scripts/combined-ext-image.sh \
		"$(4)" "$(call factoryname,$(1),$(2))" \
		"$(BUILD_DIR)/fwupgrade.cfg-$(4)" "fwupgrade.cfg" \
		"$(KDIR_TMP)/vmlinux-$(2).uImage" "kernel" \
		"$(KDIR)/root.$(1)" "rootfs"
	if [ -e "$(call factoryname,$(1),$(2))" ]; then \
		cp "$(call factoryname,$(1),$(2))" "$(call sysupname,$(1),$(2))"; \
	fi
endef


Image/Build/Zcomax/buildkernel=$(call MkuImageLzma,$(2),$(3) $(4))
Image/Build/Zcomax/initramfs=$(call MkuImageLzma/initramfs,$(2),$(3) $(4))

define Image/Build/Zcomax
	$(call Sysupgrade/RKuImage,$(1),$(2),1507328,6356992)
	if [ -e "$(call sysupname,$(1),$(2))" ]; then \
		$(STAGING_DIR_HOST)/bin/mkzcfw \
			-B $(2) \
			-k $(KDIR_TMP)/vmlinux-$(2).uImage \
			-r $(KDIR)/root.$(1) \
			-o $(call imgname,$(1),$(2))-factory.img; \
	fi
endef


# $(1): template name to be defined.
# $(2): squashfs suffix to be used.
define BuildTemplate
  # $(1)     : name of build method.
  # $(2)     : board name.
  # $(3)     : kernel command line.
  # $(4)~$(8): extra arguments.
  define Image/Build/Template/$(1)/initramfs
    $$(call Image/Build/$$(1)/initramfs,initramfs,$$(2),$$(3),$$(4),$$(5),$$(6),$$(7),$$(8),$$(9),$$(10))
  endef
  define Image/Build/Template/$(1)/loader
    $$(call Image/Build/$$(1)/loader,$$(2),$$(3),$$(4),$$(5),$$(6),$$(7),$$(8),$$(9),$$(10))
  endef
  define Image/Build/Template/$(1)/buildkernel
    $$(call Image/Build/$$(1)/buildkernel,,$$(2),$$(3),$$(4),$$(5),$$(6),$$(7),$$(8),$$(9),$$(10))
  endef
  define Image/Build/Template/$(1)/squashfs
    $$(call Image/Build/$$(1),squashfs$(2),$$(2),$$(3),$$(4),$$(5),$$(6),$$(7),$$(8),$$(9),$$(10))
  endef
endef

$(eval $(call BuildTemplate,squashfs-only))
$(eval $(call BuildTemplate,64k,-64k))
$(eval $(call BuildTemplate,64kraw,-raw))
$(eval $(call BuildTemplate,64kraw-nojffs,-raw))
$(eval $(call BuildTemplate,128k))
$(eval $(call BuildTemplate,128kraw,-raw))
$(eval $(call BuildTemplate,256k))
$(eval $(call BuildTemplate,all))

ifeq ($(SUBTARGET),generic)
$(eval $(call SingleProfile,ALFA,64k,ALFANX,alfa-nx,ALFA-NX,ttyS0,115200,$$(alfa_mtdlayout_8M),1638400,6291456,vmlinux.gz.uImage,pb9x-2.6.31-jffs2))
$(eval $(call SingleProfile,ALFA,64k,HORNETUB,hornet-ub,HORNET-UB,ttyATH0,115200,$$(alfa_mtdlayout_8M),1638400,6291456,kernel_image,rootfs_image))
$(eval $(call SingleProfile,ALFA,64k,TUBE2H8M,tube2h-8M,TUBE2H,ttyATH0,115200,$$(alfa_mtdlayout_8M),1638400,6291456,kernel.image,rootfs.image))

$(eval $(call SingleProfile,AthGzip,64k,AP96,ap96,AP96,ttyS0,115200,$$(ap96_mtdlayout),RKuImage))
$(eval $(call SingleProfile,AthGzip,64k,WNDAP360,wndap360,WNDAP360,ttyS0,9600,$$(wndap360_mtdlayout),KRuImage,65536))

$(eval $(call SingleProfile,AthLzma,64k,ALFAAP120C,alfa-ap120c,ALFA-AP120C,ttyS0,115200,$$(alfa_ap120c_mtdlayout),RKuImage))
$(eval $(call SingleProfile,AthLzma,64k,ALFAAP96,alfa-ap96,ALFA-AP96,ttyS0,115200,$$(alfa_ap96_mtdlayout),RKuImage))
$(eval $(call SingleProfile,AthLzma,64k,ALL0258N,all0258n,ALL0258N,ttyS0,115200,$$(all0258n_mtdlayout),KRuImage,65536))
$(eval $(call SingleProfile,AthLzma,256k,ALL0315N,all0315n,ALL0315N,ttyS0,115200,$$(all0315n_mtdlayout),KRuImage,262144))
$(eval $(call SingleProfile,AthLzma,64k,AP121_8M,ap121-8M,AP121,ttyATH0,115200,$$(ap121_mtdlayout_8M),RKuImage))
$(eval $(call SingleProfile,AthLzma,64k,AP121_16M,ap121-16M,AP121,ttyATH0,115200,$$(ap121_mtdlayout_16M),RKuImage))
$(eval $(call SingleProfile,AthLzma,64k,AP132,ap132,AP132,ttyS0,115200,$$(ap132_mtdlayout),KRuImage))
$(eval $(call SingleProfile,AthLzma,64k,AP135,ap135-020,AP135-020,ttyS0,115200,$$(ap135_mtdlayout),RKuImage))
$(eval $(call SingleProfile,AthLzma,64k,AP136_010,ap136-010,AP136-010,ttyS0,115200,$$(ap136_mtdlayout),RKuImage))
$(eval $(call SingleProfile,AthLzma,64k,AP136_020,ap136-020,AP136-020,ttyS0,115200,$$(ap136_mtdlayout),RKuImage))
$(eval $(call SingleProfile,AthLzma,64k,AP143_8M,ap143-8M,AP143,ttyS0,115200,$$(ap143_mtdlayout_8M),RKuImage))
$(eval $(call SingleProfile,AthLzma,64k,AP143_16M,ap143-16M,AP143,ttyS0,115200,$$(ap143_mtdlayout_16M),RKuImage))
$(eval $(call SingleProfile,AthLzma,64k,AP147_010,ap147-010,AP147-010,ttyS0,115200,$$(ap147_mtdlayout),RKuImage))
$(eval $(call SingleProfile,AthLzma,64k,AP152_16M,ap152-16M,AP152,ttyS0,115200,$$(ap152_mtdlayout_16M),RKuImage))
$(eval $(call SingleProfile,AthLzma,64k,BXU2000N2,bxu2000n-2-a1,BXU2000n-2-A1,ttyS0,115200,$$(bxu2000n2_mtdlayout),RKuImage))
$(eval $(call SingleProfile,AthLzma,64k,CAP4200AG,cap4200ag,CAP4200AG,ttyS0,115200,$$(cap4200ag_mtdlayout),KRuImage))
$(eval $(call SingleProfile,AthLzma,64k,DB120,db120,DB120,ttyS0,115200,$$(db120_mtdlayout),RKuImage))
$(eval $(call SingleProfile,AthLzma,64k,HORNETUBx2,hornet-ub-x2,HORNET-UB,ttyATH0,115200,$$(alfa_mtdlayout_16M),KRuImage,65536))
$(eval $(call SingleProfile,AthLzma,64k,SGRW500N85BV2,sgr-w500-n85b-v2,SGRW500N85BV2,ttyS0,115200,$$(sgrw500n85bv2_mtdlayout),RKuImage))
$(eval $(call SingleProfile,AthLzma,64k,TUBE2H16M,tube2h-16M,TUBE2H,ttyATH0,115200,$$(alfa_mtdlayout_16M),KRuImage,65536))

$(eval $(call SingleProfile,CameoAP121_8M,64kraw-nojffs,DIR505A1,dir-505-a1,DIR-505-A1,ttyATH0,115200,"HORNET-PACKET-DIR505A1-3",1.99.99,""))

$(eval $(call SingleProfile,CameoAP135,64kraw,DGL5500A1,dgl-5500-a1,DGL-5500-A1,ttyS0,115200,$$(dgl_5500_mtdlayout),"00AP135AR9558-RT-130508-00"))
$(eval $(call SingleProfile,CameoAP135,64kraw,TEW823DRU,tew-823dru,TEW-823DRU,ttyS0,115200,$$(tew823dru_mtdlayout) mem=256M,"00AP135AR9558-RT-131129-00"))

$(eval $(call SingleProfile,CameoDB120,64kraw,DHP1565A1,dhp-1565-a1,DHP-1565-A1,ttyS0,115200,"00DB120AR9344-RT-101214-00"))
$(eval $(call SingleProfile,CameoDB120,64kraw,DIR825C1,dir-825-c1,DIR-825-C1,ttyS0,115200,"00DB120AR9344-RT-101214-00"))
$(eval $(call SingleProfile,CameoDB120,64kraw,DIR835A1,dir-835-a1,DIR-835-A1,ttyS0,115200,"00DB120AR9344-RT-101214-00"))

$(eval $(call SingleProfile,CameoDB120_8M,64kraw,TEW732BR,tew-732br,TEW-732BR,ttyS0,115200,"00DB120AR9341-RT-120906-NA"))

$(eval $(call SingleProfile,CyberTANGZIP,64k,E2100L,e2100l,E2100L,ttyS0,115200,,1.00.01))
$(eval $(call SingleProfile,CyberTANGZIP,64k,WRT160NL,wrt160nl,WRT160NL,ttyS0,115200,,1.00.01))

$(eval $(call SingleProfile,CyberTANLZMA,64k,MYNETREXT,mynet-rext,MYNET-REXT,ttyS0,115200,$$(mynet_rext_mtdlayout) root=31:2,1.00.01))

$(eval $(call SingleProfile,CameoAP94,64kraw,DIR825B1,dir-825-b1,DIR-825-B1,ttyS0,115200,$$(cameo_ap94_mtdlayout),$$(cameo_ap94_mtdlayout_fat),01AP94-AR7161-RT-080619-00,00AP94-AR7161-RT-080619-00))
$(eval $(call SingleProfile,CameoAP94,64kraw,TEW673GRU,tew-673gru,TEW-673GRU,ttyS0,115200,$$(cameo_ap94_mtdlayout),$$(cameo_ap94_mtdlayout_fat),01AP94-AR7161-RT-080619-01,00AP94-AR7161-RT-080619-01))
$(eval $(call SingleProfile,CameoAP94,64kraw,DLRTDEV01,dlrtdev01,DIR-825-B1,ttyS0,115200,$$(dlrtdev_mtdlayout),$$(dlrtdev_mtdlayout_fat),01AP94-AR7161-RT-080619-00,00AP94-AR7161-RT-080619-00))

$(eval $(call SingleProfile,dLANLzma,64k,dLAN_Hotspot,dlan-hotspot,dLAN-Hotspot,ttyATH0,115200,$$(dlan_hotspot_mtdlayout) mem=64M,KRuImage,65536))
$(eval $(call SingleProfile,dLANLzma,64k,dLAN_pro_500_wp,dlan-pro-500-wp,dLAN-pro-500-wp,ttyS0,115200,$$(dlan_pro_500_wp_mtdlayout) mem=128M,KRuImage,65536))
$(eval $(call SingleProfile,dLANLzma,64k,dLAN_pro_1200_ac,dlan-pro-1200-ac,dLAN-pro-1200-ac,ttyS0,115200,$$(dlan_pro_1200_ac_mtdlayout) mem=128M,KRuImage,65536))

$(eval $(call SingleProfile,EnGenius,64k,ESR900,esr900,ESR900,ttyS0,115200,$$(esr900_mtdlayout),KRuImage,,0x4e))
$(eval $(call SingleProfile,EnGenius,64k,ESR1750,esr1750,ESR1750,ttyS0,115200,$$(esr1750_mtdlayout),KRuImage,,0x61))
$(eval $(call SingleProfile,EnGenius,64k,EPG5000,epg5000,EPG5000,ttyS0,115200,$$(epg5000_mtdlayout),KRuImage,,0x71))

$(eval $(call SingleProfile,MyLoader,64k,WP543_8M,wp543,,ttyS0,115200,0x800000,8M))
$(eval $(call SingleProfile,MyLoader,64k,WP543_16M,wp543,,ttyS0,115200,0x1000000,16M))
$(eval $(call SingleProfile,MyLoader,64k,WPE72_8M,wpe72,,ttyS0,115200,0x800000,8M))
$(eval $(call SingleProfile,MyLoader,64k,WPE72_16M,wpe72,,ttyS0,115200,0x1000000,16M))

$(eval $(call SingleProfile,Netgear,64kraw,WNR2200,wnr2200,WNR2200,ttyS0,115200,$$(wnr2200_mtdlayout),0x32323030,wnr2200,"" NA,))

$(eval $(call SingleProfile,OpenMesh,squashfs-only,A60,a60,,,,A60))
$(eval $(call SingleProfile,OpenMesh,squashfs-only,OM2P,om2p,,,,OM2P))
$(eval $(call SingleProfile,OpenMesh,squashfs-only,OM5P,om5p,,,,OM5P))
$(eval $(call SingleProfile,OpenMesh,squashfs-only,OM5PAC,om5pac,,,,OM5PAC))
$(eval $(call SingleProfile,OpenMesh,squashfs-only,MR600,mr600,,,,MR600))
$(eval $(call SingleProfile,OpenMesh,squashfs-only,MR900,mr900,,,,MR900))
$(eval $(call SingleProfile,OpenMesh,squashfs-only,MR1750,mr1750,,,,MR1750))

$(eval $(call SingleProfile,PB4X,128k,ALL0305,all0305,ALL0305,ttyS0,115200))
$(eval $(call SingleProfile,PB4X,128k,EAP7660D,eap7660d,EAP7660D,ttyS0,115200))
$(eval $(call SingleProfile,PB4X,64k,JA76PF,ja76pf,JA76PF,ttyS0,115200))
$(eval $(call SingleProfile,PB4X,64k,JA76PF2,ja76pf2,JA76PF2,ttyS0,115200))
$(eval $(call SingleProfile,PB4X,64k,JWAP003,jwap003,JWAP003,ttyS0,115200))
$(eval $(call SingleProfile,PB4X,64k,PB42,pb42,PB42,ttyS0,115200))
$(eval $(call SingleProfile,PB4X,64k,PB44,pb44,PB44,ttyS0,115200))

$(eval $(call SingleProfile,Planex,64kraw,MZKW04NU,mzk-w04nu,MZK-W04NU,ttyS0,115200))
$(eval $(call SingleProfile,Planex,64kraw,MZKW300NH,mzk-w300nh,MZK-W300NH,ttyS0,115200))

$(eval $(call SingleProfile,Senao,squashfs-only,EAP300V2,eap300v2,EAP300V2,ttyS0,115200,$$(eap300v2_mtdlayout)))

$(eval $(call SingleProfile,WRT400N,64k,WRT400N,wrt400n,WRT400N,ttyS0,115200))

$(eval $(call SingleProfile,WZRHP128K,128kraw,WZRHPG300NH,wzr-hp-g300nh,WZR-HP-G300NH,ttyS0,115200,WZR-HP-G300NH))
$(eval $(call SingleProfile,WZRHP64K,64kraw,WZRHPG300NH2,wzr-hp-g300nh2,WZR-HP-G300NH2,ttyS0,115200,WZR-HP-G300NH2))
$(eval $(call SingleProfile,WZRHP64K,64kraw,WZRHPAG300H,wzr-hp-ag300h,WZR-HP-AG300H,ttyS0,115200,WZR-HP-AG300H))
$(eval $(call SingleProfile,WZRHP64K,64kraw,WZRHPG450H,wzr-hp-g450h,WZR-HP-G450H,ttyS0,115200,WZR-HP-AG450H))
$(eval $(call SingleProfile,WZRHP64K,64kraw,WZR600DHP,wzr-600dhp,WZR-HP-AG300H,ttyS0,115200,WZR-600DHP))
$(eval $(call SingleProfile,WZRHP64K,64kraw,WZR450HP2,wzr-450hp2,WZR-450HP2,ttyS0,115200,WZR-450HP2))

$(eval $(call SingleProfile,Zcomax,64k,ZCN1523H28,zcn-1523h-2-8,ZCN-1523H-2,ttyS0,115200,$$(zcn1523h_mtdlayout)))
$(eval $(call SingleProfile,Zcomax,64k,ZCN1523H516,zcn-1523h-5-16,ZCN-1523H-5,ttyS0,115200,$$(zcn1523h_mtdlayout)))

endif # ifeq ($(SUBTARGET),generic)


ifeq ($(SUBTARGET),tiny)

$(eval $(call SingleProfile,Belkin,64k,F9K1115V2,f9k1115v2,F9K1115V2,ttyS0,115200,$$(f9k1115v2_mtdlayout),BR-6679BAC))

$(eval $(call SingleProfile,CameoAP91,64kraw,DIR600A1,dir-600-a1,DIR-600-A1,ttyS0,115200,"AP91-AR7240-RT-090223-00"))
$(eval $(call SingleProfile,CameoAP91,64kraw,DIR601A1,dir-601-a1,DIR-600-A1,ttyS0,115200,"AP91-AR7240-RT-090223-02"))
$(eval $(call SingleProfile,CameoAP91,64kraw,FR54RTR,fr-54rtr,DIR-600-A1,ttyS0,115200,"AP91-AR7240-RT-090223-01"))

$(eval $(call SingleProfile,CameoAP99,64kraw,EBR2310C1,ebr-2310-c1,EBR-2310-C1,ttyS0,115200,"AP91-AR7240-RT-090223-03"))
$(eval $(call SingleProfile,CameoAP99,64kraw,DIR615E1,dir-615-e1,DIR-615-E1,ttyS0,115200,"AP93-AR7240-RT-081028-00"))
$(eval $(call SingleProfile,CameoAP99,64kraw,DIR615E4,dir-615-e4,DIR-615-E4,ttyS0,115200,"AP99-AR7240-RT-091105-05"))

$(eval $(call SingleProfile,CameoAP123_4M,64kraw,DIR615I1,dir-615-i1,DIR-615-I1,ttyS0,115200,"00DB120AR9341-RT-1012I1-00"))
$(eval $(call SingleProfile,CameoAP123_4M,64kraw,DIR615I3,dir-615-i3,DIR-615-I1,ttyS0,115200,"00DB120AR9341-RT-101214-00"))

$(eval $(call SingleProfile,CameoAP81,64kraw-nojffs,A02RBW300N,a02-rb-w300n,TEW-632BRP,ttyS0,115200,"AP81-AR9130-RT-070614-03"))
$(eval $(call SingleProfile,CameoAP81,64kraw-nojffs,DIR615C1,dir-615-c1,DIR-615-C1,ttyS0,115200,"AP81-AR9130-RT-070614-02"))
$(eval $(call SingleProfile,CameoAP81,64kraw-nojffs,TEW632BRP,tew-632brp,TEW-632BRP,ttyS0,115200,"AP81-AR9130-RT-070614-00"))
$(eval $(call SingleProfile,CameoAP81,64kraw-nojffs,TEW652BRP_FW,tew-652brp,TEW-632BRP,ttyS0,115200,"AP81-AR9130-RT-080609-05"))
$(eval $(call SingleProfile,CameoAP81,64kraw-nojffs,TEW652BRP_RECOVERY,tew-652brp-recovery,TEW-632BRP,ttyS0,115200,"AP81-AR9130-RT-070614-02"))

$(eval $(call SingleProfile,CameoAP121,64kraw-nojffs,TEW712BR,tew-712br,TEW-712BR,ttyATH0,115200,"HORNET-RT-TEW712BR-3",1.99,""))
$(eval $(call SingleProfile,CameoAP121,64kraw-nojffs,DIR601B1,dir-601-b1,TEW-712BR,ttyATH0,115200,"HORNET-RT-DIR601B1-3",2.99.99,"" "NA"))

$(eval $(call SingleProfile,MyLoader,64k,WP543_4M,wp543,,ttyS0,115200,0x400000,4M))
$(eval $(call SingleProfile,MyLoader,64k,WPE72_4M,wpe72,,ttyS0,115200,0x400000,4M))

$(eval $(call SingleProfile,Netgear,64kraw,WNR2000V3,wnr2000v3,WNR2000V3,ttyS0,115200,$$(wnr2000v3_mtdlayout),0x32303033,WNR2000V3,"" NA,-H 29763551+04+32))
$(eval $(call SingleProfile,NetgearLzma,64kraw,WNR2000V4,wnr2000v4,WNR2000V4,ttyS0,115200,$$(wnr2000v4_mtdlayout),0x32303034,WNR2000V4,"" NA,))
$(eval $(call SingleProfile,Netgear,64kraw,WNR2000,wnr2000,WNR2000,ttyS0,115200,$$(wnr2000_mtdlayout),0x32303031,WNR2000,"" NA,))
$(eval $(call SingleProfile,Netgear,64kraw,REALWNR612V2,wnr612v2,WNR612V2,ttyS0,115200,$$(wnr2000v3_mtdlayout),0x32303631,WNR612V2,"",))
$(eval $(call SingleProfile,Netgear,64kraw,N150R,n150r,WNR612V2,ttyS0,115200,$$(wnr2000v3_mtdlayout),0x32303631,N150R,"",))
$(eval $(call SingleProfile,Netgear,64kraw,REALWNR1000V2,wnr1000v2,WNR1000V2,ttyS0,115200,$$(wnr2000v3_mtdlayout),0x31303031,WNR1000V2,"",))
$(eval $(call SingleProfile,Netgear,64kraw,WNR1000V2_VC,wnr1000v2-vc,WNR1000V2,ttyS0,115200,$$(wnr2000v3_mtdlayout),0x31303030,WNR1000V2-VC,"",))
$(eval $(call SingleProfile,Netgear,64kraw,WPN824N,wpn824n,WPN824N,ttyS0,115200,$$(wnr2000v3_mtdlayout),0x31313030,WPN824N,"" NA,))

$(eval $(call SingleProfile,WHRHPG300N,64kraw,WHRG301N,whr-g301n,WHR-G301N,ttyS0,115200,$$(whrhpg300n_mtdlayout),WHR-G301N))
$(eval $(call SingleProfile,WHRHPG300N,64kraw,WHRHPG300N,whr-hp-g300n,WHR-HP-G300N,ttyS0,115200,$$(whrhpg300n_mtdlayout),WHR-HP-G300N))
$(eval $(call SingleProfile,WHRHPG300N,64kraw,WHRHPGN,whr-hp-gn,WHR-HP-GN,ttyS0,115200,$$(whrhpg300n_mtdlayout),WHR-HP-GN))
$(eval $(call SingleProfile,WHRHPG300N,64kraw,WLAEAG300N,wlae-ag300n,WLAE-AG300N,ttyS0,115200,$$(whrhpg300n_mtdlayout),WLAE-AG300N))

$(eval $(call SingleProfile,ZyXEL,64k,NBG_460N_550N_550NH,nbg460n_550n_550nh,NBG460N,ttyS0,115200,NBG-460N))

endif # ifeq ($(SUBTARGET),tiny)


ifeq ($(SUBTARGET),nand)

$(eval $(call SingleProfile,NetgearNAND,64k,WNDR3700V4,wndr3700v4,WNDR3700_V4,ttyS0,115200,$$(wndr4300_mtdlayout),0x33373033,WNDR3700v4,"",-H 29763948+128+128,wndr4300))
$(eval $(call SingleProfile,NetgearNAND,64k,WNDR4300V1,wndr4300,WNDR4300,ttyS0,115200,$$(wndr4300_mtdlayout),0x33373033,WNDR4300,"",-H 29763948+0+128+128+2x2+3x3,wndr4300))
$(eval $(call SingleProfile,NetgearNAND,64k,R6100,r6100,R6100,ttyS0,115200,$$(r6100_mtdlayout),0x36303030,R6100,"",-H 29764434+0+128+128+2x2+2x2,wndr4300))

$(eval $(call SingleProfile,ZyXELNAND,128k,NBG6716,nbg6716,NBG6716,ttyS0,115200,NBG6716,$$(zyx_nbg6716_mtdlayout),mem=256M))

endif # ifeq ($(SUBTARGET),nand)

define Image/Build/squashfs
	cp $(KDIR)/root.squashfs $(KDIR)/root.squashfs-raw
	cp $(KDIR)/root.squashfs $(KDIR)/root.squashfs-64k
	$(STAGING_DIR_HOST)/bin/padjffs2 $(KDIR)/root.squashfs-64k 64
	$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-root.$(1) bs=128k conv=sync
endef

define Image/Prepare
	$(if $(wildcard $(ZYXEL_UBOOT_BIN)),cp $(ZYXEL_UBOOT_BIN) $(ZYXEL_UBOOT))
	$(call CompressLzma,$(KDIR)/vmlinux,$(KDIR)/vmlinux.bin.lzma)
ifneq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),)
	$(call CompressLzma,$(KDIR)/vmlinux-initramfs,$(KDIR)/vmlinux-initramfs.bin.lzma)
	$(call Image/BuildLoader,generic,elf,,,-initramfs)
endif
	$(call Image/BuildLoader,generic,elf)
endef

define Image/Prepare/Profile
	$(call Image/Build/Profile/$(1),loader)
endef

define Image/Build/Profile
	$(call Image/Build/Profile/$(1),buildkernel)
	$(if $(CONFIG_TARGET_ROOTFS_INITRAMFS),$(call Image/Build/Profile/$(1),initramfs))
	$(call Image/Build/Profile/$(1),$(2))
endef

# $(1): filesystem type.
define Image/Build
	$(call Image/Build/$(call rootfs_type,$(1)),$(1))
endef
