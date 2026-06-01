# Development

## Requirements

- MSVC on Windows, or a C++26-or-later compiler on macOS/Linux.
- xmake.
- Qt 6.10 or newer with Widgets, Network, SQL, Concurrent, and Test modules.
- SQLite Qt driver.

On Windows, xmake auto-detects the Qt SDK from `QT_ROOT_DIR` or the standard
`C:\Qt\6.10.3\msvc2022_64` install path.

## Configure

### xmake

```powershell
xmake f -y -m debug
```

For release:

```powershell
xmake f -y -m release
```

## Build And Run

### xmake

```powershell
xmake -y
xmake run CourierMan
```

## Tests

### xmake

```powershell
xmake build courierman_tests
xmake run courierman_tests
```

## Platform Notes

The GitHub Actions pipeline builds native Windows, macOS, and Linux packages with
xmake. Cross-compilation is possible when a matching Qt SDK is installed for the
target architecture; do not mix x64 Qt libraries with ARM64 builds.
