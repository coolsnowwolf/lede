include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/package.mk

CC = mips-openwrt-linux-gcc
CFLAGS = -Wall -g -O2
INC_DIR= $(STAGING_DIR)/usr/include -I$(BUILD_DIR)/libpcap-*/ipkg-install/usr/include
LINK_DIR= $(STAGING_DIR)/usr/lib -L$(BUILD_DIR)/libpcap-*/ipkg-install/usr/lib
LIBS= $(BUILD_DIR)/libpcap-*/ipkg-install/usr/lib/libpcap.a

OBJ  = mentohust.o dlfunc.o md5.o mycheck.o myconfig.o myfunc.o strnormalize.o myini.o mystate.o
.PHONY: all

all: mentohust

mentohust: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LIBS) -I$(INC_DIR) -L$(LINK_DIR) $(LDFLAGS)

mentohust.o : mentohust.c
	$(CC) $(CFLAGS) -c $< -I$(INC_DIR)

dlfunc.o : dlfunc.c
	$(CC) $(CFLAGS) -c $< -I$(INC_DIR) 

md5.o        : md5.c md5.h
	$(CC) $(CFLAGS) -c $< -I$(INC_DIR)

strnormalize.o: strnormalize.c strnormalize.h
	$(CC) $(CFLAGS) -c $< -I$(INC_DIR)

mycheck.o : mycheck.c
	$(CC) $(CFLAGS) -c $< -I$(INC_DIR)

myconfig.o : myconfig.c
	$(CC) $(CFLAGS) -c $< -I$(INC_DIR)

myfunc.o : myfunc.c
	$(CC) $(CFLAGS) -c $< -I$(INC_DIR)
	
myini.o : myini.c
	$(CC) $(CFLAGS) -c $< -I$(INC_DIR)	
	
mystate.o : mystate.c
	$(CC) $(CFLAGS) -c $< -I$(INC_DIR)
        
clean :
	rm -v $(OBJ) mentohust
