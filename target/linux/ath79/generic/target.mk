BOARDNAME:=Generic

DEFAULT_PACKAGES += -ddns-scripts_aliyun -ddns-scripts_dnspod wpad-openssl

define Target/Description
	Build firmware images for generic Atheros AR71xx/AR913x/AR934x based boards.
endef
