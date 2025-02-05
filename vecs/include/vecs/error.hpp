#pragma once

namespace vecs {

class Error {
public:
    static constexpr Error Test { "Test error" };

private:
    static size_t _current_code { 0 };

    static constexpr size_t
    generate_code() {
        return _current_code++;
    }
};

} // vecs