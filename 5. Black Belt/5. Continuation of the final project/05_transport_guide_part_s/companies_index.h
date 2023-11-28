#pragma once

#include "company.h"
#include "company_query.h"
#include "rubric.h"
#include "range.h"
#include "company.pb.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace YellowPages {

    class CompaniesIndex {
    public:
        template<typename CompaniesIt>
        CompaniesIndex(CompaniesIt begin, CompaniesIt end);

        [[nodiscard]] Companies Find(const CompanyQuery &query, const RubricsNameToId &rubrics_name_to_id) const;

    private:
        std::unordered_map<std::string, Companies> name_index_;
        std::unordered_map<std::string, Companies> url_index_;
        std::unordered_map<uint64_t, Companies> rubric_index_;
        std::unordered_map<PhoneQuery, Companies, PhoneQueryHasher> phone_index_;

        void UpdateNameIndex(const Company &company);

        void UpdateUrlIndex(const Company &company);

        void UpdateRubricIndex(const Company &company);

        void UpdatePhoneIndex(const Company &company);

        void SortCompanies();
    };

    template<typename CompaniesIt>
    CompaniesIndex::CompaniesIndex(CompaniesIt begin, CompaniesIt end) {
        for (const Company &company: Range(begin, end)) {
            UpdateNameIndex(company);
            UpdateUrlIndex(company);
            UpdateRubricIndex(company);
            UpdatePhoneIndex(company);
        }
        SortCompanies();
    }

}
