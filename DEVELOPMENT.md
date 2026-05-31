# Development

## Requirements

- C++ compiler with C++26 or latest draft support.
- xmake.
- CMake 3.28 or newer.
- Ninja for the CMake presets.
- Qt 6.10 or newer with Widgets, Network, SQL, Concurrent, and Test modules.
- SQLite Qt driver.

On this Windows development host, LLVM/Clang and Qt are expected to be installed
globally. The xmake configuration prefers Clang and uses Qt from the system when
available.

## Configure

### xmake

```powershell
xmake f -m debug --toolchain=clang --qt=C:\Qt\6.11.1\mingw_64
```

For release:

```powershell
xmake f -m release --toolchain=clang --qt=C:\Qt\6.11.1\mingw_64
```

### CMake

```powershell
cmake --preset debug
```

## Build And Run

### xmake

```powershell
xmake
xmake run CourierMan
```

### CMake

```powershell
cmake --build --preset debug
.\build\cmake-debug\CourierMan.exe
```

## Tests

### xmake

```powershell
xmake build courierman_tests
xmake run courierman_tests
```

### CMake

```powershell
ctest --preset debug
```

## Cross Compilation Notes

The root `xmake.lua` defines platform options for:

- Windows x64 and ARM64.
- macOS x86_64 and aarch64.
- Linux x86_64 and aarch64.

Use the appropriate Qt SDK for the target architecture and pass `--arch=` through
xmake when configuring.
