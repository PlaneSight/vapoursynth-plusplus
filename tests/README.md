# Tests

All verification code lives under this root and is grouped by the contract it
owns:

- `library/` checks the public header adapters through the root Meson project.
- `examples/` checks the separately configured example plugin at registration
  time and, when a VapourSynth runtime is available, at frame-request time.

Run the library and example suites from the repository root:

```bash
uv run --group build meson test -C build-library --print-errorlogs
uv run --group build meson test -C build-examples --print-errorlogs
```

CI runs the runtime check on CPython 3.14 against the pinned VapourSynth R78
wheel. The other jobs intentionally compile against current SDK headers so the
two compatibility boundaries remain independent. To reproduce the released
runtime check locally on a supported platform:

```bash
uv sync --group build --group runtime-test --locked
uv run --group runtime-test vapoursynth config

include_dir="$(uv run --group runtime-test python -c \
    'import vapoursynth as vs; print(vs.get_include())')"
uv run --group build meson setup build-runtime examples \
    -Dvapoursynth_include_dir="$include_dir"
uv run --group build meson compile -C build-runtime
uv run --group build meson test -C build-runtime --print-errorlogs

VAPOURSYNTH_PLUSPLUS_EXAMPLE=build-runtime/libvapoursynth-plusplus-example.so \
    uv run --group runtime-test vspipe --info tests/examples/runtime.vpy -

uv run --group runtime-test python -m tools.example_catalog \
    --plugin build-runtime/libvapoursynth-plusplus-example.so \
    --check
```

The second command requests frames from every example and compares the rendered
pixels with the documentation assets. Adjust the plugin path and module suffix
for the host platform.
