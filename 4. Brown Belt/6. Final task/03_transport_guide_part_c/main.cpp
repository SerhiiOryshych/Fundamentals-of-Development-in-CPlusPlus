#include "test_runner.h"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <unordered_map>

using namespace std;

enum QueryType {
    STOP, BUS_ROUTE, BUS_INFO, STOP_INFO
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

struct Edge {
    double len;
    string to;
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

        if (stream >> ch) {
            int len;
            while (stream >> len) {
                string to_name;
                stream >> ch >> word;
                while (stream >> word) {
                    if (!to_name.empty()) {
                        to_name += ' ';
                    }

                    if (word.back() == ',') {
                        to_name += word.substr(0, word.size() - 1);
                        break;
                    } else {
                        to_name += word;
                    }
                }

                if (!to_name.empty()) {
                    edges.push_back({(double) len, to_name});
                }
            }
        }

        name = std::move(name_raw);
        point = {lat, lng};
    }

    ostream &operator<<(ostream &str) const override {
        str << GetType() << " " << name << endl;
        str << "....(" << point.lat << "," << point.lng << ")" << endl;
        for (const auto &edge: edges) {
            str << "- " << edge.to << " " << edge.len << endl;
        }
        return str;
    }

    [[nodiscard]] string GetName() const { return name; }

    [[nodiscard]] LatLng GetPoint() const { return point; }

    [[nodiscard]] vector<Edge> GetEdges() const { return edges; }

private:
    string name;
    LatLng point{};
    vector<Edge> edges;
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

    [[nodiscard]] bool IsCircular() const { return is_circular; }

private:
    string name;
    vector<string> stops;
    bool is_circular = false;
};

bool operator==(const BusRouteQuery &q1, const BusRouteQuery &q2) {
    return q1.GetName() == q2.GetName() && q1.GetStops() == q2.GetStops();
}

class BusInfoQuery : public Query {
public:
    explicit BusInfoQuery(string name)
            : Query(QueryType::BUS_INFO), name(std::move(name)) {}

    explicit BusInfoQuery(stringstream &stream, int query_number)
            : Query(QueryType::BUS_INFO), query_index(query_number) {
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

    [[nodiscard]] int GetQueryIndex() const { return query_index; }

private:
    string name;
    int query_index;
};

bool operator==(const BusInfoQuery &q1, const BusInfoQuery &q2) {
    return q1.GetName() == q2.GetName();
}

class StopInfoQuery : public Query {
public:
    explicit StopInfoQuery(string name)
            : Query(QueryType::STOP_INFO), name(std::move(name)) {}

    explicit StopInfoQuery(stringstream &stream, int query_number)
            : Query(QueryType::STOP_INFO), query_index(query_number) {
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

    [[nodiscard]] int GetQueryIndex() const { return query_index; }

private:
    string name;
    int query_index;
};

bool operator==(const StopInfoQuery &q1, const StopInfoQuery &q2) {
    return q1.GetName() == q2.GetName();
}

struct Queries {
    vector<shared_ptr<StopQuery>> stop;
    vector<shared_ptr<BusRouteQuery>> bus_route;
    vector<shared_ptr<BusInfoQuery>> bus_info;
    vector<shared_ptr<StopInfoQuery>> stop_info;
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
            queries.bus_info.push_back(
                    make_shared<BusInfoQuery>(BusInfoQuery(stream, i)));
        } else if (type == "Stop") {
            queries.stop_info.push_back(
                    make_shared<StopInfoQuery>(StopInfoQuery(stream, i)));
        }
    }

    return queries;
}

void TestReadQueries() {
    {
        stringstream str(
                "13\n"
                "Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino\n"
                "Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka\n"
                "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo "
                "Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
                "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n"
                "Stop Rasskazovka: 55.632761, 37.333324\n"
                "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to "
                "Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam\n"
                "Stop Biryusinka: 55.581065, 37.64839, 750m to Universam\n"
                "Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, "
                "900m to Biryulyovo Tovarnaya\n"
                "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo "
                "Passazhirskaya\n"
                "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to "
                "Biryulyovo Zapadnoye\n"
                "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > "
                "Biryulyovo Zapadnoye\n"
                "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757\n"
                "Stop Prazhskaya: 55.611678, 37.603831\n"
                "6\n"
                "Bus 256\n"
                "Bus 750\n"
                "Bus 751\n"
                "Stop Samara\n"
                "Stop Prazhskaya\n"
                "Stop Biryulyovo Zapadnoye");
        auto queries = ReadQueries(str);
        Assert(queries.stop.size() == 10, "TestReadQueries_0");
        for (const auto &q: queries.stop) {
            q->operator<<(cout);
            cout << endl;
        }
        Assert(queries.bus_route.size() == 3, "TestReadQueries_1");
        for (const auto &q: queries.bus_route) {
            q->operator<<(cout);
            cout << endl;
        }
        Assert(queries.bus_info.size() == 3, "TestReadQueries_2");
        for (const auto &q: queries.bus_info) {
            q->operator<<(cout);
            cout << endl;
        }
        Assert(queries.stop_info.size() == 3, "TestReadQueries_2");
        for (const auto &q: queries.stop_info) {
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
        double route_real_length;
        double curvature;
    };

    void AddStop(const StopQuery &query) {
        stops[query.GetName()] = query.GetPoint();
        for (const auto &to: query.GetEdges()) {
            edge_len[{query.GetName(), to.to}] = to.len;
        }
    }

    void AddBusRoute(const BusRouteQuery &query) {
        set<string> unique_stops;
        double length_geo = 0;
        double length_real = 0;
        int route_stops_cnt = 0;
        if (query.IsCircular()) {
            auto route = query.GetStops();
            route_stops_cnt = route.size();
            for (int i = 0; i < route.size() - 1; i++) {
                unique_stops.insert(route[i]);
                length_geo += calculateDistance(stops[route[i]], stops[route[i + 1]]);
                length_real += GetRealDistance(route[i], route[i + 1]);
            }
        } else {
            auto route = query.GetStops();
            route_stops_cnt = route.size() * 2 - 1;
            for (int i = 0; i < route.size() - 1; i++) {
                unique_stops.insert(route[i]);
                length_geo += calculateDistance(stops[route[i]], stops[route[i + 1]]);
                length_real += GetRealDistance(route[i], route[i + 1]);
            }
            for (int i = route.size() - 1; i > 0; i--) {
                unique_stops.insert(route[i]);
                length_geo += calculateDistance(stops[route[i]], stops[route[i - 1]]);
                length_real += GetRealDistance(route[i], route[i - 1]);
            }
        }
        bus_routes[query.GetName()] = {route_stops_cnt, (int) unique_stops.size(),
                                       length_real, length_real / length_geo};
        for (const auto &s: unique_stops) {
            stop_buses[s].insert(query.GetName());
        }
    }

    void GetBusInfo(const BusInfoQuery &query) {
        stringstream str("");
        if (bus_routes.count(query.GetName())) {
            auto &route_info = bus_routes.at(query.GetName());
            str << "Bus " << query.GetName() << ": ";
            str << route_info.stops_cnt << " stops on route, ";
            str << route_info.unique_stops_cnt << " unique stops, ";
            str << std::setprecision(6) << route_info.route_real_length
                << " route length, ";
            str << std::setprecision(6) << route_info.curvature << " curvature";
        } else {
            str << "Bus " << query.GetName() << ": not found";
        }
        responses[query.GetQueryIndex()] = str.str();
    }

    void GetStopInfo(const StopInfoQuery &query) {
        stringstream str("");
        str << "Stop " << query.GetName() << ": ";
        if (stops.count(query.GetName())) {
            if (stop_buses.count(query.GetName()) == 0) {
                str << "no buses";
            } else {
                auto &buses = stop_buses.at(query.GetName());
                if (buses.empty()) {
                    str << "no buses";
                } else {
                    str << "buses ";
                    for (const auto &bus: buses) {
                        str << bus << " ";
                    }
                }
            }
        } else {
            str << "not found";
        }

        responses[query.GetQueryIndex()] = str.str();
    }

    void PrintResponses() const {
        for (int i = 0; i < responses.size(); i++) {
            cout << responses.at(i) << endl;
        }
    }

private:
    unordered_map<string, LatLng> stops;
    map<pair<string, string>, double> edge_len;
    unordered_map<string, set<string>> stop_buses;
    unordered_map<string, BusRoute> bus_routes;
    unordered_map<int, string> responses;

    double GetRealDistance(const string &from, const string &to) {
        if (edge_len.count({from, to})) {
            return edge_len.at({from, to});
        }
        if (edge_len.count({to, from})) {
            return edge_len.at({to, from});
        }
        return 0.0;
    }
};

void RunAllTests() {
    TestRunner tr;
    RUN_TEST(tr, TestReadQueries);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // RunAllTests();

    auto queries = ReadQueries();

    DbManager manager;
    for (const auto &q: queries.stop) {
        manager.AddStop(*q);
    }
    for (const auto &q: queries.bus_route) {
        manager.AddBusRoute(*q);
    }
    for (const auto &q: queries.bus_info) {
        manager.GetBusInfo(*q);
    }
    for (const auto &q: queries.stop_info) {
        manager.GetStopInfo(*q);
    }

    manager.PrintResponses();
}
