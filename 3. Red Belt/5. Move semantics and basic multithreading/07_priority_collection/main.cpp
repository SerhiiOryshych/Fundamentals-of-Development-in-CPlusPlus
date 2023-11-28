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
    using Id = int;

    Id Add(T object) {
        const int id = data_.size();
        data_.push_back({move(object), 0, true});
        priority_to_ids_[0].insert(id);
        return id;
    }

    template<typename ObjInputIt, typename IdOutputIt>
    void Add(ObjInputIt range_begin, ObjInputIt range_end,
             IdOutputIt ids_begin) {
        for (auto it = range_begin; it != range_end; it++) {
            *ids_begin++ = Add(move(*it));
        }
    }

    [[nodiscard]] bool IsValid(Id id) const {
        if (id >= data_.size()) {
            return false;
        }
        return data_[id].is_valid;
    }

    [[nodiscard]] const T &Get(Id id) const {
        return data_[id].value;
    }

    void Promote(Id id) {
        const auto prev_priority = data_[id].priority;
        priority_to_ids_[prev_priority].erase(id);
        if (priority_to_ids_[prev_priority].empty()) {
            priority_to_ids_.erase(prev_priority);
        }
        priority_to_ids_[++data_[id].priority].insert(id);
    }

    pair<const T &, int> GetMax() const {
        const auto max_priority = prev(priority_to_ids_.end())->first;
        const auto id = *prev(priority_to_ids_.at(max_priority).end());
        return {data_[id].value, data_[id].priority};
    }

    pair<T, int> PopMax() {
        const auto max_priority = prev(priority_to_ids_.end())->first;
        const auto id = *prev(priority_to_ids_.at(max_priority).end());
        priority_to_ids_[max_priority].erase(id);
        if (priority_to_ids_[max_priority].empty()) {
            priority_to_ids_.erase(max_priority);
        }
        data_[id].is_valid = false;
        return {move(data_[id].value), data_[id].priority};
    }

private:
    struct Data {
        T value;
        int priority;
        bool is_valid;
    };
    vector<Data> data_;
    map<int, set<int>> priority_to_ids_;
};


class StringNonCopyable : public string {
public:
    using string::string;  // Allows to use strings constructors
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
