-- Licensed to the public under the Apache License 2.0.

m = Map("samba4", translate("Network Shares"))

s = m:section(TypedSection, "samba", "Samba")
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

macos = s:taboption("general", Flag, "macos", translate("Enable macOS compatible shares"))
macos.description = translate("Enables Apple's AAPL extension globally and adds macOS compatibility options to all shares.")
macos.rmempty = false

if nixio.fs.access("/usr/sbin/nmbd") then
	s:taboption("general", Flag, "disable_netbios", translate("Disable Netbios"))
end
if nixio.fs.access("/usr/sbin/samba") then
	s:taboption("general", Flag, "disable_ad_dc", translate("Disable Active Directory Domain Controller"))
end
if nixio.fs.access("/usr/sbin/winbindd") then
	s:taboption("general", Flag, "disable_winbind", translate("Disable Winbind"))
end

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
	nixio.fs.writefile("/etc/samba/smb.conf.template", value)
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

br = s:option(Flag, "browseable", translate("Browse-able"))
br.enabled = "yes"
br.disabled = "no"
br.default = "yes"

ro = s:option(Flag, "read_only", translate("Read-only"))
ro.enabled = "yes"
ro.disabled = "no"
ro.default = "yes"

s:option(Flag, "force_root", translate("Force Root"))

au = s:option(Value, "users", translate("Allowed users"))
au.rmempty = true

go = s:option(Flag, "guest_ok", translate("Allow guests"))
go.enabled = "yes"
go.disabled = "no"
go.default = "no"

gon = s:option(Flag, "guest_only", translate("Guests only"))
gon.enabled = "yes"
gon.disabled = "no"
gon.default = "no"

iown = s:option(Flag, "inherit_owner", translate("Inherit owner"))
iown.enabled = "yes"
iown.disabled = "no"
iown.default = "no"

cm = s:option(Value, "create_mask", translate("Create mask"))
cm.rmempty = true
cm.maxlength = 4
cm.placeholder = "0666"

dm = s:option(Value, "dir_mask", translate("Directory mask"))
dm.rmempty = true
dm.maxlength = 4
dm.placeholder = "0777"

vfs = s:option(Value, "vfs_objects", translate("Vfs objects"))
vfs.rmempty = true

s:option(Flag, "timemachine", translate("Apple Time-machine share"))

tms = s:option(Value, "timemachine_maxsize", translate("Time-machine size in GB"))
tms.rmempty = true
tms.maxlength = 5

return m
