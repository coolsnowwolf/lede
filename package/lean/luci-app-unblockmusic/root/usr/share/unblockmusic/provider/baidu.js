const cache = require('../cache')
const insure = require('./insure')
const request = require('../request')

const search = info => {
	let url =
		'http://sug.qianqian.com/info/suggestion?' +
		'word=' + encodeURIComponent(info.keyword) + '&version=2&from=0'

	return request('GET', url)
	.then(response => response.json())
	.then(jsonBody => {
		if('data' in jsonBody){
			let matched = jsonBody.data.song[0]
			return matched.songid
		}
		else{
			return Promise.reject()
		}
	})
}

const track = id => {
	let url =
		'http://music.taihe.com/data/music/fmlink?' +
		'songIds=' + id + '&type=mp3'

	return request('GET', url)
	.then(response => response.json())
	.then(jsonBody => {
		if('songList' in jsonBody.data)
			return jsonBody.data.songList[0].songLink
		else
			return Promise.reject()
	})
	.catch(() => insure().baidu.track(id))
}

const check = info => cache(search, info).then(track)

module.exports = {check}