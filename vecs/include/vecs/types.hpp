#pragma once

#include <cstdint>

namespace vecs {

    using u8  = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    using i8  = std::int8_t;
    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;

#ifdef __SIZEOF_INT128__
    using u128 = __uint128_t;
    using i128 = __int128_t;
#else
    #error "128-bit integers are not supported on this platform."
#endif

    using f32 = float;
    using f64 = double;

#ifdef __SIZEOF_FLOAT128__
    using f128 = long double;
#else
    #error "128-bit floats are not supported on this platform."
#endif

    using usize = std::size_t;
    using isize = std::ptrdiff_t;
    using uptr = std::uintptr_t;
    using iptr = std::intptr_t;

    using c8  = char;
    using uc8 = unsigned char;
    using sc8 = signed char;
    using wc16 = wchar_t;       // wide chars such as L'Ω'. Omega (U+03A9).

} // namespace vecs
