#include "text_errors.h"
#include "string_similarity.cpp"
#include <vector>
#include <string>

struct fuzzy_match {
    std::string value1;
    std::string value2;
    float similarity_score;
    int levenshtein_distance;
    bool is_duplicate;
};

class fuzzy_matcher {
private:
    levenshtein_calculator calc;
    std::vector<fuzzy_match> detected_duplicates;

    bool exceeds_threshold(float similarity) {
        return similarity >= text_errors::SIMILARITY_THRESHOLD;
    }

public:
    void find_duplicates(const std::vector<std::string>& values) {
        for (size_t i = 0; i < values.size(); i++) {
            for (size_t j = i + 1; j < values.size(); j++) {
                float similarity = calc.calculate_similarity(
                    values[i], values[j]);

                if (exceeds_threshold(similarity)) {
                    fuzzy_match match;
                    match.value1 = values[i];
                    match.value2 = values[j];
                    match.similarity_score = similarity;
                    match.levenshtein_distance = calc.get_distance(
                        values[i], values[j]);
                    match.is_duplicate = true;

                    detected_duplicates.push_back(match);
                }
            }
        }
    }

    const std::vector<fuzzy_match>& get_duplicates() const {
        return detected_duplicates;
    }

    int get_duplicate_count() const {
        return detected_duplicates.size();
    }

    bool is_similar(const std::string& v1,
                   const std::string& v2) const {
        float sim = calc.calculate_similarity(v1, v2);
        return sim >= text_errors::SIMILARITY_THRESHOLD;
    }
};
