-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

m = Map("coovachilli")

-- uam config
s1 = m:section(TypedSection, "uam")
s1.anonymous = true

s1:option( Value, "uamserver" )
s1:option( Value, "uamsecret" ).password = true

s1:option( Flag, "uamanydns" )
s1:option( Flag, "nouamsuccess" )
s1:option( Flag, "nouamwispr" )
s1:option( Flag, "chillixml" )
s1:option( Flag, "uamanyip" ).optional = true
s1:option( Flag, "dnsparanoia" ).optional = true
s1:option( Flag, "usestatusfile" ).optional = true

s1:option( Value, "uamhomepage" ).optional = true
s1:option( Value, "uamlisten" ).optional = true
s1:option( Value, "uamport" ).optional = true
s1:option( Value, "uamiport" ).optional = true
s1:option( DynamicList, "uamdomain" ).optional = true
s1:option( Value, "uamlogoutip" ).optional = true
s1:option( DynamicList, "uamallowed" ).optional = true
s1:option( Value, "uamui" ).optional = true

s1:option( Value, "wisprlogin" ).optional = true

s1:option( Value, "defsessiontimeout" ).optional = true
s1:option( Value, "defidletimeout" ).optional = true
s1:option( Value, "definteriminterval" ).optional = true

s1:option( Value, "ssid" ).optional = true
s1:option( Value, "vlan" ).optional = true
s1:option( Value, "nasip" ).optional = true
s1:option( Value, "nasmac" ).optional = true
s1:option( Value, "wwwdir" ).optional = true
s1:option( Value, "wwwbin" ).optional = true

s1:option( Value, "localusers" ).optional = true
s1:option( Value, "postauthproxy" ).optional = true
s1:option( Value, "postauthproxyport" ).optional = true
s1:option( Value, "locationname" ).optional = true


-- mac authentication
s2 = m:section(TypedSection, "macauth")
s2.anonymous = true

s2:option( Flag, "macauth" )
s2:option( Flag, "macallowlocal" )
s2:option( DynamicList, "macallowed" )

pw = s2:option( Value, "macpasswd" )
pw.optional = true
pw.password = true

s2:option( Value, "macsuffix" ).optional = true

return m
