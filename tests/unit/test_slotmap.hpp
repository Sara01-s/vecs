#pragma once

// std
#include <unordered_map>

// lib
#include <catch2/catch_all.hpp>
#include <vecs/data_structures/slotmap.hpp>

TEST_CASE("slot_map_t initialization.", "[slot_map_t]") {
    constexpr vecs::slotmap_t<int, 5> slot_map;
    REQUIRE(slot_map.size() == 0);
}

TEST_CASE("slot_map_t insertion and retrieval.", "[slot_map_t]") {
    vecs::slotmap_t<int, 5> slot_map;
    auto const key1 = slot_map.push_back(10);
    auto const key2 = slot_map.push_back(20);
    
    REQUIRE(slot_map.size() == 2);
    REQUIRE(slot_map.is_key_valid(key1));
    REQUIRE(slot_map.is_key_valid(key2));
}

TEST_CASE("slot_map_t erase operation.", "[slot_map_t]") {
    vecs::slotmap_t<int, 5> slot_map;
    auto const key1 = slot_map.push_back(10);
    auto const key2 = slot_map.push_back(20);
    
    REQUIRE(slot_map.erase(key1));
    REQUIRE_FALSE(slot_map.is_key_valid(key1));
    REQUIRE(slot_map.size() == 1);
}

TEST_CASE("slot_map_t capacity limit.", "[slot_map_t]") {
    vecs::slotmap_t<int, 5> slot_map;
    
    for (int i = 0; i < 5; ++i) {
        auto _ = slot_map.push_back(i);
    }
    REQUIRE_THROWS_AS(slot_map.push_back(100), std::runtime_error);
}

TEST_CASE("slot_map_t reuse of freed slots.", "[slot_map_t]") {
    vecs::slotmap_t<int, 5> slot_map;
    auto const key1 = slot_map.push_back(10);
    auto const key2 = slot_map.push_back(20);
    slot_map.erase(key1);
    auto const key3 = slot_map.push_back(30);
    
    REQUIRE(slot_map.is_key_valid(key2));
    REQUIRE(slot_map.is_key_valid(key3));
}

TEST_CASE("Insertion and deletion benchmarks.", "[!benchmark]") {
    constexpr int num_elements { 10000 };

    BENCHMARK("slot_map_t Insertion and deletion.") {
        vecs::slotmap_t<int, num_elements> slot_map;
        for (int i = 0; i < num_elements; ++i) {
            auto key = slot_map.push_back(i);
            slot_map.erase(key);
        }
    };

    BENCHMARK("UnorderedMap Insertion and deletion.") {
        std::unordered_map<int, int> unordered_map;
        for (int i = 0; i < num_elements; ++i) {
            unordered_map[i] = i;
            unordered_map.erase(i);
        }
    };

    BENCHMARK("slot_map_t Iteration") {
        vecs::slotmap_t<int, num_elements> slot_map;

        for (int i = 0; i < num_elements; ++i) {
            auto const _ = slot_map.push_back(i);
        }

        int sum = 0;
        for (auto const& item : slot_map) {
            sum += item;
        }

        return sum;
    };

    BENCHMARK("UnorderedMap Iteration") {
        std::unordered_map<int, int> unordered_map;

        for (int i = 0; i < num_elements; ++i) {
            unordered_map[i] = i;
        }

        int sum = 0;
        for (auto const& [key, value] : unordered_map) {
            sum += value;
        }

        return sum;
    };
}
