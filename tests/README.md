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

The runtime check is intentionally optional because the compile and
registration jobs require only SDK headers. With VapourSynth and `vspipe`
installed:

```bash
VAPOURSYNTH_PLUSPLUS_EXAMPLE=build-examples/libvapoursynth-plusplus-example.so \
    vspipe --info tests/examples/runtime.vpy -
```

Adjust the module suffix for the host platform.
