#include "rubric.h"

using namespace std;

namespace YellowPages {

    RubricsNameToId MakeRubricsNameToId(const google::protobuf::Map<uint64_t, YellowPages::Rubric> &id_to_rubric) {
        RubricsNameToId name_to_id;
        for (const auto &key_value: id_to_rubric) {  // TODO: compiler fails with structured bindings
            name_to_id[key_value.second.name()] = key_value.first;
        }
        return name_to_id;
    }

}
