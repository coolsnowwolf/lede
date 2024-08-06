#!/bin/sh

# check if file exist
if [ ! -f /etc/config/accelerator ]; then
  echo "file not exist, should create..."
  touch /etc/config/accelerator
  uci set accelerator.base=system
  uci set accelerator.device=hardware
  uci set accelerator.Phone=acceleration
  uci set accelerator.PC=acceleration
  uci set accelerator.Game=acceleration
  uci set accelerator.Unknown=acceleration
  uci commit accelerator
fi

uci set accelerator.base.url='https://opapi.nn.com/speed/router/plug/check'
uci set accelerator.base.heart='https://opapi.nn.com/speed/router/heartbeat'
uci set accelerator.base.base_url='https://opapi.nn.com/speed'
uci commit accelerator

uci -q batch <<-EOF >/dev/null
	delete ucitrack.@accelerator[-1]
	add ucitrack accelerator
	set ucitrack.@accelerator[-1].init=acc
	commit ucitrack
EOF

rm -f /tmp/luci-indexcache
exit 0
