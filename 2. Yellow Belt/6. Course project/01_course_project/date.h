#pragma once

#include <sstream>
#include <iostream>

using namespace std;

class Date {
public:
    Date(int year, int month, int day);

    int GetYear() const;

    int GetMonth() const;

    int GetDay() const;

private:
    int year_, month_, day_;
};

Date ParseDate(istream &is);

bool operator<(const Date &a, const Date &b);

bool operator>(const Date &a, const Date &b);

bool operator==(const Date &a, const Date &b);