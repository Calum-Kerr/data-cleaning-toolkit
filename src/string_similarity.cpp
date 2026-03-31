#include <string>
#include <algorithm>
#include <cmath>

class levenshtein_calculator {
private:
    int compute_distance(const std::string& s1,
                        const std::string& s2) {
        size_t len1 = s1.length();
        size_t len2 = s2.length();

        std::vector<std::vector<int>> dp(len1 + 1,
                              std::vector<int>(len2 + 1, 0));

        for (size_t i = 0; i <= len1; i++) {
            dp[i][0] = i;
        }
        for (size_t j = 0; j <= len2; j++) {
            dp[0][j] = j;
        }

        for (size_t i = 1; i <= len1; i++) {
            for (size_t j = 1; j <= len2; j++) {
                int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;

                dp[i][j] = std::min({
                    dp[i - 1][j] + 1,
                    dp[i][j - 1] + 1,
                    dp[i - 1][j - 1] + cost
                });
            }
        }

        return dp[len1][len2];
    }

public:
    float calculate_similarity(const std::string& s1,
                              const std::string& s2) {
        if (s1.empty() && s2.empty()) {
            return 1.0f;
        }

        int distance = compute_distance(s1, s2);
        int max_len = std::max(s1.length(), s2.length());

        if (max_len == 0) {
            return 1.0f;
        }

        float similarity = 1.0f - (float)distance / max_len;
        return std::max(0.0f, similarity);
    }

    int get_distance(const std::string& s1,
                    const std::string& s2) {
        return compute_distance(s1, s2);
    }
};
