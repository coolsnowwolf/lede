log = SimpleForm("logview")
log.submit = false
log.reset = false

t = log:field(DummyValue, '', '')
t.rawhtml = true
t.template = 'go-aliyundrive-webdav/go-aliyundrive-webdav_log'

return log
