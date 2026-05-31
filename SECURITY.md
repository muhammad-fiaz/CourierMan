# Security Policy

CourierMan handles API credentials, certificates, environments, scripts, and request
history. Security issues are treated as high priority.

## Supported Versions

Security fixes target the latest stable release and the main development branch.

## Reporting A Vulnerability

Email `contact@muhammadfiaz.com` with:

- Affected version or commit.
- Steps to reproduce.
- Impact and suggested severity.
- Any proof-of-concept material.

Please do not open public issues for exploitable vulnerabilities until a fix is ready.

## Security Principles

- Secrets stay local unless the user explicitly exports or syncs them.
- Sensitive settings use OS-native storage surfaces where available.
- Logs avoid storing secret values.
- Background tasks use bounded workers and explicit cancellation.
- Update downloads show progress and never silently install code.
