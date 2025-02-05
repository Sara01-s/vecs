#pragma once

namespace vecs {

class Error {
public:
    static constexpr Error Test { "Test error" };

private:
    static size_t _current_code { 0 };

    constexpr size_t
    generate_code() const {
        return _current_code++;
    }
}

} // vecs