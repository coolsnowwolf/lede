SUBTARGET:=ipq807x
BOARDNAME:=Qualcomm Atheros IPQ807x
DEFAULT_PACKAGES += kmod-phy-aquantia ath11k-firmware-ipq8074

define Target/Description
	Build firmware images for Qualcomm Atheros IPQ807x based boards.
endef
