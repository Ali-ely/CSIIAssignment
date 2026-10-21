#ifndef SIMWORLD_WORLD_H
#define SIMWORLD_WORLD_H

#include "CustomQueue.h"
#include "Entity.h"
#include "Event.h"
#include <iosfwd>
#include <memory>
#include <vector>

enum class Outcome { Running, Won, Lost };

// Owns one heterogeneous collection and all snapshot-worthy state. Its copy
// constructor clones dynamic entity types; history belongs to Simulation, so
// copying a World never recursively copies the undo history.
class World {
    std::vector<std::unique_ptr<Entity>> entities_;
    CustomQueue<Event> events_;
    int turn_ = 0;
    int nextId_ = 1;
    int habitatIntegrity_ = 100;
    Outcome outcome_ = Outcome::Running;

    void evaluateOutcome(std::ostream& out);
public:
    World() = default;
    World(const World& other);
    World& operator=(const World& other);
    World(World&& other) noexcept = default;
    World& operator=(World&& other) noexcept = default;
    ~World() = default;
    void swap(World& other) noexcept;

    int turn() const { return turn_; }
    int nextId() const { return nextId_; }
    int habitatIntegrity() const { return habitatIntegrity_; }
    Outcome outcome() const { return outcome_; }
    std::size_t entityCount() const { return entities_.size(); }
    const Entity& entityAt(std::size_t index) const { return *entities_.at(index); }
    const CustomQueue<Event>& pendingEvents() const { return events_; }
    Entity* find(int id);
    const Entity* find(int id) const;
    // Management uses the same factory and virtual validation as the UI.
    int createDefault(EntityKind kind);
    int create(EntityKind kind, const EntitySettings& settings);
    bool modify(int id, const EntitySettings& settings);
    bool inspect(int id, std::ostream& out) const;
    int add(std::unique_ptr<Entity> entity);
    bool erase(int id);
    int duplicate(int id);
    int improveHabitat(int amount);
    int mostInjured(bool biological) const;
    int medicNeedingSupplies() const;
    bool validTarget(EventKind kind, int targetId, std::ostream& out) const;
    bool enqueueEvent(Event event, std::ostream& out);
    void processEvents(std::ostream& out);
    void advanceTurn(std::ostream& out);
    void listEntities(std::ostream& out) const;
    void showEvents(std::ostream& out) const;
    void showStatus(std::ostream& out) const;
};

#endif
