#include "test_runner.h"
#include <vector>
#include <utility>

using namespace std;

template<typename Token>
using Sentence = vector<Token>;

template<typename Token>
vector<Sentence<Token>> SplitIntoSentences(vector<Token> tokens) {
    if (tokens.empty()) {
        return {};
    }

    Sentence<Token> sentence;
    vector<Sentence<Token>> result;
    for (int i = 0; i < tokens.size(); i++) {
        auto back_size = sentence.size();
        if (back_size > 0) {
            if (sentence[back_size - 1].IsEndSentencePunctuation() &&
                !tokens[i].IsEndSentencePunctuation()) {
                result.push_back(move(sentence));
                sentence.clear();
            }
        }

        sentence.push_back(move(tokens[i]));
    }

    if (!sentence.empty()) {
        result.push_back(move(sentence));
    }

    return move(result);
}

struct TestToken {
    string data;
    bool is_end_sentence_punctuation = false;

    bool IsEndSentencePunctuation() const {
        return is_end_sentence_punctuation;
    }

    bool operator==(const TestToken &other) const {
        return data == other.data && is_end_sentence_punctuation == other.is_end_sentence_punctuation;
    }
};

ostream &operator<<(ostream &stream, const TestToken &token) {
    return stream << token.data;
}

void TestSplitting() {
    ASSERT_EQUAL(
            SplitIntoSentences(vector<TestToken>({{"Split"},
                                                  {"into"},
                                                  {"sentences"},
                                                  {"!"}})),
            vector<Sentence<TestToken>>({
                                                {{"Split"}, {"into"}, {"sentences"}, {"!"}}
                                        })
    );

    ASSERT_EQUAL(
            SplitIntoSentences(vector<TestToken>({{"Split"},
                                                  {"into"},
                                                  {"sentences"},
                                                  {"!", true}})),
            vector<Sentence<TestToken>>({
                                                {{"Split"}, {"into"}, {"sentences"}, {"!", true}}
                                        })
    );

    ASSERT_EQUAL(
            SplitIntoSentences(vector<TestToken>({{"Split"},
                                                  {"into"},
                                                  {"sentences"},
                                                  {"!", true},
                                                  {"!", true},
                                                  {"Without"},
                                                  {"copies"},
                                                  {".", true}})),
            vector<Sentence<TestToken>>({
                                                {{"Split"},   {"into"},   {"sentences"}, {"!", true}, {"!", true}},
                                                {{"Without"}, {"copies"}, {".", true}},
                                        })
    );
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestSplitting);
    return 0;
}
