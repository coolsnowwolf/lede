#!/bin/sh

cat <<-EOF >$1
base {
	log_debug = off;
	log_info = off;
	log = stderr;
	daemon = on;
	redirector = iptables;
	reuseport = on;
}
EOF

if [ "$2" == "socks5" ]; then
  if [ "$3" == "tcp" ]; then
    if [ "$7" == "0" ]; then
    cat <<-EOF >>$1
redsocks {
  bind = "0.0.0.0:$4";
  relay = "$5:$6";
  type = socks5;
  autoproxy = 0;
  timeout = 10;
}
EOF
  else
    cat <<-EOF >>$1
redsocks {
  bind = "0.0.0.0:$4";
  relay = "$5:$6";
  type = socks5;
  autoproxy = 0;
  timeout = 10;
  login = "$8";
  password = "$9";
}
EOF
    fi
  else
   if [ "$7" == "0" ]; then
    cat <<-EOF >>$1
redudp {
  bind = "0.0.0.0:$4";
  relay = "$5:$6";
  type = socks5;
  udp_timeout = 10;
}
EOF
  else
    cat <<-EOF >>$1
redudp {
  bind = "0.0.0.0:$4";
  relay = "$5:$6";
  type = socks5;
  udp_timeout = 10;
  login = "$8";
  password = "$9";
}
EOF
    fi
  fi
else
    cat <<-EOF >>$1
redsocks {
  bind = "0.0.0.0:$4";
  type = direct;
  interface = $3;
  autoproxy = 0;
  timeout = 10;
}
EOF
fi
