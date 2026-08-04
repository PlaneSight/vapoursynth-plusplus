---
title: Repository structure
description: Understand source ownership, dependency direction, and generated artifacts.
---

# Repository structure

The repository is one header-only library with two consumers: its focused
adapter tests and an independently configured example plugin. Verification has
one canonical root and is grouped by the contract under test.

```text
.
├── include/          Public C++23 headers
├── tests/
│   ├── library/      Public header adapter tests
│   └── examples/     Plugin registration and runtime tests
├── examples/
│   └── src/          Demonstration plugin and filters
├── docs/             Authored Zensical pages
├── tools/
│   └── example_catalog/  Reproducible documentation image generator
├── meson.build       Library build and install contract
├── pyproject.toml    uv tool groups
├── uv.lock           Pinned build and documentation tools
└── zensical.toml     Documentation navigation and rendering
```

## Dependency direction

`examples/` and `tests/` may include `include/`. Public headers never depend on
examples, tests, tools, documentation, or build output. The example project has
its own Meson entry point, while its verification sources remain in the single
test root. `tools/example_catalog/` consumes the built plugin and owns only the
reproducible documentation-image pipeline. Consumer-only assumptions cannot
leak into the library build.

## Generated state

| Path | Producer | Purpose | Version control | Cleanup |
| --- | --- | --- | --- | --- |
| `.venv/` | `uv sync` | Pinned tool environment | Ignored | Remove `.venv/` |
| `build-library/` | Root Meson setup | Library intermediates and tests | Ignored | Remove `build-library/` |
| `build-examples/` | Example Meson setup | Plugin, test executable, intermediates | Ignored | Remove `build-examples/` |
| `site/` | Zensical build | Rendered documentation | Ignored | Remove `site/` |

All four paths are reproducible from tracked inputs. CI configures fresh build
directories and runs the same uv-managed commands documented for contributors.
