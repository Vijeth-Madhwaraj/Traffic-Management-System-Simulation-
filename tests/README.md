# Automated tests

The current automated suite is scoped to Vijeth's `SignalController` ownership
(FR-05 to FR-08). GoogleTest is pinned in CMake and discovered by CTest. The
initial configure downloads GoogleTest, so it needs GitHub access. Tests for
other owners are outside the current scope.

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

The `windows-latest` GitHub Actions job runs configure, build, and CTest for
pushes, pull requests, and manual dispatches. View results in the repository's
Actions tab; a branch protection rule can require the `Vijeth signal tests
(Windows)` check before merging.

## Vijeth ownership status

See [`../docs/vijeth-test-matrix.md`](../docs/vijeth-test-matrix.md) for the
UT-VI01 to UT-VI10 status. Tests are added only where the current interface
supports real assertions. FR-07 adaptation and FR-08 emergency priority remain
blocked on implementation/API work; they are not represented as passing tests.
