const uriKey = '3go8&$8*3*3h0k(2)2'

export default {
	uri: {
		retrieve: id => {
			id = id.toString().trim()
			let string = Array.from(Array(id.length).keys()).map(index => String.fromCharCode(id.charCodeAt(index) ^ uriKey.charCodeAt(index % uriKey.length))).join('')
			let result = CryptoJS.MD5(string).toString(CryptoJS.enc.Base64).replace(/\//g, '_').replace(/\+/g, '-')
			return `http://p1.music.126.net/${result}/${id}`
		}
	},
	md5: {
		digest: value => CryptoJS.MD5(value).toString()
	}
}