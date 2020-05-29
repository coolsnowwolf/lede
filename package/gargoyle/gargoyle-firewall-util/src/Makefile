all: make_iptables_rules delete_chain_from_table backup_quotas restore_quotas print_quotas

print_quotas: print_quotas.c
	        $(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ -lericstools -luci -liptbwctl

restore_quotas: restore_quotas.c
	        $(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ -lericstools -luci -liptbwctl

backup_quotas: backup_quotas.c
	        $(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ -lericstools -luci -liptbwctl

delete_chain_from_table: delete_chain_from_table.c
	$(CC) $(CFLAGS) $(LDFLAGS)  $^ -o  $@ -lericstools

make_iptables_rules: make_iptables_rules.c
	$(CC) $(CFLAGS) $(LDFLAGS) make_iptables_rules.c -o make_iptables_rules -lericstools -luci -lm

clean:
	rm -rf make_iptables_rules delete_chain_from_table print_quotas backup_quotas restore_quotas *.o *~ .*sw*
