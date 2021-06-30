-- Copyright (C) 2021 dz <dingzhong110@gmail.com>

local m,s,o
local sys = require "luci.sys"
local uci = require "luci.model.uci".cursor()

m = Map("easymesh")

function detect_Node()
	local data = {}
	local lps = luci.util.execi(" batctl n 2>/dev/null | tail +2 | sed 's/^[ ][ ]*//g' | sed 's/[ ][ ]*/ /g' | sed 's/$/ /g' ")
	for value in lps do
		local row = {}
		local pos = string.find(value, " ")
		local IFA = string.sub(value, 1, pos - 1)
		local value = string.sub(value, pos + 1, string.len(value))
		pos = string.find(value, " ")
		local pos = string.find(value, " ")
		local Neighbora = string.sub(value, 1, pos - 1)
		local value = string.sub(value, pos + 1, string.len(value))
		pos = string.find(value, " ")
		local pos = string.find(value, " ")
		local lastseena = string.sub(value, 1, pos - 1)
		local value = string.sub(value, pos + 1, string.len(value))
		pos = string.find(value, " ")
		row["IF"] = IFA
		row["Neighbor"] = Neighbora
		row["lastseen"] = lastseena
		table.insert(data, row)
	end
	return data
end
local Nodes = luci.sys.exec("batctl n 2>/dev/null| tail +3 | wc -l")
local Node = detect_Node()
v = m:section(Table, Node, "" ,translate("<b>Active node：" .. Nodes .. "</b>"))
v:option(DummyValue, "IF")
v:option(DummyValue, "Neighbor")
v:option(DummyValue, "lastseen")

-- Basic
s = m:section(TypedSection, "easymesh", translate("Settings"), translate("General Settings"))
s.anonymous = true

---- Eanble
o = s:option(Flag, "enabled", translate("Enable"), translate("Enable or disable EASY MESH"))
o.default = 0
o.rmempty = false

apRadio = s:option(ListValue, "apRadio", translate("MESH Radio device"), translate("The radio device which MESH use"))
uci:foreach("wireless", "wifi-device",
							function(s)
								apRadio:value(s['.name'])
							end)
o.default = "radio0"
o.rmempty = false

---- mesh
o = s:option(Value, "mesh_id", translate("MESH ID"))
o.default = "easymesh"
o.description = translate("MESH ID")

enable = s:option(Flag, "encryption", translate("Encryption"), translate(""))
enable.default = 0
enable.rmempty = false

o = s:option(Value, "key", translate("Key"))
o.default = "easymesh"
o:depends("encryption", 1)

return m
