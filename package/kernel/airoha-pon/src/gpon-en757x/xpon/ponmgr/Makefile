#
# 	FILE: make.conf
#
# 	DESCRIPTION: Makefile for PONMGR
#
# 	AUTHOR: Kevin Lin <wl.lin@mediatek.com>
#
##########################################################################
#CROSS	:= 
AS				:= $(CROSS)as
LD				:= $(CROSS)ld
CC				:= $(CROSS)gcc
CXX				:= $(CROSS)c++
CPP				:= $(CC) -E
AR				:= $(CROSS)ar
NM				:= $(CROSS)nm
STRIP			:= $(CROSS)strip
OBJCOPY			:= $(CROSS)objcopy
OBJDUMP			:= $(CROSS)objdump
CP				:= cp
RM				:= rm
MV				:= mv
TAR				:= tar

####################################
# Source Path
####################################
TOPDIR := $(APP_PONMGR_DIR)

INCDIR := $(TOPDIR)/inc
SRCDIR := $(TOPDIR)/src
OBJDIR := $(TOPDIR)/obj
BINDIR := $(TOPDIR)/bin
HAL_INCDIR := $(TOPDIR)/inc/api/hal

SRC_PATHS := $(SRCDIR) \
			 $(SRCDIR)/api \
			 $(SRCDIR)/api/hal \
			 $(SRCDIR)/common \
			 $(SRCDIR)/dspch \
			 $(SRCDIR)/core/cmgr \
			 $(SRCDIR)/core/fmgr \
			 $(SRCDIR)/core/imgr \
			 $(SRCDIR)/core/pmgr \
			 $(SRCDIR)/core/dbmgr \
			 $(TOPDIR)/cmd \
			 $(TOPDIR)/sample
		   
vpath %.c $(SRC_PATHS)


####################################
# Set the compiler options
####################################
PONMGR_CFLAGS := -I$(INCDIR) -I$(HAL_INCDIR) -I$(GLOBAL_INC_DIR)/modules $(TC_CFLAGS)
ifndef RELEASE
	PONMGR_CFLAGS += -g -DCONFIG_DEBUG
endif

####################################
# Set the compiler and link flags
####################################
CFLAGS := $(PONMGR_CFLAGS) -Wall -O0 -Wno-trigraphs -fomit-frame-pointer \
		  -fno-strict-aliasing -fno-common
LDFLAGS = -lm -lpthread -lrt
ifneq ($(COMPILEOPTION_LDFLAGS), )
		LDFLAGS+=$(COMPILEOPTION_LDFLAGS)
endif
DLFLAGS = -fPIC -shared


####################################
# Source Files
####################################
COMMON_SRC := msgqueue.c mgr_util.c mgr_error.c
COMMON_OBJ := $(COMMON_SRC:%.c=$(OBJDIR)/%.o)

IMGR_SRC 	:= imgr.c

CMGR_SRC 	:= cmgr_init.c cmgr.c cmgr_system_proc.c cmgr_pwan_proc.c cmgr_phy_proc.c

ifneq ($(strip $(TCSUPPORT_WAN_GPON)),)
	CMGR_SRC += cmgr_gpon_proc.c
endif
ifneq ($(strip $(TCSUPPORT_WAN_EPON)),)
	CMGR_SRC += cmgr_epon_proc.c
endif

PMGR_SRC 	:= pmgr_init.c pmgr.c pmgr_proc.c 

FMGR_SRC 	:= fmgr_init.c fmgr.c fmgr_proc.c \
			   fmgr_monitor.c fmgr_irq.c 
			   
DSPCH_SRC 	:= dspch_init.c dspch.c

DBMGR_SRC 	:= dbmgr_init.c dbmgr.c 


CORE_SRC := $(IMGR_SRC) $(CMGR_SRC) $(PMGR_SRC) $(FMGR_SRC) $(DSPCH_SRC) $(DBMGR_SRC) 
CORE_OBJ := $(CORE_SRC:%.c=$(OBJDIR)/%.o)

CLI_SRC := sample.c 
CLI_OBJ := $(CLI_SRC:%.c=$(OBJDIR)/%.o)

API_SRC = mgr_api.c mgr_api_init.c
ifneq ($(strip $(TCSUPPORT_XPON_HAL_API)),)
	API_SRC += hal_itf_gpon.c
endif
ifneq ($(strip $(TCSUPPORT_XPON_HAL_API_QOS)),)
	API_SRC += hal_itf_ctqos.c
endif
API_OBJ = $(API_SRC:%.c=$(OBJDIR)/%.o)

TRAP_SRC = mgr_api_trap.c
TRAP_OBJ = $(TRAP_SRC:%.c=$(OBJDIR)/%.o)

DEPS_FILES := $(patsubst %.c, $(OBJDIR)/%.d, $(COMMON_SRC) $(CORE_SRC) $(API_SRC) $(TRAP_SRC)) 


####################################
# build stuff targets
####################################
.PHONY: all build dep clean distclean lib ponmgr sample  $(BINDIR)/libponapi.so $(BINDIR)/libponapi.a


all: dep lib ponmgr #sample_for_omci sample_for_cfgmgr ponmacfilter_lib
	
build: 
	if test ! -d $(OBJDIR); \
		then mkdir obj; \
	fi
	if test ! -d $(BINDIR); \
		then mkdir bin; \
	fi

$(BINDIR)/libponapi.a: $(CORE_OBJ) $(API_OBJ) $(COMMON_OBJ)
	$(AR) cr $@ $^

$(BINDIR)/libtrapapi.a: $(TRAP_OBJ) $(COMMON_OBJ)
	$(AR) cr $@ $^

$(BINDIR)/libponapi.so: $(CORE_OBJ) $(API_OBJ) $(COMMON_OBJ)
	$(CC) $(DLFLAGS) -o $@ $^

ponmacfilter_lib:
ifneq ($(TCSUPPORT_PON_MAC_FILTER), )
	$(MAKE) -C $(LIB_XPON_DIR) clean
	$(MAKE) -C $(LIB_XPON_DIR)
	$(MAKE) -C $(LIB_XPON_DIR)  install
endif

dep: build $(DEPS_FILES)

lib: build $(BINDIR)/libponapi.a $(BINDIR)/libtrapapi.a

sample_for_omci: sample_for_omci.o $(BINDIR)/libtrapapi.a 
	$(CC) -o $(BINDIR)/$@ $^ $(LDFLAGS)

sample_for_cfgmgr: sample_for_cfgmgr.o $(BINDIR)/libponapi.a 
	$(CC) -o $(BINDIR)/$@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.d: %.c
	set -e; \
	$(CC) -M $(CFLAGS) $(INCLUDES) $< \
	| sed "s!\($*\)\.o[ :]*!$(OBJDIR)/\1.o $@ : !g" > $@; \
	[ -s $@ ] || $(RM) $@


ifneq "$(wildcard obj/*.d)" ""
	-include $(DEPS_FILES)
endif


clean:
	$(RM) -f $(OBJDIR)/*.o $(BINDIR)/*.o
	
distclean: clean
	$(RM) -f $(OBJDIR)/*
	$(RM) -Rf $(OBJDIR) $(BINDIR)


####################################
# Cmdline Source Files
####################################
CMDLINE_SRC := main.c cmd_common.c cmdlist_system.c cmd_system.c cmdlist_pwan.c cmd_pwan.c

ifneq ($(strip $(TCSUPPORT_WAN_GPON)),)
	CMDLINE_SRC += cmdlist_gpon.c cmd_gpon.c
endif
ifneq ($(strip $(TCSUPPORT_WAN_EPON)),)
#	CMDLINE_SRC += cmdlist_epon.c cmd_epon.c
endif

CMDLINE_OBJ = $(CMDLINE_SRC:%.c=$(OBJDIR)/%.o)


ifneq ($(strip $(TCSUPPORT_PON_MAC_FILTER)),)
LIBS = -lcrypt -L ../../lib/ -lpon_mac_filter
endif

ponmgr: $(CMDLINE_OBJ) $(BINDIR)/libponapi.a
ifneq ($(strip $(TCSUPPORT_PON_MAC_FILTER)),)
	$(CC) -o $(BINDIR)/$@ $^ $(LDFLAGS) $(LIBS)
else
	$(CC) -o $(BINDIR)/$@ $^ $(LDFLAGS)
endif

