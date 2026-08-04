# vapoursynth-plusplus

A C++23, header-oriented layer for authoring VapourSynth API 4 filters with
typed maps, RAII resource ownership, compile-time signature recognition, and a
two-phase frame-acquisition model.

The public C++ patterns remain independent of the VapourSynth ABI. The API 4
adapter is concentrated at the handle, map, format, and plugin callback
boundaries.

## Requirements

- VapourSynth API 4.2 or newer
- A C++23 compiler
- [uv](https://docs.astral.sh/uv/)

uv installs the pinned Meson and Ninja versions used by CI. The VapourSynth SDK
can be discovered through pkg-config or supplied as a header directory.

## Build and test

```bash
uv sync --group build --locked
uv run --group build meson setup build-library
uv run --group build meson compile -C build-library
uv run --group build meson test -C build-library --print-errorlogs
```

Build the independently configured example plugin:

```bash
uv run --group build meson setup build-examples examples
uv run --group build meson compile -C build-examples
uv run --group build meson test -C build-examples --print-errorlogs
```

Pass `-Dvapoursynth_include_dir=/path/to/vapoursynth/include` to either setup
command when pkg-config metadata is unavailable.

The [documentation](https://planesight.github.io/vapoursynth-plusplus/) covers
installation, filter lifecycle, scheduling, ownership, the public adapters,
and the example catalogue.
