local fs = require "nixio.fs"

m = Map("cifs")
m.title = translate("Mount SMB/CIFS Netshare")
m.description = translate("Mount SMB/CIFS Netshare for OpenWrt")

s = m:section(TypedSection, "cifs")
s.anonymous = true

switch = s:option(Flag, "enabled", translate("Enable"))
switch.rmempty = false

workgroup = s:option(Value, "workgroup", translate("Workgroup"))
workgroup.default = "WORKGROUP"

s = m:section(TypedSection, "natshare", translate("CIFS/SMB Netshare"))
s.anonymous = true
s.addremove = true
s.template = "cbi/tblsection"

server = s:option(Value, "server", translate("Server IP"))
server.size = 12
server.rmempty = false

name = s:option(Value, "name", translate("Share Folder"))
name.rmempty = false
name.size = 8

pth = s:option(Value, "natpath", translate("Mount Path"))
if nixio.fs.access("/etc/config/fstab") then
        pth.titleref = luci.dispatcher.build_url("admin", "system", "fstab")
end
pth.rmempty = false
pth.size = 10

smbver = s:option(Value, "smbver", translate("SMB Version"))
smbver.rmempty = false
smbver:value("1.0","SMB v1")
smbver:value("2.0","SMB v2")
smbver:value("3.0","SMB v3")
smbver.default = "2.0"
smbver.size = 3

agm = s:option(Value, "agm", translate("Arguments"))
agm:value("ro", translate("Read Only"))
agm:value("rw", translate("Read/Write"))
agm.rmempty = true
agm.default = "ro"

iocharset = s:option(Value, "iocharset", translate("Charset"))
iocharset:value("utf8", "UTF8")
iocharset.default = "utf8"
iocharset.size = 2

users = s:option(Value, "users", translate("User"))
users:value("guest", "Guest")
users.rmempty = true
users.default = "guest"

pwd = s:option(Value, "pwd", translate("Password"))
pwd.rmempty = true
pwd.size = 8

return m
