#pragma once

// lib
#include <catch2/catch_all.hpp>
#include <vecs/vecs.hpp>

struct Position {
    vecs::f32 x, y;
};

struct Velocity {
    vecs::f32 dx, dy;
};

struct Health {
    vecs::u32 value;
};

TEST_CASE("Registered components counts updates correctly", "[ComponentStorage]") {
    vecs::component_storage_t<Position, Velocity, Health> component_storage{};
    vecs::usize const count = component_storage.get_registered_component_count();

    REQUIRE(count == 3);
}