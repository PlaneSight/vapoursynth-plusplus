# Examples

This directory is an independent Meson consumer of the headers in `../include`.
It builds one plugin containing the example filters and verifies the exported
VapourSynth registration contract without requiring a VapourSynth runtime.

From the repository root:

```bash
uv sync --group build --locked
uv run --group build meson setup build-examples examples
uv run --group build meson compile -C build-examples
uv run --group build meson test -C build-examples --print-errorlogs
```

If the SDK does not provide pkg-config metadata, pass its header directory:

```bash
uv run --group build meson setup build-examples examples \
    -Dvapoursynth_include_dir=/path/to/vapoursynth/include
```

## Runtime smoke test

The registration test proves that the library is loadable and that every
filter publishes a valid descriptor. When a VapourSynth Python environment is
also available, `scripts/smoke.vpy` executes the `Palette` source filter and
checks the resulting frame:

```bash
VAPOURSYNTH_PLUSPLUS_EXAMPLE=build-examples/libvapoursynth-plusplus-example.so \
    vspipe --info examples/scripts/smoke.vpy -
```

Adjust the module suffix for the host platform. `src/` contains the filter
implementations, `tests/` owns build-time contract checks, and `scripts/`
contains demonstrations that require a complete VapourSynth runtime.
