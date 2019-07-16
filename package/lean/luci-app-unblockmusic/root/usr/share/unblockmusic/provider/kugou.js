const cache = require('../cache')
const insure = require('./insure')
const request = require('../request')

const search = info => {
	let url =
		'http://songsearch.kugou.com/song_search_v2?' +
		'keyword=' + encodeURIComponent(info.keyword) + '&page=1'

	return request('GET', url)
	.then(response => response.json())
	.then(jsonBody => {
		let matched = jsonBody.data.lists[0]
		if(matched)
			return matched.FileHash
		else
			return Promise.reject()
	})
	.catch(() => insure().kugou.search(info))
}

const track = id => {
	let url =
		'http://www.kugou.com/yy/index.php?r=play/getdata&hash=' + id

	return request('GET', url, {cookie: `kg_mid=${id.toLowerCase()}`})
	.then(response => response.json())
	.then(jsonBody => {
		if(jsonBody.status == '1')
			return jsonBody.data.play_url
		else
			return Promise.reject()
	})
}

const check = info => cache(search, info).then(track)

module.exports = {check, search}