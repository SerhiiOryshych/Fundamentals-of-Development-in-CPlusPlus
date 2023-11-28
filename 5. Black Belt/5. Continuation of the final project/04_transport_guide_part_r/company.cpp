#include "company.h"

#include <algorithm>

using namespace std;

namespace YellowPages {

    static const auto &FindMainCompanyName(const Company &company) {
        const auto &names = company.names();
        return find_if(
                begin(names), end(names),
                [](const Name &name) { return name.type() == Name_Type_MAIN; }
        )->value();
    }

    Json::Node MakeCompanyJson(const Company &company) {
        return FindMainCompanyName(company);
    }

    string PrintCompany(const Company &company, const RubricsById &rubrics_by_id) {
        const string &name = FindMainCompanyName(company);
        if (company.rubrics_size() == 0) {
            return name;
        } else {
            const string &rubric = rubrics_by_id.at(company.rubrics(0)).name();
            return rubric + " " + name;
        }
    }

}
