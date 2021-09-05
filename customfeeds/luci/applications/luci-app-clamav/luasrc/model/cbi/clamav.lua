--[[

LuCI ClamAV module

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
require "ubus"

m = Map("clamav", translate("ClamAV"))
m.on_after_commit = function() luci.sys.call("/etc/init.d/clamav restart") end

s = m:section(TypedSection, "clamav")
s.anonymous = true
s.addremove = false

s:tab("tab_advanced", translate("Settings"))
s:tab("tab_logs", translate("Log"))

--------------- Settings --------------

LogFileMaxSize = s:taboption("tab_advanced", Value, "LogFileMaxSize", translate("Max size of log file"))
LogFileMaxSize:value("512K",  translate("512K"))
LogFileMaxSize:value("1M",  translate("1M"))
LogFileMaxSize:value("2M",  translate("2M"))
LogFileMaxSize.default = "1M"

LogTime = s:taboption("tab_advanced", ListValue, "LogTime", translate("Log time with each message"))
LogTime:value("no",  translate("No"))
LogTime:value("yes",  translate("Yes"))
LogTime.default = "no"

LogVerbose = s:taboption("tab_advanced", ListValue, "LogVerbose", translate("Enable verbose logging"))
LogVerbose:value("no",  translate("No"))
LogVerbose:value("yes",  translate("Yes"))
LogVerbose.default = "no"

ExtendedDetectionInfo = s:taboption("tab_advanced", ListValue, "ExtendedDetectionInfo", translate("Log additional infection info"))
ExtendedDetectionInfo:value("no",  translate("No"))
ExtendedDetectionInfo:value("yes",  translate("Yes"))
ExtendedDetectionInfo.default = "no"

dummy3 = s:taboption("tab_advanced", DummyValue, "")
dummy4 = s:taboption("tab_advanced", DummyValue, "")

MaxDirectoryRecursion = s:taboption("tab_advanced", Value, "MaxDirectoryRecursion", translate("Max directory scan depth"))
MaxDirectoryRecursion:value("15",  translate("15"))
MaxDirectoryRecursion:value("20",  translate("20"))
MaxDirectoryRecursion.default = "15"

FollowDirectorySymlink = s:taboption("tab_advanced", ListValue, "FollowDirectorySymlink", translate("Follow directory symlinks"))
FollowDirectorySymlink:value("no",  translate("No"))
FollowDirectorySymlink:value("yes",  translate("Yes"))
FollowDirectorySymlink.default = "no"

FollowFileSymlinks = s:taboption("tab_advanced", ListValue, "FollowFileSymlinks", translate("Follow file symlinks"))
FollowFileSymlinks:value("no",  translate("No"))
FollowFileSymlinks:value("yes",  translate("Yes"))
FollowFileSymlinks.default = "no"

DetectPUA = s:taboption("tab_advanced", ListValue, "DetectPUA", translate("Detect possibly unwanted apps"))
DetectPUA:value("no",  translate("No"))
DetectPUA:value("yes",  translate("Yes"))
DetectPUA.default = "no"

ScanPE = s:taboption("tab_advanced", ListValue, "ScanPE", translate("Scan portable executables"))
ScanPE:value("no",  translate("No"))
ScanPE:value("yes",  translate("Yes"))
ScanPE.default = "yes"

ScanELF = s:taboption("tab_advanced", ListValue, "ScanELF", translate("Scan ELF files"))
ScanELF:value("no",  translate("No"))
ScanELF:value("yes",  translate("Yes"))
ScanELF.default = "yes"

DetectBrokenExecutables = s:taboption("tab_advanced", ListValue, "DetectBrokenExecutables", translate("Detect broken executables"))
DetectBrokenExecutables:value("no",  translate("No"))
DetectBrokenExecutables:value("yes",  translate("Yes"))
DetectBrokenExecutables.default = "no"

ScanOLE2 = s:taboption("tab_advanced", ListValue, "ScanOLE2", translate("Scan MS Office and .msi files"))
ScanOLE2:value("no",  translate("No"))
ScanOLE2:value("yes",  translate("Yes"))
ScanOLE2.default = "yes"

ScanPDF = s:taboption("tab_advanced", ListValue, "ScanPDF", translate("Scan pdf files"))
ScanPDF:value("no",  translate("No"))
ScanPDF:value("yes",  translate("Yes"))
ScanPDF.default = "yes"

ScanSWF = s:taboption("tab_advanced", ListValue, "ScanSWF", translate("Scan swf files"))
ScanSWF:value("no",  translate("No"))
ScanSWF:value("yes",  translate("Yes"))
ScanSWF.default = "yes"

ScanMail = s:taboption("tab_advanced", ListValue, "ScanMail", translate("Scan emails"))
ScanMail:value("no",  translate("No"))
ScanMail:value("yes",  translate("Yes"))
ScanMail.default = "yes"

ScanPartialMessages = s:taboption("tab_advanced", ListValue, "ScanPartialMessages", translate("Scan RFC1341 messages split over many emails"))
ScanPartialMessages:value("no",  translate("No"))
ScanPartialMessages:value("yes",  translate("Yes"))
ScanPartialMessages.default = "no"

ScanArchive = s:taboption("tab_advanced", ListValue, "ScanArchive", translate("Scan archives"))
ScanArchive:value("no",  translate("No"))
ScanArchive:value("yes",  translate("Yes"))
ScanArchive.default = "yes"

ArchiveBlockEncrypted = s:taboption("tab_advanced", ListValue, "ArchiveBlockEncrypted", translate("Block encrypted archives"))
ArchiveBlockEncrypted:value("no",  translate("No"))
ArchiveBlockEncrypted:value("yes",  translate("Yes"))
ArchiveBlockEncrypted.default = "no"

dummy5 = s:taboption("tab_advanced", DummyValue, "")
dummy6 = s:taboption("tab_advanced", DummyValue, "")

StreamMinPort = s:taboption("tab_advanced", Value, "StreamMinPort", translate("Port range, lowest port"))
StreamMinPort.datatype = "portrange"
StreamMinPort:value("1024",translate("1024"))
StreamMinPort.default = "1024"

StreamMaxPort = s:taboption("tab_advanced", Value, "StreamMaxPort", translate("Port range, highest port"))
StreamMaxPort.datatype = "portrange"
StreamMaxPort:value("2048",translate("2048"))
StreamMaxPort.default = "2048"

MaxThreads = s:taboption("tab_advanced", Value, "MaxThreads", translate("Max number of threads"))
MaxThreads.datatype = "and(uinteger,min(1))"
MaxThreads:value("10",translate("10"))
MaxThreads:value("20",translate("20"))
MaxThreads.default = "10"

SelfCheck = s:taboption("tab_advanced", Value, "SelfCheck", translate("Database check every N sec"))
SelfCheck.datatype = "and(uinteger,min(1))"
SelfCheck:value("600",translate("600"))
SelfCheck.default = "600"

MaxFileSize = s:taboption("tab_advanced", Value, "MaxFileSize", translate("Max size of scanned file"))
MaxFileSize.datatype = "string"
MaxFileSize:value("150M",translate("150M"))
MaxFileSize:value("50M",translate("50M"))
MaxFileSize.default = "150M"

------------------ Log --------------------

clamav_logfile = s:taboption("tab_logs", TextValue, "lines", "")
clamav_logfile.wrap = "off"
clamav_logfile.rows = 25
clamav_logfile.rmempty = true

function clamav_logfile.cfgvalue()
	local uci = require "luci.model.uci".cursor_state()
	local file = "/tmp/clamd.log"
	if file then
		return fs.readfile(file) or ""
	else
		return ""
	end
end

function clamav_logfile.write()
end

return m
