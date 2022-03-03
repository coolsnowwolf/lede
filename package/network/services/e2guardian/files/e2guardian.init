#!/bin/sh /etc/rc.common
# Copyright (C) 2015 OpenWrt.org

START=90
STOP=10

USE_PROCD=1
PROG=/usr/sbin/e2guardian
CONFIGFILE="/tmp/e2guardian/e2guardian.conf"
LOGFILE="/tmp/e2guardian/access.log"
GROUPCONFIG="/tmp/e2guardian/e2guardianf1.conf"

validate_e2guardian_section() {
	uci_validate_section e2guardian e2guardian "${1}" \
		'accessdeniedaddress:string' \
		'bannediplist:string' \
		'contentscanexceptions:string' \
		'contentscanner:string' \
		'contentscannertimeout:uinteger' \
		'createlistcachefiles:string' \
		'custombannedflashfile:string' \
		'custombannedimagefile:string' \
		'deletedownloadedtempfiles:string' \
		'downloadmanager:string' \
		'exceptioniplist:string' \
		'filecachedir:string' \
		'filtergroups:uinteger' \
		'filtergroupslist:string' \
		'filterip:ipaddr' \
		'filterports:port:8080' \
		'forcequicksearch:string' \
		'forwardedfor:string' \
		'hexdecodecontent:string' \
		'initialtrickledelay:uinteger' \
		'ipcfilename:string' \
		'ipipcfilename:string' \
		'languagedir:string' \
		'language:string' \
		'logadblocks:string' \
		'logchildprocesshandling:string' \
		'logclienthostnames:string' \
		'logconnectionhandlingerrors:string' \
		'logexceptionhits:range(0,2)' \
		'logfileformat:range(1,6)' \
		'loglevel:range(0,3)' \
		'loglocation:string' \
		'loguseragent:string' \
		'maxagechildren:uinteger' \
		'maxchildren:uinteger' \
		'maxcontentfilecachescansize:uinteger' \
		'maxcontentfiltersize:uinteger' \
		'maxcontentramcachescansize:uinteger' \
		'maxips:uinteger' \
		'maxsparechildren:uinteger' \
		'maxuploadsize:integer' \
		'minchildren:uinteger' \
		'minsparechildren:uinteger' \
		'nodaemon:string' \
		'nologger:string' \
		'pcontimeout:range(5,300)' \
		'perroomdirectory:string' \
		'phrasefiltermode:range(0,3)' \
		'prefercachedlists:string' \
		'preforkchildren:uinteger' \
		'preservecase:range(0,2)' \
		'proxyexchange:range(20,300)' \
		'proxyip:ipaddr' \
		'proxyport:port:3128' \
		'proxytimeout:range(5,100)' \
		'recheckreplacedurls:string' \
		'reverseaddresslookups:string' \
		'reverseclientiplookups:string' \
		'scancleancache:string' \
		'showweightedfound:string' \
		'softrestart:string' \
		'trickledelay:uinteger' \
		'urlcacheage:uinteger' \
		'urlcachenumber:uinteger' \
		'urlipcfilename:string' \
		'usecustombannedflash:string' \
		'usecustombannedimage:string' \
		'usexforwardedfor:string' \
		'weightedphrasemode:range(0,2)'
}

start_service() {

	local accessdeniedaddress bannediplist contentscanexceptions contentscanner contentscannertimeout \
		createlistcachefiles custombannedflashfile custombannedimagefile deletedownloadedtempfiles \
		downloadmanager exceptioniplist filecachedir loglocation \
		filtergroups filtergroupslist filterip filterports forcequicksearch forwardedfor hexdecodecontent \
		initialtrickledelay ipcfilename ipipcfilename language languagedir logadblocks logchildprocesshandling \
		logclienthostnames logconnectionhandlingerrors logexceptionhits logfileformat loglevel loguseragent \
		maxagechildren maxchildren maxcontentfilecachescansize maxcontentfiltersize maxcontentramcachescansize \
		maxips maxsparechildren maxuploadsize minchildren minsparechildren nodaemon nologger \
		pcontimeout perroomdirectory phrasefiltermode prefercachedlists preforkchildren preservecase proxyexchange \
		proxyip proxyport proxytimeout recheckreplacedurls reverseaddresslookups reverseclientiplookups scancleancache \
		showweightedfound softrestart trickledelay urlcacheage urlcachenumber urlipcfilename usecustombannedflash \
		usecustombannedimage usexforwardedfor weightedphrasemode

	validate_e2guardian_section e2guardian || {
		echo "validation failed"
		return 1
	}

	mkdir -p $(dirname $CONFIGFILE)
	chown -R nobody:nogroup $(dirname $CONFIGFILE)

	mkdir -p $(dirname $loglocation)
	chown -R nobody:nogroup $(dirname $loglocation)

	touch $loglocation
	chown nobody:nogroup $loglocation

	ln -sf $loglocation $(dirname $LOGFILE)
	ln -sf /etc/e2guardian/e2guardian.conf $CONFIGFILE
	ln -sf /etc/e2guardian/e2guardianf1.conf $GROUPCONFIG

	echo "accessdeniedaddress = " $accessdeniedaddress > $CONFIGFILE
	echo "bannediplist = " $bannediplist >> $CONFIGFILE

	if [  "$contentscanner" != ""  ]
	then
		echo "contentscanner = " $contentscanner >> $CONFIGFILE
	fi

	echo "contentscanexceptions = " $contentscanexceptions >> $CONFIGFILE
	echo "contentscannertimeout = " $contentscannertimeout >> $CONFIGFILE
	echo "createlistcachefiles = " $createlistcachefiles >> $CONFIGFILE
	echo "custombannedflashfile = " $custombannedflashfile >> $CONFIGFILE
	echo "custombannedimagefile = " $custombannedimagefile >> $CONFIGFILE
	echo "deletedownloadedtempfiles = " $deletedownloadedtempfiles >> $CONFIGFILE
	echo "downloadmanager = " $downloadmanager >> $CONFIGFILE
	echo "exceptioniplist = " $exceptioniplist >> $CONFIGFILE
	echo "filecachedir = " $filecachedir >> $CONFIGFILE
	echo "filtergroups = " $filtergroups >> $CONFIGFILE
	echo "filtergroupslist = " $filtergroupslist >> $CONFIGFILE
	echo "filterip = " $filterip >> $CONFIGFILE
	echo "filterports = " $filterports >> $CONFIGFILE
	echo "forcequicksearch = " $forcequicksearch >> $CONFIGFILE
	echo "forwardedfor = " $forwardedfor >> $CONFIGFILE
	echo "hexdecodecontent = " $hexdecodecontent >> $CONFIGFILE
	echo "initialtrickledelay = " $initialtrickledelay >> $CONFIGFILE
	echo "ipcfilename = " $ipcfilename >> $CONFIGFILE
	echo "ipipcfilename = " $ipipcfilename >> $CONFIGFILE
	echo "language = " $language >> $CONFIGFILE
	echo "languagedir = " $languagedir >> $CONFIGFILE
	echo "logadblocks = " $logadblocks >> $CONFIGFILE
	echo "logchildprocesshandling = " $logchildprocesshandling >> $CONFIGFILE
	echo "logclienthostnames = " $logclienthostnames >> $CONFIGFILE
	echo "logconnectionhandlingerrors = " $logconnectionhandlingerrors >> $CONFIGFILE
	echo "logexceptionhits = " $logexceptionhits >> $CONFIGFILE
	echo "logfileformat = " $logfileformat >> $CONFIGFILE
	echo "loglevel = " $loglevel >> $CONFIGFILE
	echo "loglocation = " $loglocation >> $CONFIGFILE
	echo "loguseragent = " $loguseragent >> $CONFIGFILE
	echo "maxagechildren = " $maxagechildren >> $CONFIGFILE
	echo "maxchildren = " $maxchildren >> $CONFIGFILE
	echo "maxcontentfilecachescansize = " $maxcontentfilecachescansize >> $CONFIGFILE
	echo "maxcontentfiltersize = " $maxcontentfiltersize >> $CONFIGFILE
	echo "maxcontentramcachescansize = " $maxcontentramcachescansize >> $CONFIGFILE
	echo "maxips = " $maxips >> $CONFIGFILE
	echo "maxsparechildren = " $maxsparechildren >> $CONFIGFILE
	echo "maxuploadsize = " $maxuploadsize >> $CONFIGFILE
	echo "minchildren = " $minchildren >> $CONFIGFILE
	echo "minsparechildren = " $minsparechildren >> $CONFIGFILE
	echo "nodaemon = " $nodaemon >> $CONFIGFILE
	echo "nologger = " $nologger >> $CONFIGFILE
	echo "pcontimeout = " $pcontimeout >> $CONFIGFILE
	echo "perroomdirectory = " $perroomdirectory >> $CONFIGFILE
	echo "phrasefiltermode = " $phrasefiltermode >> $CONFIGFILE
	echo "prefercachedlists = " $prefercachedlists >> $CONFIGFILE
	echo "preforkchildren = " $preforkchildren >> $CONFIGFILE
	echo "preservecase = " $preservecase >> $CONFIGFILE
	echo "proxyexchange = " $proxyexchange >> $CONFIGFILE
	echo "proxyip = " $proxyip >> $CONFIGFILE
	echo "proxyport = " $proxyport >> $CONFIGFILE
	echo "proxytimeout = " $proxytimeout >> $CONFIGFILE
	echo "recheckreplacedurls = " $recheckreplacedurls >> $CONFIGFILE
	echo "reverseaddresslookups = " $reverseaddresslookups >> $CONFIGFILE
	echo "reverseclientiplookups = " $reverseclientiplookups >> $CONFIGFILE
	echo "scancleancache = " $scancleancache >> $CONFIGFILE
	echo "showweightedfound = " $showweightedfound >> $CONFIGFILE
	echo "softrestart = " $softrestart >> $CONFIGFILE
	echo "trickledelay = " $trickledelay >> $CONFIGFILE
	echo "urlcacheage = " $urlcacheage >> $CONFIGFILE
	echo "urlcachenumber = " $urlcachenumber >> $CONFIGFILE
	echo "urlipcfilename = " $urlipcfilename >> $CONFIGFILE
	echo "usecustombannedflash = " $usecustombannedflash >> $CONFIGFILE
	echo "usecustombannedimage = " $usecustombannedimage >> $CONFIGFILE
	echo "usexforwardedfor = " $usexforwardedfor >> $CONFIGFILE
	echo "weightedphrasemode = " $weightedphrasemode >> $CONFIGFILE

	procd_open_instance
	procd_set_param command $PROG -c $CONFIGFILE
	procd_set_param file $CONFIGFILE
	procd_close_instance

}

stop_service()
{
	PID=`cat /tmp/e2guardian/e2guardian.pid`
	kill $PID
	rm -f /tmp/e2guardian/e2guardian.pid
}

service_triggers()
{
	procd_add_reload_trigger "e2guardian"
	procd_add_validation validate_e2guardian_section
}
