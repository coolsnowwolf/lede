-- Copyright 2012 Gabor Juhos <juhosg@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local m, s, o

m = Map("minidlna", translate("miniDLNA"),
	translate("MiniDLNA is server software with the aim of being fully compliant with DLNA/UPnP-AV clients."))

m:section(SimpleSection).template  = "minidlna_status"

s = m:section(TypedSection, "minidlna", "miniDLNA Settings")
s.addremove = false
s.anonymous = true

s:tab("general", translate("General Settings"))
s:tab("advanced", translate("Advanced Settings"))

o = s:taboption("general", Flag, "enabled", translate("Enable:"))
o.rmempty = false

function o.write(self, section, value)
	if value == "1" then
		luci.sys.init.enable("minidlna")
		luci.sys.call("/etc/init.d/minidlna start >/dev/null")
	else
		luci.sys.call("/etc/init.d/minidlna stop >/dev/null")
		luci.sys.init.disable("minidlna")
	end

	return Flag.write(self, section, value)
end

o = s:taboption("general", Value, "port", translate("Port:"),
	translate("Port for HTTP (descriptions, SOAP, media transfer) traffic."))
o.datatype = "port"
o.default = 8200


o = s:taboption("general", Value, "interface", translate("Interfaces:"),
	translate("Network interfaces to serve."))

o.template = "cbi/network_ifacelist"
o.widget   = "checkbox"
o.nocreate = true

function o.cfgvalue(self, section)
	local rv = { }
	local val = Value.cfgvalue(self, section)
	if val then
		local ifc
		for ifc in val:gmatch("[^,%s]+") do
			rv[#rv+1] = ifc
		end
	end
	return rv
end

function o.write(self, section, value)
	local rv = { }
	local ifc
	for ifc in luci.util.imatch(value) do
		rv[#rv+1] = ifc
	end
	Value.write(self, section, table.concat(rv, ","))
end


o = s:taboption("general", Value, "friendly_name", translate("Friendly name:"),
	translate("Set this if you want to customize the name that shows up on your clients."))
o.rmempty = true
o.placeholder = "OpenWrt DLNA Server"

o = s:taboption("advanced", Value, "db_dir", translate("Database directory:"),
	translate("Set this if you would like to specify the directory where you want MiniDLNA to store its database and album art cache."))
o.rmempty = true
o.placeholder = "/var/cache/minidlna"

o = s:taboption("advanced", Value, "log_dir", translate("Log directory:"),
	translate("Set this if you would like to specify the directory where you want MiniDLNA to store its log file."))
o.rmempty = true
o.placeholder = "/var/log"

s:taboption("advanced", Flag, "inotify", translate("Enable inotify:"),
	translate("Set this to enable inotify monitoring to automatically discover new files."))

s:taboption("advanced", Flag, "enable_tivo", translate("Enable TIVO:"),
	translate("Set this to enable support for streaming .jpg and .mp3 files to a TiVo supporting HMO."))
o.rmempty = true

s:taboption("advanced", Flag, "wide_links", translate("Allow wide links:"),
	translate("Set this to allow serving content outside the media root (via symlinks)."))
o.rmempty = true

o = s:taboption("advanced", Flag, "strict_dlna", translate("Strict to DLNA standard:"),
	translate("Set this to strictly adhere to DLNA standards. This will allow server-side downscaling of very large JPEG images, which may hurt JPEG serving performance on (at least) Sony DLNA products."))
o.rmempty = true

o = s:taboption("advanced", Value, "presentation_url", translate("Presentation URL:"))
o.rmempty = true
o.placeholder = "http://192.168.1.1/"

o = s:taboption("advanced", Value, "notify_interval", translate("Notify interval:"),
	translate("Notify interval in seconds."))
o.datatype = "uinteger"
o.placeholder = 900

o = s:taboption("advanced", Value, "serial", translate("Announced serial number:"),
	translate("Serial number the miniDLNA daemon will report to clients in its XML description."))
o.placeholder = "12345678"

s:taboption("advanced", Value, "model_number", translate("Announced model number:"),
	translate("Model number the miniDLNA daemon will report to clients in its XML description."))
o.placholder = "1"

o = s:taboption("advanced", Value, "minissdpsocket", translate("miniSSDP socket:"),
	translate("Specify the path to the MiniSSDPd socket."))
o.rmempty = true
o.placeholder = "/var/run/minissdpd.sock"

o = s:taboption("general", ListValue, "root_container", translate("Root container:"))
o:value(".", translate("Standard container"))
o:value("B", translate("Browse directory"))
o:value("M", translate("Music"))
o:value("V", translate("Video"))
o:value("P", translate("Pictures"))


s:taboption("general", DynamicList, "media_dir", translate("Media directories:"),
	translate("Set this to the directory you want scanned. If you want to restrict the directory to a specific content type, you can prepend the type ('A' for audio, 'V' for video, 'P' for images), followed by a comma, to the directory (eg. A,/mnt/media/Music). Multiple directories can be specified."))


o = s:taboption("general", DynamicList, "album_art_names", translate("Album art names:"),
	translate("This is a list of file names to check for when searching for album art."))
o.rmempty = true
o.placeholder = "Cover.jpg"

function o.cfgvalue(self, section)
	local rv = { }

	local val = Value.cfgvalue(self, section)
	if type(val) == "table" then
		val = table.concat(val, "/")
	elseif not val then
		val = ""
	end

	local file
	for file in val:gmatch("[^/%s]+") do
		rv[#rv+1] = file
	end

	return rv
end

function o.write(self, section, value)
	local rv = { }
	local file
	for file in luci.util.imatch(value) do
		rv[#rv+1] = file
	end
	Value.write(self, section, table.concat(rv, "/"))
end


return m
