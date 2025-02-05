// std
#include <unordered_map>

// lib
#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include <vecs/data_structures/slot_map.hpp>

TEST_CASE("SlotMap initialization.", "[SlotMap]") {
    constexpr vecs::SlotMap<int, 5> slot_map;
    REQUIRE(slot_map.size() == 0);
}

TEST_CASE("SlotMap insertion and retrieval.", "[SlotMap]") {
    vecs::SlotMap<int, 5> slot_map;
    auto const key1 = slot_map.push_back(10);
    auto const key2 = slot_map.push_back(20);
    
    REQUIRE(slot_map.size() == 2);
    REQUIRE(slot_map.is_key_valid(key1));
    REQUIRE(slot_map.is_key_valid(key2));
}

TEST_CASE("SlotMap erase operation.", "[SlotMap]") {
    vecs::SlotMap<int, 5> slot_map;
    auto const key1 = slot_map.push_back(10);
    auto const key2 = slot_map.push_back(20);
    
    REQUIRE(slot_map.erase(key1));
    REQUIRE_FALSE(slot_map.is_key_valid(key1));
    REQUIRE(slot_map.size() == 1);
}

TEST_CASE("SlotMap capacity limit.", "[SlotMap]") {
    vecs::SlotMap<int, 5> slot_map;
    
    for (int i = 0; i < 5; ++i) {
        auto _ = slot_map.push_back(i);
    }
    REQUIRE_THROWS_AS(slot_map.push_back(100), std::runtime_error);
}

TEST_CASE("SlotMap reuse of freed slots.", "[SlotMap]") {
    vecs::SlotMap<int, 5> slot_map;
    auto const key1 = slot_map.push_back(10);
    auto const key2 = slot_map.push_back(20);
    slot_map.erase(key1);
    auto const key3 = slot_map.push_back(30);
    
    REQUIRE(slot_map.is_key_valid(key2));
    REQUIRE(slot_map.is_key_valid(key3));
}

TEST_CASE("Insertion and deletion benchmarks.", "[!benchmark]") {
    constexpr int num_elements = 10000;

    // SlotMap
    BENCHMARK("SlotMap Insertion and deletion.") {
        vecs::SlotMap<int, 5> slot_map;
        for (int i = 0; i < num_elements; ++i) {
            auto key = slot_map.push_back(i);
            slot_map.erase(key);
        }
    };

    // UnorderedMap
    BENCHMARK("UnorderedMap Insertior and deletion.") {
        std::unordered_map<int, int> unordered_map;
        for (int i = 0; i < num_elements; ++i) {
            unordered_map[i] = i;
            unordered_map.erase(i);
        }
    };
}