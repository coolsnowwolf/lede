-- Copyright 2011 Manuel Munz <freifunk at somakoma de>
-- Licensed to the public under the Apache License 2.0.

m = Map("luci_olsr", translate("OLSR - Display Options"))

s = m:section(TypedSection, "olsr")
s.anonymous = true

res = s:option(Flag, "resolve", translate("Resolve"),
        translate("Resolve hostnames on status pages. It is generally safe to allow this, but if you use public IPs and have unstable DNS-Setup then those pages will load really slow. In this case disable it here."))
res.default = "0"
res.optional = true

return m
