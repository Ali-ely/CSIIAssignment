#ifndef SIMWORLD_REPAIR_CAPABILITY_H
#define SIMWORLD_REPAIR_CAPABILITY_H

#include <iosfwd>
class World;

// Optional contract shared across the human/robot branches. It has no Entity
// base or state, avoiding a diamond and keeping repair off unrelated entities.
class RepairCapability {
public:
    virtual ~RepairCapability() = default;
    virtual bool requestRepair(World& world, int targetId, std::ostream& out) = 0;
};

#endif
