#include "date.h"

using namespace std;

Date ParseDate(istream &is) {
    char space;
    int year, month, day;
    is >> year >> space >> month >> space >> day;
    return {year, month, day};
}

Date::Date(int year, int month, int day) : year_(year), month_(month), day_(day) {}

int Date::GetYear() const {
    return year_;
}

int Date::GetMonth() const {
    return month_;
}

int Date::GetDay() const {
    return day_;
}

bool operator<(const Date &a, const Date &b) {
    if (a.GetYear() != b.GetYear()) {
        return a.GetYear() < b.GetYear();
    }

    if (a.GetMonth() != b.GetMonth()) {
        return a.GetMonth() < b.GetMonth();
    }

    return a.GetDay() < b.GetDay();
}

bool operator>(const Date &a, const Date &b) {
    if (a.GetYear() != b.GetYear()) {
        return a.GetYear() > b.GetYear();
    }

    if (a.GetMonth() != b.GetMonth()) {
        return a.GetMonth() > b.GetMonth();
    }

    return a.GetDay() > b.GetDay();
}

bool operator==(const Date &a, const Date &b) {
    return (a.GetYear() == b.GetYear() && a.GetMonth() == b.GetMonth() && a.GetDay() == b.GetDay());
}