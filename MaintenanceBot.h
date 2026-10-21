#ifndef SIMWORLD_MAINTENANCEBOT_H
#define SIMWORLD_MAINTENANCEBOT_H

#include "Robot.h"
#include "RepairCapability.h"
#include "RoleResources.h"

// A concrete robot profession. Its own role resources complement
// the state inherited from Robot; the optional interface is shared across
// branches so capability discovery does not depend on the concrete type.
class MaintenanceBot final : public Robot, public RepairCapability {
    RoleResources resources_;
public:
    MaintenanceBot();
    explicit MaintenanceBot(const EntitySettings& settings);
    MaintenanceBot(const MaintenanceBot& other);
    MaintenanceBot& operator=(const MaintenanceBot& other);
    ~MaintenanceBot() override;
    const char* kindName() const override { return "MaintenanceBot"; }
    EntitySettings settings() const override;
    void configure(const EntitySettings& settings) override;
    void takeTurn(World& world, std::ostream& out) override;
    void printDetails(std::ostream& out) const override;
    std::unique_ptr<Entity> clone() const override;
    bool requestRepair(World& world, int targetId, std::ostream& out) override;
};

#endif
