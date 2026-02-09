#include "crow_all.h"
#include "algorithms.h"
#include <iostream>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <optional>
#include <vector>
#include <string>

#ifdef __linux__
#include <unistd.h>
#include <limits.h>
#endif

namespace fs = std::filesystem;

static int levenshteinDistance(const std::string& s1, const std::string& s2){
	size_t len1=s1.length();
	size_t len2=s2.length();
	std::vector<std::vector<int>> dp(len1+1,std::vector<int>(len2+1,0));
	for(size_t i=0;i<=len1;++i)dp[i][0]=i;
	for(size_t j=0;j<=len2;++j)dp[0][j]=j;
	for(size_t i=1;i<=len1;++i){
		for(size_t j=1;j<=len2;++j){
			if(s1[i-1]==s2[j-1]){
				dp[i][j]=dp[i-1][j-1];
			}else{
				dp[i][j]=1+std::min({dp[i-1][j],dp[i][j-1],dp[i-1][j-1]});
			}
		}
	}
	return dp[len1][len2];
}

static double calculateSimilarity(const std::string& s1, const std::string& s2){
	int distance=levenshteinDistance(s1,s2);
	int maxLen=std::max(s1.length(),s2.length());
	if(maxLen==0)return 1.0;
	return 1.0-(double)distance/maxLen;
}

// ============================================================================
// UNIVERSAL TEXT CLEANING HELPER FUNCTIONS
// ============================================================================

// Normalize whitespace: trim leading/trailing, collapse multiple spaces
static std::string normalizeWhitespace(const std::string& text){
	std::string result;
	bool inSpace=false;
	for(char c : text){
		if(c==' ' || c=='\t' || c=='\r' || c=='\n'){
			if(!inSpace && !result.empty()){
				result+=' ';
				inSpace=true;
			}
		}else{
			result+=c;
			inSpace=false;
		}
	}
	// Trim trailing space
	while(!result.empty() && result.back()==' '){
		result.pop_back();
	}
	return result;
}

// Normalize punctuation: standardize dashes, commas, periods, apostrophes
// Also removes periods from abbreviations (e.g., "D.C." -> "DC")
static std::string normalizePunctuation(const std::string& text){
	std::string result;
	for(size_t i=0; i<text.length(); ++i){
		char c=text[i];
		// Convert various dashes to standard hyphen
		if(c=='\u2013' || c=='\u2014' || c=='\u2010'){
			result+='-';
		}
		// Convert various quotes to standard apostrophe
		else if(c=='\u2018' || c=='\u2019' || c=='\u201C' || c=='\u201D'){
			result+='\'';
		}
		// Remove periods (handles abbreviations like "D.C." -> "DC")
		else if(c=='.'){
			// Skip the period - don't add it to result
			continue;
		}
		// Keep standard punctuation
		else{
			result+=c;
		}
	}
	return result;
}

// Standardize null values to empty string
static std::string standardizeNullValues(const std::string& text){
	std::string trimmed=normalizeWhitespace(text);
	if(trimmed.empty()) return "";
	if(trimmed=="N/A" || trimmed=="n/a" || trimmed=="NA" || trimmed=="na") return "";
	if(trimmed=="null" || trimmed=="NULL" || trimmed=="Null") return "";
	if(trimmed=="none" || trimmed=="NONE" || trimmed=="None") return "";
	if(trimmed=="-" || trimmed=="?" || trimmed=="~") return "";
	if(trimmed=="(empty)" || trimmed=="EMPTY") return "";
	return trimmed;
}

// Check if a string is numeric
static bool isNumericColumn(const std::vector<std::string>& columnValues, int sampleSize=100){
	int numericCount=0;
	int sampleCount=std::min((int)columnValues.size(), sampleSize);
	for(int i=0; i<sampleCount; ++i){
		std::string cell=columnValues[i];
		if(cell.empty()) continue;
		char* endptr;
		strtod(cell.c_str(), &endptr);
		if(*endptr=='\0'){
			numericCount++;
		}
	}
	// If less than 80% numeric, treat as text column
	return numericCount < (sampleCount * 0.8);
}

// Detect all text columns in the CSV
static std::vector<int> detectTextColumns(const std::vector<std::vector<std::string>>& parsed){
	std::vector<int> textColumns;
	if(parsed.empty()) return textColumns;

	int numColumns=parsed[0].size();
	for(int col=0; col<numColumns; ++col){
		std::vector<std::string> columnValues;
		for(size_t row=1; row<parsed.size(); ++row){
			if(col < (int)parsed[row].size()){
				columnValues.push_back(parsed[row][col]);
			}
		}
		if(isNumericColumn(columnValues)){
			textColumns.push_back(col);
		}
	}
	return textColumns;
}

// Remove state/country suffixes: anything after comma or patterns like " STATE US"
// Generic preprocessing that works for any text data, not just locations
static std::string removeStateSuffixes(const std::string& text){
	std::string result=text;

	// Remove anything after the last comma (e.g., "MIAMI, FLORIDA" -> "MIAMI")
	size_t commaPos=result.rfind(',');
	if(commaPos != std::string::npos){
		result=result.substr(0, commaPos);
	}

	// Remove common state/country suffix patterns (case-insensitive)
	// Convert to uppercase for pattern matching
	std::string upper=result;
	std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

	// List of patterns to remove (generic, not hardcoded city names)
	// NOTE: Only include state names that are NOT also city names
	// Removed: " NEW YORK", " WASHINGTON" (these are also city names)
	const std::vector<std::string> patterns={
		" ALABAMA", " ALASKA", " ARIZONA", " ARKANSAS", " CALIFORNIA", " COLORADO",
		" CONNECTICUT", " DELAWARE", " FLORIDA", " GEORGIA", " HAWAII", " IDAHO",
		" ILLINOIS", " INDIANA", " IOWA", " KANSAS", " KENTUCKY", " LOUISIANA",
		" MAINE", " MARYLAND", " MASSACHUSETTS", " MICHIGAN", " MINNESOTA",
		" MISSISSIPPI", " MISSOURI", " MONTANA", " NEBRASKA", " NEVADA",
		" NEW HAMPSHIRE", " NEW JERSEY", " NEW MEXICO", " NORTH CAROLINA",
		" NORTH DAKOTA", " OHIO", " OKLAHOMA", " OREGON", " PENNSYLVANIA",
		" RHODE ISLAND", " SOUTH CAROLINA", " SOUTH DAKOTA", " TENNESSEE", " TEXAS",
		" UTAH", " VERMONT", " VIRGINIA", " WEST VIRGINIA",
		" WISCONSIN", " WYOMING",
		" DISTRICT OF COLUMBIA", " US", " USA", " UNITED STATES"
	};

	for(const auto& pattern : patterns){
		size_t pos=upper.rfind(pattern);
		if(pos != std::string::npos && pos + pattern.length()==upper.length()){
			result=result.substr(0, pos);
			upper=upper.substr(0, pos);
		}
	}

	// Trim trailing whitespace
	while(!result.empty() && (result.back()==' ' || result.back()=='\t')){
		result.pop_back();
	}

	return result;
}

// Remove duplicate consecutive words (e.g., "NEW ORLEANS ORLEANS" -> "NEW ORLEANS")
// Generic preprocessing that works for any text data
static std::string removeDuplicateWords(const std::string& text){
	std::vector<std::string> words;
	std::stringstream ss(text);
	std::string word;

	// Split by spaces
	while(ss >> word){
		words.push_back(word);
	}

	// Remove consecutive duplicates
	std::vector<std::string> result;
	for(size_t i=0; i<words.size(); ++i){
		if(i==0 || words[i] != words[i-1]){
			result.push_back(words[i]);
		}
	}

	// Rejoin with spaces
	std::string output;
	for(size_t i=0; i<result.size(); ++i){
		if(i > 0) output+=" ";
		output+=result[i];
	}

	return output;
}

// Build fuzzy matching groups for a single column
// Returns a map: canonical value -> list of original values that map to it
// OPTIMIZED: Only compares unique values, not all rows
static std::map<std::string, std::vector<std::string>> buildFuzzyMatchingGroups(
	const std::vector<std::string>& columnValues,
	double threshold=0.75){

	std::map<std::string, std::vector<std::string>> groups;
	std::map<std::string, int> valueCounts;

	// Step 1: Extract unique values and count occurrences
	std::set<std::string> uniqueSet;
	for(const auto& val : columnValues){
		if(!val.empty()){
			uniqueSet.insert(val);
			valueCounts[val]++;
		}
	}

	// Convert to vector for easier iteration
	std::vector<std::string> uniqueValues(uniqueSet.begin(), uniqueSet.end());

	// Step 2: Build groups using fuzzy matching on unique values only
	std::set<std::string> processed;
	for(size_t i=0; i<uniqueValues.size(); ++i){
		const auto& val1=uniqueValues[i];
		if(processed.count(val1)) continue;

		std::string canonical=val1;
		int maxCount=valueCounts[val1];
		std::vector<std::string> matches;
		matches.push_back(val1);

		// Find all similar values - only compare with unprocessed values
		for(size_t j=i+1; j<uniqueValues.size(); ++j){
			const auto& val2=uniqueValues[j];
			if(processed.count(val2)) continue;

			// Apply preprocessing to both values for comparison
			// This allows "MIAMI FLORIDA US" to match "MIAMI" after preprocessing
			std::string prep1=val1;
			std::string prep2=val2;

			// Apply preprocessing steps in order
			prep1=removeStateSuffixes(prep1);
			prep1=removeDuplicateWords(prep1);
			prep1=normalizePunctuation(prep1);
			prep1=normalizeWhitespace(prep1);

			prep2=removeStateSuffixes(prep2);
			prep2=removeDuplicateWords(prep2);
			prep2=normalizePunctuation(prep2);
			prep2=normalizeWhitespace(prep2);

			// Skip if either preprocessed value is empty
			if(prep1.empty() || prep2.empty()){
				continue;
			}

			// Length-based filtering: skip if lengths differ by more than 75%
			// This allows comparisons like "MIAMI" (5) vs "MIAMI FLORIDA US" (15)
			// where the difference is 10, which is 66% of 15 (less than 75%)
			int len1=prep1.length();
			int len2=prep2.length();
			int maxLen=std::max(len1, len2);
			int minLen=std::min(len1, len2);
			if(minLen > 0 && (maxLen - minLen) > (maxLen * 0.75)){
				continue; // Skip only very different lengths
			}

			// Compare preprocessed values
			double similarity=calculateSimilarity(prep1, prep2);
			if(similarity >= threshold){
				matches.push_back(val2);
				// Use the more common value as canonical
				if(valueCounts[val2] > maxCount){
					canonical=val2;
					maxCount=valueCounts[val2];
				}
				processed.insert(val2);
			}
		}

		// Map all matches to canonical
		for(const auto& match : matches){
			groups[canonical].push_back(match);
		}
		processed.insert(val1);
	}

	return groups;
}

// Apply fuzzy matching to a column: returns mapping of original -> canonical
static std::map<std::string, std::string> createFuzzyMatchingMapping(
	const std::vector<std::string>& columnValues,
	double threshold=0.75){

	auto groups=buildFuzzyMatchingGroups(columnValues, threshold);
	std::map<std::string, std::string> mapping;

	// First, ensure all unique values are in the mapping
	std::set<std::string> allUnique;
	for(const auto& val : columnValues){
		if(!val.empty()){
			allUnique.insert(val);
		}
	}

	// Map all unique values to themselves by default
	for(const auto& val : allUnique){
		mapping[val]=val;
	}

	// Then, override with fuzzy matching groups
	for(const auto& pair : groups){
		// Map canonical value to itself
		mapping[pair.first]=pair.first;
		// Map all matched values to canonical
		for(const auto& val : pair.second){
			mapping[val]=pair.first;
		}
	}

	return mapping;
}

// Universal text cleaning function
// Applies all cleaning operations to all text columns
struct UniversalCleaningResult {
	std::vector<std::vector<std::string>> cleanedData;
	std::map<int, std::map<std::string, std::string>> columnMappings; // column -> (original -> canonical)
	std::map<int, int> mergedCountPerColumn; // column -> number of values merged
	int duplicateRowsRemoved;
	std::vector<std::string> operationsLog;
};

static UniversalCleaningResult universalTextCleaning(
	const std::vector<std::vector<std::string>>& parsed,
	double fuzzyThreshold=0.75,
	bool removeDuplicateRows=true){

	UniversalCleaningResult result;
	result.duplicateRowsRemoved=0;

	if(parsed.empty()){
		return result;
	}

	// Detect all text columns
	auto textColumns=detectTextColumns(parsed);
	result.operationsLog.push_back("Detected " + std::to_string(textColumns.size()) + " text columns");

	// For each text column, build fuzzy matching mappings ONLY if not too many unique values
	for(int col : textColumns){
		std::vector<std::string> columnValues;
		for(size_t row=1; row<parsed.size(); ++row){
			if(col < (int)parsed[row].size()){
				columnValues.push_back(parsed[row][col]);
			}
		}

		// Count unique values
		std::set<std::string> uniqueSet(columnValues.begin(), columnValues.end());

		// Only do fuzzy matching if there are not too many unique values
		if(uniqueSet.size() <= 500){
			auto mapping=createFuzzyMatchingMapping(columnValues, fuzzyThreshold);
			result.columnMappings[col]=mapping;

			// Count how many unique values were merged
			std::set<std::string> mergedUnique;
			for(const auto& val : columnValues){
				if(!val.empty()){
					mergedUnique.insert(mapping[val]);
				}
			}
			int mergedCount=uniqueSet.size() - mergedUnique.size();
			result.mergedCountPerColumn[col]=mergedCount;

			if(mergedCount > 0){
				result.operationsLog.push_back("Column " + std::to_string(col) + ": merged " +
					std::to_string(mergedCount) + " values");
			}
		}else{
			result.operationsLog.push_back("Column " + std::to_string(col) + ": skipped fuzzy matching (too many unique values: " +
				std::to_string(uniqueSet.size()) + ")");
		}
	}

	// Clean and transform all rows
	std::set<std::vector<std::string>> seenRows;
	for(size_t i=0; i<parsed.size(); ++i){
		auto row=parsed[i];

		// Apply cleaning to each cell
		for(size_t j=0; j<row.size(); ++j){
			std::string cell=row[j];

			// Check if this is a text column
			bool isTextCol=false;
			for(int col : textColumns){
				if(col == (int)j){
					isTextCol=true;
					break;
				}
			}

			if(isTextCol){
				// Apply cleaning operations in sequence
				cell=standardizeNullValues(cell);
				if(!cell.empty()){
					cell=normalizePunctuation(cell);
					cell=normalizeWhitespace(cell);

					// Apply fuzzy matching mapping if available
					if(result.columnMappings[j].count(cell)){
						cell=result.columnMappings[j][cell];
					}
				}
			}

			row[j]=cell;
		}

		// Remove duplicate rows if requested
		if(removeDuplicateRows){
			if(!seenRows.count(row)){
				seenRows.insert(row);
				result.cleanedData.push_back(row);
			}else{
				result.duplicateRowsRemoved++;
			}
		}else{
			result.cleanedData.push_back(row);
		}
	}

	if(removeDuplicateRows && result.duplicateRowsRemoved > 0){
		result.operationsLog.push_back("Removed " + std::to_string(result.duplicateRowsRemoved) +
			" duplicate rows");
	}

	return result;
}

static std::optional<fs::path> g_frontendDir;
static std::string g_frontendDiag;

static bool isSeoCrawlerUserAgent(const std::string& ua){
	// keep this simple and explicit. we only need to recognise google for now.
	if(ua.find("Googlebot") != std::string::npos) return true;
	if(ua.find("googlebot") != std::string::npos) return true;
	if(ua.find("Google-InspectionTool") != std::string::npos) return true;
	if(ua.find("google-inspectiontool") != std::string::npos) return true;
	return false;
}

static std::string getSeoBaseUrl(const crow::request& req){
	// optional override for canonical urls in robots/sitemap
	const char* envBase = std::getenv("SEO_BASE_URL");
	if(envBase && envBase[0] != '\0'){
		std::string v(envBase);
		while(!v.empty() && v.back() == '/') v.pop_back();
		return v;
	}

	auto proto = req.get_header_value("X-Forwarded-Proto");
	if(proto.empty()) proto = "http";

	auto host = req.get_header_value("X-Forwarded-Host");
	if(host.empty()) host = req.get_header_value("Host");

	return proto + "://" + host;
}

#ifdef __linux__
static std::optional<fs::path> getSelfExePath(){
    // in linux containers, argv[0] can be relative or misleading.
    // /proc/self/exe is the most reliable way to locate the running binary.
    std::vector<char> buf;
    buf.resize(4096);
    const ssize_t len = ::readlink("/proc/self/exe", buf.data(), (ssize_t)buf.size() - 1);
    if(len <= 0){
        return std::nullopt;
    }
    buf[(size_t)len] = '\0';
    return fs::path(std::string(buf.data()));
}
#endif

static std::optional<fs::path> findFrontendDirFrom(fs::path startDir){
    // walk up a few parents looking for frontend/index.html
    for(int depth=0; depth<10; ++depth){
        fs::path candidate = startDir / "frontend";
        if(fs::exists(candidate / "index.html")){
            return candidate;
        }
        if(!startDir.has_parent_path()){
            break;
        }
        fs::path parent = startDir.parent_path();
        if(parent == startDir){
            break;
        }
        startDir = parent;
    }
    return std::nullopt;
}

static void initFrontendDir(const fs::path& argv0){
    if(g_frontendDir){
        return;
    }

    // allow manual override
    const char* envFrontend = std::getenv("FRONTEND_DIR");
    if(envFrontend && envFrontend[0] != '\0'){
        try{
            fs::path p(envFrontend);
            if(fs::exists(p / "index.html")){
                g_frontendDir = p;
                g_frontendDiag = std::string("FRONTEND_DIR override: ") + p.string();
                return;
            }
            if(fs::exists(p / "frontend" / "index.html")){
                g_frontendDir = p / "frontend";
                g_frontendDiag = std::string("FRONTEND_DIR override: ") + (p / "frontend").string();
                return;
            }
        }catch(...){
            // ignore
        }
    }

    std::vector<fs::path> starts;
    try{
        starts.push_back(fs::current_path());
    }catch(...){
        // ignore
    }

#ifdef __linux__
    try{
        auto selfExe = getSelfExePath();
        if(selfExe && selfExe->has_parent_path()){
            starts.push_back(selfExe->parent_path());
            g_frontendDiag = std::string("selfExe=") + selfExe->string();
        }
    }catch(...){
        // ignore
    }
#endif

    if(!argv0.empty()){
        try{
            fs::path exePath = argv0;
            if(exePath.is_relative()){
                exePath = fs::absolute(exePath);
            }
            if(exePath.has_parent_path()){
                starts.push_back(exePath.parent_path());
                if(g_frontendDiag.empty()){
                    g_frontendDiag = std::string("argv0=") + exePath.string();
                }else{
                    g_frontendDiag += std::string(" argv0=") + exePath.string();
                }
            }
        }catch(...){
            // ignore
        }
    }

    for(const auto& s : starts){
        auto found = findFrontendDirFrom(s);
        if(found){
            g_frontendDir = *found;
            if(g_frontendDiag.empty()){
                g_frontendDiag = std::string("frontendDir=") + found->string();
            }else{
                g_frontendDiag += std::string(" frontendDir=") + found->string();
            }
            return;
        }
    }

    // keep a small hint for error responses
    try{
        if(g_frontendDiag.empty()){
            g_frontendDiag = std::string("cwd=") + fs::current_path().string();
        }else{
            g_frontendDiag += std::string(" cwd=") + fs::current_path().string();
        }
    }catch(...){
        // ignore
    }
}

static std::optional<std::string> readFileToString(const fs::path& filePath, bool binary){
    std::ios::openmode mode = std::ios::in;
    if(binary){
        mode |= std::ios::binary;
    }
    std::ifstream file(filePath, mode);
    if(!file.is_open()){
        return std::nullopt;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

static std::optional<std::string> readFrontendAsset(const std::string& filename, bool binary){
    if(!g_frontendDir){
        return std::nullopt;
    }
    return readFileToString((*g_frontendDir) / filename, binary);
}
crow::response addSecurityHeaders(crow::response response) {
    response.add_header("Content-Security-Policy", "default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline'; font-src 'self' data:; img-src 'self' data: blob:; object-src 'none'; base-uri 'self'; upgrade-insecure-requests");
    response.add_header("X-Content-Type-Options", "nosniff");
    response.add_header("X-Frame-Options", "DENY");
    response.add_header("Referrer-Policy", "no-referrer");
    response.add_header("X-XSS-Protection", "1; mode=block");
    return response;
}

int main(int argc, char* argv[]){
	// never let an uncaught exception abort the dyno with status 134 without a useful log line.
	std::set_terminate([](){
		std::exception_ptr eptr = std::current_exception();
		if(eptr){
			try{
				std::rethrow_exception(eptr);
			}catch(const std::exception& e){
				std::cerr << "fatal: std::terminate called: " << e.what() << std::endl;
			}catch(...){
				std::cerr << "fatal: std::terminate called: unknown exception" << std::endl;
			}
		}else{
			std::cerr << "fatal: std::terminate called (no active exception)" << std::endl;
		}
		std::_Exit(1);
	});

    crow::SimpleApp app;
    DataCleaner cleaner;
    AuditLog auditLog;

	fs::path argv0;
	if(argc > 0 && argv && argv[0]){
		argv0 = fs::path(argv[0]);
	}
	// heroku crashes with status 134 if we throw during startup (uncaught exception => abort)
	// so keep startup fully exception-safe.
	try{
		initFrontendDir(argv0);
	}catch(const std::exception& e){
		g_frontendDiag = std::string("initFrontendDir failed: ") + e.what();
	}catch(...){
		g_frontendDiag = "initFrontendDir failed: unknown error";
	}

	auto parseIntEnv = [](const char* s, int fallback)->int{
		if(!s) return fallback;
		char* end = nullptr;
		long v = std::strtol(s, &end, 10);
		if(!end) return fallback;
		// allow trailing whitespace
		while(*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n') ++end;
		if(*end != '\0') return fallback;
		if(v <= 0 || v >= 65536) return fallback;
		return static_cast<int>(v);
	};

	int port = parseIntEnv(std::getenv("PORT"), 8080);
	int webConcurrency = parseIntEnv(std::getenv("WEB_CONCURRENCY"), 2);
	if(webConcurrency < 1) webConcurrency = 1;
	if(webConcurrency > 16) webConcurrency = 16;

	CROW_ROUTE(app,"/")
	([](){
		auto html = readFrontendAsset("home.html", false);
		if(!html){
			std::string msg = "could not load frontend/home.html (check that frontend/ is deployed). ";
			msg += g_frontendDiag;
			return crow::response(500, msg);
		}
		auto response = crow::response(*html);
		response.add_header("Content-Type", "text/html; charset=utf-8");
		response.add_header("Cache-Control", "no-cache");
		// homepage does not need inline script. keep a stricter policy.
		response.add_header("Content-Security-Policy", "default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline'; font-src 'self' data:; img-src 'self' data: blob:; object-src 'none'; base-uri 'self'; upgrade-insecure-requests");
		response.add_header("X-Content-Type-Options", "nosniff");
		response.add_header("X-Frame-Options", "DENY");
		response.add_header("Referrer-Policy", "no-referrer");
		response.add_header("X-XSS-Protection", "1; mode=block");
		response.add_header("Strict-Transport-Security", "max-age=31536000; includeSubDomains; preload");
		response.add_header("Permissions-Policy", "geolocation=(), microphone=(), camera=()");
		return response;
	});

		CROW_ROUTE(app, "/features")
		([](){
			auto html = readFrontendAsset("features.html", false);
			if(!html){
				std::string msg = "could not load frontend/features.html (check that frontend/ is deployed). ";
				msg += g_frontendDiag;
				return crow::response(500, msg);
			}
			auto response = crow::response(*html);
			response.add_header("Content-Type", "text/html; charset=utf-8");
			response.add_header("Cache-Control", "no-cache");
			response.add_header("Content-Security-Policy", "default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline'; font-src 'self' data:; img-src 'self' data: blob:; object-src 'none'; base-uri 'self'; upgrade-insecure-requests");
			response.add_header("X-Content-Type-Options", "nosniff");
			response.add_header("X-Frame-Options", "DENY");
			response.add_header("Referrer-Policy", "no-referrer");
			response.add_header("X-XSS-Protection", "1; mode=block");
			response.add_header("Strict-Transport-Security", "max-age=31536000; includeSubDomains; preload");
			response.add_header("Permissions-Policy", "geolocation=(), microphone=(), camera=()");
			return response;
		});

		CROW_ROUTE(app, "/honours-project")
		([](){
			auto html = readFrontendAsset("honours-project.html", false);
			if(!html){
				std::string msg = "could not load frontend/honours-project.html (check that frontend/ is deployed). ";
				msg += g_frontendDiag;
				return crow::response(500, msg);
			}
			auto response = crow::response(*html);
			response.add_header("Content-Type", "text/html; charset=utf-8");
			response.add_header("Cache-Control", "no-cache");
			response.add_header("Content-Security-Policy", "default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline'; font-src 'self' data:; img-src 'self' data: blob:; object-src 'none'; base-uri 'self'; upgrade-insecure-requests");
			response.add_header("X-Content-Type-Options", "nosniff");
			response.add_header("X-Frame-Options", "DENY");
			response.add_header("Referrer-Policy", "no-referrer");
			response.add_header("X-XSS-Protection", "1; mode=block");
			response.add_header("Strict-Transport-Security", "max-age=31536000; includeSubDomains; preload");
			response.add_header("Permissions-Policy", "geolocation=(), microphone=(), camera=()");
			return response;
		});

	CROW_ROUTE(app, "/robots.txt")
		([](const crow::request& req){
			auto base = getSeoBaseUrl(req);
			std::string body;
			body += "User-agent: *\n";
				body += "Allow: /\n";
			body += "Allow: /app\n";
			body += "Disallow: /api/\n";
			body += "Sitemap: " + base + "/sitemap.xml\n";
			auto res = crow::response(body);
			res.add_header("Content-Type", "text/plain; charset=utf-8");
			res.add_header("Cache-Control", "public, max-age=3600");
			return res;
		});

	CROW_ROUTE(app, "/sitemap.xml")
			([](const crow::request& req){
			auto base = getSeoBaseUrl(req);
				// small sitemap: homepage + key public pages
			std::string xml;
			xml += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
			xml += "<urlset xmlns=\"http://www.sitemaps.org/schemas/sitemap/0.9\">\n";
				xml += "  <url><loc>" + base + "/</loc></url>\n";
				xml += "  <url><loc>" + base + "/features</loc></url>\n";
				xml += "  <url><loc>" + base + "/honours-project</loc></url>\n";
				xml += "  <url><loc>" + base + "/app</loc></url>\n";
			xml += "</urlset>\n";
			auto res = crow::response(xml);
			res.add_header("Content-Type", "application/xml; charset=utf-8");
			res.add_header("Cache-Control", "public, max-age=3600");
			return res;
		});

    CROW_ROUTE(app,"/app")
		([](const crow::request& req){
			// next commits will use this for crawler detection
			auto ua = req.get_header_value("User-Agent");
			auto isCrawler = isSeoCrawlerUserAgent(ua);
			const char* htmlName = isCrawler ? "app-seo.html" : "index.html";
			auto html = readFrontendAsset(htmlName, false);
		if(!html){
				std::string msg = std::string("could not load frontend/") + htmlName + " (check that frontend/ is deployed). ";
			msg += g_frontendDiag;
			return crow::response(500, msg);
		}
		auto response = crow::response(*html);
		response.add_header("Content-Type", "text/html; charset=utf-8");
			response.add_header("Cache-Control", "no-cache");
			response.add_header("Vary", "User-Agent");
			response.add_header("X-Render-Mode", isCrawler ? "seo" : "app");
		// no google fonts: keep everything local. allow inline script/style because index.html is a single-file app.
		response.add_header("Content-Security-Policy", "default-src 'self'; script-src 'self' 'unsafe-eval' 'unsafe-inline'; style-src 'self' 'unsafe-inline'; font-src 'self' data:; img-src 'self' data: blob:; object-src 'none'; base-uri 'self'; upgrade-insecure-requests; worker-src 'self'; child-src 'self'");
		response.add_header("X-Content-Type-Options", "nosniff");
		response.add_header("X-Frame-Options", "DENY");
		response.add_header("Referrer-Policy", "no-referrer");
		response.add_header("X-XSS-Protection", "1; mode=block");
		response.add_header("Strict-Transport-Security", "max-age=31536000; includeSubDomains; preload");
		response.add_header("Permissions-Policy", "geolocation=(), microphone=(), camera=()");
		return response;
	});

    CROW_ROUTE(app,"/algorithms.js")
	([](){
		auto js = readFrontendAsset("algorithms.js", false);
		if(!js){
			return crow::response(404);
		}
		auto response=crow::response(*js);
        response.add_header("Content-Type","application/javascript");
        response.add_header("Content-Security-Policy", "default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline'; font-src 'self' data:; img-src 'self' data: blob:; object-src 'none'; base-uri 'self'; upgrade-insecure-requests");
        response.add_header("X-Content-Type-Options", "nosniff");
        response.add_header("X-Frame-Options", "DENY");
        response.add_header("Referrer-Policy", "no-referrer");
        return response;
    });

    CROW_ROUTE(app, "/algorithms.wasm")
	([](){
		auto wasm = readFrontendAsset("algorithms.wasm", true);
		if(!wasm){
			return crow::response(404);
		}
		auto response=crow::response(*wasm);
        response.add_header("Content-Type","application/wasm");
        response.add_header("Content-Security-Policy", "default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline'; font-src 'self' data:; img-src 'self' data: blob:; object-src 'none'; base-uri 'self'; upgrade-insecure-requests");
        response.add_header("X-Content-Type-Options", "nosniff");
        response.add_header("X-Frame-Options", "DENY");
        response.add_header("Referrer-Policy", "no-referrer");
        return response;
    });

    CROW_ROUTE(app,"/manifest.json")
	([](){
		auto manifest = readFrontendAsset("manifest.json", false);
		if(!manifest){
			return crow::response(404);
		}
		auto response=crow::response(*manifest);
        response.add_header("Content-Type","application/json");
	    response.add_header("Cache-Control", "public, max-age=3600");
        response.add_header("Content-Security-Policy", "default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline'; font-src 'self' data:; img-src 'self' data: blob:; object-src 'none'; base-uri 'self'; upgrade-insecure-requests");
        response.add_header("X-Content-Type-Options", "nosniff");
        response.add_header("X-Frame-Options", "DENY");
        response.add_header("Referrer-Policy", "no-referrer");
        return response;
    });

    CROW_ROUTE(app,"/service-worker.js")
	([](){
		auto sw = readFrontendAsset("service-worker.js", false);
		if(!sw){
			return crow::response(404);
		}
		auto response=crow::response(*sw);
        response.add_header("Content-Type","application/javascript");
	    // make sure browsers revalidate the service worker script, so updates roll out
	    response.add_header("Cache-Control", "no-cache");
        response.add_header("Content-Security-Policy", "default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline'; font-src 'self' data:; img-src 'self' data: blob:; object-src 'none'; base-uri 'self'; upgrade-insecure-requests");
        response.add_header("X-Content-Type-Options", "nosniff");
        response.add_header("X-Frame-Options", "DENY");
        response.add_header("Referrer-Policy", "no-referrer");
        return response;
    });

    CROW_ROUTE(app,"/api/parse").methods("POST"_method)
    ([&cleaner](const crow::request& req){
        auto data=req.body;
        auto parsed=cleaner.parseCSV(data);
        crow::json::wvalue result;
        result["rows"]=parsed.size();
        result["message"]="CSV parsed successfully";
        auto response = crow::response(result);
        response.add_header("Content-Security-Policy", "default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline'; font-src 'self' data:; img-src 'self' data: blob:; object-src 'none'; base-uri 'self'; upgrade-insecure-requests");
        response.add_header("X-Content-Type-Options", "nosniff");
        response.add_header("X-Frame-Options", "DENY");
        response.add_header("Referrer-Policy", "no-referrer");
        return response;
    });

    CROW_ROUTE(app,"/api/detect-missing").methods("POST"_method)
    ([&cleaner, &auditLog](const crow::request& req){
        auto data=req.body;
        auto parsed=cleaner.parseCSV(data);
        auto missing=cleaner.detectMissingValues(parsed);
        int missingCount=0;
        for(const auto& row:missing){
            for(bool isMissing:row){
                if(isMissing){
                    ++missingCount;
                }
            }
        }
        auditLog.addEntry("Detect Missing", missingCount, parsed.size(), parsed.size());
        crow::json::wvalue result;
        result["missing"]=missingCount;
        result["message"]="Missing values detected successfully";
        return crow::response(result);
    });

    CROW_ROUTE(app,"/api/detect-duplicates").methods("POST"_method)
    ([&cleaner, &auditLog](const crow::request& req){
        auto data=req.body;
        auto parsed=cleaner.parseCSV(data);
        auto duplicates=cleaner.detectDuplicates(parsed);
        int duplicateCount=0;
        for(bool isDup:duplicates){
            if(isDup){
                ++duplicateCount;
            }
        }
        auditLog.addEntry("Detect Duplicates", duplicateCount, parsed.size(), parsed.size());
        crow::json::wvalue result;
        result["duplicates"]=duplicateCount;
        result["message"]="Duplicate rows detected successfully";
        return crow::response(result);
    });

    CROW_ROUTE(app,"/api/clean").methods("POST"_method)
    ([&cleaner, &auditLog](const crow::request& req){
        auto data=req.body;
        auto parsed=cleaner.parseCSV(data);
        auto cleaned=cleaner.cleanData(parsed);
        int removedRows=parsed.size()-cleaned.size();
        auditLog.addEntry("Remove Duplicates", removedRows, parsed.size(), cleaned.size());
        crow::json::wvalue result;
        result["originalRows"]=parsed.size();
        result["cleanedRows"]=cleaned.size();
        result["removedRows"]=removedRows;
        result["message"]="Data cleaned successfully";
        return crow::response(result);
    });

    CROW_ROUTE(app,"/api/detect-whitespace").methods("POST"_method)
    ([&cleaner](const crow::request& req){
        std::string csvData=req.body;
        auto parsed=cleaner.parseCSV(csvData);
        int count=0;
        for(const auto& row:parsed){
            for(const auto& cell:row){
                if(!cell.empty()&&(cell.front()==' '||cell.back()==' '||cell.front()=='\t'||cell.back()=='\t')){
                    count++;
                }
            }
        }
        crow::json::wvalue result;
        result["message"]="whitespace detected";
        result["cellsWithWhitespace"]=count;
        result["mode"]="api";
        return result;
    });

    CROW_ROUTE(app,"/api/trim-whitespace").methods("POST"_method)
    ([&cleaner, &auditLog](const crow::request& req){
        std::string csvData=req.body;
        auto parsed=cleaner.parseCSV(csvData);
        int count=0;
        std::stringstream cleaned;
        for(const auto& row:parsed){
            for(size_t i=0;i<row.size();++i){
                std::string cell=row[i];
                size_t start=cell.find_first_not_of(" \t");
                size_t end=cell.find_last_not_of(" \t");
                if(start!=std::string::npos){
                    cell=cell.substr(start,end-start+1);
                    count++;
                }else{
                    cell="";
                }
                if(i>0)cleaned<<",";
                cleaned<<cell;
            }
            cleaned<<"\n";
        }
        auditLog.addEntry("Trim Whitespace", count, parsed.size(), parsed.size());
        crow::json::wvalue result;
        result["message"]="whitespace trimmed";
        result["cellsTrimmed"]=count;
        result["cleaned"]=cleaned.str();
        result["mode"]="api";
        return result;
    });

    CROW_ROUTE(app,"/api/to-uppercase").methods("POST"_method)
    ([&cleaner, &auditLog](const crow::request& req){
        std::string csvData=req.body;
        auto parsed=cleaner.parseCSV(csvData);
        int count=0;
        std::stringstream cleaned;
        for(const auto& row:parsed){
            for(size_t i=0;i<row.size();++i){
                std::string cell=row[i];
                for(char& c:cell){
                    if(c>='a'&&c<='z'){c=c-32;count++;}
                }
                if(i>0)cleaned<<",";
                cleaned<<cell;
            }
            cleaned<<"\n";
        }
        auditLog.addEntry("To Uppercase", count, parsed.size(), parsed.size());
        crow::json::wvalue result;
        result["message"]="converted to uppercase";
        result["cleaned"]=cleaned.str();
        result["mode"]="api";
        return result;
    });

    CROW_ROUTE(app,"/api/to-lowercase").methods("POST"_method)
    ([&cleaner, &auditLog](const crow::request& req){
        std::string csvData=req.body;
        auto parsed=cleaner.parseCSV(csvData);
        int count=0;
        std::stringstream cleaned;
        for(const auto& row:parsed){
            for(size_t i=0;i<row.size();++i){
                std::string cell=row[i];
                for(char& c:cell){
                    if(c>='A'&&c<='Z'){c=c+32;count++;}
                }
                if(i>0)cleaned<<",";
                cleaned<<cell;
            }
            cleaned<<"\n";
        }
        auditLog.addEntry("To Lowercase", count, parsed.size(), parsed.size());
        crow::json::wvalue result;
        result["message"]="converted to lowercase";
        result["cleaned"]=cleaned.str();
        result["mode"]="api";
        return result;
    });
    
    CROW_ROUTE(app,"/favicon.ico")
	([](){
		auto ico = readFrontendAsset("favicon.ico", true);
		if(!ico){
			return crow::response(404);
		}
		auto response=crow::response(*ico);
        response.add_header("Content-Type","image/x-icon");
	    response.add_header("Cache-Control", "public, max-age=86400");
        response.add_header("Content-Security-Policy", "default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline'; font-src 'self' data:; img-src 'self' data: blob:; object-src 'none'; base-uri 'self'; upgrade-insecure-requests");
        response.add_header("X-Content-Type-Options", "nosniff");
        response.add_header("X-Frame-Options", "DENY");
        response.add_header("Referrer-Policy", "no-referrer");
        return response;
    });

	CROW_ROUTE(app,"/favicon-32x32.png")
	([](){
		auto png = readFrontendAsset("favicon-32x32.png", true);
		if(!png){
			return crow::response(404);
		}
		auto response=crow::response(*png);
		response.add_header("Content-Type","image/png");
		response.add_header("Cache-Control", "public, max-age=86400");
		response.add_header("Content-Security-Policy", "default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline'; font-src 'self' data:; img-src 'self' data: blob:; object-src 'none'; base-uri 'self'; upgrade-insecure-requests");
		response.add_header("X-Content-Type-Options", "nosniff");
		response.add_header("X-Frame-Options", "DENY");
		response.add_header("Referrer-Policy", "no-referrer");
		return response;
	});

	CROW_ROUTE(app,"/favicon-16x16.png")
	([](){
		auto png = readFrontendAsset("favicon-16x16.png", true);
		if(!png){
			return crow::response(404);
		}
		auto response=crow::response(*png);
		response.add_header("Content-Type","image/png");
		response.add_header("Cache-Control", "public, max-age=86400");
		response.add_header("Content-Security-Policy", "default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline'; font-src 'self' data:; img-src 'self' data: blob:; object-src 'none'; base-uri 'self'; upgrade-insecure-requests");
		response.add_header("X-Content-Type-Options", "nosniff");
		response.add_header("X-Frame-Options", "DENY");
		response.add_header("Referrer-Policy", "no-referrer");
		return response;
	});

	CROW_ROUTE(app,"/apple-touch-icon.png")
	([](){
		auto png = readFrontendAsset("apple-touch-icon.png", true);
		if(!png){
			return crow::response(404);
		}
		auto response=crow::response(*png);
		response.add_header("Content-Type","image/png");
		response.add_header("Cache-Control", "public, max-age=86400");
		response.add_header("Content-Security-Policy", "default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline'; font-src 'self' data:; img-src 'self' data: blob:; object-src 'none'; base-uri 'self'; upgrade-insecure-requests");
		response.add_header("X-Content-Type-Options", "nosniff");
		response.add_header("X-Frame-Options", "DENY");
		response.add_header("Referrer-Policy", "no-referrer");
		return response;
	});

    CROW_ROUTE(app,"/android-chrome-192x192.png")
	([](){
		auto png = readFrontendAsset("android-chrome-192x192.png", true);
		if(!png){
			return crow::response(404);
		}
		auto response=crow::response(*png);
        response.add_header("Content-Type","image/png");
	    response.add_header("Cache-Control", "public, max-age=86400");
        response.add_header("Content-Security-Policy", "default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline'; font-src 'self' data:; img-src 'self' data: blob:; object-src 'none'; base-uri 'self'; upgrade-insecure-requests");
        response.add_header("X-Content-Type-Options", "nosniff");
        response.add_header("X-Frame-Options", "DENY");
        response.add_header("Referrer-Policy", "no-referrer");
        return response;
    });

    CROW_ROUTE(app,"/android-chrome-512x512.png")
	([](){
		auto png = readFrontendAsset("android-chrome-512x512.png", true);
		if(!png){
			return crow::response(404);
		}
		auto response=crow::response(*png);
        response.add_header("Content-Type","image/png");
	    response.add_header("Cache-Control", "public, max-age=86400");
        response.add_header("Content-Security-Policy", "default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline'; font-src 'self' data:; img-src 'self' data: blob:; object-src 'none'; base-uri 'self'; upgrade-insecure-requests");
        response.add_header("X-Content-Type-Options", "nosniff");
        response.add_header("X-Frame-Options", "DENY");
        response.add_header("Referrer-Policy", "no-referrer");
        return response;
    });

    CROW_ROUTE(app,"/api/detect-null-values").methods("POST"_method)
    ([&cleaner](const crow::request& req){
        std::string csvData=req.body;
        auto parsed=cleaner.parseCSV(csvData);
        int count=0;
        for(const auto& row:parsed){
            for(const auto& cell:row){
                if(cell.empty()||cell=="N/A"||cell=="n/a"||cell=="NA"||cell=="null"||cell=="NULL"||cell=="None"||cell=="NONE"||cell=="-"||cell=="?"){count++;}
            }
        }
        crow::json::wvalue result;
        result["message"]="null values detected";
        result["cellsWithNullValues"]=count;
        result["mode"]="api";
        return result;
    });

    CROW_ROUTE(app,"/api/standardise-null-values").methods("POST"_method)
    ([&cleaner, &auditLog](const crow::request& req){
        std::string csvData=req.body;
        auto parsed=cleaner.parseCSV(csvData);
        int count=0;
        std::stringstream cleaned;
        for(const auto& row:parsed){
            for(size_t i=0;i<row.size();++i){
                std::string cell=row[i];
                if(cell.empty()||cell=="N/A"||cell=="n/a"||cell=="NA"||cell=="null"||cell=="NULL"||cell=="None"||cell=="NONE"||cell=="-"||cell=="?"){cell="";count++;}
                if(i>0)cleaned<<",";
                cleaned<<cell;
            }
            cleaned<<"\n";
        }
        auditLog.addEntry("Standardise Null Values", count, parsed.size(), parsed.size());
        crow::json::wvalue result;
        result["message"]="null values standardised";
        result["cellsStandardised"]=count;
        result["cleaned"]=cleaned.str();
        result["mode"]="api";
        return result;
    });

    CROW_ROUTE(app,"/api/get-audit-log").methods("GET"_method)
    ([&auditLog](){
        crow::json::wvalue result;
        result["operations"]=crow::json::wvalue::list();
        int idx=0;
        for(const auto& entry:auditLog.entries){
            result["operations"][idx]["operationName"]=entry.operationName;
            result["operations"][idx]["cellsAffected"]=entry.cellsAffected;
            result["operations"][idx]["rowsBefore"]=entry.rowsBefore;
            result["operations"][idx]["rowsAfter"]=entry.rowsAfter;
            result["operations"][idx]["timestamp"]=entry.timestamp;
            idx++;
        }
        return result;
    });

    CROW_ROUTE(app,"/api/detect-outliers").methods("POST"_method)
    ([&cleaner](const crow::request& req){
        std::string csvData=req.body;
        auto parsed=cleaner.parseCSV(csvData);
        if(parsed.size()<2){crow::json::wvalue result; result["outliers"]=0;result["rowsWithOutliers"]=0;result["message"]="insufficient data for outlier detection";return crow::response(result);}
        auto isNumericStr=[](const std::string& str)->bool{
            if(str.empty())return false;
            size_t start=0;
            if(str[0]=='-'||str[0]=='+')start=1;
            if(start>=str.length())return false;
            bool hasDecimal=false;
            for(size_t i=start;i<str.length();i++){if(str[i]=='.'){if(hasDecimal)return false;hasDecimal=true;}else if(str[i]<'0'||str[i]>'9')return false;}
            return true;
        };
        int outlierCount=0;
        std::set<int> outlierRows;
        size_t numCols=parsed[0].size();
        struct OutlierDetail{int row;int col;double val;double lower;double upper;};
        std::vector<OutlierDetail> outlierDetails;
        for(size_t col=0;col<numCols;col++){
            std::vector<double> values;
            std::vector<int> rowIndices;
            for(size_t row=1;row<parsed.size();row++){if(col<parsed[row].size()&&isNumericStr(parsed[row][col])){double val=std::stod(parsed[row][col]);values.push_back(val);rowIndices.push_back(row);}}
            if(values.size()<4)continue;
            std::vector<double> sortedValues=values;
            std::sort(sortedValues.begin(),sortedValues.end());
            size_t n=sortedValues.size();
            double q1=sortedValues[n/4];
            double q3=sortedValues[3*n/4];
            double iqr=q3-q1;
            double lower=q1-1.5*iqr;
            double upper=q3+1.5*iqr;
            for(size_t i=0;i<values.size();i++){if(values[i]<lower||values[i]>upper){outlierCount++;outlierRows.insert(rowIndices[i]);outlierDetails.push_back({rowIndices[i],(int)col,values[i],lower,upper});}}
        }
        crow::json::wvalue result;
        result["outliers"]=outlierCount;
        result["rowsWithOutliers"]=(int)outlierRows.size();
        result["message"]="outlier detection completed";
        result["mode"]="api";
        result["details"]=crow::json::wvalue::list();
        for(size_t i=0;i<outlierDetails.size();i++){
            result["details"][i]["row"]=outlierDetails[i].row;
            result["details"][i]["column"]=outlierDetails[i].col;
            result["details"][i]["value"]=outlierDetails[i].val;
            result["details"][i]["lower"]=outlierDetails[i].lower;
            result["details"][i]["upper"]=outlierDetails[i].upper;
        }
        return crow::response(result);
    });

    CROW_ROUTE(app,"/api/remove-outliers").methods("POST"_method)
    ([&cleaner, &auditLog](const crow::request& req){
        std::string csvData=req.body;
        auto parsed=cleaner.parseCSV(csvData);
        if(parsed.size()<2){crow::json::wvalue result; result["originalRows"]=parsed.size();result["cleanedRows"]=parsed.size();result["removedRows"]=0;result["message"]="insufficient data for outlier removal";return crow::response(result);}
        auto isNumericStr=[](const std::string& str)->bool{
            if(str.empty())return false;
            size_t start=0;
            if(str[0]=='-'||str[0]=='+')start=1;
            if(start>=str.length())return false;
            bool hasDecimal=false;
            for(size_t i=start;i<str.length();i++){if(str[i]=='.'){if(hasDecimal)return false;hasDecimal=true;}else if(str[i]<'0'||str[i]>'9')return false;}
            return true;
        };
        std::set<int> outlierRows;
        size_t numCols=parsed[0].size();
        for(size_t col=0;col<numCols;col++){
            std::vector<double> values;
            std::vector<int> rowIndices;
            for(size_t row=1;row<parsed.size();row++){if(col<parsed[row].size()&&isNumericStr(parsed[row][col])){double val=std::stod(parsed[row][col]);values.push_back(val);rowIndices.push_back(row);}}
            if(values.size()<4)continue;
            std::vector<double> sortedValues=values;
            std::sort(sortedValues.begin(),sortedValues.end());
            size_t n=sortedValues.size();
            double q1=sortedValues[n/4];
            double q3=sortedValues[3*n/4];
            double iqr=q3-q1;
            double lower=q1-1.5*iqr;
            double upper=q3+1.5*iqr;
            for(size_t i=0;i<values.size();i++){if(values[i]<lower||values[i]>upper){outlierRows.insert(rowIndices[i]);}}
        }
        std::stringstream cleanedCSV;
        for(size_t i=0;i<parsed.size();i++){
            if(outlierRows.find(i)==outlierRows.end()){
                for(size_t j=0;j<parsed[i].size();j++){if(j>0)cleanedCSV<<",";cleanedCSV<<parsed[i][j];}
                cleanedCSV<<"\n";
            }
        }
        int removedRows=outlierRows.size();
        int cleanedRows=parsed.size()-removedRows;
        auditLog.addEntry("Remove Outliers", removedRows, parsed.size(), cleanedRows);
        crow::json::wvalue result;
        result["originalRows"]=parsed.size();
        result["cleanedRows"]=cleanedRows;
        result["removedRows"]=removedRows;
        result["cleaned"]=cleanedCSV.str();
        result["message"]="outliers removed successfully";
        result["mode"]="api";
        return crow::response(result);
    });

    CROW_ROUTE(app,"/api/detect-inconsistent-values").methods("POST"_method)
    ([&cleaner](const crow::request& req){
        std::string csvData=req.body;
        auto parsed=cleaner.parseCSV(csvData);
        if(parsed.size()<2){crow::json::wvalue result; result["inconsistentCount"]=0;result["message"]="insufficient data for inconsistency detection";return crow::response(result);}
        auto levenshteinDist=[](const std::string& s1,const std::string& s2)->int{
            size_t m=s1.length();
            size_t n=s2.length();
            std::vector<std::vector<int>> dp(m+1,std::vector<int>(n+1,0));
            for(size_t i=0;i<=m;i++)dp[i][0]=i;
            for(size_t j=0;j<=n;j++)dp[0][j]=j;
            for(size_t i=1;i<=m;i++){for(size_t j=1;j<=n;j++){if(s1[i-1]==s2[j-1]){dp[i][j]=dp[i-1][j-1];}else{dp[i][j]=1+std::min({dp[i-1][j],dp[i][j-1],dp[i-1][j-1]});}}}
            return dp[m][n];
        };
        auto toLower=[](const std::string& str)->std::string{std::string result=str;std::transform(result.begin(),result.end(),result.begin(),::tolower);return result;};
        auto isNameColumn=[&toLower](const std::string& colName)->bool{std::string lower=toLower(colName);const std::vector<std::string> nameKeywords={"player","name","first name","last name","full name","employee","customer","person","author","contact","user","username","email","phone","address","street","city","country","state","zip","postal"};for(const auto& keyword:nameKeywords){if(lower.find(keyword)!=std::string::npos){return true;}}return false;};
        int inconsistentCount=0;
        std::vector<std::string> skippedNameColumns;
        std::map<std::string,std::map<std::string,std::vector<std::string>>> suggestedMappings;
        size_t numCols=parsed[0].size();
        for(size_t col=0;col<numCols;col++){
            std::string colName=parsed[0][col];
            if(isNameColumn(colName)){skippedNameColumns.push_back(colName);continue;}
            std::vector<std::string> colValues;
            for(size_t row=1;row<parsed.size();row++){if(col<parsed[row].size()&&!parsed[row][col].empty()){colValues.push_back(parsed[row][col]);}}
            if(colValues.size()<2)continue;
            std::set<std::string> processedValues;
            for(size_t i=0;i<colValues.size();i++){
                if(processedValues.count(colValues[i]))continue;
                processedValues.insert(colValues[i]);
                for(size_t j=i+1;j<colValues.size();j++){
                    if(processedValues.count(colValues[j]))continue;
                    int distance=levenshteinDist(colValues[i],colValues[j]);
                    if(distance<=2){
                        inconsistentCount++;
                        if(suggestedMappings[colName].find(colValues[i])==suggestedMappings[colName].end()){suggestedMappings[colName][colValues[i]]=std::vector<std::string>();}
                        suggestedMappings[colName][colValues[i]].push_back(colValues[j]);
                    }
                }
            }
        }
        crow::json::wvalue result;
        result["inconsistentCount"]=inconsistentCount;
        result["message"]="inconsistency detection completed";
        result["mode"]="api";
        result["suggestedMappings"]=crow::json::wvalue::object();
        for(auto& colEntry:suggestedMappings){result["suggestedMappings"][colEntry.first]=crow::json::wvalue::object();for(auto& valEntry:colEntry.second){result["suggestedMappings"][colEntry.first][valEntry.first]=valEntry.second[0];}}
        if(!skippedNameColumns.empty()){result["warning"]="name columns excluded from detection to protect data integrity";std::string colsJson="[";for(size_t i=0;i<skippedNameColumns.size();i++){if(i>0)colsJson+=",";colsJson+="\""+skippedNameColumns[i]+"\"";}colsJson+="]";result["skippedColumns"]=crow::json::load(colsJson);}
        return crow::response(result);
    });

    CROW_ROUTE(app,"/api/standardise-values").methods("POST"_method)
    ([&cleaner, &auditLog](const crow::request& req){
        auto body=crow::json::load(req.body);
        if(!body){crow::json::wvalue result; result["message"]="invalid request body";return crow::response(400);}
        std::string csvData=body["csvData"].s();
        std::string mappingsJson=body["mappings"].s();
        auto parsed=cleaner.parseCSV(csvData);
        if(parsed.size()<2){crow::json::wvalue result; result["originalRows"]=parsed.size();result["cleanedRows"]=parsed.size();result["valuesStandardised"]=0;result["message"]="insufficient data for standardisation";return crow::response(result);}
        std::map<std::string,std::map<std::string,std::string>> columnMappings;
        size_t pos=0;
        std::string currentCol;
        while(pos<mappingsJson.length()){
            if(mappingsJson[pos]=='"'){
                size_t endQuote=mappingsJson.find('"',pos+1);
                if(endQuote!=std::string::npos){
                    std::string key=mappingsJson.substr(pos+1,endQuote-pos-1);
                    pos=endQuote+1;
                    while(pos<mappingsJson.length()&&(mappingsJson[pos]==':'||mappingsJson[pos]==' '||mappingsJson[pos]=='\t'||mappingsJson[pos]=='\n')){pos++;}
                    if(pos<mappingsJson.length()&&mappingsJson[pos]=='{'){
                        currentCol=key;
                        pos++;
                        while(pos<mappingsJson.length()&&mappingsJson[pos]!='}'){
                            if(mappingsJson[pos]=='"'){
                                size_t keyEnd=mappingsJson.find('"',pos+1);
                                if(keyEnd!=std::string::npos){
                                    std::string fromVal=mappingsJson.substr(pos+1,keyEnd-pos-1);
                                    pos=keyEnd+1;
                                    while(pos<mappingsJson.length()&&(mappingsJson[pos]==':'||mappingsJson[pos]==' '||mappingsJson[pos]=='\t'||mappingsJson[pos]=='\n')){pos++;}
                                    if(pos<mappingsJson.length()&&mappingsJson[pos]=='"'){
                                        size_t valEnd=mappingsJson.find('"',pos+1);
                                        if(valEnd!=std::string::npos){
                                            std::string toVal=mappingsJson.substr(pos+1,valEnd-pos-1);
                                            columnMappings[currentCol][fromVal]=toVal;
                                            pos=valEnd+1;
                                        }else{pos++;}
                                    }else{pos++;}
                                }else{pos++;}
                            }else{pos++;}
                        }
                    }else{pos++;}
                }else{pos++;}
            }else{pos++;}
        }
        std::stringstream cleanedCSV;
        int valuesStandardised=0;
        for(size_t row=0;row<parsed.size();row++){
            for(size_t col=0;col<parsed[row].size();col++){
                std::string cell=parsed[row][col];
                if(row>0&&col<parsed[0].size()){std::string colName=parsed[0][col];if(columnMappings.count(colName)&&columnMappings[colName].count(cell)){cell=columnMappings[colName][cell];valuesStandardised++;}}
                if(col>0)cleanedCSV<<",";
                cleanedCSV<<cell;
            }
            cleanedCSV<<"\n";
        }
        auditLog.addEntry("Standardise Values", valuesStandardised, parsed.size(), parsed.size());
        crow::json::wvalue result;
        result["originalRows"]=parsed.size();
        result["cleanedRows"]=parsed.size();
        result["valuesStandardised"]=valuesStandardised;
        result["cleaned"]=cleanedCSV.str();
        result["message"]="values standardised successfully";
        result["mode"]="api";
        return crow::response(result);
    });

    CROW_ROUTE(app,"/api/detect-data-types").methods("POST"_method)
    ([&cleaner](const crow::request& req){
        std::string csvData=req.body;
        auto parsed=cleaner.parseCSV(csvData);
        crow::json::wvalue result;
        result["types"]=crow::json::wvalue::object();
        if(parsed.size()<2){result["message"]="insufficient data for type detection";result["mode"]="api";return crow::response(result);}
        auto isNumericStr=[](const std::string& str)->bool{if(str.empty())return false;size_t start=0;if(str[0]=='-'||str[0]=='+')start=1;if(start>=str.length())return false;bool hasDecimal=false;for(size_t i=start;i<str.length();i++){if(str[i]=='.'){if(hasDecimal)return false;hasDecimal=true;}else if(str[i]<'0'||str[i]>'9')return false;}return true;};
        auto isDateStr=[](const std::string& str)->bool{if(str.length()<8)return false;int slashCount=0,dashCount=0;for(char c:str){if(c=='/')slashCount++;if(c=='-')dashCount++;}if(slashCount==2||dashCount==2)return true;return false;};
        auto isBoolStr=[](const std::string& str)->bool{std::string lower=str;std::transform(lower.begin(),lower.end(),lower.begin(),::tolower);return lower=="true"||lower=="false"||lower=="yes"||lower=="no"||lower=="1"||lower=="0";};
        size_t numCols=parsed[0].size();
        for(size_t col=0;col<numCols;col++){
            std::string colName=parsed[0][col];
            int numericCount=0,dateCount=0,boolCount=0,textCount=0,emptyCount=0;
            for(size_t row=1;row<parsed.size();row++){
                if(col>=parsed[row].size()||parsed[row][col].empty()){emptyCount++;continue;}
                std::string val=parsed[row][col];
                if(isNumericStr(val))numericCount++;
                else if(isDateStr(val))dateCount++;
                else if(isBoolStr(val))boolCount++;
                else textCount++;
            }
            int total=numericCount+dateCount+boolCount+textCount;
            if(total==0){result["types"][colName]="empty";}
            else if(numericCount>total*0.8)result["types"][colName]="numeric";
            else if(dateCount>total*0.8)result["types"][colName]="date";
            else if(boolCount>total*0.8)result["types"][colName]="boolean";
            else result["types"][colName]="text";
        }
        result["message"]="data types detected";
        result["mode"]="api";
        return crow::response(result);
    });

    CROW_ROUTE(app,"/api/detect-encoding").methods("POST"_method)
    ([](const crow::request& req){
        std::string csvData=req.body;
        crow::json::wvalue result;
        std::string encoding="UTF-8";
        bool hasHighBytes=false;
        bool hasBOM=false;
        int nonAsciiCount=0;
        if(csvData.size()>=3&&(unsigned char)csvData[0]==0xEF&&(unsigned char)csvData[1]==0xBB&&(unsigned char)csvData[2]==0xBF){hasBOM=true;encoding="UTF-8 with BOM";}
        else if(csvData.size()>=2&&(unsigned char)csvData[0]==0xFF&&(unsigned char)csvData[1]==0xFE){encoding="UTF-16 LE";}
        else if(csvData.size()>=2&&(unsigned char)csvData[0]==0xFE&&(unsigned char)csvData[1]==0xFF){encoding="UTF-16 BE";}
        else{
            for(size_t i=0;i<csvData.size()&&i<10000;i++){
                unsigned char c=(unsigned char)csvData[i];
                if(c>127){hasHighBytes=true;nonAsciiCount++;}
            }
            if(!hasHighBytes)encoding="ASCII";
            else{
                bool validUtf8=true;
                for(size_t i=0;i<csvData.size()&&i<10000;i++){
                    unsigned char c=(unsigned char)csvData[i];
                    if(c<=127)continue;
                    int expectedBytes=0;
                    if((c&0xE0)==0xC0)expectedBytes=1;
                    else if((c&0xF0)==0xE0)expectedBytes=2;
                    else if((c&0xF8)==0xF0)expectedBytes=3;
                    else{validUtf8=false;break;}
                    for(int j=0;j<expectedBytes&&i+1<csvData.size();j++){
                        i++;
                        if(((unsigned char)csvData[i]&0xC0)!=0x80){validUtf8=false;break;}
                    }
                }
                if(validUtf8)encoding="UTF-8";
                else encoding="Latin-1 (ISO-8859-1)";
            }
        }
        result["encoding"]=encoding;
        result["hasBOM"]=hasBOM;
        result["nonAsciiCharacters"]=nonAsciiCount;
        result["message"]="encoding detected";
        result["mode"]="api";
        return crow::response(result);
    });

    CROW_ROUTE(app,"/api/detect-patterns").methods("POST"_method)
    ([&cleaner](const crow::request& req){
        std::string csvData=req.body;
        auto parsed=cleaner.parseCSV(csvData);
        crow::json::wvalue result;
        result["patterns"]=crow::json::wvalue::list();
        if(parsed.size()<2){result["message"]="insufficient data for pattern detection";result["mode"]="api";return crow::response(result);}
        auto isEmail=[](const std::string& str)->bool{size_t at=str.find('@');if(at==std::string::npos||at==0||at==str.length()-1)return false;size_t dot=str.find('.',at);return dot!=std::string::npos&&dot>at+1&&dot<str.length()-1;};
        auto isPhone=[](const std::string& str)->bool{int digits=0;for(char c:str){if(c>='0'&&c<='9')digits++;}return digits>=7&&digits<=15;};
        auto isPostalCode=[](const std::string& str)->bool{if(str.length()<3||str.length()>10)return false;int digits=0,letters=0;for(char c:str){if(c>='0'&&c<='9')digits++;if((c>='A'&&c<='Z')||(c>='a'&&c<='z'))letters++;}return(digits>=2&&letters>=1)||(digits>=3&&digits<=5&&letters==0);};
        auto isURL=[](const std::string& str)->bool{return str.find("http://")==0||str.find("https://")==0||str.find("www.")==0;};
        size_t numCols=parsed[0].size();
        int patternIdx=0;
        for(size_t col=0;col<numCols;col++){
            std::string colName=parsed[0][col];
            int emailCount=0,phoneCount=0,postalCount=0,urlCount=0,totalValues=0;
            for(size_t row=1;row<parsed.size();row++){
                if(col>=parsed[row].size()||parsed[row][col].empty())continue;
                std::string val=parsed[row][col];
                totalValues++;
                if(isEmail(val))emailCount++;
                else if(isPhone(val))phoneCount++;
                else if(isPostalCode(val))postalCount++;
                else if(isURL(val))urlCount++;
            }
            if(totalValues>0){
                if(emailCount>totalValues*0.5){result["patterns"][patternIdx]["column"]=colName;result["patterns"][patternIdx]["pattern"]="email";result["patterns"][patternIdx]["count"]=emailCount;patternIdx++;}
                if(phoneCount>totalValues*0.5){result["patterns"][patternIdx]["column"]=colName;result["patterns"][patternIdx]["pattern"]="phone";result["patterns"][patternIdx]["count"]=phoneCount;patternIdx++;}
                if(postalCount>totalValues*0.5){result["patterns"][patternIdx]["column"]=colName;result["patterns"][patternIdx]["pattern"]="postal_code";result["patterns"][patternIdx]["count"]=postalCount;patternIdx++;}
                if(urlCount>totalValues*0.5){result["patterns"][patternIdx]["column"]=colName;result["patterns"][patternIdx]["pattern"]="url";result["patterns"][patternIdx]["count"]=urlCount;patternIdx++;}
            }
        }
        result["message"]="patterns detected";
        result["mode"]="api";
        return crow::response(result);
    });

    CROW_ROUTE(app,"/api/measure-performance").methods("POST"_method)
    ([&cleaner](const crow::request& req){
        std::string csvData=req.body;
        auto startTime=std::chrono::high_resolution_clock::now();
        auto parsed=cleaner.parseCSV(csvData);
        auto missingMatrix=cleaner.detectMissingValues(parsed);
        auto dupVector=cleaner.detectDuplicates(parsed);
        auto cleaned=cleaner.cleanData(parsed);
        auto endTime=std::chrono::high_resolution_clock::now();
        auto duration=std::chrono::duration_cast<std::chrono::microseconds>(endTime-startTime);
        double executionTimeMs=duration.count()/1000.0;
        int rows=(int)parsed.size();
        int cols=parsed.empty()?0:(int)parsed[0].size();
        int totalCells=rows*cols;
        double cellsPerSecond=executionTimeMs>0?(totalCells*1000.0/executionTimeMs):0;
        int missingCount=0;for(const auto& row:missingMatrix){for(bool b:row){if(b)missingCount++;}}
        int dupCount=0;for(bool b:dupVector){if(b)dupCount++;}
        crow::json::wvalue result;
        result["metrics"]=crow::json::wvalue::object();
        result["metrics"]["rows"]=rows;
        result["metrics"]["columns"]=cols;
        result["metrics"]["totalCells"]=totalCells;
        result["metrics"]["executionTimeMs"]=executionTimeMs;
        result["metrics"]["cellsPerSecond"]=(int)cellsPerSecond;
        result["metrics"]["missingValues"]=missingCount;
        result["metrics"]["duplicateRows"]=dupCount;
        result["metrics"]["cleanedRows"]=(int)cleaned.size();
        result["message"]="performance measured";
        result["mode"]="api";
        return crow::response(result);
    });

    CROW_ROUTE(app,"/api/auto-detect-all").methods("POST"_method)
    ([&cleaner](const crow::request& req){
        std::string csvData=req.body;
        auto parsed=cleaner.parseCSV(csvData);
        int totalRows=(int)parsed.size();
        int totalColumns=parsed.empty()?0:(int)parsed[0].size();
        auto missingMatrix=cleaner.detectMissingValues(parsed);
        int missingValues=0;
        for(const auto& row:missingMatrix){for(bool b:row){if(b)missingValues++;}}
        auto dupVector=cleaner.detectDuplicates(parsed);
        int duplicateRows=0;
        for(bool b:dupVector){if(b)duplicateRows++;}
        int whitespaceIssues=0;
        int nullValueIssues=0;
        int outliers=0;
        int inconsistentValues=0;
        auto isWhitespace=[](const std::string& str)->bool{if(str.empty())return false;return str[0]==' '||str[0]=='\t'||str.back()==' '||str.back()=='\t';};
        auto isNullValue=[](const std::string& str)->bool{std::string lower=str;std::transform(lower.begin(),lower.end(),lower.begin(),::tolower);return lower=="null"||lower=="na"||lower=="n/a"||lower=="none"||lower=="-"||lower=="--"||lower=="nil"||lower=="nan";};
        for(size_t row=1;row<parsed.size();row++){
            for(size_t col=0;col<parsed[row].size();col++){
                if(isWhitespace(parsed[row][col]))whitespaceIssues++;
                if(isNullValue(parsed[row][col]))nullValueIssues++;
            }
        }
        auto isNumericStr=[](const std::string& str)->bool{if(str.empty())return false;size_t start=0;if(str[0]=='-'||str[0]=='+')start=1;if(start>=str.length())return false;bool hasDecimal=false;for(size_t i=start;i<str.length();i++){if(str[i]=='.'){if(hasDecimal)return false;hasDecimal=true;}else if(str[i]<'0'||str[i]>'9')return false;}return true;};
        for(size_t col=0;col<(size_t)totalColumns;col++){
            std::vector<double> values;
            for(size_t row=1;row<parsed.size();row++){if(col<parsed[row].size()&&isNumericStr(parsed[row][col])){values.push_back(std::stod(parsed[row][col]));}}
            if(values.size()<4)continue;
            std::vector<double> sorted=values;std::sort(sorted.begin(),sorted.end());
            double q1=sorted[sorted.size()/4];double q3=sorted[3*sorted.size()/4];double iqr=q3-q1;
            double lower=q1-1.5*iqr;double upper=q3+1.5*iqr;
            for(double v:values){if(v<lower||v>upper)outliers++;}
        }
        crow::json::wvalue result;
        result["summary"]=crow::json::wvalue::object();
        result["summary"]["totalRows"]=totalRows;
        result["summary"]["totalColumns"]=totalColumns;
        result["summary"]["missingValues"]=missingValues;
        result["summary"]["duplicateRows"]=duplicateRows;
        result["summary"]["whitespaceIssues"]=whitespaceIssues;
        result["summary"]["nullValueIssues"]=nullValueIssues;
        result["summary"]["outliers"]=outliers;
        result["summary"]["inconsistentValues"]=inconsistentValues;
        result["message"]="auto detection completed";
        result["mode"]="api";
        return crow::response(result);
    });

    CROW_ROUTE(app,"/api/auto-clean-all").methods("POST"_method)
    ([&cleaner, &auditLog](const crow::request& req){
        auto body=crow::json::load(req.body);
        if(!body){crow::json::wvalue result; result["message"]="invalid request body";return crow::response(400);}
        std::string csvData=body["csvData"].s();
        auto parsed=cleaner.parseCSV(csvData);
        int originalRows=(int)parsed.size();
        int operationsPerformed=0;
        auto trimStr=[](std::string& str){size_t start=str.find_first_not_of(" \t\r\n");size_t end=str.find_last_not_of(" \t\r\n");if(start==std::string::npos){str="";}else{str=str.substr(start,end-start+1);}};
        auto isNullValue=[](const std::string& str)->bool{std::string lower=str;std::transform(lower.begin(),lower.end(),lower.begin(),::tolower);return lower=="null"||lower=="na"||lower=="n/a"||lower=="none"||lower=="-"||lower=="--"||lower=="nil"||lower=="nan";};
        int trimmed=0,nullsFixed=0;
        for(size_t row=0;row<parsed.size();row++){
            for(size_t col=0;col<parsed[row].size();col++){
                std::string original=parsed[row][col];
                trimStr(parsed[row][col]);
                if(original!=parsed[row][col])trimmed++;
                if(row>0&&isNullValue(parsed[row][col])){parsed[row][col]="";nullsFixed++;}
            }
        }
        if(trimmed>0)operationsPerformed++;
        if(nullsFixed>0)operationsPerformed++;
        auto cleaned=cleaner.cleanData(parsed);
        int duplicatesRemoved=originalRows-(int)cleaned.size();
        if(duplicatesRemoved>0)operationsPerformed++;
        std::stringstream cleanedCSV;
        for(const auto& row:cleaned){
            for(size_t i=0;i<row.size();i++){
                if(i>0)cleanedCSV<<",";
                cleanedCSV<<row[i];
            }
            cleanedCSV<<"\n";
        }
        auditLog.addEntry("Auto Clean All", duplicatesRemoved+trimmed+nullsFixed, originalRows, (int)cleaned.size());
        crow::json::wvalue result;
        result["cleaned"]=cleanedCSV.str();
        result["originalRows"]=originalRows;
        result["cleanedRows"]=(int)cleaned.size();
        result["duplicatesRemoved"]=duplicatesRemoved;
        result["cellsTrimmed"]=trimmed;
        result["nullsStandardised"]=nullsFixed;
        result["operationsPerformed"]=operationsPerformed;
        result["message"]="auto cleaning completed";
        result["mode"]="api";
        return crow::response(result);
    });

    CROW_ROUTE(app,"/api/standardise-date-column").methods("POST"_method)
    ([&cleaner, &auditLog](const crow::request& req){
        auto body=crow::json::load(req.body);
        if(!body){crow::json::wvalue result; result["message"]="invalid request body";return crow::response(400);}
        std::string csvData=body["csvData"].s();
        int colIndex=0;
        if(body.has("colIndex"))colIndex=(int)body["colIndex"].i();
        auto parsed=cleaner.parseCSV(csvData);
        int datesStandardised=0;
        auto parseDate=[](const std::string& str,int& day,int& month,int& year)->bool{
            std::vector<int> nums;std::string current;
            for(char c:str){if(c>='0'&&c<='9'){current+=c;}else if(!current.empty()){nums.push_back(std::stoi(current));current="";}}
            if(!current.empty())nums.push_back(std::stoi(current));
            if(nums.size()!=3)return false;
            if(nums[2]>31){year=nums[2];if(nums[0]>12){day=nums[0];month=nums[1];}else if(nums[1]>12){day=nums[1];month=nums[0];}else{month=nums[0];day=nums[1];}}
            else if(nums[0]>31){year=nums[0];month=nums[1];day=nums[2];}
            else{day=nums[0];month=nums[1];year=nums[2];}
            if(year<100)year+=2000;
            return day>=1&&day<=31&&month>=1&&month<=12&&year>=1900&&year<=2100;
        };
        for(size_t row=1;row<parsed.size();row++){
            if((size_t)colIndex<parsed[row].size()&&!parsed[row][colIndex].empty()){
                int day,month,year;
                if(parseDate(parsed[row][colIndex],day,month,year)){
                    char buf[11];
                    snprintf(buf,sizeof(buf),"%04d-%02d-%02d",year,month,day);
                    parsed[row][colIndex]=buf;
                    datesStandardised++;
                }
            }
        }
        std::stringstream cleanedCSV;
        for(const auto& row:parsed){for(size_t i=0;i<row.size();i++){if(i>0)cleanedCSV<<",";cleanedCSV<<row[i];}cleanedCSV<<"\n";}
        auditLog.addEntry("Standardise Dates", datesStandardised, (int)parsed.size(), (int)parsed.size());
        crow::json::wvalue result;
        result["cleaned"]=cleanedCSV.str();
        result["datesStandardised"]=datesStandardised;
        result["column"]=colIndex;
        result["message"]="dates standardised to ISO 8601 format";
        result["mode"]="api";
        return crow::response(result);
    });

    CROW_ROUTE(app,"/api/standardise-number-column").methods("POST"_method)
    ([&cleaner, &auditLog](const crow::request& req){
        auto body=crow::json::load(req.body);
        if(!body){crow::json::wvalue result; result["message"]="invalid request body";return crow::response(400);}
        std::string csvData=body["csvData"].s();
        int colIndex=0;
        if(body.has("colIndex"))colIndex=(int)body["colIndex"].i();
        auto parsed=cleaner.parseCSV(csvData);
        int numbersStandardised=0;
        auto parseNumber=[](const std::string& str,double& result)->bool{
            std::string cleaned;
            for(char c:str){if((c>='0'&&c<='9')||c=='.'||c=='-'||c=='+')cleaned+=c;}
            if(cleaned.empty())return false;
            try{result=std::stod(cleaned);return true;}catch(...){return false;}
        };
        for(size_t row=1;row<parsed.size();row++){
            if((size_t)colIndex<parsed[row].size()&&!parsed[row][colIndex].empty()){
                double num;
                if(parseNumber(parsed[row][colIndex],num)){
                    std::stringstream ss;
                    ss<<std::fixed<<std::setprecision(2)<<num;
                    std::string formatted=ss.str();
                    size_t dotPos=formatted.find('.');
                    if(dotPos!=std::string::npos){while(formatted.back()=='0')formatted.pop_back();if(formatted.back()=='.')formatted.pop_back();}
                    if(formatted!=parsed[row][colIndex]){parsed[row][colIndex]=formatted;numbersStandardised++;}
                }
            }
        }
        std::stringstream cleanedCSV;
        for(const auto& row:parsed){for(size_t i=0;i<row.size();i++){if(i>0)cleanedCSV<<",";cleanedCSV<<row[i];}cleanedCSV<<"\n";}
        auditLog.addEntry("Standardise Numbers", numbersStandardised, (int)parsed.size(), (int)parsed.size());
        crow::json::wvalue result;
        result["cleaned"]=cleanedCSV.str();
        result["numbersStandardised"]=numbersStandardised;
        result["column"]=colIndex;
        result["message"]="numbers standardised to consistent format";
        result["mode"]="api";
        return crow::response(result);
    });

    CROW_ROUTE(app,"/api/profile-column").methods("POST"_method)
    ([&cleaner](const crow::request& req){
        auto body=crow::json::load(req.body);
        if(!body){crow::json::wvalue result; result["message"]="invalid request body";return crow::response(400);}
        std::string csvData=body["csvData"].s();
        int colIndex=0;
        if(body.has("colIndex"))colIndex=(int)body["colIndex"].i();
        auto parsed=cleaner.parseCSV(csvData);
        if(parsed.empty()||colIndex<0||(size_t)colIndex>=parsed[0].size()){
            crow::json::wvalue result;
            result["message"]="invalid column index";
            result["values"]=crow::json::wvalue::object();
            return crow::response(result);
        }
        auto profile=cleaner.profileColumn(parsed, (size_t)colIndex);
        crow::json::wvalue result;
        result["values"]=crow::json::wvalue::object();
        for(const auto& pair:profile){
            result["values"][pair.first]=(int)pair.second;
        }
        result["columnIndex"]=colIndex;
        result["uniqueValues"]=(int)profile.size();
        result["message"]="column profiled successfully";
        result["mode"]="api";
        return crow::response(result);
    });

	CROW_ROUTE(app,"/api/standardize-column-case").methods("POST"_method)
	([&cleaner](const crow::request& req){
		auto body=crow::json::load(req.body);
		if(!body){crow::json::wvalue result; result["message"]="invalid request body";return crow::response(400);}
		std::string csvData=body["csvData"].s();
		int colIndex=0;
		std::string caseType="upper";
		if(body.has("colIndex"))colIndex=(int)body["colIndex"].i();
		if(body.has("caseType"))caseType=body["caseType"].s();
		auto parsed=cleaner.parseCSV(csvData);
		if(parsed.empty()||colIndex<0||(size_t)colIndex>=parsed[0].size()){
			crow::json::wvalue result;
			result["message"]="invalid column index";
			result["mode"]="api";
			return crow::response(result);
		}
		auto standardized=cleaner.standardizeColumnCase(parsed, (size_t)colIndex, caseType);
		std::string output;
		for(size_t i=0;i<standardized.size();++i){
			for(size_t j=0;j<standardized[i].size();++j){
				if(j>0)output+=",";
				output+=standardized[i][j];
			}
			output+="\n";
		}
		crow::json::wvalue result;
		result["csvData"]=output;
		result["columnIndex"]=colIndex;
		result["caseType"]=caseType;
		result["message"]="column case standardized successfully";
		result["mode"]="api";
		return crow::response(result);
	});

	CROW_ROUTE(app,"/api/extract-first-token").methods("POST"_method)
	([&cleaner, &auditLog](const crow::request& req){
		auto body=crow::json::load(req.body);
		if(!body){crow::json::wvalue result; result["message"]="invalid request body";return crow::response(400);}
		std::string csvData=body["csvData"].s();
		auto parsed=cleaner.parseCSV(csvData);
		int originalRows=(int)parsed.size();
		std::stringstream output;
		for(size_t i=0;i<parsed.size();++i){
			for(size_t j=0;j<parsed[i].size();++j){
				std::string cell=parsed[i][j];
				size_t spacePos=cell.find(' ');
				if(spacePos!=std::string::npos){
					cell=cell.substr(0,spacePos);
				}
				if(j>0)output<<",";
				output<<cell;
			}
			output<<"\n";
		}
		auditLog.addEntry("Extract First Token", 0, originalRows, originalRows);
		crow::json::wvalue resp;
		resp["csvData"]=output.str();
		resp["originalRows"]=originalRows;
		resp["cleanedRows"]=originalRows;
		resp["message"]="first token extracted successfully";
		resp["mode"]="api";
		return crow::response(resp);
	});

	CROW_ROUTE(app,"/api/extract-n-tokens").methods("POST"_method)
	([&cleaner, &auditLog](const crow::request& req){
		auto body=crow::json::load(req.body);
		if(!body){crow::json::wvalue result; result["message"]="invalid request body";return crow::response(400);}
		std::string csvData=body["csvData"].s();
		int numTokens=body["numTokens"].i();
		if(numTokens<1)numTokens=1;
		auto parsed=cleaner.parseCSV(csvData);
		std::stringstream output;
		for(size_t i=0;i<parsed.size();++i){
			for(size_t j=0;j<parsed[i].size();++j){
				std::string cell=parsed[i][j];
				std::stringstream tokens;
				int count=0;
				std::string token;
				for(size_t k=0;k<cell.length();++k){
					if(cell[k]==' '){
						if(!token.empty()){
							if(count>0)tokens<<" ";
							tokens<<token;
							count++;
							if(count>=numTokens)break;
							token="";
						}
					}else{
						token+=cell[k];
					}
				}
				if(!token.empty()&&count<numTokens){
					if(count>0)tokens<<" ";
					tokens<<token;
				}
				if(j>0)output<<",";
				output<<tokens.str();
			}
			output<<"\n";
		}
		auditLog.addEntry("Extract N Tokens", 0, (int)parsed.size(), numTokens);
		crow::json::wvalue resp;
		resp["csvData"]=output.str();
		resp["originalRows"]=(int)parsed.size();
		resp["numTokens"]=numTokens;
		resp["message"]="n tokens extracted successfully";
		resp["mode"]="api";
		return crow::response(resp);
	});

	CROW_ROUTE(app,"/api/fuzzy-match").methods("POST"_method)
	([&cleaner, &auditLog](const crow::request& req){
		auto body=crow::json::load(req.body);
		if(!body){crow::json::wvalue result; result["message"]="invalid request body";return crow::response(400);}
		std::string csvData=body["csvData"].s();
		double threshold=body["threshold"].d();
		if(threshold<0.0)threshold=0.85;
		auto parsed=cleaner.parseCSV(csvData);
		std::map<std::string,std::vector<std::string>> groups;
		std::vector<std::string> uniqueValues;

		for(size_t i=1;i<parsed.size();++i){
			if(!parsed[i].empty()){
				std::string cellValue=parsed[i][0];
				bool foundGroup=false;

				for(const auto& uniqueVal:uniqueValues){
					double similarity=calculateSimilarity(cellValue,uniqueVal);
					if(similarity>=threshold){
						groups[uniqueVal].push_back(cellValue);
						foundGroup=true;
						break;
					}
				}

				if(!foundGroup){
					uniqueValues.push_back(cellValue);
					groups[cellValue].push_back(cellValue);
				}
			}
		}

		std::stringstream output;
		output<<"original,merged\n";
		for(const auto& pair:groups){
			for(const auto& val:pair.second){
				output<<val<<","<<pair.first<<"\n";
			}
		}

		auditLog.addEntry("Fuzzy Match", 0, (int)parsed.size(), (int)groups.size());
		crow::json::wvalue resp;
		resp["csvData"]=output.str();
		resp["originalGroups"]=(int)parsed.size()-1;
		resp["mergedGroups"]=(int)groups.size();
		resp["message"]="fuzzy matching completed";
		resp["mode"]="api";
		return crow::response(resp);
	});

	// Helper function to calculate Levenshtein distance
	auto levenshteinDistance=[](const std::string& s1, const std::string& s2)->int{
		size_t len1=s1.length(), len2=s2.length();
		std::vector<std::vector<int>> dp(len1+1, std::vector<int>(len2+1, 0));
		for(size_t i=0;i<=len1;++i) dp[i][0]=i;
		for(size_t j=0;j<=len2;++j) dp[0][j]=j;
		for(size_t i=1;i<=len1;++i){
			for(size_t j=1;j<=len2;++j){
				if(s1[i-1]==s2[j-1]){
					dp[i][j]=dp[i-1][j-1];
				}else{
					dp[i][j]=1+std::min({dp[i-1][j], dp[i][j-1], dp[i-1][j-1]});
				}
			}
		}
		return dp[len1][len2];
	};

	CROW_ROUTE(app,"/api/quick-clean").methods("POST"_method)
	([&cleaner, &auditLog, &levenshteinDistance](const crow::request& req){
		auto body=crow::json::load(req.body);
		if(!body){crow::json::wvalue result; result["message"]="invalid request body";return crow::response(400);}
		std::string csvData=body["csvData"].s();
		std::string caseType=body["caseType"].s();
		auto parsed=cleaner.parseCSV(csvData);
		int originalRows=(int)parsed.size();

		// Find the first text column (skip numeric columns)
		int textColumnIndex=0;
		if(!parsed.empty() && !parsed[0].empty()){
			for(size_t j=0;j<parsed[0].size();++j){
				bool isNumeric=true;
				// Check if this column contains mostly numbers
				int numericCount=0;
				for(size_t i=1;i<std::min((size_t)100, parsed.size());++i){
					if(j<parsed[i].size()){
						std::string cell=parsed[i][j];
						// Trim whitespace
						size_t start=cell.find_first_not_of(" \t\r\n");
						size_t end=cell.find_last_not_of(" \t\r\n");
						if(start!=std::string::npos){cell=cell.substr(start,end-start+1);}
						// Check if it's a number
						if(!cell.empty()){
							char* endptr;
							strtod(cell.c_str(), &endptr);
							if(*endptr=='\0'){
								numericCount++;
							}
						}
					}
				}
				// If less than 80% numeric, treat as text column
				if(numericCount<80){
					textColumnIndex=j;
					break;
				}
			}
		}

		// Debug: log which column was detected
		std::ofstream debugLogColumn("debug.log", std::ios::app);
		debugLogColumn<<"DEBUG: Detected text column index = "<<textColumnIndex<<std::endl;
		if(!parsed.empty() && !parsed[0].empty() && textColumnIndex<(int)parsed[0].size()){
			debugLogColumn<<"DEBUG: Column header = "<<parsed[0][textColumnIndex]<<std::endl;
		}
		debugLogColumn.close();

		// PASS 1: Collect all unique locations and their counts (with normalization)
		std::map<std::string, int> locationCounts;
		for(size_t i=1;i<parsed.size();++i){
			if(!parsed[i].empty() && textColumnIndex<(int)parsed[i].size()){
				std::string location=parsed[i][textColumnIndex];
				// Trim whitespace
				size_t start=location.find_first_not_of(" \t\r\n");
				size_t end=location.find_last_not_of(" \t\r\n");
				if(start!=std::string::npos){location=location.substr(start,end-start+1);}
				// Standardize nulls
				if(location.empty()||location=="N/A"||location=="n/a"||location=="NA"||location=="null"||location=="NULL"||location=="None"||location=="NONE"||location=="-"||location=="?"){
					location="";
				}
				if(!location.empty()){
					// Convert to uppercase
					std::transform(location.begin(),location.end(),location.begin(),::toupper);
					// Normalize punctuation
					std::string normalized;
					for(size_t k=0;k<location.length();++k){
						char c=location[k];
						if(c==','||c==';'||c=='-'||c=='/'||c=='|'){
							normalized+=' ';
						}else if(c==' '||c=='\t'){
							if(normalized.empty()||normalized.back()!=' '){
								normalized+=' ';
							}
						}else{
							normalized+=c;
						}
					}
					while(!normalized.empty()&&normalized.back()==' '){normalized.pop_back();}
					location=normalized;
					locationCounts[location]++;
				}
			}
		}
		std::ofstream debugLog("debug.log", std::ios::app);
		debugLog<<"DEBUG: locationCounts size = "<<locationCounts.size()<<std::endl;
		debugLog.close();

		// PASS 2: Build mapping using substring matching
		// Strategy: if one location is a substring of another, merge them
		std::map<std::string, std::string> locationMapping;
		std::vector<std::string> locations;
		for(auto& entry : locationCounts){
			locations.push_back(entry.first);
		}

		// For each location, find similar locations and merge them
		std::set<std::string> processed;
		for(auto& loc1 : locations){
			if(processed.count(loc1)) continue;

			std::string canonical=loc1;
			int maxCount=locationCounts[loc1];
			std::vector<std::string> similar;
			similar.push_back(loc1);

			// Find all similar locations
			for(auto& loc2 : locations){
				if(loc1==loc2 || processed.count(loc2)) continue;

				bool isSimilar=false;

				// Check if one is a substring of the other
				if(loc1.find(loc2)!=std::string::npos || loc2.find(loc1)!=std::string::npos){
					isSimilar=true;
				}

				if(isSimilar){
					similar.push_back(loc2);
					// Use the more common one as canonical
					if(locationCounts[loc2]>maxCount){
						canonical=loc2;
						maxCount=locationCounts[loc2];
					}
					processed.insert(loc2);
				}
			}

			// Map all similar locations to the canonical one
			for(auto& loc : similar){
				locationMapping[loc]=canonical;
			}

			processed.insert(loc1);
		}

		// PASS 3: Fuzzy matching for spelling mistakes (85%+ similarity)
		// This catches typos like "BROOKYLN" -> "BROOKLYN"
		std::set<std::string> fuzzyProcessed;
		std::ofstream debugLog3("debug.log", std::ios::app);
		debugLog3<<"DEBUG: Starting PASS 3 (Fuzzy matching)..."<<std::endl;
		for(auto& loc1 : locations){
			if(fuzzyProcessed.count(loc1)) continue;

			// Skip if already merged in Pass 1
			if(locationMapping[loc1]!=loc1) continue;

			std::string canonical=loc1;
			int maxCount=locationCounts[loc1];
			std::vector<std::string> fuzzyMatches;
			fuzzyMatches.push_back(loc1);

			// Find all fuzzy matches (85%+ similarity)
			for(auto& loc2 : locations){
				if(loc1==loc2 || fuzzyProcessed.count(loc2)) continue;

				// For fuzzy matching, we can match against locations that were already merged
				// because we want to find the canonical location they map to
				std::string loc2Canonical=locationMapping[loc2];

				int distance=levenshteinDistance(loc1, loc2);
				int maxLen=std::max(loc1.length(), loc2.length());
				int similarity=(maxLen>0) ? (100*(maxLen-distance)/maxLen) : 0;

				// If similarity > 75%, consider them the same (typo/spelling mistake)
				// 75% catches most typos: BROOKLYN vs BROOKYLN (75%), NEWYORK vs NEW YORK (87%)
				if(similarity>75){
					debugLog3<<"DEBUG PASS3: Fuzzy match found: "<<loc1<<" (count="<<locationCounts[loc1]<<") <-> "<<loc2<<" (count="<<locationCounts[loc2]<<"), similarity="<<similarity<<"%"<<std::endl;
					fuzzyMatches.push_back(loc2);
					// Use the more common one as canonical (compare against the canonical form)
					if(locationCounts[loc2]>maxCount){
						canonical=loc2Canonical;
						maxCount=locationCounts[loc2];
					}
					fuzzyProcessed.insert(loc2);
				}
			}

			// Map all fuzzy matches to the canonical one
			for(auto& loc : fuzzyMatches){
				locationMapping[loc]=canonical;
			}

			fuzzyProcessed.insert(loc1);
		}
		debugLog3<<"DEBUG: Finished PASS 3 (Fuzzy matching)"<<std::endl;
		debugLog3.close();

		// Debug: log mapping size
		std::ofstream debugLog2("debug.log", std::ios::app);
		debugLog2<<"DEBUG: locationMapping size = "<<locationMapping.size()<<std::endl;
		if(locationMapping.size()>0){
			debugLog2<<"DEBUG: Sample mappings:"<<std::endl;
			int count=0;
			for(auto& m : locationMapping){
				if(count<10){
					debugLog2<<"  "<<m.first<<" -> "<<m.second<<std::endl;
					count++;
				}
			}
		}
		debugLog2.close();

		std::set<std::vector<std::string>> seen;
		std::vector<std::vector<std::string>> result;

		for(size_t i=0;i<parsed.size();++i){
			auto row=parsed[i];
			bool skipRow=false;

			for(size_t j=0;j<row.size();++j){
				std::string& cell=row[j];

				size_t start=cell.find_first_not_of(" \t\r\n");
				size_t end=cell.find_last_not_of(" \t\r\n");
				if(start==std::string::npos){cell="";}else{cell=cell.substr(start,end-start+1);}

				if(cell.empty()||cell=="N/A"||cell=="n/a"||cell=="NA"||cell=="null"||cell=="NULL"||cell=="None"||cell=="NONE"||cell=="-"||cell=="?"){
					cell="";
				}

				// Skip row only if the text column is empty
				if(j==textColumnIndex && cell.empty()){
					skipRow=true;
					break;
				}

				// First convert to uppercase for normalization and mapping lookup
				std::transform(cell.begin(),cell.end(),cell.begin(),::toupper);

				std::string normalized;
				for(size_t k=0;k<cell.length();++k){
					char c=cell[k];
					if(c==','||c==';'||c=='-'||c=='/'||c=='|'){
						normalized+=' ';
					}else if(c==' '||c=='\t'){
						if(normalized.empty()||normalized.back()!=' '){
							normalized+=' ';
						}
					}else{
						normalized+=c;
					}
				}
				while(!normalized.empty()&&normalized.back()==' '){normalized.pop_back();}
				cell=normalized;

				// Apply location mapping (fuzzy matching result)
				if(j==textColumnIndex && !cell.empty()){
					std::ofstream debugLog3("debug.log", std::ios::app);
					if(locationMapping.count(cell)){
						debugLog3<<"DEBUG: Mapping "<<cell<<" -> "<<locationMapping[cell]<<std::endl;
						cell=locationMapping[cell];
					}else{
						debugLog3<<"DEBUG: No mapping found for '"<<cell<<"'"<<std::endl;
					}
					debugLog3.close();
				}

				// Then apply the requested case conversion
				if(caseType=="lower" || caseType=="lowercase"){
					std::transform(cell.begin(),cell.end(),cell.begin(),::tolower);
				}
			}

			if(!skipRow){
				if(i==0){
					result.push_back(row);
					seen.insert(row);
				}else{
					if(!seen.count(row)){
						seen.insert(row);
						result.push_back(row);
					}
				}
			}
		}

		std::stringstream output;
		for(size_t i=0;i<result.size();++i){
			for(size_t j=0;j<result[i].size();++j){
				if(j>0)output<<",";
				output<<result[i][j];
			}
			output<<"\n";
		}

		int removedRows=originalRows-result.size();
		auditLog.addEntry("Quick Clean All", removedRows, originalRows, result.size());
		crow::json::wvalue resp;
		resp["csvData"]=output.str();
		resp["originalRows"]=originalRows;
		resp["cleanedRows"]=result.size();
		resp["removedRows"]=removedRows;
		resp["message"]="data cleaned successfully";
		resp["mode"]="api";
		return crow::response(resp);
	});

	// ========================================================================
	// UNIVERSAL TEXT CLEANING ENDPOINT
	// Applies comprehensive text cleaning to ALL text columns automatically
	// ========================================================================
	CROW_ROUTE(app,"/api/universal-clean").methods("POST"_method)
	([&cleaner, &auditLog](const crow::request& req){
		try{
			auto body=crow::json::load(req.body);
			if(!body){
				crow::json::wvalue result;
				result["message"]="invalid request body";
				return crow::response(400);
			}

			std::string csvData=body["csvData"].s();
			double fuzzyThreshold=0.75; // default threshold
			if(body.has("fuzzyThreshold")){
				fuzzyThreshold=body["fuzzyThreshold"].d();
				if(fuzzyThreshold<0.0 || fuzzyThreshold>1.0){
					fuzzyThreshold=0.75;
				}
			}

			bool removeDuplicates=true;
			if(body.has("removeDuplicates")){
				removeDuplicates=body["removeDuplicates"].b();
			}

			auto parsed=cleaner.parseCSV(csvData);
			int originalRows=parsed.size();

			// Apply universal text cleaning
			std::cerr << "DEBUG: Starting universal text cleaning with " << parsed.size() << " rows" << std::endl;
			auto cleaningResult=universalTextCleaning(parsed, fuzzyThreshold, removeDuplicates);
			std::cerr << "DEBUG: Universal text cleaning completed" << std::endl;

			int cleanedRows=cleaningResult.cleanedData.size();
			int removedRows=originalRows - cleanedRows;

			auditLog.addEntry("Universal Clean", removedRows, originalRows, cleanedRows);

			// Convert cleaned data to CSV string
			std::stringstream csvOutput;
			for(size_t i=0; i<cleaningResult.cleanedData.size(); ++i){
				for(size_t j=0; j<cleaningResult.cleanedData[i].size(); ++j){
					if(j>0) csvOutput<<",";
					csvOutput<<cleaningResult.cleanedData[i][j];
				}
				csvOutput<<"\n";
			}

			crow::json::wvalue resp;
			resp["csvData"]=csvOutput.str();
			resp["originalRows"]=originalRows;
			resp["cleanedRows"]=cleanedRows;
			resp["removedRows"]=removedRows;
			resp["duplicateRowsRemoved"]=cleaningResult.duplicateRowsRemoved;
			resp["message"]="universal text cleaning completed successfully";
			resp["mode"]="api";

			return crow::response(resp);
		}catch(const std::exception& e){
			crow::json::wvalue resp;
			resp["message"]=std::string("error: ") + e.what();
			std::cerr << "ERROR: " << e.what() << std::endl;
			return crow::response(500, resp);
		}catch(...){
			crow::json::wvalue resp;
			resp["message"]="unknown error during universal text cleaning";
			std::cerr << "ERROR: unknown exception" << std::endl;
			return crow::response(500, resp);
		}
	});

	std::cerr << "startup: port=" << port << " web_concurrency=" << webConcurrency << std::endl;
	std::cerr << "startup: " << (g_frontendDiag.empty() ? "(no frontend diag)" : g_frontendDiag) << std::endl;
	try{
		app.port(port).concurrency((unsigned int)webConcurrency).run();
	}catch(const std::exception& e){
		std::cerr << "fatal: server failed to start: " << e.what() << std::endl;
		return 1;
	}catch(...){
		std::cerr << "fatal: server failed to start: unknown error" << std::endl;
		return 1;
	}
}
