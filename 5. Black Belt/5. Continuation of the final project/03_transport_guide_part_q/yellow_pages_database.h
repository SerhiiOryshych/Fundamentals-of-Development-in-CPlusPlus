#pragma once

#include "json.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

struct Rubric {
    std::string name;
    std::vector<std::string> keywords;
};

struct Name {
    enum NameType {
        MAIN,
        SYNONYM,
        SHORT,
    };

    static NameType convertStrToType(const std::string &type_str) {
        if (type_str == "MAIN") {
            return NameType::MAIN;
        }

        if (type_str == "SYNONYM") {
            return NameType::SYNONYM;
        }

        if (type_str == "SYNONYM") {
            return NameType::SHORT;
        }

        return NameType::MAIN;
    }

    std::string value_;
    NameType type_;
};

struct Phone {
    enum PhoneType {
        PHONE, FAX
    };

    static PhoneType convertStrToType(const std::string &type_str) {
        if (type_str == "PHONE") {
            return PhoneType::PHONE;
        }

        if (type_str == "FAX") {
            return PhoneType::FAX;
        }

        return PhoneType::PHONE;
    }

    std::string formatted_;
    PhoneType type_ = PhoneType::PHONE;
    std::string country_code_;
    std::string local_code_;
    std::string number_;
    std::string extension_;
    std::string description_;
};

struct Company {
    std::vector<Name> names_;
    std::vector<std::string> urls_;
    std::vector<std::string> rubric_ids;
    std::vector<Phone> phones_;
};

struct PhoneQuery {
    std::optional<Phone::PhoneType> type_;
    std::string country_code_;
    std::string local_code_;
    std::string number_;
    std::string extension_;
};

class YellowPagesDB {
public:
    void FetchRubrics(const Json::Node &rubrics_node) {
        const auto &rubrics_map = rubrics_node.AsMap();
        for (const auto &[id, node]: rubrics_map) {
            // name
            std::string name = node.AsMap().at("name").AsString();
            // keywords
            std::vector<std::string> keywords;
            if (node.AsMap().count("keywords")) {
                for (const auto &keyword: node.AsMap().at("keywords").AsArray()) {
                    keywords.push_back(keyword.AsString());
                }
            }

            rubric_name_to_rubric_id_[name] = id;
            rubric_id_to_rubric_[id] = {std::move(name), std::move(keywords)};
        }
    }

    void FetchCompanies(const Json::Node &companies_node) {
        const auto &companies_array = companies_node.AsArray();
        for (const auto &company: companies_array) {
            // company id
            const int company_id = companies_.size();
            // names
            std::string main_name;
            std::vector<Name> names;
            try {
                for (const auto &name_node: company.AsMap().at("names").AsArray()) {
                    std::string value = name_node.AsMap().at("value").AsString();
                    Name::NameType type = Name::convertStrToType(
                            name_node.AsMap().count("type") ? name_node.AsMap().at("type").AsString()
                                                            : "");
                    if (main_name.empty() && type == Name::NameType::MAIN) {
                        main_name = value;
                    }
                    company_name_to_companies_ids_[value].insert(company_id);
                    names.push_back({std::move(value), type});
                }
            } catch (const std::exception &e) {
                std::cerr << "company names: " << e.what() << "\n";
                throw std::runtime_error("company names");
            }

            // urls
            std::vector<std::string> urls;
            try {
                if (company.AsMap().count("urls")) {
                    for (const auto &url_node: company.AsMap().at("urls").AsArray()) {
                        std::string value = url_node.AsMap().at("value").AsString();
                        url_name_to_companies_ids_[value].insert(company_id);
                        urls.push_back(std::move(value));
                    }
                }
            } catch (const std::exception &e) {
                std::cerr << "company urls: " << e.what() << "\n";
                throw std::runtime_error("company urls");
            }

            // rubrics
            std::vector<std::string> rubrics;
            try {
                if (company.AsMap().count("rubrics")) {
                    for (const auto &rubric_node: company.AsMap().at("rubrics").AsArray()) {
                        std::string rubric_id = std::to_string(rubric_node.AsInt());
                        rubric_id_to_companies_ids_[rubric_id].insert(company_id);
                        rubrics.push_back(std::move(rubric_id));
                    }
                }
            } catch (const std::exception &e) {
                std::cerr << "company rubrics: " << e.what() << "\n";
                throw std::runtime_error("company rubrics");
            }

            // phones
            std::vector<Phone> phones;
            try {
                if (company.AsMap().count("phones")) {
                    for (const auto &phone_node: company.AsMap().at("phones").AsArray()) {
                        const auto &phone_map = phone_node.AsMap();
                        std::string formatted = phone_map.count("formatted") ? phone_map.at("formatted").AsString()
                                                                             : "";
                        Phone::PhoneType type = Phone::convertStrToType(
                                phone_map.count("type") ? phone_map.at("type").AsString() : "");
                        std::string country_code = phone_map.count("country_code") ? phone_map.at(
                                "country_code").AsString() : "";
                        std::string local_code = phone_map.count("local_code") ? phone_map.at("local_code").AsString()
                                                                               : "";
                        std::string number = phone_map.count("number") ? phone_map.at("number").AsString() : "";
                        std::string extension = phone_map.count("extension") ? phone_map.at("extension").AsString()
                                                                             : "";
                        std::string description = phone_map.count("description") ? phone_map.at(
                                "description").AsString() : "";
                        phones.push_back({
                                                 std::move(formatted),
                                                 type,
                                                 std::move(country_code),
                                                 std::move(local_code),
                                                 std::move(number),
                                                 std::move(extension),
                                                 std::move(description)
                                         });
                    }
                }
            } catch (const std::exception &e) {
                std::cerr << "company phones: " << e.what() << "\n";
                throw std::runtime_error("company phones");
            }

            company_id_to_company_name_[company_id] = main_name;
            companies_.push_back({
                                         std::move(names),
                                         std::move(urls),
                                         std::move(rubrics),
                                         std::move(phones)
                                 });
        }
    }

    [[nodiscard]] std::unordered_set<int> GetSetWithAllCompanies() const {
        std::unordered_set<int> all_companies_ids;
        for (int i = 0; i < companies_.size(); i++) {
            all_companies_ids.insert(i);
        }
        return std::move(all_companies_ids);
    }

    [[nodiscard]] std::vector<std::string> GetCompaniesNamesByIds(const std::unordered_set<int> &companies_ids) const {
        std::vector<std::string> names;
        for (auto id: companies_ids) {
            names.push_back(company_id_to_company_name_.at(id));
        }
        return std::move(names);
    }

    [[nodiscard]] std::unordered_set<int>
    FilterCompaniesByNames(const std::unordered_set<int> &suitable_companies, const Json::Node &request) const {
        if (request.AsArray().empty()) {
            return suitable_companies;
        }

        std::unordered_set<int> filtered_companies;
        for (const auto &name_node: request.AsArray()) {
            const auto &name_str = name_node.AsString();
            if (company_name_to_companies_ids_.count(name_str)) {
                for (auto id: company_name_to_companies_ids_.at(name_str)) {
                    if (suitable_companies.count(id)) {
                        filtered_companies.insert(id);
                    }
                }
            }
        }
        return filtered_companies;
    }

    [[nodiscard]] std::unordered_set<int>
    FilterCompaniesByUrls(const std::unordered_set<int> &suitable_companies, const Json::Node &request) const {
        if (request.AsArray().empty()) {
            return suitable_companies;
        }

        std::unordered_set<int> filtered_companies;
        for (const auto &name_node: request.AsArray()) {
            const auto &name_str = name_node.AsString();
            if (url_name_to_companies_ids_.count(name_str)) {
                for (auto id: url_name_to_companies_ids_.at(name_str)) {
                    if (suitable_companies.count(id)) {
                        filtered_companies.insert(id);
                    }
                }
            }
        }
        return filtered_companies;
    }

    [[nodiscard]] std::unordered_set<int>
    FilterCompaniesByRubrics(const std::unordered_set<int> &suitable_companies, const Json::Node &request) const {
        if (request.AsArray().empty()) {
            return suitable_companies;
        }

        std::unordered_set<int> filtered_companies;
        for (const auto &name_node: request.AsArray()) {
            const auto &name_str = name_node.AsString();
            if (rubric_name_to_rubric_id_.count(name_str)) {
                const auto &id_str = rubric_name_to_rubric_id_.at(name_str);
                if (rubric_id_to_companies_ids_.count(id_str)) {
                    for (auto id: rubric_id_to_companies_ids_.at(id_str)) {
                        if (suitable_companies.count(id)) {
                            filtered_companies.insert(id);
                        }
                    }
                }
            }
        }
        return filtered_companies;
    }

    [[nodiscard]] std::unordered_set<int>
    FilterCompaniesByPhones(const std::unordered_set<int> &suitable_companies, const Json::Node &request) const {
        std::unordered_set<int> filtered_companies;

        for (const auto &query: request.AsArray()) {
            const auto phone_query = FetchPhoneQuery(query);
            for (const auto company_id: suitable_companies) {
                if (filtered_companies.count(company_id)) {
                    continue;
                }
                const auto &company_phones = companies_[company_id].phones_;
                for (const auto &phone: company_phones) {
                    if (DoesPhoneMatch(phone_query, phone)) {
                        filtered_companies.insert(company_id);
                        break;
                    }
                }
            }
        }
        return filtered_companies;
    }

private:
    std::unordered_map<std::string, Rubric> rubric_id_to_rubric_;
    std::vector<Company> companies_;
    std::unordered_map<int, std::string> company_id_to_company_name_;
    std::unordered_map<std::string, std::unordered_set<int>> company_name_to_companies_ids_;
    std::unordered_map<std::string, std::unordered_set<int>> url_name_to_companies_ids_;
    std::unordered_map<std::string, std::unordered_set<int>> rubric_id_to_companies_ids_;
    std::unordered_map<std::string, std::string> rubric_name_to_rubric_id_;

    [[nodiscard]] static PhoneQuery FetchPhoneQuery(const Json::Node &request) {
        const auto &query_map = request.AsMap();
        std::optional<Phone::PhoneType> type_ = query_map.count("type")
                                                ? std::optional<Phone::PhoneType>(
                        Phone::convertStrToType(query_map.at("type").AsString()))
                                                : std::optional<Phone::PhoneType>();
        std::string country_code_ = query_map.count("country_code") ? query_map.at("country_code").AsString() : "";
        std::string local_code_ = query_map.count("local_code") ? query_map.at("local_code").AsString() : "";
        std::string number_ = query_map.count("number") ? query_map.at("number").AsString() : "";
        std::string extension_ = query_map.count("extension") ? query_map.at("extension").AsString() : "";
        return {
                type_,
                std::move(country_code_),
                std::move(local_code_),
                std::move(number_),
                std::move(extension_)
        };
    }

    static bool DoesPhoneMatch(const PhoneQuery &query, const Phone &object) {
        if (!query.extension_.empty() && query.extension_ != object.extension_) {
            return false;
        }

        if (query.type_ && query.type_ != object.type_) {
            return false;
        }

        if (!query.country_code_.empty() && query.country_code_ != object.country_code_) {
            return false;
        }

        if (
                (!query.local_code_.empty() || !query.country_code_.empty())
                && query.local_code_ != object.local_code_
                ) {
            return false;
        }

        return query.number_ == object.number_;
    }
};