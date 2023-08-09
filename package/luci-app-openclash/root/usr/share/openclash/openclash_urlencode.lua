#!/usr/bin/lua

require "nixio"
require "luci.util"
require "luci.sys"
local HTTP = require "luci.http"
local url = arg[1]

if not url then os.exit(0) end

print(HTTP.urlencode(url) or url)

os.exit(0)