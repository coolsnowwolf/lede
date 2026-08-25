
MODULES_XPON_10G_CUSTOMER_FH_DIR=$(XPON_BSP_MODULES_XPON_10G_CUSTOMER_DRIVER)/fh
export MODULES_XPON_10G_CUSTOMER_FH_DIR



all:
#ifneq ($(strip $(TCSUPPORT_XPON_HAL_API_EXT)),)
	$(MAKE) -C $(MODULES_XPON_10G_CUSTOMER_FH_DIR)
	cp $(MODULES_XPON_10G_CUSTOMER_FH_DIR)/xpon_custom_fh.ko $(BSP_EXT_LIB)/modules/ 
#endif

clean:
#ifneq ($(strip $(TCSUPPORT_XPON_HAL_API_EXT)),)
	rm -f $(BSP_EXT_LIB)/modules/xpon_custom_fh.ko
	$(MAKE) -C $(MODULES_XPON_10G_CUSTOMER_FH_DIR) clean
#endif

