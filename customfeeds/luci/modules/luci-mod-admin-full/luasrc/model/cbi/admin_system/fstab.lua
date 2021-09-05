-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

require("luci.tools.webadmin")

local fs   = require "nixio.fs"
local util = require "nixio.util"
local tp   = require "luci.template.parser"

local block = io.popen("block info", "r")
local ln, dev, devices = nil, nil, {}

repeat
	ln = block:read("*l")
	dev = ln and ln:match("^/dev/(.-):")

	if dev then
		local e, s, key, val = { }

		for key, val in ln:gmatch([[(%w+)="(.-)"]]) do
			e[key:lower()] = val
			devices[val] = e
		end

		s = tonumber((fs.readfile("/sys/class/block/%s/size" % dev)))

		e.dev  = "/dev/%s" % dev
		e.size = s and math.floor(s / 2048)

		devices[e.dev] = e
	end
until not ln

block:close()

m = Map("fstab", translate("Mount Points"))
s = m:section(TypedSection, "global", translate("Global Settings"))
s.addremove = false
s.anonymous = true

detect = s:option(Button, "block_detect", translate("Generate Config"), translate("Find all currently attached filesystems and swap and replace configuration with defaults based on what was detected"))
detect.inputstyle = "reload"

detect.write = function(self, section)
	luci.sys.call("block detect >/etc/config/fstab")
	luci.http.redirect(luci.dispatcher.build_url("admin/system", "fstab"))
end

o = s:option(Flag, "anon_swap", translate("Anonymous Swap"), translate("Mount swap not specifically configured"))
o.default = o.disabled
o.rmempty = false

o = s:option(Flag, "anon_mount", translate("Anonymous Mount"), translate("Mount filesystems not specifically configured"))
o.default = o.disabled
o.rmempty = false

o = s:option(Flag, "auto_swap", translate("Automount Swap"), translate("Automatically mount swap on hotplug"))
o.default = o.enabled
o.rmempty = false

o = s:option(Flag, "auto_mount", translate("Automount Filesystem"), translate("Automatically mount filesystems on hotplug"))
o.default = o.enabled
o.rmempty = false

o = s:option(Flag, "check_fs", translate("Check filesystems before mount"), translate("Automatically check filesystem for errors before mounting"))
o.default = o.disabled
o.rmempty = false

local mounts = luci.sys.mounts()
local non_system_mounts = {}
for rawmount, val in pairs(mounts) do
    if (string.find(val.mountpoint, "/tmp/.jail") == nil) then
      repeat 
          val.umount = false
          if (val.mountpoint == "/") then
              break
          elseif (val.mountpoint == "/overlay") then
              break
          elseif (val.mountpoint == "/rom") then
              break
          elseif (val.mountpoint == "/tmp") then
              break
          elseif (val.mountpoint == "/tmp/shm") then
              break
          elseif (val.mountpoint == "/tmp/upgrade") then
              break
          elseif (val.mountpoint == "/dev") then
              break
          end
          val.umount = true
      until true
      non_system_mounts[rawmount] = val       
   end   
end

v = m:section(Table, non_system_mounts, translate("Mounted file systems"))

fs = v:option(DummyValue, "fs", translate("Filesystem"))

mp = v:option(DummyValue, "mountpoint", translate("Mount Point"))

avail = v:option(DummyValue, "avail", translate("Available"))
function avail.cfgvalue(self, section)
	return luci.tools.webadmin.byte_format(
		( tonumber(mounts[section].available) or 0 ) * 1024
	) .. " / " .. luci.tools.webadmin.byte_format(
		( tonumber(mounts[section].blocks) or 0 ) * 1024
	)
end

used = v:option(DummyValue, "used", translate("Used"))
function used.cfgvalue(self, section)
	return ( mounts[section].percent or "0%" ) .. " (" ..
	luci.tools.webadmin.byte_format(
		( tonumber(mounts[section].used) or 0 ) * 1024
	) .. ")"
end

unmount = v:option(Button, "unmount", translate("Unmount"))
unmount.render = function(self, section, scope)
	if non_system_mounts[section].umount then
		self.title = translate("Unmount")
		self.inputstyle = "remove"
        	Button.render(self, section, scope)
	end
end

unmount.write = function(self, section)
	if non_system_mounts[section].umount then
		luci.sys.call("/bin/umount '%s'" % luci.util.shellstartsqescape(non_system_mounts[section].mountpoint))
		return luci.http.redirect(luci.dispatcher.build_url("admin/system", "fstab"))
        end
end

mount = m:section(TypedSection, "mount", translate("Mount Points"), translate("Mount Points define at which point a memory device will be attached to the filesystem"))
mount.anonymous = true
mount.addremove = true
mount.template = "cbi/tblsection"
mount.extedit  = luci.dispatcher.build_url("admin/system/fstab/mount/%s")

mount.create = function(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(mount.extedit % sid)
		return
	end
end


mount:option(Flag, "enabled", translate("Enabled")).rmempty = false

dev = mount:option(DummyValue, "device", translate("Device"))
dev.rawhtml = true
dev.cfgvalue = function(self, section)
	local v, e

	v = m.uci:get("fstab", section, "uuid")
	e = v and devices[v:lower()]
	if v and e and e.size then
		return "UUID: %s (%s, %d MB)" %{ tp.pcdata(v), e.dev, e.size }
	elseif v and e then
		return "UUID: %s (%s)" %{ tp.pcdata(v), e.dev }
	elseif v then
		return "UUID: %s (<em>%s</em>)" %{ tp.pcdata(v), translate("not present") }
	end

	v = m.uci:get("fstab", section, "label")
	e = v and devices[v]
	if v and e and e.size then
		return "Label: %s (%s, %d MB)" %{ tp.pcdata(v), e.dev, e.size }
	elseif v and e then
		return "Label: %s (%s)" %{ tp.pcdata(v), e.dev }
	elseif v then
		return "Label: %s (<em>%s</em>)" %{ tp.pcdata(v), translate("not present") }
	end

	v = Value.cfgvalue(self, section) or "?"
	e = v and devices[v]
	if v and e and e.size then
		return "%s (%d MB)" %{ tp.pcdata(v), e.size }
	elseif v and e then
		return tp.pcdata(v)
	elseif v then
		return "%s (<em>%s</em>)" %{ tp.pcdata(v), translate("not present") }
	end
end

mp = mount:option(DummyValue, "target", translate("Mount Point"))
mp.cfgvalue = function(self, section)
	if m.uci:get("fstab", section, "is_rootfs") == "1" then
		return "/overlay"
	else
		return Value.cfgvalue(self, section) or "?"
	end
end

fs = mount:option(DummyValue, "fstype", translate("Filesystem"))
fs.cfgvalue = function(self, section)
	local v, e

	v = m.uci:get("fstab", section, "uuid")
	v = v and v:lower() or m.uci:get("fstab", section, "label")
	v = v or m.uci:get("fstab", section, "device")

	e = v and devices[v]

	return e and e.type or m.uci:get("fstab", section, "fstype") or "?"
end

op = mount:option(DummyValue, "options", translate("Options"))
op.cfgvalue = function(self, section)
	return Value.cfgvalue(self, section) or "defaults"
end

rf = mount:option(DummyValue, "is_rootfs", translate("Root"))
rf.cfgvalue = function(self, section)
	local target = m.uci:get("fstab", section, "target")
	if target == "/" then
		return translate("yes")
	elseif target == "/overlay" then
		return translate("overlay")
	else
		return translate("no")
	end
end

ck = mount:option(DummyValue, "enabled_fsck", translate("Check"))
ck.cfgvalue = function(self, section)
	return Value.cfgvalue(self, section) == "1"
		and translate("yes") or translate("no")
end


swap = m:section(TypedSection, "swap", "SWAP", translate("If your physical memory is insufficient unused data can be temporarily swapped to a swap-device resulting in a higher amount of usable <abbr title=\"Random Access Memory\">RAM</abbr>. Be aware that swapping data is a very slow process as the swap-device cannot be accessed with the high datarates of the <abbr title=\"Random Access Memory\">RAM</abbr>."))
swap.anonymous = true
swap.addremove = true
swap.template = "cbi/tblsection"
swap.extedit  = luci.dispatcher.build_url("admin/system/fstab/swap/%s")

swap.create = function(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(swap.extedit % sid)
		return
	end
end


swap:option(Flag, "enabled", translate("Enabled")).rmempty = false

dev = swap:option(DummyValue, "device", translate("Device"))
dev.cfgvalue = function(self, section)
	local v

	v = m.uci:get("fstab", section, "uuid")
	if v then return "UUID: %s" % v end

	v = m.uci:get("fstab", section, "label")
	if v then return "Label: %s" % v end

	v = Value.cfgvalue(self, section) or "?"
	e = v and devices[v]
	if v and e and e.size then
		return "%s (%s MB)" % {v, e.size}
	else
		return v
	end
end

return m
