#CROSS_PATH=mips-linux-gnu-
#CROSS=$(CROSS_PATH)
#CC=$(CROSS)gcc -mips32r2 -msoft-float -muclibc
AR=$(CROSS)ar
LD=$(CROSS)ld
STRIP=$(CROSS)strip
RANLIB=$(CROSS)ranlib
SIZE=$(CROSS)size
#LIBDIR=$(CROSS_PATH)/lib
BRCTLDIR= $(APP_XPON_DIR)/../../public/bridge-utils-1.0.6
PWD = $(shell pwd)

INCLUDE = -I$(KERNEL_HEADER) \
		-I$(APP_XPON_DIR)/inc/omci \
		-I$(APP_XPON_DIR)/inc/omci/omci_me \
		-I$(APP_XPON_DIR)/inc/common \
		-I$(LIB_DIR) \
		-I$(APP_PONMGR_DIR)/inc/common
					
ifneq ($(TCSUPPORT_PONMGR), )
INCLUDE += -I$(APP_PONMGR_DIR)/inc -I$(GLOBAL_INC_DIR)/modules
endif

OMCI_ME_DIR := omci_me
OBJ_DIR := $(PWD)/.tmp



ifneq ($(strip $(TCSUPPORT_CODE_SHRINK)),)
CFLAGS += -Os -Wall $(INCLUDE)
else
CFLAGS += -O2 -Wall $(INCLUDE)
endif

ifneq ($(TCSUPPORT_OMCI), )
	CFLAGS+=-DTCSUPPORT_OMCI
ifneq ($(TCSUPPORT_OMCI_EXTENDED_MSG), )
	CFLAGS+=-DTCSUPPORT_OMCI_EXTENDED_MSG
endif
ifneq ($(TCSUPPORT_SNMP), )
	CFLAGS+=-DTCSUPPORT_SNMP
endif
ifneq ($(TCSUPPORT_CWMP), )
	CFLAGS+=-DTCSUPPORT_CWMP
endif
ifneq ($(TCSUPPORT_OMCI_CTC), )
	CFLAGS+=-DTCSUPPORT_OMCI_CTC
endif
endif

ifneq ($(TC_CFLAGS), )
     CFLAGS+=$(TC_CFLAGS)
endif

ifneq ($(COMPILEOPTION_CFLAGS),)
     CFLAGS+=$(COMPILEOPTION_CFLAGS)
endif

ifneq ($(OPENSOURCE_CFLAGS), )
     CFLAGS+=$(OPENSOURCE_CFLAGS)
endif

LIBS = -lm -lcrypt -L $(LIB_DIR) -lpthread -lrt

ifneq ($(strip $(TCSUPPORT_TCAPI_ENHANCE)),)
ifneq ($(strip $(TCSUPPORT_TCAPI_ENHANCE_GPON)),)
LIBS +=-L $(LIB_DIR) -ltcapi_enhance
else
LIBS +=-L $(LIB_DIR) -ltcapi
endif
else
LIBS +=-L $(LIB_DIR) -ltcapi
endif

ifneq ($(COMPILEOPTION_LDFLAGS),)
LIBS += $(COMPILEOPTION_LDFLAGS)
endif

ifneq ($(OPENSOURCE_LDFLAGS),)
LIBS += $(OPENSOURCE_LDFLAGS)
endif

ifneq ($(strip $(TCSUPPORT_XPON_IGMP)),)
XPON_IGMP_LIB = -L $(LIB_DIR) -lxpon_igmp
LIBS += $(XPON_IGMP_LIB)
endif

ifneq ($(strip $(TCSUPPORT_MT7530_SWITCH_API)),)
MT7530_SWITCH_API = -L $(LIB_DIR) -lmtkswitch
LIBS += $(MT7530_SWITCH_API)
endif

ifneq ($(strip $(TCSUPPORT_PON_VLAN)),)
XPON_VLAN_LIB = -L $(LIB_DIR) -lpon_vlan
LIBS += $(XPON_VLAN_LIB)
endif

ifneq ($(strip $(TCSUPPORT_PON_MAC_FILTER)),)
XPON_MAC_FILTER_LIB = -L $(LIB_DIR) -lpon_mac_filter
LIBS += $(XPON_MAC_FILTER_LIB)
endif

ifneq ($(strip $(TCSUPPORT_GPON_MAPPING)),)
GPON_MAP_LIB = -L $(LIB_DIR) -lgponmap
LIBS += $(GPON_MAP_LIB)
endif 

ifneq ($(strip $(TCSUPPORT_RA_HWNAT)),)
ifneq ($(strip $(TCSUPPORT_MT7510_FE)),)
HWNAT_LIB = -L $(LIB_DIR) -lapi_lib_ppe
LIBS += $(HWNAT_LIB)
endif
endif

ALL_SRC = omci.c omci_me.c omci_timer.c omci_utily.c omci_message_handle.c omci_general_func.c

ifneq ($(TCSUPPORT_OMCI_LAN_DEBUG), )
ALL_SRC += omci_dbg_srv.c
endif

OMCI_ME_SRC := omci_me_equipment_management_func.c omci_me_equipment_management_nodetable.c
OMCI_ME_SRC += omci_me_ani_management_func.c omci_me_ani_management_nodetable.c
OMCI_ME_SRC += omci_me_general_purpose_func.c omci_me_general_purpose_nodetable.c
OMCI_ME_SRC += omci_me_mac_bridge_func.c omci_me_mac_bridge_nodetable.c
OMCI_ME_SRC += omci_me_gemport_mapping_func.c
OMCI_ME_SRC += omci_me_vlan_func.c omci_me_vlan_nodetable.c
OMCI_ME_SRC += omci_me_multicast_func.c omci_me_multicast_nodetable.c
OMCI_ME_SRC += omci_me_enhanced_security_ctl_func.c omci_me_enhanced_security_ctl_nodetable.c
OMCI_ME_SRC += omci_me_crypt_aes.c
OMCI_ME_SRC += omci_me_layer2_data_services_func.c omci_me_layer2_data_services_nodetable.c
OMCI_ME_SRC += omci_me_layer3_data_services_func.c omci_me_layer3_data_services_nodetable.c
OMCI_ME_SRC += omci_me_ethernet_services_func.c omci_me_ethernet_services_nodetable.c
OMCI_ME_SRC += omci_me_traffic_management_func.c omci_me_traffic_management_nodetable.c
OMCI_ME_SRC += omci_me_voip_func.c
ifneq ($(strip $(TCSUPPORT_VOIP)),)	
OMCI_ME_SRC += omci_me_voip_nodetable.c
endif
ifneq ($(TCSUPPORT_OMCI_CTC), )
OMCI_ME_SRC += omci_me_ctc_func.c omci_me_ctc_nodetable.c
endif

ALL_SRC += $(OMCI_ME_SRC:%=$(OMCI_ME_DIR)/%)

OBJS = $(ALL_SRC:.c=.o)
ifneq ($(TCSUPPORT_PONMGR),)
OBJS += $(APP_PONMGR_DIR)/bin/libtrapapi.a
endif
OBJS += $(BRCTLDIR)/libbridge/libbridge.a

TARGET = omci

%.o: %.c
	@echo "***********" compiling $<
	@$(CC) $(CFLAGS) -c -o $@ $< 

all: $(TARGET)

ifneq ($(MAKECMDGOALS), clean)
# dependency files, one for each *.c, named as *.d
# don't panic, the recipe is copied directly from GNU Make's official doc
%.d: %.c
	@echo ***********generating dependency file for $<
	@set -e; rm -f $@; \
	$(CC) $(CFLAGS) -MM $< |\
        sed "s?\\(.*\\):?$(basename $<).o $(basename $<).d :?g" \
        > $@ || rm -rf $@ 	

-include $(ALL_SRC:.c=.d)
endif

$(TARGET): $(OBJS)
	@echo "***********" generating executable: $@
	@$(CC) -Wall -o $(TARGET) $(OBJS) $(LIBS)
	@$(STRIP) $(TARGET)

ifneq ($(TCSUPPORT_PONMGR), )
$(APP_PONMGR_DIR)/bin/libtrapapi.a:
	@$(MAKE) -C $(APP_PONMGR_DIR) clean
	@$(MAKE) -C $(APP_PONMGR_DIR)
endif
	
clean:
	@rm -f *.o *.d $(OMCI_ME_DIR)/*.o $(OMCI_ME_DIR)/*.d $(TARGET)
	@echo clean done!
