#pragma once

#include "json.h"
#include "map_database.h"
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <vector>

class RequestParser {
public:
    void PerformReadBaseRequests(std::istream &in) {
        Json::Node input_json_node = Json::Load(in).GetRoot();
        try {
            MakeStopNameToIdMapping(input_json_node);
        } catch (const std::exception &e) {
            std::cerr << "MakeStopNameToIdMapping: " << e.what() << "\n";
            throw std::runtime_error("MakeStopNameToIdMapping");
        }

        std::string result;
        try {
            result = MakeJsonWriterFromBaseRequests(input_json_node).ToString();
        } catch (const std::exception &e) {
            std::cerr << "MakeJsonWriterFromBaseRequests: " << e.what() << "\n";
            throw std::runtime_error("MakeJsonWriterFromBaseRequests");
        }

        const std::string file_name = input_json_node.AsMap().at("serialization_settings").AsMap().at(
                "file").AsString();
        std::ofstream file(file_name, std::ios::binary | std::ios::ate);
        Serializer::TransportCatalog transportCatalog;
        *transportCatalog.mutable_input() = result;
        transportCatalog.SerializeToOstream(&file);
        file.close();
    }

private:
    std::unordered_map<std::string, std::string> stop_name_to_id;

    void MakeStopNameToIdMapping(const Json::Node &node) {
        if (!node.AsMap().count("base_requests")) {
            std::cerr << "no base_requests\n";
            return;
        }

        const auto &base_request = node.AsMap().at("base_requests").AsArray();

        for (const auto &request: base_request) {
            const auto &request_map = request.AsMap();
            const auto &request_type = request_map.at("type").AsString();
            if (request_type == "Stop") {
                const auto &stop_name = request_map.at("name").AsString();
                // stop_name_to_id[stop_name] = stop_name_to_id.size();
                stop_name_to_id[stop_name] = stop_name;
            }
        }
    }

    Json::JsonMap MakeJsonWriterFromBaseRequests(const Json::Node &node) {
        std::map<std::string, std::shared_ptr<Json::JsonWriter>> result;
        try {
            WriteStopsIdToNameMapping(result);
        } catch (const std::exception &e) {
            std::cerr << "WriteStopsIdToNameMapping: " << e.what() << "\n";
            throw std::runtime_error("WriteStopsIdToNameMapping");
        }

        try {
            WriteSerializationSettings(node, result);
        } catch (const std::exception &e) {
            std::cerr << "WriteSerializationSettings: " << e.what() << "\n";
            throw std::runtime_error("WriteSerializationSettings");
        }

        try {
            WriteRenderSettings(node, result);
        } catch (const std::exception &e) {
            std::cerr << "WriteRenderSettings: " << e.what() << "\n";
            throw std::runtime_error("WriteRenderSettings");
        }

        try {
            WriteRoutingSettings(node, result);
        } catch (const std::exception &e) {
            std::cerr << "WriteRoutingSettings: " << e.what() << "\n";
            throw std::runtime_error("WriteRoutingSettings");
        }

        try {
            WriteBaseRequests(node, result);
        } catch (const std::exception &e) {
            std::cerr << "WriteBaseRequests: " << e.what() << "\n";
            throw std::runtime_error("WriteBaseRequests");
        }

        return Json::JsonMap(result);
    }

    void WriteStopsIdToNameMapping(std::map<std::string, std::shared_ptr<Json::JsonWriter>> &result) {
        std::map<std::string, std::shared_ptr<Json::JsonWriter>> response;
        for (const auto &[name, id]: stop_name_to_id) {
            response[id] = std::make_shared<Json::JsonString>(name);
        }
        result["stops_id_to_name"] = std::make_shared<Json::JsonMap>(std::move(response));
    }

    static void
    WriteSerializationSettings(const Json::Node &node,
                               std::map<std::string, std::shared_ptr<Json::JsonWriter>> &result) {
        if (!node.AsMap().count("serialization_settings")) {
            std::cerr << "no serialization_settings\n";
            return;
        }

        const auto &serialization_settings = node.AsMap().at("serialization_settings");

        std::map<std::string, std::shared_ptr<Json::JsonWriter>> response;
        response["file"] = std::make_shared<Json::JsonString>(
                serialization_settings.AsMap().at("file").AsString());

        result["serialization_settings"] = std::make_shared<Json::JsonMap>(std::move(response));
    }

    static void
    WriteRoutingSettings(const Json::Node &node, std::map<std::string, std::shared_ptr<Json::JsonWriter>> &result) {
        if (!node.AsMap().count("routing_settings")) {
            std::cerr << "no routing_settings\n";
            return;
        }

        const auto &routing_settings = node.AsMap().at("routing_settings");

        std::map<std::string, std::shared_ptr<Json::JsonWriter>> response;
        response["bus_wait_time"] = std::make_shared<Json::JsonInt>(
                routing_settings.AsMap().at("bus_wait_time").AsInt());
        response["bus_velocity"] = std::make_shared<Json::JsonDouble>(
                routing_settings.AsMap().at("bus_velocity").AsDouble());

        result["routing_settings"] = std::make_shared<Json::JsonMap>(std::move(response));
    }

    static void
    WriteRenderSettings(const Json::Node &node, std::map<std::string, std::shared_ptr<Json::JsonWriter>> &result) {
        if (!node.AsMap().count("render_settings")) {
            std::cerr << "no render_settings\n";
            return;
        }

        const auto &render_settings = node.AsMap().at("render_settings").AsMap();

        std::map<std::string, std::shared_ptr<Json::JsonWriter>> response;
        // width
        response["width"] = std::make_shared<Json::JsonDouble>(render_settings.at("width").AsDouble());
        // height
        response["height"] = std::make_shared<Json::JsonDouble>(render_settings.at("height").AsDouble());
        // padding
        response["padding"] = std::make_shared<Json::JsonDouble>(render_settings.at("padding").AsDouble());
        // stop_radius
        response["stop_radius"] = std::make_shared<Json::JsonDouble>(render_settings.at("stop_radius").AsDouble());
        // line_width
        response["line_width"] = std::make_shared<Json::JsonDouble>(render_settings.at("line_width").AsDouble());
        // stop_label_font_size
        response["stop_label_font_size"] = std::make_shared<Json::JsonInt>(
                render_settings.at("stop_label_font_size").AsInt());
        // stop_label_offset
        std::vector<std::shared_ptr<Json::JsonWriter>> stop_label_offset_vector;
        for (const auto &value: render_settings.at("stop_label_offset").AsArray()) {
            stop_label_offset_vector.push_back(std::make_shared<Json::JsonDouble>(value.AsDouble()));
        }
        response["stop_label_offset"] = std::make_shared<Json::JsonArray>(stop_label_offset_vector);
        // underlayer_width
        response["underlayer_width"] = std::make_shared<Json::JsonDouble>(
                render_settings.at("underlayer_width").AsDouble());
        // underlayer_color
        std::ostringstream underlayer_color_stream;
        Map::MapDataBase::get_color_from_json(
                render_settings.at("underlayer_color")).Render(underlayer_color_stream);
        response["underlayer_color"] = std::make_shared<Json::JsonString>(underlayer_color_stream.str());
        // color_palette
        std::vector<std::shared_ptr<Json::JsonWriter>> color_palette_vector;
        for (const auto &json: render_settings.at("color_palette").AsArray()) {
            std::ostringstream color_palette_stream;
            Map::MapDataBase::get_color_from_json(json).Render(color_palette_stream);
            color_palette_vector.push_back(std::make_shared<Json::JsonString>(color_palette_stream.str()));
        }
        response["color_palette"] = std::make_shared<Json::JsonArray>(color_palette_vector);
        // bus_label_font_size
        response["bus_label_font_size"] = std::make_shared<Json::JsonInt>(
                render_settings.at("bus_label_font_size").AsInt());
        // bus_label_offset
        std::vector<std::shared_ptr<Json::JsonWriter>> bus_label_offset_vector;
        for (const auto &value: render_settings.at("bus_label_offset").AsArray()) {
            bus_label_offset_vector.push_back(std::make_shared<Json::JsonDouble>(value.AsDouble()));
        }
        response["bus_label_offset"] = std::make_shared<Json::JsonArray>(bus_label_offset_vector);

        std::vector<std::shared_ptr<Json::JsonWriter>> layers_vector;
        for (const auto &layer: render_settings.at("layers").AsArray()) {
            layers_vector.push_back(std::make_shared<Json::JsonString>(layer.AsString()));
        }
        // layers
        response["layers"] = std::make_shared<Json::JsonArray>(layers_vector);
        // outer_margin
        response["outer_margin"] = std::make_shared<Json::JsonDouble>(render_settings.at("outer_margin").AsDouble());

        result["render_settings"] = std::make_shared<Json::JsonMap>(std::move(response));
    }

    void WriteBaseRequests(const Json::Node &node, std::map<std::string, std::shared_ptr<Json::JsonWriter>> &result) {
        if (!node.AsMap().count("base_requests")) {
            std::cerr << "no base_requests\n";
            return;
        }

        std::vector<std::shared_ptr<Json::JsonWriter>> response;

        const auto &base_requests = node.AsMap().at("base_requests").AsArray();
        for (const auto &query: base_requests) {
            const auto &query_map = query.AsMap();

            std::map<std::string, std::shared_ptr<Json::JsonWriter>> query_response;

            const auto &query_type = query_map.at("type").AsString();
            query_response["type"] = std::make_shared<Json::JsonString>(query_type);
            query_response["name"] = std::make_shared<Json::JsonString>(query_map.at("name").AsString());

            if (query_type == "Stop") {
                query_response["latitude"] = std::make_shared<Json::JsonDouble>(query_map.at("latitude").AsDouble());
                query_response["longitude"] = std::make_shared<Json::JsonDouble>(query_map.at("longitude").AsDouble());

                std::map<std::string, std::shared_ptr<Json::JsonWriter>> road_distances_map;
                for (const auto &[stop_name, road_distance]: query_map.at("road_distances").AsMap()) {
                    road_distances_map[stop_name_to_id[stop_name]] = std::make_shared<Json::JsonInt>(
                            road_distance.AsInt());
                }
                query_response["road_distances"] = std::make_shared<Json::JsonMap>(road_distances_map);
            } else if (query_type == "Bus") {
                query_response["is_roundtrip"] = std::make_shared<Json::JsonBool>(
                        query_map.at("is_roundtrip").AsBool());

                std::vector<std::shared_ptr<Json::JsonWriter>> stops_vector;
                for (const auto &stop_name: query_map.at("stops").AsArray()) {
                    const auto stop_id = stop_name_to_id[stop_name.AsString()];
                    stops_vector.push_back(std::make_shared<Json::JsonString>(stop_id));
                }
                query_response["stops"] = std::make_shared<Json::JsonArray>(stops_vector);
            } else {
                std::cerr << "unknown query type\n";
            }

            response.push_back(std::make_shared<Json::JsonMap>(query_response));
        }

        result["base_requests"] = std::make_shared<Json::JsonArray>(std::move(response));
    }
};