#include "requests.h"
#include "transport_router.h"
#include "company.h"

#include <vector>
#include <optional>

using namespace std;

namespace Requests {

    Json::Dict Stop::Process(const TransportCatalog &db) const {
        const auto *stop = db.GetStop(name);
        Json::Dict dict;
        if (!stop) {
            dict["error_message"] = Json::Node("not found"s);
        } else {
            Json::Array bus_nodes;
            bus_nodes.reserve(stop->bus_names.size());
            for (const auto &bus_name: stop->bus_names) {
                bus_nodes.emplace_back(bus_name);
            }
            dict["buses"] = Json::Node(std::move(bus_nodes));
        }
        return dict;
    }

    Json::Dict Bus::Process(const TransportCatalog &db) const {
        const auto *bus = db.GetBus(name);
        Json::Dict dict;
        if (!bus) {
            dict["error_message"] = Json::Node("not found"s);
        } else {
            dict = {
                    {"stop_count",        Json::Node(static_cast<int>(bus->stop_count))},
                    {"unique_stop_count", Json::Node(static_cast<int>(bus->unique_stop_count))},
                    {"route_length",      Json::Node(static_cast<int>(bus->road_route_length))},
                    {"curvature",         Json::Node(bus->road_route_length / bus->geo_route_length)},
            };
        }
        return dict;
    }

    struct RouteItemResponseBuilder {
        const TransportCatalog &db;

        Json::Dict operator()(const TransportRouter::RouteInfo::RideBusItem &bus_item) const {
            return Json::Dict{
                    {"type",       Json::Node("RideBus"s)},
                    {"bus",        Json::Node(bus_item.bus_name)},
                    {"time",       Json::Node(bus_item.time)},
                    {"span_count", Json::Node(static_cast<int>(bus_item.span_count))}
            };
        }

        Json::Dict operator()(const TransportRouter::RouteInfo::WaitBusItem &wait_item) const {
            return Json::Dict{
                    {"type",      Json::Node("WaitBus"s)},
                    {"stop_name", Json::Node(wait_item.stop_name)},
                    {"time",      Json::Node(wait_item.time)},
            };
        }

        Json::Dict operator()(const TransportRouter::RouteInfo::WalkToCompanyItem &walk_to_company) const {
            return Json::Dict{
                    {"type",      Json::Node("WalkToCompany"s)},
                    {"stop_name", Json::Node(walk_to_company.stop_name)},
                    {"company",   Json::Node(walk_to_company.company_name)},
                    {"time",      Json::Node(walk_to_company.time)},
            };
        }
    };

    Json::Array PrintRouteItems(const TransportCatalog &db, const TransportRouter::RouteInfo &route) {
        RouteItemResponseBuilder builder{db};
        Json::Array items;
        items.reserve(route.items.size());
        for (const auto &item: route.items) {
            items.emplace_back(visit(builder, item));
        }
        return std::move(items);
    }

    Json::Dict Route::Process(const TransportCatalog &db) const {
        const auto route = db.FindRoute(stop_from, stop_to);
        Json::Dict dict;
        if (!route) {
            dict["error_message"] = Json::Node("not found"s);
        } else {
            dict["total_time"] = Json::Node(route->total_time);
            dict["items"] = PrintRouteItems(db, *route);
            dict["map"] = Json::Node(db.RenderRoute(*route));
        }
        return dict;
    }

    Json::Dict Map::Process(const TransportCatalog &db) {
        return Json::Dict{
                {"map", Json::Node(db.RenderMap())},
        };
    }

    Json::Dict FindCompanies::Process(const TransportCatalog &db) const {
        Json::Dict result;
        Json::Array companies_json;
        const auto companies = db.FindCompanies(query);
        companies_json.reserve(companies.size());
        for (const YellowPages::Company *company: companies) {
            companies_json.push_back(YellowPages::MakeCompanyJson(*company));
        }
        result["companies"] = std::move(companies_json);
        return result;
    }

    Json::Dict RouteToCompany::Process(const TransportCatalog &db) const {
        Json::Dict result;

        const auto companies = db.FindCompanies(companies_query);
        if (companies.empty()) {
            result["error_message"] = Json::Node("not found"s);
            return result;
        }

        optional<pair<double, TransportRouter::RouteInfo>> best_route = nullopt;
        for (const auto company: companies) {
            const auto &nearby_stops = company->nearby_stops();
            for (const auto &nearby_stop: nearby_stops) {
                const auto &nearby_stop_name = nearby_stop.name();

                auto route = db.FindRoute(stop_from, nearby_stop_name);
                if (!route) {
                    if (stop_from == nearby_stop_name) {
                        throw runtime_error("stop_from == nearby_stop_name");
                    }
                    continue;
                }

                auto walk_time = db.ComputeWalkTime(nearby_stop.meters());
                auto route_total_time = route->total_time + walk_time;
                if (!best_route.has_value() || best_route.value().first > route_total_time) {
                    route->total_time = route_total_time;

                    TransportRouter::RouteInfo::WalkToCompanyItem i;
                    i.time = walk_time;
                    i.stop_name = nearby_stop_name;
                    i.company_name = YellowPages::FindMainCompanyName(*company);
                    i.company_full_name = YellowPages::PrintCompany(
                            *company,
                            db.GetRubricsById());
                    i.company_id = company->id();
                    route->items.emplace_back(i);

                    best_route.emplace(route_total_time, route.value());
                }
            }
        }

        if (!best_route.has_value()) {
            result["error_message"] = Json::Node("not found"s);
            return result;
        }

        const auto &route_result = best_route.value().second;
        result["total_time"] = Json::Node(route_result.total_time);
        result["items"] = PrintRouteItems(db, route_result);
        result["map"] = Json::Node(db.RenderRoute(route_result));
        return result;
    }

    variant <Stop, Bus, Route, Map, FindCompanies, RouteToCompany> Read(const Json::Dict &attrs) {
        const string &type = attrs.at("type").AsString();
        if (type == "Bus") {
            return Bus{attrs.at("name").AsString()};
        } else if (type == "Stop") {
            return Stop{attrs.at("name").AsString()};
        } else if (type == "Route") {
            return Route{attrs.at("from").AsString(), attrs.at("to").AsString()};
        } else if (type == "Map") {
            return Map{};
        } else if (type == "RouteToCompany") {
            return RouteToCompany{attrs.at("from").AsString(),
                                  YellowPages::CompanyQuery::FromJson(attrs.at("companies").AsMap())};
        } else {
            return FindCompanies{YellowPages::CompanyQuery::FromJson(attrs)};
        }
    }

    Json::Array ProcessAll(const TransportCatalog &db, const Json::Array &requests) {
        Json::Array responses;
        responses.reserve(requests.size());
        for (const Json::Node &request_node: requests) {
            Json::Dict dict = visit([&db](const auto &request) {
                                        return request.Process(db);
                                    },
                                    Requests::Read(request_node.AsMap()));
            dict["request_id"] = Json::Node(request_node.AsMap().at("id").AsInt());
            responses.emplace_back(dict);
        }
        return responses;
    }

}
