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
    if (query.type == "Earn" || query.type == "PayTax" || query.type == "Spend") {
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

struct DayBudget {
    double earning;
    double spending;
};

void ProcessQueries(const vector<Query> &queries, ostream &out = cout) {
    const Date global_start{1999, 1, 1};
    vector<DayBudget> day_budget(111 * 370);
    for (const auto &q: queries) {
        if (q.type == "Earn") {
            int start = ComputeDaysDiff(q.from, global_start);
            int end = ComputeDaysDiff(q.to, global_start);
            int len = ComputeDaysDiff(q.to, q.from) + 1;
            double value_per_day = (double) q.value.value() / len;
            for (int i = start; i <= end; i++) {
                day_budget[i].earning += value_per_day;
            }
        } else if (q.type == "ComputeIncome") {
            int start = ComputeDaysDiff(q.from, global_start);
            int end = ComputeDaysDiff(q.to, global_start);
            double s = 0;
            for (int i = start; i <= end; i++) {
                s += day_budget[i].earning - day_budget[i].spending;
            }
            out << std::setprecision(25) << std::fixed << s << "\n";
        } else if (q.type == "PayTax") {
            int start = ComputeDaysDiff(q.from, global_start);
            int end = ComputeDaysDiff(q.to, global_start);
            double coef = (100 - q.value.value()) / 100.0;
            for (int i = start; i <= end; i++) {
                day_budget[i].earning *= coef;
            }
        } else if (q.type == "Spend") {
            int start = ComputeDaysDiff(q.from, global_start);
            int end = ComputeDaysDiff(q.to, global_start);
            int len = ComputeDaysDiff(q.to, q.from) + 1;
            double value_per_day = (double) q.value.value() / len;
            for (int i = start; i <= end; i++) {
                day_budget[i].spending += value_per_day;
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
