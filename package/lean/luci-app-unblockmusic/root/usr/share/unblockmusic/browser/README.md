# Web Extension Port

For test

## Implementation

- Convert node module to ES6 module which can be directly executed in Chrome 61+ without Babel
- Rewrite crypto module (using CryptoJS) and request (using XMLHttpRequest) module for browser environment
- Do matching in background and transfer result with chrome runtime communication
- Inject content script for hijacking Netease Music Web Ajax response

## Build

```
$ node convert.js
```

## Install

Load unpacked extension in Developer mode

## Reference

- [brix/crypto-js](https://github.com/brix/crypto-js)
- [JixunMoe/cuwcl4c](https://github.com/JixunMoe/cuwcl4c)