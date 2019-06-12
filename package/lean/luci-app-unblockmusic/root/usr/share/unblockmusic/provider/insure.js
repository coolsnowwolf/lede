const request = require('../request')
const host = 'https://public.nondanee.tk'

module.exports = () => {
	const proxy = new Proxy(() => {}, {
		get: (target, property) => {
			target.route = (target.route || []).concat(property)
			return proxy
		},
		apply: (target, _, payload) => {
			let path = target.route.join('/'), query = payload[0]
			query = encodeURIComponent(typeof(query) === 'object' ? JSON.stringify(query) : query)
			if(path != 'qq/ticket') return Promise.reject()
			return request('GET', `${host}/${path}?${query}`)
			.then(response => response.body())
		}
	})
	return proxy
}