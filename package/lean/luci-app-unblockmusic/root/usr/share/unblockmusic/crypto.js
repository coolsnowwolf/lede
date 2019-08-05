'use strict'

const crypto = require('crypto')
const bodyify = require('querystring').stringify
const parse = require('url').parse
const uriKey = '3go8&$8*3*3h0k(2)2'
const eapiKey = 'e82ckenh8dichen8'
const linuxapiKey = 'rFgB&h#%2?^eDg:Q'

const decrypt = (buffer, key) => {
	let decipher = crypto.createDecipheriv('aes-128-ecb', key, '')
	return Buffer.concat([decipher.update(buffer), decipher.final()])
}

const encrypt = (buffer, key) => {
	let cipher = crypto.createCipheriv('aes-128-ecb', key, '')
	return Buffer.concat([cipher.update(buffer), cipher.final()])
}

module.exports = {
	eapi: {
		encrypt: buffer => encrypt(buffer, eapiKey),
		decrypt: buffer => decrypt(buffer, eapiKey),
		encryptRequest: (url, object) => {
			url = parse(url)
			let text = JSON.stringify(object)
			let message = `nobody${url.path}use${text}md5forencrypt`
			let digest = crypto.createHash('md5').update(message).digest('hex')
			let data = `${url.path}-36cd479b6b5-${text}-36cd479b6b5-${digest}`
			return {
				url: url.href.replace(/\w*api/, 'eapi'),
				body: bodyify({
					params: encrypt(Buffer.from(data), eapiKey).toString('hex').toUpperCase()
				})
			}
		}
	},
	linuxapi: {
		encrypt: buffer => encrypt(buffer, linuxapiKey),
		decrypt: buffer => decrypt(buffer, linuxapiKey),
		encryptRequest: (url, object) => {
			url = parse(url)
			let text = JSON.stringify({method: 'POST', url: url.href, params: object})
			return {
				url: url.resolve('/api/linux/forward'),
				body: bodyify({
					eparams: encrypt(Buffer.from(text), linuxapiKey).toString('hex').toUpperCase()
				})
			}
		}
	},
	miguapi: {
		encrypt: object => {
			let text = JSON.stringify(object)
			const derive = (password, salt, keyLength, ivSize) => { // EVP_BytesToKey
				salt = salt || Buffer.alloc(0)
				let keySize = keyLength / 8
				let repeat = Math.ceil((keySize + ivSize * 8) / 32)
				let buffer = Buffer.concat(Array.from(Array(repeat).keys()).reduce(result =>
					result.concat(crypto.createHash('md5').update(Buffer.concat([result.slice(-1)[0], password, salt])).digest())
				, [Buffer.alloc(0)]))
				return {
					key: buffer.slice(0, keySize),
					iv: buffer.slice(keySize, keySize + ivSize)
				}
			}
			let password = Buffer.from(crypto.randomBytes(32).toString('hex')), salt = crypto.randomBytes(8),
			key = '-----BEGIN PUBLIC KEY-----\nMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC8asrfSaoOb4je+DSmKdriQJKWVJ2oDZrs3wi5W67m3LwTB9QVR+cE3XWU21Nx+YBxS0yun8wDcjgQvYt625ZCcgin2ro/eOkNyUOTBIbuj9CvMnhUYiR61lC1f1IGbrSYYimqBVSjpifVufxtx/I3exReZosTByYp4Xwpb1+WAQIDAQAB\n-----END PUBLIC KEY-----'
			let secret = derive(password, salt, 256, 16)
			let cipher = crypto.createCipheriv('aes-256-cbc', secret.key, secret.iv)
			return bodyify({
				data: Buffer.concat([Buffer.from('Salted__'), salt, cipher.update(Buffer.from(text)), cipher.final()]).toString('base64'),
				secKey: crypto.publicEncrypt({key, padding: crypto.constants.RSA_PKCS1_PADDING}, password).toString('base64')
			})
		}
	},
	base64: {
		encode: text => Buffer.from(text).toString('base64').replace(/\+/g, '-').replace(/\//g, '_'),
		decode: text => Buffer.from(text.replace(/-/g, '+').replace(/_/g, '/'), 'base64').toString('ascii')
	},
	uri: {
		retrieve: id => {
			id = id.toString().trim()
			let string = Array.from(Array(id.length).keys()).map(index => String.fromCharCode(id.charCodeAt(index) ^ uriKey.charCodeAt(index % uriKey.length))).join('')
			let result = crypto.createHash('md5').update(string).digest('base64').replace(/\//g, '_').replace(/\+/g, '-')
			return `http://p1.music.126.net/${result}/${id}`
		}
	},
	md5: {
		digest: value => crypto.createHash('md5').update(value).digest('hex'),
		pipe: source => new Promise((resolve, reject) => {
			let digest = crypto.createHash('md5').setEncoding('hex')
			source.pipe(digest)
			.on('error', error => reject(error))
			.once('finish', () => resolve(digest.read()))
		})
	}
}