#include "World.h"
#include "EventProcessor.h"
#include <algorithm>
#include <iomanip>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <utility>

World::World(const World& other)
    : events_(other.events_), turn_(other.turn_), nextId_(other.nextId_),
      habitatIntegrity_(other.habitatIntegrity_), outcome_(other.outcome_) {
    entities_.reserve(other.entities_.size());
    for (const auto& entity : other.entities_) entities_.push_back(entity->clone());
}

World& World::operator=(const World& other) {
    if (this != &other) {
        World copy(other);
        swap(copy);
    }
    return *this;
}

void World::swap(World& other) noexcept {
    entities_.swap(other.entities_);
    events_.swap(other.events_);
    std::swap(turn_, other.turn_);
    std::swap(nextId_, other.nextId_);
    std::swap(habitatIntegrity_, other.habitatIntegrity_);
    std::swap(outcome_, other.outcome_);
}

Entity* World::find(int id) {
    for (auto& entity : entities_) if (entity->id() == id) return entity.get();
    return nullptr;
}
const Entity* World::find(int id) const {
    for (const auto& entity : entities_) if (entity->id() == id) return entity.get();
    return nullptr;
}

int World::add(std::unique_ptr<Entity> entity) {
    if (!entity) throw std::invalid_argument("Cannot add an empty entity.");
    if (nextId_ == std::numeric_limits<int>::max())
        throw std::overflow_error("No more entity IDs are available.");
    entity->assignId(nextId_);
    entities_.push_back(std::move(entity));
    return nextId_++;
}

bool World::erase(int id) {
    for (auto it = entities_.begin(); it != entities_.end(); ++it) {
        if ((*it)->id() == id) {
            entities_.erase(it); // unique_ptr calls the virtual destructor.
            return true;
        }
    }
    return false;
}

int World::duplicate(int id) {
    const Entity* source = find(id);
    if (!source) throw std::invalid_argument("Unknown entity ID.");
    // clone preserves everything; add changes only the ID. Do not append a log
    // message here: the duplicate must otherwise have exactly the same state.
    return add(source->clone());
}

int World::improveHabitat(int amount) {
    if (amount < 0) throw std::invalid_argument("Habitat gain cannot be negative.");
    if (habitatIntegrity_ == 0) return 0; // A collapsed habitat cannot recover.
    const int actual = std::min(amount, 100 - habitatIntegrity_);
    habitatIntegrity_ += actual;
    return actual;
}

int World::mostInjured(bool biological) const {
    const Entity* best = nullptr;
    for (const auto& candidate : entities_) {
        if (!candidate->active() || candidate->isBiological() != biological ||
            candidate->health() == candidate->maxHealth()) continue;
        // Compare health fractions with integer arithmetic; maximums <= 1000.
        if (!best || candidate->health() * best->maxHealth() <
                     best->health() * candidate->maxHealth()) best = candidate.get();
    }
    return best ? best->id() : 0; // Collection order breaks ties consistently.
}

int World::medicNeedingSupplies() const {
    const Entity* best = nullptr;
    for (const auto& candidate : entities_) {
        if (!candidate->active() || !candidate->acceptsMedicalSupplies() ||
            candidate->medicalSupplyCount() >= 10000) continue;
        if (!best || candidate->medicalSupplyCount() < best->medicalSupplyCount())
            best = candidate.get();
    }
    return best ? best->id() : 0;
}

bool World::validTarget(EventKind kind, int targetId, std::ostream& out) const {
    const Entity* target = find(targetId);
    if (!target) { out << "Unknown target ID.\n"; return false; }
    if (!target->active()) { out << "Target is inactive.\n"; return false; }
    switch (kind) {
    case EventKind::RepairRequest:
        if (target->isBiological()) { out << "Repairs require a robot target.\n"; return false; }
        break;
    case EventKind::MedicalRequest:
        if (!target->isBiological()) { out << "Treatment requires a colonist target.\n"; return false; }
        break;
    case EventKind::SupplyDelivery:
        if (!target->acceptsMedicalSupplies()) { out << "Supply deliveries require a medic target.\n"; return false; }
        return true;
    default:
        out << "Unknown event kind.\n";
        return false;
    }
    return true;
}

bool World::enqueueEvent(Event event, std::ostream& out) {
    if (event.amount < 1 || event.amount > 1000) {
        out << "Event amount must be 1..1000.\n";
        return false;
    }
    if (event.sourceId != 0 && !find(event.sourceId)) {
        out << "Unknown source ID.\n";
        return false;
    }
    if (!validTarget(event.kind, event.targetId, out)) return false;
    events_.enqueue(std::move(event));
    return true;
}

void World::processEvents(std::ostream& out) {
    if (events_.empty()) out << "  No pending events.\n";
    Event event;
    // Handlers never enqueue: every accepted event is processed once in FIFO
    // order, including requests entered before this turn began.
    while (events_.dequeue(event)) EventProcessor::apply(*this, event, out);
}

void World::evaluateOutcome(std::ostream& out) {
    bool colonistAlive = false;
    for (const auto& entity : entities_)
        if (entity->active() && entity->isBiological()) colonistAlive = true;
    if (habitatIntegrity_ == 0 || !colonistAlive) {
        outcome_ = Outcome::Lost;
        out << "Colony lost: " << (habitatIntegrity_ == 0 ? "habitat integrity reached zero." : "no active colonists remain.") << '\n';
    } else if (turn_ >= 20) {
        outcome_ = Outcome::Won;
        out << "Colony survived 20 turns. Mission accomplished!\n";
    }
}

void World::advanceTurn(std::ostream& out) {
    if (outcome_ != Outcome::Running) throw std::logic_error("The mission has already ended.");
    ++turn_;
    out << "\n--- Turn " << turn_ << " ---\n";
    habitatIntegrity_ = std::max(0, habitatIntegrity_ - 6);
    out << "Upkeep: habitat integrity " << habitatIntegrity_ << "/100.\n";
    if (habitatIntegrity_ < 30) {
        out << "Critical habitat: all active entities take 10 damage.\n";
        for (auto& entity : entities_) {
            if (!entity->active()) continue;
            entity->damage(10);
            if (!entity->active()) out << "  #" << entity->id() << " became inactive.\n";
        }
    }
    // Every surviving active entity acts once, even on the final turn.
    // improveHabitat refuses to revive a habitat that has already collapsed.
    for (auto& entity : entities_)
        if (entity->active()) entity->takeTurn(*this, out);
    processEvents(out);
    evaluateOutcome(out);
    out << "End of turn " << turn_ << ": habitat " << habitatIntegrity_ << "/100.\n";
}

void World::listEntities(std::ostream& out) const {
    if (entities_.empty()) { out << "The colony has no entities.\n"; return; }
    for (const auto& entity : entities_) {
        out << '#' << entity->id() << " | " << entity->name() << " | " << entity->kindName()
            << " | " << entity->health() << '/' << entity->maxHealth()
            << " | " << (entity->active() ? "active" : "inactive") << '\n';
    }
}

void World::showEvents(std::ostream& out) const {
    if (events_.empty()) { out << "No pending events.\n"; return; }
    std::size_t position = 0;
    events_.forEach([&](const Event& event) {
        out << ++position << ". " << eventName(event.kind) << " | source "
            << (event.sourceId == 0 ? "external" : "#" + std::to_string(event.sourceId))
            << " | target #" << event.targetId << " | amount " << event.amount
            << " | " << event.description << '\n';
    });
}

void World::showStatus(std::ostream& out) const {
    out << "Turn " << turn_ << "/20 | Habitat " << habitatIntegrity_ << "/100 | Entities "
        << entities_.size() << " | Pending events " << events_.size() << "\nMission: ";
    switch (outcome_) {
    case Outcome::Running: out << "in progress"; break;
    case Outcome::Won: out << "won"; break;
    case Outcome::Lost: out << "lost"; break;
    }
    out << '\n';
}
