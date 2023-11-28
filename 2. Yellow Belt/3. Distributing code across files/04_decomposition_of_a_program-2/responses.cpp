#include "responses.h"

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