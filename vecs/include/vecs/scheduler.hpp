#pragma once

#include <functional>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "types.hpp"

namespace vecs {

struct world_t;

struct scheduler_t {
    static constexpr vecs::u32 MAX_SYSTEMS {64};

    scheduler_t() = default;
    ~scheduler_t() = default;

    vecs::usize
    size() const noexcept {
        return _systems.size();
    }

    template <ScheduleLabel L>
    void
    add_system(L state, system_t system) noexcept {
        assert(_systems.size() < MAX_SYSTEMS && "Maximum systems reached.");
        schedule_label_id_t const label_index = typeid(state).hash_code();

        if (_systems.find(label_index) != _systems.end()) {
            _systems[label_index].push_back(system);
        } else {
            _systems[label_index] = {system};
        }
    }

    template <ScheduleLabel L>
    void
    run_systems(L current_state, vecs::world_t& world) noexcept {
        schedule_label_id_t const label_index =
            typeid(current_state).hash_code();

        if (_systems.contains(label_index)) {
            for (auto const& system : _systems[label_index]) {
                system(world);
            }
        }
    }

private:
    std::unordered_map<schedule_label_id_t, std::vector<system_t>> _systems {};
};

} // namespace vecs
