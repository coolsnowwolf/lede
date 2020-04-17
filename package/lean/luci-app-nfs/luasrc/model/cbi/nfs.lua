local fs = require "nixio.fs"

m = Map("nfs", translate("NFS Manage"))

-- NFS Share --
s = m:section(TypedSection, "share", translate("Shared Directories"))
s.anonymous = true
s.addremove = true
s.template = "cbi/tblsection"

en = s:option(Flag, "enabled", translate("Enable"))
en.rmempty = false
en.default = "1"

ph1 = s:option(Value, "path", translate("Path"))
ph1.placeholder = "/mnt"
ph1.rmempty = false
ph1.optional = false

ct = s:option(Value, "clients", translate("Clients"))
ct.placeholder = "192.168.1.0/24"
ct.rmempty = false
ct.optional = false

op = s:option(Value, "options", translate("options"))
op.placeholder = "rw,sync,root_squash,all_squash,insecure,no_subtree_check"
op.rmempty = false
op.optional = false

-- NFS Mount --
c = m:section(TypedSection, "mount", translate("Mounted Points"))
c.anonymous = true
c.addremove = true
c.template = "cbi/tblsection"

en = c:option(Flag, "enabled", translate("Enable"))
en.default = "1"
en.rmempty = false

sr = c:option(Value, "source", translate("source"))
sr.placeholder = "192.168.1.1:/mnt/*"
sr.rmempty = false
sr.optional = false

ph2 = c:option(Value, "target", translate("target"))
ph2.placeholder = "/mnt/nfs/*"
ph2.rmempty = false
ph2.optional = false

op = c:option(Value, "options", translate("options"))
op.placeholder = "rw,nolock"
op.rmempty = false
op.optional = false

de = c:option(Value, "delay", translate("delay"))
de.placeholder = "5"
de.rmempty = false
de.optional = false

if nixio.fs.access("/etc/config/fstab") then
  ph1.titleref = luci.dispatcher.build_url("admin", "system", "mounts")
  ph2.titleref = luci.dispatcher.build_url("admin", "system", "mounts")
end

local apply = luci.http.formvalue("cbi.apply")
if apply then
    io.popen("/etc/init.d/nfs reload")
end

return m
