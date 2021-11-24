m = Map("qbittorrent")

s = m:section(NamedSection, "main", "qbittorrent")

o = s:option(Flag, "UseUPnP", translate("Use UPnP for WebUI"))
o.description = translate("Using the UPnP / NAT-PMP port of the router for connecting to WebUI.")
o.enabled = "true"
o.disabled = "false"
o.default = o.disabled

--o = s:option(Value, "Username", translate("Username"))
--o.description = translate("The login name for WebUI.")
--o.placeholder = "admin"

--o = s:option(Value, "Password", translate("Password"))
--o.description = translate("The login password for WebUI.")
--o.password  =  true

o = s:option(Value, "Locale", translate("Locale Language"))
o:value("en", translate("English"))
o:value("zh", translate("Chinese"))
o.default = "en"

o = s:option(Flag, "CSRFProtection", translate("CSRF Protection"))
o.description = translate("Enable Cross-Site Request Forgery (CSRF) protection.")
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:option(Flag, "ClickjackingProtection", translate("Clickjacking Protection"))
o.description = translate("Enable clickjacking protection.")
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:option(Flag, "HostHeaderValidation", translate("Host Header Validation"))
o.description = translate("Validate the host header.")
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:option(Flag, "LocalHostAuth", translate("Local Host Authentication"))
o.description = translate("Force authentication for clients on localhost.")
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:option(Flag, "AuthSubnetWhitelistEnabled", translate("Enable Subnet Whitelist"))
o.enabled = "true"
o.disabled = "false"
o.default = o.disabled

o = s:option(DynamicList, "AuthSubnetWhitelist", translate("Subnet Whitelist"))
o:depends("AuthSubnetWhitelistEnabled", "true")

return m
