# Milestone 1: Inheritance and concrete entity classes

Review date: October 21, 2026.

Scope: complete and verify the approved three-level entity hierarchy, validated
attributes, constructors, deep copying, destruction, and polymorphic cloning.
The hierarchy was already present from the earlier implementation; this pass
makes copy/destructor definitions explicit in the source files and adds focused
constructor/copy tests. Implementation and verification are complete, and the
user has approved committing and pushing this milestone. Work stops here.

## Changes for review

- Defined copy constructors, copy assignment, and virtual destructors out of line
  for `Colonist`, `Robot`, `Engineer`, `Medic`, `MaintenanceBot`, and `MedicalDrone`.
  Their defaulted implementations delegate dynamic-history ownership to `Entity`.
- Documented that each concrete object's base subobject owns its own variable-sized
  history array. Copying allocates separate storage; role attributes copy by value.
- Strengthened name validation to reject all control characters, including embedded
  null bytes, while preserving numeric bounds and validation before mutation.
- Added `tests/EntityTests.cpp`, a separate focused executable for all four roles.
- Updated both build helpers to compile and run the focused entity tests and the
  existing regression suite. The GCC helper also builds the full application.
- Added an optional sanitizer build with address, undefined-behavior, and leak
  checks. All generated files remain under the ignored `build/` directory.
- Added `.gitattributes` to preserve LF endings for shell scripts on Windows.
- Updated the README's GCC/WSL instructions and the AI usage log in `main.cpp`.
- Existing turn, event, and undo behavior is covered by regression checks; this
  milestone adds no new simulation features.

GCC was installed in Ubuntu/WSL because it was unavailable in the original
environment. Compiler packages are environment dependencies, not repository files.

## Validation

Compiler: `g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0`.

The full application and both test executables compiled with:

```text
-std=c++17 -Wall -Wextra -Werror -pedantic -g
```

Commands executed from the project directory:

```powershell
wsl -d Ubuntu --exec bash ./build.sh
wsl -d Ubuntu --exec bash ./build.sh --sanitize
```

Both GCC runs passed the focused entity tests for Engineer, Medic, MaintenanceBot,
and MedicalDrone. For each type these check sensible defaults, all parameterized
attributes, valid and invalid bounds, rejected edits leaving state unchanged,
copy construction, assignment, self-assignment, independent dynamic arrays,
polymorphic clone type/state preservation, and original deletion through a base
pointer. Compile-time checks also verify abstract bases and virtual destruction.

Both runs also passed all six existing regression groups:

| Group | Result |
|---|---|
| Custom stack and queue, including copies and FIFO/LIFO order | Pass |
| Deep copying and assignment, including preservation of entity IDs | Pass |
| Exact multi-level undo and failed-command atomicity | Pass |
| Capabilities, polymorphic turns, and FIFO event effects | Pass |
| Attribute bounds, mission outcomes, and ownership stress | Pass |
| Console validation and end-of-input handling | Pass |

There were no compiler warnings or errors, and the sanitizer run reported no
address, undefined-behavior, or leak errors for these tests.

The Windows build helper also compiled the application and both test executables
using Visual C++ with `/W4 /WX`; all focused and regression tests passed there too.

## Repository review

- Current branch: `main`; baseline commit: `a45ed3e`.
- Remote: `https://github.com/Ali-ely/CSIIAssignment.git`.
- Reviewed `git status`, tracked diffs, and the newly added files.
- `git diff --check` passed.
- No unrelated files, credentials, executables, or generated build files were
  identified in the proposed changes. The known-secret-pattern scan found no
  matches. GCC build outputs are covered by `.gitignore`.
- The read-only remote branch check returned no remote branches at review time.
- At the review handoff, no files had been staged, committed, or pushed for this
  milestone. Commit and push were subsequently authorized by the user.

Proposed commit message:

```text
Milestone 1: Implement entity hierarchy and deep-copy semantics
```

The existing Git signing preference remains enabled. A future commit requires
the signing key to be available or explicit authorization for an unsigned commit.
The earlier authorization to make the initial commit unsigned applied only to
that one commit.

Team names/AUC IDs and the student's review notes remain intentional placeholders
in `main.cpp` and must be completed before assignment submission.
