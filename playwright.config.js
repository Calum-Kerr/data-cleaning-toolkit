module.exports = {
  testDir: '.',
  testMatch: '**/comprehensive-playwright-test.js',
  timeout: 30000,
  retries: 0,
  workers: 1,
  use: {
    baseURL: 'http://localhost:8080',
    trace: 'on-first-retry',
  },
};

