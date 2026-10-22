# Milestone 6: Custom LIFO undo stack

Complete and thoroughly tested; the user approved committing and pushing this
milestone. The commit will be unsigned under the user's project-wide preference,
with Git signing settings unchanged.

The existing CustomStack<T> uses owned singly linked nodes with O(1) push/pop,
deep copy, move ownership, copy-and-swap assignment and iterative destruction.
No prohibited standard containers implement stack or queue behavior.

## Undo strategy

Each successful modifying command stores the complete preceding World on the
stack. Commands first run on an independent candidate world; failed commands
do not commit changes or add history. Creation, deletion, duplication, attribute
edits, manual events and capability requests all follow this same strategy.

Single-turn and Run N commands also save one snapshot each. Intervening turns
are undone in command order: create, Run 3, edit requires three undos to reverse
the edit, all three turns together, then creation. A snapshot includes exact
concrete types, IDs, collection order, all attributes, independent dynamic
histories, the FIFO event queue, turn counter, next ID, habitat and outcome.

Undo directly pops the stack into the live world; it never calls the transaction
handler and never creates another undo entry. Empty undo leaves everything
unchanged. The report now displays the number of entries remaining. There is
no redo; new commands after undo start from the restored state and ID allocator.
An ID from an undone creation can consequently be reused.

## Validation

- Ubuntu/WSL GCC 15.2.0: `bash build.sh` built the complete application and
  tests with C++17, -Wall, -Wextra, -Werror and -pedantic, without warnings/errors.
- Four entity test groups and eleven regression groups passed.
- `bash build.sh --sanitize`: all groups passed with address, undefined-behavior
  and leak checks; no diagnostics.
- New tests create and edit every role, grow each dynamic history, enqueue
  events, run turns, duplicate/edit/delete all original roles, and undo every
  command against an independent expected World. Comparisons include complete
  state, concrete types, independently allocated histories and undo depth.
- Tests cover empty/exhausted undo, failed-command atomicity, a new command
  after undo, and ID allocator restoration. Existing tests also cover undo of
  victory/defeat and a whole multi-turn batch as one entry.
- A 100-snapshot custom-stack test checks deep copying, assignment, move
  replacement, clearing, exact LIFO order and harmless empty pop.
- Reviewed git diff/status and whitespace checks. No prohibited standard
  containers or generated build files are included.

AI assistance: OpenAI Codex reviewed and reused the existing stack/snapshot
implementation, documented the strategy, clarified undo reporting and added
the focused restoration and ownership tests.
