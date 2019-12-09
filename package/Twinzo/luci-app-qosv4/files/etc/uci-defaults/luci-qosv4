#!/bin/sh 

uci -q batch <<-EOF >/dev/null
          delete ucitrack.@qosv4[-1]
          add ucitrack qosv4
          set ucitrack.@qosv4[-1].init=qosv4   
          commit ucitrack
EOF
rm -f /tmp/luci-indexcahe
exit 0
