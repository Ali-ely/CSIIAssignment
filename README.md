# SimWorld: Space Colony

A C++17 console simulation for CSCE 1101. Engineers maintain the habitat, medics treat colonists, maintenance bots repair robots, and medical drones deliver medicine. The colony must survive 20 turns.

## Build and run

From this directory with GCC:

~~~sh
g++ -std=c++17 -Wall -Wextra -pedantic *.cpp -o simworld
./simworld
~~~

For a repeatable GCC build of the application and all regression tests on Linux
or Ubuntu/WSL, use:

~~~sh
bash build.sh
bash build.sh --sanitize
~~~

Both builds use `g++ -std=c++17 -Wall` plus stricter warnings treated as errors.
The sanitizer build also checks address errors, undefined behavior, and leaks.
Outputs stay in the ignored `build/gcc` and `build/gcc-sanitized` directories.
From PowerShell in this project directory, run
`wsl -d Ubuntu --exec bash ./build.sh`. The GCC-built Linux application runs with
`wsl -d Ubuntu --exec ./build/gcc/simworld`.

On Windows, the build helper finds GCC or the installed Visual Studio C++ tools, treats warnings as errors, builds the application and regression tests, and runs the tests:

~~~powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\build.ps1
.\build\simworld.exe
~~~

For an AddressSanitizer build, run the helper with -Sanitize. GCC also enables UndefinedBehaviorSanitizer. Sanitizer executables are placed in build/asan; the helper runs the tests with the compiler runtime on PATH.

~~~powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\build.ps1 -Sanitize
~~~

To compile the regression executable separately in a POSIX shell:

~~~sh
g++ -std=c++17 -Wall -Wextra -pedantic -I. $(find . -maxdepth 1 -name '*.cpp' ! -name 'main.cpp') tests/SimWorldTests.cpp -o simworld_tests
./simworld_tests
~~~

The application and tests have separate main functions. Do not compile tests/SimWorldTests.cpp into the application executable. The application's source uses only the C++ standard library.

Both build helpers also compile and run `tests/EntityTests.cpp` as a separate
`entity_tests` executable. This focused milestone test covers all four concrete
types: default and parameterized constructors, valid/invalid attribute bounds,
copy construction, assignment, independent history storage, polymorphic cloning,
and deleting the original through an `Entity` pointer. For each concrete type,
the owned variable-sized array is the history in its `Entity` base subobject;
the allocation is exclusive to that object, including after copying.

## Try a short session

1. Choose 1 four times, creating kinds 1, 2, 3 and 4. Their IDs are 1 through 4.
2. Choose 5 and edit Engineer #1: name Vega, maximum health 100, current health 40, active 1, maximum stamina 100, current stamina 100, skill 5, supplies 10.
3. Choose 9: actor Medic #2, target Engineer #1. Choose 11 to inspect the queued treatment.
4. Choose 12 to run a turn. The engineer improves the habitat, the medic requests treatment, the maintenance bot charges if no robot needs repair, and the drone delivers medicine.
5. Choose 4 to inspect #1, then 14 to undo the entire turn. The previously queued treatment is restored along with all entity attributes.
6. Choose 6 to duplicate #1. Edit the copy, then inspect #1 to see that it is independent.
7. Delete either entity with 7 and undo with 14. Its complete history and attributes return.
8. Choose 15 for mission status/rules, 13 for several turns, or 0 to exit.

## Design and requirements

The authoritative design comments and F1-F11 implementation map are at the top of main.cpp. Class comments explain responsibilities and ownership.

~~~text
Entity (identity, health, exclusive dynamic history)
+-- Colonist (stamina and resting)
|   +-- Engineer : RepairCapability
|   +-- Medic : HealingCapability
+-- Robot (battery and charging)
    +-- MaintenanceBot : RepairCapability
    +-- MedicalDrone : HealingCapability
~~~

World owns one vector of unique entity pointers. Concrete virtual clone methods invoke copy constructors, including Entity's deep copy of its dynamic history. User duplication changes only the ID. Snapshot copying preserves IDs. Copy assignment into an existing entity preserves its destination ID while independently copying the source state, so registered IDs and queued event targets remain valid.

CustomQueue owns event nodes and processes them FIFO at the end of each turn. Event kinds are repair, medical treatment, and supply delivery. Events store target IDs; missing/inactive targets are discarded safely. Accepted capability requests cost 10 energy and one supply immediately, with no second charge when processed. Deleting a source does not cancel its accepted request.

Simulation stages each command in a complete world copy. A successful command moves the old world onto CustomStack and commits the candidate. Failed commands add no undo entry. Snapshots contain entities, independent logs, pending events in order, the turn, ID allocator, habitat integrity, and outcome. Run N is one undo action. Read-only operations do not consume history. Full snapshots use memory proportional to world size times retained undo depth.

Undo follows command order across turns: create, run three turns, then edit
requires three undos to reverse the edit, the entire three-turn run, then the
creation. Undo restores the whole snapshot, including concrete types, entity
order, dynamically allocated history, pending events and mission outcome. It
pops exactly one entry without recording another; an empty stack leaves the
world unchanged. There is no redo. A new command after undo starts from the
restored world and allocator, so an undone entity ID may be reused.

The menu validates complete input lines, IDs, attribute ranges, and target eligibility. Zero-health entities must be inactive. Health and energy maximums are 1-1000, skill is 1-10, and inventory is 0-10000. Unknown entity IDs and unsupported capabilities produce messages without changing state. EOF exits safely, including during a partially completed form.

## Mission rules and limitations

- Colony Survival uses habitat integrity as its stability metric (0-100). Check
  option 15 for stability, survival progress and rules; each turn also prints
  progress and mission outcome. Create an Engineer with option 1, add supporting
  roles, and use option 12 or 13 to advance one or multiple turns.
- Each repair, treatment or supply-delivery event with a positive actual effect
  restores 1 stability, capped at 100. This includes entity-generated requests,
  special capabilities (options 8/9), and manual assistance (option 10).
  Zero-effect and discarded requests earn no bonus. Once stability reaches zero,
  no action or event can revive the colony; option 14 can undo the losing run.
- Habitat starts at 100 and loses 6 each turn. Engineers restore 3 + skill/2 integrity at a cost of 12 stamina.
- Below 30 habitat integrity, active entities take 10 damage before acting. Zero health makes an entity inactive. A habitat that reaches zero cannot be repaired.
- Colonists without enough energy rest for up to 25 stamina; robots charge for up to 30 battery. These changes respect each entity's maximum.
- Medics target the active colonist with the lowest health fraction; bots target the active robot with the lowest health fraction. Drones target the active medic with the fewest supplies and deliver up to 1 + efficiency/3 doses for 8 battery. Ties use collection order.
- Events do not reserve targets; several requests may target the same entity. Effects are capped at health/inventory limits. Entity actions run before all queued effects.
- Win after turn 20 with positive habitat integrity and at least one active colonist. Lose at turn end if either condition fails. A batch stops immediately after an outcome. Undo can reverse the outcome.
- Inactive entities stay available for inspection, editing and deletion. Editing can reactivate a living entity. An ended mission must be undone before further turns or events.
- External user-entered events represent off-colony assistance and have no actor resource cost. This is an interactive sandbox, so editing and external requests can make survival easier.
- The initial colony is empty. There is no save/load, redo, random hazard system, or automatic resupply. IDs may be reused after undo restores an earlier allocator state.

## Verification and submission

Regression tests cover custom stack/queue copying and order; independent copies and assignment for all four entity kinds; exact multi-level undo; failed-command atomicity; all capability outcomes; three event effects; FIFO order; deleted targets; turn outcomes; input recovery; EOF; and repeated world creation/destruction.

Before submitting, replace the team-name/AUC-ID placeholders in main.cpp, review the AI usage log and record your actual changes, and include the AI chat thread as required by the assignment. Package the application's root .h/.cpp files in the prescribed team ZIP; omit executables and build directories. The tests and build helper are development aids, not additional application entry points.

## VS Code Git warning

The warning `No such branch: main` occurs when a new repository names `main` as its current branch but has no initial commit. A local initial commit establishes the branch reference; this is separate from compiling or running SimWorld. If VS Code still displays the old log entry after the commit, use **Developer: Reload Window** and check new Git output.

Run `git status --short --branch` and `git log -1 --oneline` to check the repository. Run the application in the terminal with `.\build\simworld.exe`; the Git output panel displays source-control messages.
