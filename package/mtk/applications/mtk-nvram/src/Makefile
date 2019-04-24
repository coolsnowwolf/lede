exe1 := nvram
exe2 := flash
lib:= libnvram.so
libobjs := crc32.o flash.o nvram.o
exe1objs := main_nvram.o
exe2objs := main_flash.o

CFLAGS += -Wall

ifdef CONFIG_NVRAM_MTD_NAME
CFLAGS += -DNVRAM_MTD_NAME=\"$(CONFIG_NVRAM_MTD_NAME)\"
endif

all: $(lib) $(exe1) $(exe2)

$(lib) : $(libobjs) 
	$(CC) $(CFLAGS) -shared -o $@ $^

$(exe1) : $(exe1objs) $(lib)
	$(CC) $(CFLAGS) -o $@ $^

$(exe2) : $(exe2objs) flash.o
	$(CC) $(CFLAGS) -o $@ $^

$(exe1objs): %.o : %.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

$(exe2objs): %.o : %.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

$(libobjs): %.o : %.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

clean:
	-rm *.o $(exe1) $(exe2) $(lib)
