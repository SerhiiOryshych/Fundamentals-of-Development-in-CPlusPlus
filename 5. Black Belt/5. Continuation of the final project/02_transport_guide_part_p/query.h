#pragma once

#include "json.h"
#include "types.h"
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace std;

struct Edge {
    double len;
    string to;
};

enum QueryType {
    STOP, BUS_ROUTE, BUS_INFO, STOP_INFO, ROUTE_INFO, MAP_INFO
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
            : Query(QueryType::STOP), name(std::move(name)), point(point),
              edges(std::move(edges)) {}

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
            : Query(QueryType::BUS_ROUTE), name(std::move(name)),
              stops(std::move(stops)), is_roundtrip(is_roundtrip) {}

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
            : Query(QueryType::BUS_INFO), name(std::move(name)), query_index(index) {}

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
            : Query(QueryType::STOP_INFO), name(std::move(name)), query_index(index) {
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

class RouteInfoQuery : public Query {
public:
    explicit RouteInfoQuery(string from, string to, int index)
            : Query(QueryType::ROUTE_INFO), from(std::move(from)), to(std::move(to)),
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

class MapInfoQuery : public Query {
public:
    explicit MapInfoQuery(int index)
            : Query(QueryType::MAP_INFO), query_index(index) {}

    ostream &operator<<(ostream &str) const override {
        return str << "route with id = " << query_index << endl;
    }

    [[nodiscard]] int GetQueryIndex() const { return query_index; }

private:
    int query_index;
};

bool operator==(const MapInfoQuery &q1, const MapInfoQuery &q2) {
    return q1.GetQueryIndex() == q2.GetQueryIndex();
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

struct RenderSettings {
    RenderSettings(double width, double height, double padding,
                   double stop_radius, double line_width,
                   int stop_label_font_size, Point stop_label_offset,
                   Color underlayer_color, double underlayer_width,
                   vector<Color> color_palette, int bus_label_font_size,
                   Point bus_label_offset, vector<string> layers,
                   double outer_margin)
            : width(width), height(height), padding(padding),
              stop_radius(stop_radius), line_width(line_width),
              stop_label_font_size(stop_label_font_size),
              stop_label_offset(stop_label_offset),
              underlayer_color(std::move(underlayer_color)),
              underlayer_width(underlayer_width),
              color_palette(std::move(color_palette)),
              bus_label_font_size(bus_label_font_size),
              bus_label_offset(bus_label_offset), layers(layers),
              outer_margin(outer_margin) {}

    double width;
    double height;
    double padding;
    double stop_radius;
    double line_width;
    int stop_label_font_size;
    Point stop_label_offset;
    Color underlayer_color;
    double underlayer_width;
    vector<Color> color_palette;
    int bus_label_font_size;
    Point bus_label_offset;
    vector<string> layers;
    double outer_margin;

    ostream &operator<<(ostream &out) const {
        out << "width: " << width << endl;
        out << "height: " << height << endl;
        out << "padding: " << padding << endl;
        out << "stop_radius: " << stop_radius << endl;
        out << "stop_label_font_size: " << stop_label_font_size << endl;
        out << "stop_label_offset: " << stop_label_offset.x << " "
            << stop_label_offset.y << endl;
        out << "underlayer_color: " << GetString(underlayer_color) << endl;
        out << "underlayer_width: " << underlayer_width << endl;
        out << "color_palette_size: " << color_palette.size() << endl;
        out << "outer_margin: " << outer_margin << endl;
        return out;
    }
};

struct SerializationSettings {
    SerializationSettings(string file_name) : file_name(file_name) {}

    string file_name;

    ostream &operator<<(ostream &out) const {
        out << "file: " << file_name << endl;
        return out;
    }
};

struct Queries {
    vector<unique_ptr<StopQuery>> stop;
    vector<unique_ptr<BusRouteQuery>> bus;
    vector<unique_ptr<BusInfoQuery>> bus_info;
    vector<unique_ptr<StopInfoQuery>> stop_info;
    vector<unique_ptr<RouteInfoQuery>> route_info;
    vector<unique_ptr<MapInfoQuery>> map_info;
    unique_ptr<RoutingSettings> routing_settings;
    unique_ptr<RenderSettings> render_settings;
    unique_ptr<SerializationSettings> serialization_settings;
};

Queries ParseBaseQueries(istream &in = cin) {
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
                        make_unique<BusRouteQuery>(name, std::move(stops), is_roundtrip));
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

    if (document.GetRoot().AsMap().count("render_settings")) {
        auto render_settings =
                document.GetRoot().AsMap().at("render_settings").AsMap();
        double width = render_settings.at("width").AsDouble();
        double height = render_settings.at("height").AsDouble();
        double padding = render_settings.at("padding").AsDouble();
        double stop_radius = render_settings.at("stop_radius").AsDouble();
        double lide_width = render_settings.at("line_width").AsDouble();
        int stop_label_font_size =
                render_settings.at("stop_label_font_size").AsDouble();
        double underlayer_width = render_settings.at("underlayer_width").AsDouble();
        int bus_label_font_size =
                render_settings.at("bus_label_font_size").AsDouble();
        const auto bus_label_offset_array =
                render_settings.at("bus_label_offset").AsArray();
        double outer_margin = render_settings.at("outer_margin").AsDouble();
        Point bus_label_offset = Point(bus_label_offset_array[0].AsDouble(),
                                       bus_label_offset_array[1].AsDouble());
        const auto stop_label_offset_array =
                render_settings.at("stop_label_offset").AsArray();
        Point stop_label_offset = Point(stop_label_offset_array[0].AsDouble(),
                                        stop_label_offset_array[1].AsDouble());

        Color underlayer_color;
        if (render_settings.at("underlayer_color").IsString()) {
            underlayer_color = {render_settings.at("underlayer_color").AsString()};
        } else {
            const auto &color_arr = render_settings.at("underlayer_color").AsArray();
            if (color_arr.size() == 3) {
                underlayer_color = Color{Rgb{(int) color_arr[0].AsDouble(),
                                             (int) color_arr[1].AsDouble(),
                                             (int) color_arr[2].AsDouble()}};
            } else {
                underlayer_color = Color{Rgba{
                        (int) color_arr[0].AsDouble(), (int) color_arr[1].AsDouble(),
                        (int) color_arr[2].AsDouble(), (double) color_arr[3].AsDouble()}};
            }
        }
        const auto color_palette_array =
                render_settings.at("color_palette").AsArray();
        vector<Color> color_palette;
        for (const auto &color: color_palette_array) {
            if (color.IsString()) {
                color_palette.emplace_back(color.AsString());
            } else {
                const auto &color_arr = color.AsArray();
                if (color_arr.size() == 3) {
                    color_palette.emplace_back(Color{Rgb{(int) color_arr[0].AsDouble(),
                                                         (int) color_arr[1].AsDouble(),
                                                         (int) color_arr[2].AsDouble()}});
                } else {
                    color_palette.emplace_back(Color{Rgba{
                            (int) color_arr[0].AsDouble(), (int) color_arr[1].AsDouble(),
                            (int) color_arr[2].AsDouble(), (double) color_arr[3].AsDouble()}});
                }
            }
        }
        const auto layers_array = render_settings.at("layers").AsArray();
        vector<string> layers;
        for (const auto &l: layers_array) {
            layers.push_back(l.AsString());
        }

        queries.render_settings = make_unique<RenderSettings>(
                width, height, padding, stop_radius, lide_width, stop_label_font_size,
                stop_label_offset, underlayer_color, underlayer_width, color_palette,
                bus_label_font_size, bus_label_offset, layers, outer_margin);
    }

    if (document.GetRoot().AsMap().count("serialization_settings")) {
        auto serialization_settings =
                document.GetRoot().AsMap().at("serialization_settings").AsMap();
        const string file_name = serialization_settings.at("file").AsString();
        queries.serialization_settings =
                make_unique<SerializationSettings>(file_name);
    }

    return queries;
}

Queries ParseProcessQueries(istream &in = cin) {
    Json::Document document = Json::Load(in);

    Queries queries;

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
            } else if (type == "Map") {
                const int id = request_as_map.at("id").AsDouble();
                queries.map_info.push_back(make_unique<MapInfoQuery>(id));
            }
        }
    }

    if (document.GetRoot().AsMap().count("serialization_settings")) {
        auto serialization_settings =
                document.GetRoot().AsMap().at("serialization_settings").AsMap();
        const string file_name = serialization_settings.at("file").AsString();
        queries.serialization_settings =
                make_unique<SerializationSettings>(file_name);
    }

    return queries;
}

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
                        make_unique<BusRouteQuery>(name, std::move(stops), is_roundtrip));
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
            } else if (type == "Map") {
                const int id = request_as_map.at("id").AsDouble();
                queries.map_info.push_back(make_unique<MapInfoQuery>(id));
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

    if (document.GetRoot().AsMap().count("render_settings")) {
        auto render_settings =
                document.GetRoot().AsMap().at("render_settings").AsMap();
        double width = render_settings.at("width").AsDouble();
        double height = render_settings.at("height").AsDouble();
        double padding = render_settings.at("padding").AsDouble();
        double stop_radius = render_settings.at("stop_radius").AsDouble();
        double lide_width = render_settings.at("line_width").AsDouble();
        int stop_label_font_size =
                render_settings.at("stop_label_font_size").AsDouble();
        double underlayer_width = render_settings.at("underlayer_width").AsDouble();
        int bus_label_font_size =
                render_settings.at("bus_label_font_size").AsDouble();
        const auto bus_label_offset_array =
                render_settings.at("bus_label_offset").AsArray();
        double outer_margin = render_settings.at("outer_margin").AsDouble();
        Point bus_label_offset = Point(bus_label_offset_array[0].AsDouble(),
                                       bus_label_offset_array[1].AsDouble());
        const auto stop_label_offset_array =
                render_settings.at("stop_label_offset").AsArray();
        Point stop_label_offset = Point(stop_label_offset_array[0].AsDouble(),
                                        stop_label_offset_array[1].AsDouble());

        Color underlayer_color;
        if (render_settings.at("underlayer_color").IsString()) {
            underlayer_color = {render_settings.at("underlayer_color").AsString()};
        } else {
            const auto &color_arr = render_settings.at("underlayer_color").AsArray();
            if (color_arr.size() == 3) {
                underlayer_color = Color{Rgb{(int) color_arr[0].AsDouble(),
                                             (int) color_arr[1].AsDouble(),
                                             (int) color_arr[2].AsDouble()}};
            } else {
                underlayer_color = Color{Rgba{
                        (int) color_arr[0].AsDouble(), (int) color_arr[1].AsDouble(),
                        (int) color_arr[2].AsDouble(), (double) color_arr[3].AsDouble()}};
            }
        }
        const auto color_palette_array =
                render_settings.at("color_palette").AsArray();
        vector<Color> color_palette;
        for (const auto &color: color_palette_array) {
            if (color.IsString()) {
                color_palette.emplace_back(color.AsString());
            } else {
                const auto &color_arr = color.AsArray();
                if (color_arr.size() == 3) {
                    color_palette.emplace_back(Color{Rgb{(int) color_arr[0].AsDouble(),
                                                         (int) color_arr[1].AsDouble(),
                                                         (int) color_arr[2].AsDouble()}});
                } else {
                    color_palette.emplace_back(Color{Rgba{
                            (int) color_arr[0].AsDouble(), (int) color_arr[1].AsDouble(),
                            (int) color_arr[2].AsDouble(), (double) color_arr[3].AsDouble()}});
                }
            }
        }
        const auto layers_array = render_settings.at("layers").AsArray();
        vector<string> layers;
        for (const auto &l: layers_array) {
            layers.push_back(l.AsString());
        }

        queries.render_settings = make_unique<RenderSettings>(
                width, height, padding, stop_radius, lide_width, stop_label_font_size,
                stop_label_offset, underlayer_color, underlayer_width, color_palette,
                bus_label_font_size, bus_label_offset, layers, outer_margin);
    }

    if (document.GetRoot().AsMap().count("serialization_settings")) {
        auto serialization_settings =
                document.GetRoot().AsMap().at("serialization_settings").AsMap();
        const string file_name = serialization_settings.at("file").AsString();
        queries.serialization_settings =
                make_unique<SerializationSettings>(file_name);
    }

    return queries;
}
