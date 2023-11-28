#pragma once

template<typename Map, typename Key>
const typename Map::mapped_type *GetValuePointer(const Map &map, const Key &key) {
    if (auto it = map.find(key); it != end(map)) {
        return &it->second;
    } else {
        return nullptr;
    }
}
