const CACHE_NAME='data-cleaning-toolkit-v1';
const SKIP_CACHE=['/algorithms.js','/algorithms.wasm'];
self.addEventListener('fetch',event=>{
    if(!event.request.url.startsWith('http')||SKIP_CACHE.some(path=>event.request.url.includes(path))){
        return;
    }
    event.respondWith(caches.open(CACHE_NAME).then(cache=>cache.match(event.request).then(response=>response||fetch(event.request).then(fetchResponse=>{
        if(event.request.method==='GET'){
            cache.put(event.request,fetchResponse.clone());
        }
        return fetchResponse;
    }))));
});