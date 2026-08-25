#CROSS=mips-linux-gnu-

#CC=$(CROSS)gcc
AR=$(CROSS)ar
LD=$(CROSS)ld
STRIP=$(CROSS)strip
RANLIB=$(CROSS)ranlib
SIZE=$(CROSS)size

INCLUDE =-I$(LIB_DIR) -I$(APP_XPON_DIR)/../cmd_ci -I$(GLOBAL_INC_DIR)/modules

XPON_OBJS =
XPON_CMDS =

CFLAGS += $(INCLUDE)
ifneq ($(strip $(TCSUPPORT_CODE_SHRINK)),)
CFLAGS += -Os
endif

ifneq ($(strip $(TC_CFLAGS)),)
CFLAGS += $(TC_CFLAGS)
endif


ifneq ($(strip $(TCSUPPORT_WAN_EPON)),)
EPONOBJ = epon/eponcmd.o
EPONLIB = -L $(LIB_DIR) -lepon
ifneq ($(COMPILEOPTION_LDFLAGS), )
		EPONLIB+=$(COMPILEOPTION_LDFLAGS)
endif
XPON_OBJS += epon/eponcmd.o
XPON_CMDS += eponcmd
endif

ifneq ($(strip $(TCSUPPORT_EPON_MAPPING)),)
EPONMAPOBJ = eponmap/eponmapcmd.o
EPONMAPLIB = -L $(LIB_DIR) -leponmap \
						 -L $(LIB_DIR) -ltcapi

ifneq ($(COMPILEOPTION_LDFLAGS), )
		EPONMAPLIB+=$(COMPILEOPTION_LDFLAGS)
endif

XPON_OBJS += $(EPONMAPOBJ)
XPON_CMDS += eponmapcmd
endif

ifneq ($(strip $(TCSUPPORT_GPON_MAPPING)),)
GPONMAPOBJ = gponmap/gponmapcmd.o
GPONMAPLIB = -L $(LIB_DIR) -lgponmap \
             -L $(LIB_DIR) -ltcapi
             
ifneq ($(COMPILEOPTION_LDFLAGS), )
		GPONMAPLIB+=$(COMPILEOPTION_LDFLAGS)
endif

XPON_OBJS += $(GPONMAPOBJ)
XPON_CMDS += gponmapcmd
endif									  

ifneq ($(strip $(TCSUPPORT_PON_VLAN)),)
PONVLANOBJ = pon_vlan/ponvlancmd.o
PONVLANLIB = -L $(LIB_DIR) -lpon_vlan \
						 -L $(LIB_DIR) -ltcapi
						 
ifneq ($(COMPILEOPTION_LDFLAGS), )
		PONVLANLIB+=$(COMPILEOPTION_LDFLAGS)
endif

XPON_OBJS += $(PONVLANOBJ)
XPON_CMDS += ponvlancmd
endif

ifneq ($(strip $(TCSUPPORT_PON_MAC_FILTER)),)
PONMACFILTEROBJ = pon_mac_filter/ponmacfiltercmd.o
PONMACFILTERLIB = -L $(LIB_DIR) -lpon_mac_filter \
						 -L $(LIB_DIR) -ltcapi
						 
ifneq ($(COMPILEOPTION_LDFLAGS), )
		PONMACFILTERLIB+=$(COMPILEOPTION_LDFLAGS)
endif

XPON_OBJS += $(PONMACFILTEROBJ)
XPON_CMDS += ponmacfiltercmd
endif

ifneq ($(strip $(TCSUPPORT_XPON_IGMP)),)
XPONIGMPOBJ = xpon_igmp/xponigmp_cmd.o
XPONIGMPLIB = -L $(LIB_DIR) -lxpon_igmp 
ifneq ($(COMPILEOPTION_LDFLAGS), )
		XPONIGMPLIB+=$(COMPILEOPTION_LDFLAGS)
endif
XPON_OBJS += $(XPONIGMPOBJ)
XPON_CMDS += xponigmpcmd
endif

all: $(XPON_CMDS)

#$(XPON_CMD_OBJ_DIR)/%.o:%.c
#	$(CC) $(CFLAGS) -c -o $@ $< 

eponcmd: $(EPONOBJ)
	$(CC) $(EPONOBJ) -o $@ $(EPONLIB)
	$(STRIP) --strip-unneeded eponcmd
	
eponmapcmd: $(EPONMAPOBJ)
	$(CC) $(EPONMAPOBJ) -o $@ $(EPONMAPLIB)
	$(STRIP) --strip-unneeded eponmapcmd

gponmapcmd: $(GPONMAPOBJ)
	$(CC) $(GPONMAPOBJ) -o $@ $(GPONMAPLIB)
	$(STRIP) --strip-unneeded gponmapcmd
			
ponvlancmd: $(PONVLANOBJ)
	$(CC) $(PONVLANOBJ) -o $@ $(PONVLANLIB)
	$(STRIP) --strip-unneeded ponvlancmd

ponmacfiltercmd: $(PONMACFILTEROBJ)
	$(CC) $(PONMACFILTEROBJ) -o $@ $(PONMACFILTERLIB)
	$(STRIP) --strip-unneeded ponmacfiltercmd

xponigmpcmd: $(XPONIGMPOBJ)
	$(CC) $(XPONIGMPOBJ) -o $@ $(XPONIGMPLIB)
	$(STRIP) --strip-unneeded xponigmpcmd
	

clean:
	rm -f $(XPON_CMDS) $(XPON_OBJS)
