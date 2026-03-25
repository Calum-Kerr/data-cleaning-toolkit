const { test, expect } = require('@playwright/test');
const fs = require('fs');

const API_URL = 'http://localhost:8080';
const CSV_DATA = fs.readFileSync('./stress-test-100x10.csv', 'utf-8');

test.describe('Data Cleaning Toolkit API Tests', () => {
  test('Health check', async ({ request }) => {
    const response = await request.get(`${API_URL}/api/health`);
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.status).toBe('ok');
  });

  test('Parse CSV', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/parse`, {
      data: CSV_DATA
    });
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.rows).toBeGreaterThan(0);
  });

  test('Detect duplicates', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/detect-duplicates`, {
      data: CSV_DATA
    });
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.duplicateCount).toBeDefined();
  });

  test('Detect missing values', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/detect-missing`, {
      data: CSV_DATA
    });
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.missingCount).toBeDefined();
  });

  test('Detect outliers', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/detect-outliers`, {
      data: CSV_DATA
    });
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.outlierCount).toBeDefined();
  });

  test('Clean data', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/clean`, {
      data: CSV_DATA
    });
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.message).toBeDefined();
  });

  test('Normalize whitespace', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/normalize-whitespace`, {
      data: CSV_DATA
    });
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.message).toBeDefined();
  });

  test('Standardize case', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/standardize-case`, {
      data: CSV_DATA
    });
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.message).toBeDefined();
  });

  test('Standardize nulls', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/standardize-nulls`, {
      data: CSV_DATA
    });
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.message).toBeDefined();
  });

  test('Remove outliers', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/remove-outliers`, {
      data: CSV_DATA
    });
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.originalRows).toBeDefined();
  });

  test('Remove state suffixes', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/remove-state-suffixes`, {
      data: CSV_DATA
    });
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.message).toBeDefined();
  });

  test('Remove duplicate words', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/remove-duplicate-words`, {
      data: CSV_DATA
    });
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.message).toBeDefined();
  });

  test('Fuzzy deduplicate', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/fuzzy-deduplicate`, {
      data: CSV_DATA
    });
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.originalRows).toBeDefined();
    expect(data.deduplicatedRows).toBeDefined();
  });

  test('Natural sort', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/natural-sort`, {
      data: CSV_DATA
    });
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.message).toBeDefined();
  });
});

