
local ds = require "luci.dispatcher"
local nxo = require "nixio"
local nfs = require "nixio.fs"
local ipc = require "luci.ip"
local sys = require "luci.sys"
local utl = require "luci.util"
local dsp = require "luci.dispatcher"
local uci = require "luci.model.uci"
local lng = require "luci.i18n"
local jsc = require "luci.jsonc"

local m, s
arg[1] = arg[1] or ""
m = Map("appfilter",
	translate("上网统计("..arg[1]..")"),
	translate(""))
	
local v
v=m:section(SimpleSection)
v.template="admin_network/dev_status"
v.mac=arg[1]
m.redirect = luci.dispatcher.build_url("admin", "network", "appfilter")
return m
