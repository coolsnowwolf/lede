const find = require('./find')
const crypto = require('../crypto')
const request = require('../request')

const provider = {
	netease: require('./netease'),
	qq: require('./qq'),
	xiami: require('./xiami'),
	baidu: require('./baidu'),
	kugou: require('./kugou'),
	kuwo: require('./kuwo'),
	migu: require('./migu'),
	joox: require('./joox')
}

const match = (id, source) => {
	let meta = {}
	let candidate = (source || global.source || ['netease', 'qq', 'xiami', 'baidu']).filter(name => name in provider)
	return find(id)
	.then(info => {
		meta = info
		return Promise.all(candidate.map(name => provider[name].check(info).catch(() => {})))
	})
	.then(urls => {
		urls = urls.filter(url => url)
		return Promise.all(urls.map(url => check(url)))
	})
	.then(songs => {
		songs = songs.filter(song => song.url)
		if(!songs.length) return Promise.reject()
		console.log(`[${meta.id}] ${meta.name}\n${songs[0].url}`)
		return songs[0]
	})
}

const check = url => {
	let song = {size: 0, url: null, md5: null}
	return request('HEAD', url)
	.then(response => {
		if(response.statusCode != 200) return
		if(url.includes('qq.com'))
			song.md5 = response.headers['server-md5']
		else if(url.includes('xiami.net') || url.includes('qianqian.com'))
			song.md5 = response.headers['etag'].replace(/"/g, '').toLowerCase()
		song.size = parseInt(response.headers['content-length']) || 0
		song.url = response.url.href
	})
	.catch(() => {})
	.then(() => song)
}

module.exports = match