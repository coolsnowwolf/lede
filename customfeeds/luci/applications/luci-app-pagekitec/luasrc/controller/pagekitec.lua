-- Copyright 2018 Karl Palsson <karlp@tweak.net.au>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.pagekitec", package.seeall)

function index()
    entry({"admin", "services", "pagekitec"}, cbi("pagekitec"), _("PageKite"))
end
