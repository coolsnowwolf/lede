-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

cbimap = Map("asterisk", "asterisk", "")


module = cbimap:section(TypedSection, "module", "Modules", "")
module.anonymous = true

chan_agent = module:option(ListValue, "chan_agent", "Agent Proxy Channel", "")
chan_agent:value("yes", "Load")
chan_agent:value("no", "Do Not Load")
chan_agent:value("auto", "Load as Required")
chan_agent.rmempty = true

chan_alsa = module:option(ListValue, "chan_alsa", "Channel driver for GTalk", "")
chan_alsa:value("yes", "Load")
chan_alsa:value("no", "Do Not Load")
chan_alsa:value("auto", "Load as Required")
chan_alsa.rmempty = true

chan_gtalk = module:option(ListValue, "chan_gtalk", "Channel driver for GTalk", "")
chan_gtalk:value("yes", "Load")
chan_gtalk:value("no", "Do Not Load")
chan_gtalk:value("auto", "Load as Required")
chan_gtalk.rmempty = true

chan_iax2 = module:option(Flag, "chan_iax2", "Option chan_iax2", "")
chan_iax2.rmempty = true

chan_local = module:option(ListValue, "chan_local", "Local Proxy Channel", "")
chan_local:value("yes", "Load")
chan_local:value("no", "Do Not Load")
chan_local:value("auto", "Load as Required")
chan_local.rmempty = true

chan_sip = module:option(ListValue, "chan_sip", "Session Initiation Protocol (SIP)", "")
chan_sip:value("yes", "Load")
chan_sip:value("no", "Do Not Load")
chan_sip:value("auto", "Load as Required")
chan_sip.rmempty = true


return cbimap
