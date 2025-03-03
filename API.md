# API Goals

- **Entity & Components**
    ```cpp
    #include <vecs/vecs.hpp>

    struct Position {
        float x, y;
    };

    struct Velocity {
        float x, y;
    }

    struct Player {}; // Marker component.

    enum GameState { Update, FixedUpdate, OnStart };

    void move_system(
        vecs::Query<Position, Velocity const, With<Player>> query
    ) {
        query.for_each([](Position& position, Velocity const& velocity) {
            position.x += velocity.dx;
            position.y += velocity.dy;
        });
    }>

    int main() {
        vecs::world_t world{};

        world.spawn_entity(Player, Position {}, Velocity { 1, 1 });
        world.add_system(GameState::Update, move_system);
        world.run();
    }
  ```