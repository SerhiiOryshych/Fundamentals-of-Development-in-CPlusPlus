#include "graph.h"
#include "query.h"
#include "router.h"
#include "svg.h"
#include "test_runner.h"
#include "types.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <istream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

using namespace std;

unique_ptr<Graph::Router<double>> router;

class DbManager {
private:
    struct RawEdge {
        string type;
        string name;
        double time;
        int span_count = 0;
        string from, to;
        double distance;
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
        }
    }

    void AddBusRoute(const BusRouteQuery &query) {
        set<string> unique_stops;
        double length_geo = 0;
        double length_real = 0;
        int route_stops_cnt = 0;
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

    void GetBusInfoJson(const BusInfoQuery &query) {
        stringstream str("");
        str << "{\"request_id\":" << query.GetQueryIndex() << ",";
        if (bus_routes.count(query.GetName())) {
            auto &route_info = bus_routes.at(query.GetName());
            str << "\"curvature\":" << std::fixed << std::setprecision(6)
                << route_info.curvature << ",";
            str << "\"route_length\":" << std::fixed << std::setprecision(6)
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
        if (stops.count(query.GetName())) {
            str << "\"buses\":[";
            if (stop_buses.count(query.GetName())) {
                const auto &buses = stop_buses.at(query.GetName());
                for (auto it = buses.begin(); it != buses.end(); it++) {
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

    void GetRouteInfoJson(const RouteInfoQuery &query) {
        if (name_to_id.count(query.GetFrom()) == 0 ||
            name_to_id.count(query.GetTo()) == 0) {
            cout << query.GetFrom() << " " << query.GetTo() << endl;
            throw;
        }
        stringstream str("");
        str << "{\"request_id\":" << query.GetQueryIndex() << ",";

        if (name_to_id.count(query.GetFrom()) > 0 &&
            name_to_id.count(query.GetTo()) > 0) {
            auto route_info = router->BuildRoute(name_to_id[query.GetFrom()],
                                                 name_to_id[query.GetTo()]);
            if (route_info.has_value()) {
                vector<RawEdge> v;
                for (int i = 0; i < route_info.value().edge_count; i++) {
                    auto edge =
                            id_to_raw_edge[router->GetRouteEdge(route_info.value().id, i)];
                    if (edge.type == "-")
                        continue;
                    if (edge.type == "Wait") {
                        v.push_back(edge);
                    } else if (edge.type == "Bus") {
                        if (!v.empty() && v.back().name == edge.name) {
                            auto &e = v.back();
                            e.span_count += edge.span_count;
                            e.time += edge.time;
                        } else {
                            v.push_back(edge);
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

                router->ReleaseRoute(route_info->id);
            } else {
                str << R"("error_message": "not found")";
            }
        } else {
            cout << query.GetFrom() << " " << query.GetTo() << endl;
        }
        str << "}";
        responses.push_back(str.str());
    }

    [[nodiscard]] Point
    LatLngToXY(const LatLng &p,
               const CoordinateCompressionParams &compression_params, const RenderSettings &render_settings) const {
        double x_id = compression_params.lng_to_id.at(p.lng);
        double y_id = compression_params.lat_to_id.at(p.lat);

        return Point{
                x_id * compression_params.x_step + render_settings.padding,
                render_settings.height - render_settings.padding - y_id * compression_params.y_step};
    }

    void
    AddBusLinesLayer(Svg::Document &svg,
                     const CoordinateCompressionParams &compression_params, const RenderSettings &render_settings) {
        ::size_t color_idx = 0;
        for (const auto &[name, route]: bus_routes) {
            const auto color = render_settings.color_palette[color_idx];

            Svg::Polyline polyline = Svg::Polyline{}
                    .SetStrokeColor(color)
                    .SetStrokeWidth(render_settings.line_width)
                    .SetStrokeLineCap("round")
                    .SetStrokeLineJoin("round");

            for (const auto &stop: route.stops) {
                const auto point =
                        LatLngToXY(stops[stop], compression_params, render_settings);
                polyline.AddPoint(point);
            }

            if (!route.is_roundtrip) {
                for (int i = route.stops.size() - 2; i >= 0; i--) {
                    const auto point = LatLngToXY(stops[route.stops[i]], compression_params, render_settings);
                    polyline.AddPoint(point);
                }
            }

            svg.Add(polyline);

            color_idx = (color_idx + 1) % render_settings.color_palette.size();
        }
    }

    void
    AddBusLabelsLayer(Svg::Document &svg,
                      const CoordinateCompressionParams &compression_params, const RenderSettings &render_settings) {
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

            add_stop(LatLngToXY(stops[route.stops[0]], compression_params,
                                render_settings));
            if (!route.is_roundtrip &&
                route.stops[0] != route.stops[route.stops.size() - 1]) {
                add_stop(LatLngToXY(stops[route.stops[route.stops.size() - 1]], compression_params, render_settings));
            }

            color_idx = (color_idx + 1) % render_settings.color_palette.size();
        }
    }

    void
    AddStopPointsLayer(Svg::Document &svg,
                       const CoordinateCompressionParams &compression_params, const RenderSettings &render_settings) {
        for (const auto &stop: stops) {
            const auto point =
                    LatLngToXY(stop.second, compression_params, render_settings);
            svg.Add(Svg::Circle{}
                            .SetCenter(point)
                            .SetRadius(render_settings.stop_radius)
                            .SetFillColor("white"));
        }
    }

    void
    AddStopLabelsLayer(Svg::Document &svg,
                       const CoordinateCompressionParams &compression_params, const RenderSettings &render_settings) {
        for (const auto &stop: stops) {
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
                        const RenderSettings &render_settings) {
        // const auto min_max = FindMinMaxXY();
        // const auto zoom_coef = CalcZoomCoef(render_settings, min_max);

        const auto compress_params = MapLngLatToIds(render_settings);

        Svg::Document svg;

        for (const auto &l: render_settings.layers) {
            if (l == "bus_lines") {
                AddBusLinesLayer(svg, compress_params, render_settings);
            } else if (l == "bus_labels") {
                AddBusLabelsLayer(svg, compress_params, render_settings);
            } else if (l == "stop_points") {
                AddStopPointsLayer(svg, compress_params, render_settings);
            } else if (l == "stop_labels") {
                AddStopLabelsLayer(svg, compress_params, render_settings);
            }
        }

        stringstream str;
        str << R"({"request_id":)" << query.GetQueryIndex() << ",";
        str << "\"map\":";
        svg.Render(str);
        str << "}";
        responses.push_back(str.str());
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

            name_to_id[stop_name] = name_to_id.size();
            id_to_name[name_to_id[stop_name]] = stop_name;

            string wait_stop = stop_name + ":wait";
            name_to_id[wait_stop] = name_to_id.size();
            id_to_name[name_to_id[wait_stop]] = wait_stop;
        }

        const int vertex_number = name_to_id.size();
        Graph::DirectedWeightedGraph<double> weighted_graph(vertex_number);

        for (const auto &stop: stops) {
            {
                string from = stop.first;
                string to = stop.first + ":wait";

                weighted_graph.AddEdge(
                        {name_to_id[from], name_to_id[to], routing_settings.bus_wait_time});
                const auto curr_edge_id = weighted_graph.GetEdgeCount() - 1;
                id_to_raw_edge[curr_edge_id] = {
                        "Wait", stop.first, routing_settings.bus_wait_time, 0, from, to, 0};
            }
            {
                string from = stop.first + ":wait";
                string to = stop.first;

                weighted_graph.AddEdge({name_to_id[from], name_to_id[to], 0});
                const auto curr_edge_id = weighted_graph.GetEdgeCount() - 1;
                id_to_raw_edge[curr_edge_id] = {"-", from + "_" + to, 0, 0, from, to,
                                                0};
            }
        }

        auto add_edge = [&](const string &from, const string &to,
                            const string &bus_name, double len, int span_count) {
            auto from_id = name_to_id[from];
            auto to_id = name_to_id[to];
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
                    len};
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
                    add_edge(from_name, to_name, bus_name, total_len, abs(i - j));
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
                        add_edge(from_name, to_name, bus_name, total_len, abs(i - j));
                    }
                }
            }
        }

        graph = make_unique<Graph::DirectedWeightedGraph<double>>(weighted_graph);
    }

public:
    map<string, LatLng> stops;
    map<pair<string, string>, double> edge_len;
    unordered_map<string, set<string>> stop_buses;
    map<string, BusRoute> bus_routes;
    vector<string> responses;
    unique_ptr<Graph::DirectedWeightedGraph<double>> graph;
    unordered_map<string, size_t> name_to_id;
    unordered_map<size_t, string> id_to_name;
    unordered_map<size_t, RawEdge> id_to_raw_edge;
    unordered_map<string, set<string>> edges_from_stop;

    double GetRealDistance(const string &from, const string &to) {
        edges_from_stop[from].insert(to);
        edges_from_stop[to].insert(from);
        if (edge_len.count({from, to})) {
            return edge_len.at({from, to});
        }
        return 0.0;
    }

    CoordinateCompressionParams MapLngLatToIds(const RenderSettings &render_settings) {
        map<double, string> lngToStopName, latToStopName;
        for (const auto &[name, point]: stops) {
            lngToStopName[point.lng] = name;
            latToStopName[point.lat] = name;
        }

        int cntLng = 0;
        map<double, int> lngToId;
        vector<string> stop_names_with_cnt_lng;
        for (auto &[lng, name]: lngToStopName) {
            bool ok = true;
            for (const auto &prev_name: stop_names_with_cnt_lng) {
                if (edges_from_stop[name].count(prev_name)) {
                    ok = false;
                    break;
                }
            }
            if (!ok) {
                cntLng++;
                stop_names_with_cnt_lng.clear();
            }
            lngToId[lng] = cntLng;
            stop_names_with_cnt_lng.push_back(name);
        }

        int cntLat = 0;
        map<double, int> latToId;
        vector<string> stop_names_with_cnt_lat;
        for (auto &[lat, name]: latToStopName) {
            bool ok = true;
            for (const auto &prev_name: stop_names_with_cnt_lat) {
                if (edges_from_stop[name].count(prev_name)) {
                    ok = false;
                    break;
                }
            }
            if (!ok) {
                cntLat++;
                stop_names_with_cnt_lat.clear();
            }
            latToId[lat] = cntLat;
            stop_names_with_cnt_lat.push_back(name);
        }

        double x_step = cntLng < 1 ? 0 : (render_settings.width - 2 * render_settings.padding) /
                                         cntLng;
        double y_step = cntLat < 1 ? 0 : (render_settings.height - 2 * render_settings.padding) /
                                         cntLat;
        return {lngToId, latToId, x_step, y_step};
    }

    [[nodiscard]] pair<pair<double, double>, pair<double, double>>
    FindMinMaxXY() const {
        double min_x = 1000.0;
        double min_y = 1000.0;
        double max_x = -1000.0;
        double max_y = -1000.0;

        for (const auto &[name, point]: stops) {
            min_x = min(min_x, point.lng);
            max_x = max(max_x, point.lng);

            min_y = min(min_y, point.lat);
            max_y = max(max_y, point.lat);
        }

        return {{min_x, max_x},
                {min_y, max_y}};
    }

    static double
    CalcZoomCoef(const RenderSettings &renderSettings,
                 pair<pair<double, double>, pair<double, double>> min_max) {
        auto min_lng = min_max.first.first;
        auto max_lng = min_max.first.second;

        auto min_lat = min_max.second.first;
        auto max_lat = min_max.second.second;

        if (max_lng == min_lng && max_lat == min_lat) {
            return 0.0;
        }
        if (max_lng == min_lng) {
            return (renderSettings.height - 2 * renderSettings.padding) /
                   (max_lat - min_lat);
        }
        if (max_lat == min_lat) {
            return (renderSettings.width - 2 * renderSettings.padding) /
                   (max_lng - min_lng);
        }

        auto width_zoom_coef = (renderSettings.width - 2 * renderSettings.padding) /
                               (max_lng - min_lng);
        auto height_zoom_coef =
                (renderSettings.height - 2 * renderSettings.padding) /
                (max_lat - min_lat);
        return min(width_zoom_coef, height_zoom_coef);
    }
};

int main() {
//    std::ifstream in("input.json");
//    std::cin.rdbuf(in.rdbuf());

//    std::ofstream out("my_out.json");
//    std::streambuf *coutbuf = std::cout.rdbuf();
//    std::cout.rdbuf(out.rdbuf());

    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    auto queries = ParseQueriesFromJson();

    DbManager manager;
    for (const auto &q: queries.stop) {
        manager.AddStop(*q);
    }
    for (const auto &q: queries.bus) {
        manager.AddBusRoute(*q);
    }
    manager.BuildGraph(*queries.routing_settings);
    router =
            make_unique<Graph::Router<double>>(Graph::Router<double>(*manager.graph));
    for (const auto &q: queries.bus_info) {
        manager.GetBusInfoJson(*q);
    }
    for (const auto &q: queries.stop_info) {
        manager.GetStopInfoJson(*q);
    }
    for (const auto &q: queries.route_info) {
        manager.GetRouteInfoJson(*q);
    }
    for (const auto &q: queries.map_info) {
        manager.GetMapInfoJson(*q, *queries.render_settings);
    }

    manager.PrintResponsesToJson(cout);
}
