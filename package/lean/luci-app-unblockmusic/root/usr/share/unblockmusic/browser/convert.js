const fs = require('fs')
const path = require('path')

const importReplacer = (match, state, alias, file) => {
    file = file.endsWith('.js') ? file : file + '.js'
    return `import ${alias} from '${file}'`
}

const converter = (input, output, processor) => {
    let data = fs.readFileSync(input).toString()
    if(processor){
        data = processor(data)
    }
    else{
        data = data.replace(/global\./g, 'window.')
        data = data.replace(/(const|let|var)\s+(\w+)\s*=\s*require\(\s*['|"](.+)['|"]\s*\)/g, importReplacer)
        data = data.replace(/module\.exports\s*=\s*/g, 'export default ')
    }
    fs.writeFileSync(output, data)
}

converter(path.resolve(__dirname, '..', 'cache.js'), path.resolve(__dirname, '.', 'cache.js'))

if(!fs.existsSync(path.resolve(__dirname, 'provider'))) fs.mkdirSync(path.resolve(__dirname, 'provider'))

fs.readdirSync(path.resolve(__dirname, '..', 'provider')).filter(file => !file.includes('test')).forEach(file => {
    converter(path.resolve(__dirname, '..', 'provider', file), path.resolve(__dirname, 'provider', file))
})

const providerReplacer = (match, state, data) => {
    let provider = []
    let imports = data.match(/\w+\s*:\s*require\(['|"].+['|"]\)/g).map(line => {
        line = line.match(/(\w+)\s*:\s*require\(['|"](.+)['|"]\)/)
        provider.push(line[1])
        return importReplacer(null, null, line[1], line[2])
    })
    return imports.join('\n') + '\n\n' + `${state} provider = {${provider.join(', ')}}`
}

converter(path.resolve(__dirname, 'provider', 'match.js'), path.resolve(__dirname, 'provider', 'match.js'), data => {
    data = data.replace(/(const|let|var)\s+provider\s*=\s*{([^}]+)}/g, providerReplacer)
    return data
})