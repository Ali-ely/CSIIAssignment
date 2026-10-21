#ifndef SIMWORLD_ENTITY_FACTORY_H
#define SIMWORLD_ENTITY_FACTORY_H

#include "Entity.h"
#include <memory>

// Centralizes creation-time kind selection. Ordinary turns never use this
// switch; they dispatch through Entity::takeTurn instead.
class EntityFactory {
public:
    static std::unique_ptr<Entity> createDefault(EntityKind kind);
    static std::unique_ptr<Entity> create(EntityKind kind, const EntitySettings& settings);
};

#endif
