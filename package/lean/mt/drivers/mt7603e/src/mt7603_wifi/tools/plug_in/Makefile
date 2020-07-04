export MODULE_FLAGS

KBUILD_EXTRA_SYMBOLS+= $(RT28xx_DIR)/os/linux/Module.symvers
#CONFIG_PLUGIN_SOFTQ_STAT=m

ifdef CONFIG_PLUGIN_SOFTQ_STAT
obj-$(CONFIG_PLUGIN_SOFTQ_STAT) += softq_statistic/
MODULE_LIST+=softq_statistic
endif

#CONFIG_PLUGIN_CALIB_ROBUST_TEST=m

ifdef CONFIG_PLUGIN_CALIB_ROBUST_TEST
obj-$(CONFIG_PLUGIN_CALIB_ROBUST_TEST) += calib_robust_test/
MODULE_LIST+=calib_robust_test
endif



clean:
	find . -name "*.o" | xargs rm -rf 
	find . -name "*.ko" | xargs rm -rf
	find . -name "modules.order" | xargs rm -rf 
	find . -name "Module.symvers" | xargs rm -rf 
install:
