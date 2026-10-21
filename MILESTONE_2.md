# Milestone 2: World and entity management

Implementation and testing are complete. The user has approved committing
this milestone unsigned and pushing it, with Git signing settings unchanged.

World now exposes createDefault, create, modify and inspect alongside its
existing listEntities, erase and duplicate operations. Simulation and the
console use these operations while retaining the existing undo transactions
and edit-history messages. The factory, entity hierarchy and cloning code
are reused without rewriting them.

All four concrete types live in one vector of unique_ptr<Entity>. World alone
assigns IDs. Successful insertion advances the allocator; deletion does not
reuse IDs. Snapshot copies preserve IDs and the allocator for existing undo
behavior. Duplicate clones the dynamic type and complete state, then changes
only its ID. Each clone owns independent history storage.

Validation performed with Ubuntu/WSL GCC 15.2.0:

- `bash build.sh`: complete application and tests built with C++17, -Wall,
  -Wextra, -Werror and -pedantic; no compilation errors or warnings.
- Four entity test groups and seven regression groups passed.
- `bash build.sh --sanitize`: the same tests passed with address,
  undefined-behavior and leak checks; no diagnostics.
- Reviewed git status, git diff and whitespace checks. Build outputs remain
  ignored; no executables or unrelated files are included in the changes.

New regression coverage checks default/custom creation for every type, mixed
storage, unique IDs after deletion, full clone state and independent storage,
editing/deleting a duplicate without affecting its original, deleting the
original without affecting its duplicate, listing/inspection, and rejection
of missing IDs, null entities, invalid types and invalid attributes without
changing the world or consuming an ID.

AI assistance: OpenAI Codex added the World management entry points, integrated
their existing callers and added the regression coverage at the user's request.
