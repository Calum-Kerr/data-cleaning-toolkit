#include "crow_all.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdlib>
#include "logger.h"
#include "rate_limiter.h"
#include "cache.h"

namespace fs = std::filesystem;

// resolve frontend directory: environment variable, probed paths, or fallback.
// tries in order: ../frontend, ../../frontend, ../../../frontend, ../../../../frontend, frontend.
std::string getFrontendDir() {
  // getenv result captured immediately into std::string below, no dangling pointer risk.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
  const char* env = std::getenv("FRONTEND_DIR");
#ifdef _MSC_VER
#pragma warning(pop)
#endif
  if (env) {std::string d(env); if (!d.empty() && d.back() == '/') d.pop_back(); return d;}
  for (const auto& path : {"../frontend", "../../frontend", "../../../frontend", "../../../../frontend", "frontend"})
    if (fs::exists(path)) return path;
  std::cerr << "WARNING: frontend directory not found, using ../frontend" << std::endl;
  return "../frontend";
}

// log which frontend directory was resolved at startup.
void logFrontendDirStartup() {
  std::cout << "frontend directory resolved to: " << getFrontendDir() << std::endl;
}

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
  // Helper to add security headers
  auto addSecurityHeaders = [](crow::response& res) {
    res.set_header("Content-Security-Policy", "script-src 'self' 'wasm-unsafe-eval'");
    res.set_header("Strict-Transport-Security", "max-age=31536000; includeSubDomains");
    res.set_header("X-Content-Type-Options", "nosniff");
    res.set_header("X-Frame-Options", "DENY");
    res.set_header("Referrer-Policy", "strict-origin-when-cross-origin");
    res.set_header("Permissions-Policy", "camera=(), microphone=(), geolocation=()");
  };

  // Root route - serve home.html
  CROW_ROUTE(app, "/").methods("GET"_method)
  ([addSecurityHeaders](const crow::request& req) {
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("GET", "/", 429); return crow::response(429);}
    std::string content = readFile(getFrontendDir() + "/home.html");
    if (content.empty()) {
      return crow::response(404, "Not Found");
    }
    auto res = crow::response(content);
    res.set_header("Content-Type", "text/html; charset=utf-8");
    addSecurityHeaders(res);
    logRequest("GET", "/", 200);
    return res;
  });

  // App route - serve index.html
  CROW_ROUTE(app, "/app").methods("GET"_method)
  ([addSecurityHeaders](const crow::request& req) {
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("GET", "/app", 429); return crow::response(429);}
    std::string content = readFile(getFrontendDir() + "/index.html");
    if (content.empty()) {
      return crow::response(404, "Not Found");
    }
    auto res = crow::response(content);
    res.set_header("Content-Type", "text/html; charset=utf-8");
    addSecurityHeaders(res);
    logRequest("GET", "/app", 200);
    return res;
  });

  // Features route
  CROW_ROUTE(app, "/features").methods("GET"_method)
  ([addSecurityHeaders](const crow::request& req) {
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("GET", "/features", 429); return crow::response(429);}
    std::string content = readFile(getFrontendDir() + "/features.html");
    if (content.empty()) {
      return crow::response(404, "Not Found");
    }
    auto res = crow::response(content);
    res.set_header("Content-Type", "text/html; charset=utf-8");
    addSecurityHeaders(res);
    logRequest("GET", "/features", 200);
    return res;
  });

  // Honours project route
  CROW_ROUTE(app, "/honours-project").methods("GET"_method)
  ([addSecurityHeaders](const crow::request& req) {
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("GET", "/honours-project", 429); return crow::response(429);}
    std::string content = readFile(getFrontendDir() + "/honours-project.html");
    if (content.empty()) {
      return crow::response(404, "Not Found");
    }
    auto res = crow::response(content);
    res.set_header("Content-Type", "text/html; charset=utf-8");
    addSecurityHeaders(res);
    logRequest("GET", "/honours-project", 200);
    return res;
  });

  // Privacy route
  CROW_ROUTE(app, "/privacy").methods("GET"_method)
  ([addSecurityHeaders](const crow::request& req) {
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("GET", "/privacy", 429); return crow::response(429);}
    std::string content = readFile(getFrontendDir() + "/privacy.html");
    if (content.empty()) {
      return crow::response(404, "Not Found");
    }
    auto res = crow::response(content);
    res.set_header("Content-Type", "text/html; charset=utf-8");
    addSecurityHeaders(res);
    logRequest("GET", "/privacy", 200);
    return res;
  });

  // Terms of service route
  CROW_ROUTE(app, "/terms").methods("GET"_method)
  ([addSecurityHeaders](const crow::request& req) {
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("GET", "/terms", 429); return crow::response(429);}
    std::string content = readFile(getFrontendDir() + "/terms.html");
    if (content.empty()) {
      return crow::response(404, "Not Found");
    }
    auto res = crow::response(content);
    res.set_header("Content-Type", "text/html; charset=utf-8");
    addSecurityHeaders(res);
    logRequest("GET", "/terms", 200);
    return res;
  });

  // Static files route - catch-all for frontend assets
  CROW_ROUTE(app, "/<path>").methods("GET"_method)
  ([](const crow::request& req, const std::string& path) {
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("GET", "/" + path, 429); return crow::response(429);}
    // Prevent directory traversal attacks
    if (path.find("..") != std::string::npos) {
      return crow::response(403, "Forbidden");
    }

    std::string filepath = getFrontendDir() + "/" + path;
    std::string content = getCachedFile(filepath);
    if (!content.empty()) {
      auto res = crow::response(content);
      res.set_header("Content-Type", getContentType(filepath));
      res.set_header("Content-Length", std::to_string(content.length()));
      if (endsWith(filepath, "service-worker.js")) {
        res.set_header("Service-Worker-Allowed", "/");
      }
      logRequest("GET", "/" + path, 200);
      return res;
    }

    // Check file exists before reading
    if (!fs::exists(filepath)) {
      return crow::response(404, "Not Found");
    }

    // Limit file size to prevent memory exhaustion (10MB max)
    if (fs::file_size(filepath) > 10 * 1024 * 1024) {
      return crow::response(413, "Payload Too Large");
    }

    content = readFile(filepath);

    if (content.empty()) {
      return crow::response(404, "Not Found");
    }

    setCachedFile(filepath, content);
    auto res = crow::response(content);
    res.set_header("Content-Type", getContentType(filepath));
    res.set_header("Content-Length", std::to_string(content.length()));
    if (endsWith(filepath, "service-worker.js")) {
      res.set_header("Service-Worker-Allowed", "/");
    }
    logRequest("GET", "/" + path, 200);
    return res;
  });
}
















