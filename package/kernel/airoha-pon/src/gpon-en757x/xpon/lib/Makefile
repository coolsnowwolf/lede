#
#Configured directories
#
libdir=	$(LIB_DIR)
publiclibdir= $(PUBLIC_LIB_DIR)

CFLAGS += -I$(GLOBAL_INC_DIR)/modules

INSTALL=	/usr/bin/install -c
INSTALL_DIR	=	$(INSTALL) -d
INSTALL_LIB	=	$(LIBTOOL)	$(INSTALL) -m 755

AR=$(CROSS)ar
LD=$(CROSS)ld 
STRIP=$(CROSS)strip
RANLIB=$(CROSS)ranlib
SIZE=$(CROSS)size
LIBDIR=$(CROSS_PATH)/lib


LN      =	/bin/ln -s
RM      =	/bin/rm -f
OPTIM	=	-fPIC -Os -g


CFLAGS += -fPIC -Os -g  -Wall


ifneq ($(TC_CFLAGS), )
        CFLAGS+=$(TC_CFLAGS)
endif
ifneq ($(COMPILEOPTION_CFLAGS), )
CFLAGS+=$(COMPILEOPTION_CFLAGS)
endif
#.SUFFIXES:  .c .o
#.c.o:
#	$(CC) $(CFLAGS) -c $<


LIB_OBJS =
OBJS_DIR = 
TARGETS  =
SOFTLINK =


ifneq ($(strip $(TCSUPPORT_WAN_EPON)),)
OBJS_DIR += epon
EPON_DSOFLAGS=	-Wl,-soname,libepon.so.1,-rpath,$(libdir) -shared $(OPTIM)
LIB_EPON_OBJ = epon/libepon.o
LIB_EPON_SO  = libepon.so.1
LIB_OBJS += $(LIB_EPON_OBJ)
TARGETS  += $(LIB_EPON_SO)
SOFTLINK += libepon.so
endif

ifneq ($(strip $(TCSUPPORT_EPON_MAPPING)),)
OBJS_DIR += eponmap
EPONMAP_DSOFLAGS=	-Wl,-soname,libeponmap.so.1,-rpath,$(libdir) -shared $(OPTIM)
LIB_EPONMAP_OBJ = eponmap/libeponmap.o
LIB_EPONMAP_SO  = libeponmap.so.1
LIB_OBJS += $(LIB_EPONMAP_OBJ)
TARGETS  += $(LIB_EPONMAP_SO)
SOFTLINK += libeponmap.so
endif

ifneq ($(strip $(TCSUPPORT_GPON_MAPPING)),)
OBJS_DIR += gponmap
GPONMAP_DSOFLAGS=   -Wl,-soname,libgponmap.so.1,-rpath,$(libdir) -shared $(OPTIM)
LIB_GPONMAP_OBJ = gponmap/libgponmap.o
LIB_GPONMAP_SO  = libgponmap.so.1
LIB_OBJS += $(LIB_GPONMAP_OBJ)
TARGETS  += $(LIB_GPONMAP_SO)
SOFTLINK += libgponmap.so
endif


ifneq ($(strip $(TCSUPPORT_PON_VLAN)),)
OBJS_DIR += pon_vlan
PON_VLAN_DSOFLAGS=	-Wl,-soname,libpon_vlan.so.1,-rpath,$(libdir) -shared $(OPTIM)
LIB_PON_VLAN_OBJ = pon_vlan/lib_pon_vlan.o
LIB_PON_VLAN_SO  = libpon_vlan.so.1
LIB_OBJS += $(LIB_PON_VLAN_OBJ)
TARGETS  += $(LIB_PON_VLAN_SO)
SOFTLINK += libpon_vlan.so
endif

ifneq ($(strip $(TCSUPPORT_PON_MAC_FILTER)),)
OBJS_DIR += pon_mac_filter
PON_MAC_FILTER_DSOFLAGS=	-Wl,-soname,libpon_mac_filter.so.1,-rpath,$(libdir) -shared $(OPTIM)
LIB_PON_MAC_FILTER_OBJ = pon_mac_filter/lib_pon_mac_filter.o
LIB_PON_MAC_FILTER_SO  = libpon_mac_filter.so.1
LIB_OBJS += $(LIB_PON_MAC_FILTER_OBJ)
TARGETS  += $(LIB_PON_MAC_FILTER_SO)
SOFTLINK += libpon_mac_filter.so
endif

ifneq ($(strip $(TCSUPPORT_XPON_IGMP)),)
OBJS_DIR += xpon_igmp
XPON_IGMP_DSOFLAGS =	-Wl,-soname,libxpon_igmp.so.1,-rpath,$(libdir) -shared $(OPTIM)
LIB_XPON_IGMP_OBJ = xpon_igmp/lib_xpon_igmp.o xpon_igmp/xpon_igmp_omci.o
LIB_XPON_IGMP_SO  = libxpon_igmp.so.1
LIB_OBJS += $(LIB_XPON_IGMP_OBJ)
TARGETS  += $(LIB_XPON_IGMP_SO)
SOFTLINK += libxpon_igmp.so
endif

#$(OBJS_DIR)/%.o:%.c
#	$(CC) $(CFLAGS) -c -o $@ $< 


all:  $(TARGETS)

$(LIB_EPON_SO): $(LIB_EPON_OBJ)
	$(CC) $(CFLAGS) $(EPON_DSOFLAGS) $(LIB_EPON_OBJ) -o $(LIB_EPON_SO)
	$(STRIP) --strip-unneeded $(LIB_EPON_SO)
	$(RM) libepon.so
	$(LN) $(LIB_EPON_SO) libepon.so

$(LIB_EPONMAP_SO): $(LIB_EPONMAP_OBJ)
	$(CC) $(CFLAGS) $(EPONMAP_DSOFLAGS) $(LIB_EPONMAP_OBJ) -o $(LIB_EPONMAP_SO)
	$(STRIP) --strip-unneeded $(LIB_EPONMAP_SO)
	$(RM) libeponmap.so
	$(LN) $(LIB_EPONMAP_SO) libeponmap.so

$(LIB_GPONMAP_SO): $(LIB_GPONMAP_OBJ)
	$(CC) $(CFLAGS) $(GPONMAP_DSOFLAGS) $(LIB_GPONMAP_OBJ) -o $(LIB_GPONMAP_SO)
	$(STRIP) --strip-unneeded $(LIB_GPONMAP_SO)
	$(RM) libgponmap.so
	$(LN) $(LIB_GPONMAP_SO) libgponmap.so	

$(LIB_PON_VLAN_SO): $(LIB_PON_VLAN_OBJ)
	$(CC) $(CFLAGS) $(PON_VLAN_DSOFLAGS) $(LIB_PON_VLAN_OBJ) -o $(LIB_PON_VLAN_SO)
	$(STRIP) --strip-unneeded $(LIB_PON_VLAN_SO)
	$(RM) libpon_vlan.so
	$(LN) $(LIB_PON_VLAN_SO) libpon_vlan.so

$(LIB_PON_MAC_FILTER_SO): $(LIB_PON_MAC_FILTER_OBJ)
	$(CC) $(CFLAGS) $(PON_MAC_FILTER_DSOFLAGS) $(LIB_PON_MAC_FILTER_OBJ) -o $(LIB_PON_MAC_FILTER_SO)
	$(STRIP) --strip-unneeded $(LIB_PON_MAC_FILTER_SO)
	$(RM) libpon_mac_filter.so
	$(LN) $(LIB_PON_MAC_FILTER_SO) libpon_mac_filter.so

$(LIB_PON_USER_ISOLATION_SO): $(LIB_PON_USER_ISOLATION_OBJ)
	$(CC) $(CFLAGS) $(PON_USER_ISOLATION_DSOFLAGS) $(LIB_PON_USER_ISOLATION_OBJ) -o $(LIB_PON_USER_ISOLATION_SO)
	$(STRIP) --strip-unneeded $(LIB_PON_USER_ISOLATION_SO)
	$(RM) libpon_user_isolation.so
	$(LN) $(LIB_PON_USER_ISOLATION_SO) libpon_user_isolation.so

$(LIB_XPON_IGMP_SO) : $(LIB_XPON_IGMP_OBJ)
	$(CC) $(CFLAGS) $(XPON_IGMP_DSOFLAGS) $(LIB_XPON_IGMP_OBJ) -o $(LIB_XPON_IGMP_SO)
	$(STRIP) --strip-unneeded $(LIB_XPON_IGMP_SO)
	$(RM) libxpon_igmp.so
	$(LN) $(LIB_XPON_IGMP_SO) libxpon_igmp.so
	
install:
	$(INSTALL_DIR) $(BUILDROOT)$(libdir)
#	$(INSTALL_LIB) $(TARGETS) $(BUILDROOT)$(libdir)
#	$(RM) $(BUILDROOT)$(libdir)/$(SOFTLINK)
#	$(LN) $(TARGETS) $(BUILDROOT)$(libdir)/$(SOFTLINK)
ifneq ($(TCSUPPORT_WAN_EPON),1)
	$(INSTALL_LIB) $(LIB_EPON_SO) $(BUILDROOT)$(libdir)
	$(RM) $(BUILDROOT)$(libdir)/libepon.so
	$(LN) $(LIB_EPON_SO) $(BUILDROOT)$(libdir)/libepon.so
endif		
ifneq ($(TCSUPPORT_EPON_MAPPING),1)
	$(INSTALL_LIB) $(LIB_EPONMAP_SO) $(BUILDROOT)$(libdir)
	$(RM) $(BUILDROOT)$(libdir)/libeponmap.so
	$(LN) $(LIB_EPONMAP_SO) $(BUILDROOT)$(libdir)/libeponmap.so
endif		
ifneq ($(TCSUPPORT_GPON_MAPPING),1)
	$(INSTALL_LIB) $(LIB_GPONMAP_SO) $(BUILDROOT)$(libdir)
	$(RM) $(BUILDROOT)$(libdir)/libgponmap.so
	$(LN) $(LIB_GPONMAP_SO) $(BUILDROOT)$(libdir)/libgponmap.so
endif		
ifneq ($(TCSUPPORT_PON_VLAN),1)
	$(INSTALL_LIB) $(LIB_PON_VLAN_SO) $(BUILDROOT)$(libdir)
	$(RM) $(BUILDROOT)$(libdir)/libpon_vlan.so
	$(LN) $(LIB_PON_VLAN_SO) $(BUILDROOT)$(libdir)/libpon_vlan.so
endif		
ifneq ($(TCSUPPORT_PON_MAC_FILTER),1)
	$(INSTALL_LIB) $(LIB_PON_MAC_FILTER_SO) $(BUILDROOT)$(libdir)
	$(RM) $(BUILDROOT)$(libdir)/libpon_mac_filter.so
	$(LN) $(LIB_PON_MAC_FILTER_SO) $(BUILDROOT)$(libdir)/libpon_mac_filter.so
endif	
ifneq ($(TCSUPPORT_XPON_IGMP),1)
	$(INSTALL_LIB) $(LIB_XPON_IGMP_SO) $(BUILDROOT)$(libdir)
	$(RM) $(BUILDROOT)$(libdir)/libxpon_igmp.so
	$(LN) $(LIB_XPON_IGMP_SO) $(BUILDROOT)$(libdir)/libxpon_igmp.so
endif		

clean:
	$(RM) $(TARGETS) $(LIB_OBJS) $(SOFTLINK)
