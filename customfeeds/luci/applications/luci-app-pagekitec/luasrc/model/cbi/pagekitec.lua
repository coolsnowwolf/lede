m = Map("pagekitec", translate("PageKite"),
    translate([[
<p/>Note: you need a working PageKite account, or at least, your own running front end for this form to work.
Visit <a href="https://pagekite.net/home/">your account</a> to set up a name for your
router and get a secret key for the connection.
<p/><em>Note: this web configurator only supports
some very very basic uses of pagekite.</em>
]]))

s = m:section(TypedSection, "pagekitec", translate("PageKite"))
s.anonymous = true

p = s:option(Value, "kitename", translate("Kite Name"))
p = s:option(Value, "kitesecret", translate("Kite Secret"))
p.password = true
p = s:option(Flag, "static", translate("Static Setup"),
	translate([[Static setup, disable FE failover and DDNS updates, set this if you are running your
	own frontend without a pagekite.me account]]))

p = s:option(Flag, "simple_http", translate("Basic HTTP"),
    translate([[Enable a tunnel to the local HTTP server (in most cases, this admin
site)]]))
p = s:option(Flag, "simple_ssh", translate("Basic SSH"),
    translate([[Enable a tunnel to the local SSH server]]))

return m
