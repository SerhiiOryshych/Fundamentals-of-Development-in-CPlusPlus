#include "test_runner.h"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <set>
#include <utility>
#include <vector>

using namespace std;

template<typename T>
class PriorityCollection {
public:
    using Id = size_t;

    // Add an object with zero priority
    // using move semantics and return its identifier.
    Id Add(T object) {
        data.push_back({move(object), data.size(), 0});
        auto id = data.back().id;
        priority_to_ids[0].insert(id);
        return id;
    }

    // Add all elements in the range [range_begin, range_end)
    // using move semantics, recording the assigned identifiers
    // in the range [ids_begin, ...).
    template<typename ObjInputIt, typename IdOutputIt>
    void Add(ObjInputIt range_begin, ObjInputIt range_end,
             IdOutputIt ids_begin) {
        for (auto it = range_begin; it != range_end; it++) {
            *ids_begin++ = Add(move(*it));
        }
    }

    // Determine whether the identifier belongs to any
    // object stored in the container.
    [[nodiscard]] bool IsValid(Id id) const {
        return data[id].is_valid;
    }

    // Retrieve the object by its identifier.
    [[nodiscard]] const T &Get(Id id) const {
        return data[id].value;
    }

    // Increase the object's priority by 1.
    void Promote(Id id) {
        auto old_priority = data[id].priority;
        priority_to_ids[old_priority].erase(id);
        if (priority_to_ids[old_priority].empty()) {
            priority_to_ids.erase(old_priority);
        }

        auto new_priority = ++data[id].priority;
        priority_to_ids[new_priority].insert(id);
    }

    // Get the object with the highest priority and its priority.
    [[nodiscard]] pair<const T &, int> GetMax() const {
        const auto &ids = prev(priority_to_ids.end())->second;
        const auto id = *prev(ids.end());
        return {data[id].value, data[id].priority};
    }

    // Similar to GetMax, but removes the element from the container.
    pair<T, int> PopMax() {
        const auto it = prev(priority_to_ids.end());
        const auto priority = it->first;
        const auto id = *prev(it->second.end());

        priority_to_ids[priority].erase(id);
        if (priority_to_ids[priority].empty()) {
            priority_to_ids.erase(priority);
        }

        data[id].is_valid = false;
        return {move(data[id].value), data[id].priority};
    }

private:
    struct Element {
        T value;
        size_t id;
        size_t priority;
        bool is_valid = true;
    };
    vector<Element> data;
    map<size_t, set<size_t>> priority_to_ids;
};


class StringNonCopyable : public string {
public:
    using string::string;  // Allows the use of string constructors.
    StringNonCopyable(const StringNonCopyable &) = delete;

    StringNonCopyable(StringNonCopyable &&) = default;

    StringNonCopyable &operator=(const StringNonCopyable &) = delete;

    StringNonCopyable &operator=(StringNonCopyable &&) = default;
};

void TestNoCopy() {
    PriorityCollection<StringNonCopyable> strings;
    const auto white_id = strings.Add("white");
    const auto yellow_id = strings.Add("yellow");
    const auto red_id = strings.Add("red");

    strings.Promote(yellow_id);
    for (int i = 0; i < 2; ++i) {
        strings.Promote(red_id);
    }
    strings.Promote(yellow_id);
    {
        const auto item = strings.PopMax();
        ASSERT_EQUAL(item.first, "red");
        ASSERT_EQUAL(item.second, 2);
    }
    {
        const auto item = strings.PopMax();
        ASSERT_EQUAL(item.first, "yellow");
        ASSERT_EQUAL(item.second, 2);
    }
    {
        const auto item = strings.PopMax();
        ASSERT_EQUAL(item.first, "white");
        ASSERT_EQUAL(item.second, 0);
    }
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestNoCopy);
    return 0;
}
