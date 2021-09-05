-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

cbimap = Map("asterisk", "asterisk", "")

asterisk = cbimap:section(TypedSection, "asterisk", "Asterisk General Options", "")
asterisk.anonymous = true

agidir = asterisk:option(Value, "agidir", "AGI directory", "")
agidir.rmempty = true

cache_record_files = asterisk:option(Flag, "cache_record_files", "Cache recorded sound files during recording", "")
cache_record_files.rmempty = true

debug = asterisk:option(Value, "debug", "Debug Level", "")
debug.rmempty = true

dontwarn = asterisk:option(Flag, "dontwarn", "Disable some warnings", "")
dontwarn.rmempty = true

dumpcore = asterisk:option(Flag, "dumpcore", "Dump core on crash", "")
dumpcore.rmempty = true

highpriority = asterisk:option(Flag, "highpriority", "High Priority", "")
highpriority.rmempty = true

initcrypto = asterisk:option(Flag, "initcrypto", "Initialise Crypto", "")
initcrypto.rmempty = true

internal_timing = asterisk:option(Flag, "internal_timing", "Use Internal Timing", "")
internal_timing.rmempty = true

logdir = asterisk:option(Value, "logdir", "Log directory", "")
logdir.rmempty = true

maxcalls = asterisk:option(Value, "maxcalls", "Maximum number of calls allowed", "")
maxcalls.rmempty = true

maxload = asterisk:option(Value, "maxload", "Maximum load to stop accepting new calls", "")
maxload.rmempty = true

nocolor = asterisk:option(Flag, "nocolor", "Disable console colors", "")
nocolor.rmempty = true

record_cache_dir = asterisk:option(Value, "record_cache_dir", "Sound files Cache directory", "")
record_cache_dir.rmempty = true
record_cache_dir:depends({ ["cache_record_files"] = "true" })

rungroup = asterisk:option(Flag, "rungroup", "The Group to run as", "")
rungroup.rmempty = true

runuser = asterisk:option(Flag, "runuser", "The User to run as", "")
runuser.rmempty = true

spooldir = asterisk:option(Value, "spooldir", "Voicemail Spool directory", "")
spooldir.rmempty = true

systemname = asterisk:option(Value, "systemname", "Prefix UniquID with system name", "")
systemname.rmempty = true

transcode_via_sln = asterisk:option(Flag, "transcode_via_sln", "Build transcode paths via SLINEAR, not directly", "")
transcode_via_sln.rmempty = true

transmit_silence_during_record = asterisk:option(Flag, "transmit_silence_during_record", "Transmit SLINEAR silence while recording a channel", "")
transmit_silence_during_record.rmempty = true

verbose = asterisk:option(Value, "verbose", "Verbose Level", "")
verbose.rmempty = true

zone = asterisk:option(Value, "zone", "Time Zone", "")
zone.rmempty = true


hardwarereboot = cbimap:section(TypedSection, "hardwarereboot", "Reload Hardware Config", "")

method = hardwarereboot:option(ListValue, "method", "Reboot Method", "")
method:value("web", "Web URL (wget)")
method:value("system", "program to run")
method.rmempty = true

param = hardwarereboot:option(Value, "param", "Parameter", "")
param.rmempty = true


iaxgeneral = cbimap:section(TypedSection, "iaxgeneral", "IAX General Options", "")
iaxgeneral.anonymous = true
iaxgeneral.addremove = true

allow = iaxgeneral:option(MultiValue, "allow", "Allow Codecs", "")
allow:value("alaw", "alaw")
allow:value("gsm", "gsm")
allow:value("g726", "g726")
allow.rmempty = true

canreinvite = iaxgeneral:option(ListValue, "canreinvite", "Reinvite/redirect media connections", "")
canreinvite:value("yes", "Yes")
canreinvite:value("nonat", "Yes when not behind NAT")
canreinvite:value("update", "Use UPDATE rather than INVITE for path redirection")
canreinvite:value("no", "No")
canreinvite.rmempty = true

static = iaxgeneral:option(Flag, "static", "Static", "")
static.rmempty = true

writeprotect = iaxgeneral:option(Flag, "writeprotect", "Write Protect", "")
writeprotect.rmempty = true


sipgeneral = cbimap:section(TypedSection, "sipgeneral", "Section sipgeneral", "")
sipgeneral.anonymous = true
sipgeneral.addremove = true

allow = sipgeneral:option(MultiValue, "allow", "Allow codecs", "")
allow:value("ulaw", "ulaw")
allow:value("alaw", "alaw")
allow:value("gsm", "gsm")
allow:value("g726", "g726")
allow.rmempty = true

port = sipgeneral:option(Value, "port", "SIP Port", "")
port.rmempty = true

realm = sipgeneral:option(Value, "realm", "SIP realm", "")
realm.rmempty = true


moh = cbimap:section(TypedSection, "moh", "Music On Hold", "")

application = moh:option(Value, "application", "Application", "")
application.rmempty = true
application:depends({ ["asterisk.moh.mode"] = "custom" })

directory = moh:option(Value, "directory", "Directory of Music", "")
directory.rmempty = true

mode = moh:option(ListValue, "mode", "Option mode", "")
mode:value("system", "program to run")
mode:value("files", "Read files from directory")
mode:value("quietmp3", "Quite MP3")
mode:value("mp3", "Loud MP3")
mode:value("mp3nb", "unbuffered MP3")
mode:value("quietmp3nb", "Quiet Unbuffered MP3")
mode:value("custom", "Run a custom application")
mode.rmempty = true

random = moh:option(Flag, "random", "Random Play", "")
random.rmempty = true


return cbimap
