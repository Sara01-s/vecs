// lib
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch_all.hpp>

// Test data.
struct Position {
    float x, y, z;
};

struct Velocity {
    float dx, dy, dz;
};

struct Health {
    int value { 100 };
};

struct Player {}; // Marker component.

// Unit tests.
#include "unit/test_slotmap.hpp"
#include "unit/test_component_storage.hpp"

// Integration tests.
#include "integration/test_world.hpp"