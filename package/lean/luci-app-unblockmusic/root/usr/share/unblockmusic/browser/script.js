(() => {
	let script = (document.head || document.documentElement).appendChild(document.createElement('script'))
	script.src = chrome.extension.getURL('inject.js')
	script.onload = script.parentNode.removeChild(script)
})()