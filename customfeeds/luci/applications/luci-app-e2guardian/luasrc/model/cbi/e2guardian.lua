--[[

LuCI E2Guardian module

Copyright (C) 2015, Itus Networks, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Author: Marko Ratkaj <marko.ratkaj@sartura.hr>
	Luka Perkov <luka.perkov@sartura.hr>

]]--

local fs = require "nixio.fs"
local sys = require "luci.sys"

m = Map("e2guardian", translate("E2Guardian"))
m.on_after_commit = function() luci.sys.call("/etc/init.d/e2guardian restart") end

s = m:section(TypedSection, "e2guardian")
s.anonymous = true
s.addremove = false

s:tab("tab_general", translate("General Settings"))
s:tab("tab_additional", translate("Additional Settings"))
s:tab("tab_logs", translate("Logs"))


----------------- General Settings Tab -----------------------

filterip = s:taboption("tab_general", Value, "filterip", translate("IP that E2Guardian listens"))
filterip.datatype = "ip4addr"

filterports = s:taboption("tab_general", Value, "filterports", translate("Port that E2Guardian listens"))
filterports.datatype = "portrange"
filterports.placeholder = "0-65535"

proxyip = s:taboption("tab_general", Value, "proxyip", translate("IP address of the proxy"))
proxyip.datatype = "ip4addr"
proxyip.default = "127.0.0.1"

proxyport = s:taboption("tab_general", Value, "proxyport", translate("Port of the proxy"))
proxyport.datatype = "portrange"
proxyport.placeholder = "0-65535"

languagedir = s:taboption("tab_general", Value, "languagedir", translate("Language dir"))
languagedir.datatype = "string"
languagedir.default = "/usr/share/e2guardian/languages"

language = s:taboption("tab_general", Value, "language", translate("Language to use"))
language.datatype = "string"
language.default = "ukenglish"

loglevel = s:taboption("tab_general", ListValue, "loglevel", translate("Logging Settings"))
loglevel:value("0",  translate("none"))
loglevel:value("1",  translate("just denied"))
loglevel:value("2",  translate("all text based"))
loglevel:value("3",  translate("all requests"))
loglevel.default = "2"

logexceptionhits = s:taboption("tab_general", ListValue, "logexceptionhits", translate("Log Exception Hits"))
logexceptionhits:value("0",  translate("never"))
logexceptionhits:value("1",  translate("log, but don't mark as exceptions"))
logexceptionhits:value("2",  translate("log and mark"))
logexceptionhits.default = "2"

logfileformat = s:taboption("tab_general", ListValue, "logfileformat", translate("Log File Format"))
logfileformat:value("1",  translate("DansgGuardian format, space delimited"))
logfileformat:value("2",  translate("CSV-style format"))
logfileformat:value("3",  translate("Squid Log File Format"))
logfileformat:value("4",  translate("Tab delimited"))
logfileformat:value("5",  translate("Protex format"))
logfileformat:value("6",  translate("Protex format with server field blanked"))
logfileformat.default = "1"

accessdeniedaddress = s:taboption("tab_general", Value, "accessdeniedaddress", translate("Access denied address"),
translate("Server to which the cgi e2guardian reporting script was copied. Reporting levels 1 and 2 only"))
accessdeniedaddress.datatype = "string"
accessdeniedaddress.default = "http://YOURSERVER.YOURDOMAIN/cgi-bin/e2guardian.pl"

usecustombannedimage = s:taboption("tab_general", ListValue, "usecustombannedimage", translate("Banned image replacement"))
usecustombannedimage:value("on",  translate("Yes"))
usecustombannedimage:value("off",  translate("No"))
usecustombannedimage.default = "on"

custombannedimagefile = s:taboption("tab_general", Value, "custombannedimagefile", translate("Custom banned image file"))
custombannedimagefile.datatype = "string"
custombannedimagefile.default = "/usr/share/e2guardian/transparent1x1.gif"

usecustombannedflash = s:taboption("tab_general", ListValue, "usecustombannedflash", translate("Banned flash replacement"))
usecustombannedflash:value("on",  translate("Yes"))
usecustombannedflash:value("off",  translate("No"))
usecustombannedflash.default = "on"

custombannedflashfile = s:taboption("tab_general", Value, "custombannedflashfile", translate("Custom banned flash file"))
custombannedflashfile.datatype = "string"
custombannedflashfile.default = "/usr/share/e2guardian/blockedflash.swf"

filtergroups = s:taboption("tab_general", Value, "filtergroups", translate("Number of filter groups"))
filtergroups.datatype = "and(uinteger,min(1))"
filtergroups.default = "1"

filtergroupslist = s:taboption("tab_general", Value, "filtergroupslist", translate("List of filter groups"))
filtergroupslist.datatype = "string"
filtergroupslist.default = "/etc/e2guardian/lists/filtergroupslist"

bannediplist = s:taboption("tab_general", Value, "bannediplist", translate("List of banned IPs"))
bannediplist.datatype = "string"
bannediplist.default = "/etc/e2guardian/lists/bannediplist"

exceptioniplist = s:taboption("tab_general", Value, "exceptioniplist", translate("List of IP exceptions"))
exceptioniplist.datatype = "string"
exceptioniplist.default = "/etc/e2guardian/lists/exceptioniplist"

perroomblockingdirectory = s:taboption("tab_general", Value, "perroomblockingdirectory", translate("Per-Room blocking definition directory"))
perroomblockingdirectory.datatype = "string"
perroomblockingdirectory.default = "/etc/e2guardian/lists/bannedrooms/"

showweightedfound = s:taboption("tab_general", ListValue, "showweightedfound", translate("Show weighted phrases found"))
showweightedfound:value("on",  translate("Yes"))
showweightedfound:value("off",  translate("No"))
showweightedfound.default = "on"

weightedphrasemode = s:taboption("tab_general", ListValue, "weightedphrasemode", translate("Weighted phrase mode"))
weightedphrasemode:value("0",  translate("off"))
weightedphrasemode:value("1",  translate("on, normal operation"))
weightedphrasemode:value("2",  translate("on, phrase found only counts once on a page"))
weightedphrasemode.default = "2"

urlcachenumber = s:taboption("tab_general", Value, "urlcachenumber", translate("Clean result caching for URLs"))
urlcachenumber.datatype = "and(uinteger,min(0))"
urlcachenumber.default = "1000"

urlcacheage = s:taboption("tab_general", Value, "urlcacheage", translate("Age before they should be ignored in seconds"))
urlcacheage.datatype = "and(uinteger,min(0))"
urlcacheage.default = "900"

scancleancache = s:taboption("tab_general", ListValue, "scancleancache", translate("Cache for content (AV) scans as 'clean'"))
scancleancache:value("on",  translate("Yes"))
scancleancache:value("off",  translate("No"))
scancleancache.default = "on"

phrasefiltermode = s:taboption("tab_general", ListValue, "phrasefiltermode", translate("Filtering options"))
phrasefiltermode:value("0",  translate("raw"))
phrasefiltermode:value("1",  translate("smart"))
phrasefiltermode:value("2",  translate("both raw and smart"))
phrasefiltermode:value("3",  translate("meta/title"))
phrasefiltermode.default = "2"

preservecase = s:taboption("tab_general", ListValue, "perservecase", translate("Lower caseing options"))
preservecase:value("0",  translate("force lower case"))
preservecase:value("1",  translate("don't change"))
preservecase:value("2",  translate("scan fist in lower, then in original"))
preservecase.default = "0"

hexdecodecontent = s:taboption("tab_general", ListValue, "hexdecodecontent", translate("Hex decoding options"))
hexdecodecontent:value("on",  translate("Yes"))
hexdecodecontent:value("off",  translate("No"))
hexdecodecontent.default = "off"

forcequicksearch = s:taboption("tab_general", ListValue, "forcequicksearch", translate("Quick search"))
forcequicksearch:value("on",  translate("Yes"))
forcequicksearch:value("off",  translate("No"))
forcequicksearch.default = "off"

reverseaddresslookups= s:taboption("tab_general", ListValue, "reverseaddresslookups", translate("Reverse lookups for banned site and URLs"))
reverseaddresslookups:value("on",  translate("Yes"))
reverseaddresslookups:value("off",  translate("No"))
reverseaddresslookups.default = "off"

reverseclientiplookups = s:taboption("tab_general", ListValue, "reverseclientiplookups", translate("Reverse lookups for banned and exception IP lists"))
reverseclientiplookups:value("on",  translate("Yes"))
reverseclientiplookups:value("off",  translate("No"))
reverseclientiplookups.default = "off"

logclienthostnames = s:taboption("tab_general", ListValue, "logclienthostnames", translate("Perform reverse lookups on client IPs for successful requests"))
logclienthostnames:value("on",  translate("Yes"))
logclienthostnames:value("off",  translate("No"))
logclienthostnames.default = "off"

createlistcachefiles = s:taboption("tab_general", ListValue, "createlistcachefiles", translate("Build bannedsitelist and bannedurllist cache files"))
createlistcachefiles:value("on",translate("Yes"))
createlistcachefiles:value("off",translate("No"))
createlistcachefiles.default = "on"

prefercachedlists = s:taboption("tab_general", ListValue, "prefercachedlists", translate("Prefer cached list files"))
prefercachedlists:value("on",  translate("Yes"))
prefercachedlists:value("off",  translate("No"))
prefercachedlists.default = "off"

maxuploadsize = s:taboption("tab_general", Value, "maxuploadsize", translate("Max upload size (in Kbytes)"))
maxuploadsize:value("-1",  translate("no blocking"))
maxuploadsize:value("0",  translate("complete block"))
maxuploadsize.default = "-1"

maxcontentfiltersize = s:taboption("tab_general", Value, "maxcontentfiltersize", translate("Max content filter size"),
translate("The value must not be higher than max content ram cache scan size or 0 to match it"))
maxcontentfiltersize.datatype = "and(uinteger,min(0))"
maxcontentfiltersize.default = "256"

maxcontentramcachescansize = s:taboption("tab_general", Value, "maxcontentramcachescansize", translate("Max content ram cache scan size"),
translate("This is the max size of file that DG will download and cache in RAM"))
maxcontentramcachescansize.datatype = "and(uinteger,min(0))"
maxcontentramcachescansize.default = "2000"

maxcontentfilecachescansize = s:taboption("tab_general", Value, "maxcontentfilecachescansize", translate("Max content file cache scan size"))
maxcontentfilecachescansize.datatype = "and(uinteger,min(0))"
maxcontentfilecachescansize.default = "20000"

proxytimeout = s:taboption("tab_general", Value, "proxytimeout", translate("Proxy timeout (5-100)"))
proxytimeout.datatype = "range(5,100)"
proxytimeout.default = "20"

proxyexchange = s:taboption("tab_general", Value, "proxyexchange", translate("Proxy header excahnge (20-300)"))
proxyexchange.datatype = "range(20,300)"
proxyexchange.default = "20"

pcontimeout = s:taboption("tab_general", Value, "pcontimeout", translate("Pconn timeout"),
translate("How long a persistent connection will wait for other requests"))
pcontimeout.datatype = "range(5,300)"
pcontimeout.default = "55"

filecachedir = s:taboption("tab_general", Value, "filecachedir", translate("File cache directory"))
filecachedir.datatype = "string"
filecachedir.default = "/tmp"

deletedownloadedtempfiles = s:taboption("tab_general", ListValue, "deletedownloadedtempfiles", translate("Delete file cache after user completes download"))
deletedownloadedtempfiles:value("on",  translate("Yes"))
deletedownloadedtempfiles:value("off", translate("No"))
deletedownloadedtempfiles.default = "on"

initialtrickledelay = s:taboption("tab_general", Value, "initialtrickledelay", translate("Initial Trickle delay"),
translate("Number of seconds a browser connection is left waiting before first being sent *something* to keep it alive"))
initialtrickledelay.datatype = "and(uinteger,min(0))"
initialtrickledelay.default = "20"

trickledelay = s:taboption("tab_general", Value, "trickledelay", translate("Trickle delay"),
translate("Number of seconds a browser connection is left waiting before being sent more *something* to keep it alive"))
trickledelay.datatype = "and(uinteger,min(0))"
trickledelay.default = "10"

downloadmanager = s:taboption("tab_general", Value, "downloadmanager", translate("Download manager"))
downloadmanager.datatype = "string"
downloadmanager.default = "/etc/e2guardian/downloadmanagers/default.conf"

contentscannertimeout = s:taboption("tab_general", Value, "contentscannertimeout", translate("Content scanner timeout"))
contentscannertimeout.datatype = "and(uinteger,min(0))"
contentscannertimeout.default = "60"

contentscanexceptions = s:taboption("tab_general", ListValue, "contentscanexceptions", translate("Content scan exceptions"))
contentscanexceptions:value("on",  translate("Yes"))
contentscanexceptions:value("off", translate("No"))
contentscanexceptions.default = "off"

recheckreplacedurls = s:taboption("tab_general", ListValue, "recheckreplacedurls", translate("e-check replaced URLs"))
recheckreplacedurls:value("on",  translate("Yes"))
recheckreplacedurls:value("off", translate("No"))
recheckreplacedurls.default = "off"

forwardedfor = s:taboption("tab_general", ListValue, "forwardedfor", translate("Misc setting: forwardedfor"),
translate("If on, it may help solve some problem sites that need to know the source ip."))
forwardedfor:value("on",  translate("Yes"))
forwardedfor:value("off", translate("No"))
forwardedfor.default = "off"

usexforwardedfor = s:taboption("tab_general", ListValue, "usexforwardedfor", translate("Misc setting: usexforwardedfor"),
translate("This is for when you have squid between the clients and E2Guardian"))
usexforwardedfor:value("on",  translate("Yes"))
usexforwardedfor:value("off", translate("No"))
usexforwardedfor.default = "off"

logconnectionhandlingerrors = s:taboption("tab_general", ListValue, "logconnectionhandlingerrors", translate("Log debug info about log()ing and accept()ing"))
logconnectionhandlingerrors:value("on",  translate("Yes"))
logconnectionhandlingerrors:value("off", translate("No"))
logconnectionhandlingerrors.default = "on"

logchildprocesshandling = s:taboption("tab_general", ListValue, "logchildprocesshandling", translate("Log child process handling"))
logchildprocesshandling:value("on",  translate("Yes"))
logchildprocesshandling:value("off", translate("No"))
logchildprocesshandling.default = "off"

maxchildren = s:taboption("tab_general", Value, "maxchildren", translate("Max number of processes to spawn"))
maxchildren.datatype = "and(uinteger,min(0))"
maxchildren.default = "180"

minchildren = s:taboption("tab_general", Value, "minchildren", translate("Min number of processes to spawn"))
minchildren.datatype = "and(uinteger,min(0))"
minchildren.default = "20"

minsparechildren = s:taboption("tab_general", Value, "minsparechildren", translate("Min number of processes to keep ready"))
minsparechildren.datatype = "and(uinteger,min(0))"
minsparechildren.default = "16"

preforkchildren = s:taboption("tab_general", Value, "preforkchildren", translate("Sets minimum nuber of processes when it runs out"))
preforkchildren.datatype = "and(uinteger,min(0))"
preforkchildren.default = "10"

maxsparechildren = s:taboption("tab_general", Value, "maxsparechildren", translate("Sets the maximum number of processes to have doing nothing"))
maxsparechildren.datatype = "and(uinteger,min(0))"
maxsparechildren.default = "32"

maxagechildren = s:taboption("tab_general", Value, "maxagechildren", translate("Max age of child process"))
maxagechildren.datatype = "and(uinteger,min(0))"
maxagechildren.default = "500"

maxips = s:taboption("tab_general", Value, "maxips", translate("Max number of clinets allowed to connect"))
maxips:value("0",  translate("no limit"))
maxips.default = "0"

ipipcfilename = s:taboption("tab_general", Value, "ipipcfilename", translate("IP list IPC server directory and filename"))
ipipcfilename.datatype = "string"
ipipcfilename.default = "/tmp/.dguardianipc"

urlipcfilename = s:taboption("tab_general", Value, "urlipcfilename", translate("Defines URL list IPC server directory and filename used to communicate with the URL cache process"))
urlipcfilename.datatype = "string"
urlipcfilename.default = "/tmp/.dguardianurlipc"

ipcfilename = s:taboption("tab_general", Value, "ipcfilename", translate("Defines URL list IPC server directory and filename used to communicate with the URL cache process"))
ipcfilename.datatype = "string"
ipcfilename.default = "/tmp/.dguardianipipc"

nodeamon = s:taboption("tab_general", ListValue, "nodeamon", translate("Disable deamoning"))
nodeamon:value("on",  translate("Yes"))
nodeamon:value("off", translate("No"))
nodeamon.default = "off"

nologger = s:taboption("tab_general", ListValue, "nologger", translate("Disable logger"))
nologger:value("on",  translate("Yes"))
nologger:value("off", translate("No"))
nologger.default = "off"

logadblock = s:taboption("tab_general", ListValue, "logadblock", translate("Enable logging of ADs"))
logadblock:value("on",  translate("Yes"))
logadblock:value("off", translate("No"))
logadblock.default = "off"

loguseragent = s:taboption("tab_general", ListValue, "loguseragent", translate("Enable logging of client user agent"))
loguseragent:value("on",  translate("Yes"))
loguseragent:value("off", translate("No"))
loguseragent.default = "off"

softrestart = s:taboption("tab_general", ListValue, "softrestart", translate("Enable soft restart"))
softrestart:value("on",  translate("Yes"))
softrestart:value("off", translate("No"))
softrestart.default = "off"


------------------------ Additional Settings Tab ----------------------------

e2guardian_config_file = s:taboption("tab_additional", TextValue, "_data", "")
e2guardian_config_file.wrap = "off"
e2guardian_config_file.rows = 25
e2guardian_config_file.rmempty = false

function e2guardian_config_file.cfgvalue()
	local uci = require "luci.model.uci".cursor_state()
	file = "/etc/e2guardian/e2guardianf1.conf"
	if file then
		return fs.readfile(file) or ""
	else
		return ""
	end
end

function e2guardian_config_file.write(self, section, value)
	if value then
		local uci = require "luci.model.uci".cursor_state()
		file = "/etc/e2guardian/e2guardianf1.conf"
		fs.writefile(file, value:gsub("\r\n", "\n"))
	end
end


---------------------------- Logs Tab -----------------------------

e2guardian_logfile = s:taboption("tab_logs", TextValue, "lines", "")
e2guardian_logfile.wrap = "off"
e2guardian_logfile.rows = 25
e2guardian_logfile.rmempty = true

function e2guardian_logfile.cfgvalue()
	local uci = require "luci.model.uci".cursor_state()
	file = "/tmp/e2guardian/access.log"
	if file then
		return fs.readfile(file) or ""
	else
		return "Can't read log file"
	end
end

function e2guardian_logfile.write()
        return ""
end

return m
