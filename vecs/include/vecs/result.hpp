#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

namespace vecs {

struct unit_t {
    static const unit_t ok;

    // rule of 5. :)
    unit_t() = default;
    unit_t(const unit_t&) = default;
    unit_t(unit_t&&) = default;
    unit_t&
    operator=(const unit_t&) = default;
    unit_t&
    operator=(unit_t&&) = default;
    ~unit_t() = default;
};

const unit_t unit_t::ok = unit_t();

template <typename T, typename E>
class result_t {
private:
    union {
        T value;
        E _error;
    };

    bool _is_ok;

public:
    result_t(const T& value) : value(value), _is_ok(true) {}

    result_t(T&& value) : value(std::move(value)), _is_ok(true) {}

    result_t(const E& err) : _error(err), _is_ok(false) {}

    result_t(E&& err) : _error(std::move(err)), _is_ok(false) {}

    ~result_t() {
        if (_is_ok) {
            value.~T();
        } else {
            _error.~E();
        }
    }

    result_t(const result_t&) = delete;
    result_t&
    operator=(const result_t&) = delete;

    // Allow move.
    result_t(result_t&& other) noexcept : _is_ok(other._is_ok) {
        if (_is_ok) {
            new (&value) T(std::move(other.value));
        } else {
            new (&_error) E(std::move(other._error));
        }
    }

    // Move if "copied".
    result_t&
    operator=(result_t&& other) noexcept {
        if (this != &other) {
            this->~result_t();
            _is_ok = other._is_ok;

            if (_is_ok) {
                new (&value) T(std::move(other.value));
            } else {
                new (&_error) E(std::move(other._error));
            }
        }

        return *this;
    }

    [[nodiscard]] constexpr bool
    is_error() const noexcept {
        return !_is_ok;
    }

    [[nodiscard]] constexpr bool
    is_ok() const noexcept {
        return _is_ok;
    }

    T&
    unwrap() const {
        if (!_is_ok) {
            throw std::runtime_error("Called unwrap() on an Err");
        }

        return value;
    }

    E&
    unwrap_err() const {
        if (_is_ok) {
            throw std::runtime_error("Called unwrap_err() on an Ok");
        }

        return _error;
    }

    template <typename F>
    T&
    unwrap_or_else(F&& function) const {
        if (_is_ok) {
            return value;
        }

        function();
        return nullptr;
    }
};

// Helper functions.
template <typename T, typename E>
result_t<T, E>
ok(T&& value) {
    return result_t<T, E>(std::forward<T>(value));
}

template <typename T, typename E>
result_t<T, E>
err(E&& err) {
    return result_t<T, E>(std::forward<E>(err));
}

} // namespace vecs