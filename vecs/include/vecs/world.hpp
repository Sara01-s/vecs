#pragma once

#include "component_storage.hpp"
#include "entity_storage.hpp"

namespace vecs {

template <Component... RegisteredComponents>
struct world_t final {
    using log_t = vecs::debug_t; // TODO - delete.
    world_t() = default;

    template <Component... Cs>
    vecs::entity_id_t const
    spawn_entity(Cs... components) noexcept {
        vecs::entity_id_t const entity_id = _entity_storage.add_entity();
        auto cmp_keys = _component_storage.add_components(
                                          entity_id, std::move(components)...);

        _entity_storage.add_component_keys(entity_id, cmp_keys);

        log_t::log(log_t::LIGHT_GREEN, "Spawned ", log_t::CLEAR, "entity with ID: ", entity_id);
        log_t::log("  ╰> ", log_t::GRAY, "Mask: ", _component_storage.get_entity_mask(entity_id));
        return entity_id;
    }

    void
    despawn_entity(vecs::entity_id_t entity_id) noexcept {
        _entity_storage.remove_entity(entity_id);
        _component_storage.remove_components(entity_id);

        log_t::log(log_t::LIGHT_RED, "Despawned ", log_t::CLEAR, "entity with ID: ", entity_id);
        log_t::log("  ╰> ", log_t::GRAY, "Mask cleared: ", _component_storage.get_entity_mask(entity_id));
    }

private:
    vecs::component_storage_t<
        MAX_REGISTRABLE_COMPONENTS,
        MAX_ALIVE_ENTITIES,
        RegisteredComponents...> _component_storage{};

    vecs::entity_storage_t<
        MAX_ALIVE_ENTITIES,
        MAX_REGISTRABLE_COMPONENTS> _entity_storage{};
};

} // namespace vecs