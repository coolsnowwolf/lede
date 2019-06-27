const cache = require('../cache')
const request = require('../request')

const search = info => {
	let url =
		'http://m.10086.cn/migu/remoting/scr_search_tag?' +
		'keyword=' + encodeURIComponent(info.keyword) + '&type=2&rows=20&pgc=1'

	return request('GET', url)
	.then(response => response.json())
	.then(jsonBody => {
		if('musics' in jsonBody){
			let matched = jsonBody.musics[0]
			return matched.mp3
		}
		else{
			return Promise.reject()
		}
	})
}

const check = info => cache(search, info)

module.exports = {check}