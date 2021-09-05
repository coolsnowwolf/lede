-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local config = require "luci.config"
local ccache = require "luci.ccache"

module "luci.cacheloader"

if config.ccache and config.ccache.enable == "1" then
	ccache.cache_ondemand()
end