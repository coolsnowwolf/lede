-- Licensed to the public under the Apache License 2.0.

m = Map("ksmbd", translate("Network Shares (CIFSD)"))

s = m:section(TypedSection, "globals", translate("CIFSD is an opensource In-kernel SMB1/2/3 server"))
s.anonymous = true

s:tab("general",  translate("General Settings"))
s:tab("template", translate("Edit Template"))

s:taboption("general", Value, "description", translate("Description"))

o = s:taboption("general", Value, "workgroup", translate("Workgroup"))
o.placeholder = 'WORKGROUP'

tmpl = s:taboption("template", Value, "_tmpl",
	translate("Edit the template that is used for generating the cifsd configuration."), 
	translate("This is the content of the file '/etc/ksmbd/smb.conf.template' from which your cifsd configuration will be generated. \
			Values enclosed by pipe symbols ('|') should not be changed. They get their values from the 'General Settings' tab."))

tmpl.template = "cbi/tvalue"
tmpl.rows = 20

function tmpl.cfgvalue(self, section)
	return nixio.fs.readfile("/etc/ksmbd/smb.conf.template")
end

function tmpl.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile("/etc/ksmbd/smb.conf.template", value)
end


s = m:section(TypedSection, "share", translate("Shared Directories")
  , translate("Please add directories to share. Each directory refers to a folder on a mounted device."))
s.anonymous = true
s.addremove = true
s.template = "cbi/tblsection"

s:option(Value, "name", translate("Name"))
pth = s:option(Value, "path", translate("Path"))
if nixio.fs.access("/etc/config/fstab") then
        pth.titleref = luci.dispatcher.build_url("admin", "system", "fstab")
end

br = s:option(Flag, "browseable", translate("Browseable"))
br.rmempty = false
br.default = "yes"
br.enabled = "yes"
br.disabled = "no"

ro = s:option(Flag, "read_only", translate("Read-only"))
ro.rmempty = false
ro.enabled = "yes"
ro.disabled = "no"

fr = s:option(Flag, "force_root", translate("Force Root"))
fr.rmempty = false
fr.default = "1"
fr.enabled = "1"
fr.disabled = "0"

-- s:option(Value, "users", translate("Allowed users")).rmempty = true

go = s:option(Flag, "guest_ok", translate("Allow guests"))
go.rmempty = false
go.enabled = "yes"
go.disabled = "no"
go.default = "yes"

io = s:option(Flag, "inherit_owner", translate("Inherit owner"))

hd = s:option(Flag, "hide_dot_files", translate("Hide dot files"))

cm = s:option(Value, "create_mask", translate("Create mask"),
        translate("Mask for new files"))
cm.rmempty = true
cm.size = 4
cm.default = "0666"

dm = s:option(Value, "dir_mask", translate("Directory mask"),
        translate("Mask for new directories"))
dm.rmempty = true
dm.size = 4
dm.default = "0777"

return m
