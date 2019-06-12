const cache = require('../cache')
const insure = require('./insure')
const request = require('../request')

let headers = {
	'origin': 'http://www.joox.com',
	'referer': 'http://www.joox.com'
}

const fit = info => {
	if(/[\u0800-\u4e00]/.test(info.name)) //is japanese
		return info.name
	else
		return info.keyword
}

const search = info => {
	let keyword = fit(info)
	let url =
		'http://api-jooxtt.sanook.com/web-fcgi-bin/web_search?' +
		'country=hk&lang=zh_TW&' +
		'search_input=' + encodeURIComponent(keyword) + '&sin=0&ein=30'

	return request('GET', url, headers)
	.then(response => response.body())
	.then(body => {
		let jsonBody = JSON.parse(body.replace(/(\')/g, '"'))
		let matched = jsonBody.itemlist[0]
		if(matched)
			return matched.songid
		else
			return Promise.reject()
	})
}

const track = id => {
	let url =
		'http://api.joox.com/web-fcgi-bin/web_get_songinfo?' +
		'songid=' + id + '&country=hk&lang=zh_cn&from_type=-1&' +
		'channel_id=-1&_=' + (new Date).getTime()

	return request('GET', url, headers)
	.then(response => response.jsonp())
	.then(jsonBody => {
		let songUrl = (jsonBody.r320Url || jsonBody.r192Url || jsonBody.mp3Url || jsonBody.m4aUrl).replace(/M\d00([\w]+).mp3/, 'M800$1.mp3')
		if(songUrl)
			return songUrl
		else
			return Promise.reject()
	})
	.catch(() => insure().joox.track(id))
}

const check = info => cache(search, info).then(track)

module.exports = {check, track}