(() => {
	const remote = 'oleomikdicccalekkpcbfgdmpjehnpkp'
	const remoteMatch = id => new Promise(resolve => {
		chrome.runtime.sendMessage(remote, {match: id}, response => {
			resolve(response)
		})
	})

	const waitTimeout = wait => new Promise(resolve => {
		setTimeout(() => {
			resolve()
		}, wait)
	})

	const searchFunction = (object, keyword) =>
		Object.keys(object)
		.filter(key => object[key] && typeof object[key] == 'function')
		.find(key => String(object[key]).match(keyword))

	if(self.frameElement && self.frameElement.tagName == 'IFRAME'){ //in iframe
		const keyOne = searchFunction(window.nej.e, '\\.dataset;if')
		const keyTwo = searchFunction(window.nm.x, '\\.copyrightId==')
		const keyThree = searchFunction(window.nm.x, '\\.privilege;if')
		const functionOne = window.nej.e[keyOne]

		window.nej.e[keyOne] = (z, name) => {
			if (name == 'copyright' || name == 'resCopyright') return 1
			return functionOne(z, name)
		}
		window.nm.x[keyTwo] = () => false
		window.nm.x[keyThree] = song => {
			song.status = 0
			if (song.privilege) song.privilege.pl = 320000
			return 0
		}
		const table = document.querySelector('table tbody')
		if(table) Array.from(table.childNodes)
		.filter(element => element.classList.contains('js-dis'))
		.forEach(element => element.classList.remove('js-dis'))
	}
	else{
		const keyAjax = searchFunction(window.nej.j, '\\.replace\\("api","weapi')
		const functionAjax = window.nej.j[keyAjax]
		window.nej.j[keyAjax] = (url, param) => {
			const onload = param.onload
			param.onload = data => {
				Promise.resolve()
				.then(() => {
					if(url.includes('enhance/player/url')){
						if(data.data[0].url){
							data.data[0].url = data.data[0].url.replace(/(m\d+?)(?!c)\.music\.126\.net/, '$1c.music.126.net')
						}
						else{
							return Promise.race([remoteMatch(data.data[0].id), waitTimeout(4000)])
							.then(result => {
								if(result){
									data.data[0].code = 200
									data.data[0].br = 320000
									data.data[0].type = 'mp3'
									data.data[0].size = result.size
									data.data[0].md5 = result.md5
									data.data[0].url = result.url.replace(/http:\/\//, 'https://')
								}
							})
						}
					}
				})
				.then(() => onload(data))
			}
			functionAjax(url, param)
		}
	}
})()