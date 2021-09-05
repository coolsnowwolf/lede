-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2010-2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

m = Map("network", translate("Switch"), translate("The network ports on this device can be combined to several <abbr title=\"Virtual Local Area Network\">VLAN</abbr>s in which computers can communicate directly with each other. <abbr title=\"Virtual Local Area Network\">VLAN</abbr>s are often used to separate different network segments. Often there is by default one Uplink port for a connection to the next greater network like the internet and other ports for a local network."))

local fs = require "nixio.fs"
local nw = require "luci.model.network"
local switches = { }

nw.init(m.uci)

local topologies = nw:get_switch_topologies() or {}

local update_interfaces = function(old_ifname, new_ifname)
	local info = { }

	m.uci:foreach("network", "interface", function(section)
		local old_ifnames = m.uci:get("network", section[".name"], "ifname")
		local new_ifnames = { }
		local cur_ifname
		local changed = false
		for cur_ifname in luci.util.imatch(old_ifnames) do
			if cur_ifname == old_ifname then
				new_ifnames[#new_ifnames+1] = new_ifname
				changed = true
			else
				new_ifnames[#new_ifnames+1] = cur_ifname
			end
		end
		if changed then
			m.uci:set("network", section[".name"], "ifname", table.concat(new_ifnames, " "))

			info[#info+1] = translatef("Interface %q device auto-migrated from %q to %q.",
				section[".name"], old_ifname, new_ifname)
		end
	end)

	if #info > 0 then
		m.message = (m.message and m.message .. "\n" or "") .. table.concat(info, "\n")
	end
end

m.uci:foreach("network", "switch",
	function(x)
		local sid         = x['.name']
		local switch_name = x.name or sid
		local has_vlan    = nil
		local has_learn   = nil
		local has_vlan4k  = nil
		local has_jumbo3  = nil
		local has_mirror  = nil
		local min_vid     = 0
		local max_vid     = 16
		local num_vlans   = 16

		local switch_title
		local enable_vlan4k = false

		local topo = topologies[switch_name]

		if not topo then
			m.message = translatef("Switch %q has an unknown topology - the VLAN settings might not be accurate.", switch_name)
			topo = {
				ports = {
					{ num = 0, label = "Port 1" },
					{ num = 1, label = "Port 2" },
					{ num = 2, label = "Port 3" },
					{ num = 3, label = "Port 4" },
					{ num = 4, label = "Port 5" },
					{ num = 5, label = "CPU (eth0)", tagged = false }
				}
			}
		end

		-- Parse some common switch properties from swconfig help output.
		local swc = io.popen("swconfig dev %q help 2>/dev/null" % switch_name)
		if swc then

			local is_port_attr = false
			local is_vlan_attr = false

			while true do
				local line = swc:read("*l")
				if not line then break end

				if line:match("^%s+%-%-vlan") then
					is_vlan_attr = true

				elseif line:match("^%s+%-%-port") then
					is_vlan_attr = false
					is_port_attr = true

				elseif line:match("cpu @") then
					switch_title = line:match("^switch%d: %w+%((.-)%)")
					num_vlans  = tonumber(line:match("vlans: (%d+)")) or 16
					min_vid    = 1

				elseif line:match(": pvid") or line:match(": tag") or line:match(": vid") then
					if is_vlan_attr then has_vlan4k = line:match(": (%w+)") end

				elseif line:match(": enable_vlan4k") then
					enable_vlan4k = true

				elseif line:match(": enable_vlan") then
					has_vlan = "enable_vlan"

				elseif line:match(": enable_learning") then
					has_learn = "enable_learning"

				elseif line:match(": enable_mirror_rx") then
					has_mirror = "enable_mirror_rx"

				elseif line:match(": max_length") then
					has_jumbo3 = "max_length"
				end
			end

			swc:close()
		end


		-- Switch properties
		s = m:section(NamedSection, x['.name'], "switch",
			switch_title and translatef("Switch %q (%s)", switch_name, switch_title)
					      or translatef("Switch %q", switch_name))

		s.addremove = false

		if has_vlan then
			s:option(Flag, has_vlan, translate("Enable VLAN functionality"))
		end

		if has_learn then
			x = s:option(Flag, has_learn, translate("Enable learning and aging"))
			x.default = x.enabled
		end

		if has_jumbo3 then
			x = s:option(Flag, has_jumbo3, translate("Enable Jumbo Frame passthrough"))
			x.enabled = "3"
			x.rmempty = true
		end

		-- Does this switch support port mirroring?
		if has_mirror then
			s:option(Flag, "enable_mirror_rx", translate("Enable mirroring of incoming packets"))
			s:option(Flag, "enable_mirror_tx", translate("Enable mirroring of outgoing packets"))

			local sp = s:option(ListValue, "mirror_source_port", translate("Mirror source port"))
			local mp = s:option(ListValue, "mirror_monitor_port", translate("Mirror monitor port"))

			sp:depends("enable_mirror_tx", "1")
			sp:depends("enable_mirror_rx", "1")

			mp:depends("enable_mirror_tx", "1")
			mp:depends("enable_mirror_rx", "1")

			local _, pt
			for _, pt in ipairs(topo.ports) do
				sp:value(pt.num, pt.label)
				mp:value(pt.num, pt.label)
			end
		end

		-- VLAN table
		s = m:section(TypedSection, "switch_vlan",
			switch_title and translatef("VLANs on %q (%s)", switch_name, switch_title)
						  or translatef("VLANs on %q", switch_name))

		s.template = "cbi/tblsection"
		s.addremove = true
		s.anonymous = true

		-- Filter by switch
		s.filter = function(self, section)
			local device = m:get(section, "device")
			return (device and device == switch_name)
		end

		-- Override cfgsections callback to enforce row ordering by vlan id.
		s.cfgsections = function(self)
			local osections = TypedSection.cfgsections(self)
			local sections = { }
			local section

			for _, section in luci.util.spairs(
				osections,
				function(a, b)
					return (tonumber(m:get(osections[a], has_vlan4k or "vlan") or "") or 9999)
						<  (tonumber(m:get(osections[b], has_vlan4k or "vlan") or "") or 9999)
				end
			) do
				sections[#sections+1] = section
			end

			return sections
		end

		-- When creating a new vlan, preset it with the highest found vid + 1.
		s.create = function(self, section, origin)
			-- Filter by switch
			if m:get(origin, "device") ~= switch_name then
				return
			end

			local sid = TypedSection.create(self, section)

			local max_nr = 0
			local max_id = 0

			m.uci:foreach("network", "switch_vlan",
				function(s)
					if s.device == switch_name then
						local nr = tonumber(s.vlan)
						local id = has_vlan4k and tonumber(s[has_vlan4k])
						if nr ~= nil and nr > max_nr then max_nr = nr end
						if id ~= nil and id > max_id then max_id = id end
					end
				end)

			m:set(sid, "device", switch_name)
			m:set(sid, "vlan", max_nr + 1)

			if has_vlan4k then
				m:set(sid, has_vlan4k, max_id + 1)
			end

			return sid
		end


		local port_opts = { }
		local untagged  = { }

		-- Parse current tagging state from the "ports" option.
		local portvalue = function(self, section)
			local pt
			for pt in (m:get(section, "ports") or ""):gmatch("%w+") do
				local pc, tu = pt:match("^(%d+)([tu]*)")
				if pc == self.option then return (#tu > 0) and tu or "u" end
			end
			return ""
		end

		-- Validate port tagging. Ensure that a port is only untagged once,
		-- bail out if not.
		local portvalidate = function(self, value, section)
			-- ensure that the ports appears untagged only once
			if value == "u" then
				if not untagged[self.option] then
					untagged[self.option] = true
				else
					return nil,
						translatef("%s is untagged in multiple VLANs!", self.title)
				end
			end
			return value
		end


		local vid = s:option(Value, has_vlan4k or "vlan", "VLAN ID", "<div id='portstatus-%s'></div>" % switch_name)
		local mx_vid = has_vlan4k and 4094 or (num_vlans - 1)

		vid.rmempty = false
		vid.forcewrite = true
		vid.vlan_used = { }
		vid.datatype = "and(uinteger,range("..min_vid..","..mx_vid.."))"

		-- Validate user provided VLAN ID, make sure its within the bounds
		-- allowed by the switch.
		vid.validate = function(self, value, section)
			local v = tonumber(value)
			local m = has_vlan4k and 4094 or (num_vlans - 1)
			if v ~= nil and v >= min_vid and v <= m then
				if not self.vlan_used[v] then
					self.vlan_used[v] = true
					return value
				else
					return nil,
						translatef("Invalid VLAN ID given! Only unique IDs are allowed")
				end
			else
				return nil,
					translatef("Invalid VLAN ID given! Only IDs between %d and %d are allowed.", min_vid, m)
			end
		end

		-- When writing the "vid" or "vlan" option, serialize the port states
		-- as well and write them as "ports" option to uci.
		vid.write = function(self, section, new_vid)
			local o
			local p = { }
			for _, o in ipairs(port_opts) do
				local new_tag = o:formvalue(section)
				if new_tag == "t" then
					p[#p+1] = o.option .. new_tag
				elseif new_tag == "u" then
					p[#p+1] = o.option
				end

				if o.info and o.info.device then
					local old_tag = o:cfgvalue(section)
					local old_vid = self:cfgvalue(section)
					if old_tag ~= new_tag or old_vid ~= new_vid then
						local old_ifname = (old_tag == "u") and o.info.device
							or "%s.%s" %{ o.info.device, old_vid }

						local new_ifname = (new_tag == "u") and o.info.device
							or "%s.%s" %{ o.info.device, new_vid }

						if old_ifname ~= new_ifname then
							update_interfaces(old_ifname, new_ifname)
						end
					end
				end
			end

			if enable_vlan4k then
				m:set(sid, "enable_vlan4k", "1")
			end

			m:set(section, "ports", table.concat(p, " "))
			return Value.write(self, section, new_vid)
		end

		-- Fallback to "vlan" option if "vid" option is supported but unset.
		vid.cfgvalue = function(self, section)
			return m:get(section, has_vlan4k or "vlan")
				or m:get(section, "vlan")
		end

		local _, pt
		for _, pt in ipairs(topo.ports) do
			local po = s:option(ListValue, tostring(pt.num), pt.label, '<div id="portstatus-%s-%d"></div>' %{ switch_name, pt.num })

			po:value("",  translate("off"))

			if not pt.tagged then
				po:value("u", translate("untagged"))
			end

			po:value("t", translate("tagged"))

			po.cfgvalue = portvalue
			po.validate = portvalidate
			po.write    = function() end
			po.info     = pt

			port_opts[#port_opts+1] = po
		end

		table.sort(port_opts, function(a, b) return a.option < b.option end)
		switches[#switches+1] = switch_name
	end
)

-- Switch status template
s = m:section(SimpleSection)
s.template = "admin_network/switch_status"
s.switches = switches

return m
