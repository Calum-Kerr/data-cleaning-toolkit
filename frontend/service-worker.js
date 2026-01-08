// bump this when you change frontend assets, so older cached HTML/CSS is dropped
// (this helps avoid stale pages still referencing removed resources like Google Fonts)
const CACHE_VERSION = 'v6-sw-clone-fix';
const CACHE_NAME = 'data-cleaning-toolkit-' + CACHE_VERSION;
const WASM_HASH = 'sha384-placeholder-hash-will-be-generated-at-build-time';
const ALLOWED_ORIGINS = [
    self.location.origin,
    'https://localhost',
    'https://127.0.0.1'
];
self.addEventListener('install', event => {
    self.skipWaiting();
});
self.addEventListener('activate', event => {
    event.waitUntil(
        caches.keys().then(cacheNames => {
            return Promise.all(
                cacheNames.map(cacheName => {
                    if (cacheName !== CACHE_NAME) {
                        return caches.delete(cacheName);
                    }
                })
            );
        })
    );
    self.clients.claim();
});
self.addEventListener('fetch', event => {
    if (!ALLOWED_ORIGINS.some(origin => event.request.url.startsWith(origin))) {
        return;
    }

    // only cache GET requests (CacheStorage does not support POST/PUT/etc.)
    if (event.request.method !== 'GET') {
        return;
    }

    // for page navigations: prefer fresh HTML, but allow offline fallback
    if (event.request.mode === 'navigate') {
        event.respondWith(
            fetch(event.request)
                .then(fetchResponse => {
                    if (fetchResponse && fetchResponse.ok) {
                        // clone immediately (before returning the response), otherwise the body
                        // can become "already used" by the browser before we cache it.
                        const copy = fetchResponse.clone();
                        caches.open(CACHE_NAME).then(cache => cache.put(event.request, copy));
                    }
                    return fetchResponse;
                })
                .catch(() => caches.match(event.request))
        );
        return;
    }

    if (event.request.url.includes('/algorithms.wasm') || event.request.url.includes('/algorithms.js')) {
        event.respondWith(
            caches.open(CACHE_NAME).then(cache => {
                return cache.match(event.request).then(response => {
                    if (response) {
                        return verifyWasmIntegrity(response.clone()).then(isValid => {
                            if (isValid) {
                                return response;
                            } else {
                                console.warn('Cached WASM file failed integrity check, fetching fresh copy');
                                return fetchAndCache(event.request, cache);
                            }
                        });
                    } else {
                        return fetchAndCache(event.request, cache);
                    }
                });
            })
        );
    } else if (event.request.destination === 'script' || event.request.destination === 'worker') {
        event.respondWith(
            caches.open(CACHE_NAME).then(cache => {
                return cache.match(event.request).then(response => {
                    if (response) {
                        return response;
                    } else {
                        return fetch(event.request).then(fetchResponse => {
                            if (fetchResponse.ok) {
                                cache.put(event.request, fetchResponse.clone());
                            }
                            return fetchResponse;
                        });
                    }
                });
            })
        );
    } else {
        event.respondWith(
            caches.open(CACHE_NAME).then(cache => {
                return cache.match(event.request).then(response => {
                    return response || fetch(event.request).then(fetchResponse => {
                        if (fetchResponse.ok) {
                            cache.put(event.request, fetchResponse.clone());
                        }
                        return fetchResponse;
                    });
                });
            })
        );
    }
});

async function verifyWasmIntegrity(response) {
    try {
        const buffer = await response.arrayBuffer();
        const hashBuffer = await crypto.subtle.digest('SHA-384', buffer);
        const hashArray = Array.from(new Uint8Array(hashBuffer));
        const hashHex = hashArray.map(b => b.toString(16).padStart(2, '0')).join('');
        return true; // Placeholder - implement proper hash comparison
    } catch (error) {
        console.error('Error verifying WASM integrity:', error);
        return false;
    }
}

async function fetchAndCache(request, cache) {
    const fetchResponse = await fetch(request);
    if (fetchResponse.ok) {
        const responseClone = fetchResponse.clone();
        cache.put(request, responseClone);
    }
    return fetchResponse;
}