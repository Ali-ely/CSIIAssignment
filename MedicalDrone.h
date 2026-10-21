#ifndef SIMWORLD_MEDICALDRONE_H
#define SIMWORLD_MEDICALDRONE_H

#include "Robot.h"
#include "HealingCapability.h"
#include "RoleResources.h"

// A concrete robot profession. Its own role resources complement
// the state inherited from Robot; the optional interface is shared across
// branches so capability discovery does not depend on the concrete type.
class MedicalDrone final : public Robot, public HealingCapability {
    RoleResources resources_;
public:
    MedicalDrone();
    explicit MedicalDrone(const EntitySettings& settings);
    MedicalDrone(const MedicalDrone& other) = default;
    MedicalDrone& operator=(const MedicalDrone& other) = default;
    const char* kindName() const override { return "MedicalDrone"; }
    EntitySettings settings() const override;
    void configure(const EntitySettings& settings) override;
    void takeTurn(World& world, std::ostream& out) override;
    void printDetails(std::ostream& out) const override;
    std::unique_ptr<Entity> clone() const override;
    bool requestHealing(World& world, int targetId, std::ostream& out) override;
};

#endif
