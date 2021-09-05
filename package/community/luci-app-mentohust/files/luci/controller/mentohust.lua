module("luci.controller.mentohust", package.seeall)

function index()
    if not nixio.fs.access("/etc/config/mentohust") then
        return
    end
    if luci.sys.call("command -v mentohust >/dev/null") ~= 0 then
        return
    end
    entry({"admin", "services", "mentohust"},
        alias("admin", "services", "mentohust", "general"),
        _("MentoHUST"), 10).dependent = true

    entry({"admin", "services", "mentohust", "general"}, cbi("mentohust/general"), _("MentoHUST Settings"), 10).leaf = true
    entry({"admin", "services", "mentohust", "log"}, cbi("mentohust/log"), _("MentoHUST LOG"), 20).leaf = true
end
