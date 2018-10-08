#!/usr/bin/awk

function inInterfaces(host){
    return(interfaces ~ "(^| )"host"($| )")
}

function newRule(arp_ip,
    ipt_cmd){
    # checking for existing rules shouldn't be necessary if newRule is
    # always called after db is read, arp table is read, and existing
    # iptables rules are read.
    ipt_cmd="iptables -t mangle -j RETURN -s " arp_ip
    system(ipt_cmd " -C RRDIPT_FORWARD 2>/dev/null || " ipt_cmd " -A RRDIPT_FORWARD")
    ipt_cmd="iptables -t mangle -j RETURN -d " arp_ip
    system(ipt_cmd " -C RRDIPT_FORWARD 2>/dev/null || " ipt_cmd " -A RRDIPT_FORWARD")
}

function total(i){
    return(bw[i "/in"] + bw[i "/out"])
}

function date(    cmd, d){
    cmd="date +%d-%m-%Y_%H:%M:%S"
    cmd | getline d
    close(cmd)
    #!@todo could start a process with "while true; do date ...; done"
    return(d)
}

BEGIN {
    od=""
    fid=1
    debug=0
    rrd=0
}

/^#/ { # get DB filename
    FS=","
    dbFile=FILENAME
    next
}

# data from database; first file
FNR==NR { #!@todo this doesn't help if the DB file is empty.
    if($2 == "NA")
	#!@todo could get interface IP here
	n=$1
    else
	n=$2

    hosts[n] = "" # add this host/interface to hosts
    mac[n]        =  $1
    ip[n]         =  $2
    inter[n]      =  $3
    bw[n "/in"]   =  $4
    bw[n "/out"]  =  $5
    firstDate[n]  =  $7
    lastDate[n]   =  $8
    next
}

# not triggered on the first file
FNR==1 {
    FS=" "
    fid++ #!@todo use fid for all files; may be problematic for empty files
    next
}

# arp: ip hw flags hw_addr mask device
fid==2 {
    #!@todo regex match IPs and MACs for sanity
    arp_ip    = $1
    arp_flags = $3
    arp_mac   = $4
    arp_dev   = $6
    if(arp_flags != "0x0" && !(arp_ip in ip)){
	if(debug)
	    print "new host:", arp_ip, arp_flags > "/dev/stderr"
	hosts[arp_ip] = ""
	mac[arp_ip]   = arp_mac
	ip[arp_ip]    = arp_ip
	inter[arp_ip] = arp_dev
	bw[arp_ip "/in"] = bw[arp_ip "/out"] = 0
	firstDate[arp_ip] = lastDate[arp_ip] = date()
    }
    next
}

#!@todo could use mangle chain totals or tailing "unnact" rules to
# account for data for new hosts from their first presence on the
# network to rule creation. The "unnact" rules would have to be
# maintained at the end of the list, and new rules would be inserted
# at the top.

# skip line
# read the chain name and deal with the data accordingly
fid==3 && $1 == "Chain"{
    rrd=$2 ~ /RRDIPT_.*/
    next
}

fid==3 && rrd && (NF < 9 || $1=="pkts"){ next }

fid==3 && rrd { # iptables input
    if($6 != "*"){
	m=$6
	n=m "/out"
    } else if($7 != "*"){
	m=$7
	n=m "/in"
    } else if($8 != "0.0.0.0/0"){
	m=$8
	n=m "/out"
    } else { # $9 != "0.0.0.0/0"
	m=$9
	n=m "/in"
    }

    # remove host from array; any hosts left in array at END get new
    # iptables rules

    #!@todo this deletes a host if any rule exists; if only one
    # directional rule is removed, this will not remedy the situation
    delete hosts[m]

    if($2 > 0){ # counted some bytes
	if(mode == "diff" || mode == "noUpdate")
	    print n, $2
	if(mode!="noUpdate"){
	    if(inInterfaces(m)){ # if label is an interface
		if(!(m in mac)){ # if label was not in db (also not in
				 # arp table, but interfaces won't be
				 # there anyway)
		    firstDate[m] = date()
		    mac[m] = inter[m] = m
		    ip[m] = "NA"
		    bw[m "/in"]=bw[m "/out"]= 0
		}
	    }
	    bw[n]+=$2
	    lastDate[m] = date()
	}
    }
}

END {
    if(mode=="noUpdate") exit
    close(dbFile)
    system("rm -f " dbFile)
    print "#mac,ip,iface,in,out,total,first_date,last_date" > dbFile
    OFS=","
    for(i in mac)
	print mac[i], ip[i], inter[i], bw[i "/in"], bw[i "/out"], total(i), firstDate[i], lastDate[i] > dbFile
    close(dbFile)
    # for hosts without rules
    for(host in hosts) if(!inInterfaces(host)) newRule(host)
}
