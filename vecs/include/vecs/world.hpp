#pragma once

#include "component_storage.hpp"
#include "entity_storage.hpp"
#include "query.hpp"

namespace vecs {

struct world_t final {
    using log_t = vecs::log_t; // TODO - delete.
    world_t() = default;

    template <Component... Cs>
    vecs::entity_id_t const
    spawn_entity(Cs... components) noexcept {
        vecs::entity_id_t const entity_id = _entity_storage.add_entity();

        auto cmp_keys = _component_storage.add_components(
            entity_id,
            std::move(components)...
        );

        _entity_storage.add_component_keys(entity_id, cmp_keys);

        log_t::log(
            log_t::LIGHT_GREEN,
            "Spawned ",
            log_t::CLEAR,
            "entity with ID: ",
            entity_id
        );
        log_t::log(
            "  ╰> Mask: ",
            log_t::LIGHT_MAGENTA,
            "0b",
            _component_storage.get_entity_mask(entity_id)
        );
        log_t::log("  ╰> Component keys: ");
        for (component_key_t const key : cmp_keys) {
            log_t::log("      ╰> ", log_t::LIGHT_MAGENTA, "0x", std::hex, key);
        }

        return entity_id;
    }

    void
    despawn_entity(vecs::entity_id_t entity_id) noexcept {
        _entity_storage.remove_entity(entity_id);
        _component_storage.remove_components(entity_id);

        log_t::log(
            log_t::LIGHT_RED,
            "Despawned ",
            log_t::CLEAR,
            "entity with ID: ",
            entity_id
        );
        log_t::log(
            "  ╰> Mask cleared: ",
            log_t::LIGHT_MAGENTA,
            _component_storage.get_entity_mask(entity_id)
        );
    }

    template <Component... Cs>
    void
    register_components() noexcept {
        _component_storage.register_components<Cs...>();
    }

    template <typename... Cs>
    void
    for_each(auto&& system) {
        _component_storage.for_each<Cs...>(std::forward<decltype(system)>(system
        ));
    }

    template <Component... Cs>
    [[nodiscard]] query_t<Cs...>
    query() {
        query_t<Cs...> query {};

        assert(false && "Not implemented yet.");

        return query;
    }

private:
    vecs::component_storage_t<
        vecs::MAX_REGISTRABLE_COMPONENTS,
        vecs::MAX_ALIVE_ENTITIES>
        _component_storage {};

    vecs::entity_storage_t<
        vecs::MAX_ALIVE_ENTITIES,
        vecs::MAX_REGISTRABLE_COMPONENTS>
        _entity_storage {};
};

} // namespace vecs