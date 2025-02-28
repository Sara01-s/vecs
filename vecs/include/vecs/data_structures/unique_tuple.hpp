#pragma once

#include <tuple>
#include <type_traits>

namespace vecs {

template <typename T, typename... Ts>
concept UniqueTypes = (!std::is_same_v<T, Ts> && ...);

template <UniqueTypes... Ts>
struct unique_tuple_t final {
    static_assert(sizeof...(Ts) > 0, 
        "unique_tuple_t must have at least 1 unique type.");

    // Ok the reason for the existance of this function when unique_tuple_t
    // can easily be converted to std::tuple is to provide a way to access
    // the first element of the tuple without converting the object to std::tuple.
    // This is useful if the compiler cannot deduce the type of the tuple.
    // Yeah, it happened before, that's why this function exists.
    constexpr auto& first() noexcept {
        return std::get<0>(_data);
    }

    // Returns a const reference to the underlying tuple data.
    // This provides a read-only view of the data without transferring ownership.
    constexpr auto const& data() const noexcept {
        return _data;
    }

    // Implicit conversion to std::tuple
    operator std::tuple<Ts...>&() & noexcept {
        return _data;
    }

    // Implicit const conversion to std::tuple
    operator const std::tuple<Ts...>&() const& noexcept {
        return _data;
    }

    operator std::tuple<Ts...>() && noexcept {
        return std::move(_data);
    }

private:
    std::tuple<Ts...> _data{};
};

} // namespace vecs
