#include "Medic.h"
#include "World.h"
#include <algorithm>
#include <ostream>

namespace {
EntitySettings defaults() {
    EntitySettings result;
    result.name = "Medic";
    return result;
}
}

Medic::Medic() : Medic(defaults()) {}
Medic::Medic(const EntitySettings& settings)
    : Colonist(settings), resources_(settings) {}

// The base deep-copies history; role resources have independent value state.
Medic::Medic(const Medic& other) = default;
Medic& Medic::operator=(const Medic& other) = default;
Medic::~Medic() = default;

EntitySettings Medic::settings() const {
    auto result = colonistSettings();
    resources_.fillSettings(result);
    return result;
}

void Medic::configure(const EntitySettings& settings) {
    settings.validate();
    applyColonistSettings(settings);
    resources_.configure(settings);
}

void Medic::printDetails(std::ostream& out) const {
    printColonist(out);
    out << "  medical skill: " << resources_.skill()
        << " | medical supplies: " << resources_.supplies()
        << " | treatment strength: " << resources_.strength() << '\n';
}

std::unique_ptr<Entity> Medic::clone() const {
    return std::make_unique<Medic>(*this);
}

bool Medic::requestHealing(World& world, int targetId, std::ostream& out) {
    if (!active()) { out << "Actor is inactive.\n"; return false; }
    if (!world.validTarget(EventKind::MedicalRequest, targetId, out)) return false;
    const Entity* target = world.find(targetId);
    if (target->health() == target->maxHealth()) {
        out << "Target is already at full health/integrity.\n";
        return false;
    }
    if (resources_.supplies() == 0) { out << "No medical supplies available.\n"; return false; }
    if (stamina() < 10) { out << "Not enough stamina (need 10).\n"; return false; }
    Event event{EventKind::MedicalRequest, id(), targetId,
                resources_.strength(), "Medic support"};
    if (!world.enqueueEvent(event, out)) return false;
    // Pay at acceptance, not at processing; a later-invalid target receives no
    // effect and the report explains the discarded request.
    spendStamina(10);
    resources_.consume(1);
    addLog("Queued treatment for #" + std::to_string(targetId) + ".");
    out << "  #" << id() << " queued treatment for #" << targetId << ".\n";
    return true;
}

int Medic::receiveMedicalSupplies(int amount) {
    const int actual = resources_.add(amount);
    addLog("Received " + std::to_string(actual) + " medical supplies.");
    return actual;
}

void Medic::takeTurn(World& world, std::ostream& out) {
    if (!active()) return;
    const int target = world.mostInjured(true);
    if (stamina() < 10 || resources_.supplies() == 0 || target == 0) {
        out << "  Medic #" << id() << " has no available treatment task.\n";
        rest(out);
        return;
    }
    requestHealing(world, target, out);
}
