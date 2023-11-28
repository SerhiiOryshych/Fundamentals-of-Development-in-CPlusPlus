#include "map_renderer.h"
#include "transport_catalog.h"
#include "json.h"
#include "sphere.h"
#include "sphere_serialize.h"
#include "svg_serialize.h"

#include <algorithm>
#include <iterator>
#include <sstream>

using namespace std;


static Svg::Point ParsePoint(const Json::Node &json) {
    const auto &array = json.AsArray();
    return {
            array[0].AsDouble(),
            array[1].AsDouble()
    };
}

static Svg::Color ParseColor(const Json::Node &json) {
    if (json.IsString()) {
        return json.AsString();
    }
    const auto &array = json.AsArray();
    assert(array.size() == 3 || array.size() == 4);
    Svg::Rgb rgb{
            static_cast<uint8_t>(array[0].AsInt()),
            static_cast<uint8_t>(array[1].AsInt()),
            static_cast<uint8_t>(array[2].AsInt())
    };
    if (array.size() == 3) {
        return rgb;
    } else {
        return Svg::Rgba{rgb, array[3].AsDouble()};
    }
}

static vector<Svg::Color> ParseColors(const Json::Node &json) {
    const auto &array = json.AsArray();
    vector<Svg::Color> colors;
    colors.reserve(array.size());
    transform(begin(array), end(array), back_inserter(colors), ParseColor);
    return colors;
}

RenderSettings ParseRenderSettings(const Json::Dict &json) {
    RenderSettings result;
    result.max_width = json.at("width").AsDouble();
    result.max_height = json.at("height").AsDouble();
    result.padding = json.at("padding").AsDouble();
    result.outer_margin = json.at("outer_margin").AsDouble();
    result.palette = ParseColors(json.at("color_palette"));
    result.line_width = json.at("line_width").AsDouble();
    result.underlayer_color = ParseColor(json.at("underlayer_color"));
    result.underlayer_width = json.at("underlayer_width").AsDouble();
    result.stop_radius = json.at("stop_radius").AsDouble();
    result.bus_label_offset = ParsePoint(json.at("bus_label_offset"));
    result.bus_label_font_size = json.at("bus_label_font_size").AsInt();
    result.stop_label_offset = ParsePoint(json.at("stop_label_offset"));
    result.stop_label_font_size = json.at("stop_label_font_size").AsInt();
    result.company_radius = json.at("company_radius").AsDouble();
    result.company_line_width = json.at("company_line_width").AsDouble();

    const auto &layers_array = json.at("layers").AsArray();
    result.layers.reserve(layers_array.size());
    for (const auto &layer_node: layers_array) {
        result.layers.push_back(layer_node.AsString());
    }

    return result;
}

static map<string, Descriptions::Bus> CopyBusesDict(const Descriptions::BusesDict &source) {
    map<string, Descriptions::Bus> target;
    for (const auto &[name, data_ptr]: source) {
        target.emplace(name, *data_ptr);
    }
    return target;
}

static unordered_set<string> FindSupportStops(const Descriptions::BusesDict &buses_dict) {
    unordered_set<string> support_stops;
    unordered_map<string, const Descriptions::Bus *> stops_first_bus;
    unordered_map<string, int> stops_rank;
    for (const auto &[_, bus_ptr]: buses_dict) {
        for (const string &stop: bus_ptr->endpoints) {
            support_stops.insert(stop);
        }
        for (const string &stop: bus_ptr->stops) {
            ++stops_rank[stop];
            const auto [it, inserted] = stops_first_bus.emplace(stop, bus_ptr);
            if (!inserted && it->second != bus_ptr) {
                support_stops.insert(stop);
            }
        }
    }

    for (const auto &[stop, rank]: stops_rank) {
        if (rank > 2) {
            support_stops.insert(stop);
        }
    }

    return support_stops;
}

static unordered_map<string, Sphere::Point> ComputeInterpolatedStopsGeoCoords(
        const Descriptions::StopsDict &stops_dict,
        const Descriptions::BusesDict &buses_dict,
        const YellowPages::Database &yellow_pages_db
) {
    const unordered_set<string> support_stops = FindSupportStops(buses_dict);

    unordered_map<string, Sphere::Point> stop_points;
    for (const auto &[stop_name, stop_ptr]: stops_dict) {
        stop_points[stop_name] = stop_ptr->position;
    }

    // add companies
    for (int i = 0; i < yellow_pages_db.companies_size(); i++) {
        const auto &company = yellow_pages_db.companies(i);
        stop_points["company__" + std::to_string(i)] = {company.address().coords().lat(),
                                                        company.address().coords().lon()};
    }

    for (const auto &[_, bus_ptr]: buses_dict) {
        const auto &stops = bus_ptr->stops;
        if (stops.empty()) {
            continue;
        }
        size_t last_support_idx = 0;
        stop_points[stops[0]] = stops_dict.at(stops[0])->position;
        for (size_t stop_idx = 1; stop_idx < stops.size(); ++stop_idx) {
            if (support_stops.count(stops[stop_idx])) {
                const Sphere::Point prev_point = stops_dict.at(stops[last_support_idx])->position;
                const Sphere::Point next_point = stops_dict.at(stops[stop_idx])->position;
                const double lat_step = (next_point.latitude - prev_point.latitude) / (stop_idx - last_support_idx);
                const double lon_step = (next_point.longitude - prev_point.longitude) / (stop_idx - last_support_idx);
                for (size_t middle_stop_idx = last_support_idx + 1; middle_stop_idx < stop_idx; ++middle_stop_idx) {
                    stop_points[stops[middle_stop_idx]] = {
                            prev_point.latitude + lat_step * (middle_stop_idx - last_support_idx),
                            prev_point.longitude + lon_step * (middle_stop_idx - last_support_idx),
                    };
                }
                stop_points[stops[stop_idx]] = stops_dict.at(stops[stop_idx])->position;
                last_support_idx = stop_idx;
            }
        }
    }

    return stop_points;
}

struct NeighboursDicts {
    unordered_map<double, vector<double>> neighbour_lats;
    unordered_map<double, vector<double>> neighbour_lons;
};

static NeighboursDicts BuildCoordNeighboursDicts(const unordered_map<string, Sphere::Point> &stop_points,
                                                 const Descriptions::BusesDict &buses_dict,
                                                 const YellowPages::Database &yellow_pages_db) {
    unordered_map<double, vector<double>> neighbour_lats;
    unordered_map<double, vector<double>> neighbour_lons;

    auto add_points = [&neighbour_lats, &neighbour_lons](Sphere::Point lhs, Sphere::Point rhs) {
        const auto [min_lat, max_lat] = minmax(lhs.latitude, rhs.latitude);
        const auto [min_lon, max_lon] = minmax(lhs.longitude, rhs.longitude);
        neighbour_lats[max_lat].push_back(min_lat);
        neighbour_lons[max_lon].push_back(min_lon);
    };

    for (const auto &[bus_name, bus_ptr]: buses_dict) {
        const auto &stops = bus_ptr->stops;
        if (stops.empty()) {
            continue;
        }
        Sphere::Point point_prev = stop_points.at(stops[0]);
        for (size_t stop_idx = 1; stop_idx < stops.size(); ++stop_idx) {
            const auto point_cur = stop_points.at(stops[stop_idx]);
            if (stops[stop_idx] != stops[stop_idx - 1]) {
                add_points(point_prev, point_cur);
            }
            point_prev = point_cur;
        }
    }

    // add companies
    for (int i = 0; i < yellow_pages_db.companies_size(); i++) {
        const auto &company = yellow_pages_db.companies(i);
        const auto company_coords = stop_points.at("company__" + std::to_string(i));
        for (const auto &nearby_stop: company.nearby_stops()) {
            add_points(company_coords, stop_points.at(nearby_stop.name()));
        }
    }

    for (auto *neighbours_dict: {&neighbour_lats, &neighbour_lons}) {
        for (auto &[_, values]: *neighbours_dict) {
            sort(begin(values), end(values));
            values.erase(unique(begin(values), end(values)), end(values));
        }
    }

    return {std::move(neighbour_lats), std::move(neighbour_lons)};
}

static vector<Sphere::Point> ExtractPoints(const unordered_map<string, Sphere::Point> &stop_points) {
    vector<Sphere::Point> points;
    points.reserve(stop_points.size());
    for (const auto &[_, point]: stop_points) {
        points.push_back(point);
    }
    return points;
}

class CoordsCompressor {
public:
    CoordsCompressor(const vector<Sphere::Point> &points) {
        for (const auto &point: points) {
            lats_.push_back({point.latitude});
            lons_.push_back({point.longitude});
        }

        sort(begin(lats_), end(lats_));
        sort(begin(lons_), end(lons_));
    }

    void FillIndices(const unordered_map<double, vector<double>> &neighbour_lats,
                     const unordered_map<double, vector<double>> &neighbour_lons) {
        FillCoordIndices(lats_, neighbour_lats);
        FillCoordIndices(lons_, neighbour_lons);
    }

    void FillTargets(double max_width, double max_height, double padding) {
        if (lats_.empty() || lons_.empty()) {
            return;
        }

        const size_t max_lat_idx = FindMaxLatIdx();
        const double y_step = max_lat_idx ? (max_height - 2 * padding) / max_lat_idx : 0;

        const size_t max_lon_idx = FindMaxLonIdx();
        const double x_step = max_lon_idx ? (max_width - 2 * padding) / max_lon_idx : 0;

        for (auto &[_, idx, value]: lats_) {
            value = max_height - padding - idx * y_step;
        }
        for (auto &[_, idx, value]: lons_) {
            value = idx * x_step + padding;
        }
    }

    double MapLat(double value) const {
        return Find(lats_, value).target;
    }

    double MapLon(double value) const {
        return Find(lons_, value).target;
    }

private:
    struct CoordInfo {
        double source;
        size_t idx = 0;
        double target = 0;

        bool operator<(const CoordInfo &other) const {
            return source < other.source;
        }
    };

    vector<CoordInfo> lats_;
    vector<CoordInfo> lons_;

    void FillCoordIndices(vector<CoordInfo> &coords, const unordered_map<double, vector<double>> &neighbour_values) {
        for (auto coord_it = begin(coords); coord_it != end(coords); ++coord_it) {
            const auto neighbours_it = neighbour_values.find(coord_it->source);
            if (neighbours_it == neighbour_values.end()) {
                coord_it->idx = 0;
                continue;
            }
            const auto &neighbours = neighbours_it->second;
            optional<size_t> max_neighbour_idx;
            for (const double value: neighbours) {
                const size_t idx = Find(coords, value, coord_it).idx;
                if (idx > max_neighbour_idx) {
                    max_neighbour_idx = idx;
                }
            }
            coord_it->idx = *max_neighbour_idx + 1;
        }
    }

    static const CoordInfo &Find(const vector<CoordInfo> &sorted_values,
                                 double value,
                                 optional<vector<CoordInfo>::const_iterator> end_it = nullopt) {
        return *lower_bound(begin(sorted_values), end_it.value_or(end(sorted_values)), CoordInfo{value});
    }

    static size_t FindMaxIdx(const vector<CoordInfo> &coords) {
        return max_element(
                begin(coords), end(coords),
                [](const CoordInfo &lhs, const CoordInfo &rhs) {
                    return lhs.idx < rhs.idx;
                }
        )->idx;
    }

    size_t FindMaxLatIdx() const {
        return FindMaxIdx(lats_);
    }

    size_t FindMaxLonIdx() const {
        return FindMaxIdx(lons_);
    }
};

static map<string, Svg::Point> ComputeObjectsCoordsByGrid(
        const Descriptions::StopsDict &stops_dict,
        const Descriptions::BusesDict &buses_dict,
        const RenderSettings &render_settings,
        const YellowPages::Database &yellow_pages_db
) {
    const auto stop_points = ComputeInterpolatedStopsGeoCoords(stops_dict, buses_dict, yellow_pages_db);

    const auto [neighbour_lats, neighbour_lons] = BuildCoordNeighboursDicts(stop_points, buses_dict, yellow_pages_db);

    CoordsCompressor compressor(ExtractPoints(stop_points));
    compressor.FillIndices(neighbour_lats, neighbour_lons);
    compressor.FillTargets(render_settings.max_width, render_settings.max_height, render_settings.padding);

    map<string, Svg::Point> stop_map_points;

    for (const auto &[stop_name, point]: stop_points) {
        stop_map_points[stop_name] =
                {compressor.MapLon(point.longitude), compressor.MapLat(point.latitude)};
    }

    return stop_map_points;
}

static unordered_map<string, Svg::Color> ChooseBusColors(const Descriptions::BusesDict &buses_dict,
                                                         const RenderSettings &render_settings) {
    const auto &palette = render_settings.palette;
    unordered_map<string, Svg::Color> bus_colors;
    int idx = 0;
    for (const auto &[bus_name, bus_ptr]: buses_dict) {
        bus_colors[bus_name] = palette[idx++ % palette.size()];
    }
    return bus_colors;
}

MapRenderer::MapRenderer(const Descriptions::StopsDict &stops_dict,
                         const Descriptions::BusesDict &buses_dict,
                         const Json::Dict &render_settings_json,
                         const YellowPages::Database &yellow_pages_db)
        : render_settings_(ParseRenderSettings(render_settings_json)),
          stop_points_(ComputeObjectsCoordsByGrid(stops_dict, buses_dict, render_settings_, yellow_pages_db)),
          bus_colors_(ChooseBusColors(buses_dict, render_settings_)),
          buses_dict_(CopyBusesDict(buses_dict)) {
}

void RenderSettings::Serialize(TCProto::RenderSettings &proto) const {
    proto.set_max_width(max_width);
    proto.set_max_height(max_height);
    proto.set_padding(padding);
    proto.set_outer_margin(outer_margin);

    for (const Svg::Color &color: palette) {
        Svg::SerializeColor(color, *proto.add_palette());
    }

    proto.set_line_width(line_width);
    SerializeColor(underlayer_color, *proto.mutable_underlayer_color());
    proto.set_underlayer_width(underlayer_width);
    proto.set_stop_radius(stop_radius);
    Svg::SerializePoint(bus_label_offset, *proto.mutable_bus_label_offset());
    proto.set_bus_label_font_size(bus_label_font_size);
    Svg::SerializePoint(stop_label_offset, *proto.mutable_stop_label_offset());
    proto.set_stop_label_font_size(stop_label_font_size);
    proto.set_company_radius(company_radius);
    proto.set_company_line_width(company_line_width);

    for (const string &layer: layers) {
        proto.add_layers(layer);
    }
}

RenderSettings RenderSettings::Deserialize(const TCProto::RenderSettings &proto) {
    RenderSettings settings;
    settings.max_width = proto.max_width();
    settings.max_height = proto.max_height();
    settings.padding = proto.padding();
    settings.outer_margin = proto.outer_margin();

    settings.palette.reserve(proto.palette_size());
    for (const auto &color: proto.palette()) {
        settings.palette.push_back(Svg::DeserializeColor(color));
    }

    settings.line_width = proto.line_width();
    settings.underlayer_color = Svg::DeserializeColor(proto.underlayer_color());
    settings.underlayer_width = proto.underlayer_width();
    settings.stop_radius = proto.stop_radius();
    settings.bus_label_offset = Svg::DeserializePoint(proto.bus_label_offset());
    settings.bus_label_font_size = proto.bus_label_font_size();
    settings.stop_label_offset = Svg::DeserializePoint(proto.stop_label_offset());
    settings.stop_label_font_size = proto.stop_label_font_size();
    settings.company_radius = proto.company_radius();
    settings.company_line_width = proto.company_line_width();

    settings.layers.reserve(proto.layers_size());
    for (const auto &layer: proto.layers()) {
        settings.layers.push_back(layer);
    }

    return settings;
}

void MapRenderer::Serialize(TCProto::MapRenderer &proto, YellowPages::Database &yellow_pages_db) {
    render_settings_.Serialize(*proto.mutable_render_settings());

    for (int i = 0; i < yellow_pages_db.companies_size(); i++) {
        auto company = yellow_pages_db.mutable_companies(i);
        const auto new_coords = stop_points_.at("company__" + to_string(i));
        company->mutable_address()->mutable_coords()->set_lat(new_coords.x);
        company->mutable_address()->mutable_coords()->set_lon(new_coords.y);
    }

    for (const auto &[name, point]: stop_points_) {
        if (name.compare(0, 9, "company__") == 0) {
            continue;
        }

        auto &stop_point_proto = *proto.add_stop_points();
        stop_point_proto.set_name(name);
        Svg::SerializePoint(point, *stop_point_proto.mutable_point());
    }

    for (const auto &[name, color]: bus_colors_) {
        auto &bus_color_proto = *proto.add_bus_colors();
        bus_color_proto.set_name(name);
        Svg::SerializeColor(color, *bus_color_proto.mutable_color());
    }

    for (const auto &[_, bus]: buses_dict_) {
        bus.Serialize(*proto.add_bus_descriptions());
    }
}

std::unique_ptr<MapRenderer>
MapRenderer::Deserialize(const TCProto::MapRenderer &proto, const YellowPages::Database &yellow_pages_db) {
    std::unique_ptr<MapRenderer> renderer_holder(new MapRenderer);
    auto &renderer = *renderer_holder;

    renderer.render_settings_ = RenderSettings::Deserialize(proto.render_settings());

    for (const auto &stop_point_proto: proto.stop_points()) {
        renderer.stop_points_.emplace(stop_point_proto.name(), Svg::DeserializePoint(stop_point_proto.point()));
    }

    // add company_points
    for (const auto &company: yellow_pages_db.companies()) {
        renderer.company_points_[company.id()] = {company.address().coords().lat(), company.address().coords().lon()};
    }

    for (const auto &bus_color_proto: proto.bus_colors()) {
        renderer.bus_colors_.emplace(bus_color_proto.name(), Svg::DeserializeColor(bus_color_proto.color()));
    }

    for (const auto &bus_proto: proto.bus_descriptions()) {
        renderer.buses_dict_.emplace(bus_proto.name(), Descriptions::Bus::Deserialize(bus_proto));
    }

    return renderer_holder;
}

using RouteBusItem = TransportRouter::RouteInfo::RideBusItem;
using RouteWaitItem = TransportRouter::RouteInfo::WaitBusItem;
using RouteWalkItem = TransportRouter::RouteInfo::WalkToCompanyItem;

void MapRenderer::RenderCompanyLines(Svg::Document &svg) const {}

void MapRenderer::RenderCompanyPoints(Svg::Document &svg) const {}

void MapRenderer::RenderCompanyLabels(Svg::Document &svg) const {}

void MapRenderer::RenderBusLines(Svg::Document &svg) const {
    for (const auto &[bus_name, bus]: buses_dict_) {
        const auto &stops = bus.stops;
        if (stops.empty()) {
            continue;
        }
        Svg::Polyline line;
        line.SetStrokeColor(bus_colors_.at(bus_name))
                .SetStrokeWidth(render_settings_.line_width)
                .SetStrokeLineCap("round").SetStrokeLineJoin("round");
        for (const auto &stop_name: stops) {
            line.AddPoint(stop_points_.at(stop_name));
        }
        svg.Add(line);
    }
}

void MapRenderer::RenderRouteCompanyLines(Svg::Document &svg, const TransportRouter::RouteInfo &route) const {
    for (const auto &item: route.items) {
        if (!holds_alternative<RouteWalkItem>(item)) {
            continue;
        }
        const auto &walk_item = get<RouteWalkItem>(item);
        const string &stop_name = walk_item.stop_name;
        int company_id = walk_item.company_id;

        Svg::Polyline line;
        line.SetStrokeColor("black")
                .SetStrokeWidth(render_settings_.company_line_width)
                .SetStrokeLineCap("round").SetStrokeLineJoin("round");
        line.AddPoint(stop_points_.at(stop_name));
        line.AddPoint(company_points_.at(company_id));
        svg.Add(line);
    }
}

void MapRenderer::RenderRouteCompanyPoint(Svg::Document &svg, const TransportRouter::RouteInfo &route) const {
    for (const auto &item: route.items) {
        if (!holds_alternative<RouteWalkItem>(item)) {
            continue;
        }
        const auto &walk_item = get<RouteWalkItem>(item);
        svg.Add(Svg::Circle{}
                        .SetCenter(company_points_.at(walk_item.company_id))
                        .SetRadius(render_settings_.company_radius)
                        .SetFillColor("black"));
    }
}

void MapRenderer::RenderRouteCompanyLabels(Svg::Document &svg, const TransportRouter::RouteInfo &route) const {
    for (const auto &item: route.items) {
        if (!holds_alternative<RouteWalkItem>(item)) {
            continue;
        }
        const auto &walk_item = get<RouteWalkItem>(item);
        RenderLabel(svg, company_points_.at(walk_item.company_id), walk_item.company_full_name);
    }
}

void MapRenderer::RenderRouteBusLines(Svg::Document &svg, const TransportRouter::RouteInfo &route) const {
    for (const auto &item: route.items) {
        if (!holds_alternative<RouteBusItem>(item)) {
            continue;
        }
        const auto &bus_item = get<RouteBusItem>(item);
        const string &bus_name = bus_item.bus_name;
        const auto &stops = buses_dict_.at(bus_name).stops;
        if (stops.empty()) {
            continue;
        }
        Svg::Polyline line;
        line.SetStrokeColor(bus_colors_.at(bus_name))
                .SetStrokeWidth(render_settings_.line_width)
                .SetStrokeLineCap("round").SetStrokeLineJoin("round");
        for (size_t stop_idx = bus_item.start_stop_idx; stop_idx <= bus_item.finish_stop_idx; ++stop_idx) {
            const string &stop_name = stops[stop_idx];
            line.AddPoint(stop_points_.at(stop_name));
        }
        svg.Add(line);
    }
}

void MapRenderer::RenderBusLabel(Svg::Document &svg, const string &bus_name, const string &stop_name) const {
    const auto &color = bus_colors_.at(bus_name);  // can be optimized a bit by moving upper
    const auto point = stop_points_.at(stop_name);
    const auto base_text =
            Svg::Text{}
                    .SetPoint(point)
                    .SetOffset(render_settings_.bus_label_offset)
                    .SetFontSize(render_settings_.bus_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetFontWeight("bold")
                    .SetData(bus_name);
    svg.Add(
            Svg::Text(base_text)
                    .SetFillColor(render_settings_.underlayer_color)
                    .SetStrokeColor(render_settings_.underlayer_color)
                    .SetStrokeWidth(render_settings_.underlayer_width)
                    .SetStrokeLineCap("round").SetStrokeLineJoin("round")
    );
    svg.Add(
            Svg::Text(base_text)
                    .SetFillColor(color)
    );
}

void MapRenderer::RenderBusLabels(Svg::Document &svg) const {
    for (const auto &[bus_name, bus]: buses_dict_) {
        const auto &stops = bus.stops;
        if (!stops.empty()) {
            for (const string &endpoint: bus.endpoints) {
                RenderBusLabel(svg, bus_name, endpoint);
            }
        }
    }
}

void MapRenderer::RenderRouteBusLabels(Svg::Document &svg, const TransportRouter::RouteInfo &route) const {
    for (const auto &item: route.items) {
        if (!holds_alternative<RouteBusItem>(item)) {
            continue;
        }
        const auto &bus_item = get<RouteBusItem>(item);
        const string &bus_name = bus_item.bus_name;
        const auto &bus = buses_dict_.at(bus_name);
        const auto &stops = bus.stops;
        if (stops.empty()) {
            continue;
        }
        for (const size_t stop_idx: {bus_item.start_stop_idx, bus_item.finish_stop_idx}) {
            const auto stop_name = stops[stop_idx];
            if (stop_idx == 0
                || stop_idx == stops.size() - 1
                || find(begin(bus.endpoints), end(bus.endpoints), stop_name) != end(bus.endpoints)
                    ) {
                RenderBusLabel(svg, bus_name, stop_name);
            }
        }
    }
}

void MapRenderer::RenderStopPoint(Svg::Document &svg, Svg::Point point) const {
    svg.Add(Svg::Circle{}
                    .SetCenter(point)
                    .SetRadius(render_settings_.stop_radius)
                    .SetFillColor("white"));
}

void MapRenderer::RenderStopPoints(Svg::Document &svg) const {
    for (const auto &[_, point]: stop_points_) {
        RenderStopPoint(svg, point);
    }
}

void MapRenderer::RenderRouteStopPoints(Svg::Document &svg, const TransportRouter::RouteInfo &route) const {
    for (const auto &item: route.items) {
        if (!holds_alternative<RouteBusItem>(item)) {
            continue;
        }
        const auto &bus_item = get<RouteBusItem>(item);
        const string &bus_name = bus_item.bus_name;
        const auto &stops = buses_dict_.at(bus_name).stops;
        if (stops.empty()) {
            continue;
        }
        for (size_t stop_idx = bus_item.start_stop_idx; stop_idx <= bus_item.finish_stop_idx; ++stop_idx) {
            const string &stop_name = stops[stop_idx];
            RenderStopPoint(svg, stop_points_.at(stop_name));
        }
    }
}

void MapRenderer::RenderLabel(Svg::Document &svg, Svg::Point point, const string &name) const {
    auto base_text =
            Svg::Text{}
                    .SetPoint(point)
                    .SetOffset(render_settings_.stop_label_offset)
                    .SetFontSize(render_settings_.stop_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetData(name);
    svg.Add(
            Svg::Text(base_text)
                    .SetFillColor(render_settings_.underlayer_color)
                    .SetStrokeColor(render_settings_.underlayer_color)
                    .SetStrokeWidth(render_settings_.underlayer_width)
                    .SetStrokeLineCap("round").SetStrokeLineJoin("round")
    );
    svg.Add(
            base_text
                    .SetFillColor("black")
    );
}

void MapRenderer::RenderStopLabels(Svg::Document &svg) const {
    for (const auto &[name, point]: stop_points_) {
        RenderLabel(svg, point, name);
    }
}

void MapRenderer::RenderRouteStopLabels(Svg::Document &svg, const TransportRouter::RouteInfo &route) const {
    auto route_without_walk = route;
    if (!route.items.empty() && holds_alternative<RouteWalkItem>(route.items.back())) {
        if (route.items.size() == 1) {
            const auto &walk_item = get<RouteWalkItem>(route.items.back());
            RenderLabel(svg, stop_points_.at(walk_item.stop_name), walk_item.stop_name);
            return;
        }
        route_without_walk.items.pop_back();
    }

    if (route_without_walk.items.empty()) {
        return;
    }

    for (const auto &item: route_without_walk.items) {
        if (!holds_alternative<RouteWaitItem>(item)) {
            continue;
        }
        const auto &wait_item = get<RouteWaitItem>(item);
        const string &stop_name = wait_item.stop_name;
        RenderLabel(svg, stop_points_.at(stop_name), stop_name);
    }

    auto last_item_it = route_without_walk.items.crbegin();
    while (!holds_alternative<RouteBusItem>(*last_item_it)) {
        ++last_item_it;
    }

    // draw stop label for last stop
    const auto &last_bus_item = get<RouteBusItem>(*last_item_it);
    const string &last_stop_name = buses_dict_.at(last_bus_item.bus_name).stops[last_bus_item.finish_stop_idx];
    RenderLabel(svg, stop_points_.at(last_stop_name), last_stop_name);
}

const unordered_map<
        string,
        void (MapRenderer::*)(Svg::Document &) const
> MapRenderer::MAP_LAYER_ACTIONS = {
        {"bus_lines",      &MapRenderer::RenderBusLines},
        {"bus_labels",     &MapRenderer::RenderBusLabels},
        {"stop_points",    &MapRenderer::RenderStopPoints},
        {"stop_labels",    &MapRenderer::RenderStopLabels},
        {"company_lines",  &MapRenderer::RenderCompanyLines},
        {"company_points", &MapRenderer::RenderCompanyPoints},
        {"company_labels", &MapRenderer::RenderCompanyLabels}
};

const unordered_map<
        string,
        void (MapRenderer::*)(Svg::Document &, const TransportRouter::RouteInfo &) const
> MapRenderer::ROUTE_LAYER_ACTIONS = {
        {"bus_lines",      &MapRenderer::RenderRouteBusLines},
        {"bus_labels",     &MapRenderer::RenderRouteBusLabels},
        {"stop_points",    &MapRenderer::RenderRouteStopPoints},
        {"stop_labels",    &MapRenderer::RenderRouteStopLabels},
        {"company_lines",  &MapRenderer::RenderRouteCompanyLines},
        {"company_points", &MapRenderer::RenderRouteCompanyPoint},
        {"company_labels", &MapRenderer::RenderRouteCompanyLabels}
};

Svg::Document MapRenderer::Render() const {
    Svg::Document svg;

    for (const auto &layer: render_settings_.layers) {
        (this->*MAP_LAYER_ACTIONS.at(layer))(svg);
    }

    return svg;
}

Svg::Document MapRenderer::RenderRoute(
        Svg::Document svg,
        const TransportRouter::RouteInfo &route
) const {

    const double outer_margin = render_settings_.outer_margin;
    svg.Add(
            Svg::Rectangle{}
                    .SetFillColor(render_settings_.underlayer_color)
                    .SetTopLeftPoint({-outer_margin, -outer_margin})
                    .SetBottomRightPoint({
                                                 render_settings_.max_width + outer_margin,
                                                 render_settings_.max_height + outer_margin
                                         })
    );

    for (const auto &layer: render_settings_.layers) {
        (this->*ROUTE_LAYER_ACTIONS.at(layer))(svg, route);
    }

    return svg;
}
