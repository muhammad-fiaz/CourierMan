<div align="center">
  <img src="resources/images/logo_full.png" alt="CourierMan logo" width="400" />

  <a href="https://muhammad-fiaz.github.io/CourierMan/"><img src="https://img.shields.io/badge/docs-muhammad--fiaz.github.io-blue" alt="Documentation"></a>
  <a href="https://isocpp.org/"><img src="https://img.shields.io/badge/C%2B%2B-26-blue.svg?logo=cplusplus" alt="C++26"></a>
  <a href="https://www.qt.io/"><img src="https://img.shields.io/badge/Qt-6.10%2B-41cd52.svg?logo=qt" alt="Qt 6"></a>
  <a href="https://xmake.io/"><img src="https://img.shields.io/badge/build-xmake-2f74c0.svg" alt="xmake"></a>
  <a href="https://cmake.org/"><img src="https://img.shields.io/badge/build-CMake-064f8c.svg?logo=cmake" alt="CMake"></a>
  <a href="https://github.com/muhammad-fiaz/CourierMan"><img src="https://img.shields.io/github/stars/muhammad-fiaz/CourierMan" alt="GitHub stars"></a>
  <a href="https://github.com/muhammad-fiaz/CourierMan/issues"><img src="https://img.shields.io/github/issues/muhammad-fiaz/CourierMan" alt="GitHub issues"></a>
  <a href="https://github.com/muhammad-fiaz/CourierMan/pulls"><img src="https://img.shields.io/github/issues-pr/muhammad-fiaz/CourierMan" alt="GitHub pull requests"></a>
  <a href="https://github.com/muhammad-fiaz/CourierMan"><img src="https://img.shields.io/github/last-commit/muhammad-fiaz/CourierMan" alt="GitHub last commit"></a>
  <a href="https://github.com/muhammad-fiaz/CourierMan"><img src="https://img.shields.io/github/license/muhammad-fiaz/CourierMan" alt="License"></a>
  <a href="https://github.com/muhammad-fiaz/CourierMan/actions/workflows/ci-build-test.yml"><img src="https://github.com/muhammad-fiaz/CourierMan/actions/workflows/ci-build-test.yml/badge.svg" alt="CI"></a>
  <img src="https://img.shields.io/badge/platforms-linux%20%7C%20windows%20%7C%20macos-blue" alt="Supported Platforms">
  <a href="https://github.com/muhammad-fiaz/CourierMan/actions/workflows/codeql.yml"><img src="https://github.com/muhammad-fiaz/CourierMan/actions/workflows/codeql.yml/badge.svg" alt="CodeQL"></a>
  <a href="https://github.com/muhammad-fiaz/CourierMan/actions/workflows/release-publish.yml"><img src="https://github.com/muhammad-fiaz/CourierMan/actions/workflows/release-publish.yml/badge.svg" alt="Release"></a>
  <a href="https://github.com/muhammad-fiaz/CourierMan/releases/latest"><img src="https://img.shields.io/github/v/release/muhammad-fiaz/CourierMan?label=Latest%20Release&style=flat-square" alt="Latest Release"></a>
  <a href="https://pay.muhammadfiaz.com"><img src="https://img.shields.io/badge/Sponsor-pay.muhammadfiaz.com-ff69b4?style=flat&logo=heart" alt="Sponsor"></a>
  <a href="https://github.com/sponsors/muhammad-fiaz"><img src="https://img.shields.io/badge/Sponsor-GitHub%20Sponsors-pink?style=social&logo=github" alt="GitHub Sponsors"></a>
  <a href="https://hits.sh/muhammad-fiaz/CourierMan/"><img src="https://hits.sh/muhammad-fiaz/CourierMan.svg?label=Visitors&extraCount=0&color=green" alt="Repo Visitors"></a>

  <p><em>A modern, native, offline-first API platform for desktop engineers.</em></p>

  <b><a href="https://muhammad-fiaz.github.io/CourierMan/">Website</a> |
  <a href="DEVELOPMENT.md">Development</a> |
  <a href="SECURITY.md">Security</a> |
  <a href="CONTRIBUTING.md">Contributing</a></b>
</div>

# CourierMan

CourierMan is a modern native desktop API testing application for engineers who want a
fast, offline-first, zero lock-in alternative to Postman, Insomnia, Bruno, and Hoppscotch.

The 1.0 roadmap combines the everyday API client workflows with capabilities that are
usually locked behind paid or enterprise plans: local workspaces, collections,
environments, secrets, protocol clients, scripting, runners, generated code snippets,
schema tooling, update checks, GitHub reporting, and AI-assisted documentation and tests.

## Current Foundation

This repository now contains the Phase 1 production foundation:

- xmake-based C++26/Qt 6 project configuration.
- CMake build parity for environments that need it.
- Native Qt Widgets application shell with a custom title bar and app menus.
- Offline-first configuration paths using `QStandardPaths`.
- TOML configuration loading and saving via tomlplusplus.
- Async rotating file logging via spdlog.
- SQLite database manager using Qt SQL with WAL mode and migrations.
- Async REST request service backed by Qt Network.
- System tray and configurable close/minimize-to-tray behavior.
- Cross-platform autostart integration surface.
- Full-page settings view inside the main window.
- Update checker popup with live progress for GitHub Releases downloads.
- GitHub ownership, funding, CodeQL, and release automation metadata.
- Feature catalog covering free and traditionally paid API-client capabilities.
- Docker, NSIS installer, GitHub Actions, and contribution/security docs.

## Product Scope

CourierMan is designed around these modules:

- REST, GraphQL, WebSocket, Socket.IO, gRPC, SSE, SOAP, MQTT, Raw TCP, and UDP.
- Collections, folders, history, environments, variables, and secret vaults.
- Auth helpers: API key, Bearer, Basic, OAuth 1/2, AWS, JWT, mTLS, and proxies.
- Request bodies: JSON, XML, form data, multipart, binary, GraphQL, and SOAP.
- Response viewers: pretty JSON/XML/HTML, images, PDFs, raw, hex, and diffs.
- Code generation for C, C++, C#, Java, Kotlin, Swift, Rust, Go, Python,
  JavaScript, TypeScript, Node.js, PHP, Ruby, Dart, Zig, Shell, Objective-C,
  Perl, R, Wget, HTTPie, and cURL.
- JavaScript pre-request and test scripting with `pm.test()` compatibility.
- Desktop runner and planned native headless CLI runner.
- AI providers: OpenAI, Gemini, Claude, Ollama, and llama.cpp.

## Build

### xmake

```powershell
xmake f -m debug --toolchain=clang --qt=C:\Qt\6.11.1\mingw_64
xmake
xmake run CourierMan
```

### CMake

```powershell
cmake -S . -B build/cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build build/cmake --config Debug
ctest --test-dir build/cmake --output-on-failure
```

The CI matrix builds Windows, macOS, and Linux targets with xmake and CMake, then keeps
release automation in GitHub Actions.

## Developer

CourierMan is authored and maintained by Muhammad Fiaz.

| Platform | Link |
| --- | --- |
| GitHub | [@muhammad-fiaz](https://github.com/muhammad-fiaz) |
| Website | [muhammadfiaz.com](https://muhammadfiaz.com) |
| Project Site | [CourierMan documentation](https://muhammad-fiaz.github.io/CourierMan/) |
| Sponsorship | [pay.muhammadfiaz.com](https://pay.muhammadfiaz.com) |
| GitHub Sponsors | [github.com/sponsors/muhammad-fiaz](https://github.com/sponsors/muhammad-fiaz) |
| Email | [contact@muhammadfiaz.com](mailto:contact@muhammadfiaz.com) |

## Repository Architecture

CourierMan uses clean layering:

- `src/core`: paths, config, logging, results, and shared cross-cutting helpers.
- `src/backend`: workspaces, persistence, auth, environments, snippets, scripting, and feature services.
- `src/network`: protocol engines.
- `src/ui`: Qt Widgets presentation, shell, settings, dialogs, and reusable widgets.

The UI owns no database connections and protocol clients do not write directly to UI
widgets. Services emit Qt signals or return value objects, keeping the main window as
composition rather than business logic.

## License

CourierMan is GPL v3 licensed. Copyright 2026 Muhammad Fiaz.
