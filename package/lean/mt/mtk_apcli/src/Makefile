SRCS=main.o
apcli:$(SRCS)
	$(CC) $(LDFLAGS) $(SRCS) -liwinfo -o apcli
	ln -s apcli apcli_2g
	ln -s apcli apcli_5g

main.o:main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	rm *.o -f
	rm *.gch -f
	rm apcli -f
	rm apcli_2g -f
	rm apcli_5g -f

distclean:
	rm *.o -f
	rm *.gch -f
	rm apcli -f
	rm apcli_2g -f
	rm apcli_5g -f

