#pragma once

#include "range.h"
#include <unordered_set>

template<typename It>
size_t ComputeUniqueItemsCount(Range<It> range) {
    return std::unordered_set<typename Range<It>::ValueType>{
            range.begin(), range.end()
    }.size();
}
