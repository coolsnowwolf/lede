#!/usr/bin/env node

const package = require('./package.json')
const config = require('./cli.js')
.program({name: package.name, version: package.version})
.option(['-v', '--version'], {action: 'version'})
.option(['-p', '--port'], {metavar: 'port', help: 'specify server port'})
.option(['-u', '--proxy-url'], {metavar: 'url', help: 'request through upstream proxy'})
.option(['-f', '--force-host'], {metavar: 'host', help: 'force the netease server ip'})
.option(['-o', '--match-order'], {metavar: 'source', nargs: '+', help: 'set priority of sources'})
.option(['-t', '--token'], {metavar: 'token', help: 'set up http basic authentication'})
.option(['-e', '--endpoint'], {metavar: 'url', help: 'replace virtual endpoint with public host'})
.option(['-s', '--strict'], {action: 'store_true', help: 'enable proxy limitation'})
.option(['-h', '--help'], {action: 'help'})
.parse(process.argv)

config.port = (config.port || '8080').split(':').map(string => parseInt(string))
const invalid = value => (isNaN(value) || value < 1 || value > 65535)
if(config.port.some(invalid)){
	console.log('Port must be a number higher than 0 and lower than 65535.')
	process.exit(1)
}
if(config.proxyUrl && !/http(s?):\/\/.+:\d+/.test(config.proxyUrl)){
	console.log('Please check the proxy url.')
	process.exit(1)
}
if(config.endpoint && !/http(s?):\/\/.+/.test(config.endpoint)){
	console.log('Please check the endpoint host.')
	process.exit(1)
}
if(config.forceHost && !/\d+\.\d+\.\d+\.\d+/.test(config.forceHost)){
	console.log('Please check the server host.')
	process.exit(1)
}
if(config.matchOrder){
	const provider = ['netease', 'qq', 'xiami', 'baidu', 'kugou', 'kuwo', 'migu', 'joox']
	const candidate = config.matchOrder
	if(candidate.some((key, index) => index != candidate.indexOf(key))){
		console.log('Please check the duplication in match order.')
		process.exit(1)
	}
	else if(candidate.some(key => !provider.includes(key))){
		console.log('Please check the availability of match sources.')
		process.exit(1)
	}
	global.source = candidate
}
if(config.token && !/\S+:\S+/.test(config.token)){
	console.log('Please check the authentication token.')
	process.exit(1)
}

const parse = require('url').parse
const hook = require('./hook')
const server = require('./server')
const escape = string => string.replace(/\./g, '\\.')

global.port = config.port
global.proxy = config.proxyUrl ? parse(config.proxyUrl) : null
global.hosts = {}, hook.target.host.forEach(host => global.hosts[host] = config.forceHost)
server.whitelist = ['music.126.net', 'vod.126.net'].map(escape)
if(config.strict) server.blacklist.push('.*')
server.authentication = config.token || null
global.endpoint = config.endpoint
if(config.endpoint) server.whitelist.push(escape(config.endpoint))

const dns = host => new Promise((resolve, reject) => require('dns').lookup(host, {all: true}, (error, records) => error? reject(error) : resolve(records.map(record => record.address))))
const httpdns = host => require('./request')('POST', 'http://music.httpdns.c.163.com/d', {}, host).then(response => response.json()).then(jsonBody => jsonBody.dns[0].ips)

Promise.all([httpdns(hook.target.host[0])].concat(hook.target.host.map(host => dns(host))))
.then(result => {
	let extra = Array.from(new Set(result.reduce((merged, array) => merged.concat(array), [])))
	hook.target.host = hook.target.host.concat(extra)
	server.whitelist = server.whitelist.concat(hook.target.host.map(escape))
	if(port[0]){
		server.http.listen(port[0])
		console.log(`HTTP Server running @ http://0.0.0.0:${port[0]}`)
	}
	if(port[1]){
		server.https.listen(port[1])
		console.log(`HTTPS Server running @ https://0.0.0.0:${port[1]}`)
	}
})
.catch(error => console.log(error))