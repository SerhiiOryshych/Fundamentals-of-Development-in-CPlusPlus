#include <iostream>
#include <numeric>
#include <vector>
#include <ctime>

using namespace std;

struct Date {
    int year, month, day;
};

time_t getTimestamp(const Date &date) {
    tm x = {0, 0, 0, date.day, date.month - 1, date.year - 1900 + 401};
    return mktime(&x);
}

int getDayNumber(const Date &date) {
    return (getTimestamp(date) - getTimestamp({1699, 12, 31})) / (60 * 60 * 24);
}

istream &operator>>(istream &stream, Date &date) {
    stream >> date.year;
    stream.ignore();
    stream >> date.month;
    stream.ignore();
    stream >> date.day;
    return stream;
}

int main() {
    int earnings_cnt;
    cin >> earnings_cnt;
    vector<double> earnings(366 * 402);
    while (earnings_cnt--) {
        Date date;
        double value;
        cin >> date >> value;
        int day_number = getDayNumber(date);
        earnings[day_number] += value;
    }

    vector<double> partial_sums(earnings.size());
    partial_sum(earnings.begin(), earnings.end(), partial_sums.begin());

    int queries_cnt;
    cin >> queries_cnt;
    std::cout.precision(25);
    while (queries_cnt--) {
        Date start, end;
        cin >> start >> end;
        int start_day = getDayNumber(start);
        int end_day = getDayNumber(end);
        cout << partial_sums[end_day] - partial_sums[start_day - 1] << endl;
    }
}
