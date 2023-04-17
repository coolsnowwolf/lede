
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local uci = luci.model.uci.cursor()
local fs = require "luci.clash"
local http = luci.http
local clash = "clash"




kr = Map(clash)
s = kr:section(TypedSection, "clash", translate("Subscription Config"))
s.anonymous = true
kr.pageaction = false

o = s:option(ListValue, "subcri", translate("Subcription Type"))
o.default = clash
o:value("clash", translate("clash"))
o:value("ssr2clash", translate("ssr2clash"))
o:value("v2clash", translate("v2clash"))
o.description = translate("Select Subcription Type")

o = s:option(Value, "config_name")
o.title = translate("Config Name")
o.description = translate("Config Name. Do not use a config name that already exist")

o = s:option(Value, "clash_url")
o.title = translate("Subcription Url")
o.description = translate("Clash Subscription Address")
o.rmempty = true
o:depends("subcri", 'clash')


o = s:option(Button,"update")
o.title = translate("Download Config")
o.inputtitle = translate("Download Config")
o.inputstyle = "reload"
o.write = function()
  kr.uci:commit("clash")
  SYS.call("sh /usr/share/clash/clash.sh >>/usr/share/clash/clash.txt 2>&1 &")
  SYS.call("sleep 1")
  HTTP.redirect(DISP.build_url("admin", "services", "clash"))
end
o:depends("subcri", 'clash')

o = s:option(Value, "ssr_url")
o.title = translate("Subcription Url")
o.placeholder = translate("https://www.example.com/link/QkjokZXktyyr35gfj")
o.rmempty = true
o:depends("subcri", 'ssr2clash')


o = s:option(Button,"updatee")
o.title = translate("Download Config")
o.inputtitle = translate("Download Config")
o.inputstyle = "reload"
o.write = function()
  kr.uci:commit("clash")
  luci.sys.call("bash /usr/share/clash/clash.sh >>/usr/share/clash/clash.txt 2>&1 &")
  HTTP.redirect(DISP.build_url("admin", "services", "clash"))
end
o:depends("subcri", 'ssr2clash')

o = s:option(Value, "v2_url")
o.title = translate("Subcription Url")
o.placeholder = translate("https://www.example.com/link/QkjokZXktyyr35gfj")
o.rmempty = true
o:depends("subcri", 'v2clash')


o = s:option(Button,"updateee")
o.title = translate("Download Config")
o.inputtitle = translate("Download Config")
o.inputstyle = "reload"
o.write = function()
  kr.uci:commit("clash")
  luci.sys.call("bash /usr/share/clash/clash.sh >>/usr/share/clash/clash.txt 2>&1 &")
  HTTP.redirect(DISP.build_url("admin", "services", "clash"))
end
o:depends("subcri", 'v2clash')


function IsYamlFile(e)
   e=e or""
   local e=string.lower(string.sub(e,-5,-1))
   return e == ".yaml"
end

function IsYmlFile(e)
   e=e or""
   local e=string.lower(string.sub(e,-4,-1))
   return e == ".yml"
end


ko = Map(clash)
ko.reset = false
ko.submit = false
sul =ko:section(TypedSection, "clash", translate("Upload Config"))
sul.anonymous = true
sul.addremove=false
o = sul:option(FileUpload, "")
--o.description = translate("NB: Only upload file with name .yaml.It recommended to rename each upload file name to avoid overwrite")
o.title = translate("  ")
o.template = "clash/clash_upload"
um = sul:option(DummyValue, "", nil)
um.template = "clash/clash_dvalue"

local dir, fd
dir = "/usr/share/clash/config/upload/"
http.setfilehandler(

	function(meta, chunk, eof)
		if not fd then
			if not meta then return end

			if	meta and chunk then fd = nixio.open(dir .. meta.file, "w") end

			if not fd then
				um.value = translate("upload file error.")
				return
			end
		end
		if chunk and fd then
			fd:write(chunk)
		end
		if eof and fd then
			fd:close()
			fd = nil
			local e=string.lower(string.sub(meta.file,-4,-1))
			local yml2=string.lower(string.sub(meta.file,0,-5))
			if e == '.yml'  then
			local yml=string.lower(string.sub(meta.file,0,-5))
			local c=fs.rename(dir .. meta.file,"/usr/share/clash/config/upload/".. yml .. ".yaml")
			um.value = translate("File saved to") .. ' "/usr/share/clash/config/upload/'..yml..'.yaml"'
			else
			um.value = translate("File saved to") .. ' "/usr/share/clash/config/upload/'..yml2..'yaml"'
			end
			
		end
	end
)

if luci.http.formvalue("upload") then
	local f = luci.http.formvalue("ulfile")
	if #f <= 0 then
		um.value = translate("No specify upload file.")
	end
end


return kr,ko
