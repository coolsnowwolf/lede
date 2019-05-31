--teasiu<teasiu@163.com>
local fs = require "nixio.fs"
local sys = require "luci.sys"
m = Map("fileconfiguration", translate("脚本文件配置"), translate("各类服务内置脚本文档的直接编辑,除非你知道自己在干什么,否则请不要轻易修改这些配置文档"))
s = m:section(TypedSection, "fileconfiguration")
s.anonymous=true
--SSR
s:tab("config", translate("SSR配置文件"),translate(""))
conf = s:taboption("config", Value, "editconf", nil, translate("开头的数字符号（＃）或分号的每一行（;）被视为注释；删除（;）启用指定选项。"))
conf.template = "cbi/tvalue"
conf.rows = 20
conf.wrap = "off"
function conf.cfgvalue(self, section)
        return fs.readfile("/etc/config/shadowsocksr") or ""
end
function conf.write(self, section, value)
        if value then
                value = value:gsub("\r\n?", "\n")
                fs.writefile("/tmp/shadowsocksr", value)
                if (luci.sys.call("cmp -s /tmp/shadowsocksr /etc/config/shadowsocksr") == 1) then
                        fs.writefile("/etc/config/shadowsocksr", value)
			luci.sys.call("/etc/init.d/shadowsocksr restart >/dev/null")  --这里增加了一条重启shadowsocksr 的命令
                end
                fs.remove("/tmp/shadowsocksr")
        end
end
--启用路由表
if nixio.fs.access("/etc/config/enableroutingtable") then
s:tab("config2", translate("启用路由表"),translate(""))
conf = s:taboption("config2", Value, "editconf2", nil, translate("开头的数字符号（＃）或分号的每一行（;）被视为注释；删除（;）启用指定选项。"))
conf.template = "cbi/tvalue"
conf.rows = 20
conf.wrap = "off"
function conf.cfgvalue(self, section)
    return fs.readfile("/etc/config/enableroutingtable") or ""
end
function conf.write(self, section, value)
    if value then
        value = value:gsub("\r\n?", "\n")
        fs.writefile("/tmp/enableroutingtable", value)
        if (luci.sys.call("cmp -s /tmp/enableroutingtable /etc/config/enableroutingtable") == 1) then
            fs.writefile("/etc/config/enableroutingtable", value)
            luci.sys.call("/etc/init.d/shadowsocksr restart >/dev/null")  --这里增加了一条重启shadowsocksr 的命令
        end
        fs.remove("/tmp/enableroutingtable")
    end
end
end
--禁用路由表
if nixio.fs.access("/etc/config/disableroutingtable") then
s:tab("config3", translate("禁用路由表"),translate(""))
conf = s:taboption("config3", Value, "editconf3", nil, translate("开头的数字符号（＃）或分号的每一行（;）被视为注释；删除（;）启用指定选项。"))
conf.template = "cbi/tvalue"
conf.rows = 20
conf.wrap = "off"
function conf.cfgvalue(self, section)
    return fs.readfile("/etc/config/disableroutingtable") or ""
end
function conf.write(self, section, value)
    if value then
        value = value:gsub("\r\n?", "\n")
        fs.writefile("/tmp/disableroutingtable", value)
        if (luci.sys.call("cmp -s /tmp/disableroutingtable /etc/config/disableroutingtable") == 1) then
            fs.writefile("/etc/config/disableroutingtable", value)
            luci.sys.call("/etc/init.d/shadowsocksr restart >/dev/null")  --这里增加了一条重启shadowsocksr 的命令
        end
        fs.remove("/tmp/disableroutingtable")
    end
end
end
--待添加
return m