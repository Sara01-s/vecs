#pragma once

// std
#include <array>
#include <stdint.h>
#include <stdexcept>
#include <cassert>

namespace vecs {

template <typename T, size_t Capacity = 10, typename TIndex = uint64_t>
struct SlotMap {
public:
    using index_t = TIndex;
    using gen_t   = index_t;
    using key_t   = struct { index_t id; gen_t generation; };
    using iterator = T*;
    using const_iterator = T const*;

private:
    // if using this header file alone, delete all debug tags (they are only for memory debugging).
    DebugTag<8> _size_tag       { "#_size#" };
    index_t _size{};

    DebugTag<8> _freelist_tag   { "#_free#" };
    index_t _freelist{};

    DebugTag<8> _generation_tag { "#_gene#" };
    gen_t _generation{};

    DebugTag<16> _indices_tag    { "#_indices#keys#" };
    std::array<key_t, Capacity> _indices{}; // keys

    DebugTag<16> _data_tag       { "#_data#########" };
    std::array<T, Capacity> _data{};

    DebugTag<16> _erase_tag      { "#_erase########" };
    std::array<index_t, Capacity> _erase{};

public:
    constexpr explicit SlotMap() {
        clear();
    }

    [[nodiscard]] inline constexpr size_t size() const noexcept { return _size; }
    [[nodiscard]] inline constexpr size_t capacity() const noexcept { return Capacity; }
    [[nodiscard]] constexpr iterator begin() noexcept { return _data.begin(); }
    [[nodiscard]] constexpr iterator   end() noexcept { return _data.begin() + _size; }
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return _data.cbegin(); }
    [[nodiscard]] constexpr const_iterator   cend() const noexcept { return _data.cbegin() + _size; }

    [[nodiscard]] constexpr key_t push_back(T const& value) { return push_back(T { value }); };
    [[nodiscard]] constexpr key_t push_back(T&& value) {
        index_t reserved_slot_id = _allocate_slot();
        auto& slot = _indices[reserved_slot_id];

        // Move data.
        _data[slot.id] = std::move(value); // Move is like "cast to rvalue" (at low level is not that).
        _erase[slot.id] = reserved_slot_id;

        // Copy slot and generate key for user.
        key_t key { slot };
        key.id = reserved_slot_id;

        return key;
    }

    constexpr bool
    erase(key_t key) noexcept {
        if (!is_key_valid(key)) {
            return false;
        }

        _free_slot(key);
        return true; 
    }

    [[nodiscard]]
    constexpr bool 
    is_key_valid(key_t key) const noexcept {
        if (key.id >= Capacity || key.generation != _indices[key.id].generation) {
            return false;
        }

        return true;
    }

    constexpr void clear() noexcept { _init_freelist(); }

private:
    constexpr void
    _init_freelist() noexcept {
        for (index_t i{}; i < _indices.size(); ++i) {
            _indices[i].id = i + 1; // Store next free index.
        }

        _freelist = 0; // Point to first free index.
    }

    [[nodiscard]]
    constexpr index_t
    _allocate_slot() {
        if (_size >= Capacity) {
            throw std::runtime_error("Failed to add item to slotmap: No space left.");
        }

        assert(_freelist < Capacity);

        // Reserve slot.
        index_t slot_id = _freelist;
        _freelist = _indices[slot_id].id; // Freelist -> first free.

        // Init slot.
        auto& slot = _indices[slot_id];
        slot.id = _size;
        slot.generation = _generation;

        // Update space and generation.
        ++_size;
        ++_generation;
        
        return slot_id;
    }

    constexpr void
    _free_slot(key_t key) noexcept {
        assert(is_key_valid(key));

        auto& slot = _indices[key.id];
        auto data_id = slot.id; // Save slot to check if it is last or not.

        slot.id = _freelist;
        slot.generation = _generation;
        _freelist = key.id;

        if (data_id != _size - 1) { // Data slot is not last.
            // Data slot is not last, copy last here.

            _data[data_id] = _data[_size - 1];
            _erase[data_id] = _erase[_size - 1];
            _indices[_erase[data_id]].id = data_id;
        }

        // Update size and generation.
        --_size;
        ++_generation;
    }
};

}
