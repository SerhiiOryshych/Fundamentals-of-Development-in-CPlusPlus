#include "phone_number.h"
#include <stdexcept>
#include <vector>
#include <string>

using namespace std;

PhoneNumber::PhoneNumber(const string &international_number) {
    if (international_number.length() == 0 ||
        international_number[0] != '+') {
        throw invalid_argument("");
    }

    vector<int> hyphen_positions{0};
    for (int i = 1; i < international_number.length(); i++) {
        if (international_number[i] == '-') {
            hyphen_positions.push_back(i);
        }
    }
    hyphen_positions.push_back(international_number.length());

    int hyphen_size = hyphen_positions.size();
    int hyphen_0 = hyphen_positions[0];
    int hyphen_1 = hyphen_positions[1];
    int hyphen_2 = hyphen_positions[2];
    int hyphen_n = hyphen_positions[hyphen_size - 1];
    if (hyphen_size < 4 ||
        hyphen_1 == hyphen_0 + 1 ||
        hyphen_2 == hyphen_1 + 1 ||
        hyphen_n == hyphen_2 + 1) {
        throw invalid_argument("");
    }

    country_code_ = international_number.substr(hyphen_0 + 1,
                                                hyphen_1 - hyphen_0 - 1);
    city_code_ = international_number.substr(hyphen_1 + 1,
                                             hyphen_2 - hyphen_1 - 1);
    local_number_ = international_number.substr(hyphen_2 + 1,
                                                hyphen_n - hyphen_2 - 1);

    for (int i = 0; i < local_number_.size() - 1; i++) {
        if (local_number_[i] == '-' && local_number_[i + 1] == '-') {
            throw invalid_argument("");
        }
    }
}

string PhoneNumber::GetCountryCode() const {
    return country_code_;
}

string PhoneNumber::GetCityCode() const {
    return city_code_;
}

string PhoneNumber::GetLocalNumber() const {
    return local_number_;
}

string PhoneNumber::GetInternationalNumber() const {
    return "+" + country_code_ + "-" + city_code_ + "-" + local_number_;
}
