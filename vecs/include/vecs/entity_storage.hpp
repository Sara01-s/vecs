#pragma once

// std
#include <array>
#include <bitset>
#include <cassert>

// lib
#include "types.hpp"
#include "debug.hpp"
#include "data_structures/resusable_id.hpp"

namespace vecs {

template <vecs::usize Capacity = 64, vecs::usize MaxComponents = 64>
struct entity_storage_t {
    using log_t = vecs::debug_t;

    [[nodiscard]] 
    constexpr vecs::entity_id_t const
    add_entity() noexcept {
        auto const created_entity_id = _entity_ids.next_free_id();

        assert(created_entity_id < Capacity 
            && "Maximum number of entities reached.");

        return created_entity_id;
    }

    void
    remove_entity(vecs::entity_id_t entity_id) noexcept {
        assert(_entity_ids.is_used(entity_id) 
            && "Cannot despawn Entity, it is not alive.");

        _entity_ids.free_id(entity_id);
    }

    void
    add_component_keys(
        vecs::entity_id_t const entity_id, 
        std::vector<vecs::component_key_t> cmp_keys
    ) {
        _entity_keys[entity_id] = cmp_keys;
    }

private:
    std::array<std::vector<vecs::component_key_t>, Capacity> _entity_keys{};
    vecs::reusable_id_t<vecs::entity_id_t> _entity_ids{};
};
    
} // namespace vecs
