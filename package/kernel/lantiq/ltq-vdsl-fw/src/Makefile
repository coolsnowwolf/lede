PROG=w921v_fw_cutter
OBJS=w921v_fw_cutter.c LzmaDecode.c LzmaWrapper.c

all: $(PROG) 

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	rm *.o $(PROG)

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@
