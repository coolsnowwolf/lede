-- Copyright 2020 IceG <eko.one.pl>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.modem.3ginfo", package.seeall)

function index()

	entry({"admin", "modem"}, firstchild(), "Modem", 30).dependent=false
	entry({"admin", "modem", "3ginfo"}, alias("admin", "modem", "3ginfo", "3gdetail"), translate("Information about 3G/4G connection"), 10).acl_depends={ "luci-app-3ginfo" }
	entry({"admin", "modem", "3ginfo", "3gdetail"},template("modem/3gdetail"),translate("Details"), 10)
	entry({"admin", "modem", "3ginfo", "3gconfig"},cbi("modem/3gconfig"),translate("Configuration"), 20)
	entry({"admin", "modem", "3ginfo", "devicedata"}, call("action_devicedata")).leaf = true

end

function action_devicedata()
    local fs = require "nixio.fs"
    local data = luci.sys.exec("3ginfo json")
    luci.http.prepare_content("application/json")
    luci.http.write(data)
end
