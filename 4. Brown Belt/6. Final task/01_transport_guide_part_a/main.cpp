#include "test_runner.h"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <unordered_map>

using namespace std;

enum QueryType {
    STOP, BUS_ROUTE, BUS_QUERY
};

class Query {
public:
    explicit Query(const QueryType &query_type) : type(query_type) {}

    virtual ostream &operator<<(ostream &str) const = 0;

    [[nodiscard]] virtual QueryType GetType() const { return type; }

private:
    QueryType type;
};

struct LatLng {
    double lat, lng;
};

bool operator==(const LatLng &p1, const LatLng &p2) {
    return p1.lat == p2.lat && p1.lng == p2.lng;
}

class StopQuery : public Query {
public:
    StopQuery(string name, LatLng point)
            : Query(QueryType::STOP), name(std::move(name)), point(point) {}

    explicit StopQuery(stringstream &stream) : Query(QueryType::STOP) {
        string name_raw;
        string word;
        while (stream >> word) {
            if (!name_raw.empty()) {
                name_raw += ' ';
            }
            if (word.back() == ':') {
                name_raw += word.substr(0, word.size() - 1);
                break;
            } else {
                name_raw += word;
            }
        }

        char ch;
        double lat, lng;
        stream >> lat >> ch >> lng;

        name = std::move(name_raw);
        point = {lat, lng};
    }

    ostream &operator<<(ostream &str) const override {
        str << GetType() << " " << name << endl;
        str << "....(" << point.lat << "," << point.lng << ")" << endl;
        return str;
    }

    [[nodiscard]] string GetName() const { return name; }

    [[nodiscard]] LatLng GetPoint() const { return point; }

private:
    string name;
    LatLng point{};
};

class BusRouteQuery : public Query {
public:
    BusRouteQuery(string name, vector<string> stops)
            : Query(QueryType::BUS_ROUTE), name(std::move(name)), stops(std::move(stops)) {}

    explicit BusRouteQuery(stringstream &stream) : Query(QueryType::BUS_ROUTE) {
        string name_raw;
        string word;
        while (stream >> word) {
            if (!name_raw.empty()) {
                name_raw += ' ';
            }
            if (word.back() == ':') {
                name_raw += word.substr(0, word.size() - 1);
                break;
            } else {
                name_raw += word;
            }
        }

        string stop_name;
        while (stream >> word) {
            if (word != ">" && word != "-") {
                if (!stop_name.empty()) {
                    stop_name += ' ';
                }
                stop_name += word;
            } else {
                if (!stop_name.empty()) {
                    stops.push_back(stop_name);
                    stop_name = "";
                }
                if (word == ">") {
                    is_circular = true;
                }
            }
        }
        if (!stop_name.empty()) {
            stops.push_back(stop_name);
        }

        name = std::move(name_raw);
    }

    ostream &operator<<(ostream &str) const override {
        str << GetType() << " " << name << endl;
        str << "stops: " << endl;
        for (const auto &st: stops) {
            str << "...." << st << endl;
        }
        return str;
    }

    [[nodiscard]] string GetName() const { return name; }

    [[nodiscard]] vector<string> GetStops() const { return stops; }

    bool IsCircular() const { return is_circular; }

private:
    string name;
    vector<string> stops;
    bool is_circular = false;
};

bool operator==(const BusRouteQuery &q1, const BusRouteQuery &q2) {
    return q1.GetName() == q2.GetName() && q1.GetStops() == q2.GetStops();
}

class BusQuery : public Query {
public:
    explicit BusQuery(string name)
            : Query(QueryType::BUS_QUERY), name(std::move(name)) {}

    explicit BusQuery(stringstream &stream) : Query(QueryType::BUS_QUERY) {
        name = "";
        string word;
        while (stream >> word) {
            if (!name.empty()) {
                name += ' ';
            }
            name += word;
        }
    }

    ostream &operator<<(ostream &str) const override {
        return str << GetType() << " " << name << endl;
    }

    [[nodiscard]] string GetName() const { return name; }

private:
    string name;
};

bool operator==(const BusQuery &q1, const BusQuery &q2) {
    return q1.GetName() == q2.GetName();
}

struct Queries {
    vector<shared_ptr<StopQuery>> stop;
    vector<shared_ptr<BusRouteQuery>> bus_route;
    vector<shared_ptr<BusQuery>> bus;
};

Queries ReadQueries(istream &in = cin) {
    Queries queries;

    string line;
    int n;
    in >> n;
    getline(in, line);
    for (int i = 0; i < n; i++) {
        getline(in, line);

        stringstream stream(line);
        string type;
        stream >> type;
        if (type == "Stop") {
            queries.stop.push_back(make_shared<StopQuery>(StopQuery(stream)));
        } else if (type == "Bus") {
            queries.bus_route.push_back(
                    make_shared<BusRouteQuery>(BusRouteQuery(stream)));
        }
    }

    int m;
    in >> m;
    getline(in, line);
    for (int i = 0; i < m; i++) {
        getline(in, line);

        stringstream stream(line);
        string type;
        stream >> type;
        if (type == "Bus") {
            queries.bus.push_back(make_shared<BusQuery>(BusQuery(stream)));
        }
    }

    return queries;
}

void TestReadQueries() {
    {
        stringstream str(
                "10\n"
                "Stop Tolstopaltsevo: 55.611087, 37.20829\n"
                "Stop Marushkino: 55.595884, 37.209755\n"
                "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo "
                "Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
                "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n"
                "Stop Rasskazovka: 55.632761, 37.333324\n"
                "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517\n"
                "Stop Biryusinka: 55.581065, 37.64839\n"
                "Stop Universam: 55.587655, 37.645687\n"
                "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656\n"
                "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164\n"
                "3\n"
                "Bus 256\n"
                "Bus 750\n"
                "Bus 751");
        auto queries = ReadQueries(str);
        Assert(queries.stop.size() == 8, "TestReadQueries_0");
        for (const auto &q: queries.stop) {
            q->operator<<(cout);
            cout << endl;
        }
        Assert(queries.bus_route.size() == 2, "TestReadQueries_1");
        for (const auto &q: queries.bus_route) {
            q->operator<<(cout);
            cout << endl;
        }
        Assert(queries.bus.size() == 3, "TestReadQueries_2");
        for (const auto &q: queries.bus) {
            q->operator<<(cout);
            cout << endl;
        }
    }
}

const double PI = 3.1415926535;
const double EARTH_RADIUS_M = 6371.0 * 1000;

double toRadians(double degrees) { return degrees * PI / 180.0; }

double calculateDistance(const LatLng &p1, const LatLng &p2) {
    double lat1 = p1.lat;
    double lon1 = p1.lng;
    double lat2 = p2.lat;
    double lon2 = p2.lng;

    double lat1Rad = toRadians(lat1);
    double lon1Rad = toRadians(lon1);
    double lat2Rad = toRadians(lat2);
    double lon2Rad = toRadians(lon2);

    double deltaLat = lat2Rad - lat1Rad;
    double deltaLon = lon2Rad - lon1Rad;

    double a = pow(sin(deltaLat / 2), 2) +
               cos(lat1Rad) * cos(lat2Rad) * pow(sin(deltaLon / 2), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    double distanceKm = EARTH_RADIUS_M * c;
    return distanceKm;
}

class DbManager {
public:
    struct BusRoute {
        int stops_cnt;
        int unique_stops_cnt;
        double route_length;
    };

    void AddStop(const StopQuery &query) {
        stops[query.GetName()] = query.GetPoint();
    }

    void AddBusRoute(const BusRouteQuery &query) {
        set<string> unique_stops;
        double length = 0;
        int route_stops_cnt = 0;
        if (query.IsCircular()) {
            auto route = query.GetStops();
            route_stops_cnt = route.size();
            for (int i = 0; i < route.size() - 1; i++) {
                unique_stops.insert(route[i]);
                length += calculateDistance(stops[route[i]], stops[route[i + 1]]);
            }
        } else {
            auto route = query.GetStops();
            route_stops_cnt = route.size() * 2 - 1;
            for (int i = 0; i < route.size() - 1; i++) {
                unique_stops.insert(route[i]);
                length += calculateDistance(stops[route[i]], stops[route[i + 1]]);
            }
            for (int i = route.size() - 1; i > 0; i--) {
                unique_stops.insert(route[i]);
                length += calculateDistance(stops[route[i]], stops[route[i - 1]]);
            }
        }
        bus_routes[query.GetName()] = {route_stops_cnt, (int) unique_stops.size(),
                                       length};
    }

    void GetBusInfo(const BusQuery &query, ostream &str = cout) const {
        if (bus_routes.count(query.GetName())) {
            auto &route_info = bus_routes.at(query.GetName());
            str << "Bus " << query.GetName() << ": ";
            str << route_info.stops_cnt << " stops on route, ";
            str << route_info.unique_stops_cnt << " unique stops, ";
            str << std::setprecision(6) << route_info.route_length << " route length"
                << endl;
        } else {
            cout << "Bus " << query.GetName() << ": not found" << endl;
        }
    }

private:
    unordered_map<string, LatLng> stops;
    unordered_map<string, BusRoute> bus_routes;
};

void RunAllTests() {
    TestRunner tr;
    RUN_TEST(tr, TestReadQueries);
}

int main() {
    // RunAllTests();

    auto queries = ReadQueries();

    DbManager manager;
    for (const auto &q: queries.stop) {
        manager.AddStop(*q);
    }
    for (const auto &q: queries.bus_route) {
        manager.AddBusRoute(*q);
    }
    for (const auto &q: queries.bus) {
        manager.GetBusInfo(*q);
    }
}
