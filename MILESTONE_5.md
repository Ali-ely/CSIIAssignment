# Milestone 5: Custom FIFO queue and events

Complete and tested; the user approved committing and pushing this milestone.
The commit will be unsigned under the user's project-wide preference, with
Git signing settings unchanged.

The existing CustomQueue<T> owns singly linked nodes with head/tail pointers,
O(1) enqueue/dequeue, deep copying, move ownership, copy-and-swap assignment,
const traversal and iterative destruction. No prohibited standard queue, stack,
deque, list or priority_queue containers are used in the project.

The three event effects are robot integrity repair, biological health treatment,
and medical-supply inventory delivery. Entities generate these requests during
turns, and menu option 10 lets users enqueue any event type manually. Menu option
11 displays every pending event, including source, target, amount and description,
without dequeuing it.

The FIFO queue drains after all entities have acted and before mission outcome
evaluation. This milestone adds an explicit event-phase heading, pending count
and numbered processing progress. Previously pending requests precede requests
generated during the turn. Every accepted event is removed once, including
requests safely discarded because their target no longer exists or is inactive.

Validation with Ubuntu/WSL GCC 15.2.0:

- `bash build.sh`: full C++17 build with -Wall, -Wextra, -Werror and -pedantic;
  no compilation errors or warnings. Four entity groups and ten regression
  groups passed.
- `bash build.sh --sanitize`: all groups passed with address, undefined-behavior
  and leak checks; no diagnostics.
- New tests exercise a 1,000-node queue through copying, assignment, clearing,
  moving, draining and reuse; empty dequeue preserves the output argument.
- A mixed turn proves exact arrival order across three manual requests and
  three entity-generated requests, with distinct effects applied once.
- Complete repeated queue displays preserve all world state and undo depth;
  processing an empty queue neither repeats effects nor modifies state.
- A scripted console session manually enqueues an event, displays it twice,
  and resolves it during a turn.
- Reviewed git diff/status and whitespace checks. Source scan found no prohibited
  standard containers; generated build files remain ignored.

AI assistance: OpenAI Codex reviewed and reused the existing queue/event code,
clarified event-phase reporting, and added the focused regression tests.
