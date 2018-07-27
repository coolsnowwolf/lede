-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

m = Map("samba4", translate("Network Shares"))

s = m:section(TypedSection, "samba", "Samba 4")
s.anonymous = true

s:tab("general",  translate("General Settings"))
s:tab("template", translate("Edit Template"))

s:taboption("general", Value, "name", translate("Hostname"))
s:taboption("general", Value, "description", translate("Description"))
s:taboption("general", Value, "workgroup", translate("Workgroup"))
h = s:taboption("general", Flag, "homes", translate("Share home-directories"),
        translate("Allow system users to reach their home directories via " ..
                "network shares"))
h.rmempty = false
s:taboption("general", Flag, "disable_netbios", translate("Disable netbios"))
s:taboption("general", Flag, "disable_ad_dc", translate("Disable AD-DC"))
s:taboption("general", Flag, "disable_winbind", translate("Disable Winbind"))

tmpl = s:taboption("template", Value, "_tmpl",
	translate("Edit the template that is used for generating the samba configuration."), 
	translate("This is the content of the file '/etc/samba/smb.conf.template' from which your samba configuration will be generated. " ..
		"Values enclosed by pipe symbols ('|') should not be changed. They get their values from the 'General Settings' tab."))

tmpl.template = "cbi/tvalue"
tmpl.rows = 20

function tmpl.cfgvalue(self, section)
	return nixio.fs.readfile("/etc/samba/smb.conf.template")
end

function tmpl.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile("//etc/samba/smb.conf.template", value)
end


s = m:section(TypedSection, "sambashare", translate("Shared Directories")
  , translate("Please add directories to share. Each directory refers to a folder on a mounted device."))
s.anonymous = true
s.addremove = true
s.template = "cbi/tblsection"

s:option(Value, "name", translate("Name"))
pth = s:option(Value, "path", translate("Path"))
if nixio.fs.access("/etc/config/fstab") then
        pth.titleref = luci.dispatcher.build_url("admin", "system", "fstab")
end

s:option(Value, "users", translate("Allowed users")).rmempty = true

ro = s:option(Flag, "read_only", translate("Read-only"))
ro.rmempty = false
ro.enabled = "yes"
ro.disabled = "no"

br = s:option(Flag, "browseable", translate("Browseable"))
br.rmempty = false
br.default = "yes"
br.enabled = "yes"
br.disabled = "no"

go = s:option(Flag, "guest_ok", translate("Allow Guests"))
go.rmempty = false
go.enabled = "yes"
go.disabled = "no"

gon = s:option(Flag, "guest_only", translate("Guests only"))
gon.rmempty = false
gon.enabled = "yes"
gon.disabled = "no"

io = s:option(Flag, "inherit_owner", translate("Inherit Owner"))
io.rmempty = false
io.enabled = "yes"
io.disabled = "no"

cm = s:option(Value, "create_mask", translate("Create Mask"))
cm.rmempty = true
cm.size = 4

dm = s:option(Value, "dir_mask", translate("Directory Mask"))
dm.rmempty = true
dm.size = 4

s:option(Value, "vfs_objects", translate("Vfs Objects")).rmempty = true

function m.on_commit(self,map)
	require("luci.sys").call('/sbin/reload_config')
end

return m
