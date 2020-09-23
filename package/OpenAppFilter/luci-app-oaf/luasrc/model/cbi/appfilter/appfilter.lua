
local ds = require "luci.dispatcher"

local m, s

m = Map("appfilter",
	translate("appfilter"),
	translate(""))
	
s = m:section(TypedSection, "global", translate("Basic Settings"))
s:option(Flag, "enable", translate("Enable App Filter"),translate(""))
s.anonymous = true


s = m:section(TypedSection, "appfilter", translate("App Filter Rules"))
s.anonymous = true
s.addremove = false


local class_fd = io.popen("find /tmp/appfilter/ -type f -name '*.class'")
if class_fd then
	while true do
		local apps
		local class
		local path = class_fd:read("*l")
		if not path then
			break
		end
		
		class = path:match("([^/]+)%.class$")
		-- add a tab
		s:tab(class, translate(class))
		-- multi value option
		apps = s:taboption(class, MultiValue, class.."apps", translate(""))
		apps.rmempty=true
		--apps.delimiter=";"
		-- select 
		apps.widget="checkbox"
		apps.size=12

		local fd = io.open(path)
		if fd then
			local line
			while true do
				local cmd
				local cmd_fd
				line = fd:read("*l")
				if not line then break end
				if string.len(line) < 5 then break end
				if not string.find(line,"#") then 
					cmd = "echo "..line.."|awk '{print $1}'"
					cmd_fd = io.popen(cmd)
					id = cmd_fd:read("*l");
					cmd_fd:close()
				
					cmd = "echo "..line.."|awk '{print $2}'"
					cmd_fd = io.popen(cmd)
					name = cmd_fd:read("*l")
				
					cmd_fd:close()
					if not id then break end
					if not name then break end
					apps:value(id, name)
				end
			end
			fd:close()
		end
	end
	class_fd:close()
end


s=m:section(TypedSection,"user",translate("Select users"))
s.anonymous = true
users = s:option(MultiValue, "users", "", translate("Select at least one user, otherwise it will take effect for all users"))
users.widget="checkbox"

function get_hostname_by_mac(dst_mac)
    leasefile="/tmp/dhcp.leases"
    local fd = io.open(leasefile, "r")
	if not fd then return end
    while true do
        local ln = fd:read("*l")
        if not ln then
            break
        end
        local ts, mac, ip, name, duid = ln:match("^(%d+) (%S+) (%S+) (%S+) (%S+)")
        print(ln)
        if  dst_mac == mac then
			fd:close()
            return name
        end
    end
	fd:close()
    return nil
end

users.widget="checkbox"
--users.widget="select"
users.size=1

local fd = io.open("/proc/net/arp", "r")
if not fd then return end
while true do
	local line = fd:read("*l")
	if not line then
		break
	end
	if not line:match("Ip*") then
		local ip, hw_type, flags, mac, mask, device = line:match("(%S+) %s+ (%S+) %s+ (%S+) %s+ (%S+) %s+ (%S+) %s+ (%S+)")
		if device ~= nil and mac ~= nil and device:match("lan") then
			local hostname=get_hostname_by_mac(mac)
			if not hostname then
				users:value(mac, mac);
			else
				users:value(mac, hostname);
			end
		end
	end
end

m:section(SimpleSection).template = "admin_network/user_status"


return m
