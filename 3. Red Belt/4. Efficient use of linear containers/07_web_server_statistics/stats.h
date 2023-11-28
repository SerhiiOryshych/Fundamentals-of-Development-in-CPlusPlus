#pragma once

#include "http_request.h"

#include <string_view>
#include <string>
#include <array>
#include <map>

using namespace std;

class Stats {
public:
    Stats();

    void AddMethod(string_view method);

    void AddUri(string_view uri);

    [[nodiscard]] const map<string_view, int> &GetMethodStats() const;

    [[nodiscard]] const map<string_view, int> &GetUriStats() const;

private:
    const array<string, 5> methods{"GET", "POST", "PUT", "DELETE", "UNKNOWN"};
    const array<string, 6> uris{"/", "/order", "/product", "/basket", "/help", "unknown"};

    map<string_view, int> method_stats;
    map<string_view, int> uri_stats;
};

HttpRequest ParseRequest(string_view line);
