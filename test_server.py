#!/usr/bin/env python3
from flask import Flask, request, jsonify
import csv
import io

app = Flask(__name__)

@app.route('/health', methods=['GET'])
def health():
    return jsonify({"status": "ok"})

@app.route('/test-csv-parser', methods=['POST'])
def test_csv_parser():
    try:
        csv_data = request.get_data(as_text=True)
        reader = csv.reader(io.StringIO(csv_data))
        rows = [row for row in reader]
        return jsonify({
            "success": True,
            "rows_parsed": len(rows),
            "columns_in_first_row": len(rows[0]) if rows else 0,
            "data": rows[:10]
        })
    except Exception as e:
        return jsonify({"success": False, "error": str(e)}), 400

@app.route('/', methods=['GET'])
def index():
    return '''
    <html>
    <head><title>RFC 4180 CSV Parser Test</title></head>
    <body style="font-family: monospace; padding: 20px;">
    <h1>RFC 4180 CSV Parser Validation</h1>
    <p>POST raw CSV data to /test-csv-parser to validate parsing</p>
    <textarea id="csvInput" rows="10" cols="80" placeholder="Paste CSV data here..."></textarea><br><br>
    <button onclick="testParser()">Test Parser</button>
    <div id="output" style="margin-top: 20px; white-space: pre; background: #f0f0f0; padding: 10px;"></div>
    <script>
    function testParser(){
      const csv = document.getElementById('csvInput').value;
      fetch('/test-csv-parser', {method:'POST', body: csv})
        .then(r => r.json())
        .then(d => document.getElementById('output').textContent = JSON.stringify(d, null, 2))
        .catch(e => document.getElementById('output').textContent = 'Error: '+e);
    }
    </script>
    </body></html>
    '''

if __name__ == '__main__':
    print("RFC 4180 CSV Parser Test Server")
    print("Running on http://localhost:5000")
    print("POST CSV data to http://localhost:5000/test-csv-parser")
    app.run(debug=True, port=5000)
