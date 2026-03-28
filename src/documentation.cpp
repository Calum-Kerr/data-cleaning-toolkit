#include "documentation.h"

crow::response getApiDocumentation() {
  crow::json::wvalue doc;
  doc["api_version"] = "1.0";
  doc["status"] = "API documentation endpoint";
  return crow::response(doc);
}

crow::response getApiDocumentation_old() {
  crow::json::wvalue doc;
  doc["api_version"] = "1.0";
  doc["endpoints"] = crow::json::wvalue::list();

  auto& endpoints = doc["endpoints"];

  // Core endpoints
  endpoints.push_back(crow::json::wvalue{
    {"method", "GET"},
    {"path", "/api/health"},
    {"description", "Health check endpoint"}
  });

  endpoints.push_back(crow::json::wvalue{
    {"method", "POST"},
    {"path", "/api/parse"},
    {"description", "Parse CSV data from request body"}
  });

  endpoints.push_back(crow::json::wvalue{
    {"method", "POST"},
    {"path", "/api/clean"},
    {"description", "Clean data by removing duplicates"}
  });

  endpoints.push_back(crow::json::wvalue{
    {"method", "POST"},
    {"path", "/api/detect-duplicates"},
    {"description", "Detect duplicate rows in data"}
  });

  endpoints.push_back(crow::json::wvalue{
    {"method", "GET"},
    {"path", "/api/analytics"},
    {"description", "Get and log analytics summary"}
  });

  // Detection endpoints
  endpoints.push_back(crow::json::wvalue{
    {"method", "POST"},
    {"path", "/api/detect-outliers"},
    {"description", "Detect outlier values in data"}
  });

  endpoints.push_back(crow::json::wvalue{
    {"method", "POST"},
    {"path", "/api/detect-missing"},
    {"description", "Detect missing values in data"}
  });

  // Text processing endpoints
  endpoints.push_back(crow::json::wvalue{
    {"method", "POST"},
    {"path", "/api/normalize-whitespace"},
    {"description", "Normalize whitespace in data"}
  });

  endpoints.push_back(crow::json::wvalue{
    {"method", "POST"},
    {"path", "/api/standardize-case"},
    {"description", "Standardize text case in data"}
  });

  endpoints.push_back(crow::json::wvalue{
    {"method", "POST"},
    {"path", "/api/remove-state-suffixes"},
    {"description", "Remove state suffixes from text"}
  });

  endpoints.push_back(crow::json::wvalue{
    {"method", "POST"},
    {"path", "/api/remove-duplicate-words"},
    {"description", "Remove duplicate words from text"}
  });

  endpoints.push_back(crow::json::wvalue{
    {"method", "POST"},
    {"path", "/api/natural-sort/<int>"},
    {"description", "Sort data naturally by column index"}
  });

  // Cleaning endpoints
  endpoints.push_back(crow::json::wvalue{
    {"method", "POST"},
    {"path", "/api/standardize-nulls"},
    {"description", "Standardize null values in data"}
  });

  endpoints.push_back(crow::json::wvalue{
    {"method", "POST"},
    {"path", "/api/fuzzy-deduplicate/<double>"},
    {"description", "Fuzzy deduplicate rows by threshold"}
  });

}
