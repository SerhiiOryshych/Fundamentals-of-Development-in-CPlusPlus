#pragma once

#include "transport_catalog.pb.h"
#include "db_item_id_name_map.pb.h"

#include <map>
#include <string>

class DbItemIdNameMap {
public:
    DbItemIdNameMap() = default;

    explicit DbItemIdNameMap(const DbItemIdNameMapProto::IdToNameMap &data) {
        for (const auto &key_value: data.value()) {
            id_to_name[key_value.first] = key_value.second;
            name_to_id[key_value.second] = key_value.first;
            object_cnt = id_to_name.size();
        }
    }

    int GetIdByName(const std::string &name) {
        if (name_to_id.count(name)) {
            return name_to_id.at(name);
        }
        return AddObject(name);
    }

    [[nodiscard]] std::string GetNameById(int id) const {
        if (id_to_name.count(id)) {
            return id_to_name.at(id);
        }
        throw std::runtime_error("GetNameById: no keys with id=" + std::to_string(id));
    }

    void Serialize(TCProto::TransportCatalog &tc) const {
        DbItemIdNameMapProto::IdToNameMap id_to_name_map_proto;
        for (const auto &[name, id]: name_to_id) {
            (*id_to_name_map_proto.mutable_value())[id] = name;
        }
        *tc.mutable_id_to_name_map() = std::move(id_to_name_map_proto);
    }

private:
    int object_cnt = 0;
    std::unordered_map<int, std::string> id_to_name;
    std::unordered_map<std::string, int> name_to_id;

    int AddObject(const std::string &name) {
        id_to_name[object_cnt] = name;
        name_to_id[name] = object_cnt;
        return object_cnt++;
    }
};