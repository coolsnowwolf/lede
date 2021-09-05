-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

m = Map("coovachilli")

-- radius server
s1 = m:section(TypedSection, "radius")
s1.anonymous = true

s1:option( Value, "radiusserver1" )
s1:option( Value, "radiusserver2" )
s1:option( Value, "radiussecret" ).password = true

s1:option( Value, "radiuslisten" ).optional = true
s1:option( Value, "radiusauthport" ).optional = true
s1:option( Value, "radiusacctport" ).optional = true

s1:option( Value, "radiusnasid" ).optional = true
s1:option( Value, "radiusnasip" ).optional = true

s1:option( Value, "radiuscalled" ).optional = true
s1:option( Value, "radiuslocationid" ).optional = true
s1:option( Value, "radiuslocationname" ).optional = true

s1:option( Value, "radiusnasporttype" ).optional = true

s1:option( Flag, "radiusoriginalurl" )

s1:option( Value, "adminuser" ).optional = true
rs = s1:option( Value, "adminpassword" )
rs.optional = true
rs.password = true

s1:option( Flag, "swapoctets" )
s1:option( Flag, "openidauth" )
s1:option( Flag, "wpaguests" )
s1:option( Flag, "acctupdate" )

s1:option( Value, "coaport" ).optional = true
s1:option( Flag, "coanoipcheck" )


-- radius proxy
s2 = m:section(TypedSection, "proxy")
s2.anonymous = true

s2:option( Value, "proxylisten" ).optional = true
s2:option( Value, "proxyport" ).optional = true
s2:option( Value, "proxyclient" ).optional = true
ps = s2:option( Value, "proxysecret" )
ps.optional = true
ps.password = true

return m
