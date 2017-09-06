authsae_start_interface() {
	local mcast_rate
	local mesh_htmode
	local mesh_band
	local authsae_conf_file="/var/run/authsae-$ifname.cfg"
	local ret=1

	json_get_vars mcast_rate mesh_id
	set_default mcast_rate "12000"

	case "$htmode" in
		HT20|HT40+|HT40-) mesh_htmode="$htmode";;
		*) mesh_htmode="none";;
	esac

	case "$hwmode" in
		*g*) mesh_band=11g;;
		*a*) mesh_band=11a;;
	esac

	if [ "$mcast_rate" -gt 1000 ]; then
		# authsae only allows integers as rates and not things like 5.5
		mcval=$(($mcast_rate / 1000))
	else
		# compat: to still support mbit/s rates
		mcval="$mcast_rate"
	fi

	cat > "$authsae_conf_file" <<EOF
authsae:
{
 sae:
  {
    debug = 0;
    password = "$key";
    group = [19, 26, 21, 25, 20];
    blacklist = 5;
    thresh = 5;
    lifetime = 3600;
  };
 meshd:
  {
    meshid = "$mesh_id";
    interface = "$ifname";
    passive = 0;
    debug = 0;
    mediaopt = 1;
    band = "$mesh_band";
    channel = $channel;
    htmode = "$mesh_htmode";
    mcast-rate = $mcval;
  };
};
EOF

	/usr/bin/meshd-nl80211 -i "$ifname" -s "$mesh_id" -c "$authsae_conf_file" </dev/null >/dev/null 2>/dev/null &
	authsae_pid="$!"
	ret="$?"

	echo $authsae_pid > /var/run/authsae-$ifname.pid
	wireless_add_process "$authsae_pid" "/usr/bin/meshd-nl80211" 1

	[ "$ret" != 0 ] && wireless_setup_vif_failed AUTHSAE_FAILED
	return $ret
}
