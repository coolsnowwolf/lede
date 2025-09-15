#!/usr/bin/awk -f

function bitcount(c) {
	c=and(rshift(c, 1),0x55555555)+and(c,0x55555555)
	c=and(rshift(c, 2),0x33333333)+and(c,0x33333333)
	c=and(rshift(c, 4),0x0f0f0f0f)+and(c,0x0f0f0f0f)
	c=and(rshift(c, 8),0x00ff00ff)+and(c,0x00ff00ff)
	c=and(rshift(c,16),0x0000ffff)+and(c,0x0000ffff)
	return c
}

function ip2int(ip) {
	ret=0
	n=split(ip,a,"\\.")
	for (x=1;x<=n;x++)
		ret=or(lshift(ret,8),a[x])
	return ret
}

function int2ip(ip,ret,x) {
	ret=and(ip,255)
	ip=rshift(ip,8)
	for(;x<3;x++) {
		ret=and(ip,255)"."ret
		ip=rshift(ip,8)
	}
	return ret
}

function compl32(v) {
	ret=xor(v, 0xffffffff)
	return ret
}

BEGIN {
	slpos=index(ARGV[1],"/")
	if (slpos == 0) {
		ipaddr=ip2int(ARGV[1])
		dotpos=index(ARGV[2],".")
		if (dotpos == 0)
			netmask=compl32(2**(32-int(ARGV[2]))-1)
		else
			netmask=ip2int(ARGV[2])
	} else {
		ipaddr=ip2int(substr(ARGV[1],0,slpos-1))
		netmask=compl32(2**(32-int(substr(ARGV[1],slpos+1)))-1)
		ARGV[4]=ARGV[3]
		ARGV[3]=ARGV[2]
	}

	network=and(ipaddr,netmask)
	prefix=32-bitcount(compl32(netmask))
	broadcast=or(network,compl32(netmask))

	print "IP="int2ip(ipaddr)
	print "NETMASK="int2ip(netmask)
	print "BROADCAST="int2ip(broadcast)
	print "NETWORK="int2ip(network)
	print "PREFIX="prefix

	# range calculations:
	# ipcalc <ip> <netmask> <start> <num>

	if (ARGC <= 3)
		exit(0)

	start=or(network,and(ip2int(ARGV[3]),compl32(netmask)))
	limit=network+1
	if (start<limit) start=limit
	if (start==ipaddr) start=ipaddr+1

	end=start+ARGV[4]
	limit=or(network,compl32(netmask))-1
	if (end>limit) end=limit
	if (end==ipaddr) end=ipaddr-1

	if (start>end) {
		print "network ("int2ip(network)"/"prefix") too small" > "/dev/stderr"
		exit(1)
	}

	if (ipaddr > start && ipaddr < end) {
		print "warning: ipaddr inside range - this might not be supported in future releases of Openwrt" > "/dev/stderr"
	}

	print "START="int2ip(start)
	print "END="int2ip(end)
}
