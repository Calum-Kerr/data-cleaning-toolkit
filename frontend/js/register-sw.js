if ('serviceWorker' in navigator) {
  navigator.serviceWorker.register('/js/service-worker.js', {scope: '/'}).then(reg => {console.log('service worker registered');}).catch(err => {console.warn('service worker registration failed:', err);});
}
