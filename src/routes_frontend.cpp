#include "crow_all.h"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

bool endsWith(const std::string& str, const std::string& suffix) {
  if (str.length() < suffix.length()) return false;
  return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

std::string readFile(const std::string& filepath) {
  std::ifstream file(filepath, std::ios::binary);
  if (!file.is_open()) {
    return "";
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

std::string getContentType(const std::string& filepath) {
  if (endsWith(filepath, ".html")) return "text/html; charset=utf-8";
  if (endsWith(filepath, ".css")) return "text/css; charset=utf-8";
  if (endsWith(filepath, ".js")) return "application/javascript; charset=utf-8";
  if (endsWith(filepath, ".json")) return "application/json; charset=utf-8";
  if (endsWith(filepath, ".wasm")) return "application/wasm";
  if (endsWith(filepath, ".png")) return "image/png";
  if (endsWith(filepath, ".ico")) return "image/x-icon";
  if (endsWith(filepath, ".svg")) return "image/svg+xml";
  if (endsWith(filepath, ".webmanifest")) return "application/manifest+json";
  if (endsWith(filepath, ".xml")) return "application/xml; charset=utf-8";
  if (endsWith(filepath, ".txt")) return "text/plain; charset=utf-8";
  return "application/octet-stream";
}

void registerFrontendRoutes(crow::SimpleApp& app) {
  // Root route - serve home.html
  CROW_ROUTE(app, "/").methods("GET"_method)
  ([]() {
    std::string content = readFile("../frontend/home.html");
    if (content.empty()) {
      return crow::response(404, "Not Found");
    }
    auto res = crow::response(content);
    res.set_header("Content-Type", "text/html; charset=utf-8");
    return res;
  });

  // App route - serve index.html
  CROW_ROUTE(app, "/app").methods("GET"_method)
  ([]() {
    std::string content = readFile("../frontend/index.html");
    if (content.empty()) {
      return crow::response(404, "Not Found");
    }
    auto res = crow::response(content);
    res.set_header("Content-Type", "text/html; charset=utf-8");
    return res;
  });

  // Features route
  CROW_ROUTE(app, "/features").methods("GET"_method)
  ([]() {
    std::string content = readFile("../frontend/features.html");
    if (content.empty()) {
      return crow::response(404, "Not Found");
    }
    auto res = crow::response(content);
    res.set_header("Content-Type", "text/html; charset=utf-8");
    return res;
  });

  // Honours project route
  CROW_ROUTE(app, "/honours-project").methods("GET"_method)
  ([]() {
    std::string content = readFile("../frontend/honours-project.html");
    if (content.empty()) {
      return crow::response(404, "Not Found");
    }
    auto res = crow::response(content);
    res.set_header("Content-Type", "text/html; charset=utf-8");
    return res;
  });

  // Static files route - catch-all for frontend assets
  CROW_ROUTE(app, "/<path>").methods("GET"_method)
  ([](const std::string& path) {
    // Prevent directory traversal attacks
    if (path.find("..") != std::string::npos) {
      return crow::response(403, "Forbidden");
    }

    std::string filepath = "../frontend/" + path;

    // Check file exists before reading
    if (!fs::exists(filepath)) {
      return crow::response(404, "Not Found");
    }

    // Limit file size to prevent memory exhaustion (10MB max)
    if (fs::file_size(filepath) > 10 * 1024 * 1024) {
      return crow::response(413, "Payload Too Large");
    }

    std::string content = readFile(filepath);

    if (content.empty()) {
      return crow::response(404, "Not Found");
    }

    auto res = crow::response(content);
    res.set_header("Content-Type", getContentType(filepath));
    res.set_header("Content-Length", std::to_string(content.length()));
    return res;
  });
}
















