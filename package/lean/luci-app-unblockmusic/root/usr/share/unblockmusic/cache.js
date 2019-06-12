module.exports = (job, parameter, live = 30 * 60 * 1000) => {
    const cache = job.cache ? job.cache : job.cache = {}
    const key = parameter == null ? 'default' : (parameter.id || parameter.key || parameter)
    if(!(key in cache) || cache[key].expiration < Date.now())
        cache[key] = {
            execution: job(parameter),
            expiration: Date.now() + live
        }
    return cache[key].execution
}