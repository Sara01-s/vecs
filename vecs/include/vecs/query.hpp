#pragma once

// std
#include <tuple>
#include <vector>

// lib
#include "types.hpp"

namespace vecs {

template <Component... Cs>
struct query_t final {



std::tuple<std::vector<vecs::component_t<Cs...>>> data{};

};
    
} // namespace vecs
