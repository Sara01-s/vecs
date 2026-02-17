#pragma once

// std
#include <any>
#include <cassert>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <tuple>

// lib
#include "archetype.hpp"
#include "types.hpp"

namespace vecs {

using component_accessor_fn = void* (*)(void*, vecs::u32);
using archetype_size_fn = size_t (*)(void*);

struct archetype_entry_t {
    std::any instance;
    std::unordered_map<size_t, component_accessor_fn> accessors;
    archetype_size_fn get_size;
};

template <vecs::usize MaxComponents = 64, vecs::usize MaxAliveEntities = 64>
class component_storage_t final {
    using log_t = vecs::log_t;
public:

    component_storage_t() = default;
    ~component_storage_t() = default;

    template <Component... Cs>
    void 
    register_components() noexcept {
        std::array<std::type_info const*, sizeof...(Cs)> components_type_info { &typeid(Cs)... };
        _register_components_metadata<Cs...>(components_type_info);
    }

    template <Component... Cs>
    std::vector<component_key_t> 
    add_components(vecs::entity_id_t const entity_id, Cs&&... components) noexcept {
        vecs::mask_t mask = (_component_masks[typeid(Cs).hash_code()] | ...);
        _entity_masks[entity_id] = mask;

        if (!_is_archetype_registered(mask)) {
            _register_archetype<Cs...>();
        }

        auto& entry = _archetypes[mask];
        auto& archetype = std::any_cast<vecs::archetype_t<Cs...>&>(entry.instance);

        return archetype.add_entity(std::forward<Cs>(components)...);
    }

    template <typename C>
    [[nodiscard]] auto& 
    get_component(vecs::entity_id_t entity_id) {
        using T = std::remove_cvref_t<C>;
        vecs::mask_t const mask = _entity_masks[entity_id];
        
        auto& entry = _archetypes.at(mask);
        
        auto const it = entry.accessors.find(typeid(T).hash_code());
        assert(it != entry.accessors.end() && "The archetype does not contain requested component.");

        void* raw_ptr = it->second(&entry.instance, static_cast<vecs::u32>(entity_id));
        
        return *static_cast<T*>(raw_ptr);
    }

    void 
    remove_components(vecs::entity_id_t entity_id) {
        _entity_masks[entity_id] = 0;
    }

    // --- GETTERS FOR WORLD/QUERY ---
    [[nodiscard]] auto& get_archetypes_map() noexcept {
        return _archetypes;
    }

    template <typename C>
    [[nodiscard]] vecs::mask_t get_type_mask() noexcept {
        size_t hash = typeid(std::remove_cvref_t<C>).hash_code();
        assert(_component_masks.contains(hash) && "Componente no registrado.");
        return _component_masks[hash];
    }

    [[nodiscard]] vecs::mask_t get_entity_mask(vecs::entity_id_t const entity_id) const noexcept {
        return _entity_masks[entity_id];
    }

private:
    template <Component... Cs>
    void 
    _register_archetype() noexcept {
        vecs::mask_t const mask = (_get_component_mask<Cs>() | ...);
        
        archetype_entry_t entry {};
        entry.instance = std::make_any<vecs::archetype_t<Cs...>>(mask);

        ([&] {
            entry.accessors[typeid(Cs).hash_code()] = [](void* any_ptr, vecs::u32 entity_idx) -> void* {
                auto* arch = std::any_cast<vecs::archetype_t<Cs...>>(static_cast<std::any*>(any_ptr));
                auto& table = arch->get_table(); 
                auto& slotmap = std::get<vecs::slotmap_t<Cs, vecs::MAX_INSTANCES_PER_COMPONENT>>(table);
                
                return static_cast<void*>(&slotmap[entity_idx]);
            };

            entry.get_size = [](void* any_ptr) -> size_t {
                auto* arch = std::any_cast<vecs::archetype_t<Cs...>>(static_cast<std::any*>(any_ptr));
                return arch->size();
            };
        }(), ...);

        _archetypes[mask] = std::move(entry);
    }

    template <Component... Cs>
    constexpr void 
    _register_components_metadata(auto const& components_type_info) noexcept {
        for (auto const& component_info : components_type_info) {
            vecs::component_id_t const component_id = component_info->hash_code();

            if (_component_masks.contains(component_id)) {
                continue;
            }

            _component_masks[component_id] = _next_component_mask;

            log_t::log(
                log_t::LIGHT_CYAN, "[Component Registry] ",
                log_t::CLEAR, "Registered: ",
                log_t::LIGHT_MAGENTA, component_info->name(),
                log_t::CLEAR, " | Mask: ",
                log_t::LIGHT_YELLOW, "0b", _next_component_mask,
                log_t::CLEAR, " (Hash: ", component_id, ")"
            );

            _next_component_mask <<= 1;
        }
    }

    template <Component C>
    [[nodiscard]] vecs::mask_t 
    _get_component_mask() noexcept {
        return _component_masks[typeid(C).hash_code()];
    }

    [[nodiscard]] bool 
    _is_archetype_registered(vecs::mask_t mask) const noexcept {
        return _archetypes.contains(mask);
    }

private:
    vecs::u64 _next_component_mask {0b1};

    std::unordered_map<vecs::component_id_t, vecs::mask_t> _component_masks {};
    std::array<vecs::mask_t, MaxAliveEntities> _entity_masks {};
    
    std::unordered_map<vecs::mask_t, archetype_entry_t> _archetypes {};
};

} // namespace vecs