#pragma once

#include "date.h"
#include <set>
#include <map>
#include <iostream>
#include <functional>

using namespace std;

class Database {
public:
    Database();

    void Add(const Date &date, const string &event);

    void Print(ostream &os) const;

    int RemoveIf(function<bool(Date, std::string)> predicate);

    vector<pair<Date, string>> FindIf(function<bool(Date, std::string)> predicate) const;

    [[nodiscard]] string Last(const Date &date) const;

private:
    set<pair<Date, string>> date_event_set_;
    map<Date, vector<string>> date_events_map_;
};

ostream &operator<<(ostream &os, const Date date);

ostream &operator<<(ostream &os, const pair<Date, string> &p);