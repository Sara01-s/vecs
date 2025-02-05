#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace vecs {

// FIXME - Move this somewhere else?
#define DEBUG // comment to deactivate.
constexpr bool USE_COLOR = true;
constexpr char EOL = '\n';

template <size_t CharCount = 16>
struct DebugTag {
#ifdef DEBUG
    char tag[CharCount] {};
    constexpr DebugTag(char const (&str)[CharCount]) { 
        std::copy_n(str, CharCount, tag);
    }
#else
    constexpr DebugTag(char const (&)[CharCount]) {}
#endif
};

class Debug {

public:
    // This class is not meant to be instantiated. (aka static class).
    Debug() = delete;
    ~Debug() = delete;
    Debug(const Debug&) = delete;

    static constexpr const char* ANSI_RESET  = "\033[0m";
    static constexpr const char* ANSI_WHITE  = "\033[37m";
    static constexpr const char* ANSI_YELLOW = "\033[33m";
    static constexpr const char* ANSI_RED    = "\033[31m";

    template <typename... Args>
    static void 
    log(Args&&... args) {
        log_message("[INFO]: ", ANSI_WHITE, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void 
    log_header(Args&&... args) {
        std::ostringstream oss;
        (oss << ... << args);
        
        const size_t total_width = 50;  // Arbitrarily selected.
        std::string message = oss.str();
        
        size_t padding = (total_width > message.length())
                        ? (total_width - message.length()) / 2
                        : 0;

        std::string padded_message = std::string(padding, ' ') + message + std::string(padding, ' ');
        
        if (padded_message.length() < total_width) {
            padded_message += ' ';
        }
        
        std::cout << "########################################################" << EOL;
        std::cout << "## "              << padded_message << " ##"              << EOL;
        std::cout << "########################################################" << EOL;
    }

    template <typename... Args>
    static void 
    log_warn(Args&&... args) {
        log_message("[WARNING]: ", ANSI_YELLOW, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void 
    log_error(Args&&... args) {
        log_message("[ERROR]: ", ANSI_RED, std::forward<Args>(args)...);
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
            std::cout << color << prefix << message << ANSI_RESET << EOL;
        } 
        else {
            std::cout << prefix << message << EOL;
        }
#else
        if (!std::filesystem::exists("../logs/")) {
            std::filesystem::create_directories("../logs/");
        }

        std::ofstream logFile("../logs/app.log", std::ios_base::app);

        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file." << std::endl;
        }
        else {
            logFile << prefix << message << EOL;
        }
#endif
    }
};

} // namespace vecs