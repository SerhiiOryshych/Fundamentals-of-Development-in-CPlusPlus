#include "database.h"
#include <vector>

using namespace std;

Database::Database() {}

void Database::Add(const Date &date, const string &event) {
    if (date_event_set_.count({date, event})) {
        return;
    }

    date_event_set_.insert({date, event});
    date_events_map_[date].push_back(event);
}

void Database::Print(ostream &os) const {
    for (const auto &[date, events]: date_events_map_) {
        for (const auto &event: events) {
            os << date << " " << event << endl;
        }
    }
}

int Database::RemoveIf(function<bool(Date, std::string)> predicate) {
    int cnt = 0;
    map<Date, vector<string>> date_events_map_copy;
    for (auto &[date, events]: date_events_map_) {
        auto it_stable_partition = stable_partition(events.begin(), events.end(),
                                                    [predicate, date = date](const string &event) {
                                                        return !predicate(date, event);
                                                    });
        for (auto it = it_stable_partition; it != events.end(); it++) {
            date_event_set_.erase({date, *it});
            cnt++;
        }

        if (events.begin() != it_stable_partition) {
            date_events_map_copy[date] = {events.begin(), it_stable_partition};
        }
    }
    date_events_map_ = date_events_map_copy;

    return cnt;
}

vector<pair<Date, string>> Database::FindIf(function<bool(Date, std::string)> predicate) const {
    vector<pair<Date, string>> response;
    for (const auto &[date, events]: date_events_map_) {
        for (const auto &event: events) {
            if (predicate(date, event)) {
                response.push_back({date, event});
            }
        }
    }
    return response;
}

string DateToString(const Date &date) {
    string year = to_string(date.GetYear());
    while (year.length() < 4) {
        year = '0' + year;
    }
    string month = to_string(date.GetMonth());
    while (month.length() < 2) {
        month = '0' + month;
    }
    string day = to_string(date.GetDay());
    while (day.length() < 2) {
        day = '0' + day;
    }
    return year + "-" + month + "-" + day;
}

string Database::Last(const Date &date) const {
    auto it = date_events_map_.upper_bound(date);
    if (it == date_events_map_.begin()) {
        return "No entries";
    }
    it--;

    return DateToString(it->first) + " " + date_events_map_.at(it->first)[date_events_map_.at(it->first).size() - 1];
}

ostream &operator<<(ostream &os, const Date date) {
    return os << DateToString(date);
}

ostream &operator<<(ostream &os, const pair<Date, string> &p) {
    return os << p.first << " " << p.second;
}
