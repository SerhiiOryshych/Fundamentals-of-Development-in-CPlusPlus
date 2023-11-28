#include <vector>
#include <iostream>
#include <optional>
#include <iomanip>
#include <ctime>

using namespace std;

struct Date {
    int year;
    int month;
    int day;

    [[nodiscard]] time_t AsTimestamp() const {
        std::tm t;
        t.tm_sec = 0;
        t.tm_min = 0;
        t.tm_hour = 0;
        t.tm_mday = day;
        t.tm_mon = month - 1;
        t.tm_year = year - 1900;
        t.tm_isdst = 0;
        return mktime(&t);
    }
};

int ComputeDaysDiff(const Date &date_to, const Date &date_from) {
    const time_t timestamp_to = date_to.AsTimestamp();
    const time_t timestamp_from = date_from.AsTimestamp();
    static const int SECONDS_IN_DAY = 60 * 60 * 24;
    return (timestamp_to - timestamp_from) / SECONDS_IN_DAY;
}

struct Query {
    string type;
    Date from;
    Date to;
    optional<long long> value;
};

istream &operator>>(istream &in, Date &date) {
    char q;
    return in >> date.year >> q >> date.month >> q >> date.day;
}

ostream &operator<<(ostream &out, Date &date) {
    return out << date.year << " " << date.month << " " << date.day;
}

istream &operator>>(istream &in, Query &query) {
    in >> query.type >> query.from >> query.to;
    if (query.type == "Earn") {
        long long value;
        in >> value;
        query.value = value;
    }
    return in;
}

ostream &operator<<(ostream &out, Query &query) {
    return out << query.type << " " << query.from << " " << query.to;
}

vector<Query> ReadQueries(istream &in) {
    int n;
    in >> n;
    vector<Query> queries;
    for (int i = 0; i < n; i++) {
        Query q;
        in >> q;
        queries.push_back(q);
    }
    return queries;
}

bool IsInDateRange(const Date &date, const Date &start, const Date &finish) {
    auto dist1 = ComputeDaysDiff(start, date);
    auto dist2 = ComputeDaysDiff(date, finish);
    return (dist1 >= 0 && dist2 >= 0);
}

void ProcessQueries(const vector<Query> &queries, ostream &out = cout) {
    const Date global_start{1999, 1, 1};
    vector<double> earnings(111 * 370);
    for (const auto &q: queries) {
        if (q.type == "Earn") {
            int start = ComputeDaysDiff(q.from, global_start);
            int end = ComputeDaysDiff(q.to, global_start);
            int len = ComputeDaysDiff(q.to, q.from) + 1;
            double value_per_day = (double) q.value.value() / len;
            for (int i = start; i <= end; i++) {
                earnings[i] += value_per_day;
            }
        } else if (q.type == "ComputeIncome") {
            int start = ComputeDaysDiff(q.from, global_start);
            int end = ComputeDaysDiff(q.to, global_start);
            double s = 0;
            for (int i = start; i <= end; i++) {
                s += earnings[i];
            }
            out << std::setprecision(25) << std::fixed << s << "\n";
        } else if (q.type == "PayTax") {
            int start = ComputeDaysDiff(q.from, global_start);
            int end = ComputeDaysDiff(q.to, global_start);
            for (int i = start; i <= end; i++) {
                earnings[i] *= 0.87;
            }
        }
    }
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    auto queries = ReadQueries(cin);
    ProcessQueries(queries);
}
