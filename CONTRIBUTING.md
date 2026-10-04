# Contributing

Contributions are welcome through issues and pull requests.

## Before contributing

1. Read the safety and design guidance in `docs/01-safety-and-design-decisions.md`.
2. Search existing issues and pull requests for related work.
3. Keep changes focused and document any hardware assumptions.

## Development

Firmware is built with PlatformIO from the `firmware/` directory. Copy
`firmware/include/fog_controller_config.example.h` to
`firmware/include/fog_controller_config.h` for local configuration; the
resulting file is intentionally ignored by Git.

Before opening a pull request, build the firmware and test affected hardware
paths where practical. Describe the tests performed and call out anything that
could not be tested.

By participating in this project, you agree to follow `CODE_OF_CONDUCT.md`.
