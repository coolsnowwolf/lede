local ubus = require "ubus"
local u = ubus.connect()
local b = u:call("system", "board", {})

local labels = {
    board_name = b.board_name,
    id = b.release.distribution,
    model = b.model,
    release = b.release.version,
    revision = b.release.revision,
    system = b.system,
    target = b.release.target
}

b = nil
u = nil
ubus = nil

local function scrape()
    metric("node_openwrt_info", "gauge", labels, 1)
end

return { scrape = scrape }

