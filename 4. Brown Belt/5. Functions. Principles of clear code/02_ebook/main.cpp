#include <iomanip>
#include <iostream>
#include <vector>
#include <utility>

using namespace std;

class ReadingManager {
public:
    ReadingManager()
            : user_page_counts_(MAX_USER_COUNT_ + 1, 0),
              not_less_pages_counts_(MAX_PAGE_COUNT_ + 1, 0) {}

    void Read(int user_id, int page_count) {
        for (int i = user_page_counts_[user_id] + 1; i <= page_count; i++) {
            not_less_pages_counts_[i]++;
        }
        user_page_counts_[user_id] = page_count;
    }

    [[nodiscard]] double Cheer(int user_id) const {
        if (user_page_counts_[user_id] == 0) {
            return 0;
        }
        const int user_count = GetUserCount();
        if (user_count == 1) {
            return 1;
        }
        const int page_count = user_page_counts_[user_id];
        return (user_count - not_less_pages_counts_[page_count]) * 1.0 / (user_count - 1);
    }

private:
    static const int MAX_USER_COUNT_ = 100'000;
    static const int MAX_PAGE_COUNT_ = 1'000;

    vector<int> user_page_counts_;
    vector<int> not_less_pages_counts_;

    [[nodiscard]] int GetUserCount() const {
        return not_less_pages_counts_[1];
    }
};


int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    ReadingManager manager;

    int query_count;
    cin >> query_count;

    for (int query_id = 0; query_id < query_count; ++query_id) {
        string query_type;
        cin >> query_type;
        int user_id;
        cin >> user_id;

        if (query_type == "READ") {
            int page_count;
            cin >> page_count;
            manager.Read(user_id, page_count);
        } else if (query_type == "CHEER") {
            cout << setprecision(6) << manager.Cheer(user_id) << "\n";
        }
    }

    return 0;
}
