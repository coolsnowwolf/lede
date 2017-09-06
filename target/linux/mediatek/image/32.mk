define Image/BuilduImage
	$(CP) $(KDIR)/zImage$(2) $(KDIR)/zImage-$(1)$(2)
	cat $(LINUX_DIR)/arch/arm/boot/dts/$1.dtb >> $(KDIR)/zImage-$(1)$(2)
	mkimage -A arm -O linux -T kernel -C none -a 0x80008000 -e 0x80008000 -n 'MIPS OpenWrt Linux-$(LINUX_VERSION)'  -d $(KDIR)/zImage-$(1)$(2) $(KDIR)/uImage-$(1)$(2)
endef

define Image/Build/SysupgradeCombined
	$(call Image/BuilduImage,$1)
ifneq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),)
	$(call Image/BuilduImage,$1,-initramfs)
	$(CP) $(KDIR)/uImage-$(1)-initramfs $(BIN_DIR)/$(IMG_PREFIX)-uImage-$(1)-initramfs
endif
	mkdir -p "$(KDIR_TMP)/sysupgrade-$(3)/"
	echo "BOARD=$(3)" > "$(KDIR_TMP)/sysupgrade-$(3)/CONTROL"
	$(CP) "$(KDIR)/root.$(2)" "$(KDIR_TMP)/sysupgrade-$(3)/root"
	$(CP) "$(KDIR)/uImage-$(1)" "$(KDIR_TMP)/sysupgrade-$(3)/kernel"
	(cd "$(KDIR_TMP)"; $(TAR) cvf \
		"$(BIN_DIR)/$(IMG_PREFIX)-$(3)-sysupgrade.tar" sysupgrade-$(3) \
			$(if $(SOURCE_DATE_EPOCH),--mtime="@$(SOURCE_DATE_EPOCH)") \
	)
endef

COMPAT_BPI-R2:=bananapi,bpi-r2
COMPAT_EMMC:=mediatek,mt7623-rfb-emmc
COMPAT_NAND:=mediatek,mt7623-rfb-nand
COMPAT_NAND_EPHY:=mediatek,mt7623-rfb-nand-ephy

define Image/Build/squashfs
	$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
	$(CP) $(KDIR)/root.squashfs $(BIN_DIR)/$(IMG_PREFIX)-root.squashfs

	$(call Image/Build/SysupgradeCombined,mt7623n-bananapi-bpi-r2,squashfs,$$(COMPAT_EMMC))
	$(call Image/Build/SysupgradeCombined,mt7623-eMMC,squashfs,$$(COMPAT_BPI-R2))

	$(call Image/BuilduImage,mt7623-NAND)
	$(call Image/BuilduImage,mt7623-NAND-ePHY)
ifneq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),)
	$(call Image/BuilduImage,mt7623-NAND,-initramfs)
	$(call Image/BuilduImage,mt7623-NAND-ePHY,-initramfs)
	$(CP) $(KDIR)/uImage-mt7623-NAND-initramfs $(BIN_DIR)/$(IMG_PREFIX)-uImage-NAND-initramfs
	$(CP) $(KDIR)/uImage-mt7623-NAND-ePHY-initramfs $(BIN_DIR)/$(IMG_PREFIX)-uImage-NAND-ePHY-initramfs
endif
	$(call Image/Build/SysupgradeNAND,mt7623-NAND,$(1),$(KDIR)/uImage-mt7623-NAND,$$(COMPAT_NAND))
	$(call Image/Build/SysupgradeNAND,mt7623-NAND-ePHY,$(1),$(KDIR)/uImage-mt7623-NAND-ePHY,$$(COMPAT_NAND_EPHY))
endef
