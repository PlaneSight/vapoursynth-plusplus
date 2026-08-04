---
title: Contributing
description: Build, validate, and document changes to vapoursynth-plusplus.
---

# Contributing

Keep changes small enough to review and explicit enough that the ownership, scheduling, and
format assumptions are visible in the code.

## Validate a change

For C++ changes:

~~~bash
meson setup build -Dbuild_examples=true
ninja -C build
meson test -C build
~~~

For documentation changes:

~~~bash
uv sync
uv run zensical serve
uv run zensical build --clean --strict
~~~

The documentation build is also run by .github/workflows/docs.yml and deployed to GitHub Pages
from master.

## Documentation expectations

- Document the reader's task before listing implementation details.
- Keep signatures, format restrictions, scheduling policies, and ownership rules aligned with the
  headers and examples.
- Link reference pages to the source header or example that defines the behavior.
- Prefer a focused page for a concept or public type over a large undifferentiated manual.
- Treat site/ and local Python environments as generated files.

## C++ expectations

- Prefer the existing wrapper types and conventions before introducing a new abstraction.
- Validate unsupported formats and dimensions at construction.
- Declare frame dependencies explicitly.
- Return owned frame wrappers from frame generators.
- Keep API calls at the adapter boundary and preserve the project's C++23 baseline.
