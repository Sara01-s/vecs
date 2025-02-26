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
    std::printf("Registered components count: %zu\n", component_storage.get_registered_component_count());

    //component_storage.spawn_entity(Position{0,0});

    return 0;
}

