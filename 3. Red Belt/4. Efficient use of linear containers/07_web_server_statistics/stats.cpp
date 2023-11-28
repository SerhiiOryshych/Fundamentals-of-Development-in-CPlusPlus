#include "http_request.h"
#include "stats.h"

#include <string_view>
#include <algorithm>
#include <vector>
#include <string>
#include <array>
#include <map>

using namespace std;

Stats::Stats() {
    for (int i = 0; i < 5; i++) {
        method_stats[string_view(methods[i])] = 0;
    }

    for (int i = 0; i < 6; i++) {
        uri_stats[string_view(uris[i])] = 0;
    }
}

void Stats::AddMethod(string_view method) {
    auto it = find(methods.begin(), methods.end(), string(method));
    if (it == methods.end()) {
        it = prev(methods.end());
    }
    method_stats[string_view(*it)]++;
}

void Stats::AddUri(string_view uri) {
    auto it = find(uris.begin(), uris.end(), string(uri));
    if (it == uris.end()) {
        it = prev(uris.end());
    }
    uri_stats[string_view(*it)]++;
}

const map<string_view, int> &Stats::GetMethodStats() const {
    return method_stats;
}

const map<string_view, int> &Stats::GetUriStats() const {
    return uri_stats;
}

HttpRequest ParseRequest(string_view line) {
    vector<string_view> result;

    size_t start = 0;
    while (line[start] == ' ') {
        start++;
    }
    size_t end = line.npos;
    while (start < end) {
        auto space = line.find(' ', start);
        result.push_back(line.substr(start, space - start));
        if (space == end) {
            break;
        }
        start = space + 1;
    }

    return {result[0], result[1], result[2]};
}
