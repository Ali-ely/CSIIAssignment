# Milestone 7: Menu-driven console interface

Complete and tested; the user approved committing and pushing this milestone.
The commit will be unsigned under the user's project-wide preference, with
Git signing settings unchanged.

All menu options remain available: default/custom creation, listing, detailed
inspection, modification, duplication, deletion, both capabilities, manual
events, queue inspection, single/multiple turns, undo, status/rules and exit.

Changes centralize complete-line reading, flush prompts before waiting for
input, display current health/energy bounds, and accept redirected CRLF input
under Linux. EOF at any prompt reports session end and aborts an unfinished
command. A failed stream exits instead of repeatedly retrying it. Invalid
numeric lines are discarded and re-prompted, preserving a usable input stream.
Names and every attribute retain existing validation; unknown IDs and invalid
operations return to the menu without committing changes.

## Manual terminal walkthrough

Ran the compiled application interactively using Ubuntu/WSL in a terminal:

- Rejected non-numeric, negative and out-of-range menu selections; inspected
  empty entity/event lists and attempted empty undo.
- Created all four default types and listed them.
- Created a custom Engineer while rejecting an invalid kind, blank name,
  invalid maximum/current health, active flag, energy, skill and supply values.
- Inspected its details, modified a bot, duplicated the custom Engineer,
  deleted the duplicate, undid deletion and inspected the restored duplicate.
- Rejected zero/missing IDs and unsupported repair/treatment actors, then
  successfully used both capabilities on suitable targets.
- Added all three manual event types, rejecting an invalid kind and negative
  amount, and inspected the complete pending queue.
- Ran one turn, rejected negative/zero turn counts, ran two more turns,
  inspected status/rules and exited normally with process exit code 0.

## Automated validation

- Ubuntu/WSL GCC 15.2.0: `bash build.sh` compiled the full C++17 application
  with -Wall, -Wextra, -Werror and -pedantic without warnings/errors.
- Four entity test groups and twelve regression groups passed.
- `bash build.sh --sanitize`: all groups passed with address, undefined-behavior
  and leak checking; no diagnostics.
- Added coverage for blank/malformed/overflowing numbers, every custom attribute
  boundary, missing IDs, negative counts, EOF at every creation field and other
  unfinished commands, CRLF input, zero-health creation and a bad input stream.
- Reviewed git diff/status and whitespace checks; build artifacts remain ignored.

AI assistance: OpenAI Codex improved the existing console input handling,
added regression coverage and performed the manual terminal walkthrough.
