# Contributing

Thank you for helping build CourierMan. This project values fast native software,
offline-first data ownership, readable C++ architecture, and respectful collaboration.

## Development Rules

- Use C++26 style with RAII, smart pointers, `std::optional`, `std::expected`,
  ranges where practical, and clear ownership.
- On Windows, build and test with MSVC rather than a Clang-based toolchain.
- On Windows, pair MSVC with a Qt MSVC SDK; a MinGW Qt install will not link.
- Use xmake only.
- Keep UI work in Qt Widgets. Do not add QML.
- Keep layers separated:
  - `src/core` for paths, config, logging, errors, and cross-cutting services.
  - `src/backend` for application and persistence logic.
  - `src/network` for protocol engines.
  - `src/ui` for presentation and view state.
- Store user data with `QStandardPaths`; never hardcode user directories.
- Keep expensive work off the UI thread.
- Add tests for core, backend, network, and non-visual behavior.

## Pull Requests

1. Create a focused branch.
2. Run formatting, static analysis, build, and tests.
3. Update docs when behavior or setup changes.
4. Include screenshots or screen recordings for visible UI changes.
5. Explain user impact and risk in the PR body.

## Style

Use `.clang-format` and `.clang-tidy`. Prefer short, explicit functions and a small
number of well-owned classes over broad utility objects.
