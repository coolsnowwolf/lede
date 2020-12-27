module("luci.controller.passwall", package.seeall)

function index()
  if not nixio.fs.access("/etc/config/passwall") then
    return
  end
  entry({"admin", "services", "passwall"}, firstchild(), "PassWall Plus", 99).dependent = true
  entry({"admin", "services", "passwall", "main"},cbi("passwall/main"), _("Main"), 10).leaf = true
  entry({"admin", "services", "passwall", "proxy"},cbi("passwall/proxy"), _("Server"), 15).leaf = true
  entry({"admin", "services", "passwall", "check"},call("action_status")).leaf = true
  entry({"admin", "services", "passwall", "info"},call("from_info")).leaf = true
end

local function is_running()
  local status = luci.sys.exec('sh /etc/init.d/passwall check')
  if (status == nil or status == '') then
    return true
  else
    return status
  end
end

local function health_check(url)
  local time_total = luci.sys.exec('sh /etc/init.d/passwall link '..url)
  if (string.len(time_total) >= 5) then
    return tonumber(time_total, 10)
  else
    return 0.0000
  end
end

local function from_addr(addr)
  local data_total = luci.sys.exec('sh /etc/init.d/passwall from '..addr)
  if (string.len(data_total) > 0) then
    return data_total
  else
    return false
  end
end

function action_status()
  --io.write("Content-type: text/html\nPragma: no-cache\n\n")
  --io.write("hello")

  luci.http.prepare_content("application/json")
  luci.http.write_json({
    switch = is_running()
  })
end

function from_info()
  luci.http.prepare_content("application/json")
  luci.http.write_json({
    baidu = health_check('https://www.baidu.com'),
    google = health_check('https://www.google.com/ncr'),
    china = from_addr('china'),
    foreign = from_addr('foreign')
  })
end