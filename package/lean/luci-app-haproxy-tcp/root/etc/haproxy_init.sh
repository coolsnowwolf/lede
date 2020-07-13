#!/bin/sh /etc/rc.common

CFG_FILE=/etc/haproxy.cfg
stop(){
	logger -t alex stopping haproxy
	echo "stopping haproxy"
	/etc/init.d/haproxy disable
	/etc/init.d/haproxy stop
	[ -f /etc/haproxy_backup ] && {
		cp /etc/haproxy_backup /etc/init.d/haproxy
	}
	iptables -t nat -D OUTPUT -j HAPROXY &> /dev/null
	iptables -t nat -F HAPROXY &> /dev/null
	sleep 1
	iptables -t nat -X HAPROXY &> /dev/null
}
start(){
	echo "starting haproxy"
	logger -t restarting haproxy
	echo global > $CFG_FILE
	cat >> $CFG_FILE <<EOF
  log 127.0.0.1   local0           #[日志输出配置，所有日志都记录在本机，通过local0输出]
  log 127.0.0.1   local1 notice    #定义haproxy 日志级别[error warringinfo debug]
  daemon		                #以后台形式运行harpoxy
  nbproc 1                         #设置进程数量
  pidfile /var/run/haproxy.pid
  ulimit-n 1024                    #ulimit 的数量限制
  maxconn 1024	                #默认最大连接数,需考虑ulimit-n限制
  #chroot /usr/local/haproxy
defaults
  log global
  mode tcp                  #默认的模式mode { tcp|http|health }，tcp是4层，http是7层，health只会返回OK
  retries 3                 #两次连接失败就认为是服务器不可用，也可以通过后面设置
  option abortonclose       #当服务器负载很高的时候，自动结束掉当前队列处理比较久的链接
  option redispatch
  maxconn 1024              #默认的最大连接数
  timeout connect  5000ms   #连接超时
  timeout client 30000ms    #客户端超时
  timeout server 30000ms    #服务器超时
  balance roundrobin        #设置默认负载均衡方式，轮询方式
  #balance source           #设置默认负载均衡方式，类似于nginx的ip_hash
  #balnace leastconn        #设置默认负载均衡方式，最小连接数
listen admin_stats
  bind 0.0.0.0:1111               #节点统计页面的访问端口
  mode http                       #http的7层模式
  option httplog                  #采用http日志格式
  maxconn 10                      #节点统计页面默认的最大连接数
  stats refresh 30s               #节点统计页面自动刷新时间
  stats uri /haproxy              #节点统计页面url
  stats realm Haproxy             #统计页面密码框上提示文本
  stats auth admin:root           #设置监控页面的用户和密码:admin,可以设置多个用户名
  stats  admin if TRUE            #设置手工启动/禁用，后端服务器(haproxy-1.4.9以后版本)
resolvers mydns
  nameserver dns1 114.114.114.114:53
  nameserver dns2 223.5.5.5:53

frontend ss-in
	bind 127.0.0.1:2222
	default_backend ss-out
backend ss-out
	mode tcp
	balance   roundrobin
	option tcplog
EOF
	local COUNTER=0
	#添加主服务器
	iptables -t nat -X HAPROXY
	iptables -t nat -N HAPROXY
	iptables -t nat -F HAPROXY

	while true
	do	
		local server_ip=`uci get haproxy.@main_server[$COUNTER].server_ip 2>/dev/null`
		local server_name=`uci get haproxy.@main_server[$COUNTER].server_name 2>/dev/null`
		local server_port=`uci get haproxy.@main_server[$COUNTER].server_port 2>/dev/null`
		local server_weight=`uci get haproxy.@main_server[$COUNTER].server_weight 2>/dev/null`
		local validate=`uci get haproxy.@main_server[$COUNTER].validate 2>/dev/null`
		if [ -z "$server_ip" ] || [ -z "$server_name" ] || [ -z "$server_port" ] || [ -z "$server_weight" ]; then
			echo break
			break
		fi
		echo the main server $COUNTER $server_ip $server_name $server_port $server_weight
		[ "$validate" = 1 ] && {
			echo server $server_name $server_ip:$server_port weight $server_weight maxconn 1024 check resolvers mydns inter 1500 rise 3 fall 3 >> $CFG_FILE
		}
		iptables -t nat -A HAPROXY -p tcp -d $server_ip -j ACCEPT
		COUNTER=$(($COUNTER+1))
	done
	COUNTER=0
	#添加备用服务器
	while true
	do	
		local server_ip=`uci get haproxy.@backup_server[$COUNTER].server_ip 2>/dev/null`
		local server_name=`uci get haproxy.@backup_server[$COUNTER].server_name 2>/dev/null`
		local server_port=`uci get haproxy.@backup_server[$COUNTER].server_port 2>/dev/null`
		local validate=`uci get haproxy.@backup_server[$COUNTER].validate 2>/dev/null`
		if [ -z "$server_ip" ] || [ -z "$server_name" ] || [ -z "$server_port" ]; then
			echo break
			break
		fi
		echo the backup server $COUNTER $server_ip $server_name $server_port
		[ "$validate" = 1 ] && {
			echo server $server_name $server_ip:$server_port weight 10 check resolvers mydns backup inter 1500 rise 3 fall 3 >> $CFG_FILE
		}
		iptables -t nat -A HAPROXY -p tcp -d $server_ip -j ACCEPT
		COUNTER=$(($COUNTER+1))
	done
	iptables -t nat -I OUTPUT -j HAPROXY
	/etc/init.d/haproxy enable
	/etc/init.d/haproxy restart
	cp /etc/init.d/haproxy /etc/haproxy_backup
	cp /etc/haproxy_start /etc/init.d/haproxy
}

restart(){
	echo luci for haproxy
	sleep 1s
	local vt_enabled=`uci get haproxy.@arguments[0].enabled 2>/dev/null`
	logger -t haproxy is initializing enabled is $vt_enabled
	echo $vt_enabled 
	if [ "$vt_enabled" = 1 ]; then
		[ -f /etc/haproxy_backup ] && {
			cp /etc/haproxy_backup /etc/init.d/haproxy
		}
		iptables -t nat -D OUTPUT -j HAPROXY &> /dev/null
		iptables -t nat -F HAPROXY &> /dev/null
		sleep 1
		iptables -t nat -X HAPROXY &> /dev/null
		start;
	else	
		stop;
	fi
}