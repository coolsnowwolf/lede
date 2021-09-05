add_luci_conffiles()
{
	local filelist="$1"

	# save ssl certs
	if [ -d /etc/nixio ]; then
		find /etc/nixio -type f >> $filelist
	fi

	# save uhttpd certs
	[ -f "/etc/uhttpd.key" ] && echo /etc/uhttpd.key >> $filelist
	[ -f "/etc/uhttpd.crt" ] && echo /etc/uhttpd.crt >> $filelist
}

sysupgrade_init_conffiles="$sysupgrade_init_conffiles add_luci_conffiles"

