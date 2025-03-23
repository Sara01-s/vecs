#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <typeinfo>

namespace vecs {

#define DEBUG // comment to deactivate.

template <size_t CharCount = 16>
struct debug_tag_t {
#ifdef DEBUG
    char tag[CharCount] {};

    constexpr explicit debug_tag_t(char const (&str)[CharCount]) {
        std::copy_n(str, CharCount, tag);
    }
#else
    constexpr debug_tag_t(char const (&)[CharCount]) {}
#endif
};

class log_t {
    static constexpr bool USE_COLOR {true};
    static constexpr char EOL {'\n'};

public:
    // This class is not meant to be instantiated (aka static class).
    log_t() = delete;
    ~log_t() = delete;
    log_t(const log_t&) = delete;

    static constexpr auto CLEAR {"\033[0m"};
    static constexpr auto BLACK {"\033[30m"};
    static constexpr auto RED {"\033[31m"};
    static constexpr auto GREEN {"\033[32m"};
    static constexpr auto YELLOW {"\033[33m"};
    static constexpr auto BLUE {"\033[34m"};
    static constexpr auto MAGENTA {"\033[35m"};
    static constexpr auto CYAN {"\033[36m"};
    static constexpr auto WHITE {"\033[37m"};
    static constexpr auto GRAY {"\033[90m"};

    static constexpr auto LIGHT_BLACK {"\033[90m"};
    static constexpr auto LIGHT_RED {"\033[91m"};
    static constexpr auto LIGHT_GREEN {"\033[92m"};
    static constexpr auto LIGHT_YELLOW {"\033[93m"};
    static constexpr auto LIGHT_BLUE {"\033[94m"};
    static constexpr auto LIGHT_MAGENTA {"\033[95m"};
    static constexpr auto LIGHT_CYAN {"\033[96m"};
    static constexpr auto LIGHT_WHITE {"\033[97m"};
    static constexpr auto LIGHT_GRAY {"\033[97m"};

    static constexpr auto BOLD {"\033[1m"};
    static constexpr auto UNDERLINE {"\033[4m"};
    static constexpr auto INVERT {"\033[7m"};

    static constexpr auto BG_BLACK {"\033[40m"};
    static constexpr auto BG_RED {"\033[41m"};
    static constexpr auto BG_GREEN {"\033[42m"};
    static constexpr auto BG_YELLOW {"\033[43m"};
    static constexpr auto BG_BLUE {"\033[44m"};
    static constexpr auto BG_MAGENTA {"\033[45m"};
    static constexpr auto BG_CYAN {"\033[46m"};
    static constexpr auto BG_WHITE {"\033[47m"};
    static constexpr auto BG_GRAY {"\033[100m"};

    static constexpr auto LIGHT_BG_BLACK {"\033[100m"};
    static constexpr auto LIGHT_BG_RED {"\033[101m"};
    static constexpr auto LIGHT_BG_GREEN {"\033[102m"};
    static constexpr auto LIGHT_BG_YELLOW {"\033[103m"};
    static constexpr auto LIGHT_BG_BLUE {"\033[104m"};
    static constexpr auto LIGHT_BG_MAGENTA {"\033[105m"};
    static constexpr auto LIGHT_BG_CYAN {"\033[106m"};
    static constexpr auto LIGHT_BG_WHITE {"\033[107m"};
    static constexpr auto LIGHT_BG_GRAY {"\033[107m"};

    template <typename... Args>
    static void
    log(Args&&... args) {
        log_message("[INFO]: ", WHITE, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void
    log_header(Args&&... args) {
        std::ostringstream oss;
        (oss << ... << args);

        constexpr size_t total_width = 50; // Arbitrarily selected.
        std::string const message = oss.str();

        size_t const padding = (total_width > message.length())
            ? (total_width - message.length()) / 2
            : 0;

        std::string padded_message =
            std::string(padding, ' ') + message + std::string(padding, ' ');

        if (padded_message.length() < total_width) {
            padded_message += ' ';
        }

        std::cout << "########################################################"
                  << EOL;
        std::cout << "## " << padded_message << " ##" << EOL;
        std::cout << "########################################################"
                  << EOL;
    }

    template <typename... Args>
    static void
    log_warn(Args&&... args) {
        log_message("[WARNING]: ", YELLOW, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void
    log_error(Args&&... args) {
        log_message("[ERROR]: ", RED, std::forward<Args>(args)...);
    }

    // Cursor.
    static void
    move_cursor(int x, int y) noexcept {
        std::cout << "\033[" << x << ";" << y << "H";
    }

    static void
    move_cursor_up(int n) noexcept {
        std::cout << "\033[" << n << "A";
    }

    static void
    move_cursor_down(int n) noexcept {
        std::cout << "\033[" << n << "B";
    }

    static void
    move_cursor_right(int n) noexcept {
        std::cout << "\033[" << n << "C";
    }

    static void
    move_cursor_left(int n) noexcept {
        std::cout << "\033[" << n << "D";
    }

    static void
    move_cursor_to_column(int n) noexcept {
        std::cout << "\033[" << n << "G";
    }

    static void
    move_cursor_line_start() noexcept {
        std::cout << "\r";
    }

    static void
    save_cursor_position() noexcept {
        std::cout << "\033[s";
    }

    static void
    load_cursor_position() noexcept {
        std::cout << "\033[u";
    }

    static void
    show_cursor() noexcept {
        std::cout << "\033[?25h";
    }

    static void
    hide_cursor() noexcept {
        std::cout << "\033[?25l";
    }

    // Clearing.
    static void
    clear_line() noexcept {
        std::cout << "\033[K";
    }

    static void
    clear_screen() noexcept {
        std::cout << "\033[2J";
    }

    static void
    clear_from_cursor() noexcept {
        std::cout << "\033[0K";
    }

    static void
    clear_from_start_to_cursor() noexcept {
        std::cout << "\033[1K";
    }

private:
    template <typename... Args>
    static void
    log_message(const char* prefix, const char* color, Args&&... args) {
        std::ostringstream oss;
        (oss << ... << args); // C++17 fold expression to concatenate args.

        const std::string message = oss.str();

#ifdef DEBUG
        if (USE_COLOR) {
            std::cout << color << prefix << message << CLEAR << EOL;
        } else {
            std::cout << prefix << message << EOL;
        }
#else
        if (!std::filesystem::exists("../logs/")) {
            std::filesystem::create_directories("../logs/");
        }

        std::ofstream logFile("../logs/app.log", std::ios_base::app);

        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file." << std::endl;
        } else {
            logFile << prefix << message << EOL;
        }
#endif
    }
};

} // namespace vecs