-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

cbimap = Map("asterisk", "asterisk", "")

module = cbimap:section(TypedSection, "module", "Modules", "")
module.anonymous = true

res_config_mysql = module:option(ListValue, "res_config_mysql", "MySQL Config Resource", "")
res_config_mysql:value("yes", "Load")
res_config_mysql:value("no", "Do Not Load")
res_config_mysql:value("auto", "Load as Required")
res_config_mysql.rmempty = true

res_config_odbc = module:option(ListValue, "res_config_odbc", "ODBC Config Resource", "")
res_config_odbc:value("yes", "Load")
res_config_odbc:value("no", "Do Not Load")
res_config_odbc:value("auto", "Load as Required")
res_config_odbc.rmempty = true

res_config_pgsql = module:option(ListValue, "res_config_pgsql", "PGSQL Module", "")
res_config_pgsql:value("yes", "Load")
res_config_pgsql:value("no", "Do Not Load")
res_config_pgsql:value("auto", "Load as Required")
res_config_pgsql.rmempty = true

res_crypto = module:option(ListValue, "res_crypto", "Cryptographic Digital Signatures", "")
res_crypto:value("yes", "Load")
res_crypto:value("no", "Do Not Load")
res_crypto:value("auto", "Load as Required")
res_crypto.rmempty = true

res_features = module:option(ListValue, "res_features", "Call Parking Resource", "")
res_features:value("yes", "Load")
res_features:value("no", "Do Not Load")
res_features:value("auto", "Load as Required")
res_features.rmempty = true

res_indications = module:option(ListValue, "res_indications", "Indications Configuration", "")
res_indications:value("yes", "Load")
res_indications:value("no", "Do Not Load")
res_indications:value("auto", "Load as Required")
res_indications.rmempty = true

res_monitor = module:option(ListValue, "res_monitor", "Call Monitoring Resource", "")
res_monitor:value("yes", "Load")
res_monitor:value("no", "Do Not Load")
res_monitor:value("auto", "Load as Required")
res_monitor.rmempty = true

res_musiconhold = module:option(ListValue, "res_musiconhold", "Music On Hold Resource", "")
res_musiconhold:value("yes", "Load")
res_musiconhold:value("no", "Do Not Load")
res_musiconhold:value("auto", "Load as Required")
res_musiconhold.rmempty = true

res_odbc = module:option(ListValue, "res_odbc", "ODBC Resource", "")
res_odbc:value("yes", "Load")
res_odbc:value("no", "Do Not Load")
res_odbc:value("auto", "Load as Required")
res_odbc.rmempty = true

res_smdi = module:option(ListValue, "res_smdi", "SMDI Module", "")
res_smdi:value("yes", "Load")
res_smdi:value("no", "Do Not Load")
res_smdi:value("auto", "Load as Required")
res_smdi.rmempty = true

res_snmp = module:option(ListValue, "res_snmp", "SNMP Module", "")
res_snmp:value("yes", "Load")
res_snmp:value("no", "Do Not Load")
res_snmp:value("auto", "Load as Required")
res_snmp.rmempty = true


return cbimap
