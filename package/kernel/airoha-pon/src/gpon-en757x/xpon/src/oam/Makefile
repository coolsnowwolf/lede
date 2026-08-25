#CROSS_PATH=mips-linux-gnu-
#CROSS=$(CROSS_PATH)
#CC=$(CROSS)gcc -mips32r2 -msoft-float -muclibc
AR=$(CROSS)ar
LD=$(CROSS)ld
STRIP=$(CROSS)strip
RANLIB=$(CROSS)ranlib
SIZE=$(CROSS)size

ifneq ($(strip $(TCSUPPORT_TCAPI_ENHANCE)),)
ifneq ($(strip $(TCSUPPORT_TCAPI_ENHANCE_EPON)),)
TCAPILIB=-L $(LIB_DIR) -ltcapi_enhance
else
TCAPILIB=-L $(LIB_DIR) -ltcapi
endif
else
TCAPILIB=-L $(LIB_DIR) -ltcapi
endif

INCLUDE = -I$(KERNEL_HEADER) \
					-I$(APP_XPON_DIR)/inc/oam \
					-I$(APP_XPON_DIR)/inc/common \
					-I$(LIB_DIR)/
ifneq ($(TCSUPPORT_PONMGR), )
INCLUDE += -I$(APP_PONMGR_DIR)/inc -I$(GLOBAL_INC_DIR)/modules
endif

ifneq ($(strip $(TCSUPPORT_CODE_SHRINK)),)
CFLAGS += -Os -Wall $(INCLUDE)
else
CFLAGS += -O2 -Wall $(INCLUDE)
endif
ifneq ($(TC_CFLAGS), )
        CFLAGS+=$(TC_CFLAGS)
endif
ifndef RELEASE
	CFLAGS+=-DCONFIG_DEBUG 
endif
#CFLAGS+=-DTCSUPPORT_EPON_OAM_CUC
ifneq ($(COMPILEOPTION_CFLAGS),)
	CFLAGS+=$(COMPILEOPTION_CFLAGS)
endif

LIBS = -lcrypt -L $(LIB_DIR) -lpthread -lrt
LIBS += -L $(LIB_DIR) -lepon
ifneq ($(COMPILEOPTION_LDFLAGS), )
	LIBS+=$(COMPILEOPTION_LDFLAGS)
endif

OBJ_DIR := $(APP_EPON_OAM_DIR)
CTC_DIR := ctc
OBJ_DIR += $(CTC_DIR)

#ifneq ($(strip $(TCSUPPORT_EPON_OAM_LAN_DBG)),)
LAN_DBG_OBJ := epon_oam_dbg_srv.o
#endif

ifneq ($(strip $(TCSUPPORT_EPON_OAM_CTC)),)
CTC_SRC := oam_ext_buff.c oam_ext_cmn.c oam_sys_api.c oam_ctc_alarm.c oam_ctc_auth.c oam_ctc_churning.c oam_ctc_dba.c oam_ctc_dspch.c oam_ctc_event.c oam_ctc_node.c oam_ctc_swup.c oam_ctc_var.c oam_ext_mgr.c oam_ctc_conf.c
ifneq ($(strip $(TCSUPPORT_VOIP)),)
CTC_SRC += oam_ctc_voip_conf.c
endif
ifneq ($(strip $(TCSUPPORT_EPON_OAM_CUC)),)
CTC_SRC += oam_cuc_node.c oam_cuc_init.c
endif
CTC_OBJ := $(CTC_SRC:%.c=$(CTC_DIR)/%.o)


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

ifneq ($(strip $(TCSUPPORT_EPON_MAPPING)),)
EPON_MAP_LIB = -L $(LIB_DIR) -leponmap
LIBS += $(EPON_MAP_LIB)
endif 
endif


OBJS = epon_oam.o epon_oam_util.o epon_oam_timer.o epon_oam_var.o epon_oam_ext.o epon_oam_cmd.o epon_sys_api.o
ifneq ($(TCSUPPORT_PONMGR),)
OBJS += $(APP_PONMGR_DIR)/bin/libtrapapi.a
OBJS += $(APP_PONMGR_DIR)/bin/libponapi.a
endif
#ifneq ($(strip $(TCSUPPORT_EPON_OAM_LAN_DBG)),)
OBJS += $(LAN_DBG_OBJ)
#endif
ifneq ($(strip $(TCSUPPORT_EPON_OAM_CTC)),)
OBJS += $(CTC_OBJ)
endif


#NEWOBJS
TARGET= epon_oam
$(OBJ_DIR)/%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $< 
	
	
all: $(TARGET)


epon_oam: $(OBJS)
	$(CC) -Wall -o $(TARGET) $(OBJS) $(LIBS) $(TCAPILIB)
	$(STRIP) $(TARGET)
	
ifneq ($(TCSUPPORT_PONMGR), )
$(APP_PONMGR_DIR)/bin/libtrapapi.a:
	$(MAKE) -C $(APP_PONMGR_DIR) clean
	$(MAKE) -C $(APP_PONMGR_DIR)
endif

clean:
	rm -f *.o $(OBJ_DIR)/*.o $(TARGET)
