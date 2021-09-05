
local ds = require "luci.dispatcher"
local nxo = require "nixio"
local nfs = require "nixio.fs"
local ipc = require "luci.ip"
local sys = require "luci.sys"
local utl = require "luci.util"
local dsp = require "luci.dispatcher"
local uci = require "luci.model.uci"
local lng = require "luci.i18n"
local jsc = require "luci.jsonc"
local http = luci.http
local SYS  = require "luci.sys"
local m, s

m = Map("appfilter",
	translate("appfilter"),
	translate(""))
	
s = m:section(TypedSection, "global", translate("Basic Settings"))
s:option(Flag, "enable", translate("Enable App Filter"),translate(""))
um = s:option(DummyValue, "")
um.template="cbi/oaf_dvalue"
local fullcone=SYS.exec("uci get firewall.@defaults[0].fullcone");
local bbr=SYS.exec("uci get flowoffload.@flow[0].bbr");
local flow_offloading=SYS.exec("uci get flowoffload.@flow[0].flow_offloading");
if string.match(fullcone, "1")  or string.match(bbr, "1") or string.match(flow_offloading, "1") then
    um.value="运行环境检测失败，请先关闭ACC加速模块!"
end
s.anonymous = true

local rule_count=0
local version=""
if nixio.fs.access("/etc/appfilter/feature.cfg") then
	rule_count=tonumber(SYS.exec("cat /etc/appfilter/feature.cfg | wc -l"))
	version=SYS.exec("cat /etc/appfilter/feature.cfg  |grep \"#version\" | awk '{print $2}'")
end
local display_str="<strong>当前版本:  </strong>"..version.."<br><strong>特征码个数:</strong>  "..rule_count.."<br><strong>  下载地址:</strong><a href=\"https://destan19.github.io\">https://destan19.github.io</a>"
s = m:section(TypedSection, "feature", translate("特征库更新"), display_str )

fu = s:option(FileUpload, "")
fu.template = "cbi/oaf_upload"
s.anonymous = true

um = s:option(DummyValue, "rule_data")
um.template="cbi/oaf_dvalue"

s=m:section(TypedSection,"time",translate("时间控制")) s.anonymous = true
hv = s:option(Value, "start_time", translate("开始时间")) hv.default="00:00"
hv.optional=false
hv = s:option(Value, "end_time", translate("结束时间")) hv.default="23:59"
hv.optional=false days = s:option(MultiValue, "days", "", translate("")) 
days.widget="checkbox" days.size=10 
days:value("0", "周日");
days:value("1", "周一"); 
days:value("2", "周二");
days:value("3", "周三"); 
days:value("4", "周四"); 
days:value("5", "周五"); 
days:value("6", "周六"); 

--um.value =rule_count .. " " .. translate("Records").. "  "..version

 



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
		apps.size=10

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
function get_cmd_result(command)
	local fd      
	local result
	fd = io.popen(command);
	if not fd then return "" end                                              
	result = fd:read("*l");
	fd:close()                
	return result  
end

s=m:section(TypedSection,"user",translate("Select users"))
s.anonymous = true
users = s:option(MultiValue, "users", "", translate("Select at least one user, otherwise it will take effect for all users"))
users.widget="checkbox"
--users.widget="select"
users.size=6

local fd = io.open("/tmp/dev_list", "r")
if not fd then return m end
while true do
	local line = fd:read("*l")
	if not line then
		break
	end
	if not string.match(line, "^Id") then
		local ip=get_cmd_result(string.format("echo '%s' | awk '{print $3}'", line))
		local mac=get_cmd_result(string.format("echo '%s' | awk '{print $2}'", line))
		local hostname=get_cmd_result(string.format("echo '%s' | awk '{print $4}'", line))
		if mac ~= nil then

			if not hostname or hostname == "*" then
				users:value(mac, mac);
			else
				users:value(mac, hostname.."("..mac..")");
			end
		end
	end
end
fd:close()

local config_users=m.uci:get_all("appfilter.user.users")
if config_users~=nil and config_users~=false then
	local r=utl.split(config_users, "%s+", nil, true)
	local max = table.getn(r)
	for i=1,max,1 do
		users:value(r[i], r[i]);
	end
end
m:section(SimpleSection).template = "admin_network/user_status"
local dir, fd
dir = "/tmp/upload/"
nixio.fs.mkdir(dir)
http.setfilehandler(
	function(meta, chunk, eof)
		if not fd then
			if not meta then return end
			if	meta and chunk then fd = nixio.open(dir .. meta.file, "w") end
			if not fd then
				return
			end
		end
		if chunk and fd then
			fd:write(chunk)
		end
		if eof and fd then   
			fd:close()   
			local fd2 = io.open("/tmp/upload/"..meta.file)
			local line=fd2:read("*l");       
			fd2:close()        
			local ret=string.match(line, "#version")
			if ret ~= nil then 
					local cmd="cp /tmp/upload/"..meta.file.." /etc/appfilter/feature.cfg";
					os.execute(cmd);
					os.execute("rm /tmp/appfilter -fr");
					luci.sys.exec("/etc/init.d/appfilter restart &");
					um.value = translate("更新成功，请刷新页面!")
			else                                      
					um.value = translate("更新失败，格式错误!")
			end
			os.execute("rm /tmp/upload/* -fr");
		end

	end
)

if luci.http.formvalue("upload") then
	local f = luci.http.formvalue("ulfile")
	if #f <= 0 then
		--um.value = translate("No specify upload file.")
	end
elseif luci.http.formvalue("download") then
	Download()
end


return m
