#include "test_runner.h"
#include <algorithm>
#include <iostream>
#include <optional>
#include <string_view>
#include <vector>

using namespace std;

template<typename It>
class Range {
public:
    Range(It begin, It end) : begin_(begin), end_(end) {}

    [[nodiscard]] It begin() const { return begin_; }

    [[nodiscard]] It end() const { return end_; }

private:
    It begin_;
    It end_;
};

pair<string_view, optional<string_view>> SplitTwoStrict(string_view s, string_view delimiter = " ") {
    const size_t pos = s.find(delimiter);
    if (pos == s.npos) {
        return {s, nullopt};
    } else {
        return {s.substr(0, pos), s.substr(pos + delimiter.length())};
    }
}

vector<string_view> Split(string_view s, string_view delimiter = " ") {
    vector<string_view> parts;
    if (s.empty()) {
        return parts;
    }
    while (true) {
        const auto [lhs, rhs_opt] = SplitTwoStrict(s, delimiter);
        parts.push_back(lhs);
        if (!rhs_opt) {
            break;
        }
        s = *rhs_opt;
    }
    return parts;
}

void Test1() {
    {
        string s = "a";
        vector<string_view> expected = {"a"};
        AssertEqual(Split(s), expected);
        AssertEqual(Split(s, "."), expected);
    }
    {
        string s = "a.bc.defg";
        vector<string_view> expected = {"a", "bc", "defg"};
        AssertEqual(Split(s, "."), expected);
    }
    {
        string s = "a bc defg";
        vector<string_view> expected = {"a", "bc", "defg"};
        AssertEqual(Split(s), expected);
    }
}

class Domain {
public:
    explicit Domain(string_view text) {
        vector<string_view> parts = Split(text, ".");
        parts_reversed_.assign(rbegin(parts), rend(parts));
    }

    [[nodiscard]] size_t GetPartCount() const {
        return parts_reversed_.size();
    }

    [[nodiscard]] auto GetReversedParts() const {
        return Range(begin(parts_reversed_), end(parts_reversed_));
    }

private:
    vector<string> parts_reversed_;
};

void Test2() {
    {
        Domain domain{"a"};
        vector<string> expected_parts = {"a"};
        AssertEqual(domain.GetPartCount(), expected_parts.size());
        auto range = domain.GetReversedParts();
        int i = 0;
        for (const auto &s: range) {
            AssertEqual(s, expected_parts[i++]);
        }
    }
    {
        Domain domain{"a.bc.def"};
        vector<string> expected_parts = {"def", "bc", "a"};
        AssertEqual(domain.GetPartCount(), expected_parts.size());
        auto range = domain.GetReversedParts();
        int i = 0;
        for (const auto &s: range) {
            AssertEqual(s, expected_parts[i++]);
        }
    }
}

// domain is subdomain of itself
bool IsSubdomain(const Domain &subdomain, const Domain &domain) {
    const auto subdomain_reversed_parts = subdomain.GetReversedParts();
    const auto domain_reversed_parts = domain.GetReversedParts();
    return
            subdomain.GetPartCount() >= domain.GetPartCount()
            && equal(begin(domain_reversed_parts), end(domain_reversed_parts),
                     begin(subdomain_reversed_parts));
}

void Test3() {
    {
        Domain subdomain{"a"};
        Domain domain{"a"};
        Assert(IsSubdomain(subdomain, domain), "test3_1");
    }
    {
        Domain subdomain{"a.b"};
        Domain domain{"a.b"};
        Assert(IsSubdomain(subdomain, domain), "test3_2");
    }
    {
        Domain subdomain{"c.b.a"};
        Domain domain{"b.a"};
        Assert(IsSubdomain(subdomain, domain), "test3_3");
    }
    {
        Domain subdomain{"b.a"};
        Domain domain{"c.b.a"};
        Assert(!IsSubdomain(subdomain, domain), "test3_3");
    }
    {
        Domain subdomain{"cb.a"};
        Domain domain{"b.a"};
        Assert(!IsSubdomain(subdomain, domain), "test3_4");
    }
}

bool IsSubOrSuperDomain(const Domain &lhs, const Domain &rhs) {
    return lhs.GetPartCount() >= rhs.GetPartCount()
           ? IsSubdomain(lhs, rhs)
           : IsSubdomain(rhs, lhs);
}


class DomainChecker {
public:
    template<typename InputIt>
    DomainChecker(InputIt domains_begin, InputIt domains_end) {
        sorted_domains_.reserve(distance(domains_begin, domains_end));
        for (const Domain &domain: Range(domains_begin, domains_end)) {
            sorted_domains_.push_back(&domain);
        }
        sort(begin(sorted_domains_), end(sorted_domains_), IsDomainLess);
        sorted_domains_ = AbsorbSubdomains(std::move(sorted_domains_));
    }

    // Check if candidate is subdomain of some domain
    [[nodiscard]] bool IsSubdomain(const Domain &candidate) const {
        const auto it = upper_bound(
                begin(sorted_domains_), end(sorted_domains_),
                &candidate, IsDomainLess);
        if (it == begin(sorted_domains_)) {
            return false;
        }
        return ::IsSubdomain(candidate, **prev(it));
    }

private:
    vector<const Domain *> sorted_domains_;

    static bool IsDomainLess(const Domain *lhs, const Domain *rhs) {
        const auto lhs_reversed_parts = lhs->GetReversedParts();
        const auto rhs_reversed_parts = rhs->GetReversedParts();
        return lexicographical_compare(
                begin(lhs_reversed_parts), end(lhs_reversed_parts),
                begin(rhs_reversed_parts), end(rhs_reversed_parts)
        );
    }

    static vector<const Domain *> AbsorbSubdomains(vector<const Domain *> domains) {
        domains.erase(
                unique(begin(domains), end(domains),
                       [](const Domain *lhs, const Domain *rhs) {
                           return IsSubOrSuperDomain(*lhs, *rhs);
                       }),
                end(domains)
        );
        return domains;
    }
};

void Test4() {
    {
        vector<Domain> prohibited = {
                Domain("b.c"),
                Domain("c")
        };
        Domain subdomain{"c.c"};
        DomainChecker checker(prohibited.begin(), prohibited.end());
        Assert(checker.IsSubdomain(subdomain), "test4_1");
    }
    {
        vector<Domain> prohibited = {
                Domain("b.c"),
                Domain("c")
        };
        Domain subdomain{"c"};
        DomainChecker checker(prohibited.begin(), prohibited.end());
        Assert(checker.IsSubdomain(subdomain), "test4_1");
    }
}

vector<Domain> ReadDomains(istream &in_stream = cin) {
    vector<Domain> domains;

    size_t count;
    in_stream >> count;
    domains.reserve(count);

    for (size_t i = 0; i < count; ++i) {
        string domain_text;
        in_stream >> domain_text;
        domains.emplace_back(domain_text);
    }
    return domains;
}

void Test7() {
    {
        stringstream in("3\na\na.b\na.b.c");
        vector<Domain> expected = {
                Domain("a"),
                Domain("a.b"),
                Domain("a.b.c")
        };
        vector<Domain> domains = ReadDomains(in);
        AssertEqual(domains.size(), expected.size());
        for (const auto &d: domains) {
            Assert(d.GetPartCount() > 0, "test7_1");
        }
    }
}

vector<bool> CheckDomains(const vector<Domain> &banned_domains, const vector<Domain> &domains_to_check) {
    const DomainChecker checker(begin(banned_domains), end(banned_domains));

    vector<bool> check_results;
    check_results.reserve(domains_to_check.size());
    for (const Domain &domain_to_check: domains_to_check) {
        check_results.push_back(!checker.IsSubdomain(domain_to_check));
    }

    return check_results;
}

void Test5() {
    {
        vector<Domain> prohibited = {
                Domain("c"),
                Domain("b.c"),
                Domain("c.d.a")
        };
        vector<Domain> candidates = {
                Domain("a"),
                Domain("a.c"),
                Domain("b.c"),
                Domain("d.c"),
                Domain("d.a")
        };
        vector<bool> expected = {true, false, false, false, true};
        AssertEqual(CheckDomains(prohibited, candidates), expected);
    }
}

void PrintCheckResults(const vector<bool> &check_results, ostream &out_stream = cout) {
    for (const bool check_result: check_results) {
        out_stream << (check_result ? "Good" : "Bad") << "\n";
    }
}

void Test6() {
    {
        stringstream out;
        PrintCheckResults({true, false}, out);
        AssertEqual(out.str(), "Good\nBad\n");
    }
}

void TestAll() {
    TestRunner testRunner;
    RUN_TEST(testRunner, Test1);
    RUN_TEST(testRunner, Test2);
    RUN_TEST(testRunner, Test3);
    RUN_TEST(testRunner, Test4);
    RUN_TEST(testRunner, Test5);
    RUN_TEST(testRunner, Test6);
    RUN_TEST(testRunner, Test7);
}

int main() {
    TestAll();

    const vector<Domain> banned_domains = ReadDomains();
    const vector<Domain> domains_to_check = ReadDomains();
    PrintCheckResults(CheckDomains(banned_domains, domains_to_check));
    return 0;
}
