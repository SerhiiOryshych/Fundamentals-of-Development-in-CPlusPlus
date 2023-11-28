#pragma once

#include "rubric.pb.h"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace YellowPages {

    using RubricsNameToId = std::unordered_map<std::string, uint64_t>;
    using RubricsById = google::protobuf::Map<uint64_t, YellowPages::Rubric>;

    RubricsNameToId MakeRubricsNameToId(const RubricsById &id_to_rubric);

}
