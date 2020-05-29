all: bw_set bw_get bw_print_history_file set_kernel_timezone
bw_set: bw_set.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@  $^ -liptbwctl
bw_get: bw_get.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@  $^ -liptbwctl
bw_print_history_file: bw_print_history_file.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@  $^ -liptbwctl
set_kernel_timezone: set_kernel_timezone.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@  $^ -liptbwctl

clean:
	rm -rf bw_set bw_get print_history_file set_kernel_timezone *.o *~ .*sw*
