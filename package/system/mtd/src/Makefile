CC = gcc
CFLAGS += -Wall
LDFLAGS += -lubox

obj = mtd.o jffs2.o crc32.o md5.o
obj.seama = seama.o md5.o
obj.wrg = wrg.o md5.o
obj.wrgg = wrgg.o md5.o
obj.tpl = tpl_ramips_recoveryflag.o
obj.ath79 = $(obj.seama) $(obj.wrgg)
obj.gemini = $(obj.wrgg)
obj.brcm = trx.o
obj.bcm47xx = $(obj.brcm)
obj.bcm53xx = $(obj.brcm) $(obj.seama)
obj.mediatek = $(obj.brcm) linksys_bootcount.o
obj.bcm63xx = imagetag.o
obj.bmips = imagetag.o
obj.ramips = $(obj.brcm) $(obj.seama) $(obj.tpl) $(obj.wrg) linksys_bootcount.o
obj.mvebu = linksys_bootcount.o
obj.kirkwood = linksys_bootcount.o
obj.ipq806x = linksys_bootcount.o
obj.ipq40xx = linksys_bootcount.o
obj.qualcommax = linksys_bootcount.o

ifdef FIS_SUPPORT
  obj += fis.o
endif

mtd: $(obj) $(obj.$(TARGET))
clean:
	rm -f *.o jffs2
