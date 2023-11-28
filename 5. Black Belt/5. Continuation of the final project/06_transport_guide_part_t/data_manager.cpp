#include "data_manager.h"

#include "transport_catalog.pb.h"
#include "db_item_name_id_map.h"

#include <algorithm>
#include <cmath>

Data_Structure::DataBase::DataBase(std::istream &is) {
    Deserialize(is);
}

Data_Structure::DataBase::DataBase(const std::vector<DBItem> &elems,
                                   const RoutingSettings routing_settings_,
                                   DbItemIdNameMap &db_item_id_name_map) {
    Init(elems, db_item_id_name_map);

    router = std::make_unique<DataBaseRouter>(
            pure_stops,
            pure_buses,
            routing_settings_,
            db_item_id_name_map
    );
}

Data_Structure::DataBase::DataBase(const std::vector<DBItem> &items,
                                   RoutingSettings routing_settings_,
                                   RenderSettings render_settings,
                                   DbItemIdNameMap &db_item_id_name_map) {
    Init(items, db_item_id_name_map);

    router = std::make_unique<DataBaseRouter>(
            pure_stops,
            pure_buses,
            routing_settings_,
            db_item_id_name_map
    );

    svg_builder = std::make_unique<DataBaseSvgBuilder>(
            std::move(render_settings)
    );
}

Data_Structure::DataBase::DataBase(const std::vector<DBItem> &items,
                                   YellowPages::Database yellow_pages,
                                   RoutingSettings routing_settings_,
                                   Data_Structure::RenderSettings render_settings,
                                   DbItemIdNameMap &&db_item_id_name_map_new) {
    Init(items, db_item_id_name_map_new);

    yellow_pages_db = std::make_unique<DataBaseYellowPages>(std::move(yellow_pages));

    router = std::make_unique<DataBaseRouter>(
            pure_stops,
            pure_buses,
            routing_settings_,
            db_item_id_name_map_new
    );

    svg_builder = std::make_unique<DataBaseSvgBuilder>(
            std::move(render_settings)
    );

    db_item_id_name_map = std::make_unique<DbItemIdNameMap>(std::move(db_item_id_name_map_new));
}

void Data_Structure::DataBase::Init(std::vector<DBItem> const &elems,
                                    DbItemIdNameMap &db_item_id_name_map) {
    for (auto &el: elems) {
        if (std::holds_alternative<Bus>(el)) {
            const auto &bus = std::get<Bus>(el);
            pure_buses.emplace(bus.bus_id, bus);
        } else {
            const auto &stop = std::get<Stop>(el);
            pure_stops.emplace(stop.stop_id, stop);
            stops.emplace(stop.stop_id, std::make_shared<StopResponse>());
        }
    }
    try {
        for (auto [bus_id, bus]: pure_buses) {
            BusRespType bus_resp = std::make_shared<BusResponse>();
            bus_resp->stop_count = bus.stops.size();
            bus_resp->unique_stop_count = Ranges::GetUniqueItems(Ranges::AsRange(bus.stops));
            bus_resp->length = ComputeRouteDistance(bus.stops, pure_stops, db_item_id_name_map);
            bus_resp->curvature = bus_resp->length / ComputeGeoDistance(bus.stops, pure_stops, db_item_id_name_map);
            buses.emplace(bus_id, std::move(bus_resp));

            for (auto &el: !bus.is_roundtrip
                           ? Ranges::ToMiddle(Ranges::AsRange(bus.stops))
                           : Ranges::AsRange(bus.stops)) {
                stops[el]->buses.emplace(
                        db_item_id_name_map.GetNameById(bus.bus_id));
            }
        }
    } catch (...) {
        std::cout << "The DB condition is violated\n";
    }
}

ResponseType Data_Structure::DataBase::FindBus(const std::string &title) const {
    auto ret = buses.find(db_item_id_name_map->GetIdByName(title));
    if (ret != buses.end()) {
        return ret->second;
    } else return GenerateBad();
}

ResponseType Data_Structure::DataBase::FindStop(const std::string &title) const {
    auto ret = stops.find(db_item_id_name_map->GetIdByName(title));
    if (ret != stops.end()) {
        return ret->second;
    } else return GenerateBad();
}

ResponseType Data_Structure::DataBase::FindRoute(const std::string &from,
                                                 const std::string &to) const {
    auto ret = router->CreateRoute(from, to, *db_item_id_name_map);
    if (ret) {
        auto render_items = ret->items;
        if (!render_items.empty()) {
            auto finish = render_items.insert(render_items.end(), std::make_shared<RouteResponse::Wait>());
            (*finish)->name = to;
        }
        ret->route_render = std::move(svg_builder->RenderRoute(render_items, *db_item_id_name_map)->svg_xml_answer);

        return ret;
    } else return GenerateBad();
}

ResponseType Data_Structure::DataBase::BuildMap() const {
    if (!svg_builder)
        return GenerateBad();
    return svg_builder->RenderMap();
}

ResponseType Data_Structure::DataBase::FindCompanies(const std::vector<std::shared_ptr<Query>> &queries) const {
    if (!yellow_pages_db)
        return GenerateBad();
    auto ret = yellow_pages_db->FindCompanies(queries);
    if (!ret)
        return GenerateBad();

    return ret;
}

ResponseType Data_Structure::DataBase::FindRouteToCompanies(const std::string &from, const Datetime &cur_time,
                                                            const std::vector<std::shared_ptr<Query>> &queries) const {
    if (!yellow_pages_db)
        return GenerateBad();
    auto resp = yellow_pages_db->FindCompanies(queries);
    if (reinterpret_cast<CompaniesResponse *>(resp.get())->companies.empty())
        return GenerateBad();

    double route_time = 0;
    double time_from_stop = 0;
    double time_to_wait = 0;
    YellowPages::Company const *company = nullptr;
    std::string nearby_stop;
    for (auto company_ptr: reinterpret_cast<CompaniesResponse const *>(resp.get())->companies) {
        for (auto &stop: company_ptr->nearby_stops()) {
            auto cur_route_time = router->GetRouteWeight(from, db_item_id_name_map->GetNameById(stop.nearby_stop_id()),
                                                         *db_item_id_name_map);
            double time_to_cur = ComputeTimeToWalking(stop.meters(), router->GetSettings().pedestrian_velocity);

            if (company && route_time + time_from_stop + time_to_wait < *cur_route_time + time_to_cur) {
                continue;
            }

            double cur_time_to_wait = 0;
            double cur_min_time = route_time + time_from_stop + time_to_wait;
            ExtraTime(*company_ptr, ToDatetime(ToMinute(cur_time) + *cur_route_time + time_to_cur, cur_time.day),
                      !company ? nullptr : &cur_min_time, *cur_route_time + time_to_cur, cur_time_to_wait);

            if (!company || cur_min_time > *cur_route_time + time_to_cur + cur_time_to_wait) {
                company = company_ptr;
                route_time = *cur_route_time;
                time_from_stop = time_to_cur;
                nearby_stop = db_item_id_name_map->GetNameById(stop.nearby_stop_id());
                time_to_wait = cur_time_to_wait;
            }
        }
    }

    if (!company)
        return GenerateBad();

    auto result_resp = std::make_shared<RouteToCompaniesResponse>();
    auto router_resp = router->CreateRoute(from, nearby_stop, *db_item_id_name_map);

    result_resp->total_time = router_resp->total_time + time_from_stop + time_to_wait;
    result_resp->time_to_walk = time_from_stop;
    result_resp->nearby_stop_name = nearby_stop;
    std::string full_name;
    for (auto &name: company->names()) {
        if (name.type() == YellowPages::Name_Type_MAIN) {
            result_resp->company_name = name.value();
            full_name = result_resp->company_name;
            if (!company->rubrics().empty()) {
                full_name =
                        yellow_pages_db->GetRubric(company->rubrics(0)).keywords(0) + " " + result_resp->company_name;
            }
            break;
        }
    }

    result_resp->items = router_resp->items;
    {
        auto render_items = result_resp->items;

        auto finish = render_items.insert(render_items.end(), std::make_shared<RouteResponse::Wait>());
        (*finish)->name = nearby_stop;

        result_resp->route_render = std::move(
                svg_builder->RenderPathToCompany(render_items, full_name, *db_item_id_name_map)->svg_xml_answer);
    }
    if (time_to_wait > 0)
        result_resp->time_to_wait.emplace(time_to_wait);

    return std::move(result_resp);
}


void
Data_Structure::DataBase::ExtraTime(const YellowPages::Company &company, const Data_Structure::Datetime &datetime,
                                    double *min_time, double cur_time, double &result) const {
    if (!company.has_working_time() || company.working_time().intervals().empty()) {
        result = 0;
        return;
    }

    static const std::map<size_t, YellowPages::WorkingTimeInterval_Day> num_by_day
            {
                    {0, YellowPages::WorkingTimeInterval_Day_MONDAY},
                    {1, YellowPages::WorkingTimeInterval_Day_TUESDAY},
                    {2, YellowPages::WorkingTimeInterval_Day_WEDNESDAY},
                    {3, YellowPages::WorkingTimeInterval_Day_THURSDAY},
                    {4, YellowPages::WorkingTimeInterval_Day_FRIDAY},
                    {5, YellowPages::WorkingTimeInterval_Day_SATURDAY},
                    {6, YellowPages::WorkingTimeInterval_Day_SUNDAY}
            };

    static const auto calculate_time = [](const YellowPages::WorkingTimeInterval *cur_interval,
                                          const Datetime &datetime)
            -> std::pair<double, bool> {
        double eps = 1e-7;
        if ((cur_interval->minutes_from() < ToMinute(datetime) ||
             (std::fabs(cur_interval->minutes_from() - ToMinute(datetime)) < eps))
            && cur_interval->minutes_to() > ToMinute(datetime) &&
            std::fabs(cur_interval->minutes_to() - ToMinute(datetime)) >= eps) {
            return {0, true};
        } else if (cur_interval->minutes_from() > ToMinute(datetime)) {
            return {cur_interval->minutes_from() - ToMinute(datetime), true};
        } else {
            return {(ToMinute(datetime) != 0 ? (static_cast<double>(24 * 60) - ToMinute(datetime)) : 0), false};
        }
    };

    const YellowPages::WorkingTimeInterval *cur_interval = nullptr;
    std::string company_name;
    for (auto &name: company.names()) {
        if (name.type() == YellowPages::Name_Type_MAIN) {
            company_name = name.value();
            break;
        }
    }
    auto working_time = time_database.at(company_name).find(datetime.day);
    if (company.working_time().intervals(0).day() == YellowPages::WorkingTimeInterval_Day_EVERYDAY) {
        working_time = time_database.at(company_name).begin();
    }
    if (working_time != time_database.at(company_name).end()) {
        cur_interval = *working_time->second[std::floor(ToMinute(datetime))];
    }

    if (!cur_interval) {
        Datetime new_datetime{};
        new_datetime.day = (datetime.day + 1) % 7;
        new_datetime.hours = new_datetime.minutes = new_datetime.part_of_minute = 0;
        result += static_cast<double>(24 * 60) - ToMinute(datetime);

        if (min_time && cur_time + result > *min_time)
            return;
        return ExtraTime(company, new_datetime, min_time, cur_time, result);
    } else if (!calculate_time(cur_interval, datetime).second) {
        Datetime new_datetime{};
        new_datetime.day = (datetime.day + 1) % 7;
        new_datetime.hours = new_datetime.minutes = new_datetime.part_of_minute = 0;
        result += calculate_time(cur_interval, datetime).first;

        if (min_time && cur_time + result > *min_time)
            return;
        return ExtraTime(company, new_datetime, min_time, cur_time, result);
    } else {
        result += calculate_time(cur_interval, datetime).first;
        return;
    }
}

ResponseType Data_Structure::DataBase::GenerateBad() {
    auto ret = std::make_shared<BadResponse>();
    ret->error_message = "not found";
    return ret;
}

int Data_Structure::ComputeStopsDistance(const Data_Structure::Stop &lhs,
                                         const Data_Structure::Stop &rhs,
                                         DbItemIdNameMap &db_item_id_name_map) {
    if (auto it = lhs.adjacent_stops.find(rhs.stop_id); it !=
                                                        lhs.adjacent_stops.end()) {
        return it->second;
    } else {
        return rhs.adjacent_stops.at(lhs.stop_id);
    }
}

int Data_Structure::ComputeRouteDistance(const std::vector<int> &stops,
                                         const std::unordered_map<int, Stop> &db_stops,
                                         DbItemIdNameMap &db_item_id_name_map) {
    int result = 0;
    for (size_t i = 1; i < stops.size(); i++) {
        result += ComputeStopsDistance(db_stops.at(stops[i - 1]),
                                       db_stops.at(stops[i]),
                                       db_item_id_name_map);
    }
    return result;
}

double Data_Structure::ComputeGeoDistance(const std::vector<int> &stops,
                                          const std::unordered_map<int, Stop> &db_stops,
                                          DbItemIdNameMap &db_item_id_name_map) {
    double result = 0.;
    for (size_t i = 1; i < stops.size(); i++) {
        result += ComputeDistance(db_stops.at(stops.at(i - 1)).dist,
                                  db_stops.at(stops.at(i)).dist);
    }
    return result;
}

std::set<std::string>
Data_Structure::GetBearingPoints(const std::unordered_map<std::string, stop_n_companies> &stops,
                                 const std::unordered_map<int, Bus> &buses,
                                 DbItemIdNameMap &db_item_id_name_map) {
    std::set<std::string> res;
    std::map<std::string, std::string> stop_and_buses;
    std::map<std::string, int> stops_count;

    for (auto &[stop_name, _]: stops) {
        stops_count[stop_name] = 0;
    }

    for (auto const &[_, bus]: buses) {
        if (!bus.stops.empty()) {
            res.insert(db_item_id_name_map.GetNameById(bus.stops.front()));
            if (!bus.is_roundtrip) {
                res.insert(db_item_id_name_map.GetNameById(
                        *(std::prev(Ranges::ToMiddle(Ranges::AsRange(bus.stops)).end()))));
            }
        }
        for (auto const &stop_id: bus.stops) {
            ++stops_count[db_item_id_name_map.GetNameById(stop_id)];
            const auto [it, inserted] = stop_and_buses.emplace(db_item_id_name_map.GetNameById(stop_id),
                                                               db_item_id_name_map.GetNameById(bus.bus_id));
            if (!inserted && it->second != db_item_id_name_map.GetNameById(bus.bus_id))
                res.insert(db_item_id_name_map.GetNameById(stop_id));
        }
    }

    for (const auto &[stop, count]: stops_count) {
        if (count > 2 || count == 0) {
            res.insert(stop);
        }
    }

    return res;
}

double Data_Structure::ComputeTimeToWalking(double meters, double walk_speed) {
    return meters / (walk_speed / 3.6f) / 60;
}

double Data_Structure::ToMinute(const Data_Structure::Datetime &datetime) {
    return static_cast<double>(datetime.hours * 60 + datetime.minutes) + datetime.part_of_minute;
}

Data_Structure::Datetime Data_Structure::ToDatetime(double minutes, size_t day) {
    Data_Structure::Datetime datetime{};
    minutes += static_cast<double>(day * 24 * 60);
    datetime.day = minutes / (24 * 60);
    size_t whole_minutes = static_cast<size_t>(std::floor(minutes)) % (24 * 60);
    datetime.hours = whole_minutes / 60;
    datetime.minutes = whole_minutes % 60;
    datetime.part_of_minute = minutes - std::floor(minutes);

    datetime.day %= 7;
    return datetime;
}

void Data_Structure::DataBase::Serialize(std::ostream &os) const {
    TCProto::TransportCatalog tc;

    for (auto &stop: stops) {
        TCProto::Stop dummy_stop;
        dummy_stop.set_stop_id(stop.first);
        for (auto &bus_name: stop.second->buses) {
            dummy_stop.add_buses_id(db_item_id_name_map->GetIdByName(bus_name));
        }

        auto &pure_stop = pure_stops.at(stop.first);
        dummy_stop.set_latitude(pure_stop.dist.GetLatitude());
        dummy_stop.set_longitude(pure_stop.dist.GetLongitude());
        for (auto &el: pure_stop.adjacent_stops) {
            TCProto::AdjacentStops as;
            as.set_stop_id(el.first);
            as.set_dist(el.second);
            *dummy_stop.add_adjacent_stops() = std::move(as);
        }

        *tc.add_stops() = std::move(dummy_stop);
    }

    for (auto &[bus_id, bus]: buses) {
        TCProto::Bus dummy_bus;
        dummy_bus.set_bus_id(bus_id);
        dummy_bus.set_length(bus->length);
        dummy_bus.set_curvature(bus->curvature);
        dummy_bus.set_stop_count(bus->stop_count);
        dummy_bus.set_unique_stop_count(bus->unique_stop_count);

        auto &pure_bus = pure_buses.at(bus_id);
        dummy_bus.set_is_roundtrip(pure_bus.is_roundtrip);
        for (auto &el: pure_bus.stops) {
            dummy_bus.add_stops_id(el);
        }

        *tc.add_buses() = std::move(dummy_bus);
    }

    *tc.mutable_yellow_pages() = yellow_pages_db->Serialize();
    router->Serialize(tc, *db_item_id_name_map);
    svg_builder->Serialize(tc);
    db_item_id_name_map->Serialize(tc);

    tc.SerializePartialToOstream(&os);
}

void Data_Structure::DataBase::Deserialize(std::istream &is) {
    TCProto::TransportCatalog tc;
    tc.ParseFromIstream(&is);

    db_item_id_name_map = std::make_unique<DbItemIdNameMap>(tc.id_to_name_map());

    for (auto &stop: tc.stops()) {
        std::set<std::string> s_buses;
        for (auto &bus_id: stop.buses_id()) {
            s_buses.insert(db_item_id_name_map->GetNameById(bus_id));
        }
        StopResponse sr;
        sr.buses = std::move(s_buses);
        this->stops.emplace(stop.stop_id(),
                            std::make_shared<StopResponse>(std::move(sr)));

        Stop pure_stop(stop.stop_id(),
                       Distance{stop.longitude(), stop.latitude()},
                       {});
        for (auto &as: stop.adjacent_stops()) {
            pure_stop.adjacent_stops.emplace(as.stop_id(), as.dist());
        }
        pure_stops.emplace(stop.stop_id(), pure_stop);
    }

    for (auto &bus: tc.buses()) {
        BusResponse br;
        br.length = bus.length();
        br.curvature = bus.curvature();
        br.stop_count = bus.stop_count();
        br.unique_stop_count = bus.unique_stop_count();
        this->buses.emplace(bus.bus_id(),
                            std::make_shared<BusResponse>(std::move(br)));

        Bus pure_bus(bus.bus_id(),
                     {},
                     bus.is_roundtrip());
        for (auto &stop_id: bus.stops_id()) {
            pure_bus.stops.emplace_back(stop_id);
        }
        pure_buses.emplace(bus.bus_id(), pure_bus);
    }

    yellow_pages_db = std::make_unique<DataBaseYellowPages>(tc.yellow_pages());

    router = std::make_unique<DataBaseRouter>(tc.router(), *db_item_id_name_map);

    std::unordered_map<std::string, const YellowPages::Company *> companies_map;
    for (auto &company: yellow_pages_db->GetOrigin().companies()) {
        std::string company_name;
        for (auto &name: company.names()) {
            if (name.type() == YellowPages::Name_Type_MAIN) {
                company_name = name.value();
                break;
            }
        }
        if (company.has_working_time()) {
            for (auto &interval: company.working_time().intervals()) {
                time_database[company_name][static_cast<int>(interval.day()) - 1].add(interval.minutes_from(),
                                                                                      interval.minutes_to(), &interval);
            }
        }
        if (!company.rubrics().empty()) {
            std::string new_name = yellow_pages_db->GetRubric(company.rubrics(0)).keywords(0) + " " + company_name;
            company_name = new_name;
        }
        companies_map.emplace(company_name, &company);
    }
    svg_builder = std::make_unique<DataBaseSvgBuilder>(tc.render(), pure_stops, pure_buses, companies_map,
                                                       *db_item_id_name_map);
}