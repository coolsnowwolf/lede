m = Map("qbittorrent")

s = m:section(NamedSection, "main", "qbittorrent")

o = s:option(Flag, "UPnP", translate("Use UPnP for Connections"))
o.description = translate("Use UPnP/ NAT-PMP port forwarding from my router. Refer to the <a href=\"https://en.wikipedia.org/wiki/Port_forwarding\" target=\"_blank\">wiki</a>")
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:option(Value, "GlobalDLLimit", translate("Global Download Speed"))
o.description = translate("Global Download Speed Limit(KiB/s).")
o.datatype = "float"
o.placeholder = "0"

o = s:option(Value, "GlobalUPLimit", translate("Global Upload Speed"))
o.description = translate("Global Upload Speed Limit(KiB/s).")
o.datatype = "float"
o.placeholder = "0"

o = s:option(Value, "GlobalDLLimitAlt", translate("Alternative Download Speed"))
o.description = translate("Alternative Download Speed Limit(KiB/s).")
o.datatype = "float"
o.placeholder = "10"

o = s:option(Value, "GlobalUPLimitAlt", translate("Alternative Upload Speed"))
o.description = translate("Alternative Upload Speed Limit(KiB/s).")
o.datatype = "float"
o.placeholder = "10"

o = s:option(ListValue, "BTProtocol", translate("Enabled protocol"))
o.description = translate("The protocol that was enabled.")
o:value("Both", translate("TCP and UTP"))
o:value("TCP", translate("TCP"))
o:value("UTP", translate("UTP"))
o.default = "Both"

o = s:option(Value, "InetAddress", translate("Inet Address"))
o.description = translate("The address that respond to the trackers.")

return m
