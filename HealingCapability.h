#ifndef SIMWORLD_HEALING_CAPABILITY_H
#define SIMWORLD_HEALING_CAPABILITY_H

#include <iosfwd>
class World;

// Optional treatment contract: a selected Entity may or may not implement it.
// The command controller discovers that fact with one checked dynamic_cast.
class HealingCapability {
public:
    virtual ~HealingCapability() = default;
    virtual bool requestHealing(World& world, int targetId, std::ostream& out) = 0;
};

#endif
