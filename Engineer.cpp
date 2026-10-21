#include "Engineer.h"
#include "World.h"
#include <algorithm>
#include <ostream>

namespace {
EntitySettings defaults() {
    EntitySettings result;
    result.name = "Engineer";
    return result;
}
}

Engineer::Engineer() : Engineer(defaults()) {}
Engineer::Engineer(const EntitySettings& settings)
    : Colonist(settings), resources_(settings) {}

// The base deep-copies history; role resources have independent value state.
Engineer::Engineer(const Engineer& other) = default;
Engineer& Engineer::operator=(const Engineer& other) = default;
Engineer::~Engineer() = default;

EntitySettings Engineer::settings() const {
    auto result = colonistSettings();
    resources_.fillSettings(result);
    return result;
}

void Engineer::configure(const EntitySettings& settings) {
    settings.validate();
    applyColonistSettings(settings);
    resources_.configure(settings);
}

void Engineer::printDetails(std::ostream& out) const {
    printColonist(out);
    out << "  engineering skill: " << resources_.skill()
        << " | spare parts: " << resources_.supplies()
        << " | repair strength: " << resources_.strength() << '\n';
}

std::unique_ptr<Entity> Engineer::clone() const {
    return std::make_unique<Engineer>(*this);
}

bool Engineer::requestRepair(World& world, int targetId, std::ostream& out) {
    if (!active()) { out << "Actor is inactive.\n"; return false; }
    if (!world.validTarget(EventKind::RepairRequest, targetId, out)) return false;
    const Entity* target = world.find(targetId);
    if (target->health() == target->maxHealth()) {
        out << "Target is already at full health/integrity.\n";
        return false;
    }
    if (resources_.supplies() == 0) { out << "No spare parts available.\n"; return false; }
    if (stamina() < 10) { out << "Not enough stamina (need 10).\n"; return false; }
    Event event{EventKind::RepairRequest, id(), targetId,
                resources_.strength(), "Engineer support"};
    if (!world.enqueueEvent(event, out)) return false;
    // Pay at acceptance, not at processing; a later-invalid target receives no
    // effect and the report explains the discarded request.
    spendStamina(10);
    resources_.consume(1);
    addLog("Queued repair for #" + std::to_string(targetId) + ".");
    out << "  #" << id() << " queued repair for #" << targetId << ".\n";
    return true;
}

void Engineer::takeTurn(World& world, std::ostream& out) {
    if (!active()) return;
    if (world.habitatIntegrity() == 100 || !spendStamina(12)) { rest(out); return; }
    const int gain = world.improveHabitat(3 + resources_.skill() / 2);
    addLog("Improved habitat integrity by " + std::to_string(gain) + ".");
    out << "  #" << id() << " strengthens the habitat (+" << gain << ").\n";
}
