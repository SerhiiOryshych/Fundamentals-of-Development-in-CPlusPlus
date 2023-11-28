#include "airline_ticket.h"
#include "test_runner.h"

#include <iostream>
#include <algorithm>
#include <numeric>

using namespace std;

ostream &operator<<(ostream &os, const Time &time) {
    return os << time.hours << ":" << time.minutes;
}

ostream &operator<<(ostream &os, const Date &date) {
    return os << date.year << "." << date.month << "." << date.day;
}

bool operator==(const Time &a, const Time &b) {
    return (a.hours == b.hours && a.minutes == b.minutes);
}

bool operator==(const Date &a, const Date &b) {
    return (a.year == b.year && a.month == b.month && a.day == b.day);
}

bool operator<(const Time &a, const Time &b) {
    if (a.hours != b.hours) {
        return a.hours < b.hours;
    }
    return a.minutes < b.minutes;
}

bool operator<(const Date &a, const Date &b) {
    if (a.year != b.year) {
        return a.year < b.year;
    }

    if (a.month != b.month) {
        return a.month < b.month;
    }

    return a.day < b.day;
}

#define SORT_BY(field)                                      \
    [](const AirlineTicket &a, const AirlineTicket &b) {    \
        return a.field < b.field;                           \
    }


void TestSortBy() {
    vector<AirlineTicket> tixs = {
            {"VKO", "AER", "Utair",     {2018, 2, 28}, {17, 40}, {2018, 2, 28}, {20, 0},  1200},
            {"AER", "VKO", "Utair",     {2018, 3, 5},  {14, 15}, {2018, 3, 5},  {16, 30}, 1700},
            {"AER", "SVO", "Aeroflot",  {2018, 3, 5},  {18, 30}, {2018, 3, 5},  {20, 30}, 2300},
            {"PMI", "DME", "Iberia",    {2018, 2, 8},  {23, 00}, {2018, 2, 9},  {3,  30}, 9000},
            {"CDG", "SVO", "AirFrance", {2018, 3, 1},  {13, 00}, {2018, 3, 1},  {17, 30}, 8000},
    };

    sort(begin(tixs), end(tixs), SORT_BY(price));
    ASSERT_EQUAL(tixs.front().price, 1200);
    ASSERT_EQUAL(tixs.back().price, 9000);

    sort(begin(tixs), end(tixs), SORT_BY(from));
    ASSERT_EQUAL(tixs.front().from, "AER");
    ASSERT_EQUAL(tixs.back().from, "VKO");

    sort(begin(tixs), end(tixs), SORT_BY(arrival_date));
    ASSERT_EQUAL(tixs.front().arrival_date, (Date{2018, 2, 9}));
    ASSERT_EQUAL(tixs.back().arrival_date, (Date{2018, 3, 5}));
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestSortBy);
}
