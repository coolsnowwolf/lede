CC = gcc
CFLAGS += -Wall
LDFLAGS += -lubox

obj = mtd.o jffs2.o crc32.o md5.o
obj.seama = seama.o md5.o
obj.wrg = wrg.o md5.o
obj.wrgg = wrgg.o md5.o
obj.ar71xx = trx.o $(obj.seama) $(obj.wrgg)
obj.brcm = trx.o
obj.brcm47xx = $(obj.brcm)
obj.bcm53xx = $(obj.brcm) $(obj.seama)
obj.brcm63xx = imagetag.o
obj.ramips = $(obj.seama) $(obj.wrg)
obj.mvebu = linksys_bootcount.o
obj.kirkwood = linksys_bootcount.o
obj.ipq806x = linksys_bootcount.o

ifdef FIS_SUPPORT
  obj += fis.o
endif

mtd: $(obj) $(obj.$(TARGET))
clean:
	rm -f *.o jffs2
