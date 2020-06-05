BINDIR = /usr/sbin
#TCDIR:=$(BUILD_DIR)/iproute2*/ip*
TCDIR:=$(shell if [ -e "${BUILD_DIR}/iproute2-full" ] ; then echo "${BUILD_DIR}/iproute2-full"/ip* ; else echo "${BUILD_DIR}/iproute2-tiny"/ip* ; fi ; )
#The ncurses library only needed if we remove the ONLYBG switch
#below.  Mostly for debug
#LDLIBS += -lncurses

LDLIBS += -lm


TCOBJS := $(TCDIR)/tc/tc_util.o
TCOBJS += $(TCDIR)/tc/tc_core.o
TCOBJS += $(TCDIR)/tc/q_hfsc.o
TCOBJS += $(TCDIR)/lib/libnetlink.a
TCOBJS += $(TCDIR)/lib/libutil.a
LDFLAGS += -Wl,-export-dynamic 

all: qosmon

qosmon: qosmon.o
	$(CC) $(LDFLAGS) $^ $(TCOBJS) -o $@ $(LDLIBS)

qosmon.o: qosmon.c
	$(CC) -D ONLYBG $(CFLAGS) -I $(TCDIR)/include -I $(TCDIR)/tc -c $^ -o $@

install: all uninstall
	-mkdir -p $(BINDIR)
	cp qosmon  $(BINDIR)

uninstall:
	rm -f $(BINDIR)/qosmon

clean:
	rm -rf *.o *~ .*sw* qosmon

