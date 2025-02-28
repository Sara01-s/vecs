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

    auto archetype = component_storage.create_archetype(Position{0.0f, 0.0f}, Velocity{1.0f, 1.0f});
    
    vecs::debug_t::log("Archetype Mask: ", archetype.mask);

    return 0;
}

