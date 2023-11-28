#include "yellow_pages.h"

#include <tuple>
#include <set>

namespace YellowPages {
    bool operator<(const Name &lhs, const Name &rhs) {
        if (lhs.value() == rhs.value()) {
            return lhs.type() < rhs.type();
        } else {
            return lhs.value() < rhs.value();
        }
    }

    bool operator<(const Phone &lhs, const Phone &rhs) {
        const auto lhs_t = std::tie(
                lhs.formatted(),
                lhs.country_code(),
                lhs.local_code(),
                lhs.number(),
                lhs.extension(),
                lhs.description()
        );

        const auto rhs_t = std::tie(
                rhs.formatted(),
                rhs.country_code(),
                rhs.local_code(),
                rhs.number(),
                rhs.extension(),
                rhs.description()
        );

        if (lhs_t == rhs_t) {
            return lhs.type() < rhs.type();
        } else {
            return lhs_t < rhs_t;
        }
    }

    bool operator<(const Url &lhs, const Url &rhs) {
        return lhs.value() < rhs.value();
    }

    class CompanyMerger {
    public:
        Company Merge(const Signals &signals, const Providers &providers) {
            for (const auto &signal: signals) {
                if (!signal.has_company()) continue;

                const auto &company = signal.company();
                const auto priority = providers.at(signal.provider_id()).priority();

                MergeAddress(company, priority);
                MergeWorkingTime(company, priority);
                MergeNames(company, priority);
                MergePhones(company, priority);
                MergeUrls(company, priority);
            }

            return MergeAttrs();
        }

    private:
        std::pair<Address, long long> address_with_priority = {Address{}, -1};
        std::pair<WorkingTime, long long> working_time_with_priority = {WorkingTime{}, -1};
        std::pair<std::set<Name>, long long> names_with_priority = {{}, -1};
        std::pair<std::set<Url>, long long> urls_with_priority = {{}, -1};
        std::pair<std::set<Phone>, long long> phones_with_priority = {{}, -1};

        void MergeAddress(const Company &company, long long priority) {
            if (!company.has_address()) return;
            if (priority < address_with_priority.second) return;

            address_with_priority = {company.address(), priority};
        }

        void MergeWorkingTime(const Company &company, long long priority) {
            if (!company.has_working_time()) return;
            if (priority < working_time_with_priority.second) return;

            working_time_with_priority = {company.working_time(), priority};
        }

        void MergeNames(const Company &company, long long priority) {
            if (company.names_size() == 0) return;
            if (priority < names_with_priority.second) return;

            if (priority > names_with_priority.second) {
                names_with_priority = {{}, priority};
            }

            for (const auto &name: company.names()) {
                names_with_priority.first.insert(name);
            }
        }

        void MergeUrls(const Company &company, long long priority) {
            if (company.urls_size() == 0) return;
            if (priority < urls_with_priority.second) return;

            if (priority > urls_with_priority.second) {
                urls_with_priority = {{}, priority};
            }

            for (const auto &url: company.urls()) {
                urls_with_priority.first.insert(url);
            }
        }

        void MergePhones(const Company &company, long long priority) {
            if (company.phones_size() == 0) return;
            if (priority < phones_with_priority.second) return;

            if (priority > phones_with_priority.second) {
                phones_with_priority = {{}, priority};
            }

            for (const auto &phone: company.phones()) {
                phones_with_priority.first.insert(phone);
            }
        }

        Company MergeAttrs() {
            Company company;
            *company.mutable_address() = address_with_priority.first;
            *company.mutable_working_time() = working_time_with_priority.first;

            for (const auto &name: names_with_priority.first) {
                *company.mutable_names()->Add() = name;
            }
            for (const auto &phone: phones_with_priority.first) {
                *company.mutable_phones()->Add() = phone;
            }
            for (const auto &url: urls_with_priority.first) {
                *company.mutable_urls()->Add() = url;
            }
            return company;
        }
    };

    Company Merge(const Signals &signals, const Providers &providers) {
        CompanyMerger merger;
        return merger.Merge(signals, providers);
    }

}
