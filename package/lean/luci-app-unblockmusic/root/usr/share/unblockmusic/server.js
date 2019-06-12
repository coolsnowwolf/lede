const fs = require('fs')
const net = require('net')
const path = require('path')
const parse = require('url').parse

const hook = require('./hook')
const request = require('./request')

const proxy = {
	core: {
		mitm: (req, res) => {
			if(req.url == '/proxy.pac'){
				let url = parse('http://' + req.headers.host)
				res.writeHead(200, {'Content-Type': 'application/x-ns-proxy-autoconfig'})
				res.end(`
					function FindProxyForURL(url, host) {
						if (${hook.target.host.map(host => (`host == '${host}'`)).join(' || ')}) {
							return 'PROXY ${url.hostname}:${url.port || 80}'
						}
						return 'DIRECT'
					}
				`)
			}
			else{
				const ctx = {res, req}
				Promise.resolve()
				.then(() => proxy.authenticate(ctx))
				.then(() => hook.request.before(ctx))
				.then(() => proxy.filter(ctx))
				.then(() => proxy.log(ctx))
				.then(() => proxy.mitm.request(ctx))
				.then(() => hook.request.after(ctx))
				.then(() => proxy.mitm.response(ctx))
				.catch(() => proxy.mitm.close(ctx))
			}
		},
		tunnel: (req, socket, head) => {
			const ctx = {req, socket, head}
			Promise.resolve()
			.then(() => proxy.authenticate(ctx))
			.then(() => hook.connect.before(ctx))
			.then(() => proxy.filter(ctx))
			.then(() => proxy.log(ctx))
			.then(() => proxy.tunnel.connect(ctx))
			.then(() => proxy.tunnel.handshake(ctx))
			.then(() => proxy.tunnel.pipe(ctx))
			.catch(() => proxy.tunnel.close(ctx))
		}
	},
	log: ctx => {
		const mark = {close: '|', blank: '-', proxy: '>'}[ctx.decision] || '>'
		if(ctx.socket)
			console.log('TUNNEL', mark, ctx.req.url)
		else
			console.log('MITM', mark, parse(ctx.req.url).host, ctx.req.socket.encrypted ? '(ssl)' : '')
	},
	authenticate: ctx => {
		const req = ctx.req
		const res = ctx.res
		const socket = ctx.socket
		let credential = Buffer.from((req.headers['proxy-authorization'] || '').split(/\s+/).pop() || '', 'base64').toString()
		if(server.authentication && credential != server.authentication){
			if(socket)
				socket.write('HTTP/1.1 407 Proxy Auth Required\r\nProxy-Authenticate: Basic realm="realm"\r\n\r\n')
			else
				res.writeHead(407, {'proxy-authenticate': 'Basic realm="realm"'})
			return Promise.reject(ctx.error = 'authenticate')
		}
	},
	filter: ctx => {
		const url = parse((ctx.socket ? 'https://' : '') + ctx.req.url)
		const match = pattern => url.href.search(new RegExp(pattern, 'g')) != -1
		if(!(ctx.decision || ctx.req.local)){
			try{
				let allow = server.whitelist.some(match)
				let deny = server.blacklist.some(match)
				// console.log('allow', allow, 'deny', deny)
				if(!allow && deny){
					return Promise.reject(ctx.error = 'filter')
				}
			}
			catch(error){
				ctx.error = error
			}
		}
	},
	mitm: {
		request: ctx => new Promise((resolve, reject) => {
			if(ctx.decision === 'close') return reject(ctx.error = ctx.decision)
			const req = ctx.req
			const url = parse(req.url)
			const options = request.configure(req.method, url, req.headers)
			ctx.proxyReq = request.create(url)(options)
			.on('response', proxyRes => {
				return resolve(ctx.proxyRes = proxyRes)
			})
			.on('error', error => {
				return reject(ctx.error = error)
			})
			req.readable ? req.pipe(ctx.proxyReq) : ctx.proxyReq.end(req.body)
		}),
		response: ctx => {
			const res = ctx.res
			const proxyRes = ctx.proxyRes
			res.writeHead(proxyRes.statusCode, proxyRes.headers)
			proxyRes.readable ? proxyRes.pipe(res) : res.end(proxyRes.body)
		},
		close: ctx => {
			ctx.res.socket.end()
		}
	},
	tunnel: {
		connect: ctx => new Promise((resolve, reject) => {
			if(ctx.decision === 'close') return reject(ctx.error = ctx.decision)
			const req = ctx.req
			const socket = ctx.socket
			const head = ctx.head
			const url = parse('https://' + req.url)
			socket.on('error', error => {
				return reject(ctx.error = error)
			})
			if(global.proxy && !req.local){
				const options = request.configure(req.method, url, req.headers)
				request.create(proxy)(options)
				.on('connect', (_, proxySocket) => {
					return resolve(ctx.proxySocket = proxySocket)
				})
				.on('error', error => {
					return reject(ctx.error = error)
				})
				.end()
			}
			else{
				const proxySocket = net.connect(url.port || 443, request.translate(url.hostname))
				.on('connect', () => {
					proxySocket.write(head)
					return resolve(ctx.proxySocket = proxySocket)
				})
				.on('error', error => {
					return reject(ctx.error = error)
				})
			}
		}),
		handshake: ctx => {
			const req = ctx.req
			const socket = ctx.socket
			const message = `HTTP/${req.httpVersion} 200 Connection established\r\n\r\n`
			socket.write(message)
		},
		pipe: ctx => new Promise((resolve, reject) => {
			if(ctx.decision === 'blank') return reject(ctx.error = ctx.decision)
			const socket = ctx.socket
			const proxySocket = ctx.proxySocket
			socket.pipe(proxySocket)
			proxySocket.pipe(socket)
		}),
		close: ctx => {
			ctx.socket.end()
		}
	}
}

const options = {
	key: fs.readFileSync(path.join(__dirname, 'server.key')),
	cert: fs.readFileSync(path.join(__dirname, 'server.crt'))
}

const server = {
	http: require('http').createServer().on('request', proxy.core.mitm).on('connect', proxy.core.tunnel),
	https: require('https').createServer(options).on('request', proxy.core.mitm).on('connect', proxy.core.tunnel)
}

server.whitelist = []
server.blacklist = ['//127\.\d+\.\d+\.\d+', '//localhost']
server.authentication = null

module.exports = server