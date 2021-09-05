-- Copyright 2017 Yousong Zhou <yszhou4tech@gmail.com>
-- Licensed to the public under the Apache License 2.0.
--
module("luci.controller.shadowsocks-libev", package.seeall)

function index()
	entry({"admin", "services", "shadowsocks-libev"},
		alias("admin", "services", "shadowsocks-libev", "instances"),
		_("Shadowsocks-libev"), 59)

	entry({"admin", "services", "shadowsocks-libev", "instances"},
		arcombine(cbi("shadowsocks-libev/instances"), cbi("shadowsocks-libev/instance-details")),
		_("Local Instances"), 10).leaf = true

	entry({"admin", "services", "shadowsocks-libev", "servers"},
		cbi("shadowsocks-libev/servers"),
		_("Remote Servers"), 20).leaf = true

	entry({"admin", "services", "shadowsocks-libev", "rules"},
		cbi("shadowsocks-libev/rules"),
		_("Redir Rules"), 30).leaf = true

	entry({"admin", "services", "shadowsocks-libev", "status"}, call("ss_status"), nil).leaf = true

end

function ss_status()
	local ut = require "luci.util"
	local rv = ut.ubus("service", "list", {name = "shadowsocks-libev"})["shadowsocks-libev"] or {_=0}

	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
end
