define Profile/Default
	NAME:=Default Profile (all drivers)
	PACKAGES:= \
		kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport
	PRIORITY := 1
endef

define Profile/Default/Description
	Default package set compatible with most boards.
endef
$(eval $(call Profile,Default))
