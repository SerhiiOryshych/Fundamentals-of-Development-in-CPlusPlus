#pragma once

#include "rubric.h"
#include "company.pb.h"
#include "json.h"

#include <cstdint>
#include <string>

namespace YellowPages {

    using CompanyId = uint32_t;

    using Companies = std::vector<const Company *>;

    using CompaniesProto = google::protobuf::RepeatedPtrField<YellowPages::Company>;

    Json::Node MakeCompanyJson(const Company &company);

    std::string PrintCompany(const Company &company, const RubricsById &rubrics_by_id);

}
