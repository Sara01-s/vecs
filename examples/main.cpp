// std
#include <iostream>

// libs
#include <vecs/vecs.hpp>

struct Position {
    float x, y, z;
};

struct Velocity {
    float dx, dy, dz;
};

struct Health {
    int value;
    constexpr void reset() noexcept { value = 100; };
};

int main() {
    using log_t = vecs::debug_t;
    vecs::world_t<Position, Velocity, Health> world{};
    log_t::log("-------------------------------------------------");
    
    auto position { Position { 1.0f, 1.0f, 0.0f } };
    auto velocity { Velocity { 0.0f, 2.0f, 0.0f } };
    auto health { Health { 50 } };

    world.spawn_entity(position, velocity, health);
    auto const entity_id = world.spawn_entity(velocity, health);

    world.despawn_entity(entity_id);

    world.spawn_entity(velocity);
    world.spawn_entity(health);

    return 0;
}

