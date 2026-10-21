#include "MedicalDrone.h"
#include "World.h"
#include <algorithm>
#include <ostream>

namespace {
EntitySettings defaults() {
    EntitySettings result;
    result.name = "Medical Drone";
    return result;
}
}

MedicalDrone::MedicalDrone() : MedicalDrone(defaults()) {}
MedicalDrone::MedicalDrone(const EntitySettings& settings)
    : Robot(settings), resources_(settings) {}

EntitySettings MedicalDrone::settings() const {
    auto result = robotSettings();
    resources_.fillSettings(result);
    return result;
}

void MedicalDrone::configure(const EntitySettings& settings) {
    settings.validate();
    applyRobotSettings(settings);
    resources_.configure(settings);
}

void MedicalDrone::printDetails(std::ostream& out) const {
    printRobot(out);
    out << "  delivery efficiency: " << resources_.skill()
        << " | medicine doses: " << resources_.supplies()
        << " | treatment strength: " << resources_.strength() << '\n';
}

std::unique_ptr<Entity> MedicalDrone::clone() const {
    return std::make_unique<MedicalDrone>(*this);
}

bool MedicalDrone::requestHealing(World& world, int targetId, std::ostream& out) {
    if (!active()) { out << "Actor is inactive.\n"; return false; }
    if (!world.validTarget(EventKind::MedicalRequest, targetId, out)) return false;
    const Entity* target = world.find(targetId);
    if (target->health() == target->maxHealth()) {
        out << "Target is already at full health/integrity.\n";
        return false;
    }
    if (resources_.supplies() == 0) { out << "No medicine doses available.\n"; return false; }
    if (battery() < 10) { out << "Not enough battery (need 10).\n"; return false; }
    Event event{EventKind::MedicalRequest, id(), targetId,
                resources_.strength(), "MedicalDrone support"};
    if (!world.enqueueEvent(event, out)) return false;
    // Pay at acceptance, not at processing; a later-invalid target receives no
    // effect and the report explains the discarded request.
    spendBattery(10);
    resources_.consume(1);
    addLog("Queued treatment for #" + std::to_string(targetId) + ".");
    out << "  #" << id() << " queued treatment for #" << targetId << ".\n";
    return true;
}

void MedicalDrone::takeTurn(World& world, std::ostream& out) {
    if (!active()) return;
    const int target = world.medicNeedingSupplies();
    if (battery() < 8 || resources_.supplies() == 0 || target == 0) {
        out << "  MedicalDrone #" << id() << " has no available delivery task.\n";
        charge(out);
        return;
    }
    const int amount = std::min({1 + resources_.skill() / 3, resources_.supplies(),
                                10000 - world.find(target)->medicalSupplyCount()});
    Event event{EventKind::SupplyDelivery, id(), target, amount, "Drone medicine delivery"};
    if (world.enqueueEvent(event, out)) {
        spendBattery(8);
        resources_.consume(amount);
        addLog("Dispatched " + std::to_string(amount) + " doses to #" + std::to_string(target) + ".");
        out << "  #" << id() << " dispatches " << amount << " doses to medic #" << target << ".\n";
    }
}
