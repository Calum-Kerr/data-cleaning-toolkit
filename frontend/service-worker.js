// bump this when you change frontend assets, so older cached HTML/CSS is dropped
// (this helps avoid stale pages still referencing removed resources like Google Fonts)
const CACHE_VERSION = 'v8-offline-first';
const CACHE_NAME = 'data-cleaning-toolkit-' + CACHE_VERSION;
const WASM_HASH = 'sha384-placeholder-hash-will-be-generated-at-build-time';
const ALLOWED_ORIGINS = [
    self.location.origin,
    'https://localhost',
    'https://127.0.0.1'
];
// Critical assets to pre-cache for offline-first functionality
const CRITICAL_ASSETS = [
    '/',
    '/app',
    '/index.html',
    '/home.html',
    '/features.html',
    '/honours-project.html',
    '/algorithms.js',
    '/algorithms.wasm',
    '/jspdf.umd.min.js',
    '/jspdf.plugin.autotable.min.js'
];
self.addEventListener('install', event => {
    event.waitUntil(
        caches.open(CACHE_NAME).then(cache => {
            // Try to cache each asset individually, don't fail entire install
            return Promise.all(CRITICAL_ASSETS.map(asset => {
                return cache.add(asset).catch(err => {
                    console.warn('Failed to pre-cache:', asset, err);
                    // Continue on individual asset failures
                });
            }));
        }).then(() => {
            console.log('Service Worker install complete, cache version:', CACHE_VERSION);
            self.skipWaiting();
        }).catch(err => {
            console.warn('Service Worker install warning:', err);
            self.skipWaiting();
        })
    );
});
self.addEventListener('activate', event => {
    event.waitUntil(
        Promise.all([
            caches.keys().then(cacheNames => {
                return Promise.all(
                    cacheNames.map(cacheName => {
                        if (cacheName !== CACHE_NAME) {
                            console.log('Deleting old cache:', cacheName);
                            return caches.delete(cacheName);
                        }
                    })
                );
            }),
            self.clients.claim()
        ])
    );
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
                        caches.open(CACHE_NAME).then(cache => {
                            cache.put(event.request, copy);
                            console.log('Cached page:', event.request.url);
                        });
                    }
                    return fetchResponse;
                })
                .catch(() => {
                    console.log('Network failed, falling back to cache:', event.request.url);
                    return caches.match(event.request);
                })
        );
        return;
    }

    if (event.request.url.includes('/algorithms.wasm') || event.request.url.includes('/algorithms.js')) {
        event.respondWith(
            caches.open(CACHE_NAME).then(cache => {
                return cache.match(event.request).then(response => {
                    if (response) {
                        console.log('Serving WASM/JS from cache:', event.request.url);
                        return response;
                    } else {
                        console.log('WASM/JS not in cache, fetching:', event.request.url);
                        return fetch(event.request).then(fetchResponse => {
                            if (fetchResponse.ok) {
                                cache.put(event.request, fetchResponse.clone());
                            }
                            return fetchResponse;
                        }).catch(err => {
                            console.error('Failed to fetch WASM/JS:', event.request.url, err);
                            throw err;
                        });
                    }
                });
            }).catch(err => {
                console.error('WASM/JS fetch error:', err);
                return new Response('WASM module unavailable', { status: 503 });
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
    } else if (event.request.url.includes('cleaned_data.csv') || event.request.url.includes('download')) {
        // do not cache downloads
        event.respondWith(fetch(event.request));
    } else {
        event.respondWith(
            caches.open(CACHE_NAME).then(cache => {
                return cache.match(event.request).then(response => {
                    if (response) {
                        console.log('Serving from cache:', event.request.url);
                        return response;
                    }
                    return fetch(event.request).then(fetchResponse => {
                        if (fetchResponse && fetchResponse.ok) {
                            cache.put(event.request, fetchResponse.clone());
                        }
                        return fetchResponse;
                    }).catch(err => {
                        console.warn('Fetch failed, offline mode:', event.request.url);
                        // Don't return fallback for non-HTML requests (favicons, etc.)
                        if (event.request.mode === 'navigate') {
                            return cache.match('/app').then(r => r || new Response('Offline - page not cached', { status: 503 }));
                        }
                        // For other requests, just let it fail gracefully
                        return new Response('', { status: 503 });
                    });
                });
            }).catch(err => {
                console.error('Cache error:', err);
                return new Response('Service unavailable', { status: 503 });
            })
        );
    }
});

// Removed: verifyWasmIntegrity (was causing fetch errors)
// WASM files are cached on first successful fetch

async function fetchAndCache(request, cache) {
    try {
        const fetchResponse = await fetch(request);
        if (fetchResponse && fetchResponse.ok) {
            const responseClone = fetchResponse.clone();
            cache.put(request, responseClone);
            console.log('Cached:', request.url);
        }
        return fetchResponse;
    } catch (error) {
        console.error('fetchAndCache failed:', request.url, error);
        throw error;
    }
}