#include "test_runner.h"

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct Record {
    string id;
    string title;
    string user;
    int timestamp;
    int karma;
};

class Database {
public:
    bool Put(const Record &record) {
        if (ids.count(record.id) > 0) {
            return false;
        }

        const size_t num_id = data.size();

        ids.insert({record.id, num_id});
        timestamp_to_id.insert({record.timestamp, num_id});
        karma_to_id.insert({record.karma, num_id});
        user_to_id.insert({record.user, num_id});

        data.push_back({record, true});
        return true;
    }

    [[nodiscard]] const Record *GetById(const string &id) const {
        if (ids.count(id) == 0) {
            return nullptr;
        }

        return &data[ids.at(id)].record;
    }

    bool Erase(const string &id) {
        if (ids.count(id) == 0) {
            return false;
        }

        const auto num_id = ids[id];
        data[num_id].is_valid = false;
        ids.erase(id);

        return true;
    }

    template<typename Callback>
    void RangeByTimestamp(int low, int high, Callback callback) const {
        for (auto it = timestamp_to_id.lower_bound(low); it != timestamp_to_id.end(); it++) {
            if (it->first > high) {
                break;
            }

            const auto num_id = it->second;
            if (!data[num_id].is_valid) {
                continue;
            }

            if (!callback(data[num_id].record)) {
                break;
            }
        }
    }

    template<typename Callback>
    void RangeByKarma(int low, int high, Callback callback) const {
        for (auto it = karma_to_id.lower_bound(low); it != karma_to_id.end(); it++) {
            if (it->first > high) {
                break;
            }

            const auto num_id = it->second;
            if (!data[num_id].is_valid) {
                continue;
            }

            if (!callback(data[num_id].record)) {
                break;
            }
        }
    }

    template<typename Callback>
    void AllByUser(const string &user, Callback callback) const {
        const auto range = user_to_id.equal_range(user);
        for (auto it = range.first; it != range.second; it++) {
            const auto num_id = it->second;
            if (!data[num_id].is_valid) {
                continue;
            }

            if (!callback(data[num_id].record)) {
                break;
            }
        }
    }

private:
    struct Data {
        Record record;
        bool is_valid;
    };
    vector<Data> data;
    unordered_map<string, size_t> ids;
    multimap<int, size_t> timestamp_to_id;
    multimap<int, size_t> karma_to_id;
    multimap<string, size_t> user_to_id;
};

void TestRangeBoundaries() {
    const int good_karma = 1000;
    const int bad_karma = -10;

    Database db;
    db.Put({"id1", "Hello there", "master", 1536107260, good_karma});
    db.Put({"id2", "O>>-<", "general2", 1536107260, bad_karma});

    int count = 0;
    db.RangeByKarma(bad_karma, good_karma, [&count](const Record &) {
        ++count;
        return true;
    });

    ASSERT_EQUAL(2, count);
}

void TestSameUser() {
    Database db;
    db.Put({"id1", "Don't sell", "master", 1536107260, 1000});
    db.Put({"id2", "Rethink life", "master", 1536107260, 2000});

    int count = 0;
    db.AllByUser("master", [&count](const Record &) {
        ++count;
        return true;
    });

    ASSERT_EQUAL(2, count);
}

void TestReplacement() {
    const string final_body = "Feeling sad";

    Database db;
    db.Put({"id", "Have a hand", "not-master", 1536107260, 10});
    db.Erase("id");
    db.Put({"id", final_body, "not-master", 1536107260, -10});

    auto record = db.GetById("id");
    ASSERT(record != nullptr);
    ASSERT_EQUAL(final_body, record->title);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestRangeBoundaries);
    RUN_TEST(tr, TestSameUser);
    RUN_TEST(tr, TestReplacement);
    return 0;
}
