const { test, expect } = require('@playwright/test');
const fs = require('fs');

const API_URL = 'http://localhost:8080';
const CSV_DATA = fs.readFileSync('./stress-test-100x10.csv', 'utf-8');

let testResults = [];

test.describe('Data Cleaning Toolkit - Comprehensive API Tests', () => {
  test('1. Health Check', async ({ request }) => {
    const response = await request.get(`${API_URL}/api/health`);
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.status).toBe('ok');
    testResults.push({ name: 'Health Check', status: 'PASS', endpoint: '/api/health' });
  });

  test('2. Parse CSV', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/parse`, { data: CSV_DATA });
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.rows).toBeGreaterThan(0);
    testResults.push({ name: 'Parse CSV', status: 'PASS', endpoint: '/api/parse', rows: data.rows });
  });

  test('3. Detect Duplicates', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/detect-duplicates`, { data: CSV_DATA });
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.duplicateCount).toBeDefined();
    testResults.push({ name: 'Detect Duplicates', status: 'PASS', endpoint: '/api/detect-duplicates', count: data.duplicateCount });
  });

  test('4. Detect Missing Values', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/detect-missing`, { data: CSV_DATA });
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.missingCount).toBeDefined();
    testResults.push({ name: 'Detect Missing', status: 'PASS', endpoint: '/api/detect-missing', count: data.missingCount });
  });

  test('5. Detect Outliers', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/detect-outliers`, { data: CSV_DATA });
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.outlierCount).toBeDefined();
    testResults.push({ name: 'Detect Outliers', status: 'PASS', endpoint: '/api/detect-outliers', count: data.outlierCount });
  });

  test('6. Clean Data', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/clean`, { data: CSV_DATA });
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.originalRows).toBeDefined();
    testResults.push({ name: 'Clean Data', status: 'PASS', endpoint: '/api/clean', original: data.originalRows, cleaned: data.cleanedRows });
  });

  test('7. Normalize Whitespace', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/normalize-whitespace`, { data: CSV_DATA });
    expect(response.status()).toBe(200);
    testResults.push({ name: 'Normalize Whitespace', status: 'PASS', endpoint: '/api/normalize-whitespace' });
  });

  test('8. Standardize Case', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/standardize-case`, { data: CSV_DATA });
    expect(response.status()).toBe(200);
    testResults.push({ name: 'Standardize Case', status: 'PASS', endpoint: '/api/standardize-case' });
  });

  test('9. Standardize Nulls', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/standardize-nulls`, { data: CSV_DATA });
    expect(response.status()).toBe(200);
    testResults.push({ name: 'Standardize Nulls', status: 'PASS', endpoint: '/api/standardize-nulls' });
  });

  test('10. Remove Outliers', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/remove-outliers`, { data: CSV_DATA });
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.originalRows).toBeDefined();
    testResults.push({ name: 'Remove Outliers', status: 'PASS', endpoint: '/api/remove-outliers', removed: data.outliersRemoved });
  });

  test('11. Remove State Suffixes', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/remove-state-suffixes`, { data: CSV_DATA });
    expect(response.status()).toBe(200);
    testResults.push({ name: 'Remove State Suffixes', status: 'PASS', endpoint: '/api/remove-state-suffixes' });
  });

  test('12. Remove Duplicate Words', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/remove-duplicate-words`, { data: CSV_DATA });
    expect(response.status()).toBe(200);
    testResults.push({ name: 'Remove Duplicate Words', status: 'PASS', endpoint: '/api/remove-duplicate-words' });
  });

  test('13. Fuzzy Deduplicate (0.85)', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/fuzzy-deduplicate/0.85`, { data: CSV_DATA });
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.originalRows).toBeDefined();
    testResults.push({ name: 'Fuzzy Deduplicate', status: 'PASS', endpoint: '/api/fuzzy-deduplicate/0.85', merged: data.merged });
  });

  test('14. Natural Sort (Column 0)', async ({ request }) => {
    const response = await request.post(`${API_URL}/api/natural-sort/0`, { data: CSV_DATA });
    expect(response.status()).toBe(200);
    const data = await response.json();
    expect(data.rows).toBeDefined();
    testResults.push({ name: 'Natural Sort', status: 'PASS', endpoint: '/api/natural-sort/0', rows: data.rows });
  });

  test.afterAll(() => {
    fs.writeFileSync('./test-results.json', JSON.stringify(testResults, null, 2));
  });
});

