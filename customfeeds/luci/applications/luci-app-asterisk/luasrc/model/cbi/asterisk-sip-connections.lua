-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

cbimap = Map("asterisk", "asterisk", "")

sip = cbimap:section(TypedSection, "sip", "SIP Connection", "")
sip.addremove = true

alwaysinternational = sip:option(Flag, "alwaysinternational", "Always Dial International", "")
alwaysinternational.optional = true

canreinvite = sip:option(ListValue, "canreinvite", "Reinvite/redirect media connections", "")
canreinvite:value("yes", "Yes")
canreinvite:value("nonat", "Yes when not behind NAT")
canreinvite:value("update", "Use UPDATE rather than INVITE for path redirection")
canreinvite:value("no", "No")
canreinvite.optional = true

context = sip:option(ListValue, "context", "Context to use", "")
context.titleref = luci.dispatcher.build_url( "admin", "services", "asterisk", "dialplans" )
cbimap.uci:foreach( "asterisk", "dialplan", function(s) context:value(s['.name']) end )
cbimap.uci:foreach( "asterisk", "dialzone", function(s) context:value(s['.name']) end )

countrycode = sip:option(Value, "countrycode", "Country Code for connection", "")
countrycode.optional = true

dtmfmode = sip:option(ListValue, "dtmfmode", "DTMF mode", "")
dtmfmode:value("info", "Use RFC2833 or INFO for the BudgeTone")
dtmfmode:value("rfc2833", "Use RFC2833 for the BudgeTone")
dtmfmode:value("inband", "Use Inband (only with ulaw/alaw)")
dtmfmode.optional = true

extension = sip:option(Value, "extension", "Add as Extension", "")
extension.optional = true

fromdomain = sip:option(Value, "fromdomain", "Primary domain identity for From: headers", "")
fromdomain.optional = true

fromuser = sip:option(Value, "fromuser", "From user (required by many SIP providers)", "")
fromuser.optional = true

host = sip:option(Value, "host", "Host name (or blank)", "")
host.optional = true

incoming = sip:option(DynamicList, "incoming", "Ring on incoming dialplan contexts", "")
incoming.optional = true

insecure = sip:option(ListValue, "insecure", "Allow Insecure for", "")
insecure:value("port", "Allow mismatched port number")
insecure:value("invite", "Do not require auth of incoming INVITE")
insecure:value("port,invite", "Allow mismatched port and Do not require auth of incoming INVITE")
insecure.optional = true

internationalprefix = sip:option(Value, "internationalprefix", "International Dial Prefix", "")
internationalprefix.optional = true

mailbox = sip:option(Value, "mailbox", "Mailbox for MWI", "")
mailbox.optional = true

nat = sip:option(Flag, "nat", "NAT between phone and Asterisk", "")
nat.optional = true

pedantic = sip:option(Flag, "pedantic", "Check tags in headers", "")
pedantic.optional = true

port = sip:option(Value, "port", "SIP Port", "")
port.optional = true

prefix = sip:option(Value, "prefix", "Dial Prefix (for external line)", "")
prefix.optional = true

qualify = sip:option(Value, "qualify", "Reply Timeout (ms) for down connection", "")
qualify.optional = true

register = sip:option(Flag, "register", "Register connection", "")
register.optional = true

secret = sip:option(Value, "secret", "Secret", "")
secret.optional = true

selfmailbox = sip:option(Flag, "selfmailbox", "Dial own extension for mailbox", "")
selfmailbox.optional = true

timeout = sip:option(Value, "timeout", "Dial Timeout (sec)", "")
timeout.optional = true

type = sip:option(ListValue, "type", "Client Type", "")
type:value("friend", "Friend (outbound/inbound)")
type:value("user", "User (inbound - authenticate by \"from\")")
type:value("peer", "Peer (outbound - match by host)")
type.optional = true

username = sip:option(Value, "username", "Username", "")
username.optional = true


return cbimap
