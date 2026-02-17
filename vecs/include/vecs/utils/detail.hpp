#pragma once
#include "../types.hpp"

namespace detail {
    // Busca el índice de un componente C en la lista de tipos del arquetipo
    template <typename C, typename... ArchetypeCs>
    constexpr std::size_t find_component_index() {
        constexpr bool matches[] = { std::is_same_v<std::remove_cvref_t<C>, std::remove_cvref_t<ArchetypeCs>>... };
        for (std::size_t i = 0; i < sizeof...(ArchetypeCs); ++i) {
            if (matches[i]) return i;
        }
        return sizeof...(ArchetypeCs); // Esto dispararía el static_assert si falla
    }
}