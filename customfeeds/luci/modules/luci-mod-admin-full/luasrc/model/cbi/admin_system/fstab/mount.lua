-- Copyright 2010 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local fs   = require "nixio.fs"
local util = require "nixio.util"

local has_fscheck = fs.access("/usr/sbin/e2fsck")

local block = io.popen("block info", "r")
local ln, dev, devices = nil, nil, {}

repeat
	ln = block:read("*l")
	dev = ln and ln:match("^/dev/(.-):")

	if dev then
		local e, s, key, val = { }

		for key, val in ln:gmatch([[(%w+)="(.-)"]]) do
			e[key:lower()] = val
		end

		s = tonumber((fs.readfile("/sys/class/block/%s/size" % dev)))

		e.dev  = "/dev/%s" % dev
		e.size = s and math.floor(s / 2048)

		devices[#devices+1] = e
	end
until not ln

block:close()


m = Map("fstab", translate("Mount Points - Mount Entry"))
m.redirect = luci.dispatcher.build_url("admin/system/fstab")

if not arg[1] or m.uci:get("fstab", arg[1]) ~= "mount" then
	luci.http.redirect(m.redirect)
	return
end



mount = m:section(NamedSection, arg[1], "mount", translate("Mount Entry"))
mount.anonymous = true
mount.addremove = false

mount:tab("general",  translate("General Settings"))
mount:tab("advanced", translate("Advanced Settings"))


mount:taboption("general", Flag, "enabled", translate("Enable this mount")).rmempty = false


o = mount:taboption("general", Value, "uuid", translate("UUID"),
	translate("If specified, mount the device by its UUID instead of a fixed device node"))

o:value("", translate("-- match by uuid --"))

for i, d in ipairs(devices) do
	if d.uuid and d.size then
		o:value(d.uuid, "%s (%s, %d MB)" %{ d.uuid, d.dev, d.size })
	elseif d.uuid then
		o:value(d.uuid, "%s (%s)" %{ d.uuid, d.dev })
	end
end


o = mount:taboption("general", Value, "label", translate("Label"),
	translate("If specified, mount the device by the partition label instead of a fixed device node"))

o:value("", translate("-- match by label --"))

o:depends("uuid", "")

for i, d in ipairs(devices) do
	if d.label and d.size then
		o:value(d.label, "%s (%s, %d MB)" %{ d.label, d.dev, d.size })
	elseif d.label then
		o:value(d.label, "%s (%s)" %{ d.label, d.dev })
	end
end


o = mount:taboption("general", Value, "device", translate("Device"),
	translate("The device file of the memory or partition (<abbr title=\"for example\">e.g.</abbr> <code>/dev/sda1</code>)"))

o:value("", translate("-- match by device --"))

o:depends({ uuid = "", label = "" })

for i, d in ipairs(devices) do
	if d.size then
		o:value(d.dev, "%s (%d MB)" %{ d.dev, d.size })
	else
		o:value(d.dev)
	end
end


o = mount:taboption("general", Value, "target", translate("Mount point"),
	translate("Specifies the directory the device is attached to"))

o:value("/opt", translate("Use as Docker data (/opt)"))
o:value("/", translate("Use as root filesystem (/)"))
o:value("/overlay", translate("Use as external overlay (/overlay)"))


o = mount:taboption("general", DummyValue, "__notice", translate("Root preparation"))
o:depends("target", "/")
o.rawhtml = true
o.default = [[
<p>%s</p><pre>mkdir -p /tmp/introot
mkdir -p /tmp/extroot
mount --bind / /tmp/introot
mount /dev/sda1 /tmp/extroot
tar -C /tmp/introot -cvf - . | tar -C /tmp/extroot -xf -
umount /tmp/introot
umount /tmp/extroot</pre>
]] %{
	translate("Make sure to clone the root filesystem using something like the commands below:"),

}


o = mount:taboption("advanced", Value, "fstype", translate("Filesystem"),
	translate("The filesystem that was used to format the memory (<abbr title=\"for example\">e.g.</abbr> <samp><abbr title=\"Third Extended Filesystem\">ext3</abbr></samp>)"))

o:value("", "auto")

local fs
for fs in io.lines("/proc/filesystems") do
	fs = fs:match("%S+")
	if fs ~= "nodev" then
		o:value(fs)
	end
end


o = mount:taboption("advanced", Value, "options", translate("Mount options"),
	translate("See \"mount\" manpage for details"))

o.placeholder = "defaults"


if has_fscheck then
	o = mount:taboption("advanced", Flag, "enabled_fsck", translate("Run filesystem check"),
		translate("Run a filesystem check before mounting the device"))
end

return m
