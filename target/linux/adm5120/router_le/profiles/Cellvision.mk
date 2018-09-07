#
# Copyright (C) 2007-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/CAS630
	NAME:=Cellvision CAS-630 IP camera (Experimental)
	# TODO: add default packages
	PACKAGES:=-wpad-mini
endef

define Profile/CAS630/Description
	Package set optimized for the Cellvision CAS-630 device.
endef

define Profile/CAS630W
	NAME:=Cellvision CAS-630W IP camera (Experimental)
	# TODO: add default packages
	PACKAGES:=-wpad-mini
endef

define Profile/CAS630W/Description
	Package set optimized for the Cellvision CAS-630W device.
endef

define Profile/CAS670
	NAME:=Cellvision CAS-670 IP camera (Experimental)
	# TODO: add default packages
	PACKAGES:=-wpad-mini
endef

define Profile/CAS670/Description
	Package set optimized for the Cellvision CAS-670 device.
endef

define Profile/CAS670W
	NAME:=Cellvision CAS-670W IP camera (Experimental)
	# TODO: add default packages
	PACKAGES:=-wpad-mini
endef

define Profile/CAS670W/Description
	Package set optimized for the Cellvision CAS-670 device.
endef

define Profile/CAS700
	NAME:=Cellvision CAS-700 IP camera (Experimental)
	# TODO: add default packages
	PACKAGES:=-wpad-mini
endef

define Profile/CAS700/Description
	Package set optimized for the Cellvision CAS-700 device.
endef

define Profile/CAS700W
	NAME:=Cellvision CAS-700W IP camera (Experimental)
	# TODO: add default packages
	PACKAGES:=-wpad-mini
endef

define Profile/CAS700W/Description
	Package set optimized for the Cellvision CAS-700W device.
endef

define Profile/CAS771
	NAME:=Cellvision CAS-771 IP camera (Experimental)
	PACKAGES:=-wpad-mini kmod-video-cpia2 kmod-usb-ohci kmod-usb2 kmod-usb-audio
endef

define Profile/CAS771/Description
	Package set optimized for the Cellvision CAS-771 device.
endef

define Profile/CAS771W
	NAME:=Cellvision CAS-771W IP camera (Experimental)
	PACKAGES:=-wpad-mini kmod-video-cpia2 kmod-usb-ohci kmod-usb2 kmod-usb-audio kmod-rt2500-pci
endef

define Profile/CAS771W/Description
	Package set optimized for the Cellvision CAS-771W device.
endef

define Profile/CAS790
	NAME:=Cellvision CAS-790 IP camera (Experimental)
	# TODO: add default packages
	PACKAGES:=-wpad-mini
endef

define Profile/CAS790/Description
	Package set optimized for the Cellvision CAS-790 device.
endef

define Profile/CAS861
	NAME:=Cellvision CAS-861 IP camera (Experimental)
	# TODO: add default packages
	PACKAGES:=-wpad-mini
endef

define Profile/CAS861/Description
	Package set optimized for the Cellvision CAS-861 device.
endef

define Profile/CAS861W
	NAME:=Cellvision CAS-861W IP camera (Experimental)
	PACKAGES:=kmod-rt2500-pci
endef

define Profile/CAS861W/Description
	Package set optimized for the Cellvision CAS-861W device.
endef

define Profile/NFS101U
	NAME:=Cellvision NFS-101U Network File Server (Experimental)
	PACKAGES:=-wpad-mini kmod-usb-ohci kmod-usb2
endef

define Profile/NFS101U/Description
	Package set optimized for the Cellvision NFS-101U device.
endef

define Profile/NFS101WU
	NAME:=Cellvision NFS-101WU Network File Server (Experimental)
	PACKAGES:=-wpad-mini kmod-usb-ohci kmod-usb2
endef

define Profile/NFS101WU/Description
	Package set optimized for the Cellvision NFS-101WU device.
endef

$(eval $(call Profile,CAS630))
$(eval $(call Profile,CAS630W))
$(eval $(call Profile,CAS670))
$(eval $(call Profile,CAS670W))
$(eval $(call Profile,CAS700))
$(eval $(call Profile,CAS700W))
$(eval $(call Profile,CAS771))
$(eval $(call Profile,CAS771W))
$(eval $(call Profile,CAS790))
$(eval $(call Profile,CAS861))
$(eval $(call Profile,CAS861W))
$(eval $(call Profile,NFS101U))
$(eval $(call Profile,NFS101WU))

