export MODULE_FLAGS

KBUILD_EXTRA_SYMBOLS+= $(RT28xx_DIR)/os/linux/Module.symvers

ifdef CONFIG_WHNAT_SUPPORT
obj-$(CONFIG_WHNAT_SUPPORT)+= whnat/
MODULE_LIST+=mt_whnat
endif

clean:
	find . -name "*.o" | xargs rm -rf
	find . -name "*.ko" | xargs rm -rf
	find . -name "modules.order" | xargs rm -rf
	find . -name "Module.symvers" | xargs rm -rf
install:
