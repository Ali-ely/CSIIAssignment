#include "MaintenanceBot.h"
#include "World.h"
#include <algorithm>
#include <ostream>

namespace {
EntitySettings defaults() {
    EntitySettings result;
    result.name = "Maintenance Bot";
    return result;
}
}

MaintenanceBot::MaintenanceBot() : MaintenanceBot(defaults()) {}
MaintenanceBot::MaintenanceBot(const EntitySettings& settings)
    : Robot(settings), resources_(settings) {}

EntitySettings MaintenanceBot::settings() const {
    auto result = robotSettings();
    resources_.fillSettings(result);
    return result;
}

void MaintenanceBot::configure(const EntitySettings& settings) {
    settings.validate();
    applyRobotSettings(settings);
    resources_.configure(settings);
}

void MaintenanceBot::printDetails(std::ostream& out) const {
    printRobot(out);
    out << "  maintenance efficiency: " << resources_.skill()
        << " | spare parts: " << resources_.supplies()
        << " | repair strength: " << resources_.strength() << '\n';
}

std::unique_ptr<Entity> MaintenanceBot::clone() const {
    return std::make_unique<MaintenanceBot>(*this);
}

bool MaintenanceBot::requestRepair(World& world, int targetId, std::ostream& out) {
    if (!active()) { out << "Actor is inactive.\n"; return false; }
    if (!world.validTarget(EventKind::RepairRequest, targetId, out)) return false;
    const Entity* target = world.find(targetId);
    if (target->health() == target->maxHealth()) {
        out << "Target is already at full health/integrity.\n";
        return false;
    }
    if (resources_.supplies() == 0) { out << "No spare parts available.\n"; return false; }
    if (battery() < 10) { out << "Not enough battery (need 10).\n"; return false; }
    Event event{EventKind::RepairRequest, id(), targetId,
                resources_.strength(), "MaintenanceBot support"};
    if (!world.enqueueEvent(event, out)) return false;
    // Pay at acceptance, not at processing; a later-invalid target receives no
    // effect and the report explains the discarded request.
    spendBattery(10);
    resources_.consume(1);
    addLog("Queued repair for #" + std::to_string(targetId) + ".");
    out << "  #" << id() << " queued repair for #" << targetId << ".\n";
    return true;
}

void MaintenanceBot::takeTurn(World& world, std::ostream& out) {
    if (!active()) return;
    const int target = world.mostInjured(false);
    if (battery() < 10 || resources_.supplies() == 0 || target == 0) {
        out << "  MaintenanceBot #" << id() << " has no available repair task.\n";
        charge(out);
        return;
    }
    requestRepair(world, target, out);
}
