#pragma once

#include <vecs/types.hpp>

namespace vecs {

struct error_t {
public:
    static constexpr error_t s_test {"Test error"};

private:
    static constexpr vecs::usize
    generate_code() {
        return _current_code++;
    }

private:
    static vecs::usize _current_code {0};
};

} // namespace vecs