-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Cpyright by Twinzo<1282055288@qq.com>
-- Licensed to the public under the Apache License 2.0.

local m, s, o
local fs = require "nixio.fs"
local sys = require "luci.sys"

m = Map("configEditor", translate("configEditor"))
m.template ="configEditor"


return m

