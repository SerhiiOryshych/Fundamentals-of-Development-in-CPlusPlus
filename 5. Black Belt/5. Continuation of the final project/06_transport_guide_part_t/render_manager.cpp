#include "render_manager.h"
#include "data_manager.h"

#include <cmath>
#include <memory>

Data_Structure::MapRespType Data_Structure::DataBaseSvgBuilder::RenderMap() const {
    MapRespType MapResp = std::make_shared<MapResponse>();
    MapResp->svg_xml_answer = doc.Get();
    return MapResp;
}

Svg::Document Data_Structure::DataBaseSvgBuilder::GenerateRouteSVG(std::vector<RouteResponse::ItemPtr> const &items) {
    Svg::Document route_doc(doc);

    Svg::Rect rect;
    auto left_corner = Svg::Point{-renderSettings.outer_margin, -renderSettings.outer_margin};
    rect.SetFillColor(renderSettings.underlayer_color)
            .SetPoint(left_corner)
            .SetWidth(renderSettings.width + renderSettings.outer_margin - left_corner.x)
            .SetHeight(renderSettings.height + renderSettings.outer_margin - left_corner.y);
    route_doc.Add(rect);

    return std::move(route_doc);
}

Data_Structure::MapRespType
Data_Structure::DataBaseSvgBuilder::RenderRoute(std::vector<RouteResponse::ItemPtr> const &items,
                                                DbItemIdNameMap &db_item_id_name_map) {
    Svg::Document route_doc = GenerateRouteSVG(items);

    if (!items.empty()) {
        std::vector<std::string> route_coords;
        std::vector<std::pair<int, size_t>> used_bus;
        for (auto &item: items) {
            if (item->type == RouteResponse::Item::ItemType::WAIT)
                route_coords.push_back(item->name);
            else
                used_bus.emplace_back(
                        std::pair{db_item_id_name_map.GetIdByName(item->name),
                                  reinterpret_cast<RouteResponse::Bus *>(item.get())->span_count});
        }
        for (const auto &layer: renderSettings.layers) {
            if (layersStrategy.count(layer))
                (layersStrategy[layer])->DrawPartial(route_coords, used_bus, route_doc, db_item_id_name_map);
        }
    }

    route_doc.SimpleRender();
    MapRespType MapResp = std::make_shared<MapResponse>();
    MapResp->svg_xml_answer = route_doc.Get();
    return MapResp;
}

Data_Structure::MapRespType
Data_Structure::DataBaseSvgBuilder::RenderPathToCompany(std::vector<RouteResponse::ItemPtr> const &items,
                                                        const std::string &company,
                                                        DbItemIdNameMap &db_item_id_name_map) {
    Svg::Document route_doc = GenerateRouteSVG(items);

    if (!items.empty()) {
        std::vector<std::string> route_coords;
        std::vector<std::pair<int, size_t>> used_bus;
        for (auto &item: items) {
            if (item->type == RouteResponse::Item::ItemType::WAIT)
                route_coords.push_back(item->name);
            else
                used_bus.emplace_back(
                        std::pair{db_item_id_name_map.GetIdByName(item->name),
                                  reinterpret_cast<RouteResponse::Bus *>(item.get())->span_count});
        }
        for (const auto &layer: renderSettings.layers) {
            if (layer.find("company") != std::string::npos)
                (CompanyRouteLayersStrategy[layer])->DrawPartial(items.back()->name, company, route_doc,
                                                                 db_item_id_name_map);
            else
                (CompanyRouteLayersStrategy[layer])->DrawPartial(route_coords, used_bus, route_doc,
                                                                 db_item_id_name_map);
        }
    }

    route_doc.SimpleRender();
    MapRespType MapResp = std::make_shared<MapResponse>();
    MapResp->svg_xml_answer = route_doc.Get();
    return MapResp;
}

Data_Structure::DataBaseSvgBuilder::DataBaseSvgBuilder(Data_Structure::RenderSettings render_set) : renderSettings(
        std::move(render_set)) {}

Data_Structure::DataBaseSvgBuilder::DataBaseSvgBuilder(const RenderProto::RenderSettings &ren_set,
                                                       const std::unordered_map<int, Stop> &stops,
                                                       const std::unordered_map<int, Bus> &buses,
                                                       const std::unordered_map<std::string, const YellowPages::Company *> &companies,
                                                       DbItemIdNameMap &db_item_id_name_map) {
    std::unordered_map<std::string, stop_n_companies> points;
    for (auto &stop: stops)
        points.emplace(std::piecewise_construct, std::forward_as_tuple(db_item_id_name_map.GetNameById(stop.first)),
                       std::forward_as_tuple(stop.second));
    for (auto &company: companies)
        points.emplace(std::piecewise_construct, std::forward_as_tuple(company.first),
                       std::forward_as_tuple(*company.second));

    Deserialize(ren_set);
    CalculateCoordinates(points, buses, db_item_id_name_map);
    Init(buses, db_item_id_name_map);
    for (const auto &layer: renderSettings.layers) {
        if (layersStrategy.count(layer))
            (layersStrategy[layer])->Draw(db_item_id_name_map);
    }

    doc.SimpleRender();
}

std::map<std::string, Svg::Point> Data_Structure::DataBaseSvgBuilder::CoordinateUniformDistribution(
        const std::unordered_map<std::string, stop_n_companies> &stops,
        const std::unordered_map<int, Bus> &buses,
        DbItemIdNameMap &db_item_id_name_map) {
    auto bearing_points = GetBearingPoints(stops, buses, db_item_id_name_map);

    auto step = [](double left, double right, size_t count) {
        return (right - left) / count;
    };

    std::map<std::string, Svg::Point> uniform;
    for (auto &[_, bus]: buses) {
        if (bus.stops.empty()) continue;
        auto left_bearing_point = bus.stops.begin();
        size_t l = 0;
        size_t k = l;
        auto right_bearing_point = left_bearing_point;
        size_t count;
        auto bus_range = bus.is_roundtrip ? Ranges::AsRange(bus.stops) : Ranges::ToMiddle(Ranges::AsRange(bus.stops));
        for (auto stop_iter = bus_range.begin(); stop_iter != bus_range.end(); stop_iter++) {
            count = std::distance(left_bearing_point, right_bearing_point);
            if (stop_iter == right_bearing_point) {
                left_bearing_point = right_bearing_point;
                l = k;
                right_bearing_point = std::find_if(std::next(left_bearing_point), bus_range.end(),
                                                   [&bearing_points, db_item_id_name_map](auto const &cur) {
                                                       return bearing_points.find(
                                                               db_item_id_name_map.GetNameById(cur)) !=
                                                              bearing_points.end();
                                                   });
                Distance left_bearing_distance;
                if (std::holds_alternative<Stop>(stops.at(db_item_id_name_map.GetNameById(*left_bearing_point)))) {
                    left_bearing_distance = std::get<Stop>(
                            stops.at(db_item_id_name_map.GetNameById(*left_bearing_point))).dist;
                } else {
                    auto &address = std::get<YellowPages::Company>(
                            stops.at(db_item_id_name_map.GetNameById(*left_bearing_point))).address();
                    left_bearing_distance = Distance{address.coords().lon(), address.coords().lat()};
                }
                uniform.insert_or_assign(db_item_id_name_map.GetNameById(*left_bearing_point),
                                         Svg::Point{left_bearing_distance.GetLongitude(),
                                                    left_bearing_distance.GetLatitude()});
            } else {
                if (right_bearing_point == bus_range.end())
                    throw std::logic_error("bad right point!");

                Distance left_bearing_distance;
                Distance right_bearing_distance;
                if (std::holds_alternative<Stop>(stops.at(db_item_id_name_map.GetNameById(*left_bearing_point)))) {
                    left_bearing_distance = std::get<Stop>(
                            stops.at(db_item_id_name_map.GetNameById(*left_bearing_point))).dist;
                    right_bearing_distance = std::get<Stop>(
                            stops.at(db_item_id_name_map.GetNameById(*right_bearing_point))).dist;
                } else {
                    auto &left_address = std::get<YellowPages::Company>(
                            stops.at(db_item_id_name_map.GetNameById(*left_bearing_point))).address();
                    auto &right_address = std::get<YellowPages::Company>(
                            stops.at(db_item_id_name_map.GetNameById(*right_bearing_point))).address();

                    left_bearing_distance = Distance{left_address.coords().lon(), left_address.coords().lat()};
                    right_bearing_distance = Distance{right_address.coords().lon(), right_address.coords().lat()};
                }

                double x = left_bearing_distance.GetLongitude()
                           + step(left_bearing_distance.GetLongitude(),
                                  right_bearing_distance.GetLongitude(), count)
                             * static_cast<double>(k - l);
                double y = left_bearing_distance.GetLatitude()
                           + step(left_bearing_distance.GetLatitude(),
                                  right_bearing_distance.GetLatitude(), count)
                             * static_cast<double>(k - l);
                uniform.insert_or_assign(db_item_id_name_map.GetNameById(*stop_iter), Svg::Point{x, y});
            }
            ++k;
        }
    }

    BuildNeighborhoodConnections(buses, db_item_id_name_map);
    BuildNeighborhoodCompaniesWithStops(stops, db_item_id_name_map);
    return uniform;
}

auto Data_Structure::DataBaseSvgBuilder::SortingByCoordinates(const std::map<std::string, Svg::Point> &uniform,
                                                              const std::unordered_map<std::string, stop_n_companies> &stops) {
    std::pair<std::vector<std::pair<double, std::string>>, std::vector<std::pair<double, std::string>>> sorted_xy;
    auto &sorted_x = sorted_xy.first;
    auto &sorted_y = sorted_xy.second;
    for (auto &[name, point]: stops) {
        auto it = uniform.find(name);
        if (it != uniform.end()) {
            auto &[name, new_coord] = *it;
            sorted_x.emplace_back(new_coord.x, name);
            sorted_y.emplace_back(new_coord.y, name);
        } else {
            if (std::holds_alternative<Stop>(point)) {
                sorted_x.emplace_back(std::get<Stop>(point).dist.GetLongitude(), name);
                sorted_y.emplace_back(std::get<Stop>(point).dist.GetLatitude(), name);
            } else {
                auto address = std::get<YellowPages::Company>(point).address();
                sorted_x.emplace_back(address.coords().lon(), name);
                sorted_y.emplace_back(address.coords().lat(), name);
            }
        }
    }
    std::sort(sorted_x.begin(), sorted_x.end());
    std::sort(sorted_y.begin(), sorted_y.end());
    return std::move(sorted_xy);
}

std::pair<std::map<std::string, int>, int> Data_Structure::DataBaseSvgBuilder::GluingCoordinates(
        const std::vector<std::pair<double, std::string>> &sorted_by_coord) {

    std::map<std::string, int> gluing;
    int idx_max = 0;

    int idx = 0;

    auto refresh = [&idx, this, &gluing](std::string const &stop) {
        int potential_id = 0;
        bool was_checked = false;
        auto it = db_connected.find(stop);
        if (it != db_connected.end()) {
            for (auto &cur_stop: it->second) {
                auto cur_it = gluing.find(cur_stop);
                if (cur_it != gluing.end()) {
                    was_checked = true;
                    potential_id = std::max(potential_id, cur_it->second);
                }
            }
        }
        idx = was_checked ? potential_id + 1 : 0;
    };

    auto beg_of_cur_idx = sorted_by_coord.begin();
    for (const auto &it_x: sorted_by_coord) {
        refresh(it_x.second);
        gluing.emplace(it_x.second, idx);
        if (idx > idx_max)
            idx_max = idx;
    }

    return {std::move(gluing), idx_max};
}

std::pair<std::map<std::string, Svg::Point>, std::map<std::string, Svg::Point>>
Data_Structure::DataBaseSvgBuilder::CoordinateCompression(
        const std::unordered_map<std::string, stop_n_companies> &stops,
        const std::unordered_map<int, Bus> &buses,
        DbItemIdNameMap &db_item_id_name_map) {
    auto [sorted_x, sorted_y] = SortingByCoordinates(CoordinateUniformDistribution(stops, buses, db_item_id_name_map),
                                                     stops);

    auto [gluing_y, yid] = GluingCoordinates(sorted_y);
    auto [gluing_x, xid] = GluingCoordinates(sorted_x);
    double x_step = xid ? (renderSettings.width - 2.f * renderSettings.padding) / xid : 0;
    double y_step = yid ? (renderSettings.height - 2.f * renderSettings.padding) / yid : 0;

    std::map<std::string, double> new_x;
    std::map<std::string, double> new_y;
    for (auto [coord, name]: sorted_y) {
        new_y.emplace(name, renderSettings.height - renderSettings.padding - (gluing_y.at(name)) * y_step);
    }
    for (auto [coord, name]: sorted_x) {
        new_x.emplace(name, (gluing_x.at(name)) * x_step + renderSettings.padding);
    }

    std::map<std::string, Svg::Point> CompressCoordStops;
    std::map<std::string, Svg::Point> CompressCoordCompanies;
    for (auto &[name, obj]: stops) {
        if (std::holds_alternative<Stop>(obj)) {
            CompressCoordStops.emplace(name, Svg::Point{new_x.at(name), new_y.at(name)});
        } else {
            CompressCoordCompanies.emplace(name, Svg::Point{new_x.at(name), new_y.at(name)});
        }
    }
    return {CompressCoordStops, CompressCoordCompanies};
}

void
Data_Structure::DataBaseSvgBuilder::CalculateCoordinates(const std::unordered_map<std::string, stop_n_companies> &stops,
                                                         const std::unordered_map<int, Bus> &buses,
                                                         DbItemIdNameMap &db_item_id_name_map) {
    auto coords = CoordinateCompression(stops, buses, db_item_id_name_map);
    stops_coordinates = coords.first;
    company_coordinates = coords.second;
}

void Data_Structure::DataBaseSvgBuilder::Init(const std::unordered_map<int, Bus> &buses,
                                              DbItemIdNameMap &db_item_id_name_map) {
    for (auto &[bus_id, bus]: buses) {
        bus_dict.emplace(db_item_id_name_map.GetNameById(bus_id), std::pair{bus, Svg::NoneColor});
    }
    size_t size = renderSettings.color_palette.size();
    size_t i = 0;
    for (auto &[bus_id, bus]: bus_dict) {
        bus.second = renderSettings.color_palette[i++ % size];
    }

    layersStrategy.emplace(std::piecewise_construct,
                           std::forward_as_tuple("bus_lines"),
                           std::forward_as_tuple(std::make_shared<BusPolylinesDrawer>(this)));
    layersStrategy.emplace(std::piecewise_construct,
                           std::forward_as_tuple("bus_labels"),
                           std::forward_as_tuple(std::make_shared<BusTextDrawer>(this)));
    layersStrategy.emplace(std::piecewise_construct,
                           std::forward_as_tuple("stop_points"),
                           std::forward_as_tuple(std::make_shared<StopsRoundDrawer>(this)));
    layersStrategy.emplace(std::piecewise_construct,
                           std::forward_as_tuple("stop_labels"),
                           std::forward_as_tuple(std::make_shared<StopsTextDrawer>(this)));
    CompanyRouteLayersStrategy = layersStrategy;
    CompanyRouteLayersStrategy.emplace(std::piecewise_construct,
                                       std::forward_as_tuple("company_lines"),
                                       std::forward_as_tuple(std::make_shared<CompanyPolylinesDrawer>(this)));
    CompanyRouteLayersStrategy.emplace(std::piecewise_construct,
                                       std::forward_as_tuple("company_points"),
                                       std::forward_as_tuple(std::make_shared<CompanyRoundDrawer>(this)));
    CompanyRouteLayersStrategy.emplace(std::piecewise_construct,
                                       std::forward_as_tuple("company_labels"),
                                       std::forward_as_tuple(std::make_shared<CompanyTextDrawer>(this)));

}

void
Data_Structure::DataBaseSvgBuilder::BuildNeighborhoodConnections(const std::unordered_map<int, Bus> &buses,
                                                                 DbItemIdNameMap &db_item_id_name_map) {
    for (auto &[_, bus]: buses) {
        auto &stops_ = bus.stops;
        if (stops_.empty()) continue;
        for (auto stops_it = stops_.begin(); stops_it != std::prev(stops_.end()); stops_it++) {
            db_connected[db_item_id_name_map.GetNameById(*stops_it)].insert(
                    db_item_id_name_map.GetNameById(*std::next(stops_it)));
            db_connected[db_item_id_name_map.GetNameById(*std::next(stops_it))].insert(
                    db_item_id_name_map.GetNameById(*stops_it));
        }
    }
}

bool Data_Structure::DataBaseSvgBuilder::IsConnected(std::string const &lhs, std::string const &rhs,
                                                     std::unordered_map<std::string, std::unordered_set<std::string>> const &db_s) {
    auto it = db_s.find(lhs);
    if (it != db_s.end())
        return it->second.count(rhs);
    else
        return false;
}

void Data_Structure::DataBaseSvgBuilder::BuildNeighborhoodCompaniesWithStops(
        const std::unordered_map<std::string, stop_n_companies> &points,
        DbItemIdNameMap &db_item_id_name_map) {
    for (auto &point: points) {
        if (std::holds_alternative<YellowPages::Company>(point.second)) {
            for (auto &stop: std::get<YellowPages::Company>(point.second).nearby_stops()) {
                db_connected[point.first].emplace(db_item_id_name_map.GetNameById(stop.nearby_stop_id()));
                db_connected[db_item_id_name_map.GetNameById(stop.nearby_stop_id())].emplace(point.first);
            }
        }
    }
}

void Data_Structure::BusPolylinesDrawer::Draw(DbItemIdNameMap &db_item_id_name_map) {
    for (auto &bus_info: db_svg->bus_dict) {
        auto &[bus, color] = bus_info.second;
        auto polyline = Svg::Polyline{}
                .SetStrokeColor(color)
                .SetStrokeWidth(db_svg->renderSettings.line_width)
                .SetStrokeLineJoin("round")
                .SetStrokeLineCap("round");
        for (auto &stop_id: bus.stops) {
            polyline.AddPoint({db_svg->stops_coordinates.at(db_item_id_name_map.GetNameById(stop_id))});
        }
        db_svg->doc.Add(std::move(polyline));
    }
}

void Data_Structure::BusPolylinesDrawer::DrawPartial(std::vector<std::string> &names_stops,
                                                     std::vector<std::pair<int, size_t>> &used_buses,
                                                     Svg::Document &doc,
                                                     DbItemIdNameMap &db_item_id_name_map) const {
    size_t i = 0;
    for (auto stop_it = names_stops.begin(); stop_it != std::prev(names_stops.end()); stop_it++, i++) {
        auto &stops = db_svg->bus_dict.at(db_item_id_name_map.GetNameById(used_buses[i].first)).first.stops;

        auto beg_it = std::find(stops.begin(), stops.end(), db_item_id_name_map.GetIdByName(*stop_it));
        auto end_it = std::find(beg_it, stops.end(), db_item_id_name_map.GetIdByName(*(std::next(stop_it))));
        while (true) {
            if (end_it - beg_it == used_buses[i].second || beg_it == stops.end())
                break;
            else {
                beg_it = std::find(beg_it + 1, stops.end(), db_item_id_name_map.GetIdByName(*stop_it));
                end_it = std::find(beg_it, stops.end(), db_item_id_name_map.GetIdByName(*(std::next(stop_it))));
            }
        }
        if (beg_it == stops.end() || end_it == stops.end())
            throw std::logic_error("invalid response on route request!");

        auto polyline = Svg::Polyline{}
                .SetStrokeColor(db_svg->bus_dict.at(db_item_id_name_map.GetNameById(used_buses[i].first)).second)
                .SetStrokeWidth(db_svg->renderSettings.line_width)
                .SetStrokeLineJoin("round")
                .SetStrokeLineCap("round");
        for (auto first = beg_it; first != std::next(end_it); first++) {
            polyline.AddPoint({db_svg->stops_coordinates.at(db_item_id_name_map.GetNameById(*first))});
        }
        doc.Add(std::move(polyline));
    }
}

void Data_Structure::StopsRoundDrawer::RenderRoundLabel(Svg::Document &doc, const std::string &stop_name) const {
    doc.Add(Svg::Circle{}
                    .SetFillColor("white")
                    .SetRadius(db_svg->renderSettings.stop_radius)
                    .SetCenter(db_svg->stops_coordinates.at(stop_name)));
}


void Data_Structure::StopsRoundDrawer::Draw(DbItemIdNameMap &db_item_id_name_map) {
    for (auto &[stop_name, _]: db_svg->stops_coordinates) {
        RenderRoundLabel(db_svg->doc, stop_name);
    }
}

void
Data_Structure::StopsRoundDrawer::DrawPartial(std::vector<std::string> &names_stops,
                                              std::vector<std::pair<int, size_t>> &used_buses,
                                              Svg::Document &doc,
                                              DbItemIdNameMap &db_item_id_name_map) const {
    size_t i = 0;
    for (auto stop_it = names_stops.begin(); stop_it != std::prev(names_stops.end()); stop_it++, i++) {
        auto &stops = db_svg->bus_dict.at(db_item_id_name_map.GetNameById(used_buses[i].first)).first.stops;

        auto beg_it = std::find(stops.begin(), stops.end(), db_item_id_name_map.GetIdByName(*stop_it));
        auto end_it = std::find(beg_it, stops.end(), db_item_id_name_map.GetIdByName(*(std::next(stop_it))));
        while (true) {
            if (end_it - beg_it == used_buses[i].second || beg_it == stops.end())
                break;
            else {
                beg_it = std::find(beg_it + 1, stops.end(), db_item_id_name_map.GetIdByName(*stop_it));
                end_it = std::find(beg_it, stops.end(), db_item_id_name_map.GetIdByName(*(std::next(stop_it))));
            }
        }
        if (beg_it == stops.end() || end_it == stops.end())
            throw std::logic_error("invalid response on route request!");

        for (auto first = beg_it; first != std::next(end_it); first++) {
            RenderRoundLabel(doc, db_item_id_name_map.GetNameById(*first));
        }
    }
}

void Data_Structure::StopsTextDrawer::RenderStopLabel(Svg::Document &doc, const std::string &stop_name) const {
    Svg::Text text = Svg::Text{}
            .SetPoint(db_svg->stops_coordinates.at(stop_name))
            .SetOffset({db_svg->renderSettings.stop_label_offset[0], db_svg->renderSettings.stop_label_offset[1]})
            .SetFontSize(db_svg->renderSettings.stop_label_font_size)
            .SetFontFamily("Verdana")
            .SetData(stop_name);

    Svg::Text substrates = text;
    substrates
            .SetFillColor(db_svg->renderSettings.underlayer_color)
            .SetStrokeColor(db_svg->renderSettings.underlayer_color)
            .SetStrokeWidth(db_svg->renderSettings.underlayer_width)
            .SetStrokeLineCap("round")
            .SetStrokeLineJoin("round");

    text.SetFillColor("black");

    doc.Add(std::move(substrates));
    doc.Add(std::move(text));
}

void Data_Structure::StopsTextDrawer::Draw(DbItemIdNameMap &db_item_id_name_map) {
    for (auto &[stop_name, _]: db_svg->stops_coordinates) {
        RenderStopLabel(db_svg->doc, stop_name);
    }
}

void
Data_Structure::StopsTextDrawer::DrawPartial(std::vector<std::string> &names_stops,
                                             std::vector<std::pair<int, size_t>> &used_buses,
                                             Svg::Document &doc,
                                             DbItemIdNameMap &db_item_id_name_map) const {
    for (auto &stop_name: names_stops) {
        RenderStopLabel(doc, stop_name);
    }
}

void Data_Structure::BusTextDrawer::RenderBusLabel(Svg::Document &doc, int bus_id,
                                                   const std::string &stop_name,
                                                   DbItemIdNameMap &db_item_id_name_map) const {
    const auto &color = db_svg->bus_dict.at(db_item_id_name_map.GetNameById(bus_id)).second;
    const auto point = db_svg->stops_coordinates.at(stop_name);
    const auto base_text =
            Svg::Text{}
                    .SetPoint(point)
                    .SetOffset({db_svg->renderSettings.bus_label_offset[0],
                                db_svg->renderSettings.bus_label_offset[1]})
                    .SetFontSize(db_svg->renderSettings.bus_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetFontWeight("bold")
                    .SetData(db_item_id_name_map.GetNameById(bus_id));
    doc.Add(
            Svg::Text(base_text)
                    .SetFillColor(db_svg->renderSettings.underlayer_color)
                    .SetStrokeColor(db_svg->renderSettings.underlayer_color)
                    .SetStrokeWidth(db_svg->renderSettings.underlayer_width)
                    .SetStrokeLineCap("round").SetStrokeLineJoin("round")
    );
    doc.Add(
            Svg::Text(base_text)
                    .SetFillColor(color)
    );
}

void Data_Structure::BusTextDrawer::Draw(DbItemIdNameMap &db_item_id_name_map) {
    for (auto &[bus_id, bus_info]: db_svg->bus_dict) {
        auto &[bus, _] = bus_info;
        auto beg = bus.stops.begin();
        auto last = std::prev(Ranges::ToMiddle(Ranges::AsRange(bus.stops)).end());

        if (!bus.is_roundtrip && *last != *beg) {
            RenderBusLabel(db_svg->doc, db_item_id_name_map.GetIdByName(bus_id), db_item_id_name_map.GetNameById(*beg),
                           db_item_id_name_map);
            RenderBusLabel(db_svg->doc, db_item_id_name_map.GetIdByName(bus_id), db_item_id_name_map.GetNameById(*last),
                           db_item_id_name_map);
        } else {
            RenderBusLabel(db_svg->doc, db_item_id_name_map.GetIdByName(bus_id), db_item_id_name_map.GetNameById(*beg),
                           db_item_id_name_map);
        }
    }
}

void
Data_Structure::BusTextDrawer::DrawPartial(std::vector<std::string> &names_stops,
                                           std::vector<std::pair<int, size_t>> &used_buses,
                                           Svg::Document &doc,
                                           DbItemIdNameMap &db_item_id_name_map) const {
    size_t i = 0;
    for (auto stop_it = names_stops.begin(); stop_it != std::prev(names_stops.end()); stop_it++, i++) {
        auto &bus = db_svg->bus_dict.at(db_item_id_name_map.GetNameById(used_buses[i].first)).first;

        if ((db_item_id_name_map.GetNameById(bus.stops.front()) == *stop_it) ||
            (!bus.is_roundtrip &&
             db_item_id_name_map.GetNameById(*std::prev(Ranges::ToMiddle(Ranges::AsRange(bus.stops)).end())) ==
             *stop_it)) {
            RenderBusLabel(doc, used_buses[i].first, *stop_it, db_item_id_name_map);
        }
        if ((db_item_id_name_map.GetNameById(bus.stops.front()) == *std::next(stop_it)) ||
            (!bus.is_roundtrip && db_item_id_name_map.GetNameById(*std::prev(
                    Ranges::ToMiddle(Ranges::AsRange(bus.stops)).end())) == *std::next(stop_it))) {
            RenderBusLabel(doc, used_buses[i].first, *(std::next(stop_it)), db_item_id_name_map);
        }
    }
}

void Data_Structure::CompanyRoundDrawer::DrawPartial(const std::string &nearby_stop,
                                                     const std::string &company_name,
                                                     Svg::Document &doc,
                                                     DbItemIdNameMap &db_item_id_name_map) const {
    doc.Add(Svg::Circle{}
                    .SetFillColor("black")
                    .SetRadius(db_svg->renderSettings.company_radius)
                    .SetCenter(db_svg->company_coordinates.at(company_name)));

}

void
Data_Structure::CompanyPolylinesDrawer::DrawPartial(const std::string &nearby_stop,
                                                    const std::string &company_name,
                                                    Svg::Document &doc,
                                                    DbItemIdNameMap &db_item_id_name_map) const {
    doc.Add(Svg::Polyline{}
                    .SetStrokeColor("black")
                    .SetStrokeWidth(db_svg->renderSettings.company_line_width)
                    .SetStrokeLineJoin("round")
                    .SetStrokeLineCap("round")
                    .AddPoint(db_svg->stops_coordinates.at(nearby_stop))
                    .AddPoint(db_svg->company_coordinates.at(company_name)));
}

void Data_Structure::CompanyTextDrawer::DrawPartial(const std::string &nearby_stop,
                                                    const std::string &company_name,
                                                    Svg::Document &doc,
                                                    DbItemIdNameMap &db_item_id_name_map) const {
    Svg::Text text = Svg::Text{}
            .SetPoint(db_svg->company_coordinates.at(company_name))
            .SetFontFamily("Verdana")
            .SetData(company_name)
            .SetOffset({db_svg->renderSettings.stop_label_offset[0], db_svg->renderSettings.stop_label_offset[1]})
            .SetFontSize(db_svg->renderSettings.stop_label_font_size);

    Svg::Text substrates = text;
    substrates
            .SetFillColor(db_svg->renderSettings.underlayer_color)
            .SetStrokeColor(db_svg->renderSettings.underlayer_color)
            .SetStrokeWidth(db_svg->renderSettings.underlayer_width)
            .SetStrokeLineCap("round")
            .SetStrokeLineJoin("round");

    text.SetFillColor("black");

    doc.Add(std::move(substrates));
    doc.Add(std::move(text));
}

void Data_Structure::DataBaseSvgBuilder::Serialize(TCProto::TransportCatalog &tc) const {
    RenderProto::RenderSettings ser_render_sets;

    ser_render_sets.set_width(renderSettings.width);
    ser_render_sets.set_height(renderSettings.height);
    ser_render_sets.set_outer_margin(renderSettings.outer_margin);
    ser_render_sets.set_padding(renderSettings.padding);
    ser_render_sets.set_stop_radius(renderSettings.stop_radius);
    ser_render_sets.set_line_width(renderSettings.line_width);
    ser_render_sets.set_company_radius(renderSettings.company_radius);
    ser_render_sets.set_company_line_width(renderSettings.company_line_width);

    ser_render_sets.set_stop_label_font_size(renderSettings.stop_label_font_size);

    ser_render_sets.add_stop_label_offset(renderSettings.stop_label_offset[0]);
    ser_render_sets.add_stop_label_offset(renderSettings.stop_label_offset[1]);

    ser_render_sets.set_bus_label_font_size(renderSettings.bus_label_font_size);

    ser_render_sets.add_bus_label_offset(renderSettings.bus_label_offset[0]);
    ser_render_sets.add_bus_label_offset(renderSettings.bus_label_offset[1]);

    *ser_render_sets.mutable_color() = renderSettings.underlayer_color.Serialize();
    ser_render_sets.set_underlayer_width(renderSettings.underlayer_width);

    for (auto &color: renderSettings.color_palette) {
        *ser_render_sets.add_color_palette() = color.Serialize();
    }
    for (auto &layer: renderSettings.layers) {
        ser_render_sets.add_layers(layer);
    }

    *tc.mutable_render() = std::move(ser_render_sets);
}

void Data_Structure::DataBaseSvgBuilder::Deserialize(const RenderProto::RenderSettings &ren_mes) {
    renderSettings.width = ren_mes.width();
    renderSettings.height = ren_mes.height();
    renderSettings.outer_margin = ren_mes.outer_margin();
    renderSettings.padding = ren_mes.padding();
    renderSettings.stop_radius = ren_mes.stop_radius();
    renderSettings.line_width = ren_mes.line_width();
    renderSettings.company_radius = ren_mes.company_radius();
    renderSettings.company_line_width = ren_mes.company_line_width();

    renderSettings.stop_label_font_size = ren_mes.stop_label_font_size();
    renderSettings.stop_label_offset[0] = ren_mes.stop_label_offset(0);
    renderSettings.stop_label_offset[1] = ren_mes.stop_label_offset(1);

    renderSettings.bus_label_font_size = ren_mes.bus_label_font_size();
    renderSettings.bus_label_offset[0] = ren_mes.bus_label_offset(0);
    renderSettings.bus_label_offset[1] = ren_mes.bus_label_offset(1);

    renderSettings.underlayer_color = Svg::Color(ren_mes.color());
    renderSettings.underlayer_width = ren_mes.underlayer_width();

    for (auto &color: ren_mes.color_palette()) {
        renderSettings.color_palette.emplace_back(color);
    }
    for (auto &layer: ren_mes.layers()) {
        renderSettings.layers.emplace_back(layer);
    }
}
