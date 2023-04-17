local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local uci = require("luci.model.uci").cursor()
local m , r, k
local http = luci.http

font_red = [[<font color="red">]]
font_green = [[<font color="green">]]
font_off = [[</font>]]
bold_on  = [[<strong>]]
bold_off = [[</strong>]]



ko = Map("clash")
ko.reset = false
ko.submit = false
sul =ko:section(TypedSection, "clash",translate("Manual Upload"))
sul.anonymous = true
sul.addremove=false
o = sul:option(FileUpload, "")
o.description =''..font_red..bold_on..translate("Manually download, unzip and rename clash core from links below and upload")..bold_off..font_off..' '
.."<br />"
..translatef("<a href=\"%s\" target=\"_blank\">" .. "Dreamacro clash core - clash</a>", translate("https://github.com/Dreamacro/clash/releases/latest"))
.."<br />"
..translatef("<a href=\"%s\" target=\"_blank\">" .. "Frainzy1477 clashr core - clash</a>", translate("https://github.com/frainzy1477/clash_dev/releases/latest"))
.."<br />"
..translatef("<a href=\"%s\" target=\"_blank\">" .. "comzyh clash tun core - clash(ctun)</a>", translate("https://github.com/comzyh/clash/releases/latest"))
.."<br />"
..translatef("<a href=\"%s\" target=\"_blank\">" .. "Dreamacro clash tun core - clash(premium)</a>", translate("https://github.com/Dreamacro/clash/releases/tag/premium"))


o.title = translate("  ")
o.template = "clash/upload_core"
um = sul:option(DummyValue, "", nil)
um.template = "clash/clash_dvalue"

local dir, fd,dtun,ctun,cssr
dir = "/etc/clash/"
dtun="/etc/clash/dtun/"
ctun="/etc/clash/clashtun/"

http.setfilehandler(
	function(meta, chunk, eof)
		local fp = HTTP.formvalue("file_type")
		if not fd then
			if not meta then return end
			
			if fp == "clash" then
			   if meta and chunk then fd = nixio.open(dir .. meta.file, "w") end
			elseif fp == "clashctun" then
			   if meta and chunk then fd = nixio.open(ctun .. meta.file, "w") end
			elseif fp == "clashdtun" then
			   if meta and chunk then fd = nixio.open(dtun .. meta.file, "w") end  
			end

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
			
			if fp == "clash" then
			    	SYS.exec("chmod 755 /etc/clash/clash 2>&1 &")
				SYS.exec("rm -rf /usr/share/clash/core_version 2>/dev/null && /etc/clash/clash -v | awk -F ' ' '{print $2}' >> /usr/share/clash/core_version 2>/dev/null")
				um.value = translate("File saved to") .. ' "/etc/clash/'..meta.file..'"'
			elseif fp == "clashctun" then
			    	SYS.exec("chmod 755 /etc/clash/clashtun/clash 2>&1 &")
				SYS.exec("rm -rf /usr/share/clash/tun_version 2>/dev/null && /etc/clash/clashtun/clash -v | awk -F ' ' '{print $2}' >> /usr/share/clash/tun_version 2>/dev/null")
				um.value = translate("File saved to") .. ' "/etc/clash/clashtun/'..meta.file..'"'
			elseif fp == "clashdtun" then
			    SYS.exec("chmod 755 /etc/clash/dtun/clash 2>&1 &")
				SYS.exec("rm -rf /usr/share/clash/dtun_core_version 2>/dev/null && /etc/clash/dtun/clash -v | awk -F ' ' '{print $2}' >> /usr/share/clash/dtun_core_version 2>/dev/null")
				um.value = translate("File saved to") .. ' "/etc/clash/dtun/'..meta.file..'"'  
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



m = Map("clash")
m:section(SimpleSection).template  = "clash/update"
m.pageaction = false

k = Map("clash")
s = k:section(TypedSection, "clash",translate("Download Online"))
s.anonymous = true
o = s:option(ListValue, "dcore", translate("Core Type"))
o.default = "clashcore"
o:value("1", translate("clash"))
o:value("3", translate("clash(ctun)"))
o:value("4", translate("clash(premium)"))



local cpu_model=SYS.exec("opkg status libc 2>/dev/null |grep 'Architecture' |awk -F ': ' '{print $2}' 2>/dev/null")
o = s:option(ListValue, "download_core", translate("Select Core"))
o.description = translate("CPU Model")..': '..font_green..bold_on..cpu_model..bold_off..font_off..' '
o:value("linux-386")
o:value("linux-amd64", translate("linux-amd64(x86-64)"))
o:value("linux-armv5")
o:value("linux-armv6")
o:value("linux-armv7")
o:value("linux-armv8")
o:value("linux-mips-hardfloat")
o:value("linux-mips-softfloat")
o:value("linux-mips64")
o:value("linux-mips64le")
o:value("linux-mipsle-softfloat")
o:value("linux-mipsle-hardfloat")


o=s:option(Button,"down_core")
o.inputtitle = translate("Save & Apply")
o.title = luci.util.pcdata(translate("Save & Apply"))
o.inputstyle = "reload"
o.write = function()
  k.uci:commit("clash")
end

o = s:option(Button,"download")
o.title = translate("Download")
o.template = "clash/core_check"


return m, ko,k


