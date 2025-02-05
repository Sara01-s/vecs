#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

namespace vecs {

struct Unit {
    static const Unit ok;

    // 3, 5 rule. :)
    Unit() = default;
    Unit(const Unit&) = default;
    Unit(Unit&&) = default;
};

const Unit Unit::ok = Unit();

template <typename T, typename E>
class Result {
private:
    union {
        T value;
        E _error;
    };
    
    bool _is_ok;

public:
    Result(const T& val) : value(val), _is_ok(true) {}
    Result(T&& val) : value(std::move(val)), _is_ok(true) {}
    
    Result(const E& err) : _error(err), _is_ok(false) {}
    Result(E&& err) : _error(std::move(err)), _is_ok(false) {}

    ~Result() {
        if (_is_ok) {
            value.~T();
        } 
        else {
            _error.~E();
        }
    }

    Result(const Result&) = delete;
    Result& operator=(const Result&) = delete;

    // Allow move.
    Result(Result&& other) noexcept : _is_ok(other._is_ok) {
        if (_is_ok) {
            new (&value) T(std::move(other.value));
        } 
        else {
            new (&_error) E(std::move(other._error));
        }
    }

    // Move if "copied".
    Result& 
    operator=(Result&& other) noexcept {
        if (this != &other) {
            this->~Result(); // Destruir el contenido actual
            _is_ok = other._is_ok;

            if (_is_ok) {
                new (&value) T(std::move(other.value));
            } 
            else {
                new (&_error) E(std::move(other._error));
            }
        }

        return *this;
    }

    [[nodiscard]] constexpr bool is_error() const noexcept { return !_is_ok; }
    [[nodiscard]] constexpr bool is_ok() const noexcept { return _is_ok; }

    const T& 
    unwrap() const {
        if (!_is_ok) {
            throw std::runtime_error("Called unwrap() on an Err");
        }

        return value;
    }

    const E& 
    unwrap_err() const {
        if (_is_ok) {
            throw std::runtime_error("Called unwrap_err() on an Ok");
        } 

        return _error;
    }

    template <typename F>
    const T& 
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
Result<T, E> 
Ok(T&& val) {
    return Result<T, E>(std::forward<T>(val));
}

template <typename T, typename E>
Result<T, E> 
Err(E&& err) {
    return Result<T, E>(std::forward<E>(err));
}

}  // namespace vecs