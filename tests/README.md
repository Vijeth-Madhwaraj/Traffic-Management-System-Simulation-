# Automated tests

The suite includes Vijeth's signal tests, Aarya's vehicle, lane, scenario, integration and CLI
tests, and Vaibhav's road-network and movement tests. GoogleTest v1.15.2 is fetched on first
configure and registered with CTest.

## Linux

```sh
cmake -S . -B build -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
ctest --test-dir build -R 'UT_A|Scenario|cli_' --output-on-failure
```

Static analysis (Clang required) also builds the project with warnings as errors:

```sh
cmake -S . -B build-analysis -DBUILD_TESTING=OFF -DTRAFFIC_ENABLE_ANALYZER=ON
cmake --build build-analysis --parallel 2
```

For offline testing with a previously downloaded GoogleTest source tree, configure with
`-DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/path/to/googletest`. Disabling `BUILD_TESTING` builds
only the application and does not establish test results.

See [Aarya's matrix](../docs/aarya-test-matrix.md) for the recorded input-module run and
remaining integration/NFR work. Explicit-route movement has unit coverage; automatic routing,
full lane-capacity, incident and trip-metric acceptance remain in progress.

## Windows PowerShell (local MinGW toolchain)

Run these commands from the repository root. The generator and compiler match
the project's current Windows environment (MinGW GCC). GoogleTest is fetched
the first time testing is configured.

```powershell
$cmake = 'C:\Program Files\CMake\bin\cmake.exe'
$ctest = 'C:\Program Files\CMake\bin\ctest.exe'
$gxx = 'C:\msys64\ucrt64\bin\g++.exe'

& $cmake -S . -B build -G 'MinGW Makefiles' `
  -DCMAKE_CXX_COMPILER=$gxx -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed' }

& $cmake --build build --parallel
if ($LASTEXITCODE -ne 0) { throw 'Build failed' }

& $ctest --test-dir build --output-on-failure
```

List tests or run a single planned test ID:

```powershell
& $ctest --test-dir build -N
& $ctest --test-dir build -R 'UT_VI04' --output-on-failure
```

For Release, use a separate build folder:

```powershell
& $cmake -S . -B build-release -G 'MinGW Makefiles' `
  -DCMAKE_CXX_COMPILER=$gxx -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Release
& $cmake --build build-release --parallel
& $ctest --test-dir build-release --output-on-failure
```

To clean only generated build output, run this from the repository root:

```powershell
if (Test-Path -LiteralPath .\build) {
  Remove-Item -LiteralPath .\build -Recurse -Force
}
```

The `windows-latest` GitHub Actions job runs configure, build, and all CTest cases for
pushes, pull requests, and manual dispatches. View results in the repository's
Actions tab. The required `Basic CI Check` aggregate passes only when Windows tests,
Linux tests, and static analysis all succeed; each job has a unique display name.

## Vijeth ownership status

See [`../docs/vijeth-test-matrix.md`](../docs/vijeth-test-matrix.md) for the
UT-VI01 to UT-VI10 status. Tests are added only where the current interface
supports real assertions. FR-07 adaptation and FR-08 emergency priority remain
blocked on implementation/API work; they are not represented as passing tests.


Linux unit/integration/CLI and Clang static-analysis jobs are configured alongside Windows.
Remote CI results are pending until the branch is pushed; local Linux success is recorded
separately. The existing signal matrix describes signal-specific evidence only.


PR #69 integration (31 October 2026): 63/63 CTest cases passed locally on Linux,
including management and explicit-route movement tests. `cli_lane_status` verifies
both a draining demo and the slow-road fixture in `tests/fixtures/blocked_entry_scenario.txt`.
Remote workflow results are recorded separately after pushing the integration commit.
