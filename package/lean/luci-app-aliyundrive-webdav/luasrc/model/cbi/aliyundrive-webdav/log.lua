log = SimpleForm("logview")
log.submit = false
log.reset = false

t = log:field(DummyValue, '', '')
t.rawhtml = true
t.template = 'aliyundrive-webdav/aliyundrive-webdav_log'

return log
