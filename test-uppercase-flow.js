const { test, expect } = require('@playwright/test');

const API_URL = 'http://localhost:8080';

// Test data with mixed case and whitespace
const TEST_CSV = `Name,City,Status
john doe,  new york  ,active
jane smith,los angeles,INACTIVE
bob JONES,  chicago  ,pending`;

test.describe('UPPERCASE Cleaning Flow Analysis', () => {
  
  test('1. Test /api/clean endpoint (Remove Duplicates)', async ({ request }) => {
    console.log('\n=== TEST 1: /api/clean (Remove Duplicates) ===');
    console.log('INPUT:', TEST_CSV);
    
    const response = await request.post(`${API_URL}/api/clean`, {
      data: TEST_CSV
    });
    
    expect(response.status()).toBe(200);
    const result = await response.json();
    console.log('RESPONSE:', JSON.stringify(result, null, 2));
  });

  test('2. Test /api/normalize-whitespace endpoint', async ({ request }) => {
    console.log('\n=== TEST 2: /api/normalize-whitespace ===');
    console.log('INPUT:', TEST_CSV);
    
    const response = await request.post(`${API_URL}/api/normalize-whitespace`, {
      data: TEST_CSV
    });
    
    expect(response.status()).toBe(200);
    const result = await response.json();
    console.log('RESPONSE:', JSON.stringify(result, null, 2));
  });

  test('3. Test /api/standardize-case endpoint', async ({ request }) => {
    console.log('\n=== TEST 3: /api/standardize-case ===');
    console.log('INPUT:', TEST_CSV);
    
    const response = await request.post(`${API_URL}/api/standardize-case`, {
      data: TEST_CSV
    });
    
    expect(response.status()).toBe(200);
    const result = await response.json();
    console.log('RESPONSE:', JSON.stringify(result, null, 2));
  });

  test('4. Full performQuickClean flow simulation', async ({ request }) => {
    console.log('\n=== TEST 4: Full performQuickClean Flow ===');
    console.log('INITIAL INPUT:', TEST_CSV);
    
    let current = TEST_CSV;
    
    // Step 1: Remove duplicates
    console.log('\nStep 1: Calling /api/clean...');
    let res = await request.post(`${API_URL}/api/clean`, { data: current });
    expect(res.status()).toBe(200);
    console.log('Response:', await res.json());
    // NOTE: Frontend doesn't update 'current' with response body!
    
    // Step 2: Normalize whitespace
    console.log('\nStep 2: Calling /api/normalize-whitespace...');
    res = await request.post(`${API_URL}/api/normalize-whitespace`, { data: current });
    expect(res.status()).toBe(200);
    console.log('Response:', await res.json());
    // NOTE: Frontend doesn't update 'current' with response body!
    
    // Step 3: Standardize case
    console.log('\nStep 3: Calling /api/standardize-case...');
    res = await request.post(`${API_URL}/api/standardize-case`, { data: current });
    expect(res.status()).toBe(200);
    console.log('Response:', await res.json());
    // NOTE: Frontend doesn't update 'current' with response body!
    
    console.log('\nFINAL OUTPUT (should be cleaned):', current);
    console.log('ISSUE: current variable was never updated with API responses!');
  });

  test('5. Check if responses contain actual cleaned data', async ({ request }) => {
    console.log('\n=== TEST 5: Checking Response Bodies ===');
    
    const response = await request.post(`${API_URL}/api/standardize-case`, {
      data: TEST_CSV
    });
    
    const body = await response.text();
    console.log('Raw response body:', body);
    console.log('Does it contain cleaned CSV data?', body.includes('JOHN') || body.includes('john'));
  });
});

