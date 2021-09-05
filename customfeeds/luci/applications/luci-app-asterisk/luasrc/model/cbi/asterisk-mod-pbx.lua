-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

cbimap = Map("asterisk", "asterisk", "")

module = cbimap:section(TypedSection, "module", "Modules", "")
module.anonymous = true

pbx_ael = module:option(ListValue, "pbx_ael", "Asterisk Extension Language Compiler", "")
pbx_ael:value("yes", "Load")
pbx_ael:value("no", "Do Not Load")
pbx_ael:value("auto", "Load as Required")
pbx_ael.rmempty = true

pbx_config = module:option(ListValue, "pbx_config", "Text Extension Configuration", "")
pbx_config:value("yes", "Load")
pbx_config:value("no", "Do Not Load")
pbx_config:value("auto", "Load as Required")
pbx_config.rmempty = true

pbx_functions = module:option(ListValue, "pbx_functions", "load => .so ; Builtin dialplan functions", "")
pbx_functions:value("yes", "Load")
pbx_functions:value("no", "Do Not Load")
pbx_functions:value("auto", "Load as Required")
pbx_functions.rmempty = true

pbx_loopback = module:option(ListValue, "pbx_loopback", "Loopback Switch", "")
pbx_loopback:value("yes", "Load")
pbx_loopback:value("no", "Do Not Load")
pbx_loopback:value("auto", "Load as Required")
pbx_loopback.rmempty = true

pbx_realtime = module:option(ListValue, "pbx_realtime", "Realtime Switch", "")
pbx_realtime:value("yes", "Load")
pbx_realtime:value("no", "Do Not Load")
pbx_realtime:value("auto", "Load as Required")
pbx_realtime.rmempty = true

pbx_spool = module:option(ListValue, "pbx_spool", "Outgoing Spool Support", "")
pbx_spool:value("yes", "Load")
pbx_spool:value("no", "Do Not Load")
pbx_spool:value("auto", "Load as Required")
pbx_spool.rmempty = true

pbx_wilcalu = module:option(ListValue, "pbx_wilcalu", "Wil Cal U (Auto Dialer)", "")
pbx_wilcalu:value("yes", "Load")
pbx_wilcalu:value("no", "Do Not Load")
pbx_wilcalu:value("auto", "Load as Required")
pbx_wilcalu.rmempty = true


return cbimap
