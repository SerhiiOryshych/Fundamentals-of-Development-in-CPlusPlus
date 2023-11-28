#include "bus_manager.h"

void BusManager::AddBus(const string &bus, const vector<string> &stops) {
    buses_to_stops[bus] = stops;
    for (const auto &stop: stops) {
        stops_to_buses[stop].push_back(bus);
    }
}

BusesForStopResponse BusManager::GetBusesForStop(const string &stop) const {
    if (stops_to_buses.count(stop) == 0) {
        return {{}};
    } else {
        return {stops_to_buses.at(stop)};
    }
}

StopsForBusResponse BusManager::GetStopsForBus(const string &bus) const {
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

AllBusesResponse BusManager::GetAllBuses() const {
    return {buses_to_stops};
}
