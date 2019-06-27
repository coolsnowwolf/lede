const cache = require('../cache')
const request = require('../request')

const filter = (object, keys) => Object.keys(object).filter(key => keys.includes(key)).reduce((result, key) => Object.assign(result, {[key]: object[key]}), {})
// Object.keys(object).filter(key => !keys.includes(key)).forEach(key => delete object[key])

const find = id => {
	let url =
		'https://music.163.com/api/song/detail?ids=[' + id + ']'

	return request('GET', url)
	.then(response => response.json())
	.then(jsonBody => {
		let info = filter(jsonBody.songs[0], ['id', 'name', 'alias', 'duration'])
		info.album = filter(jsonBody.songs[0].album, ['id', 'name'])
		info.artists = jsonBody.songs[0].artists.map(artist => filter(artist, ['id', 'name']))
		info.keyword = info.name + ' - ' + info.artists[0].name
		return info
	})
}

module.exports = id => cache(find, id)