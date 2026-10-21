/*
CSCE 1101 Assignment 1 - SimWorld: Space Colony
Team member 1: TODO - enter name and AUC ID before submission.
Team member 2: TODO - enter name and AUC ID before submission.

WORLD AND DESIGN
Humans and robots cooperate to keep a habitat alive for 20 turns. Upkeep costs
6 integrity. Engineers restore the habitat, medics queue treatment, maintenance
bots queue robot repairs, and medical drones queue medicine deliveries. Energy
and inventories limit work. Below 30 integrity, active entities take 10 damage.
Zero health makes an entity inactive. Events run FIFO after all entity actions.

Entity (abstract)
  +-- Colonist (abstract): stamina and rest
  |     +-- Engineer + RepairCapability
  |     +-- Medic + HealingCapability
  +-- Robot (abstract): battery and charging
        +-- MaintenanceBot + RepairCapability
        +-- MedicalDrone + HealingCapability

Shared identity/health/dynamic history belongs to Entity. Biological and
mechanical energy rules belong to their middle-level bases. RoleResources is a
small composed value class for reusable skill/inventory validation. Capability
interfaces span the two branches without introducing an Entity diamond.

FEATURE MAP
F1  Entity.h, Colonist.h, Robot.h, four concrete role classes: three levels,
    four concrete kinds, private validated attributes and shared base behavior.
F2  EntityFactory.cpp create/createDefault; Entity.cpp constructors, assignment,
    destructor and growable string array; specialized constructors initialize
    their inherited portions. Every concrete object owns its own base log.
F3  World.cpp duplicate; concrete clone overrides preserve type and deep state;
    World::add assigns the sole difference, a fresh ID.
F4  World.cpp advanceTurn; four takeTurn overrides; one vector of owning base
    pointers; no entity-type checks for normal turn dispatch.
F5  Simulation.cpp repair/heal: checked dynamic_cast to optional interfaces;
    the concrete request methods validate energy, inventory, and targets.
F6  CustomQueue.h; Event.h; World.cpp enqueueEvent/showEvents/processEvents;
    EventProcessor.cpp apply handles three distinct effects in FIFO order.
F7  Simulation.cpp transact/undo; World.cpp copy constructor/assignment; custom
    stack stores entire independent worlds including logs, pending events,
    turn, next ID, habitat integrity, and outcome. Repeated undo is LIFO.
F8  CustomStack.h and CustomQueue.h implement linked nodes and ownership. No
    prohibited standard stack/queue/deque/priority_queue/list container is used.
F9  ConsoleUI.cpp run/readInt/readSettings: complete menu, line-based validation,
    safe EOF, inspection, readable reports, unknown-ID and range checks.
F10 Entity virtual destructor; unique_ptr entity ownership; Rule of Three for
    raw log/node memory; event references are IDs; copies use allocate-then-
    commit; Simulation stages mutations before committing them and their undo.
F11 World.cpp evaluateOutcome and ConsoleUI option 15: survive 20 turns with
    positive habitat integrity and at least one active colonist. Reaching zero
    habitat or having no active colonists loses. Undo reverses an outcome.

BUILD AND RUN (C++17, standard library only)
  g++ -std=c++17 -Wall -Wextra -pedantic *.cpp -o simworld
  ./simworld                  (Windows: .\simworld.exe)
On Windows with Visual Studio C++ tools, build.ps1 also builds and runs tests.
See README.md for commands and a short demonstration sequence.

ASSUMPTIONS AND LIMITATIONS
- No mutable global variables. IDs are unique within the current world; undo
  restores the allocator as well, so an undone creation's ID can be reused.
- Targets are selected by lowest health fraction / fewest medical supplies,
  with collection order breaking ties. Pending treatments may overlap; later
  healing is capped at the target maximum. Event handlers generate no events.
- Accepted support requests pay immediately. A later deleted/inactive target
  discards the event without refund. Deleting a source does not cancel it.
- Inactive entities remain inspectable and can be edited or deleted. They do
  not act or receive events. A collapsed habitat cannot be repaired.
- Each successful mutation, including Run N as a whole, is one undo step.
  Failed/read-only commands add none. Whole-world snapshots favor simplicity
  over memory efficiency; this is intended for modest classroom simulations.
- The mission outcome remains final until undo. Afterward, inspection and
  entity editing remain available, but turns/events require undoing the end.
- There is no disk save/load or redo. Bounds are documented in the menu.

AI USAGE LOG (update with the team's actual use before submission)
Tool: OpenAI Codex.
1. Request: design a Space Colony architecture from the assignment PDF, map
   F1-F11, explain memory/copying/polymorphism/undo; do not implement yet.
   Used: the proposed hierarchy, capabilities, three events, and snapshot undo.
   User response: approved the architecture.
2. Request/context: approval to proceed with the proposed implementation.
   Used: Codex-generated source, design comments, build helper, and regression
   tests. Implementation refinements include composed RoleResources, staged
   command commits, and capability casts in Simulation's command handlers.
3. Request: investigate the VS Code missing-main warning and fix program issues.
   Used: diagnosis of the repository with no initial commit, preservation of
   destination IDs during entity assignment, and regression coverage for all
   four roles including queued event targets.
4. Request: validate Milestone 1 under the GitHub version-control protocol,
   including the required GCC build and tests before commit/push approval.
   Used: a Linux/WSL GCC build script, shell-script line-ending settings, and
   build instructions. Milestone results are recorded in MILESTONE_1.md.
5. Request: complete the inheritance/entity milestone with validated attributes,
   constructors, deep copying, polymorphic cloning, and a focused test; then stop.
   Used: explicit out-of-line copy/destructor definitions, rejection of control
   characters in entity names, and tests/EntityTests.cpp in both build helpers.
   Student modifications/review: TODO - record what you changed and verified.
Both team members must review and be able to explain the submitted program.
The assignment requires the AI chat thread to accompany the source submission.
*/

#include "ConsoleUI.h"
#include <exception>
#include <iostream>

int main() {
    try {
        ConsoleUI console(std::cin, std::cout);
        console.run();
    } catch (const std::exception& error) {
        std::cerr << "Unable to continue: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
