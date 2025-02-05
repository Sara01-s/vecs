// std
#include <vector>
#include <array>
#include <string>
#include <thread>
#include <chrono>

// libs
#include <vecs/vecs.hpp>
#include <vecs/data_structures/slotmap.hpp>
#include <vecs/utils/memory_viewer.hpp>

struct Name {
    char name[8] { "@noname" };
};

void 
clear_console() {
    std::cout << "\033[2J\033[H" << std::flush;
}

int 
main() {
    constexpr size_t item_count = 10;
    vecs::SlotMap<Name, item_count> slotmap;
    vecs::MemoryViewer memory_viewer { &slotmap };
    std::array<vecs::SlotMap<Name, 10>::key_t, 10> keys;

    memory_viewer.print();

    for (int i{}; i < item_count; ++i) {
        Name n{};
        std::snprintf(n.name, sizeof(n.name), "name__%d", i);

        if (i == 5) {
            slotmap.erase(keys[i - 3]);
        }
        else {
            auto key = slotmap.push_back(n);
            keys[i] = key;
        }

        memory_viewer.print();
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        clear_console();
    }
    
    memory_viewer.print();

    std::printf("Final data: ");
    for (auto const& item: slotmap) {
        std::printf("%s, ", item.name);
    }
    std::printf("\n");
    return 0;
}
