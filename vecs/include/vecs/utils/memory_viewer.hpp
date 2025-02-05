#pragma once

// std
#include <cstdio>
#include <stdint.h>
#include <memory>
#include <unordered_map>

namespace vecs {

static constexpr size_t DEFAULT_WIDTH = 16;
static constexpr char const* GREEN_ANSI_HIGHLIGHT = "\033[42m";
static constexpr char const* CLEAR_ANSI = "\033[0m";

template<typename T>
class MemoryViewer {
private:
    std::shared_ptr<T> const _trackedObj{};
    std::unordered_map<uintptr_t, uint8_t> _trackedBytes{};
    std::unordered_map<uintptr_t, uint8_t> _trackedChars{};

public:
    MemoryViewer(T const& obj) 
        : _trackedObj(std::make_shared<T>(obj)) {}

    void
    print() {
        _print_memory_object(**_trackedObj);
    }

private:
    size_t
    _get_needed_display_lines(size_t const size, size_t const width = DEFAULT_WIDTH) {
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
        // This whole funciton could be reduced to: return (size * width - 1) / width;
    }

    void
    _print_memory_line(
        uint8_t const* memory,
        size_t const size,
        size_t const line_number,
        size_t const width = DEFAULT_WIDTH
    ) {
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
            auto const byte_pos = (line_number * width + i);
            uintptr_t address = reinterpret_cast<uintptr_t>(&memory[i]);

            if (byte_pos >= size) {
                std::printf(" --");
            }
            else {
                if (_trackedBytes.contains(address) && _trackedBytes[address] != memory[i]) {
                    std::printf(" %s%02X%s", GREEN_ANSI_HIGHLIGHT, memory[i], CLEAR_ANSI);
                }
                else {
                    // %02X formats the data of memory[i] as a two-digit hexadecimal number.
                    std::printf(" %02X", memory[i]);
                }

                _trackedBytes[address] = memory[i];
            }
        }

        std::printf(" || ");
        for (size_t i{}; i < width; ++i) {
            uintptr_t address = reinterpret_cast<uintptr_t>(&memory[i]);

            /*
                src: https://www.ascii-code.com/#:~:text=ASCII%20printable%20characters%20(character%20code%2032-127)
                Although the 'SPC' and 'DEL' characters (32 and 127) are technically valid, 
                it is preferable to avoid displaying them.
            */
            bool char_is_ascii_printable = (33 <= memory[i] && memory[i] <= 126);
            auto character = char_is_ascii_printable ? memory[i] : '.';

            if (_trackedChars.contains(address) && _trackedChars[address] != memory[i]) {
                std::printf("%s%c%s", GREEN_ANSI_HIGHLIGHT, character, CLEAR_ANSI);
            }
            else {
                std::printf("%c", character);
            }

            _trackedChars[address] = memory[i];
        }

        std::printf(" ||\n");
    }

    void
    _print_memory(
        uint8_t const* memory, 
        size_t const size,
        size_t const width = DEFAULT_WIDTH
    ) {
        auto const display_lines { _get_needed_display_lines(size, width) };

        std::printf("[]------------------[]-------------------------------------------------[]------------------[]\n");
        std::printf("||     Address      || 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F ||   Decoded text   ||\n");
        std::printf("[]------------------[]-------------------------------------------------[]------------------[]\n");
        for (size_t line_number{}; line_number < display_lines; ++line_number) {
            _print_memory_line(memory, size, line_number, width);
            /*
                After displaying a line, we advance the memory pointer by the specified width. 
                Otherwise, we would display the same data repeatedly.
            */
            memory += width;
        }
        std::printf("[]------------------[]-------------------------------------------------[]------------------[]\n");
    }

    void
    _print_memory_object(auto const& obj) {
        uint8_t const* ptr = reinterpret_cast<uint8_t const*>(&obj);

        std::printf("\n");
        std::printf("##### Memory Inspector.\n");
        std::printf("##### Showing object of size: %ld bytes. (%.2fkb)\n", sizeof(obj), sizeof(obj) / 1024.0);
        std::printf(">>>\n");

        _print_memory(ptr, sizeof(obj));
    }

    void
    _print_memory_ptr(auto const* memory, size_t const size) {
        uint8_t const* ptr = reinterpret_cast<uint8_t const*>(memory);

        std::printf("\n");
        std::printf("##### Memory Inspector.\n");
        std::printf("##### Showing pointer's data with a size of: %ld bytes. (%.2fkb)\n", sizeof(size), sizeof(size) / 1024.0);
        std::printf(">>>\n");
        
        _print_memory(ptr, size);
    }
};

} // namespace vecs