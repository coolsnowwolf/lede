OBJS:=appfilter_user.o appfilter_netlink.o appfilter_ubus.o appfilter_config.o main.o
EXEC:=oafd
all: $(OBJS)
	$(CC) -o $(EXEC) $(OBJS) $(LIBS)
clean:
	rm $(EXEC) *.o
