#pragma once

#include <string>

using namespace std;

class PhoneNumber {
public:
    /*
    Accepts a string in the format +XXX-YYY-ZZZZZZ
    The part from '+' to the first '-' is the country code.
    The part between the first and second '-' characters is the city code.
    Everything after the second '-' character is the local number.
    The country code, city code, and local number should not be empty.
    If the string does not match this format, an invalid_argument exception should be thrown. There is no need to check that the number contains only digits.

    Examples:
    * +7-495-111-22-33
    * +7-495-1112233
    * +323-22-460002
    * +1-2-coursera-cpp
    * 1-2-333 - incorrect number - does not start with '+'
    * +7-1233 - incorrect number - only country and city codes are present
    */

    explicit PhoneNumber(const string &international_number);

    string GetCountryCode() const;

    string GetCityCode() const;

    string GetLocalNumber() const;

    string GetInternationalNumber() const;

private:
    string country_code_;
    string city_code_;
    string local_number_;
};
