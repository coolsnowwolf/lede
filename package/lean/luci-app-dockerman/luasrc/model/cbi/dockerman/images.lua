--[[
LuCI - Lua Configuration Interface
Copyright 2019 lisaac <https://github.com/lisaac/luci-app-dockerman>
]]--

require "luci.util"
local uci = luci.model.uci.cursor()
local docker = require "luci.model.docker"
local dk = docker.new()

local containers, images
local res = dk.images:list()
if res.code <300 then images = res.body else return end
res = dk.containers:list({query = {all=true}})
if res.code <300 then containers = res.body else return end

function get_images()
  local data = {}
  for i, v in ipairs(images) do
    local index = v.Created .. v.Id
    data[index]={}
    data[index]["_selected"] = 0
    data[index]["_id"] = v.Id:sub(8,20)
    if v.RepoTags then
      for i, v1 in ipairs(v.RepoTags) do
        data[index]["_tags"] =(data[index]["_tags"] and ( data[index]["_tags"] .. "<br\>" )or "") .. v1
      end
    else 
      _,_, data[index]["_tags"] = v.RepoDigests[1]:find("^(.-)@.+")
      data[index]["_tags"]=data[index]["_tags"]..":none"
    end
    for ci,cv in ipairs(containers) do
      if v.Id == cv.ImageID then
        data[index]["_containers"] = (data[index]["_containers"] and (data[index]["_containers"] .. " | ") or "")..
        "<a href=/cgi-bin/luci/admin/services/docker/container/"..cv.Id.." >".. cv.Names[1]:sub(2).."</a>"
      end
    end
    data[index]["_size"] = string.format("%.2f", tostring(v.Size/1024/1024)).."MB"
    data[index]["_created"] = os.date("%Y/%m/%d %H:%M:%S",v.Created)
  end
  return data
end

local image_list = get_images()

-- m = Map("docker", translate("Docker"))
m = SimpleForm("docker", translate("Docker"))
m.template = "dockerman/cbi/xsimpleform"
m.submit=false
m.reset=false

local pull_value={{_image_tag_name="", _registry="index.docker.io"}}
local pull_section = m:section(Table,pull_value, translate("Pull Image"))
pull_section.template="cbi/nullsection"
local tag_name = pull_section:option(Value, "_image_tag_name")
tag_name.template = "dockerman/cbi/inlinevalue"
tag_name.placeholder="hello-world:latest"
local registry = pull_section:option(Value, "_registry")
registry.template = "dockerman/cbi/inlinevalue"
registry:value("index.docker.io", "Docker Hub")
registry:value("hub-mirror.c.163.com", "163 Mirror")
registry:value("mirror.ccs.tencentyun.com", "Tencent Mirror")
registry:value("docker.mirrors.ustc.edu.cn", "USTC Mirror")
local action_pull = pull_section:option(Button, "_pull")
action_pull.inputtitle= translate("Pull")
action_pull.template = "dockerman/cbi/inlinebutton"
action_pull.inputstyle = "add"
tag_name.write = function(self, section,value)
  local hastag = value:find(":")
  if not hastag then
    value = value .. ":latest"
  end
  pull_value[section]["_image_tag_name"] = value
end
registry.write = function(self, section,value)
  pull_value[section]["_registry"] = value
end
action_pull.write = function(self, section)
  local tag = pull_value[section]["_image_tag_name"]
  local server = pull_value[section]["_registry"]
  --去掉协议前缀和后缀
  local _,_,tmp = server:find(".-://([%.%w%-%_]+)")
  if not tmp then
    _,_,server = server:find("([%.%w%-%_]+)")
  end
  local json_stringify = luci.jsonc and luci.jsonc.stringify
  if tag and tag ~= "" then
    docker:clear_status()
    docker:append_status("Images: " .. "pulling" .. " " .. tag .. "...")
    local x_auth = nixio.bin.b64encode(json_stringify({serveraddress= server}))
    local res = dk.images:create({query = {fromImage=tag}, header={["X-Registry-Auth"] = x_auth}})
    if res and res.code >=300 then
      docker:append_status("fail code:" .. res.code.." ".. (res.body.message and res.body.message or res.message).. "<br>")
    else
      docker:append_status("done<br>")
    end
  else
    docker:append_status("fail code: 400 please input the name of image name!")
  end
  luci.http.redirect(luci.dispatcher.build_url("admin/services/docker/images"))
end

image_table = m:section(Table, image_list, translate("Images"))

image_selecter = image_table:option(Flag, "_selected","")
image_selecter.disabled = 0
image_selecter.enabled = 1
image_selecter.default = 0

image_id = image_table:option(DummyValue, "_id", translate("ID"))
image_table:option(DummyValue, "_tags", translate("RepoTags")).rawhtml = true
image_table:option(DummyValue, "_containers", translate("Containers")).rawhtml = true
image_table:option(DummyValue, "_size", translate("Size"))
image_table:option(DummyValue, "_created", translate("Created"))
image_selecter.write = function(self, section, value)
  image_list[section]._selected = value
end

local remove_action = function(force)
  local image_selected = {}
  -- 遍历table中sectionid
  local image_table_sids = image_table:cfgsections()
  for _, image_table_sid in ipairs(image_table_sids) do
    -- 得到选中项的名字
    if image_list[image_table_sid]._selected == 1 then
      image_selected[#image_selected+1] = image_id:cfgvalue(image_table_sid)
    end
  end
  if next(image_selected) ~= nil then
    local success = true
    docker:clear_status()
    for _,img in ipairs(image_selected) do
      docker:append_status("Images: " .. "remove" .. " " .. img .. "...")
      local query
      if force then query = {force = true} end
      local msg = dk.images:remove({id = img, query = query})
      if msg.code ~= 200 then
        docker:append_status("fail code:" .. msg.code.." ".. (msg.body.message and msg.body.message or msg.message).. "<br>")
        success = false
      else
        docker:append_status("done<br>")
      end
    end
    if success then docker:clear_status() end
    luci.http.redirect(luci.dispatcher.build_url("admin/services/docker/images"))
  end
end

docker_status = m:section(SimpleSection)
docker_status.template = "dockerman/apply_widget"
docker_status.err=nixio.fs.readfile(dk.options.status_path)
if docker_status.err then docker:clear_status() end

action = m:section(Table,{{}})
action.notitle=true
action.rowcolors=false
action.template="cbi/nullsection"

btnremove = action:option(Button, "remove")
btnremove.inputtitle= translate("Remove")
btnremove.template = "dockerman/cbi/inlinebutton"
btnremove.inputstyle = "remove"
btnremove.forcewrite = true
btnremove.write = function(self, section)
  remove_action()
end

btnforceremove = action:option(Button, "forceremove")
btnforceremove.inputtitle= translate("Force Remove")
btnforceremove.template = "dockerman/cbi/inlinebutton"
btnforceremove.inputstyle = "remove"
btnforceremove.forcewrite = true
btnforceremove.write = function(self, section)
  remove_action(true)
end
return m
