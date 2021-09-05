-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

local util = require "luci.util"
module("luci.config",
		function(m)
			if pcall(require, "luci.model.uci") then
				local config = util.threadlocal()
				setmetatable(m, {
					__index = function(tbl, key)
						if not config[key] then
							config[key] = luci.model.uci.cursor():get_all("luci", key)
						end
						return config[key]
					end
				})
			end
		end)
