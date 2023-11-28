#pragma once

#include "rubric.h"
#include "json.h"
#include "phone.pb.h"

namespace YellowPages {

    struct PhoneQuery {
        Phone phone;
        bool has_type;

        bool operator==(const PhoneQuery &other) const;

        static auto MakePhoneKey(const Phone &phone) {
            return std::tie(phone.country_code(), phone.local_code(), phone.number(), phone.extension());
        }

        static PhoneQuery FromJson(const Json::Dict &json);
    };

    struct PhoneQueryHasher {
        size_t operator()(const PhoneQuery &) const;
    };

    struct CompanyQuery {
        std::vector<std::string> names;
        std::vector<std::string> urls;
        std::vector<std::string> rubrics;
        std::vector<PhoneQuery> phones;

        static CompanyQuery FromJson(const Json::Dict &json);
    };

}
