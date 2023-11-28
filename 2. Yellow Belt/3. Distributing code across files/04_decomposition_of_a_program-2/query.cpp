#include "query.h"

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
