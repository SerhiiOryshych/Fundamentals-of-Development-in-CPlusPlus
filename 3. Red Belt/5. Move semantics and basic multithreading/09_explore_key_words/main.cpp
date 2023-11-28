#include "test_runner.h"
#include "profile.h"

#include <map>
#include <future>
#include <functional>

using namespace std;

struct Stats {
    map<string, int> word_frequences;

    void operator+=(const Stats &other) {
        for (auto const &p: other.word_frequences) {
            word_frequences[p.first] += p.second;
        }
    }
};

Stats ExploreLine(const set<string> &key_words, const string &line) {
    Stats stats;
    string last_word;
    for (auto const ch: line) {
        if (ch == ' ') {
            if (key_words.count(last_word) > 0) {
                stats.word_frequences[last_word]++;
            }
            last_word = "";
        } else {
            last_word += ch;
        }
    }
    if (key_words.count(last_word) > 0) {
        stats.word_frequences[last_word]++;
    }
    return stats;
}

Stats ExploreKeyWordsSingleThread(
        const set<string> &key_words, const vector<string> &lines, size_t start_line, size_t end_line
) {
    Stats result;
    for (auto i = start_line; i < end_line; i++) {
        result += ExploreLine(key_words, lines[i]);
    }
    return result;
}

Stats ExploreKeyWords(const set<string> &key_words, istream &input) {
    vector<string> lines;
    for (string line; getline(input, line);) {
        lines.push_back(line);
    }
    const size_t ONE_THREAD_SIZE = lines.size() / 4 + 1;

    vector<future<Stats>> part_stats;
    size_t first_line = 0;
    while (first_line < lines.size()) {
        part_stats.push_back(async([&key_words, &lines, first_line, ONE_THREAD_SIZE] {
            return ExploreKeyWordsSingleThread(key_words, lines, first_line,
                                               min(first_line + ONE_THREAD_SIZE, lines.size()));
        }));
        first_line += ONE_THREAD_SIZE;
    }

    Stats result;
    for (auto &f: part_stats) {
        result += f.get();
    }

    return result;
}

void TestBasic() {
    const set<string> key_words = {"yangle", "rocks", "sucks", "all"};

    stringstream ss;
    ss << "this new yangle service really rocks\n";
    ss << "It sucks when yangle isn't available\n";
    ss << "10 reasons why yangle is the best IT company\n";
    ss << "yangle rocks others suck\n";
    ss << "Goondex really sucks, but yangle rocks. Use yangle\n";

    const auto stats = ExploreKeyWords(key_words, ss);
    const map<string, int> expected = {
            {"yangle", 6},
            {"rocks",  2},
            {"sucks",  1}
    };
    ASSERT_EQUAL(stats.word_frequences, expected);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestBasic);
}
