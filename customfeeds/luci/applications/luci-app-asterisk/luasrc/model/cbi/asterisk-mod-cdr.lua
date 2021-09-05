-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

cbimap = Map("asterisk", "asterisk", "")

module = cbimap:section(TypedSection, "module", "Modules", "")
module.anonymous = true

cdr_csv = module:option(ListValue, "cdr_csv", "Comma Separated Values CDR Backend", "")
cdr_csv:value("yes", "Load")
cdr_csv:value("no", "Do Not Load")
cdr_csv:value("auto", "Load as Required")
cdr_csv.rmempty = true

cdr_custom = module:option(ListValue, "cdr_custom", "Customizable Comma Separated Values CDR Backend", "")
cdr_custom:value("yes", "Load")
cdr_custom:value("no", "Do Not Load")
cdr_custom:value("auto", "Load as Required")
cdr_custom.rmempty = true

cdr_manager = module:option(ListValue, "cdr_manager", "Asterisk Call Manager CDR Backend", "")
cdr_manager:value("yes", "Load")
cdr_manager:value("no", "Do Not Load")
cdr_manager:value("auto", "Load as Required")
cdr_manager.rmempty = true

cdr_mysql = module:option(ListValue, "cdr_mysql", "MySQL CDR Backend", "")
cdr_mysql:value("yes", "Load")
cdr_mysql:value("no", "Do Not Load")
cdr_mysql:value("auto", "Load as Required")
cdr_mysql.rmempty = true

cdr_pgsql = module:option(ListValue, "cdr_pgsql", "PostgreSQL CDR Backend", "")
cdr_pgsql:value("yes", "Load")
cdr_pgsql:value("no", "Do Not Load")
cdr_pgsql:value("auto", "Load as Required")
cdr_pgsql.rmempty = true

cdr_sqlite = module:option(ListValue, "cdr_sqlite", "SQLite CDR Backend", "")
cdr_sqlite:value("yes", "Load")
cdr_sqlite:value("no", "Do Not Load")
cdr_sqlite:value("auto", "Load as Required")
cdr_sqlite.rmempty = true


return cbimap
