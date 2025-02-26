#pragma once

#include <unordered_set>
#include <cassert>
#include <algorithm>
#include <span>

namespace vecs {

template <typename Id = vecs::usize>
struct reusable_id_t final {
    Id
    next_free_id() noexcept {
        if (!_available_ids.empty()) {
            auto it = _available_ids.begin();
            Id next_id = *it;

            _available_ids.erase(it);
            _used_ids.insert(next_id);

            return next_id;
        }

        Id next_id = _next_id++;
        _used_ids.insert(next_id);

        return next_id;
    }

    void
    erase(Id id) noexcept {
        assert(_used_ids.contains(id));
        assert(!_available_ids.contains(id));

        _used_ids.erase(id);
        _available_ids.insert(id);
    }

private:
    // Switch to use swap back array to avoid heap-allocations and O(log n) operations.
    std::unordered_set<Id> _used_ids{};
    std::unordered_set<Id> _available_ids{};
        
    Id _next_id{};
};

} // namespace vecs