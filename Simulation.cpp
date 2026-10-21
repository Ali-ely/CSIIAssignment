#include "Simulation.h"
#include "EntityFactory.h"
#include "HealingCapability.h"
#include "RepairCapability.h"
#include <exception>
#include <ostream>
#include <sstream>
#include <utility>

bool Simulation::transact(const std::function<bool(World&, std::ostream&)>& action,
                          std::ostream& out) {
    try {
        World candidate(world_);
        std::ostringstream report;
        if (!action(candidate, report)) {
            out << report.str();
            return false;
        }
        // Materialize the report before committing: allocation failure must not
        // turn a completed command into an apparently failed operation.
        const std::string message = report.str();
        history_.push(std::move(world_));
        world_ = std::move(candidate); // noexcept; ownership transfers once.
        out << message;
        return true;
    } catch (const std::exception& error) {
        out << "Action failed: " << error.what() << '\n';
        return false;
    }
}

bool Simulation::createDefault(EntityKind kind, std::ostream& out) {
    return transact([=](World& world, std::ostream& report) {
        const int id = world.add(EntityFactory::createDefault(kind));
        report << "Created " << world.find(id)->kindName() << " #" << id << ".\n";
        return true;
    }, out);
}

bool Simulation::create(EntityKind kind, const EntitySettings& settings, std::ostream& out) {
    return transact([&](World& world, std::ostream& report) {
        const int id = world.add(EntityFactory::create(kind, settings));
        report << "Created " << world.find(id)->kindName() << " #" << id << ".\n";
        return true;
    }, out);
}

bool Simulation::duplicate(int id, std::ostream& out) {
    return transact([=](World& world, std::ostream& report) {
        const int newId = world.duplicate(id);
        report << "Duplicated #" << id << " as #" << newId << ".\n";
        return true;
    }, out);
}

bool Simulation::remove(int id, std::ostream& out) {
    return transact([=](World& world, std::ostream& report) {
        if (!world.erase(id)) { report << "Unknown entity ID.\n"; return false; }
        report << "Deleted #" << id << ". Pending requests keep their target IDs.\n";
        return true;
    }, out);
}

bool Simulation::modify(int id, const EntitySettings& settings, std::ostream& out) {
    return transact([&](World& world, std::ostream& report) {
        Entity* target = world.find(id);
        if (!target) { report << "Unknown entity ID.\n"; return false; }
        target->configure(settings);
        target->addLog("Attributes edited by the colony operator.");
        report << "Updated #" << id << ".\n";
        return true;
    }, out);
}

bool Simulation::enterEvent(EventKind kind, int targetId, int amount, std::ostream& out) {
    return transact([=](World& world, std::ostream& report) {
        if (world.outcome() != Outcome::Running) {
            report << "The mission has ended. Undo to resume before adding events.\n";
            return false;
        }
        if (!world.enqueueEvent(Event{kind, 0, targetId, amount, "External colony request"}, report)) return false;
        report << "Event queued for the end of the next turn.\n";
        return true;
    }, out);
}

bool Simulation::repair(int actorId, int targetId, std::ostream& out) {
    return transact([=](World& world, std::ostream& report) {
        if (world.outcome() != Outcome::Running) {
            report << "The mission has ended. Undo to resume.\n";
            return false;
        }
        Entity* actor = world.find(actorId);
        if (!actor) { report << "Unknown actor ID.\n"; return false; }
        // F5: this is capability discovery across two independent branches.
        // Ordinary turns do not cast or inspect a concrete entity type.
        auto* capability = dynamic_cast<RepairCapability*>(actor);
        if (!capability) { report << "Selected entity has no repair capability.\n"; return false; }
        return capability->requestRepair(world, targetId, report);
    }, out);
}

bool Simulation::heal(int actorId, int targetId, std::ostream& out) {
    return transact([=](World& world, std::ostream& report) {
        if (world.outcome() != Outcome::Running) {
            report << "The mission has ended. Undo to resume.\n";
            return false;
        }
        Entity* actor = world.find(actorId);
        if (!actor) { report << "Unknown actor ID.\n"; return false; }
        auto* capability = dynamic_cast<HealingCapability*>(actor);
        if (!capability) { report << "Selected entity has no healing capability.\n"; return false; }
        return capability->requestHealing(world, targetId, report);
    }, out);
}

bool Simulation::runTurns(int count, std::ostream& out) {
    if (count < 1 || count > 1000) { out << "Turn count must be 1..1000.\n"; return false; }
    if (world_.outcome() != Outcome::Running) {
        out << "The mission has ended. Undo a run to resume.\n";
        return false;
    }
    // One menu command, one snapshot, even when the command runs many turns.
    return transact([=](World& world, std::ostream& report) {
        for (int i = 0; i < count && world.outcome() == Outcome::Running; ++i)
            world.advanceTurn(report);
        return true;
    }, out);
}

bool Simulation::undo(std::ostream& out) {
    if (!history_.pop(world_)) { out << "Nothing to undo.\n"; return false; }
    out << "Restored the world before the last successful action.\n";
    world_.showStatus(out);
    return true;
}
