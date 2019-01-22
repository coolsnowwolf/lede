
m = Map("cpulimit", translate("cpulimit"),translate("cpulimit  "))
s = m:section(TypedSection, "list", translate("Settings"))
s.template = "cbi/tblsection"
s.anonymous = true
s.addremove = true

enable = s:option(Flag, "enabled", translate("enable", "enable"))
enable.optional = false
enable.rmempty = false

exename = s:option(Value, "exename", translate("exename"), translate("name of the executable program file or path name"))
exename.optional = false
exename.rmempty = false
exename.default = "/usr/bin/transmission-daemon"
exename:value("transmission","/usr/bin/transmission-daemon")
exename:value("samba","/usr/sbin/smbd")
exename:value("mount.ntfs-3g","mount.ntfs-3g")
exename:value("vsftpd","/usr/sbin/vsftpd")
exename:value("pure-ftpd","/usr/sbin/pure-ftpd")

limit = s:option(Value, "limit", translate("limit"))
limit.optional = false
limit.rmempty = false
limit.default = "50"
limit:value("100","100%")
limit:value("90","90%")
limit:value("80","80%")
limit:value("70","70%")
limit:value("60","60%")
limit:value("50","50%")
limit:value("40","40%")
limit:value("30","30%")
limit:value("20","20%")
limit:value("10","10%")


return m
