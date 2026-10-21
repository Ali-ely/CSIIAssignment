#ifndef SIMWORLD_SIMULATION_H
#define SIMWORLD_SIMULATION_H

#include "CustomStack.h"
#include "World.h"
#include <functional>
#include <iosfwd>

// Application command boundary. Every mutation works on a temporary deep copy;
// only a successful command commits and pushes the prior world onto our stack.
// Keeping history outside World avoids snapshots containing earlier snapshots.
class Simulation {
    World world_;
    CustomStack<World> history_;
    bool transact(const std::function<bool(World&, std::ostream&)>& action,
                  std::ostream& out);
public:
    const World& world() const { return world_; }
    std::size_t undoDepth() const { return history_.size(); }
    bool createDefault(EntityKind kind, std::ostream& out);
    bool create(EntityKind kind, const EntitySettings& settings, std::ostream& out);
    bool duplicate(int id, std::ostream& out);
    bool remove(int id, std::ostream& out);
    bool modify(int id, const EntitySettings& settings, std::ostream& out);
    bool enterEvent(EventKind kind, int targetId, int amount, std::ostream& out);
    bool repair(int actorId, int targetId, std::ostream& out);
    bool heal(int actorId, int targetId, std::ostream& out);
    bool runTurns(int count, std::ostream& out);
    bool undo(std::ostream& out);
};

#endif
