#include <iostream>
#include <map>
#include <tuple>
#include <vector>

using namespace std;

enum class Lang {
    DE, FR, IT
};

struct Region {
    string std_name;
    string parent_std_name;
    map<Lang, string> names;
    int64_t population;
};

string getRegionTranslations(const Region &r) {
    string s;
    for (auto i = Lang::DE; i <= Lang::IT;
         i = static_cast<Lang>(static_cast<int>(i) + 1)) {
        if (r.names.find(i) != r.names.end()) {
            s += r.names.at(i);
        }
        s += '|';
    }
    return s;
}

bool operator<(const Region &r1, const Region &r2) {
    auto r1_translation = getRegionTranslations(r1);
    auto r2_translation = getRegionTranslations(r2);
    auto t1 = tie(r1.std_name, r1.parent_std_name, r1.population, r1_translation);
    auto t2 = tie(r2.std_name, r2.parent_std_name, r2.population, r2_translation);
    return t1 < t2;
}

bool operator!=(const Region &r1, const Region &r2) {
    auto r1_translation = getRegionTranslations(r1);
    auto r2_translation = getRegionTranslations(r2);
    auto t1 = tie(r1.std_name, r1.parent_std_name, r1.population, r1_translation);
    auto t2 = tie(r2.std_name, r2.parent_std_name, r2.population, r2_translation);
    return t1 != t2;
}

int FindMaxRepetitionCount(const vector<Region> &regions) {
    if (regions.empty()) {
        return 0;
    }

    auto sortedRegions = regions;
    sort(sortedRegions.begin(), sortedRegions.end());

    int sameCnt = 1;
    int sameMaxCnt = 1;
    for (int i = 1; i < sortedRegions.size(); i++) {
        if (sortedRegions[i] != sortedRegions[i - 1]) {
            sameCnt = 1;
        } else {
            sameCnt++;
        }
        sameMaxCnt = max(sameMaxCnt, sameCnt);
    }

    return sameMaxCnt;
}

int main() {
    cout
            << FindMaxRepetitionCount({
                                              {"Moscow",
                                                      "Russia",
                                                      {{Lang::DE, "Moskau"}, {Lang::FR, "Moscou"}, {Lang::IT, "Mosca"}},
                                                      89},
                                              {"Russia",
                                                      "Eurasia",
                                                      {{Lang::DE, "Russland"},
                                                                             {Lang::FR, "Russie"},
                                                                                                   {Lang::IT, "Russia"}},
                                                      89},
                                              {"Moscow",
                                                      "Russia",
                                                      {{Lang::DE, "Moskau"}, {Lang::FR, "Moscou"}, {Lang::IT, "Mosca"}},
                                                      89},
                                              {"Moscow",
                                                      "Russia",
                                                      {{Lang::DE, "Moskau"}, {Lang::FR, "Moscou"}, {Lang::IT, "Mosca"}},
                                                      89},
                                              {"Russia",
                                                      "Eurasia",
                                                      {{Lang::DE, "Russland"},
                                                                             {Lang::FR, "Russie"},
                                                                                                   {Lang::IT, "Russia"}},
                                                      89},
                                      })
            << endl;

    cout
            << FindMaxRepetitionCount({
                                              {"Moscow",
                                                      "Russia",
                                                      {{Lang::DE, "Moskau"}, {Lang::FR, "Moscou"}, {Lang::IT, "Mosca"}},
                                                      89},
                                              {"Russia",
                                                      "Eurasia",
                                                      {{Lang::DE, "Russland"},
                                                                             {Lang::FR, "Russie"},
                                                                                                   {Lang::IT, "Russia"}},
                                                      89},
                                              {"Moscow",
                                                      "Russia",
                                                      {{Lang::DE, "Moskau"},
                                                                             {Lang::FR, "Moscou deux"},
                                                                                                   {Lang::IT, "Mosca"}},
                                                      89},
                                              {"Moscow",
                                                      "Toulouse",
                                                      {{Lang::DE, "Moskau"}, {Lang::FR, "Moscou"}, {Lang::IT, "Mosca"}},
                                                      89},
                                              {"Moscow",
                                                      "Russia",
                                                      {{Lang::DE, "Moskau"}, {Lang::FR, "Moscou"}, {Lang::IT, "Mosca"}},
                                                      31},
                                      })
            << endl;

    return 0;
}
