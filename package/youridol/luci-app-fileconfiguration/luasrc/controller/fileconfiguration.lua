module("luci.controller.fileconfiguration", package.seeall)
function index()
        if not nixio.fs.access("/etc/config/fileconfiguration") then
                return
        end
        entry({"admin", "services", "fileconfiguration"}, cbi("fileconfiguration"), _("文件配置"),109).dependent = true
end
