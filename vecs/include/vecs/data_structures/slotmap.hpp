#pragma once

// std
#include <array>
#include <cassert>
#include <cstdint>
#include <stdexcept>

// lib
#include "../debug.hpp"
#include "../types.hpp"

namespace vecs {

template <typename T, vecs::usize Capacity = 10, typename Index = vecs::u32>
class slotmap_t final {
public:
    using index_t = Index;
    using component_type_t = T;

    // Assert Capacity does not exceed the maximum value for 32-bit index_t.
    static_assert(
        Capacity <=
            static_cast<vecs::usize>(std::numeric_limits<index_t>::max()),
        "Capacity must be <= 2^32 - 1 for id to fit in 32 bits"
    );

    using gen_t = vecs::u32;

    using slot_t = struct {
        index_t data_id;
        gen_t generation;
    };

    // `key_t` is a `64-bit` representation of an `id` and a `generation`.
    // `id` is encoded in the `hi 32` bits of `key_t`.
    // `generation` is encoded in the `lo 32` bits of `key_t`.
    // (e.g.)
    //  | 0000 0000 0000 0011 | 0000 0000 0000 0001 |
    //  |          id          |       generation     |
    using key_t = vecs::u64;

    using iterator_t = T*;
    using const_iterator_t = T const*;

private:
    // Debugging tags (can be removed if debug.hpp is not used)
    debug_tag_t<8> _size_tag {"#_size#"};
    index_t _size {};

    debug_tag_t<8> _freelist_tag {"#_free#"};
    index_t _freelist {};

    debug_tag_t<8> _generation_tag {"#_gene#"};
    gen_t _generation {};

    debug_tag_t<16> _indices_tag {"#_ids####_gene#"};
    std::array<slot_t, Capacity> _indices {};

    debug_tag_t<16> _data_tag {"#_data#########"};
    std::array<T, Capacity> _data {};

    debug_tag_t<16> _erase_tag {"#_erase########"};
    std::array<index_t, Capacity> _erase {};

public:
    constexpr explicit slotmap_t() {
        clear();
    }

    // Getters.
    [[nodiscard]] constexpr vecs::usize
    size() const noexcept {
        return _size;
    }

    [[nodiscard]] static constexpr vecs::usize
    capacity() noexcept {
        return Capacity;
    }

    // Fix: MSVC iterators are not raw pointers. Use .data() to get T*.
    [[nodiscard]] constexpr iterator_t
    begin() noexcept {
        return _data.data();
    }

    [[nodiscard]] constexpr iterator_t
    end() noexcept {
        return _data.data() + _size;
    }

    [[nodiscard]] constexpr const_iterator_t
    cbegin() const noexcept {
        return _data.data();
    }

    [[nodiscard]] constexpr const_iterator_t
    cend() const noexcept {
        return _data.data() + _size;
    }

    [[nodiscard]] constexpr key_t
    push_back(T const& value) {
        return push_back(T {value});
    };

    [[nodiscard]] constexpr key_t
    push_back(T&& value) {
        index_t const reserved_slot_id = _allocate_slot();
        slot_t const& slot = _indices[reserved_slot_id];

        _data[slot.data_id] = std::move(value);
        _erase[slot.data_id] = reserved_slot_id;

        // Pack id and generation into key_t.
        key_t key = (static_cast<vecs::u64>(reserved_slot_id) << 32) |
            static_cast<vecs::u64>(slot.generation);

        return key;
    }

    constexpr bool
    erase(key_t key) noexcept {
        // Unpack id and generation from key.
        auto id = static_cast<vecs::u32>(key >> 32);
        auto generation = static_cast<vecs::u32>(key);

        // Validate the key.
        if (id >= Capacity || _indices[id].generation != generation) {
            return false;
        }

        _free_slot(id);
        return true;
    }

    [[nodiscard]]
    constexpr bool
    is_key_valid(key_t key) const noexcept {
        // Unpack id and generation from key.
        vecs::u32 id = static_cast<vecs::u32>(key >> 32);
        vecs::u32 generation = static_cast<vecs::u32>(key);

        if (id >= Capacity || _indices[id].generation != generation) {
            return false;
        }

        return true;
    }

    constexpr void
    clear() noexcept {
        _init_freelist();
    }

    [[nodiscard]] constexpr T&
    operator[](index_t index) {
        return _data[index];
    }

    [[nodiscard]] constexpr T const&
    operator[](index_t index) const {
        return _data[index];
    }

private:
    constexpr void
    _init_freelist() noexcept {
        for (index_t i {}; i < static_cast<index_t>(_indices.size()); ++i) {
            _indices[i].data_id = i + 1; // Store next free index.
        }

        _freelist = 0; // Point to first free index.
    }

    [[nodiscard]]
    constexpr index_t
    _allocate_slot() {
        if (_size >= Capacity) {
            throw std::runtime_error(
                "Failed to add item to slot_map: No space left."
            );
        }

        assert(_freelist < Capacity);

        // Reserve slot.
        index_t slot_id = _freelist;
        _freelist = _indices[slot_id].data_id; // Freelist -> first free.

        // Init slot.
        auto& slot = _indices[slot_id];
        slot.data_id = _size;
        slot.generation = _generation;

        // Update space and generation.
        ++_size;
        ++_generation;

        return slot_id;
    }

    constexpr void
    _free_slot(index_t slot_id) noexcept {
        auto& slot = _indices[slot_id];
        auto data_id = slot.data_id; // Save slot to check if it is last or not.

        slot.data_id = _freelist;
        slot.generation = _generation;
        _freelist = slot_id;

        if (data_id != _size - 1) {
            // Data slot is not last, copy last here.
            _data[data_id] = std::move(_data[_size - 1]);
            _erase[data_id] = _erase[_size - 1];
            _indices[_erase[data_id]].data_id = data_id;
        }

        // Update size and generation.
        --_size;
        ++_generation;
    }
};

} // namespace vecs