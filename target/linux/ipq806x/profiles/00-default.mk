define Profile/Default
  NAME:=Default Profile
  PRIORITY:=1
  PACKAGES:=ath10k-firmware-qca99x0 ath10k-firmware-qca988x ath10k-firmware-qca9984
endef

define Profile/Default/Description
	Default package set compatible with most boards.
endef
$(eval $(call Profile,Default))
