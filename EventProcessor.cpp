#include "EventProcessor.h"
#include "World.h"
#include <ostream>

void EventProcessor::apply(World& world, const Event& event, std::ostream& out) {
    out << "  Event: " << eventName(event.kind) << " -> #" << event.targetId << ". ";
    if (!world.validTarget(event.kind, event.targetId, out)) {
        out << "  Request discarded.\n";
        return;
    }
    Entity* target = world.find(event.targetId);
    int actual = 0;
    switch (event.kind) {
    case EventKind::RepairRequest:
    case EventKind::MedicalRequest:
        actual = target->restoreHealth(event.amount);
        out << "Restored " << actual << " health/integrity.\n";
        break;
    case EventKind::SupplyDelivery:
        actual = target->receiveMedicalSupplies(event.amount);
        out << "Delivered " << actual << " medical supplies.\n";
        break;
    }
    // Effective assistance strengthens the colony; rejected or capped-to-zero
    // events earn nothing. A collapsed habitat cannot be revived by a bonus.
    if (actual > 0) {
        const int gain = world.improveHabitat(1);
        out << "  Colony stability +" << gain << " from effective assistance ("
            << world.stability() << "/100).\n";
    }
}
