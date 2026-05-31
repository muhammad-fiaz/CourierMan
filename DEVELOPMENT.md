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

## Cross Compilation Notes

The root `xmake.lua` defines platform options for:

- Windows x64 and ARM64.
- macOS x86_64 and aarch64.
- Linux x86_64 and aarch64.

Use the appropriate Qt SDK for the target architecture and pass `--arch=` through
xmake when configuring.
