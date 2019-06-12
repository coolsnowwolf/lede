export default (method, url, headers, body) => new Promise((resolve, reject) => {
	headers = headers || {}
	const xhr = new XMLHttpRequest()
	xhr.onreadystatechange = () => {if(xhr.readyState == 4) resolve(xhr)}
	xhr.onerror = error => reject(error)
	xhr.open(method, url, true)
	Object.keys(headers).filter(key => !['origin', 'referer'].includes(key)).forEach(key => xhr.setRequestHeader(key, headers[key]))
	xhr.send(body)
}).then(xhr => Object.assign(xhr, {
	statusCode: xhr.status,
	headers: 
		xhr.getAllResponseHeaders().split('\r\n').filter(line => line).map(line => line.split(/\s*:\s*/))
		.reduce((result, pair) => Object.assign(result, {[pair[0].toLowerCase()]: pair[1]}), {}),
	url: {href: xhr.responseURL},
	body: () => xhr.responseText,
	json: () => JSON.parse(xhr.responseText),
	jsonp: () => JSON.parse(xhr.responseText.slice(xhr.responseText.indexOf('(') + 1, -')'.length))
}))