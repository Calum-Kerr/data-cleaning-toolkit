const CACHE_VERSION='v2';
const CACHE_NAME='data-cleaning-toolkit-'+CACHE_VERSION;
const SKIP_CACHE=['/algorithms.js','/algorithms.wasm','/app','/index.html'];
self.addEventListener('install',event=>{self.skipWaiting();});
self.addEventListener('activate',event=>{
    event.waitUntil(caches.keys().then(cacheNames=>{
        return Promise.all(cacheNames.map(cacheName=>{
            if(cacheName!==CACHE_NAME){return caches.delete(cacheName);}
        }));
    }));
    self.clients.claim();
});
self.addEventListener('fetch',event=>{
    if(!event.request.url.startsWith('http')||SKIP_CACHE.some(path=>event.request.url.includes(path))){return;}
    event.respondWith(caches.open(CACHE_NAME).then(cache=>cache.match(event.request).then(response=>response||fetch(event.request).then(fetchResponse=>{
        if(event.request.method==='GET'){cache.put(event.request,fetchResponse.clone());}
        return fetchResponse;
    }))));
});