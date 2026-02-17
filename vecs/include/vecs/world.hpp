#pragma once

#include <functional>

#include "component_storage.hpp"
#include "entity_storage.hpp"
#include "query.hpp"
#include "scheduler.hpp"

namespace vecs {

struct world_t final {
    using log_t = vecs::log_t;
public:
    world_t() = default;
    world_t(world_t const&) = delete;
    world_t(world_t&&) = delete;

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
            "  ╰> Mask cleared, now: ",
            log_t::LIGHT_MAGENTA,
            _component_storage.get_entity_mask(entity_id)
        );
    }

    template <Component... Cs>
    void
    register_components() noexcept {
        _component_storage.register_components<Cs...>();
    }

    template <ScheduleLabel L>
    void
    add_system(L state, system_t system) noexcept {
        _scheduler.add_system(state, system);
    }

    template <typename... Cs>
    void for_each(auto&& func) {
        auto q = query<Cs...>();
        for (auto&& components : q) {
            std::apply(func, components);
        }
    }

    template <Component... Cs>
    [[nodiscard]] auto query() {
        vecs::query_t<Cs...> q;
        vecs::mask_t const req_mask = ( _component_storage.get_type_mask<std::remove_cvref_t<Cs>>() | ... );

        for (auto& [mask, entry] : _component_storage.get_archetypes_map()) {
            if ((mask & req_mask) == req_mask) {
                typename vecs::query_t<Cs...>::match_t match;
                match.instance_ptr = &entry.instance;
                match.get_size_fn = entry.get_size;
                
                match.component_accessors = { 
                    entry.accessors.at(typeid(std::remove_cvref_t<Cs>).hash_code())... 
                };
                
                q.matched_archetypes.push_back(match);
            }
        }
        return q;
    }

    template <ScheduleLabel L>
    void
    run(L current_state) noexcept {
        assert(_scheduler.size() > 0 && "No systems registered.");
        _scheduler.run_systems(current_state, *this);
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

    vecs::scheduler_t _scheduler {};
};

} // namespace vecs