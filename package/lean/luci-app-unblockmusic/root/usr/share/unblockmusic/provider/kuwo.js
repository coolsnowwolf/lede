const cache = require('../cache')
const insure = require('./insure')
const request = require('../request')

const search = info => {
	let url =
		// 'http://search.kuwo.cn/r.s?' +
		// 'ft=music&itemset=web_2013&client=kt&' +
		// 'rformat=json&encoding=utf8&' +
		// 'all=' + encodeURIComponent(info.keyword) + '&pn=0&rn=20'
		'http://search.kuwo.cn/r.s?' +
		'ft=music&rformat=json&encoding=utf8&' +
		'rn=8&callback=song&vipver=MUSIC_8.0.3.1&' +
		'SONGNAME=' + encodeURIComponent(info.name) + '&' +
		'ARTIST=' + encodeURIComponent(info.artists[0].name)

	return request('GET', url)
	.then(response => response.body())
	.then(body => {
		let jsonBody = JSON.parse(body.replace(/\'/g, '"').replace('try {var jsondata =', '').replace(';song(jsondata);}catch(e){jsonError(e)}', ''))
		let matched = jsonBody.abslist[0]
		if(matched)
			return matched.MUSICRID.split('_').pop()
		else
			return Promise.reject()
	})
}

const track = id => {
	let url =
		'http://antiserver.kuwo.cn/anti.s?' +
		'type=convert_url&format=mp3&response=url&rid=MUSIC_' + id
		// 'type=convert_url&format=aac|mp3|wma&response=url&rid=MUSIC_' + id

	return request('GET', url)
	.then(response => response.body())
	.then(body => {
		if(body.startsWith('http'))
			return body
		else
			return Promise.reject()
	})
	.catch(() => insure().kuwo.track(id))
}

const check = info => cache(search, info).then(track)

module.exports = {check, track}