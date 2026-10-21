#ifndef SIMWORLD_ENGINEER_H
#define SIMWORLD_ENGINEER_H

#include "Colonist.h"
#include "RepairCapability.h"
#include "RoleResources.h"

// A concrete colonist profession. Its own role resources complement
// the state inherited from Colonist; the optional interface is shared across
// branches so capability discovery does not depend on the concrete type.
class Engineer final : public Colonist, public RepairCapability {
    RoleResources resources_;
public:
    Engineer();
    explicit Engineer(const EntitySettings& settings);
    Engineer(const Engineer& other);
    Engineer& operator=(const Engineer& other);
    ~Engineer() override;
    const char* kindName() const override { return "Engineer"; }
    EntitySettings settings() const override;
    void configure(const EntitySettings& settings) override;
    void takeTurn(World& world, std::ostream& out) override;
    void printDetails(std::ostream& out) const override;
    std::unique_ptr<Entity> clone() const override;
    bool requestRepair(World& world, int targetId, std::ostream& out) override;
};

#endif
