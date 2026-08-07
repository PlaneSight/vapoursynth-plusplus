---
title: Examples
description: Build and verify the independent example plugin.
---

# Examples

The `examples/` directory is an independently configured Meson consumer of the
library. It owns the demonstration plugin and its build definition. All test
code lives under `tests/`, grouped into `library/` and `examples/` contracts.
The catalog starts with small video filters and then moves into examples that
combine frame properties, temporal requests, and audio frames.

## Build-time verification

```bash
uv sync --group build --locked
uv run --group build meson setup build-examples examples
uv run --group build meson compile -C build-examples
uv run --group build meson test -C build-examples --print-errorlogs
```

This path does not require an installed VapourSynth runtime. The test loads the
module directly and verifies:

- the `VapourSynthPluginInit2` export;
- the plugin identifier, namespace, and description;
- every registered function name, argument signature, and return signature.

## Runtime verification

With VapourSynth and `vspipe` installed, point the smoke script at the built
module:

```bash
VAPOURSYNTH_PLUSPLUS_EXAMPLE=build-examples/libvapoursynth-plusplus-example.so \
    vspipe --info tests/examples/runtime.vpy -
```

The script loads the plugin, creates a frame with `test.Palette`, requests
that frame, and checks its dimensions and sample value. Adjust the library
suffix for the host platform.

## Generate the visual catalog

The catalog images exercise every example filter against synthetic VapourSynth
clips. Generate the committed assets with:

```bash
uv run --group runtime-test python -m tools.example_catalog \
    --plugin build-examples/libvapoursynth-plusplus-example.so
```

Add `--check` to regenerate in memory and fail if the committed dimensions or
RGB pixels differ. CI runs that mode against VapourSynth R78, making the visual
documentation part of the runtime compatibility contract.

## Read the implementations

Start with [Palette](palette.md) for a source filter or
[GaussBlur](gauss-blur.md) for a single-input spatial filter. Continue with
[Property-driven gain](prop-gain.md) when you want frame properties to control
pixel work, [Temporal difference](temporal-difference.md) for a two-frame
request plan, or [Audio gain](audio-gain.md) for a complete audio filter.
The [example catalog](catalog.md) compares every filter by input contract and
scheduling model, then links to a guided code reading for each implementation.

!!! warning "Examples are contracts, not universal filters"
    Several filters deliberately restrict formats or dimensions. Widening
    support requires corresponding changes to validation, typed access,
    metadata, and boundary behavior.
