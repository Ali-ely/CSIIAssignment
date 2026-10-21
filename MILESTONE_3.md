# Milestone 3: Turn-based simulation

Complete and tested; the user has reviewed the milestone and approved
committing and pushing it.

The existing single-turn and multi-turn commands, role actions and ownership
model were retained. World now labels each acting entity by role, ID and name,
reports its energy/supply changes, and explicitly reports skipped inactive
entities. Event effects and end-of-turn habitat status remain separate reports.

The normal turn loop calls the virtual Entity::takeTurn method through the
shared collection. It contains no casts or concrete-type conditionals. Each
concrete role implements its own action: Engineer strengthens the habitat,
Medic queues treatment, MaintenanceBot queues repairs, and MedicalDrone sends
medical supplies. Exhausted colonists rest and robots recharge before resuming
work on a later turn.

Critical habitat damage can deactivate entities before their action. Inactive
entities stay available for inspection and explicit deletion. World::erase
releases owned entities through unique_ptr and the virtual destructor. Pending
events store IDs rather than object pointers; deleted/inactive targets are
checked and discarded safely.

Validation with Ubuntu/WSL GCC 15.2.0:

- `bash build.sh`: full C++17 application and tests compiled with -Wall,
  -Wextra, -Werror and -pedantic, without errors or warnings.
- All four entity groups and eight regression groups passed.
- `bash build.sh --sanitize`: all tests passed with address, undefined-behavior
  and leak checking; no sanitizer diagnostics.
- New tests compare three single turns with one three-turn command; verify
  per-role resource costs, healing, repairs and delivery; check exhaustion and
  recovery; keep all four inactive types unchanged over consecutive turns;
  exercise deactivation before dispatch on turn 12; and delete an entity with
  an outstanding event before advancing again.
- Reviewed git diff and status; whitespace checks passed. Build outputs remain
  ignored and no generated artifacts are included.

AI assistance: OpenAI Codex reviewed the existing turn implementation, improved
its reporting and added the focused regression tests at the user's request.
