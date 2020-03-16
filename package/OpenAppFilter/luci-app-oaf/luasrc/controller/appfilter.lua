module("luci.controller.appfilter", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/appfilter") then
		return
	end
	
	local page

	page = entry({"admin", "network", "appfilter"}, cbi("appfilter/appfilter"), _("appfilter"))
	page.dependent = true
	
	page = entry({"admin", "network", "user_status"}, call("user_status"), nil)
	page.leaf = true
end

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
            print("match mac", mac, "hostname=", name);
			fd:close()
            return name
        end
    end
	fd:close()
    return ""
end

function get_app_name_by_id(appid)
	local class_fd = io.popen("find /etc/appfilter/ -type f -name *.class |xargs cat |grep "..appid.."|awk '{print $2}'")
	if class_fd then
		local name = class_fd:read("*l")
		class_fd:close()
		return name
	end
	return ""
end

function cmp_func(a,b)
	return a.latest_time > b.latest_time
end




function user_status()
	local json = require "luci.jsonc"
	luci.http.prepare_content("application/json")
	--local fs=require "nixio.fs"
	--local ok, status_data = pcall(json.parse, fs.readfile("/proc/net/af_client"))
	--luci.http.write_json(tb);
	local fd = io.open("/proc/net/af_client","r")
	
	status_buf=fd:read('*a')
	fd:close()
	user_array=json.parse(status_buf)
	
	local history={}
	for i, v in pairs(user_array) do
		visit_array=user_array[i].visit_info
		for j,s in pairs(visit_array) do
			print(user_array[i].mac, user_array[i].ip,visit_array[j].appid, visit_array[j].latest_time)
			history[#history+1]={
				mac=user_array[i].mac,
				ip=user_array[i].ip,
				hostname=get_hostname_by_mac(user_array[i].mac),
				appid=visit_array[j].appid,
				appname=get_app_name_by_id(visit_array[j].appid),
				total_num=visit_array[j].total_num,
				drop_num=visit_array[j].drop_num,
				latest_action=visit_array[j].latest_action,
				latest_time=os.date("%Y/%m/%d %H:%M:%S", visit_array[j].latest_time)
			}
		end
	end
	table.sort(history, cmp_func)
	--luci.http.write(history);
	luci.http.write_json(history);
end

