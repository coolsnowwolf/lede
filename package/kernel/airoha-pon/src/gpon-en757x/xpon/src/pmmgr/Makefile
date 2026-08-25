#CROSS_PATH=mips-linux-gnu-
#CROSS=$(CROSS_PATH)
#CC=$(CROSS)gcc -mips32r2 -msoft-float -muclibc
AR=$(CROSS)ar
LD=$(CROSS)ld
STRIP=$(CROSS)strip
RANLIB=$(CROSS)ranlib
SIZE=$(CROSS)size
#LIBDIR=$(CROSS_PATH)/lib

####################################
# Source Path
####################################
TOPDIR := $(shell pwd)

INCLUDE = -I$(KERNEL_HEADER)
INCLUDE += -I$(APP_XPON_DIR)/inc/pmmgr
INCLUDE += -I$(APP_XPON_DIR)
INCLUDE += -I$(APP_XPON_DIR)/inc/common
INCLUDE += -I$(LIB_DIR)
INCLUDE += -I$(APP_PONMGR_DIR)/inc
INCLUDE += -I$(GLOBAL_INC_DIR)/modules
ifneq ($(strip $(TCSUPPORT_CODE_SHRINK)),)
CFLAGS += -Os -Wall $(INCLUDE)
else
CFLAGS += -O2 -Wall $(INCLUDE)
endif


ifneq ($(TC_CFLAGS), )
      CFLAGS+=$(TC_CFLAGS)
endif

LDLIBS=-lcrypt -lpthread -lrt

ifneq ($(strip $(TCSUPPORT_WAN_EPON)),)
LIBS += -L $(LIB_DIR) -lepon
endif
ifneq ($(strip $(TCSUPPORT_MT7530_SWITCH_API)),)
LDLIBS += -L $(LIB_DIR) -lmtkswitch
endif


ifneq ($(strip $(CROSS)),)
TCAPILIB=-L ../lib -ltcapi
else
TCAPILIB=-L ../lib
endif
	
ifneq ($(COMPILEOPTION_LDFLAGS), )
		TCAPILIB+=$(COMPILEOPTION_LDFLAGS)
endif
	
CFLAGS+=-DPMMGR_DEBUG

LIBS += $(TCOMCILIB)
LIBOBJS = $(TOPDIR)/pmmgr.o $(TOPDIR)/pmmgr_api.o $(TOPDIR)/pmmgr_timer.o $(TOPDIR)/pmmgr_pm.o
OBJS = sample.o $(TOPDIR)/libpmapi.a
.PHONY:all clean $(TOPDIR)/libpmapi.a sample
#NEWOBJS
EXEC = sample
$(TOPDIR)/%.o:$(TOPDIR)/%.c
	$(CC) $(CFLAGS) -c $<

all: lib 

lib: $(TOPDIR)/libpmapi.a

$(TOPDIR)/libpmapi.a: $(LIBOBJS)
	$(AR) cr $@ $^

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS) $(TCAPILIB)
	$(STRIP) $(EXEC)
	
#$(EXEC): $(OBJS)
#	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS) $(TCAPILIB)
#	$(STRIP) $(EXEC)

clean:
	rm -f *.o $(EXEC) *.a
