# Milestone 8: Colony Survival

Complete and tested; the user approved committing and pushing this milestone.
The commit will be unsigned under the user's project-wide preference, with
Git signing settings unchanged.

Colony stability uses the existing habitat integrity value, preserving one
authoritative metric and compatibility with world snapshots. It starts at 100,
loses 6 per turn, and stays within 0-100. Engineers restore 3 + skill/2 when
they can work. Each effective repair, medical treatment or supply-delivery
event now restores 1 stability. This applies equally to requests from turns,
special capabilities and manual entry. Zero-effect and discarded events earn
no stability; assistance cannot revive a habitat that has reached zero.

Victory requires finishing 20 turns with positive stability and at least one
active colonist. Failure occurs if stability reaches zero or no active
colonists remain at turn end. Failure takes precedence on the target turn.
Below 30 stability, upkeep damages active entities by 10 before they act.
Turn batches stop on either outcome. Ended missions reject further turns and
new events; undo restores the preceding complete world, including the outcome.

The startup message, every turn summary and option 15 expose survival progress
and stability. Option 15 explains all rules. Users create Engineers/supporting
roles (options 1/2), use capabilities (8/9) or manual assistance (10), advance
turns (12/13), inspect status (15), and undo (14). README.md documents these
interactions and main.cpp's design overview describes event stability bonuses.

Validation with Ubuntu/WSL GCC 15.2.0:

- `bash build.sh`: full C++17 build with -Wall, -Wextra, -Werror and -pedantic;
  no compilation errors or warnings. Four entity and thirteen regression
  groups passed.
- `bash build.sh --sanitize`: all groups passed with address,
  undefined-behavior and leak checks; no diagnostics.
- New tests check no premature victory at turn 19, victory and batch stopping
  at turn 20, failure without active colonists (including at turn 20), collapse,
  effective/ineffective/discarded event bonuses, stability bounds, terminal
  command rejection, and exact undo before victory and collapse.
- A queued delivery still has its normal effect on the collapse turn but
  cannot revive zero stability. Console tests verify progress and rule display.
- Existing tests passed; two expected habitat values were updated to account
  for the new effective-event bonus.
- Reviewed git status/diff and whitespace checks; generated builds remain
  ignored and no unrelated files are included.

AI assistance: OpenAI Codex extended the existing survival model with event
influence, visible stability/progress, documentation and focused tests.
