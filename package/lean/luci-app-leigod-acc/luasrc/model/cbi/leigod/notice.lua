local uci                         = require "luci.model.uci".cursor()

-- config
m                                 = Map("accelerator")
m.title                           = translate("Leigod Notice")
m.description                     = translate("Leigod Accelerator Usage")

m:section(SimpleSection).template = "leigod/notice"

return m
