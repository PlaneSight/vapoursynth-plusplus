---
title: Contributing
description: Build, validate, and document changes to vapoursynth-plusplus.
---

# Contributing

Keep changes small enough to review and explicit enough that ownership,
scheduling, and format assumptions remain visible.

## Validate C++ behavior

```bash
uv sync --group build --locked

uv run --group build meson setup build-library
uv run --group build meson compile -C build-library
uv run --group build meson test -C build-library --print-errorlogs

uv run --group build meson setup build-examples examples
uv run --group build meson compile -C build-examples
uv run --group build meson test -C build-examples --print-errorlogs
```

Use `meson setup --reconfigure` for an existing directory after changing
options. Build directories are disposable and ignored; authored files must
never be generated into `include/`, `examples/`, `tests/`, or `docs/`.

## Validate documentation

```bash
uv sync --group docs --locked
uv run --group docs zensical serve
uv run --group docs zensical build --clean --strict
```

The strict build runs in `.github/workflows/docs.yml` before GitHub Pages is
deployed from `master`.

## Documentation expectations

- Start from the reader's task or decision.
- Keep signatures, format restrictions, scheduling, and ownership aligned with
  the headers and executable examples.
- Link reference pages to the source that defines the behavior.
- Give distinct public concepts focused pages instead of accumulating a manual.
- Treat `site/`, `.venv/`, and `build-*/` as generated local state.

## C++ expectations

- Reuse the existing wrapper types and conventions.
- Validate unsupported formats and dimensions at construction.
- Declare frame dependencies explicitly.
- Return owned frame wrappers from frame generators.
- Keep API calls at adapter boundaries and preserve the C++23 baseline.
