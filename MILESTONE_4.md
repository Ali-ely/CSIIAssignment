# Milestone 4: Optional special capabilities

Complete and tested; the user approved committing and pushing this milestone.
The commit will be unsigned under the user's project-wide preference, with
Git signing settings unchanged.

The approved interface design and existing implementations were reused:

| Capability | Implementing entities | Menu operation |
| --- | --- | --- |
| RepairCapability | Engineer, MaintenanceBot | 8: Emergency repair |
| HealingCapability (medical assistance) | Medic, MedicalDrone | 9: Emergency treatment |

Both operations select an actor and target by ID. The menu now explains eligible
actors and targets before selection. Missing capabilities report the actor's
ID/type and suggest eligible roles, then return normally to the menu.

The two checked dynamic_cast expressions remain in Simulation::repair and
Simulation::heal, where runtime cross-casting to optional interfaces is needed.
Normal turns continue to call Entity::takeTurn virtually without casts or
concrete-type conditionals. Successful requests pay energy/supplies once and
queue the appropriate effect. Failed capability detection leaves state and undo
history unchanged.

Validation with Ubuntu/WSL GCC 15.2.0:

- `bash build.sh`: full application and tests compiled as C++17 with -Wall,
  -Wextra, -Werror and -pedantic, with no errors or warnings.
- All four entity test groups and nine regression groups passed.
- `bash build.sh --sanitize`: all groups passed with address,
  undefined-behavior and leak checks; no diagnostics.
- New tests exercise both commands against all four concrete actor types:
  four successful casts and four unsuccessful casts. They check resource costs,
  event kind/source/target/effect, undo, unchanged state on rejection, and
  continued operation after failure.
- A scripted console session attempts two unsupported capabilities, then uses
  both valid operations, inspects pending events and runs a normal turn.
- Git diff/status and whitespace checks reviewed; generated builds remain
  ignored and no unrelated files are included.

AI assistance: OpenAI Codex reviewed the existing capability implementations,
improved user-facing guidance and added focused coverage at the user's request.
