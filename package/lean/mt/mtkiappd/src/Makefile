OBJS = rtmpiapp.o mt_iapp_util.o

# If you want to debug daemon, uncomment following line
# CFLAGS += -ggdb3 -DDBG

mtkiappd: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

clean:
	rm -f core *.o mtkiappd
