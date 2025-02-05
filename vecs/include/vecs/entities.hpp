#pragma once

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>

namespace vecs {

    using EntityId = size_t;

    struct Component {
        virtual ~Component() = default;
    }

    class World {
    private:
        std::unordered_map<std::type_index, std::unordered_map<EntityId, std::shared_ptr<Component>>> _components;

    public:
        template <typename T>
        void 
        add_component(EntityId entityId, T component) {
            _components[typeid(component)] = std::make_shared<T>(component);
        }

    };

    
} // namespace vecs
