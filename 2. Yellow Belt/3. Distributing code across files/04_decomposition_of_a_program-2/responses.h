#pragma once

#include <map>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

struct BusesForStopResponse {
    vector<string> buses;
};

ostream &operator<<(ostream &os, const BusesForStopResponse &r);

struct StopsForBusResponse {
    vector<string> stops;
    map<string, vector<string>> interchanges;
};

ostream &operator<<(ostream &os, const StopsForBusResponse &r);

struct AllBusesResponse {
    map<string, vector<string>> buses_to_stops;
};

ostream &operator<<(ostream &os, const AllBusesResponse &r);
