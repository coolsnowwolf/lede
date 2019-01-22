-- Copyright 2018 Rosy Song <rosysong@rosinson.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.nft-qos", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/nft-qos") then
		return
	end

	local page

	page = entry({"admin", "services", "nft-qos"}, cbi("nft-qos"), _("Qos over Nftables"))
	page.dependent = true
end
