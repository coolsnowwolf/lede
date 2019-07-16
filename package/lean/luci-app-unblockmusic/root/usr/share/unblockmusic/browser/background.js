import match from './provider/match.js'

chrome.runtime.onMessageExternal.addListener((request, sender, sendResponse) => {
	match(request.match, ['netease', 'qq', 'xiami'])
	.then(song => sendResponse(song))
	.catch(e => console.log(e))
	return true
})

chrome.webRequest.onBeforeSendHeaders.addListener(details => {
	let headers = details.requestHeaders
	headers.push({name: 'X-Real-IP', value: '118.88.88.88'})
	return {requestHeaders: headers}
}, {urls: ['*://music.163.com/*']}, ['blocking', 'requestHeaders'])

chrome.webRequest.onHeadersReceived.addListener(details => {
	let headers = details.responseHeaders
	if(details.initiator == "https://music.163.com" && details.type == 'media'){
		headers.push({name: 'Access-Control-Allow-Origin', value: '*'})
	}
	return {responseHeaders: headers}
}, {urls: ['*://*/*']}, ['blocking', 'responseHeaders'])