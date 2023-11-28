#include <sstream>
#include <iostream>
#include <vector>
#include <ctime>

using namespace std;

struct Date {
    int year, month, day;
};

time_t getTimestamp(const Date &date) {
    tm x = {0, 0, 0, date.day, date.month - 1, date.year - 1900};
    return mktime(&x);
}

int getDayNumber(const Date &date) {
    return (getTimestamp(date) - getTimestamp({2000, 1, 1})) / (60 * 60 * 24);
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
    int q;
    cin >> q;

    vector<long double> earnings(366 * 101);
    std::cout.precision(25);

    while (q--) {
        string type;
        cin >> type;

        if (type == "Earn") {
            Date start, end;
            long double sum;
            cin >> start >> end >> sum;
            int start_day = getDayNumber(start);
            int end_day = getDayNumber(end);
            long double per_day = sum / (end_day - start_day + 1);
            for (int day = start_day; day <= end_day; day++) {
                earnings[day] += per_day;
            }

        } else {
            Date start, end;
            cin >> start >> end;
            int start_day = getDayNumber(start);
            int end_day = getDayNumber(end);

            long double ans = 0;
            for (int day = start_day; day <= end_day; day++) {
                ans += earnings[day];
            }
            cout << ans << endl;
        }
    }
}
