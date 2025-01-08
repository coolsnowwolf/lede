GRUB_SERIAL:=$(call qstrip,$(CONFIG_TARGET_SERIAL))
ifeq ($(GRUB_SERIAL),)
$(error This platform requires CONFIG_TARGET_SERIAL be set!)
endif

define Package/base-files/install-target
	$(SED) "s#@GRUB_SERIAL@#$(GRUB_SERIAL)#" $(1)/etc/inittab
endef
