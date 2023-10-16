all:sendat
 
OBJS = sendat.o

sendat:$(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -lpthread
 
clean:
	rm -f sendat *.o