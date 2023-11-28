#include "test_runner.h"
#include <string>
#include <vector>
#include <set>

using namespace std;

class Translator {
public:
    void Add(string_view source, string_view target) {
        auto source_it = words.emplace(source).first;
        auto target_it = words.emplace(target).first;

        forward[string_view(*source_it)] = string_view(*target_it);
        backward[string_view(*target_it)] = string_view(*source_it);
    }

    [[nodiscard]] string_view TranslateForward(string_view source) const {
        if (forward.count(source) == 0) {
            return "";
        }
        return forward.at(source);
    }

    [[nodiscard]] string_view TranslateBackward(string_view target) const {
        if (backward.count(target) == 0) {
            return "";
        }
        return backward.at(target);
    }

private:
    set<string> words;
    map<string_view, string_view> forward, backward;
};

void TestSimple() {
    Translator translator;
    translator.Add(string("okno"), string("window"));
    translator.Add(string("stol"), string("table"));

    ASSERT_EQUAL(translator.TranslateForward("okno"), "window");
    ASSERT_EQUAL(translator.TranslateBackward("table"), "stol");
    ASSERT_EQUAL(translator.TranslateBackward("stol"), "");
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestSimple);
    return 0;
}
