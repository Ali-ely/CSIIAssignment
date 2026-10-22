// Regression checks for ownership, polymorphic copies, FIFO events, undo, and
// input recovery. This is a separate executable; do not link it with main.cpp.
#include "ConsoleUI.h"
#include "CustomQueue.h"
#include "CustomStack.h"
#include "Engineer.h"
#include "EntityFactory.h"
#include "MaintenanceBot.h"
#include "Medic.h"
#include "MedicalDrone.h"
#include "Simulation.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <set>
#include <typeinfo>
#include <utility>

namespace {
void check(bool condition, const std::string& description) {
    if (!condition) throw std::runtime_error(description);
}

std::string fingerprint(const World& world) {
    std::ostringstream out;
    out << world.turn() << ' ' << world.nextId() << ' ' << world.habitatIntegrity()
        << ' ' << static_cast<int>(world.outcome()) << '\n';
    for (std::size_t i = 0; i < world.entityCount(); ++i) {
        const Entity& entity = world.entityAt(i);
        entity.printDetails(out);
        const auto s = entity.settings();
        out << s.name << ' ' << s.health << ' ' << s.maxHealth << ' ' << s.active
            << ' ' << s.energy << ' ' << s.maxEnergy << ' ' << s.skill << ' ' << s.supplies << '\n';
    }
    world.showEvents(out);
    return out.str();
}

void containers() {
    CustomQueue<std::string> queue;
    std::string value = "untouched";
    check(!queue.dequeue(value) && value == "untouched" && !queue.front(), "empty queue");
    queue.enqueue("first"); queue.enqueue("second"); queue.enqueue("third");
    CustomQueue<std::string> copy(queue);
    CustomQueue<std::string> assigned;
    assigned.enqueue("old"); assigned = queue;
    queue = queue;
    check(queue.size() == 3, "queue self-assignment");
    for (const char* expected : {"first", "second", "third"}) {
        check(queue.dequeue(value) && value == expected, "FIFO order");
        check(copy.dequeue(value) && value == expected, "queue copy order");
        check(assigned.dequeue(value) && value == expected, "queue assignment order");
    }
    queue.enqueue("after empty");
    check(queue.dequeue(value) && value == "after empty", "queue tail reset");
    queue.enqueue("moved");
    CustomQueue<std::string> moved(std::move(queue));
    check(queue.empty() && moved.dequeue(value) && value == "moved", "queue move ownership");

    CustomStack<std::string> stack;
    check(!stack.pop(value) && !stack.top(), "empty stack");
    stack.push("first"); stack.push("second"); stack.push("third");
    CustomStack<std::string> stackCopy(stack);
    CustomStack<std::string> stackAssigned;
    stackAssigned.push("old"); stackAssigned = stack;
    stack = stack;
    for (const char* expected : {"third", "second", "first"}) {
        check(stack.pop(value) && value == expected, "LIFO order");
        check(stackCopy.pop(value) && value == expected, "stack copy order");
        check(stackAssigned.pop(value) && value == expected, "stack assignment order");
    }
    stack.push("moved");
    CustomStack<std::string> movedStack(std::move(stack));
    check(stack.empty() && movedStack.pop(value) && value == "moved", "stack move ownership");
}

template <typename T>
void assignmentCheck() {
    T original;
    for (int i = 0; i < 50; ++i) original.addLog("Entry " + std::to_string(i));
    T assigned;
    assigned.addLog("Replaced");
    assigned = original;
    assigned = assigned;
    original.addLog("Original only");
    assigned.addLog("Assigned only");
    check(assigned.logSize() == 51 && assigned.logEntry(50) == "Assigned only", "derived assignment independence");
    check(original.logEntry(50) == "Original only", "original survives assignment");
    T copied(original);
    original.addLog("Another");
    check(copied.logSize() == 51, "derived copy constructor independence");

    // Assignment between objects already owned by one world must not duplicate
    // IDs or redirect queued events. Exercise this for every concrete kind.
    World world;
    auto first = std::make_unique<T>();
    auto second = std::make_unique<T>();
    T* destination = first.get();
    T* source = second.get();
    source->damage(60);
    source->addLog("Source history");
    const int destinationId = world.add(std::move(first));
    const int sourceId = world.add(std::move(second));
    std::ostringstream report;
    const EventKind eventKind = destination->isBiological()
        ? EventKind::MedicalRequest : EventKind::RepairRequest;
    check(world.enqueueEvent(Event{eventKind, 0, destinationId, 10, "Pending before assignment"}, report),
          "queue event for assignment destination");
    *destination = *source;
    check(destination->id() == destinationId && source->id() == sourceId,
          "assignment preserves distinct registered IDs");
    check(world.find(destinationId) == destination && world.find(sourceId) == source,
          "assignment preserves world lookup");
    check(destination->logEntry(1) == "Source history", "assignment copies registered history");
    world.processEvents(report);
    check(destination->health() == 50 && source->health() == 40,
          "queued event still reaches assignment destination only");
    destination->addLog("Destination only");
    check(source->logSize() == 2, "registered assignment retains independent history");
    check(world.erase(sourceId) && destination->logEntry(1) == "Source history",
          "assigned entity survives source deletion");

}

void copying() {
    std::ostringstream report;
    for (int kind = 1; kind <= 4; ++kind) {
        World world;
        auto entity = EntityFactory::createDefault(static_cast<EntityKind>(kind));
        for (int i = 0; i < 40; ++i) entity->addLog("History " + std::to_string(i));
        const int originalId = world.add(std::move(entity));
        const int copyId = world.duplicate(originalId);
        check(copyId != originalId, "duplicate gets unique ID");
        const Entity* original = world.find(originalId);
        Entity* duplicate = world.find(copyId);
        check(std::string(original->kindName()) == duplicate->kindName(), "clone preserves concrete type");
        check(original->settings().name == duplicate->settings().name, "duplicate preserves name");
        for (std::size_t j = 0; j < original->logSize(); ++j)
            check(original->logEntry(j) == duplicate->logEntry(j), "clone preserves full dynamic history");
        auto settings = duplicate->settings();
        settings.name = "Independent"; settings.skill = 9; settings.supplies = 2;
        duplicate->configure(settings);
        duplicate->addLog("Copy only");
        check(original->logSize() == 40 && original->settings().supplies == 10, "duplicate mutations independent");
        check(world.erase(originalId), "delete original");
        check(world.find(copyId)->logEntry(39) == "History 39", "duplicate survives original deletion");
        World snapshot(world);
        const std::string expected = fingerprint(snapshot);
        world.find(copyId)->damage(5);
        check(fingerprint(snapshot) == expected, "world copy independence");
        World assigned;
        assigned = snapshot;
        assigned = assigned;
        check(fingerprint(assigned) == expected, "world copy assignment");
    }
    assignmentCheck<Engineer>(); assignmentCheck<Medic>();
    assignmentCheck<MaintenanceBot>(); assignmentCheck<MedicalDrone>();
}

// Ignore only the leading ID when comparing complete printed role state/history.
std::string stateWithoutId(const Entity& entity) {
    std::ostringstream out;
    entity.printDetails(out);
    return out.str().substr(out.str().find(' '));
}

void worldManagement() {
    World world;
    std::ostringstream empty;
    world.listEntities(empty);
    check(empty.str().find("no entities") != std::string::npos, "empty listing");
    std::set<int> issued;
    for (int kind = 1; kind <= 4; ++kind) {
        const auto type = static_cast<EntityKind>(kind);
        const int defaultId = world.createDefault(type);
        check(issued.insert(defaultId).second && defaultId > 0, "unique default ID");
        auto expectedDefault = EntityFactory::createDefault(type);
        check(stateWithoutId(*world.find(defaultId)) == stateWithoutId(*expectedDefault),
              "World preserves concrete defaults");

        EntitySettings custom;
        custom.name = "Custom role " + std::to_string(kind);
        custom.health = 47; custom.maxHealth = 170; custom.active = false;
        custom.energy = 23; custom.maxEnergy = 190;
        custom.skill = 8; custom.supplies = 321;
        const int originalId = world.create(type, custom);
        check(issued.insert(originalId).second, "unique custom ID");
        Entity* original = world.find(originalId);
        auto expectedCustom = EntityFactory::create(type, custom);
        check(stateWithoutId(*original) == stateWithoutId(*expectedCustom),
              "World preserves all custom attributes");
        for (int entry = 0; entry < 40; ++entry)
            original->addLog("Original history " + std::to_string(entry));
        const std::string before = stateWithoutId(*original);
        const int duplicateId = world.duplicate(originalId);
        check(issued.insert(duplicateId).second, "unique duplicate ID");
        Entity* duplicate = world.find(duplicateId);
        check(typeid(*original) == typeid(*duplicate) &&
              stateWithoutId(*duplicate) == before, "duplicate preserves complete state except ID");
        for (std::size_t entry = 0; entry < original->logSize(); ++entry)
            check(&original->logEntry(entry) != &duplicate->logEntry(entry),
                  "duplicate owns separate dynamic storage");
        auto edited = custom;
        edited.name = "Edited duplicate"; edited.health = 80; edited.maxHealth = 200;
        edited.active = true; edited.energy = 91; edited.maxEnergy = 210;
        edited.skill = 3; edited.supplies = 17;
        check(world.modify(duplicateId, edited), "modify existing duplicate");
        auto expectedEdit = EntityFactory::create(type, edited);
        for (int entry = 0; entry < 40; ++entry)
            expectedEdit->addLog("Original history " + std::to_string(entry));
        check(duplicate->id() == duplicateId &&
              stateWithoutId(*duplicate) == stateWithoutId(*expectedEdit),
              "editing preserves ID, concrete type and history");
        duplicate->addLog("Duplicate only");
        check(stateWithoutId(*original) == before, "editing duplicate leaves original unchanged");
        check(world.erase(duplicateId) && !world.find(duplicateId), "delete duplicate");
        check(stateWithoutId(*world.find(originalId)) == before,
              "deleting duplicate leaves original alive and unchanged");
        const int anotherId = world.duplicate(originalId);
        check(issued.insert(anotherId).second && anotherId > duplicateId, "deleted IDs are not reused");
        check(world.erase(originalId) && stateWithoutId(*world.find(anotherId)) == before,
              "duplicate also survives original deletion");
    }
    check(world.entityCount() == 8, "all four types share one collection");
    const auto before = fingerprint(world);
    const World& readOnly = world;
    std::ostringstream listing;
    readOnly.listEntities(listing);
    for (std::size_t i = 0; i < readOnly.entityCount(); ++i) {
        const Entity& entity = readOnly.entityAt(i);
        check(readOnly.find(entity.id()) == &entity, "const lookup matches collection");
        check(listing.str().find("#" + std::to_string(entity.id()) + " | " + entity.name())
              != std::string::npos, "listing includes every entity");
        std::ostringstream actual, expected;
        entity.printDetails(expected);
        check(readOnly.inspect(entity.id(), actual) && actual.str() == expected.str(),
              "inspection includes all role details and history");
    }
    std::ostringstream missing;
    check(!world.inspect(-1, missing) && !world.modify(-1, EntitySettings{}) && !world.erase(-1),
          "unknown IDs rejected");
    const auto mustReject = [&](auto operation) {
        bool rejected = false;
        try { operation(); } catch (const std::invalid_argument&) { rejected = true; }
        check(rejected && fingerprint(world) == before, "invalid operation leaves world and next ID unchanged");
    };
    mustReject([&] { world.add(nullptr); });
    mustReject([&] { world.duplicate(-1); });
    mustReject([&] { world.createDefault(static_cast<EntityKind>(99)); });
    mustReject([&] { world.create(static_cast<EntityKind>(99), EntitySettings{}); });
    const int targetId = world.entityAt(0).id();
    auto invalid = world.find(targetId)->settings();
    invalid.name = "Must not be applied"; invalid.energy = invalid.maxEnergy + 1;
    mustReject([&] { world.modify(targetId, invalid); });
    for (int kind = 1; kind <= 4; ++kind)
        mustReject([&] { world.create(static_cast<EntityKind>(kind), invalid); });
    check(fingerprint(world) == before, "list and inspect are read-only");
}

void undoAllActions() {
    Simulation simulation;
    std::ostringstream out;
    const std::string empty = fingerprint(simulation.world());
    check(!simulation.undo(out), "empty undo");
    check(simulation.createDefault(EntityKind::Engineer, out), "create defaults");
    const std::string one = fingerprint(simulation.world());
    check(simulation.duplicate(1, out), "duplicate command");
    const std::string two = fingerprint(simulation.world());
    auto changed = simulation.world().find(2)->settings();
    changed.name = "Changed"; changed.health = 42; changed.supplies = 99;
    check(simulation.modify(2, changed, out), "modify command");
    const std::string edited = fingerprint(simulation.world());
    check(simulation.remove(2, out), "remove command");
    check(simulation.undo(out) && fingerprint(simulation.world()) == edited, "undo deletion exact");
    check(simulation.undo(out) && fingerprint(simulation.world()) == two, "undo edit exact");
    check(simulation.undo(out) && fingerprint(simulation.world()) == one, "undo duplicate exact");
    check(simulation.undo(out) && fingerprint(simulation.world()) == empty, "undo creation exact incl ID allocator");
    check(!simulation.undo(out), "exhausted undo");

    simulation.createDefault(EntityKind::Engineer, out);
    simulation.createDefault(EntityKind::Medic, out);
    simulation.enterEvent(EventKind::MedicalRequest, 1, 7, out);
    const auto pending = fingerprint(simulation.world());
    const auto depth = simulation.undoDepth();
    check(!simulation.remove(999, out), "invalid delete rejected");
    changed = simulation.world().find(1)->settings();
    changed.health = -1;
    check(!simulation.modify(1, changed, out), "invalid attributes rejected");
    check(!simulation.create(static_cast<EntityKind>(99), EntitySettings{}, out), "invalid kind rejected");
    check(simulation.undoDepth() == depth && fingerprint(simulation.world()) == pending, "failed actions are atomic");
    simulation.remove(1, out);
    check(simulation.undo(out) && fingerprint(simulation.world()) == pending, "deletion undo restores queued target");
    check(simulation.runTurns(5, out), "run multiple turns");
    check(simulation.undoDepth() == depth + 1, "Run N is one undo level");
    check(simulation.undo(out) && fingerprint(simulation.world()) == pending, "run undo restores entire world and events");
}

void capabilitiesAndEvents() {
    Simulation simulation;
    std::ostringstream out;
    for (int kind = 1; kind <= 4; ++kind) simulation.createDefault(static_cast<EntityKind>(kind), out);
    auto wounded = simulation.world().find(1)->settings(); wounded.health = 30;
    simulation.modify(1, wounded, out);
    wounded = simulation.world().find(3)->settings(); wounded.health = 30;
    simulation.modify(3, wounded, out);
    auto before = fingerprint(simulation.world());
    auto depth = simulation.undoDepth();
    check(!simulation.repair(2, 3, out), "medic lacks repair capability");
    check(!simulation.heal(1, 1, out), "engineer lacks healing capability");
    check(!simulation.repair(999, 3, out), "missing actor");
    check(!simulation.repair(1, 1, out), "invalid repair target");
    check(!simulation.heal(2, 3, out), "invalid healing target");
    check(!simulation.enterEvent(EventKind::SupplyDelivery, 1, 5, out), "invalid supply target");
    check(!simulation.enterEvent(EventKind::RepairRequest, 3, 0, out), "zero event amount");
    check(!simulation.enterEvent(static_cast<EventKind>(99), 3, 5, out), "invalid event kind");
    check(simulation.undoDepth() == depth && fingerprint(simulation.world()) == before, "failed capabilities leave no changes");
    check(simulation.repair(1, 3, out), "engineer repair capability");
    check(simulation.world().find(1)->settings().energy == 90 &&
          simulation.world().find(1)->settings().supplies == 9, "capability charges once at acceptance");
    check(simulation.undo(out) && fingerprint(simulation.world()) == before, "capability undo restores cost and queue");
    check(simulation.repair(3, 3, out), "bot repair capability");
    check(simulation.heal(2, 1, out), "medic healing capability");
    check(simulation.heal(4, 1, out), "drone healing capability");
    before = fingerprint(simulation.world());
    std::ostringstream view1, view2;
    simulation.world().showEvents(view1); simulation.world().showEvents(view2);
    check(view1.str() == view2.str() && fingerprint(simulation.world()) == before, "event inspection is read-only");
    check(simulation.runTurns(1, out), "polymorphic turn");
    check(simulation.world().pendingEvents().empty(), "turn drains events");
    check(simulation.world().find(1)->health() > 30 && simulation.world().find(3)->health() > 30, "medical and repair effects");
    check(simulation.world().find(4)->settings().supplies < 9, "drone distinct delivery behavior");
    check(simulation.world().habitatIntegrity() == 99, "engineer distinct habitat behavior");
    check(simulation.undo(out) && fingerprint(simulation.world()) == before, "complex turn undo exact");

    World fifo;
    auto injured = EntitySettings{}; injured.health = 90;
    const int id = fifo.add(EntityFactory::create(EntityKind::Engineer, injured));
    fifo.enqueueEvent(Event{EventKind::MedicalRequest, 0, id, 7, "first"}, out);
    fifo.enqueueEvent(Event{EventKind::MedicalRequest, 0, id, 8, "second"}, out);
    fifo.processEvents(out);
    check(fifo.find(id)->logEntry(0) == "Restored 7 health/integrity." &&
          fifo.find(id)->logEntry(1) == "Restored 3 health/integrity.", "observable FIFO ordering with capped effects");
    const int medicId = fifo.add(EntityFactory::createDefault(EntityKind::Medic));
    fifo.enqueueEvent(Event{EventKind::SupplyDelivery, 0, medicId, 5, "supplies"}, out);
    fifo.processEvents(out);
    check(fifo.find(medicId)->medicalSupplyCount() == 15, "supply delivery effect");
    fifo.enqueueEvent(Event{EventKind::MedicalRequest, 0, id, 5, "deleted target"}, out);
    fifo.erase(id);
    fifo.processEvents(out);
    check(fifo.pendingEvents().empty() && out.str().find("Request discarded") != std::string::npos, "deleted event target safe");
}

void boundariesAndOutcomes() {
    std::ostringstream out;
    Simulation simulation;
    simulation.createDefault(EntityKind::Engineer, out);
    const auto before = fingerprint(simulation.world());
    check(simulation.runTurns(1000, out) && simulation.world().turn() == 20 &&
          simulation.world().outcome() == Outcome::Won, "survival win stops batch");
    check(!simulation.runTurns(1, out), "ended mission blocks runs");
    check(simulation.undo(out) && fingerprint(simulation.world()) == before, "undo restores pre-victory world");
    check(!simulation.runTurns(0, out) && !simulation.runTurns(1001, out), "invalid turn count");

    Simulation empty;
    check(empty.runTurns(1, out) && empty.world().outcome() == Outcome::Lost, "empty colony loses safely");
    check(empty.undo(out) && empty.world().outcome() == Outcome::Running, "undo defeat");
    Simulation noEngineer;
    noEngineer.createDefault(EntityKind::Medic, out);
    check(noEngineer.runTurns(20, out) && noEngineer.world().outcome() == Outcome::Lost &&
          noEngineer.world().habitatIntegrity() == 0, "habitat collapse");

    World world;
    auto settings = EntitySettings{}; settings.health = 5;
    auto entity = EntityFactory::create(EntityKind::MedicalDrone, settings);
    entity->damage(1000);
    check(!entity->active() && entity->health() == 0, "damage clamps and deactivates");
    const int id = world.add(std::move(entity));
    check(!world.enqueueEvent(Event{EventKind::RepairRequest, 0, id, 5, "inactive"}, out), "inactive event target rejected");
    settings = EntitySettings{}; settings.energy = 0;
    const int engineer = world.add(EntityFactory::create(EntityKind::Engineer, settings));
    world.advanceTurn(out);
    check(world.find(engineer)->settings().energy == 25, "low-stamina rest");
    check(world.find(id)->logSize() == 1, "inactive entity skips normal turn");

    // Repeated allocation, growth, snapshot replacement and destruction are
    // exercised under the memory checker in addition to functional assertions.
    for (int cycle = 0; cycle < 50; ++cycle) {
        Simulation stress;
        for (int kind = 1; kind <= 4; ++kind) stress.createDefault(static_cast<EntityKind>(kind), out);
        stress.runTurns(20, out);
        stress.undo(out);
        stress.duplicate(1, out);
        stress.remove(1, out);
        while (stress.undoDepth()) stress.undo(out);
    }
}

void turnSimulation() {
    Simulation batch, singles;
    std::ostringstream setup;
    for (int kind = 1; kind <= 4; ++kind) {
        EntitySettings settings;
        settings.name = "Turn role " + std::to_string(kind);
        settings.health = 20;
        check(batch.create(static_cast<EntityKind>(kind), settings, setup) &&
              singles.create(static_cast<EntityKind>(kind), settings, setup), "prepare turn colony");
    }
    std::ostringstream batchReport, singleReport;
    check(batch.runTurns(3, batchReport), "run three consecutive turns");
    for (int turn = 1; turn <= 3; ++turn) {
        check(singles.runTurns(1, singleReport), "run a single turn");
        const World& world = singles.world();
        check(world.turn() == turn && world.habitatIntegrity() == 100 - turn,
              "turn counter, upkeep and engineer habitat work");
        check(world.find(1)->settings().energy == 100 - 12 * turn &&
              world.find(2)->settings().energy == 100 - 10 * turn &&
              world.find(3)->settings().energy == 100 - 10 * turn &&
              world.find(4)->settings().energy == 100 - 8 * turn,
              "each role acts once and pays its own energy cost");
        check(world.find(2)->settings().supplies == 10 + turn &&
              world.find(3)->settings().supplies == 10 - turn &&
              world.find(4)->settings().supplies == 10 - 2 * turn,
              "treatment, repair and delivery inventories evolve");
        check(world.pendingEvents().empty(), "turn resolves queued effects");
    }
    check(fingerprint(batch.world()) == fingerprint(singles.world()),
          "batch equals consecutive single turns");
    check(batch.world().find(1)->health() + batch.world().find(2)->health() == 94 &&
          batch.world().find(3)->health() + batch.world().find(4)->health() == 94,
          "three treatments and repairs change health");
    for (const char* action : {"Engineer #1", "Medic #2", "MaintenanceBot #3",
                               "MedicalDrone #4", "strengthens the habitat",
                               "queued treatment", "queued repair", "dispatches",
                               "Energy: 100 -> 88", "End of turn 3"})
        check(batchReport.str().find(action) != std::string::npos, "clear distinct action reports");

    World recovery;
    for (int kind = 1; kind <= 4; ++kind) {
        EntitySettings exhausted;
        exhausted.health = 20; exhausted.energy = 0;
        recovery.create(static_cast<EntityKind>(kind), exhausted);
    }
    recovery.advanceTurn(setup);
    check(recovery.find(1)->settings().energy == 25 && recovery.find(2)->settings().energy == 25 &&
          recovery.find(3)->settings().energy == 30 && recovery.find(4)->settings().energy == 30,
          "exhausted colonists rest and robots recharge");
    recovery.advanceTurn(setup);
    check(recovery.find(1)->settings().energy == 13 && recovery.find(2)->settings().energy == 15 &&
          recovery.find(3)->settings().energy == 20 && recovery.find(4)->settings().energy == 22,
          "recovered entities resume role actions next turn");

    World inactive;
    inactive.createDefault(EntityKind::Engineer);
    for (int kind = 1; kind <= 4; ++kind) {
        EntitySettings disabled;
        disabled.active = false; disabled.health = 0; disabled.energy = 17;
        inactive.create(static_cast<EntityKind>(kind), disabled);
    }
    World initial(inactive);
    for (int turn = 0; turn < 3; ++turn) inactive.advanceTurn(setup);
    for (int id = 2; id <= 5; ++id)
        check(stateWithoutId(*inactive.find(id)) == stateWithoutId(*initial.find(id)),
              "inactive roles never act, recover or receive effects");

    // Without an engineer, upkeep becomes critical on turn 12. The fragile
    // bot dies before dispatch, so it must not act or receive its queued repair.
    World danger;
    EntitySettings medic;
    medic.supplies = 0;
    danger.create(EntityKind::Medic, medic);
    EntitySettings fragile;
    fragile.health = 5; fragile.supplies = 0;
    const int bot = danger.create(EntityKind::MaintenanceBot, fragile);
    for (int turn = 0; turn < 11; ++turn) danger.advanceTurn(setup);
    check(danger.enqueueEvent(Event{EventKind::RepairRequest, 0, bot, 10, "Pending repair"}, setup),
          "queue repair before critical upkeep");
    const auto oldLogSize = danger.find(bot)->logSize();
    std::ostringstream critical;
    danger.advanceTurn(critical);
    check(!danger.find(bot)->active() && danger.find(bot)->health() == 0 &&
          danger.find(bot)->logSize() == oldLogSize + 1,
          "critical damage deactivates entity before its action");
    check(critical.str().find("became inactive") != std::string::npos &&
          critical.str().find("turn skipped") != std::string::npos &&
          critical.str().find("Request discarded") != std::string::npos,
          "deactivation and discarded event clearly reported");
    const auto deadState = stateWithoutId(*danger.find(bot));
    danger.advanceTurn(setup);
    check(stateWithoutId(*danger.find(bot)) == deadState, "newly inactive entity stays unchanged");
    check(danger.erase(bot), "remove inactive entity safely");

    World removed;
    removed.createDefault(EntityKind::Engineer);
    const int target = removed.createDefault(EntityKind::MedicalDrone);
    check(removed.enqueueEvent(Event{EventKind::RepairRequest, 0, target, 10, "Deleted target"}, setup),
          "queue event before deletion");
    check(removed.erase(target), "delete active entity before next turn");
    std::ostringstream afterDeletion;
    removed.advanceTurn(afterDeletion);
    check(!removed.find(target) && removed.entityCount() == 1 && removed.pendingEvents().empty() &&
          afterDeletion.str().find("Request discarded") != std::string::npos &&
          afterDeletion.str().find("MedicalDrone") == std::string::npos,
          "deleted entity never acts and stale event is safely discarded");
}

void optionalCapabilities() {
    // Exercise both actual command-handler casts for every concrete type.
    // Each attempt starts from the same state so failed casts cannot hide costs.
    for (int kind = 1; kind <= 4; ++kind) {
        for (bool repair : {true, false}) {
            Simulation simulation;
            std::ostringstream setup, report;
            check(simulation.createDefault(static_cast<EntityKind>(kind), setup), "create capability actor");
            EntitySettings injured;
            injured.health = 40;
            check(simulation.create(repair ? EntityKind::MaintenanceBot : EntityKind::Engineer,
                                    injured, setup), "create suitable capability target");
            const std::string before = fingerprint(simulation.world());
            const auto depth = simulation.undoDepth();
            const bool expected = repair ? (kind == 1 || kind == 3) : (kind == 2 || kind == 4);
            const bool accepted = repair ? simulation.repair(1, 2, report) : simulation.heal(1, 2, report);
            check(accepted == expected, "optional capability membership matches design");
            if (expected) {
                const auto actor = simulation.world().find(1)->settings();
                const Event* event = simulation.world().pendingEvents().front();
                check(actor.energy == 90 && actor.supplies == 9 &&
                      simulation.world().find(2)->health() == 40 && event &&
                      event->sourceId == 1 && event->targetId == 2 && event->amount == 18 &&
                      event->kind == (repair ? EventKind::RepairRequest : EventKind::MedicalRequest),
                      "successful cast queues correct effect and charges exactly once");
                World resolved(simulation.world());
                resolved.processEvents(report);
                check(resolved.find(2)->health() == 58 && resolved.pendingEvents().empty(),
                      "capability has its expected effect when events resolve");
                check(simulation.undoDepth() == depth + 1 && simulation.undo(report) &&
                      fingerprint(simulation.world()) == before, "successful capability remains undoable");
            } else {
                check(report.str().find(repair ? "has no repair capability" : "has no healing capability")
                      != std::string::npos && report.str().find("Entity #1") != std::string::npos,
                      "failed cast identifies actor and missing capability");
                check(fingerprint(simulation.world()) == before && simulation.undoDepth() == depth,
                      "failed cast preserves resources, history, events and undo depth");
                check(simulation.createDefault(EntityKind::Engineer, report),
                      "application continues after unsupported capability");
            }
        }
    }

    // Drive the real menu: two failed casts, then two accepted requests,
    // followed by event inspection and an ordinary polymorphic turn.
    std::istringstream input(
        "2\n1\nInjured engineer\n100\n40\n1\n100\n100\n5\n10\n"
        "1\n2\n"
        "2\n3\nDamaged bot\n100\n40\n1\n100\n100\n5\n10\n"
        "8\n2\n3\n9\n1\n1\n8\n1\n3\n9\n2\n1\n11\n12\n0\n");
    std::ostringstream output;
    ConsoleUI ui(input, output);
    ui.run();
    const auto text = output.str();
    for (const char* expected : {"has no repair capability", "has no healing capability",
                                 "queued repair for #3", "queued treatment for #1",
                                 "source #1", "source #2", "End of turn 1"})
        check(text.find(expected) != std::string::npos, "menu continues after failed capability casts");
}

void consoleInput() {
    std::istringstream input("oops\n999999999999999999999999999999999\n1junk\n-5\n16\n1\n7\n1\n4\n999\n14\n14\n0\n");
    std::ostringstream output;
    ConsoleUI ui(input, output);
    ui.run();
    const auto text = output.str();
    check(text.find("Enter a whole number") != std::string::npos &&
          text.find("Created Engineer #1") != std::string::npos &&
          text.find("Unknown entity ID") != std::string::npos &&
          text.find("Nothing to undo") != std::string::npos, "console recovers from invalid input");
    std::istringstream partial("2\n1\nName\n");
    std::ostringstream partialOut;
    ConsoleUI partialUi(partial, partialOut);
    partialUi.run();
    check(partialOut.str().find("Created") == std::string::npos, "EOF aborts incomplete creation");
    std::istringstream custom("2\n2\nDr Vega\n100\n60\n1\n100\n80\n7\n12\n4\n1\n0\n");
    std::ostringstream customOut;
    ConsoleUI customUi(custom, customOut);
    customUi.run();
    check(customOut.str().find("Dr Vega [Medic]") != std::string::npos &&
          customOut.str().find("60/100") != std::string::npos, "custom console creation");
}
}

int main() {
    try {
        containers(); std::cout << "PASS custom stack and queue\n";
        copying(); std::cout << "PASS dynamic copying and assignment\n";
        worldManagement(); std::cout << "PASS World entity management and duplicate independence\n";
        undoAllActions(); std::cout << "PASS exact multi-level undo\n";
        capabilitiesAndEvents(); std::cout << "PASS capabilities, polymorphism and FIFO events\n";
        boundariesAndOutcomes(); std::cout << "PASS bounds, outcomes and ownership stress\n";
        turnSimulation(); std::cout << "PASS consecutive turns, recovery, inactivity and deletion\n";
        optionalCapabilities(); std::cout << "PASS optional capability cast matrix and menu recovery\n";
        consoleInput(); std::cout << "PASS console validation and EOF\n";
        std::cout << "All regression groups passed.\n";
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
