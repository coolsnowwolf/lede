--
-- Copyright (C) 2018-2020 Ruilin Peng (Nick) <pymumu@gmail.com>.
--
-- smartdns is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- smartdns is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.

require ("nixio.fs")
require ("luci.http")
require ("luci.dispatcher")
require ("nixio.fs")

local uci = require "luci.model.uci".cursor()

module("luci.model.smartdns", package.seeall)

function get_config_option(module, section, option, default)
	return uci:get_first(module, section, option) or default
end

return m

