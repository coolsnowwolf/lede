# Makefile for DataFlashBoot.bin
# Must use toolchain with H/W FLoating Point

BASENAME=dfboot
BINNAME=$(BASENAME).bin
OUTNAME=$(BASENAME).out
LSSNAME=$(BASENAME).lss
MAPNAME=$(BASENAME).map

BASENAME2=dfbptest
BINNAME2=$(BASENAME2).bin
OUTNAME2=$(BASENAME2).out
LSSNAME2=$(BASENAME2).lss
MAPNAME2=$(BASENAME2).map

INCPATH=include

CFLAGS_LOCAL=-Os -Wall -I$(INCPATH)
BUILD=$(CC) $(CFLAGS) $(CFLAGS_LOCAL)

LDFLAGS+=-T elf32-littlearm.lds -Ttext 0
LINK=$(LD) $(LDFLAGS)

OBJS=objs/cstartup_ram.o objs/at45.o objs/com.o objs/dataflash.o\
	objs/div0.o objs/init.o objs/main.o objs/asm_isr.o objs/asm_mci_isr.o\
	objs/mci_device.o objs/jump.o objs/_udivsi3.o objs/_umodsi3.o

OBJS2=objs/cstartup_ram.o objs/at45.o objs/com.o objs/dataflash.o\
	objs/div0.o objs/init.o objs/ptmain.o objs/asm_isr.o objs/asm_mci_isr.o\
	objs/mci_device.o objs/jump.o objs/_udivsi3.o objs/_umodsi3.o

I=config.h com.h dataflash.h embedded_services.h main.h stdio.h include/AT91RM9200.h include/lib_AT91RM9200.h 

all:clean $(BASENAME) $(BASENAME2)

$(BASENAME): $(OBJS)
	$(LINK) -n -o $(OUTNAME) $(OBJS)
	$(OBJCOPY) $(OUTNAME) -O binary $(BINNAME)
	$(OBJDUMP) -h -s $(OUTNAME) > $(LSSNAME)
	$(NM) -n $(OUTNAME) | grep -v '\( [aUw] \)\|\(__crc_\)\|\( \$[adt]\)' > $(MAPNAME)
	cp $(BINNAME) binary
	
$(BASENAME2): $(OBJS2)
	$(LINK) -n -o $(OUTNAME2) $(OBJS2)
	$(OBJCOPY) $(OUTNAME2) -O binary $(BINNAME2)
	$(OBJDUMP) -h -s $(OUTNAME2) > $(LSSNAME2)
	$(NM) -n $(OUTNAME2) | grep -v '\( [aUw] \)\|\(__crc_\)\|\( \$[adt]\)' > $(MAPNAME2)
	cp $(BINNAME2) binary
	
# C objects here
objs/at45.o: at45.c	$(I)
	$(BUILD) -c -o objs/at45.o		at45.c
objs/com.o: com.c	$(I)
	$(BUILD) -c -o objs/com.o		com.c
objs/dataflash.o: dataflash.c	$(I)
	$(BUILD) -c -o objs/dataflash.o		dataflash.c
objs/mci_device.o: mci_device.c	$(I)
	$(BUILD) -c -o objs/mci_device.o		mci_device.c
objs/div0.o: div0.c	$(I)
	$(BUILD) -c -o objs/div0.o		div0.c
objs/init.o: init.c	$(I)
	$(BUILD) -c -o objs/init.o		init.c
objs/main.o: main.c	$(I)
	$(BUILD) -c -o objs/main.o		main.c
objs/ptmain.o: main.c	$(I)
	$(BUILD) -c -D PRODTEST -o objs/ptmain.o		main.c

# ASM objects here
objs/asm_isr.o: asm_isr.S
	$(BUILD) -c -o objs/asm_isr.o		asm_isr.S
objs/asm_mci_isr.o: asm_mci_isr.S
	$(BUILD) -c -o objs/asm_mci_isr.o		asm_mci_isr.S
objs/cstartup_ram.o: cstartup_ram.S
	$(BUILD) -c -o objs/cstartup_ram.o	cstartup_ram.S
objs/jump.o: jump.S
	$(BUILD) -c -o objs/jump.o		jump.S
objs/_udivsi3.o: _udivsi3.S
	$(BUILD) -c -o objs/_udivsi3.o		_udivsi3.S
objs/_umodsi3.o: _umodsi3.S
	$(BUILD) -c -o objs/_umodsi3.o 		_umodsi3.S

install:	$(BINNAME) $(BINNAME2)
	cp $(BINNAME) binary
	cp $(BINNAME2) binary

clean:
	rm -f *~
	rm -f objs/*
	rm -f *.out
	rm -f *.bin
	rm -f *.lss
	rm -f *.map
	rm -f .unpacked
	mkdir -p objs
