# Build and code quality evidence

11 November 2026. Source fingerprint: `33a9f36aaee0111f517a08e28aa33c58a4f17aef07aae7e86fbf231e664bf4e2`.

- Linux C++17 Debug configure/build and 115 CTest cases: pass.
- GCC coverage build (`--coverage -O0 -g`) and 115 CTest cases: pass.
- Pinned clang-format 18.1.8 dry-run: pass across project source and tests.
- Analyzer-enabled build with `-Wall -Wextra -Wpedantic -Werror`: pass.
- Clang static analyzer: 12 translation units, 0 diagnostics; each analyzer invocation fails on findings.
- System acceptance: 12/12 CLI/report/replay/benchmark checks pass.
- Traceability: 18/18 FR rows resolve to existing source and executed exact test names.
- Workflow YAML parses; all five required jobs feed the preserved Basic CI Check aggregate.
- Container shell script and Python tooling syntax checks pass.
- Current MP4 videos decode successfully and have exact 60/120 second durations.

Docker daemon was unavailable even outside the filesystem sandbox; no container run is
claimed. A Jenkins server/agent was not available. New remote CI is pending push. Human
usability review and approvals are tracked separately. Sanitizer execution is not claimed.
