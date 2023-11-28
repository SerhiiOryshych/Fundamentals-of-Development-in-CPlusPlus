#include <iomanip>
#include <iostream>
#include <vector>
#include <utility>

using namespace std;

class ReadingManager {
public:
    ReadingManager()
            : user_page_counts_(100001, 0),
              page_positions_(1001, 0) {}

    void Read(int user_id, int page_count) {
        if (user_page_counts_[user_id] == 0) {
            AddUser();
        }

        for (int i = user_page_counts_[user_id] + 1; i <= page_count; i++) {
            page_positions_[i]++;
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
        int position = page_positions_[page_count];

        if (position == user_count) {
            return 0;
        }

        return (user_count - position) * 1.0 / (user_count - 1);
    }

private:
    int user_count_ = 0;
    vector<int> user_page_counts_;
    vector<int> page_positions_;

    [[nodiscard]] int GetUserCount() const {
        return user_count_;
    }

    void AddUser() {
        user_count_++;
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
