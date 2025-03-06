// libs
#include <vecs/vecs.hpp>

struct Position {
    float x, y, z;
    friend std::ostream& operator<<(std::ostream& os, Position const& pos) {
        return os << "Position { x: " << pos.x << ", y: " << pos.y << ", z: " << pos.z << " }";
    }
};

struct Velocity {
    float dx, dy, dz;
    friend std::ostream& operator<<(std::ostream& os, Velocity const& vel) {
        return os << "Velocity { dx: " << vel.dx << ", dy: " << vel.dy << ", dz: " << vel.dz << " }";
    }
};

struct Health {
    int value;
    constexpr void reset() noexcept { value = 100; };
    friend std::ostream& operator<<(std::ostream& os, Health const& health) {
        return os << "Health { value: " << health.value << " }";
    }
};


int main() {
    using log_t = vecs::debug_t;

    vecs::world_t<Position, Velocity, Health> world{};
    log_t::log("-------------------------------------------------");
    
    auto position1 { Position { 1.0f, 1.0f, 0.0f } };
    auto velocity1 { Velocity { 0.0f, 2.0f, 0.0f } };
    auto health1 { Health { 50 } };

    auto position2 { Position { 2.0f, 2.0f, 0.0f } };
    auto velocity2 { Velocity { 1.0f, 1.0f, 0.0f } };
    auto health2 { Health { 75 } };

    auto position3 { Position { 3.0f, 3.0f, 0.0f } };
    auto velocity3 { Velocity { 0.5f, 0.5f, 0.0f } };
    auto health3 { Health { 100 } };

    world.spawn_entity(position1, velocity1, health1);
    world.spawn_entity(position2, velocity2, health2);
    auto const entity_id = world.spawn_entity(velocity1, health1);

    world.despawn_entity(entity_id);
    world.spawn_entity(velocity3);

    log_t::log("-------------------------------------------------");
    world.for_each<Position, Velocity, Health>(
        [](Position const& pos, Velocity const& vel, Health const& health
    ) {
        log_t::log("System matched the following components in entity: ");
        log_t::log("  ╰> ", log_t::YELLOW, pos);
        log_t::log("  ╰> ", log_t::YELLOW, vel);
        log_t::log("  ╰> ", log_t::YELLOW, health);
        log_t::log("");
    });

    return 0;
}

