#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

enum class QueryType {
    NewBus, BusesForStop, StopsForBus, AllBuses
};

struct Query {
    QueryType type;
    string bus;
    string stop;
    vector<string> stops;
};

istream &operator>>(istream &is, Query &q) {
    string operation_type;
    is >> operation_type;

    if (operation_type == "NEW_BUS") {
        string bus;
        is >> bus;
        int stop_count;
        is >> stop_count;
        vector<string> stops(stop_count);
        for (auto &stop: stops) {
            is >> stop;
        }
        q = {QueryType::NewBus, bus, "", stops};
    } else if (operation_type == "BUSES_FOR_STOP") {
        string stop;
        is >> stop;
        q = {QueryType::BusesForStop, "", stop, {}};
    } else if (operation_type == "STOPS_FOR_BUS") {
        string bus;
        is >> bus;
        q = {QueryType::StopsForBus, bus, "", {}};
    } else if (operation_type == "ALL_BUSES") {
        q = {QueryType::AllBuses, "", "", {}};
    }

    return is;
}

struct BusesForStopResponse {
    vector<string> buses;
};

ostream &operator<<(ostream &os, const BusesForStopResponse &r) {
    if (r.buses.empty()) {
        os << "No stop" << endl;
    } else {
        for (const string &bus: r.buses) {
            os << bus << " ";
        }
        os << endl;
    }
    return os;
}

struct StopsForBusResponse {
    vector<string> stops;
    map<string, vector<string>> interchanges;
};

ostream &operator<<(ostream &os, const StopsForBusResponse &r) {
    if (r.stops.empty()) {
        os << "No bus" << endl;
    } else {
        for (const string &stop: r.stops) {
            os << "Stop " << stop << ": ";
            if (r.interchanges.at(stop).empty()) {
                os << "no interchange";
            } else {
                for (const string &interchange: r.interchanges.at(stop)) {
                    os << interchange << " ";
                }
            }
            os << endl;
        }
    }
    return os;
}

struct AllBusesResponse {
    map<string, vector<string>> buses_to_stops;
};

ostream &operator<<(ostream &os, const AllBusesResponse &r) {
    if (r.buses_to_stops.empty()) {
        os << "No buses" << endl;
    } else {
        for (const auto &bus_item: r.buses_to_stops) {
            os << "Bus " << bus_item.first << ": ";
            for (const string &stop: bus_item.second) {
                os << stop << " ";
            }
            os << endl;
        }
    }
    return os;
}

class BusManager {
public:
    void AddBus(const string &bus, const vector<string> &stops) {
        buses_to_stops[bus] = stops;
        for (const auto &stop: stops) {
            stops_to_buses[stop].push_back(bus);
        }
    }

    [[nodiscard]] BusesForStopResponse GetBusesForStop(const string &stop) const {
        if (stops_to_buses.count(stop) == 0) {
            return {{}};
        } else {
            return {stops_to_buses.at(stop)};
        }
    }

    [[nodiscard]] StopsForBusResponse GetStopsForBus(const string &bus) const {
        StopsForBusResponse r = {{},
                                 {}};
        if (buses_to_stops.count(bus) == 0) {
            return r;
        } else {
            for (const string &stop: buses_to_stops.at(bus)) {
                r.stops.push_back(stop);
                r.interchanges[stop] = {};
                if (stops_to_buses.at(stop).size() > 1) {
                    for (const string &other_bus: stops_to_buses.at(stop)) {
                        if (bus != other_bus) {
                            r.interchanges[stop].push_back(other_bus);
                        }
                    }
                }
            }
            return r;
        }
    }

    [[nodiscard]] AllBusesResponse GetAllBuses() const {
        return {buses_to_stops};
    }

private:
    map<string, vector<string>> buses_to_stops, stops_to_buses;
};

void correctSolution() {
    int query_count;
    Query q;

    cin >> query_count;

    BusManager bm;
    for (int i = 0; i < query_count; ++i) {
        cin >> q;
        switch (q.type) {
            case QueryType::NewBus:
                bm.AddBus(q.bus, q.stops);
                break;
            case QueryType::BusesForStop:
                cout << bm.GetBusesForStop(q.stop) << endl;
                break;
            case QueryType::StopsForBus:
                cout << bm.GetStopsForBus(q.bus) << endl;
                break;
            case QueryType::AllBuses:
                cout << bm.GetAllBuses() << endl;
                break;
        }
    }
}

int main() {
    correctSolution();
    return 0;
}