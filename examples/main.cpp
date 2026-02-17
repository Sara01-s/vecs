// libs
#include <vecs/vecs.hpp>

using log_t = vecs::log_t;

struct Position {
    float x, y, z;

    friend std::ostream&
    operator<<(std::ostream& os, Position const& pos) {
        return os << "Position { x: " << pos.x << ", y: " << pos.y
                  << ", z: " << pos.z << " }";
    }
};

struct Velocity {
    float dx, dy, dz;

    friend std::ostream&
    operator<<(std::ostream& os, Velocity const& vel) {
        return os << "Velocity { dx: " << vel.dx << ", dy: " << vel.dy
                  << ", dz: " << vel.dz << " }";
    }
};

struct Health {
    int value;

    constexpr void
    reset() noexcept {
        value = 100;
    };

    friend std::ostream&
    operator<<(std::ostream& os, Health const& health) {
        return os << "Health { value: " << health.value << " }";
    }
};

void system2(vecs::world_t& world) {
    auto query = world.query<Position, Velocity const, Health>();

    for (auto [pos, vel, health] : query) {
        pos.x += vel.dx;
        pos.y += vel.dy;
        pos.z += vel.dz;
        health.value += 1;

        log_t::log("Entity updated: ", pos);
    }

    log_t::log("-------------------------------------------------");
    std::cin.get();
}

// Schedule label.
struct Update {};

int
main() {
    vecs::world_t world {};
    world.register_components<Position, Velocity, Health>();
    log_t::log("-------------------------------------------------");

    auto position1 {Position {1.0f, 1.0f, 0.0f}};
    auto velocity1 {Velocity {0.0f, 2.0f, 0.0f}};
    auto health1 {Health {50}};

    auto position2 {Position {2.0f, 2.0f, 0.0f}};
    auto velocity2 {Velocity {1.0f, 1.0f, 0.0f}};
    auto health2 {Health {75}};

    auto velocity3 {Velocity {0.5f, 0.5f, 0.0f}};

    world.spawn_entity(position1, velocity1, health1);
    world.spawn_entity(position2, velocity2, health2);
    auto const entity_id = world.spawn_entity(velocity1, health1);

    world.despawn_entity(entity_id);
    world.spawn_entity(velocity3);

    log_t::log("-------------------------------------------------");
    log_t::save_cursor_position();

    world.add_system(Update {}, system2);

    while (true) {
        world.run(Update {});
    }

    return 0;
}
