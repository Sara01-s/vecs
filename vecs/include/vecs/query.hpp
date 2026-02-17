#pragma once

// std
#include <tuple>
#include <vector>
#include <array>
#include <utility>

// lib
#include "types.hpp"

namespace vecs {

template <typename... Cs>
class query_t {
public:
    struct match_t {
        void* instance_ptr;
        archetype_size_fn get_size_fn;
        std::array<component_accessor_fn, sizeof...(Cs)> component_accessors;
    };

    std::vector<match_t> matched_archetypes {};

    struct iterator {
        std::vector<match_t>& matches;
        size_t arch_idx {};
        size_t ent_idx {};

        template <size_t... Is>
        auto get_row(std::index_sequence<Is...>) {
            auto& m = matches[arch_idx];
            return std::forward_as_tuple(
                *static_cast<std::remove_reference_t<Cs>*>(
                    m.component_accessors[Is](m.instance_ptr, static_cast<vecs::u32>(ent_idx))
                )...
            );
        }

        auto operator*() {
            return get_row(std::index_sequence_for<Cs...>{});
        }

        iterator& operator++() {
            ent_idx++;
            vecs::usize const size = matches[arch_idx].get_size_fn(matches[arch_idx].instance_ptr);
            while (arch_idx < matches.size() && ent_idx >= size) {
                arch_idx++;
                ent_idx = 0;
            }
            return *this;
        }

        bool operator!=(const iterator& other) const {
            return arch_idx != other.arch_idx || ent_idx != other.ent_idx;
        }
    };

    iterator begin() { 
        size_t first_arch = 0;
        // Buscamos el primer arquetipo que no esté vacío
        while (first_arch < matched_archetypes.size() && 
               matched_archetypes[first_arch].get_size_fn(matched_archetypes[first_arch].instance_ptr) == 0) {
            first_arch++;
        }
        return iterator{matched_archetypes, first_arch, 0}; 
    }
    
    iterator end() { 
        return iterator{matched_archetypes, matched_archetypes.size(), 0}; 
    }
};

} // namespace vecs