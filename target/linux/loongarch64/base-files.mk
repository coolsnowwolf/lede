GRUB_SERIAL:=$(call qstrip,$(CONFIG_GRUB_SERIAL))
ifeq ($(GRUB_SERIAL),)
$(error This platform requires CONFIG_GRUB_SERIAL be set!)
endif

define Package/base-files/install-target
	$(SED) "s#@GRUB_SERIAL@#$(GRUB_SERIAL)#" $(1)/etc/inittab
endef
