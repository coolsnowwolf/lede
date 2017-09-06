#!/bin/sh 

uci -q batch <<-EOF >/dev/null
          delete ucitrack.@xunlei[-1]
          add ucitrack xunlei
          set ucitrack.@xunlei[-1].init=xunlei
          commit ucitrack
EOF

rm -f /tmp/luci-indexcahe
exit 0
