#pragma once

#include "component_storage.hpp"

namespace vecs {

template <Component... RegisteredComponents>
struct world_t {

private:
    vecs::component_storage_t<RegisteredComponents...> component_storage{};
};

} // namespace vecs