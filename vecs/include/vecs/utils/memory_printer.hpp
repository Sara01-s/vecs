#pragma once

#include <cstdio>
#include <stdint.h>

constexpr char const* RESET      = "\033[0m";
constexpr char const* BLACK      = "\033[30m";
constexpr char const* RED        = "\033[31m";
constexpr char const* GREEN      = "\033[32m";
constexpr char const* YELLOW     = "\033[33m";
constexpr char const* BLUE       = "\033[34m";
constexpr char const* MAGENTA    = "\033[35m";
constexpr char const* CYAN       = "\033[36m";
constexpr char const* WHITE      = "\033[37m";
constexpr char const* BOLD       = "\033[1m";
constexpr size_t DEFAULT_WIDTH { 16 };

size_t
get_needed_display_lines(size_t const size, size_t const width = DEFAULT_WIDTH) {
    /*
        This function performs an integer division. For example,
        if we want to display 24 bytes using 16 bytes per line, 24 / 16 equals 1. (fractional part is discarded)
        Since 24 bytes cannot be displayed in only 1 full line of 16 bytes,
        we check if there is a remainder (i.e., if size % width != 0) 
        and, if so, add an extra line.
    */
    auto const lines { size / width };

    if (size % width != 0) { // Size is odd.
        return lines + 1;
    }

    return lines;
}

void
print_memory_line(uint8_t const* memory, size_t const width = DEFAULT_WIDTH) {
    /*
        %p prints a pointer (usable only with void* arguments). 
        The notation '%p16' is intended to force the printing of the pointer with 16 digits, 
        although this is not part of the standard and may not work on all compilers.

        -- Note on: (void*)memory

            A C-style cast first attempts a static_cast<void*>; if that fails,
            it falls back to a reinterpret_cast<void*> (which is the most common case).
            This approach can lead to undefined behavior in many scenarios,
            so it should generally be avoided. In this instance, 
            it is acceptable since we are only **reading** from memory (const*).
            However, using it for **writing** to memory could result in undefined behavior.
    */
    std::printf("|| %p16 ||", (void*)memory);

    for (size_t i{}; i < width; ++i) {
        // %02X formats the data of memory[i] as a two-digit hexadecimal number.
        std::printf(" %02X", memory[i]);
    }

    std::printf(" || ");
    for (size_t i{}; i < width; ++i) {
        // src: https://www.ascii-code.com/#:~:text=ASCII%20printable%20characters%20(character%20code%2032-127)
        // Although the 'SPC' and 'DEL' characters (32 and 127) are technically valid, 
        // it is preferable to avoid displaying them.
        bool char_is_ascii_printable = (33 <= memory[i] && memory[i] <= 126);
        auto character = char_is_ascii_printable ? memory[i] : '.';

        std::printf("%c", character);
    }

    std::printf(" ||\n");
}

void
print_memory(
    uint8_t const* memory, 
    size_t const size,
    size_t const width = DEFAULT_WIDTH
) {
    auto const display_lines { get_needed_display_lines(size, width) };

    std::printf(GREEN);
    std::printf("[]------------------[]-------------------------------------------------[]------------------[]\n");
    std::printf("||     Address      || 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F ||     Char view    ||\n");
    std::printf("[]------------------[]-------------------------------------------------[]------------------[]\n");
    for (size_t i{}; i < display_lines; ++i) {
        print_memory_line(memory, width);
        /*
            After displaying a line, we advance the memory pointer by the specified width. 
            Otherwise, we would display the same data repeatedly.
        */
        memory += width;
    }
    std::printf("[]------------------[]-------------------------------------------------[]------------------[]\n");
    std::printf(RESET);
}

void // This is the same as making a template (auto parameters are a feature of C++20).
print_memory_object(auto const& obj) {
    /*
        By reinterpret_casting to a uint8_t (unsigned char) const*, 
        we inform the compiler that we intend to **only read** the memory.
        This behavior is allowed by the standard.
    */
    uint8_t const* ptr = reinterpret_cast<uint8_t const*>(&obj);

    std::printf("\n");
    std::printf("##### Memory Inspector.\n");
    std::printf("##### Showing object of size: %ld.\n", sizeof(obj));
    std::printf(">>>\n");

    print_memory(ptr, sizeof(obj));
}

void // This is the same as making a template (auto parameters are a feature of C++20).
print_memory_ptr(auto const* memory, size_t const size) {
    uint8_t const* ptr = reinterpret_cast<uint8_t const*>(memory);

    std::printf("\n");
    std::printf("##### Memory Inspector.\n");
    std::printf("##### Showing pointer's data with a size of: %ld.\n", size);
    std::printf(">>>\n");

    print_memory(ptr, size);
}

