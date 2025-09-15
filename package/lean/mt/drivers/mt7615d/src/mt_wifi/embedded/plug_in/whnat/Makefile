ccflags-y += $(MODULE_FLAGS) -Inet/nat

MODULE_NAME=mt_whnat

OBJS:= woe_main.o
OBJS+= woe_basic.o
OBJS+= wed.o
OBJS+= wdma.o
OBJS+= woe_proc.o
OBJS+= woe_hw.o
OBJS+= woe_mt7615.o
OBJS+= woe_hif.o
OBJS+= woe_ser.o

#######################################
# For integrating to kernel source
######################################
obj-m += $(MODULE_NAME).o


$(MODULE_NAME)-objs += $(OBJS)

install:

clean:
	rm -rf *.o *.ko *.modules .*.cmd .*.o *.order $(MODULE_NAME).mod.c
