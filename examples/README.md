# Examples

This directory is an independently configured Meson consumer of the headers in
`../include`. It builds one plugin containing the example filters. The
repository's `../tests/examples/registration.cxx` verifies the exported
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
also available, `../tests/examples/runtime.vpy` executes the `Palette` source
filter and checks the resulting frame:

```bash
VAPOURSYNTH_PLUSPLUS_EXAMPLE=build-examples/libvapoursynth-plusplus-example.so \
    vspipe --info tests/examples/runtime.vpy -
```

Adjust the module suffix for the host platform. `src/` contains only the
example plugin and filter implementations; all verification code has one
canonical home under `../tests/`.

## Visual catalog

The repository-level generator dogfoods all nine filters and writes the images
used by the documentation catalog:

```bash
uv run --group runtime-test python -m tools.example_catalog \
    --plugin build-examples/libvapoursynth-plusplus-example.so
```

Run it with `--check` to verify the committed pixels without rewriting files.
The generator lives under `tools/` because it produces documentation artifacts;
it is not plugin source or a second test root.
