#include "test_runner.h"

using namespace std;

template<typename T>
class Table {
public:
    Table(const ::size_t n, const ::size_t m) : n_(n), m_(m) {
        table_.resize(n_, vector<T>(m_));
    }

    vector<T> operator[](int i) const {
        return table_[i];
    }

    vector<T> &operator[](int i) {
        return table_[i];
    }

    [[nodiscard]] pair<::size_t, ::size_t> Size() const {
        return {n_, m_};
    }

    void Resize(const ::size_t n, const ::size_t m) {
        n_ = n;
        m_ = m;
        table_.resize(n_, vector<T>(m_));
        for (auto &row: table_) {
            row.resize(m, T());
        }
    }

private:
    ::size_t n_, m_;
    vector<vector<T>> table_;
};

void TestTable() {
    Table<int> t(1, 1);
    ASSERT_EQUAL(t.Size().first, 1u);
    ASSERT_EQUAL(t.Size().second, 1u);
    t[0][0] = 42;
    ASSERT_EQUAL(t[0][0], 42);
    t.Resize(3, 4);
    ASSERT_EQUAL(t.Size().first, 3u);
    ASSERT_EQUAL(t.Size().second, 4u);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestTable);
}
