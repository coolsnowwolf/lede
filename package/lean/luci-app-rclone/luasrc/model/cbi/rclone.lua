require('luci.sys')
require('luci.util')

local ipkg = require('luci.model.ipkg')

local fs = require 'nixio.fs'

local uci = require 'luci.model.uci'.cursor()

local m, s

local running = (luci.sys.call('pidof rclone > /dev/null') == 0)

local state_msg = ''
local trport = uci:get('rclone', 'config', 'port')

local triptype = uci:get('rclone', 'config', 'addr_type')
local trip = ''

if triptype == 'local' then
    trip = uci:get('network', 'loopback', 'ipaddr')
elseif triptype == 'lan' then
    trip = uci:get('network', 'lan', 'ipaddr')
else
    trip = '[ip]'
end

if running then
    state_msg = '<b><font color="green">' .. translate('rclone running') .. '</font></b>'
    address_msg = translate('rclone address') .. ' : http://' .. trip .. ':' .. trport .. '<br/> <br/>'
else
    state_msg = '<b><font color="red">' .. translate('rclone not run') .. '</font></b>'
    address_msg = ''
end

if ipkg.installed("rclone-webui-react") and ipkg.installed("rclone-ng") then
m =
    Map(
    'rclone',
    translate('Rclone'),
    translate('Rclone ("rsync for cloud storage") is a command line program to sync root/usr/bin and directories to and from different cloud storage providers.') ..
        ' <br/> <br/> ' .. translate('rclone state') .. ' : ' .. state_msg .. '<br/> <br/>'
        .. address_msg ..
        translate('Installed Web Interface') ..                                                                                                                     
        '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="button" class="cbi-button" style="margin: 0 5px;" value=" ' ..                                            
        translate('Webui React') ..                                                                                                                                 
        " \" onclick=\"window.open('http://'+window.location.hostname+'/rclone-webui-react')\"/>" ..                                                                
        '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="button" class="cbi-button" style="margin: 0 5px;" value=" ' ..                                            
        translate('RcloneNg') ..                                                                                                                                    
        " \" onclick=\"window.open('http://'+window.location.hostname+'/RcloneNg')\"/> <br/><br/>"  
)
elseif ipkg.installed("rclone-webui-react") then
m =
    Map(
    'rclone',
    translate('Rclone'),
    translate('Rclone ("rsync for cloud storage") is a command line program to sync root/usr/bin and directories to and from different cloud storage providers.') ..
        ' <br/> <br/> ' .. translate('rclone state') .. ' : ' .. state_msg .. '<br/> <br/>'
        .. address_msg ..
        translate('Installed Web Interface') ..   
	
        '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="button" class="cbi-button" style="margin: 0 5px;" value=" ' ..                                            
        translate('Webui React') ..                                                                                                                                 
        " \" onclick=\"window.open('http://'+window.location.hostname+'/rclone-webui-react')\"/>" 
)
elseif ipkg.installed("rclone-ng") then
m =
    Map(
    'rclone',
    translate('Rclone'),
    translate('Rclone ("rsync for cloud storage") is a command line program to sync root/usr/bin and directories to and from different cloud storage providers.') ..
        ' <br/> <br/> ' .. translate('rclone state') .. ' : ' .. state_msg .. '<br/> <br/>'
        .. address_msg ..
        translate('Installed Web Interface') ..     
	
        '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="button" class="cbi-button" style="margin: 0 5px;" value=" ' ..                                            
        translate('RcloneNg') ..                                                                                                                                    
        " \" onclick=\"window.open('http://'+window.location.hostname+'/RcloneNg')\"/> <br/><br/>"  
)
else
m =
    Map(
    'rclone',
    translate('Rclone'),
    translate('Rclone ("rsync for cloud storage") is a command line program to sync root/usr/bin and directories to and from different cloud storage providers.') ..
        ' <br/> <br/> ' .. translate('rclone state') .. ' : ' .. state_msg .. '<br/> <br/>'
        .. address_msg
)
end

s = m:section(TypedSection, 'global', translate('global'))
s.addremove = false
s.anonymous = true

enable = s:option(Flag, 'enabled', translate('run Rclone as daemon'))
enable.rmempty = false

s = m:section(TypedSection, 'conf', translate('configure'))
s.addremove = false
s.anonymous = true

o = s:option(ListValue, 'addr_type', translate('listen address'))
o:value('local', translate('localhost address'))
o:value('lan', translate('local network address'))
o:value('all', translate('all address'))
o.default = 'lan'
o.rmempty = false

o = s:option(Value, 'port', translate('listen port'))
o.placeholder = 5572
o.default = 5572
o.datatype = 'port'
o.rmempty = false

o = s:option(Value, 'config_path', translate('rclone configuration file path'))
o.placeholder = '/etc/rclone/rclone.conf'
o.default = '/etc/rclone/rclone.conf'
o.rmempty = false
o.description = translate("Recommand: run ") ..
    "<span style=\"background: lightgray;border-radius: 4px;padding-left: 4px;padding-right: 4px;\">rclone config --config rclone.conf</span>" ..
    translate(" to setup configuration on pc,") .. "</br>" .. translate("than updaload configuration to here.")

o = s:option(Button,"config_download",translate("download configuration"))
o.inputtitle = translate("download")
o.inputstyle = "apply"
o.write = function()
  	Download()
end

function Download()
	local t,e
	t=nixio.open(uci:get('rclone', 'config', 'config_path'),"r")
	luci.http.header('Content-Disposition','attachment; filename="rclone.conf"')
	luci.http.prepare_content("application/octet-stream")
	while true do
		e=t:read(nixio.const.buffersize)
		if(not e)or(#e==0)then
			break
		else
			luci.http.write(e)
		end
	end
	t:close()
	luci.http.close()
end

o = s:option(Value, 'username', translate('username'))
o.placeholder = 'admin'
o.default = 'admin'
o.rmempty = false

o = s:option(Value, 'password', translate('password'))
o.password = true
o.placeholder = 'admin'
o.default = 'admin'
o.rmempty = false

s = m:section(TypedSection, 'proxy', translate('proxy'))
s.addremove = false
s.anonymous = true
s.description = "<a style=\"color:#fb6340;\" href=\"#\" onclick=\"window.open('https://rclone.org/faq/#can-i-use-rclone-with-an-http-proxy')\">"
    ..translate("FAQ").."</a>"

enable = s:option(Flag, 'enabled', translate('enable proxy'))
enable.rmempty = false

o = s:option(Value, 'proxy_addr', translate('proxy address'))
o.placeholder = 'socks5://127.0.0.1:1080'
o.default = 'socks5://127.0.0.1:1080'
o.rmempty = false
o.description = translate("The content of the variable is protocol://server:port. The protocol value is commonly either http or socks5.")

s = m:section(TypedSection, 'log', translate('log'))
s.addremove = false
s.anonymous = true

o = s:option(Value, 'path', translate('path'))
o.placeholder = '/var/log/rclone/output'
o.default = '/var/log/rclone/output'
o.rmempty = false

return m
