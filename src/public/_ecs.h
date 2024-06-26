#pragma once
#include "_common.h"
#include "_ecs.h"

#include <bitset>
#include <cassert>
#include <memory>
#include <numeric>
#include <ranges>
#include <set>
#include <span>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>


namespace ecs {


struct Registry;
struct System;
using Entity = usize;


namespace detail {
    constexpr usize NUM_EXPECTED_ENTITIES = 10;
    constexpr usize NUM_EXPECTED_COMPONENTS = 10;
    constexpr usize MAX_NUM_COMPONENTS = 32;
    using Component_Type_Id = usize;
    using Components_Bitset = std::bitset<MAX_NUM_COMPONENTS>;
    using Component_Map = std::array<usize, MAX_NUM_COMPONENTS>; // component indices


    template <typename T>
    concept C_System = std::is_base_of_v<System, T>;


    template <typename T>
    concept C_Component = !C_System<T>;


    // Get a unique id for the given component type.
    extern usize NUM_COMPONENT_TYPES_MUT;
    template <C_Component>
    Component_Type_Id get_component_type_id() {
        static const Component_Type_Id component_type_id = detail::NUM_COMPONENT_TYPES_MUT++;
        return component_type_id;
    }


    consteval Component_Map create_empty_component_map() {
        Component_Map component_map;
        std::fill(component_map.begin(), component_map.end(), -1);
        return component_map;
    }


    template <C_Component Component_Type>
    Components_Bitset get_component_bitset() {
        return Components_Bitset{detail::get_component_type_id<Component_Type>()};
    }


    template <C_Component Component_Type>
    void add_component_to_bitset(Components_Bitset& bitset) {
        bitset.set(detail::get_component_type_id<Component_Type>());
    }


    struct Component_Pool {
        std::vector<u8> data;
    };


    template <C_Component T_Component>
    struct Typed_Component_Pool {
        Component_Pool& pool;
        explicit Typed_Component_Pool(Component_Pool& in_pool): pool(in_pool) {}

        usize component_count() const {
            return pool.data.size() / sizeof(T_Component);
        }

        std::span<T_Component> view_components() {
            return std::span<T_Component>{reinterpret_cast<T_Component*>(&pool.data[0]), component_count()};
        }

        usize add_component(const T_Component& new_component) {
            pool.data.resize(pool.data.size() + sizeof(T_Component));
            const usize component_idx = component_count() - 1;
            view_components()[component_idx] = new_component;
            return component_idx;
        }

        usize add_component(const T_Component&& new_component) {
            pool.data.resize(pool.data.size() + sizeof(T_Component));
            const usize component_idx = component_count() - 1;
            view_components()[component_idx] = new_component;
            return component_idx;
        }
    };
}


struct System {
    System() = default;
    virtual ~System() = default;

    template <typename T>
    void require_component() {
        detail::add_component_to_bitset<T>(components_bitset);
    }

    const std::vector<Entity>& get_entities() { return entities; }

private:
    friend Registry;

    detail::Components_Bitset components_bitset{};
    std::vector<Entity> entities;
};


struct Registry {
    Registry() {
        component_pools.resize(detail::MAX_NUM_COMPONENTS);
        entity_component_bitset.reserve(detail::NUM_EXPECTED_ENTITIES);
        entity_component_ids.reserve(detail::NUM_EXPECTED_ENTITIES);
    }


    usize num_entities() const {
        return entity_component_bitset.size();
    }


    // Check if entity has component
    template<detail::C_Component T_Component>
    bool has(const Entity entity) const {
        return entity_component_bitset[entity].test(detail::get_component_type_id<T_Component>());
    }


    // Check if registry has system
    template <detail::C_System T_System>
    bool has() const {
        return systems.contains(std::type_index(typeid(T_System)));
    }


    // Add entity entry
    Entity add() {
        entity_component_bitset.emplace_back() = detail::Components_Bitset{};
        entity_component_ids.emplace_back() = detail::create_empty_component_map();
        return Entity{num_entities() - 1};
    }


    // Add system
    template <detail::C_System System_Type,
              typename ...System_Constructor_Args
    >
    void add(System_Constructor_Args&&... args) {
        assert(!has<System_Type>());
        std::unique_ptr<System> system = std::make_unique<System_Type>(std::forward<System_Constructor_Args>(args)...);
        std::type_index system_id = std::type_index(typeid(System_Type));
        systems.insert(std::make_pair(system_id, std::move(system)));
        newly_added_systems.insert(system_id);
    }


    // Add component to entity
    template <detail::C_Component T_Component>
    void add(const Entity entity, const T_Component& component_data) {
        const detail::Component_Type_Id component_type_id = detail::get_component_type_id<T_Component>();
        entity_component_bitset[entity].set(component_type_id, true);
        entity_component_ids[entity][component_type_id] = use_component_pool<T_Component>().add_component(component_data);
    }


    // Add component to entity
    template <detail::C_Component T_Component>
    void add(const Entity entity, const T_Component&& component_data) {
        const detail::Component_Type_Id component_type_id = detail::get_component_type_id<T_Component>();
        entity_component_bitset[entity].set(component_type_id, true);
        entity_component_ids[entity][component_type_id] = use_component_pool<T_Component>().add_component(component_data);
    }


    // Get component from entity
    template<detail::C_Component T_Component>
    T_Component& get(const Entity entity) {
        assert(has<T_Component>(entity));
        const detail::Component_Type_Id component_type_id = detail::get_component_type_id<T_Component>();
        const usize component_idx = entity_component_ids[entity][component_type_id];
        return use_component_pool<T_Component>().view_components()[component_idx];
    }


    // Get system from registry
    template <detail::C_System T_System>
    T_System& get() {
        assert(has<T_System>());
        const auto system_it = systems.find(std::type_index(typeid(T_System)));
        std::unique_ptr<System>& system = system_it->second;
        return *static_cast<T_System*>(system.get());
    }


    // Get system from registry
    template <detail::C_System T_System>
    const T_System& get() const {
        assert(has<T_System>());
        const auto system_it = systems.find(std::type_index(typeid(T_System)));
        const std::unique_ptr<System>& system = system_it->second;
        return *static_cast<const T_System*>(system.get());
    }


    void refresh_systems_entity_sets() {
        // Register new entities with systems
        for (Entity entity = systems_latest_known_entity; entity < num_entities(); entity++) {
            const detail::Components_Bitset entity_components = entity_component_bitset[entity];

            for (const auto& system : std::views::values(systems)) {
                // if system requires component that newly added entity has, add the entity to the system
                if (system->components_bitset.test(entity_components.all())) {
                    system->entities.emplace_back() = entity;
                }
            }
        }
        systems_latest_known_entity = num_entities();


        // Initialize newly added systems
        for (const std::type_index& system_type_idx : newly_added_systems) {
            const std::unique_ptr<System>& system = systems[system_type_idx];
            get_all_entities_with(system->components_bitset, system->entities);
        }
        newly_added_systems.clear();
    }

private:
    std::vector<detail::Components_Bitset> entity_component_bitset; // component set per entity
    std::vector<detail::Component_Map> entity_component_ids;        // component idx per entity
    std::vector<detail::Component_Pool> component_pools;

    std::unordered_map<std::type_index, std::unique_ptr<System>> systems;
    std::set<std::type_index> newly_added_systems;
    usize systems_latest_known_entity = 0;


    template <detail::C_Component T>
    detail::Typed_Component_Pool<T> use_component_pool() {
        const detail::Component_Type_Id component_type_id = detail::get_component_type_id<T>();
        return detail::Typed_Component_Pool<T>(component_pools[component_type_id]);
    }


    void get_all_entities_with(const detail::Components_Bitset requested_components, std::vector<Entity>& entities) const {
        entities.resize(num_entities());
        std::iota(entities.begin(), entities.end(), 0);
        std::erase_if(entities, [&](const Entity entity) {
            const bool has_components = (entity_component_bitset[entity] & requested_components) == requested_components;
            return !has_components;
        });
    }
};


} // namespace ecs


using namespace ecs;
