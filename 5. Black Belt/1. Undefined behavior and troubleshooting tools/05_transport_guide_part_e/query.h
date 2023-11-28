#pragma once

#include "json.h"
#include "types.h"
#include <string>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

struct Edge {
    double len;
    string to;
};

enum QueryType {
    STOP, BUS_ROUTE, BUS_INFO, STOP_INFO, ROUTE_INFO
};

class Query {
public:
    explicit Query(const QueryType &query_type) : type(query_type) {}

    virtual ostream &operator<<(ostream &str) const = 0;

    [[nodiscard]] virtual QueryType GetType() const { return type; }

private:
    QueryType type;
};

class StopQuery : public Query {
public:
    StopQuery(string name, LatLng point, vector<Edge> edges)
            : Query(QueryType::STOP), name(move(name)), point(point),
              edges(move(edges)) {}

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
    BusRouteQuery(string name, vector<string> stops, bool is_roundtrip)
            : Query(QueryType::BUS_ROUTE), name(move(name)), stops(move(stops)),
              is_roundtrip(is_roundtrip) {}

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

    [[nodiscard]] bool IsCircular() const { return is_roundtrip; }

private:
    string name;
    vector<string> stops;
    bool is_roundtrip = false;
};

bool operator==(const BusRouteQuery &q1, const BusRouteQuery &q2) {
    return q1.GetName() == q2.GetName() && q1.GetStops() == q2.GetStops();
}

class BusInfoQuery : public Query {
public:
    explicit BusInfoQuery(string name, int index)
            : Query(QueryType::BUS_INFO), name(move(name)), query_index(index) {}

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
    explicit StopInfoQuery(string name, int index)
            : Query(QueryType::STOP_INFO), name(move(name)), query_index(index) {}

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

class RouteInfoQuery : public Query {
public:
    explicit RouteInfoQuery(string from, string to, int index)
            : Query(QueryType::ROUTE_INFO), from(move(from)), to(move(to)),
              query_index(index) {}

    ostream &operator<<(ostream &str) const override {
        return str << "route with id = " << query_index << " from " << from
                   << " to " << to << endl;
    }

    [[nodiscard]] string GetFrom() const { return from; }

    [[nodiscard]] string GetTo() const { return to; }

    [[nodiscard]] int GetQueryIndex() const { return query_index; }

private:
    string from;
    string to;
    int query_index;
};

bool operator==(const RouteInfoQuery &q1, const RouteInfoQuery &q2) {
    return q1.GetFrom() == q2.GetFrom() && q1.GetTo() == q2.GetTo();
}

struct RoutingSettings {
    RoutingSettings(double wait_time, double velocity)
            : bus_wait_time(wait_time), bus_velocity(velocity) {}

    double bus_wait_time;
    double bus_velocity;

    ostream &operator<<(ostream &out) const {
        out << "routing_settings:" << endl;
        out << "    bus_wait_time: " << this->bus_wait_time << endl;
        out << "    bus_velocity: " << this->bus_velocity << endl;
        return out;
    }
};

struct Queries {
    vector<unique_ptr<StopQuery>> stop;
    vector<unique_ptr<BusRouteQuery>> bus;
    vector<unique_ptr<BusInfoQuery>> bus_info;
    vector<unique_ptr<StopInfoQuery>> stop_info;
    vector<unique_ptr<RouteInfoQuery>> route_info;
    unique_ptr<RoutingSettings> routing_settings;
};

Queries ParseQueriesFromJson(istream &in = cin) {
    Json::Document document = Json::Load(in);

    Queries queries;

    if (document.GetRoot().AsMap().count("base_requests")) {
        auto base_requests =
                document.GetRoot().AsMap().at("base_requests").AsArray();

        for (const auto &request: base_requests) {
            const auto &request_as_map = request.AsMap();
            const auto &type = request_as_map.at("type").AsString();
            if (type == "Stop") {
                const auto &name = request_as_map.at("name").AsString();
                const auto &lat = request_as_map.at("latitude").AsDouble();
                const auto &lng = request_as_map.at("longitude").AsDouble();

                vector<Edge> edges;
                if (request_as_map.count("road_distances")) {
                    const auto &road_distances =
                            request_as_map.at("road_distances").AsMap();
                    for (const auto &it: road_distances) {
                        const auto &stop_name = it.first;
                        const auto len = it.second.AsDouble();
                        edges.push_back({len, stop_name});
                    }
                }
                queries.stop.push_back(
                        make_unique<StopQuery>(name, LatLng({lat, lng}), edges));
            } else if (type == "Bus") {
                const auto &name = request_as_map.at("name").AsString();
                const auto is_roundtrip = request_as_map.at("is_roundtrip").AsBool();

                vector<string> stops;
                if (request_as_map.count("stops")) {
                    const auto &bus_stops = request_as_map.at("stops").AsArray();
                    for (const auto &s: bus_stops) {
                        stops.push_back(s.AsString());
                    }
                }
                queries.bus.push_back(
                        make_unique<BusRouteQuery>(name, move(stops), is_roundtrip));
            }
        }
    }

    if (document.GetRoot().AsMap().count("stat_requests")) {
        auto stat_requests =
                document.GetRoot().AsMap().at("stat_requests").AsArray();
        for (const auto &request: stat_requests) {
            const auto &request_as_map = request.AsMap();
            const auto &type = request_as_map.at("type").AsString();
            if (type == "Stop") {
                const int id = request_as_map.at("id").AsDouble();
                const auto &name = request_as_map.at("name").AsString();
                queries.stop_info.push_back(make_unique<StopInfoQuery>(name, id));
            } else if (type == "Bus") {
                const int id = request_as_map.at("id").AsDouble();
                const auto &name = request_as_map.at("name").AsString();
                queries.bus_info.push_back(make_unique<BusInfoQuery>(name, id));
            } else if (type == "Route") {
                const int id = request_as_map.at("id").AsDouble();
                const auto &from = request_as_map.at("from").AsString();
                const auto &to = request_as_map.at("to").AsString();
                queries.route_info.push_back(make_unique<RouteInfoQuery>(from, to, id));
            }
        }
    }

    if (document.GetRoot().AsMap().count("routing_settings")) {
        auto routing_settings =
                document.GetRoot().AsMap().at("routing_settings").AsMap();
        const int bus_wait_time = routing_settings.at("bus_wait_time").AsDouble();
        const int bus_velocity = routing_settings.at("bus_velocity").AsDouble();
        queries.routing_settings =
                make_unique<RoutingSettings>(bus_wait_time, bus_velocity);
    }

    return queries;
}
