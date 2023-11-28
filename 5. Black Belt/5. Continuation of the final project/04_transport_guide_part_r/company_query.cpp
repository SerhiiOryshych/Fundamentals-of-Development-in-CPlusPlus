#include "company_query.h"
#include "hash_combine.h"
#include "map.h"

using namespace std;

namespace YellowPages {

    bool PhoneQuery::operator==(const PhoneQuery &other) const {
        return
                has_type == other.has_type
                && phone.type() == other.phone.type()
                && MakePhoneKey(phone) == MakePhoneKey(other.phone);
    }

    PhoneQuery PhoneQuery::FromJson(const Json::Dict &json) {
        PhoneQuery phone_query;
        Phone &phone = phone_query.phone;

        if ((phone_query.has_type = json.count("type"))) {
            phone.set_type(static_cast<Phone_Type>(json.at("type").AsString() == "FAX"));
        }

        if (const auto it = json.find("country_code"); it != json.end()) {
            const auto &s = it->second.AsString();
            phone.set_country_code(s);
        }

        if (const auto it = json.find("local_code"); it != json.end()) {
            const auto &s = it->second.AsString();
            phone.set_local_code(s.c_str(), s.size());
        }

        if (const auto it = json.find("number"); it != json.end()) {
            const auto &s = it->second.AsString();
            phone.set_number(s.c_str(), s.size());
        }

        if (const auto it = json.find("extension"); it != json.end()) {
            const auto &s = it->second.AsString();
            phone.set_extension(s.c_str(), s.size());
        }

        return phone_query;
    }

    size_t PhoneQueryHasher::operator()(const PhoneQuery &phone_query) const {
        const Phone &phone = phone_query.phone;
        return ComputeCombinedHash(
                phone_query.has_type,
                phone.type(),
                phone.country_code(),
                phone.local_code(),
                phone.number(),
                phone.extension()
        );
    }

    CompanyQuery CompanyQuery::FromJson(const Json::Dict &json) {
        CompanyQuery query;

        if (const auto *names = GetValuePointer(json, "names")) {
            query.names.reserve(names->AsArray().size());
            for (const auto &name: names->AsArray()) {
                query.names.push_back(name.AsString());
            }
        }

        if (const auto *urls = GetValuePointer(json, "urls")) {
            query.urls.reserve(urls->AsArray().size());
            for (const auto &url: urls->AsArray()) {
                query.urls.push_back(url.AsString());
            }
        }

        if (const auto *rubrics = GetValuePointer(json, "rubrics")) {
            query.rubrics.reserve(rubrics->AsArray().size());
            for (const auto &rubric: rubrics->AsArray()) {
                query.rubrics.push_back(rubric.AsString());
            }
        }

        if (const auto *phones = GetValuePointer(json, "phones")) {
            query.phones.reserve(phones->AsArray().size());
            for (const auto &phone: phones->AsArray()) {
                query.phones.push_back(PhoneQuery::FromJson(phone.AsMap()));
            }
        }

        return query;
    }

}
