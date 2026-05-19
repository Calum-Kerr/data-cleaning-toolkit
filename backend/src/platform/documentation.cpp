#include "documentation.h"

crow::response getApiDocumentation() {
  crow::json::wvalue doc;
  doc["api_version"] = "1.0";
  doc["status"] = "API documentation available";
  return crow::response(doc);
}
