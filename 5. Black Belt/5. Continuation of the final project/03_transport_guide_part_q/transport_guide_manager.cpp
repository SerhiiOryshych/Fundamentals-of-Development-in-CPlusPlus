#include <sstream>
#include <string>
#include <string_view>
#include <fstream>
#include "transport_guide_manager.h"
#include "json.h"
#include "transport_catalog.pb.h"
#include "yellow_pages_database.h"
#include "request_parser.h"

std::string ReadFileData(const std::string &file_name) {
    std::ifstream file(file_name, std::ios::binary | std::ios::ate);
    const std::ifstream::pos_type end_pos = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string data(end_pos, '\0');
    file.read(&data[0], end_pos);
    return data;
}

void TransportGuideManager::PerformReadQueries(std::istream &input) {
    std::string result;
    std::string line;
    while (std::getline(input, line)) {
        result += line + '\n';
    }
    std::istringstream in(result);
    const auto json_input = Json::Load(in).GetRoot();
    const std::string file_name = json_input.AsMap().at("serialization_settings").AsMap().at("file").AsString();
    std::ofstream file(file_name, std::ios::binary | std::ios::ate);
    Serializer::TransportCatalog transportCatalog;
    *transportCatalog.mutable_input() = result;
    transportCatalog.SerializeToOstream(&file);
    file.close();
}

void TransportGuideManager::PerformMakeBaseRequests(std::istream &input) {
    RequestParser requestParser;
    requestParser.PerformReadBaseRequests(input);
}

void TransportGuideManager::perform_read_base_requests(const std::vector<Json::Node> &post_requests) const {
    for (const auto &request: post_requests) {
        if (request.AsMap().at("type").AsString() == "Stop") {
            const auto stop = read_stop(request);
        }

        if (request.AsMap().at("type").AsString() == "Bus") {
            const auto bus_response = read_bus(request);
        }
    }
}

void TransportGuideManager::PerformWriteQueries(std::istream &input, std::ostream &output) {
    const auto write_input = Json::Load(input).GetRoot();
    const std::string file_name = write_input.AsMap().at("serialization_settings").AsMap().at("file").AsString();
    Serializer::TransportCatalog deserialized_catalog;
    deserialized_catalog.ParseFromString(ReadFileData(file_name));
    std::istringstream deserialized_input(deserialized_catalog.input());
    const auto read_input = Json::Load(deserialized_input).GetRoot();

    try {
        perform_read_queries(read_input.AsMap().at("base_requests").AsArray());
    } catch (const std::exception &e) {
        std::cerr << "perform_read_queries: " << e.what() << "\n";
        throw std::runtime_error("perform_read_queries");
    }

    try {
        if (read_input.AsMap().count("yellow_pages")) {
            build_yellow_pages_database(read_input.AsMap().at("yellow_pages"));
        }
    } catch (const std::exception &e) {
        std::cerr << "build_yellow_pages_database: " << e.what() << "\n";
        throw std::runtime_error("build_yellow_pages_database");
    }

    try {
        build_transport_databases(read_input);
    } catch (const std::exception &e) {
        std::cerr << "build_transport_databases: " << e.what() << "\n";
        throw std::runtime_error("build_transport_databases");
    }

    try {
        build_map_database(read_input);
    } catch (const std::exception &e) {
        std::cerr << "build_map_database: " << e.what() << "\n";
        throw std::runtime_error("build_map_database");
    }

    try {
        perform_write_queries(write_input.AsMap().at("stat_requests").AsArray(), output);
    } catch (const std::exception &e) {
        std::cerr << "perform_write_queries: " << e.what() << "\n";
        throw std::runtime_error("perform_write_queries");
    }
}

void TransportGuideManager::perform_read_queries(
        const std::vector<Json::Node> &post_requests) const {
    for (const auto &request: post_requests) {
        if (request.AsMap().at("type").AsString() == "Stop") {
            const auto stop = read_stop(request);
            AddStop(stop);
        }


        if (request.AsMap().at("type").AsString() == "Bus") {
            const auto bus_response = read_bus(request);
            AddRoute(bus_response.first, bus_response.second);
        }
    }
}

BusStop TransportGuideManager::read_stop(const Json::Node &request) {
    BusStop stop;

    stop.name = request.AsMap().at("name").AsString();
    stop.coordinate = GroundPoint(request.AsMap().at("latitude").AsDouble(),
                                  request.AsMap().at("longitude").AsDouble());

    for (const auto &pair: request.AsMap().at("road_distances").AsMap()) {
        stop.distance_to_other_stops[pair.first] = pair.second.AsInt();
    }

    return std::move(stop);
}

std::pair<std::string, std::shared_ptr<IRouteInfo>> TransportGuideManager::read_bus(const Json::Node &request) const {
    std::shared_ptr<IRouteInfo> route_info;
    std::string bus_name = request.AsMap().at("name").AsString();

    if (request.AsMap().at("is_roundtrip").AsBool()) {
        route_info = std::make_shared<RoundRoute>();
    } else {
        route_info = std::make_shared<DirectRoute>();
    }
    for (const auto &stop: request.AsMap().at("stops").AsArray()) {
        route_info->AddStop(stop.AsString());
    }

    return {std::move(bus_name), std::move(route_info)};
}

void TransportGuideManager::build_yellow_pages_database(const Json::Node &db) {
    YellowPagesDB yellow_pages_db;
    if (db.AsMap().count("rubrics")) {
        try {
            yellow_pages_db.FetchRubrics(db.AsMap().at("rubrics"));
        } catch (const std::exception &e) {
            std::cerr << "FetchRubrics: " << e.what() << "\n";
            throw std::runtime_error("FetchRubrics");
        }
    }
    if (db.AsMap().count("companies")) {
        try {
            yellow_pages_db.FetchCompanies(db.AsMap().at("companies"));
        } catch (const std::exception &e) {
            std::cerr << "FetchCompanies: " << e.what() << "\n";
            throw std::runtime_error("FetchCompanies");
        }
    }
    // throw std::runtime_error("build_yellow_pages_database");
    yellow_pages_database_ = std::make_shared<YellowPagesDB>(std::move(yellow_pages_db));
}

void TransportGuideManager::build_transport_databases(const Json::Node &configs) {
    NavigationSettings settings(
            configs.AsMap().at("routing_settings").AsMap().at("bus_wait_time").AsInt(),
            configs.AsMap().at("routing_settings").AsMap().at("bus_velocity").AsDouble());

    navigation_database_ =
            std::make_shared<NavigationDataBase>(settings, stops_database_->GetBusStopsCount());

    routes_database_->BuildAllRoutes(stops_database_, navigation_database_);
    navigation_database_->Build();
}

void TransportGuideManager::build_map_database(const Json::Node &configs) {
    try {
        if (configs.AsMap().count("render_settings")) {
            map_database_->AddRenderSettings(configs.AsMap().at("render_settings").AsMap());
        } else {
            std::cerr << "build_map_database: no render_settings\n";
            throw std::runtime_error("build_map_database: no render_settings");
        }
    } catch (const std::exception &e) {
        std::cerr << "AddRenderSettings: " << e.what() << "\n";
        throw std::runtime_error("AddRenderSettings");
    }
    map_database_->BuildMap();
}

void TransportGuideManager::perform_write_queries(
        const std::vector<Json::Node> &get_requests, std::ostream &output) const {
    using namespace Json;

    std::vector<std::shared_ptr<JsonWriter>> result;

    for (const auto &request: get_requests) {
        std::map<std::string, std::shared_ptr<JsonWriter>> response;
        response["request_id"] =
                std::make_shared<JsonInt>(request.AsMap().at("id").AsInt());

        if (request.AsMap().at("type").AsString() == "Bus")
            perform_bus_query(request, response);

        if (request.AsMap().at("type").AsString() == "Stop")
            perform_stop_query(request, response);

        if (request.AsMap().at("type").AsString() == "Route")
            perform_route_query(request, response);

        if (request.AsMap().at("type").AsString() == "Map")
            perform_map_query(response);

        if (request.AsMap().at("type").AsString() == "FindCompanies")
            perform_find_companies_query(request, response);

        result.push_back(std::make_shared<JsonMap>(std::move(response)));
    }

    output << JsonArray(result).ToString();
}

void TransportGuideManager::perform_bus_query(
        const Json::Node &request,
        std::map<std::string, std::shared_ptr<Json::JsonWriter>> &response
) const {

    const auto route_response = routes_database_->GetRouteStats(
            request.AsMap().at("name").AsString());

    if (route_response.second) {
        response["route_length"] =
                std::make_shared<Json::JsonInt>(route_response.second->route_length);
        response["curvature"] =
                std::make_shared<Json::JsonDouble>(route_response.second->curvature);
        response["stop_count"] =
                std::make_shared<Json::JsonInt>(route_response.second->stops_on_route);
        response["unique_stop_count"] =
                std::make_shared<Json::JsonInt>(route_response.second->unique_stops);
    } else {
        response["error_message"] =
                std::make_shared<Json::JsonString>(std::string("not found"));
    }
}

void TransportGuideManager::perform_stop_query(
        const Json::Node &request,
        std::map<std::string, std::shared_ptr<Json::JsonWriter>> &response
) const {

    const auto stop_response = stops_database_->GetBusStopStat(
            request.AsMap().at("name").AsString());

    if (stop_response.second) {
        std::vector<std::shared_ptr<Json::JsonWriter>> buses;
        buses.reserve(stop_response.second->size());

        for (const auto &bus: *stop_response.second) {
            buses.push_back(
                    std::make_shared<Json::JsonString>(bus));
        }

        response["buses"] = std::make_shared<Json::JsonArray>(std::move(buses));
    } else {
        response["error_message"] =
                std::make_shared<Json::JsonString>(std::string("not found"));
    }
}

void TransportGuideManager::perform_route_query(
        const Json::Node &request,
        std::map<std::string, std::shared_ptr<Json::JsonWriter>> &response
) const {

    const auto navigation_response =
            navigation_database_->GetDirections(request.AsMap().at("from").AsString(),
                                                request.AsMap().at("to").AsString());

    if (navigation_response) {
        response["map"] = std::make_shared<Json::JsonString>(std::move(map_database_->GetRouteMapResponse(
                *navigation_response,
                navigation_database_)));
        response["total_time"] =
                std::make_shared<Json::JsonDouble>(navigation_response->total_time);

        std::vector<std::shared_ptr<Json::JsonWriter>> items;

        for (const auto &item: navigation_response->items) {
            std::map<std::string, std::shared_ptr<Json::JsonWriter>> item_map;

            item_map["type"] = std::make_shared<Json::JsonString>(item->GetType());
            item_map[item->GetName().first] = std::make_shared<Json::JsonString>(item->GetName().second);
            item_map["time"] = std::make_shared<Json::JsonDouble>(item->GetTime());

            if (item->GetType() == "Bus") {
                item_map["span_count"] = std::make_shared<Json::JsonInt>(*item->GetSpanCount());
            }

            items.push_back(std::make_shared<Json::JsonMap>(std::move(item_map)));
        }

        response["items"] = std::make_shared<Json::JsonArray>(std::move(items));
    } else {
        response["error_message"] =
                std::make_shared<Json::JsonString>(std::string("not found"));
    }
}

void TransportGuideManager::perform_map_query(
        std::map<std::string, std::shared_ptr<Json::JsonWriter>> &response
) const {
    response["map"] =
            std::make_shared<Json::JsonString>(std::move(map_database_->GetMapResponse()));
}

void TransportGuideManager::perform_find_companies_query(const Json::Node &request,
                                                         std::map<std::string, std::shared_ptr<Json::JsonWriter>> &response) const {
    auto filtered_companies = yellow_pages_database_->GetSetWithAllCompanies();
    if (request.AsMap().count("names")) {
        try {
            filtered_companies = yellow_pages_database_->FilterCompaniesByNames(filtered_companies,
                                                                                request.AsMap().at("names"));
        } catch (const std::exception &e) {
            std::cerr << "FilterCompaniesByNames: " << e.what() << "\n";
            throw std::runtime_error("FilterCompaniesByNames");
        }
    }
    if (request.AsMap().count("urls")) {
        try {
            filtered_companies = yellow_pages_database_->FilterCompaniesByUrls(filtered_companies,
                                                                               request.AsMap().at("urls"));
        } catch (const std::exception &e) {
            std::cerr << "FilterCompaniesByUrls: " << e.what() << "\n";
            throw std::runtime_error("FilterCompaniesByUrls");
        }

    }
    if (request.AsMap().count("rubrics")) {
        try {
            filtered_companies = yellow_pages_database_->FilterCompaniesByRubrics(filtered_companies,
                                                                                  request.AsMap().at("rubrics"));
        } catch (const std::exception &e) {
            std::cerr << "FilterCompaniesByRubrics: " << e.what() << "\n";
            throw std::runtime_error("FilterCompaniesByRubrics");
        }
    }
    if (request.AsMap().count("phones")) {
        try {
            filtered_companies = yellow_pages_database_->FilterCompaniesByPhones(filtered_companies,
                                                                                 request.AsMap().at("phones"));
        } catch (const std::exception &e) {
            std::cerr << "FilterCompaniesByPhones: " << e.what() << "\n";
            throw std::runtime_error("FilterCompaniesByPhones");
        }
    }

    std::vector<std::shared_ptr<Json::JsonWriter>> companies;
    auto companies_names = yellow_pages_database_->GetCompaniesNamesByIds(filtered_companies);
    for (const auto &company_name: companies_names) {
        companies.push_back(
                std::make_shared<Json::JsonString>(company_name));
    }
    response["companies"] = std::make_shared<Json::JsonArray>(std::move(companies));
}

void TransportGuideManager::AddStop(const BusStop &stop) const {
    stops_database_->AddStop(stop);
}

void TransportGuideManager::AddRoute(
        const std::string &bus_name, const std::shared_ptr<IRouteInfo> &route_info) const {
    routes_database_->AddRoute(bus_name, route_info);
}