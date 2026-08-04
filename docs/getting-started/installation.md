---
title: Installation
description: Create the pinned tool environment and build vapoursynth-plusplus.
---

# Installation

The repository uses uv as the single entry point for build and documentation
tools. uv installs the exact Meson and Ninja versions recorded in `uv.lock`;
your compiler and VapourSynth SDK remain native system dependencies.

## Requirements

- VapourSynth API 4.2 or newer
- A compiler with C++23 support
- [uv](https://docs.astral.sh/uv/)

The required API level is enforced by
[`VapourSynthConfig.vxx`](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/include/VapourSynthConfig.vxx):

```cpp
#define VS_USE_API_42
#include "VapourSynth4.h"

static_assert(VAPOURSYNTH_API_MAJOR == 4);
static_assert(VAPOURSYNTH_API_MINOR >= 2);
```

## Build the library tests

```bash
git clone https://github.com/PlaneSight/vapoursynth-plusplus.git
cd vapoursynth-plusplus

uv sync --group build --locked
uv run --group build meson setup build-library
uv run --group build meson compile -C build-library
uv run --group build meson test -C build-library --print-errorlogs
```

Meson normally discovers the VapourSynth SDK through pkg-config. If you have
only the SDK headers, configure their location explicitly:

```bash
uv run --group build meson setup build-library \
    -Dvapoursynth_include_dir=/path/to/vapoursynth/include
```

## Build the example plugin

The example plugin is a separate Meson project. Configuring it independently
proves that the public headers work from a consumer boundary:

```bash
uv run --group build meson setup build-examples examples
uv run --group build meson compile -C build-examples
uv run --group build meson test -C build-examples --print-errorlogs
```

The test loads the produced module, calls `VapourSynthPluginInit2`, and
validates the plugin descriptor and registered function contracts. See the
[examples guide](../examples/index.md) for the optional runtime smoke test.

## Install the headers

Use Meson's normal install command after configuring `build-library`:

```bash
uv run --group build meson install -C build-library
```

The headers are installed below `vsFilterScript` for compatibility with
existing consumers. The generated pkg-config package remains
`vsfilterscript`.

## Troubleshooting

| Symptom | Resolution |
| --- | --- |
| `Dependency "vapoursynth" not found` | Install the SDK pkg-config metadata or pass `vapoursynth_include_dir` |
| API version assertion fails | Check that the selected SDK is API 4.2 or newer |
| Meson or Ninja is missing | Prefix the command with `uv run --group build` |
| A setup directory has stale options | Run `meson setup --wipe` for that specific build directory |
