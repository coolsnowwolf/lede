uci = require "luci.model.uci".cursor()
local fs = require "nixio.fs"
local json = require "luci.jsonc"
m = Map("fogvdn", translate("FOGVDN Node"))
s = m:section(NamedSection, "main", "main", translate("Main"))

act_status = s:option(DummyValue, "act_status", translate("Status"))
act_status.template = "pcdn/act_status"

enabled = s:option(Flag, "enable", translate("Enable"))
enabled.default = 0

node_info_file = "/etc/pear/pear_monitor/node_info.json"
if fs.access(node_info_file) then
    local node_info = fs.readfile(node_info_file)
    node_info = json.parse(node_info)
    for k,v in pairs(node_info) do

node_info_file = "/etc/pear/pear_monitor/node_info.json"
if fs.access(node_info_file) then
    local node_info = fs.readfile(node_info_file)
    node_info = json.parse(node_info)
    for k,v in pairs(node_info) do
        if k == "node_id" then
            option = s:option(DummyValue, "_"..k,translate(k))
            option.value = v
        end
    end
end

storage_info_file = "/etc/pear/pear_monitor/storage_info.json"
if fs.access(storage_info_file) then
    local storage_info = fs.readfile(storage_info_file)
    storage_info = json.parse(storage_info)
    for k,v in pairs(storage_info) do
        if k == "os_drive_serial" then
            option = s:option(DummyValue, "_"..k,translate(k))
            option.value = v
        end
    end
end

openfog_link=s:option(DummyValue, "openfog_link", translate("<input type=\"button\" class=\"cbi-button cbi-button-apply\" value=\"Openfogos.com\" onclick=\"window.open('https://openfogos.com/')\" />"))
openfog_link.description = translate("OpenFogOS Official Website")

s = m:section(TypedSection, "instance", translate("Settings"))
s.anonymous = true
s.description = translate("Fogvdn Settings")

username = s:option(Value, "username", translate("username"))
username.description = translate("<input type=\"button\" class=\"cbi-button cbi-button-apply\" value=\"Register\" onclick=\"window.open('https://account.openfogos.com/signup?source%3Dopenfogos.com%26')\" />")

region = s:option(Value, "region", translate("Region"))
region.optional = true
region.template="cbi/city"

isp = s:option(Value, "isp", translate("ISP"))
isp.optional = true
isp:value("电信",  translate("China Telecom"))
isp:value("移动",  translate("China Mobile"))
isp:value("联通",  translate("China Unicom"))

per_line_up_bw = s:option(Value, "per_line_up_bw", translate("Per Line Up BW"))
per_line_up_bw.template = "cbi/digitonlyvalue"
per_line_up_bw.datatype = "uinteger"

per_line_down_bw = s:option(Value, "per_line_down_bw", translate("Per Line Down BW"))
per_line_down_bw.template = "cbi/digitonlyvalue"
per_line_down_bw.datatype = "uinteger"

limited_memory = s:option(Value, "limited_memory", translate("Limited Memory"))
limited_memory.optional = true
limited_memory.template = "cbi/digitonlyvalue"
limited_memory.datatype = "range(0, 100)"
-- 0-100%
limited_storage = s:option(Value, "limited_storage", translate("Limited Storage"))
limited_storage.optional = true
limited_storage.template = "cbi/digitonlyvalue"
limited_storage.datatype = "range(0, 100)"
-- 0-100%

limited_area = s:option(Value, "limited_area", translate("Limited Area"))
limited_area.default = "2"
limited_area:value("-1", "不设置：若不确定运营商对流量出省的限制情况，建议选择该项")
limited_area:value("0", "全国调度：流量将会向全国调度，出省比例较高，可能会导致运营商的限制")
limited_area:value("1", "本省调度：流量只会向所在省份内调度，是比较安全的调度模式，但跑量可能会降低")
limited_area:value("2", "大区调度：流量只会向所在大区内调度，出省比例较低，是安全性和跑量之间较为均衡的模式")
-- 限制地区 -1 不设置（采用openfogos默认） 0 全国调度，1 省份调度，2 大区调度

nics = s:option(DynamicList,"nics",translate("netdev"))
-- uci:foreach("multiwan","multiwan",function (instance)
--     nics:value(instance["tag"])
-- end
-- )
--list /sys/class/net, filter bridge device
cmd="/usr/share/pcdn/check_netdev get_netdevs"
json_dump=luci.sys.exec(cmd)
devs=json.parse(json_dump)
for k,v in pairs(devs) do
    nics:value(k,k.." ["..v.."]")
end

storage = s:option(DynamicList, "storage", translate("Storage"))
storage.default = "/opt/openfogos"
storage.description = translate("Warnning: System directory is not allowed!")
--filter start with /etc /usr /root /var /tmp /dev /proc /sys /overlay /rom and root
mount_point = {}
cmd="/usr/share/pcdn/check_mount_ponit mount_point"
json_dump=luci.sys.exec(cmd)
mount_point=json.parse(json_dump)
for k,v in pairs(mount_point) do
    storage:value(k,k.."("..v..")")
end

btn = s:option(Button, "_filter", "")
btn.inputtitle = "应用最佳存储设置"
btn.description = "为避免木桶效应，点击估算最佳设置"
btn.inputstyle = "apply"

-- 获取表中元素的索引
table.indexOf = function(tbl, value)
    for i, v in ipairs(tbl) do
        if v == value then return i end
    end
    return -1
end

-- 拷贝表
table.copy = function(tbl)
    local copy = {}
    for k, v in pairs(tbl) do
        copy[k] = v
    end
    return copy
end

-- 解析并转换大小单位
function parseSize(size)
    if type(size) ~= "string" then return 0 end

    local unit = {"B", "K", "M", "G", "T"}

    local num, u = string.match(size, "^(%d+%.?%d*)([KMGT]?)$")
    if not num then return 0 end

    local i = table.indexOf(unit, u)

    -- 如果没有找到单位，默认为字节
    if i == -1 then
        return tonumber(num)
    else
        return tonumber(num) * (1024 ^ i)
    end
end

-- 获取最佳存储方案
function getOptimalSolution(numsObjArray)
    if type(numsObjArray) ~= "table" or #numsObjArray == 0 then return {} end

    -- 拷贝并按大小排序
    local nums = {}
    for _, v in ipairs(numsObjArray) do
        table.insert(nums, {name = v.name, size = v.size})
    end
    table.sort(nums, function(a, b)
        return parseSize(a.size) < parseSize(b.size)
    end)

    local max = 0
    local optimal = {}

    while #nums > 0 do
        local temp = parseSize(nums[1].size) * #nums
        if temp > max then
            max = temp
            optimal = table.copy(nums)
        end
        table.remove(nums, 1)  -- 移除第一个元素
    end

    -- 提取名称并排序
    local result = {}
    for _, v in ipairs(optimal) do
        table.insert(result, v.name)
    end
    table.sort(result)

    return result
end

function btn.write(self, section)
    local numsObjArray = {}

    for k,v in pairs(mount_point) do
        table.insert(numsObjArray, {name = k, size = v})
    end

    local optimalStorage = getOptimalSolution(numsObjArray)

    self.map:set(section, "storage", optimalStorage)
end

return m
