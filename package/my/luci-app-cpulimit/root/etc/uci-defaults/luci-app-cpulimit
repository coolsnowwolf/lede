#!/bin/sh

uci -q batch <<-EOF >/dev/null
          delete ucitrack.@cpulimit[-1]
          add ucitrack cpulimit
          set ucitrack.@cpulimit[-1].init=cpulimit
          commit ucitrack
EOF

/etc/init.d/cpulimit enable

rm -f /tmp/luci-indexcache
exit 0
