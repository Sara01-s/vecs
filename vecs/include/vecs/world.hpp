#pragma once

#include <functional>

#include "component_storage.hpp"
#include "entity_storage.hpp"
#include "query.hpp"
#include "scheduler.hpp"
#include "types.hpp"

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
    void 
    for_each(auto&& func) {
        auto q = query<Cs...>();
        for (auto&& components : q) {
            std::apply(func, components);
        }
    }

    template <typename... Args>
    [[nodiscard]] auto 
    query() {
        vecs::mask_t required_mask = 0;
        vecs::mask_t forbidden_mask = 0;

        ([&] {
            using RawT = std::remove_cvref_t<typename vecs::extract_type<Args>::type>;
            vecs::mask_t bit = _component_storage.get_type_mask<RawT>();

            if constexpr (vecs::is_exclusion<Args>::value) {
                forbidden_mask |= bit;
            } else {
                required_mask |= bit;
            }
        }(), ...);

        return _build_query_impl<Args...>(required_mask, forbidden_mask);
    }

    template <ScheduleLabel L>
    void
    run(L current_state) noexcept {
        assert(_scheduler.size() > 0 && "No systems registered.");
        _scheduler.run_systems(current_state, *this);
    }

private:
template <typename... Args>
    auto _build_query_impl(vecs::mask_t req_mask, vecs::mask_t forb_mask) {
        using concrete_query_t = typename vecs::filter_types<Args...>::query_type;
        concrete_query_t q {};

        for (auto& [arch_mask, entry] : _component_storage.get_archetypes_map()) {
            if ((arch_mask & req_mask) == req_mask && (arch_mask & forb_mask) == 0) {
                _fill_match<Args...>(q, entry);
            }
        }
        return q;
    }

    template <typename... Args, typename Q>
    void 
    _fill_match(Q& query, auto& entry) {
        typename Q::match_t match;
        match.instance_ptr = &entry.instance;
        match.get_size_fn = entry.get_size;

        std::vector<component_accessor_fn> real_accessors {};
        
        ([&] {
            if constexpr (!is_filter<Args>::value) {
                using T = std::remove_cvref_t<Args>;
                real_accessors.push_back(entry.accessors.at(typeid(T).hash_code()));
            }
        }(), ...);

        for (size_t i = 0; i < real_accessors.size(); ++i) {
            match.component_accessors[i] = real_accessors[i];
        }

        query.matched_archetypes.push_back(match);
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