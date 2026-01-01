# API Reference

The toolkit provides a REST API for server-side data cleaning.

## Base URL

- **Production**: https://www.calumkerr.com
- **Local**: http://localhost:8080

## Endpoints

### Parse CSV

Parses CSV data and returns row count.

```
POST /api/parse
Content-Type: text/plain

name,age,city
John,25,Edinburgh
Sarah,30,London
```

**Response:**
```json
{
  "message": "CSV parsed successfully",
  "rows": 2,
  "mode": "api"
}
```

### Detect Missing Values

Detects empty cells in CSV data.

```
POST /api/detect-missing
Content-Type: text/plain

name,age,city
John,,Edinburgh
Sarah,30,
```

**Response:**
```json
{
  "message": "Missing values detected",
  "missing": 2,
  "mode": "api"
}
```

### Detect Duplicates

Detects duplicate rows in CSV data.

```
POST /api/detect-duplicates
Content-Type: text/plain

name,age
John,25
Sarah,30
John,25
```

**Response:**
```json
{
  "message": "Duplicates detected",
  "duplicates": 1,
  "mode": "api"
}
```

### Clean Data (Remove Duplicates)

Removes duplicate rows from CSV data.

```
POST /api/clean
Content-Type: text/plain

name,age
John,25
Sarah,30
John,25
```

**Response:**
```json
{
  "message": "Data cleaned successfully",
  "originalRows": 3,
  "cleanedRows": 2,
  "removedRows": 1,
  "mode": "api"
}
```

### Detect Whitespace

Detects cells with leading/trailing whitespace.

```
POST /api/detect-whitespace
Content-Type: text/plain

name,age
  John  ,25
Sarah,30
```

**Response:**
```json
{
  "message": "Whitespace detected",
  "cellsWithWhitespace": 1,
  "mode": "api"
}
```

### Trim Whitespace

Removes whitespace and returns cleaned CSV.

```
POST /api/trim-whitespace
Content-Type: text/plain
```

**Response:**
```json
{
  "message": "Whitespace trimmed",
  "cellsTrimmed": 1,
  "cleaned": "name,age\nJohn,25\nSarah,30\n",
  "mode": "api"
}
```

## Error Handling

All endpoints return JSON errors:

```json
{
  "error": "Error description",
  "mode": "api"
}
```

## CORS

The API supports CORS for browser requests.

