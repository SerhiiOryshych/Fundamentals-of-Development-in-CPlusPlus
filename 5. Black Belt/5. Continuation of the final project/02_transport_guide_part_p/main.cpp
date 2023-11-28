#include "graph.h"
#include "query.h"
#include "router.h"
#include "svg.h"
#include "test_runner.h"
#include "transport_catalog.pb.h"
#include "types.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <istream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

using namespace std;

unique_ptr<Graph::Router<double>> router;

template<>
struct hash<std::pair<std::string, std::string>> {
    size_t operator()(const std::pair<std::string, std::string> &p) const {
        // Combine the hash values of the two strings to create a unique hash
        size_t h1 = std::hash<std::string>{}(p.first);
        size_t h2 = std::hash<std::string>{}(p.second);
        return h1 ^ (h2 << 1); // Adjust as needed for your use case
    }
};

class DbManager {
private:
    struct RawEdge {
        string type;
        string name;
        double time;
        int span_count = 0;
        string from, to;
        double distance;
        int idx_start;
        int idx_end;
    };

public:
    struct BusRoute {
        int stops_cnt;
        int unique_stops_cnt;
        double route_real_length;
        double curvature;
        vector<string> stops;
        bool is_roundtrip;
    };

    struct CoordinateCompressionParams {
        map<double, int> lng_to_id;
        map<double, int> lat_to_id;
        double x_step;
        double y_step;
    };

    void AddStop(const StopQuery &query) {
        stops[query.GetName()] = query.GetPoint();
        for (const auto &to: query.GetEdges()) {
            edge_len[{query.GetName(), to.to}] = to.len;
            stop_road_distances[query.GetName()].emplace_back(to.to, to.len);
        }
        stop_buses[query.GetName()] = {};
    }

    void AddBusRoute(const BusRouteQuery &query) {
        set<string> unique_stops;
        double length_geo = 0;
        double length_real = 0;
        int route_stops_cnt;
        auto route = query.GetStops();
        if (query.IsCircular()) {
            route_stops_cnt = route.size();
            for (int i = 0; i < (int) route.size() - 1; i++) {
                unique_stops.insert(route[i]);
                length_geo += calculateDistance(stops[route[i]], stops[route[i + 1]]);
                double real_distance = GetRealDistance(route[i], route[i + 1]);
                if (real_distance == 0) {
                    real_distance = GetRealDistance(route[i + 1], route[i]);
                }
                length_real += real_distance;
            }
        } else {
            route_stops_cnt = route.size() * 2 - 1;
            for (int i = 0; i < (int) route.size() - 1; i++) {
                unique_stops.insert(route[i]);
                length_geo += calculateDistance(stops[route[i]], stops[route[i + 1]]);
                double real_distance = GetRealDistance(route[i], route[i + 1]);
                if (real_distance == 0) {
                    real_distance = GetRealDistance(route[i + 1], route[i]);
                }
                length_real += real_distance;
            }
            for (int i = (int) route.size() - 1; i > 0; i--) {
                unique_stops.insert(route[i]);
                length_geo += calculateDistance(stops[route[i]], stops[route[i - 1]]);
                double real_distance = GetRealDistance(route[i], route[i - 1]);
                if (real_distance == 0) {
                    real_distance = GetRealDistance(route[i - 1], route[i]);
                }
                length_real += real_distance;
            }
        }
        bus_routes[query.GetName()] = {route_stops_cnt, (int) unique_stops.size(),
                                       length_real, length_real / length_geo,
                                       route, query.IsCircular()};

        for (const auto &s: unique_stops) {
            stop_buses[s].insert(query.GetName());
        }
    }

    void
    AddBaseStopResponses(TransportCatalog::TransportCatalog &transportCatalog) {
        for (const auto &entry: stop_buses) {
            TransportCatalog::StopStatsResponse stopStatsResponse;

            stopStatsResponse.mutable_buses()->Reserve(entry.second.size());
            for (const auto &bus_name: entry.second) {
                *stopStatsResponse.mutable_buses()->Add() = bus_name;
            }

            for (const auto &road: stop_road_distances[entry.first]) {
                TransportCatalog::RoadDistance roadDistance;
                roadDistance.set_stop_name(road.first);
                roadDistance.set_distance(road.second);
                *stopStatsResponse.mutable_road_distances()->Add() = roadDistance;
            }

            if (stopStatsResponse.buses_size() == 0) {
                *stopStatsResponse.mutable_buses()->Add() = "";
            }
            stopStatsResponse.set_name(entry.first);
            stopStatsResponse.set_lat(stops[entry.first].lat);
            stopStatsResponse.set_lng(stops[entry.first].lng);
            *transportCatalog.mutable_stop_responses()->Add() = stopStatsResponse;
        }
    }

    void
    AddBaseBusResponses(TransportCatalog::TransportCatalog &transportCatalog) {
        for (const auto &[bus_name, route]: bus_routes) {
            TransportCatalog::BusStatsResponse busStatsResponse;
            busStatsResponse.set_name(bus_name);
            busStatsResponse.set_stop_count(route.stops_cnt);
            busStatsResponse.set_unique_stop_count(route.unique_stops_cnt);
            busStatsResponse.set_route_length(route.route_real_length);
            busStatsResponse.set_curvature(route.curvature);
            busStatsResponse.set_is_roundtrip(route.is_roundtrip);

            for (const auto &stop_name: route.stops) {
                *busStatsResponse.mutable_route_stops()->Add() = stop_name;
            }

            *transportCatalog.mutable_bus_responses()->Add() = busStatsResponse;
        }
    }

    void
    AddBaseRoutingSettings(TransportCatalog::TransportCatalog &transportCatalog,
                           const RoutingSettings &routing_settings) {
        TransportCatalog::RoutingSettings settings;
        settings.set_bus_velocity(routing_settings.bus_velocity);
        settings.set_bus_wait_time(routing_settings.bus_wait_time);
        *transportCatalog.mutable_routing_settings() = settings;
    }

    void
    ParseBaseResponses(unique_ptr<TransportCatalog::TransportCatalog> catalog,
                       Queries &queries) {
        if (!catalog->IsInitialized())
            return;

        // stop_buses.clear();
        for (const auto &entry: catalog->stop_responses()) {
            string all_buses;
            for (const auto &bus_name: entry.buses()) {
                all_buses += bus_name + ", ";
                stop_buses[entry.name()].insert(bus_name);
            }

            stops[entry.name()] = {entry.lat(), entry.lng()};
            for (const auto &road: entry.road_distances()) {
                edge_len[{entry.name(), road.stop_name()}] = road.distance();
            }
        }

        // bus_routes.clear();
        for (const auto &entry: catalog->bus_responses()) {
            vector<string> route_stops;
            route_stops.reserve(entry.route_stops_size());
            for (const auto &stop_name: entry.route_stops()) {
                route_stops.push_back(stop_name);
            }
            bus_routes[entry.name()] =
                    BusRoute{entry.stop_count(), entry.unique_stop_count(),
                             entry.route_length(), entry.curvature(),
                             route_stops, entry.is_roundtrip()};
        }

        // routing_settings
        queries.routing_settings = make_unique<RoutingSettings>(
                catalog->routing_settings().bus_wait_time(),
                catalog->routing_settings().bus_velocity());
    }

    void GetBusInfoJson(const BusInfoQuery &query) {
        stringstream str("");
        str << "{\"request_id\":" << query.GetQueryIndex() << ",";
        if (bus_routes.count(query.GetName())) {
            auto &route_info = bus_routes.at(query.GetName());
            str << "\"curvature\":" << std::fixed << std::setprecision(7)
                << route_info.curvature << ",";
            str << "\"route_length\":" << std::fixed << std::setprecision(7)
                << route_info.route_real_length << ",";
            str << "\"stop_count\":" << route_info.stops_cnt << ",";
            str << "\"unique_stop_count\":" << route_info.unique_stops_cnt;
        } else {
            str << R"("error_message":"not found")";
        }
        str << "}";

        responses.push_back(str.str());
    }

    void GetStopInfoJson(const StopInfoQuery &query) {
        stringstream str("");

        str << "{\"request_id\":" << query.GetQueryIndex() << ",";
        if (stop_buses.count(query.GetName())) {
            str << "\"buses\":[";
            if (stop_buses.count(query.GetName()) >= 0) {
                const auto &buses = stop_buses.at(query.GetName());
                for (auto it = buses.begin(); it != buses.end(); it++) {
                    if ((*it) == "") {
                        continue;
                    }
                    str << "\"" << *it << "\"";
                    if (it != prev(buses.end())) {
                        str << ",";
                    }
                }
            }
            str << "]";
        } else {
            str << R"("error_message":"not found")";
        }
        str << "}";

        responses.push_back(str.str());
    }

    string GetRouteMapString(const RenderSettings &render_settings,
                             const map<string, LatLng> &stop_coordinates,
                             const CoordinateCompressionParams &compress_params,
                             const vector<pair<string, vector<pair<string, int>>>>
                             &bus_name_with_stops,
                             const vector<string> &all_wait_stops) {
        stringstream str;
        str << "\"map\":";
        Svg::Document svg;
        GetMapString(render_settings, stop_coordinates, compress_params, svg);
        AddRectLayer(svg, render_settings);
        AddRouteMapLayers(render_settings, stop_coordinates, compress_params, svg,
                          bus_name_with_stops, all_wait_stops);
        svg.Render(str);
        return str.str();
    }

    void GetRouteInfoJson(
            const RouteInfoQuery
            &query
    ) {
        if (name_to_id_origin_stop_name.count(query.GetFrom()) == 0 ||
            name_to_id_origin_stop_name.count(query.GetTo()) == 0) {
            cout << query.GetFrom() << " " << query.GetTo() << endl;
            throw runtime_error("GetRouteInfoJson: " + query.GetFrom() + " -> " +
                                query.GetTo());
        }
        stringstream str("");
        str << "{\"request_id\":" << query.GetQueryIndex() << ",";

        vector<pair<string, vector<pair<string, int>>>> bus_name_with_stops;
        vector<string> all_wait_stops;

        if (name_to_id_origin_stop_name.count(query.GetFrom()) > 0 &&
            name_to_id_origin_stop_name.count(query.GetTo()) > 0) {
            auto route_info =
                    router->BuildRoute(name_to_id_origin_stop_name[query.GetFrom()].first,
                                       name_to_id_origin_stop_name[query.GetTo()].first);
            if (route_info.has_value()) {
                string error_str;
                vector<RawEdge> v;
                v.reserve(route_info.value().edge_count);
                for (int i = 0; i < route_info.value().edge_count; i++) {
                    auto edge =
                            id_to_raw_edge[router->GetRouteEdge(route_info.value().id, i)];
                    if (edge.type == "-")
                        continue;
                    if (edge.type == "Wait") {
                        v.push_back(edge);
                    } else if (edge.type == "Bus") {
                        if (!v.empty() && v.back().name == edge.name &&
                            v.back().type != "Wait") {
                            auto &e = v.back();
                            e.span_count += edge.span_count;
                            e.time += edge.time;

                            auto &curr_route_stops = bus_name_with_stops.back().second;
                            const auto &bus_stops = bus_routes.at(edge.name).stops;
                            if (edge.idx_start < edge.idx_end) {
                                for (int idx = edge.idx_start + 1; idx <= edge.idx_end; idx++) {
                                    curr_route_stops.emplace_back(bus_stops[idx], idx);
                                }
                            } else {
                                for (int idx = edge.idx_start - 1; idx >= edge.idx_end; idx--) {
                                    curr_route_stops.emplace_back(bus_stops[idx], idx);
                                }
                            }
                        } else {
                            v.push_back(edge);

                            bus_name_with_stops.push_back({edge.name, {}});
                            auto &curr_route_stops = bus_name_with_stops.back().second;
                            const auto &bus_stops = bus_routes.at(edge.name).stops;
                            if (edge.idx_start < edge.idx_end) {
                                for (int idx = edge.idx_start; idx <= edge.idx_end; idx++) {
                                    curr_route_stops.emplace_back(bus_stops[idx], idx);
                                }
                            } else {
                                for (int idx = edge.idx_start; idx >= edge.idx_end; idx--) {
                                    curr_route_stops.emplace_back(bus_stops[idx], idx);
                                }
                            }
                        }
                    }
                }

                str << "\"total_time\":" << std::fixed << std::setprecision(6)
                    << route_info.value().weight << ",";
                str << "\"items\":[";
                for (int i = 0; i < v.size(); i++) {
                    str << "{";
                    if (v[i].type == "Bus") {
                        str << R"("type":"Bus",)";
                        str << R"("bus":")" << v[i].name << "\",";
                        str << "\"time\":" << v[i].time << ",";
                        str << "\"span_count\":" << v[i].span_count;
                    } else {
                        str << R"("type":"Wait",)";
                        str << R"("stop_name":")" << v[i].name << "\",";
                        str << "\"time\":" << v[i].time;
                    }
                    str << "}";
                    if (i + 1 < v.size()) {
                        str << ",";
                    }
                }
                str << "]";
            } else {
                str << R"("error_message": "not found")";
            }
        } else {
            cout << query.GetFrom() << " " << query.GetTo() << endl;
        }
        str << "}";
        responses.push_back(str.str());
    }

    [[nodiscard]] static Point
    LatLngToXY(const LatLng &p,
               const CoordinateCompressionParams &compression_params,
               const RenderSettings &render_settings) {
        double x_id = compression_params.lng_to_id.at(p.lng);
        double y_id = compression_params.lat_to_id.at(p.lat);

        return Point{x_id * compression_params.x_step + render_settings.padding,
                     render_settings.height - render_settings.padding -
                     y_id * compression_params.y_step};
    }

    static void AddRectLayer(Svg::Document &svg,
                             const RenderSettings &render_settings) {
        Svg::Rect rect =
                Svg::Rect{}
                        .SetPointXY(
                                {-render_settings.outer_margin, -render_settings.outer_margin})
                        .SetWidth(render_settings.width + 2 * render_settings.outer_margin)
                        .SetHeight(render_settings.height +
                                   2 * render_settings.outer_margin)
                        .SetFillColor(render_settings.underlayer_color);
        svg.Add(rect);
    }

    void AddRouteMapLayers(const RenderSettings &render_settings,
                           const map<string, LatLng> &stop_coordinates,
                           const CoordinateCompressionParams &compress_params,
                           Svg::Document &svg,
                           const vector<pair<string, vector<pair<string, int>>>>
                           &bus_name_with_stops,
                           const vector<string> &all_wait_stops) {
        for (const auto &l: render_settings.layers) {
            if (l == "bus_lines") {
                AddRouteBusLinesLayer(svg, compress_params, render_settings,
                                      stop_coordinates, bus_name_with_stops);
            } else if (l == "bus_labels") {
                AddRouteBusLabelsLayer(svg, compress_params, render_settings,
                                       stop_coordinates, bus_name_with_stops);
            } else if (l == "stop_points") {
                AddRouteStopPointsLayer(svg, compress_params, render_settings,
                                        stop_coordinates, bus_name_with_stops);
            } else if (l == "stop_labels") {
                AddRouteStopLabelsLayer(svg, compress_params, render_settings,
                                        stop_coordinates, all_wait_stops);
            }
        }
    }

    void
    AddRouteBusLinesLayer(Svg::Document &svg,
                          const CoordinateCompressionParams &compression_params,
                          const RenderSettings &render_settings,
                          const map<string, LatLng> &stops_coordinates,
                          const vector<pair<string, vector<pair<string, int>>>>
                          &bus_name_with_stops) {
        for (const auto &[bus_name, route_stops]: bus_name_with_stops) {
            const auto color =
                    render_settings.color_palette[bus_name_to_color_id[bus_name]];

            Svg::Polyline polyline = Svg::Polyline{}
                    .SetStrokeColor(color)
                    .SetStrokeWidth(render_settings.line_width)
                    .SetStrokeLineCap("round")
                    .SetStrokeLineJoin("round");

            for (const auto &stop: route_stops) {
                const auto point = LatLngToXY(stops_coordinates.at(stop.first),
                                              compression_params, render_settings);
                polyline.AddPoint(point);
            }

            svg.Add(polyline);
        }
    }

    void
    AddRouteBusLabelsLayer(Svg::Document &svg,
                           const CoordinateCompressionParams &compression_params,
                           const RenderSettings &render_settings,
                           const map<string, LatLng> &stops_coordinates,
                           const vector<pair<string, vector<pair<string, int>>>>
                           &bus_name_with_stops) {
        string curr_bus_name;
        for (const auto &[bus_name, route_stops]: bus_name_with_stops) {
            curr_bus_name = bus_name;
            const auto color =
                    render_settings.color_palette[bus_name_to_color_id[bus_name]];

            const auto add_stop = [&](const Point &p) {
                svg.Add(Svg::Text{}
                                .SetPoint(p)
                                .SetOffset(render_settings.bus_label_offset)
                                .SetFontSize(render_settings.bus_label_font_size)
                                .SetFontFamily("Verdana")
                                .SetFontWeight("bold")
                                .SetData(curr_bus_name)
                                .SetFillColor(render_settings.underlayer_color)
                                .SetStrokeColor(render_settings.underlayer_color)
                                .SetStrokeWidth(render_settings.underlayer_width)
                                .SetStrokeLineCap("round")
                                .SetStrokeLineJoin("round"));
                svg.Add(Svg::Text{}
                                .SetPoint(p)
                                .SetOffset(render_settings.bus_label_offset)
                                .SetFontSize(render_settings.bus_label_font_size)
                                .SetFontFamily("Verdana")
                                .SetFontWeight("bold")
                                .SetData(curr_bus_name)
                                .SetFillColor(color));
            };

            const auto &bus_route_global = bus_routes[bus_name];
            for (const auto &stop: route_stops) {
                if (stop.first == bus_route_global.stops[0] ||
                    (!bus_route_global.is_roundtrip &&
                     stop.first == bus_route_global.stops.back())) {
                    add_stop(LatLngToXY(stops_coordinates.at(stop.first),
                                        compression_params, render_settings));
                }
            }
        }
    }

    static void
    AddRouteStopPointsLayer(Svg::Document &svg,
                            const CoordinateCompressionParams &compression_params,
                            const RenderSettings &render_settings,
                            const map<string, LatLng> &stops_coordinates,
                            const vector<pair<string, vector<pair<string, int>>>>
                            &bus_name_with_stops) {
        for (const auto &[bus_name, route_stops]: bus_name_with_stops) {
            for (const auto &stop: route_stops) {
                const auto point = LatLngToXY(stops_coordinates.at(stop.first),
                                              compression_params, render_settings);
                svg.Add(Svg::Circle{}
                                .SetCenter(point)
                                .SetRadius(render_settings.stop_radius)
                                .SetFillColor("white"));
            }
        }
    }

    static void
    AddRouteStopLabelsLayer(Svg::Document &svg,
                            const CoordinateCompressionParams &compression_params,
                            const RenderSettings &render_settings,
                            const map<string, LatLng> &stops_coordinates,
                            const vector<string> &all_wait_stops) {
        if (all_wait_stops.size() == 1) {
            return;
        }
        for (const auto &stop: all_wait_stops) {
            const auto point = LatLngToXY(stops_coordinates.at(stop),
                                          compression_params, render_settings);
            svg.Add(Svg::Text{}
                            .SetPoint(point)
                            .SetOffset(render_settings.stop_label_offset)
                            .SetFontSize(render_settings.stop_label_font_size)
                            .SetFontFamily("Verdana")
                            .SetData(stop)
                            .SetFillColor(render_settings.underlayer_color)
                            .SetStrokeColor(render_settings.underlayer_color)
                            .SetStrokeWidth(render_settings.underlayer_width)
                            .SetStrokeLineCap("round")
                            .SetStrokeLineJoin("round"));
            svg.Add(Svg::Text{}
                            .SetPoint(point)
                            .SetOffset(render_settings.stop_label_offset)
                            .SetFontSize(render_settings.stop_label_font_size)
                            .SetFontFamily("Verdana")
                            .SetData(stop)
                            .SetFillColor("black"));
        }
    }

    void AddBusLinesLayer(Svg::Document &svg,
                          const CoordinateCompressionParams &compression_params,
                          const RenderSettings &render_settings,
                          const map<string, LatLng> &stops_coordinates) {
        ::size_t color_idx = 0;
        for (const auto &[name, route]: bus_routes) {
            const auto color = render_settings.color_palette[color_idx];
            bus_name_to_color_id[name] = color_idx;

            Svg::Polyline polyline = Svg::Polyline{}
                    .SetStrokeColor(color)
                    .SetStrokeWidth(render_settings.line_width)
                    .SetStrokeLineCap("round")
                    .SetStrokeLineJoin("round");

            for (const auto &stop: route.stops) {
                const auto point = LatLngToXY(stops_coordinates.at(stop),
                                              compression_params, render_settings);
                polyline.AddPoint(point);
            }

            if (!route.is_roundtrip) {
                for (int i = route.stops.size() - 2; i >= 0; i--) {
                    const auto point = LatLngToXY(stops_coordinates.at(route.stops[i]),
                                                  compression_params, render_settings);
                    polyline.AddPoint(point);
                }
            }

            svg.Add(polyline);

            color_idx = (color_idx + 1) % render_settings.color_palette.size();
        }
    }

    void AddBusLabelsLayer(Svg::Document &svg,
                           const CoordinateCompressionParams &compression_params,
                           const RenderSettings &render_settings,
                           const map<string, LatLng> &stops_coordinates) {
        size_t color_idx = 0;
        string bus_name;
        for (const auto &[name, route]: bus_routes) {
            bus_name = name;
            const auto color = render_settings.color_palette[color_idx];

            const auto add_stop = [&](const Point &p) {
                svg.Add(Svg::Text{}
                                .SetPoint(p)
                                .SetOffset(render_settings.bus_label_offset)
                                .SetFontSize(render_settings.bus_label_font_size)
                                .SetFontFamily("Verdana")
                                .SetFontWeight("bold")
                                .SetData(bus_name)
                                .SetFillColor(render_settings.underlayer_color)
                                .SetStrokeColor(render_settings.underlayer_color)
                                .SetStrokeWidth(render_settings.underlayer_width)
                                .SetStrokeLineCap("round")
                                .SetStrokeLineJoin("round"));
                svg.Add(Svg::Text{}
                                .SetPoint(p)
                                .SetOffset(render_settings.bus_label_offset)
                                .SetFontSize(render_settings.bus_label_font_size)
                                .SetFontFamily("Verdana")
                                .SetFontWeight("bold")
                                .SetData(bus_name)
                                .SetFillColor(color));
            };

            add_stop(LatLngToXY(stops_coordinates.at(route.stops[0]),
                                compression_params, render_settings));

            if (!route.is_roundtrip &&
                route.stops[0] != route.stops[route.stops.size() - 1]) {
                add_stop(LatLngToXY(
                        stops_coordinates.at(route.stops[route.stops.size() - 1]),
                        compression_params, render_settings));
            }

            color_idx = (color_idx + 1) % render_settings.color_palette.size();
        }
    }

    static void
    AddStopPointsLayer(Svg::Document &svg,
                       const CoordinateCompressionParams &compression_params,
                       const RenderSettings &render_settings,
                       const map<string, LatLng> &stops_coordinates) {
        for (const auto &stop: stops_coordinates) {
            const auto point =
                    LatLngToXY(stop.second, compression_params, render_settings);
            svg.Add(Svg::Circle{}
                            .SetCenter(point)
                            .SetRadius(render_settings.stop_radius)
                            .SetFillColor("white"));
        }
    }

    static void
    AddStopLabelsLayer(Svg::Document &svg,
                       const CoordinateCompressionParams &compression_params,
                       const RenderSettings &render_settings,
                       const map<string, LatLng> &stops_coordinates) {
        for (const auto &stop: stops_coordinates) {
            const auto point =
                    LatLngToXY(stop.second, compression_params, render_settings);
            svg.Add(Svg::Text{}
                            .SetPoint(point)
                            .SetOffset(render_settings.stop_label_offset)
                            .SetFontSize(render_settings.stop_label_font_size)
                            .SetFontFamily("Verdana")
                            .SetData(stop.first)
                            .SetFillColor(render_settings.underlayer_color)
                            .SetStrokeColor(render_settings.underlayer_color)
                            .SetStrokeWidth(render_settings.underlayer_width)
                            .SetStrokeLineCap("round")
                            .SetStrokeLineJoin("round"));
            svg.Add(Svg::Text{}
                            .SetPoint(point)
                            .SetOffset(render_settings.stop_label_offset)
                            .SetFontSize(render_settings.stop_label_font_size)
                            .SetFontFamily("Verdana")
                            .SetData(stop.first)
                            .SetFillColor("black"));
        }
    }

    void GetMapInfoJson(const MapInfoQuery &query,
                        const RenderSettings &render_settings,
                        const map<string, LatLng> &stop_coordinates,
                        const CoordinateCompressionParams &compress_params) {

        stringstream str;
        str << R"({"request_id":)" << query.GetQueryIndex() << ",";
        str << "\"map\":";
        Svg::Document svg;
        GetMapString(render_settings, stop_coordinates, compress_params, svg);
        svg.Render(str);
        str << "}";
        responses.push_back(str.str());
    }

    void GetMapString(const RenderSettings &render_settings,
                      const map<string, LatLng> &stop_coordinates,
                      const CoordinateCompressionParams &compress_params,
                      Svg::Document &svg) {
        for (const auto &l: render_settings.layers) {
            if (l == "bus_lines") {
                AddBusLinesLayer(svg, compress_params, render_settings,
                                 stop_coordinates);
            } else if (l == "bus_labels") {
                AddBusLabelsLayer(svg, compress_params, render_settings,
                                  stop_coordinates);
            } else if (l == "stop_points") {
                AddStopPointsLayer(svg, compress_params, render_settings,
                                   stop_coordinates);
            } else if (l == "stop_labels") {
                AddStopLabelsLayer(svg, compress_params, render_settings,
                                   stop_coordinates);
            }
        }
    }

    void PrintResponsesToJson(ostream &out) const {
        out << "[" << endl;
        int cnt = responses.size();
        for (const auto &r: responses) {
            out << r;
            if (--cnt) {
                out << ",";
            }
            out << endl;
        }
        out << "]" << endl;
    }

    void BuildGraph(const RoutingSettings &routing_settings) {
        for (const auto &stop: stops) {
            const auto &stop_name = stop.first;

            name_to_id_origin_stop_name[stop_name] = {
                    name_to_id_origin_stop_name.size(), stop_name};
            id_to_name[name_to_id_origin_stop_name[stop_name].first] = stop_name;

            string wait_stop = stop_name + ":wait";
            name_to_id_origin_stop_name[wait_stop] = {
                    name_to_id_origin_stop_name.size(), stop_name};
            id_to_name[name_to_id_origin_stop_name[wait_stop].first] = wait_stop;
        }

        const int vertex_number = name_to_id_origin_stop_name.size();
        Graph::DirectedWeightedGraph<double> weighted_graph(vertex_number);

        for (const auto &stop: stops) {
            {
                string from = stop.first;
                string to = stop.first + ":wait";

                weighted_graph.AddEdge({name_to_id_origin_stop_name[from].first,
                                        name_to_id_origin_stop_name[to].first,
                                        routing_settings.bus_wait_time});
                const auto curr_edge_id = weighted_graph.GetEdgeCount() - 1;
                id_to_raw_edge[curr_edge_id] = {
                        "Wait", stop.first, routing_settings.bus_wait_time, 0, from, to, 0};
            }
            {
                string from = stop.first + ":wait";
                string to = stop.first;

                weighted_graph.AddEdge({name_to_id_origin_stop_name[from].first,
                                        name_to_id_origin_stop_name[to].first, 0});
                const auto curr_edge_id = weighted_graph.GetEdgeCount() - 1;
                id_to_raw_edge[curr_edge_id] = {"-", from + "_" + to, 0, 0, from, to,
                                                0};
            }
        }

        auto add_edge = [&](const string &from, const string &to,
                            const string &bus_name, double len, int i_value,
                            int j_value) {
            int span_count = abs(i_value - j_value);
            auto from_id = name_to_id_origin_stop_name[from].first;
            auto to_id = name_to_id_origin_stop_name[to].first;
            string edge_name = from + "_" + to;
            weighted_graph.AddEdge(
                    {from_id, to_id,
                     len / 1000.0 / routing_settings.bus_velocity * 60.0});
            const auto curr_edge_id = weighted_graph.GetEdgeCount() - 1;
            id_to_raw_edge[curr_edge_id] = {
                    "Bus",
                    bus_name,
                    len / 1000.0 / routing_settings.bus_velocity * 60,
                    span_count,
                    from,
                    to,
                    len,
                    i_value,
                    j_value};
        };

        for (const auto &bus: bus_routes) {
            const auto &bus_name = bus.first;
            auto bus_stops = bus.second.stops;

            for (int i = 0; i < bus_stops.size(); i++) {
                double total_len = 0;

                for (int j = i + 1; j < bus_stops.size(); j++) {
                    double len =
                            GetRealDistance(bus.second.stops[j - 1], bus.second.stops[j]);
                    if (len == 0.0) {
                        len = GetRealDistance(bus.second.stops[j], bus.second.stops[j - 1]);
                        if (len == 0.0) {
                            len = calculateDistance(stops[bus_stops[j - 1]],
                                                    stops[bus_stops[j]]) *
                                  1000;
                        }
                    }
                    total_len += len;
                    string from_name = bus.second.stops[i] + ":wait";
                    string to_name = bus.second.stops[j];
                    add_edge(from_name, to_name, bus_name, total_len, i, j);
                }
            }

            if (!bus.second.is_roundtrip) {
                for (int i = (int) bus_stops.size() - 1; i >= 0; i--) {
                    double total_len = 0;
                    for (int j = i - 1; j >= 0; j--) {
                        double len =
                                GetRealDistance(bus.second.stops[j + 1], bus.second.stops[j]);
                        if (len == 0.0) {
                            len =
                                    GetRealDistance(bus.second.stops[j], bus.second.stops[j + 1]);
                            if (len == 0.0) {
                                len = calculateDistance(stops[bus_stops[j + 1]],
                                                        stops[bus_stops[j]]) *
                                      1000;
                            }
                        }
                        total_len += len;
                        string from_name = bus.second.stops[i] + ":wait";
                        string to_name = bus.second.stops[j];
                        add_edge(from_name, to_name, bus_name, total_len, i, j);
                    }
                }
            }
        }

        graph = make_unique<Graph::DirectedWeightedGraph<double>>(weighted_graph);
    }

public:
    map<string, LatLng> stops;
    unordered_map<pair<string, string>, double> edge_len;
    unordered_map<string, set<string>> stop_buses;
    unordered_map<string, BusRoute> bus_routes;
    unordered_map<string, int> bus_name_to_color_id;
    vector<string> responses;
    unique_ptr<Graph::DirectedWeightedGraph<double>> graph;
    unordered_map<string, pair<size_t, string>> name_to_id_origin_stop_name;
    unordered_map<size_t, string> id_to_name;
    unordered_map<size_t, RawEdge> id_to_raw_edge;
    unordered_map<string, set<string>> edges_from_stop;
    unordered_map<string, vector<pair<string, int>>> stop_road_distances;

    double GetRealDistance(const string &from, const string &to) {
        edges_from_stop[from].insert(to);
        edges_from_stop[to].insert(from);
        if (edge_len.count({from, to})) {
            return edge_len.at({from, to});
        }
        return 0.0;
    }

    map<string, LatLng> FindNewStopCoordinates() {
        unordered_set<string> anchor_stops;
        for (const auto &[name, buses]: stop_buses) {
            if (buses.size() > 1) {
                anchor_stops.insert(name);
            }
        }

        for (const auto &[name, route]: bus_routes) {
            anchor_stops.insert(route.stops[0]);
            anchor_stops.insert(route.stops[route.stops.size() - 1]);

            unordered_map<string, int> stop_visit_cnt;
            for (const auto &stop: route.stops) {
                stop_visit_cnt[stop]++;
            }
            if (!route.is_roundtrip) {
                for (int i = route.stops.size() - 1; i > 0; i--) {
                    stop_visit_cnt[route.stops[i - 1]]++;
                }
            }

            for (const auto &[stop, visit_cnt]: stop_visit_cnt) {
                if (visit_cnt > 2) {
                    anchor_stops.insert(stop);
                }
            }
        }

        map<string, LatLng> updated_stop_lat_lng;
        for (const auto &[stop_name, lng_lat]: stops) {
            updated_stop_lat_lng[stop_name] = lng_lat;
        }

        for (const auto &[_, route]: bus_routes) {
            auto route_stops = route.stops;
            if (!route.is_roundtrip) {
                for (int i = route.stops.size() - 1; i > 0; i--) {
                    route_stops.push_back(route.stops[i - 1]);
                }
            }

            for (int i = 0; i < route_stops.size();) {
                const auto lng_lat_i = updated_stop_lat_lng[route_stops[i]];

                int j = i + 1;
                for (; j < route_stops.size(); j++) {
                    if (anchor_stops.count(route_stops[j]) == 0) {
                        continue;
                    }

                    const auto lng_lat_j = stops[route_stops[j]];
                    const double lng_step = (lng_lat_j.lng - lng_lat_i.lng) / (j - i);
                    const double lat_step = (lng_lat_j.lat - lng_lat_i.lat) / (j - i);
                    for (int k = i + 1; k < j; k++) {
                        updated_stop_lat_lng[route_stops[k]] = {
                                lng_lat_i.lat + lat_step * (k - i),
                                lng_lat_i.lng + lng_step * (k - i)};
                    }

                    break;
                }
                i = j;
            }
        }

        return updated_stop_lat_lng;
    }

    CoordinateCompressionParams
    MapLngLatToIds(const RenderSettings &render_settings,
                   const map<string, LatLng> &stop_coordinates) {
        map<double, string> lngToStopName, latToStopName;
        for (const auto &[name, point]: stop_coordinates) {
            lngToStopName[point.lng] = name;
            latToStopName[point.lat] = name;
        }

        int maxLngId = -1;
        map<double, int> lngToId;
        {
            unordered_map<string, int> nameToId;
            for (auto &[lng, name]: lngToStopName) {
                int max_id = -1;
                for (const auto &edge_to: edges_from_stop[name]) {
                    if (nameToId.count(edge_to) > 0) {
                        max_id = max(max_id, nameToId[edge_to]);
                    }
                }
                lngToId[lng] = nameToId[name] = max_id + 1;
                maxLngId = max(maxLngId, max_id + 1);
            }
        }

        int maxLatId = -1;
        map<double, int> latToId;
        {
            unordered_map<string, int> nameToId;
            for (auto &[lat, name]: latToStopName) {
                int max_id = -1;
                for (const auto &edge_to: edges_from_stop[name]) {
                    if (nameToId.count(edge_to) > 0) {
                        max_id = max(max_id, nameToId[edge_to]);
                    }
                }
                latToId[lat] = nameToId[name] = max_id + 1;
                maxLatId = max(maxLatId, max_id + 1);
            }
        }

        double x_step =
                maxLngId < 1
                ? 0
                : (render_settings.width - 2 * render_settings.padding) / maxLngId;
        double y_step =
                maxLatId < 1
                ? 0
                : (render_settings.height - 2 * render_settings.padding) / maxLatId;
        return {lngToId, latToId, x_step, y_step};
    }
};

void MakeBase() {
    auto queries = ParseBaseQueries();

    DbManager manager;
    for (const auto &q: queries.stop) {
        manager.AddStop(*q);
    }

    for (const auto &q: queries.bus) {
        manager.AddBusRoute(*q);
    }

    TransportCatalog::TransportCatalog transportCatalog;
    manager.AddBaseStopResponses(transportCatalog);
    manager.AddBaseBusResponses(transportCatalog);
    manager.AddBaseRoutingSettings(transportCatalog, *queries.routing_settings);

    const string file_name = queries.serialization_settings->file_name;
    std::ofstream output_file(file_name, std::ios::binary);
    if (transportCatalog.SerializeToOstream(&output_file)) {
    } else {
        std::cerr << "Failed to serialize to file." << std::endl;
    }
    output_file.close();
}

string ReadFileData(const string &file_name) {
    ifstream file(file_name, ios::binary | ios::ate);
    const ifstream::pos_type end_pos = file.tellg();
    file.seekg(0, ios::beg);

    string data(end_pos, '\0');
    file.read(&data[0], end_pos);
    return data;
}

void ProcessRequests() {
    auto queries = ParseProcessQueries();

    DbManager manager;
    unique_ptr<TransportCatalog::TransportCatalog> catalog =
            make_unique<TransportCatalog::TransportCatalog>();
    if (catalog->ParseFromString(
            ReadFileData(queries.serialization_settings->file_name))) {
        manager.ParseBaseResponses(std::move(catalog), queries);
    } else {
        throw runtime_error("process request: parseFromString() failed");
    }

    manager.BuildGraph(*queries.routing_settings);
    router =
            make_unique<Graph::Router<double>>(Graph::Router<double>(*manager.graph));

    manager.responses.reserve(queries.bus_info.size() + queries.stop_info.size() +
                              queries.route_info.size());
    for (const auto &q: queries.bus_info) {
        manager.GetBusInfoJson(*q);
    }

    for (const auto &q: queries.stop_info) {
        manager.GetStopInfoJson(*q);
    }

    for (const auto &q: queries.route_info) {
        manager.GetRouteInfoJson(*q);
    }

    manager.PrintResponsesToJson(cout);
}

int main(int argc, const char *argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (argc != 2) {
        cerr << "Usage: transport_catalog_part_o [make_base|process_requests]\n";
        return 5;
    }

    const string_view mode(argv[1]);

    if (mode == "make_base") {
        MakeBase();
    } else if (mode == "process_requests") {
        ProcessRequests();
    }

    return 0;
}
