// std
#include <iostream>

// libs
#include <vecs/vecs.hpp>

struct Position {
    vecs::f32 x, y;
};

struct Velocity {
    vecs::f32 dx, dy;
};

struct Health {
    vecs::u32 value;
    void reset() noexcept { value = 100; };
};

int main() {
    vecs::component_storage_t<Position, Velocity, Health> component_storage{};
    vecs::debug_t::log("Registered components count: ", 
                        component_storage.get_registered_component_count());

    vecs::debug_t::log("-------------------------------------------------");
    
    auto position { Position { 0.0f, 0.0f } };
    auto velocity { Velocity { 0.0f, 0.0f } };
    auto health { Health { 100 } };

    auto const e_id  = component_storage.spawn_entity(position, velocity, health);
    auto const e_id2 = component_storage.spawn_entity(position, health);
    auto const e_id3 = component_storage.spawn_entity(velocity, health);
    auto const e_id4 = component_storage.spawn_entity(velocity);
    auto const e_id5 = component_storage.spawn_entity(health);

    return 0;
}

