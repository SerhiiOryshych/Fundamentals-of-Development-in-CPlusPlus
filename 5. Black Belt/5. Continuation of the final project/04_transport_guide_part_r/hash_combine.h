#pragma once

#include <functional>

template<typename T>
size_t ComputeHash(const T &obj);

template<typename... Types>
size_t ComputeCombinedHash(const Types &...types);

// --- realization ---

template<typename T>
size_t ComputeHash(const T &obj) {
    return std::hash<T>{}(obj);
}

template<typename FirstHash>
size_t CombineHashes(FirstHash first_hash) {
    return first_hash;
}

template<typename FirstHash, typename... Hashes>
size_t CombineHashes(FirstHash first_hash, Hashes ...hashes) {
    static constexpr size_t POWER = 4'294'967'291;
    return CombineHashes(hashes...) * POWER + first_hash;
}

template<typename... Types>
size_t ComputeCombinedHash(const Types &...types) {
    return CombineHashes(ComputeHash(types)...);
}
