#include "airline_ticket.h"
#include "test_runner.h"

#include <algorithm>
#include <iostream>
#include <sstream>
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

stringstream &operator>>(stringstream &ss, Time &time) {
    char q;
    int hours, minutes;
    ss >> hours >> q >> minutes;
    time = {hours, minutes};
    return ss;
}

stringstream &operator>>(stringstream &ss, Date &date) {
    char q;
    int year, month, day;
    ss >> year >> q >> month >> q >> day;
    date = {year, month, day};
    return ss;
}

#define UPDATE_FIELD(ticket, field, values) \
{                                           \
    const auto it = values.find(#field);    \
    if (it != values.end()) {               \
        stringstream ss(it->second);        \
        ss >> ticket.field;                 \
    }                                       \
}

void TestUpdate() {
    AirlineTicket t;
    t.price = 0;

    const map<string, string> updates1 = {
            {"departure_date", "2018-2-28"},
            {"departure_time", "17:40"},
    };
    UPDATE_FIELD(t, departure_date, updates1);
    UPDATE_FIELD(t, departure_time, updates1);
    UPDATE_FIELD(t, price, updates1);

    ASSERT_EQUAL(t.departure_date, (Date{2018, 2, 28}));
    ASSERT_EQUAL(t.departure_time, (Time{17, 40}));
    ASSERT_EQUAL(t.price, 0);

    const map<string, string> updates2 = {
            {"price",        "12550"},
            {"arrival_time", "20:33"},
    };
    UPDATE_FIELD(t, departure_date, updates2);
    UPDATE_FIELD(t, departure_time, updates2);
    UPDATE_FIELD(t, arrival_time, updates2);
    UPDATE_FIELD(t, price, updates2);

    // updates2 does not contain the keys "departure_date" and "departure_time", therefore
    // the values of these fields should not change.
    ASSERT_EQUAL(t.departure_date, (Date{2018, 2, 28}));
    ASSERT_EQUAL(t.departure_time, (Time{17, 40}));
    ASSERT_EQUAL(t.price, 12550);
    ASSERT_EQUAL(t.arrival_time, (Time{20, 33}));
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestUpdate);
}
