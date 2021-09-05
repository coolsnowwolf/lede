-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

cbimap = Map("asterisk", "asterisk", "")

iax = cbimap:section(TypedSection, "iax", "IAX Connection", "")
iax.addremove = true

alwaysinternational = iax:option(Flag, "alwaysinternational", "Always Dial International", "")
alwaysinternational.optional = true

context = iax:option(ListValue, "context", "Context to use", "")
context.titleref = luci.dispatcher.build_url( "admin", "services", "asterisk", "dialplans" )
cbimap.uci:foreach( "asterisk", "dialplan", function(s) context:value(s['.name']) end )
cbimap.uci:foreach( "asterisk", "dialzone", function(s) context:value(s['.name']) end )

countrycode = iax:option(Value, "countrycode", "Country Code for connection", "")
countrycode.optional = true

extension = iax:option(Value, "extension", "Add as Extension", "")
extension.optional = true

host = iax:option(Value, "host", "Host name (or blank)", "")
host.optional = true

internationalprefix = iax:option(Value, "internationalprefix", "International Dial Prefix", "")
internationalprefix.optional = true

prefix = iax:option(Value, "prefix", "Dial Prefix (for external line)", "")
prefix.optional = true

secret = iax:option(Value, "secret", "Secret", "")
secret.optional = true

timeout = iax:option(Value, "timeout", "Dial Timeout (sec)", "")
timeout.optional = true

type = iax:option(ListValue, "type", "Option type", "")
type:value("friend", "Friend (outbound/inbound)")
type:value("user", "User (inbound - authenticate by \"from\")")
type:value("peer", "Peer (outbound - match by host)")
type.optional = true

username = iax:option(Value, "username", "User name", "")
username.optional = true


return cbimap
