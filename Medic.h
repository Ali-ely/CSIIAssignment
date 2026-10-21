#ifndef SIMWORLD_MEDIC_H
#define SIMWORLD_MEDIC_H

#include "Colonist.h"
#include "HealingCapability.h"
#include "RoleResources.h"

// A concrete colonist profession. Its own role resources complement
// the state inherited from Colonist; the optional interface is shared across
// branches so capability discovery does not depend on the concrete type.
class Medic final : public Colonist, public HealingCapability {
    RoleResources resources_;
public:
    Medic();
    explicit Medic(const EntitySettings& settings);
    Medic(const Medic& other);
    Medic& operator=(const Medic& other);
    ~Medic() override;
    const char* kindName() const override { return "Medic"; }
    EntitySettings settings() const override;
    void configure(const EntitySettings& settings) override;
    void takeTurn(World& world, std::ostream& out) override;
    void printDetails(std::ostream& out) const override;
    std::unique_ptr<Entity> clone() const override;
    bool requestHealing(World& world, int targetId, std::ostream& out) override;
    bool acceptsMedicalSupplies() const override { return true; }
    int medicalSupplyCount() const override { return resources_.supplies(); }
    int receiveMedicalSupplies(int amount) override;
};

#endif
