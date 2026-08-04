---
title: Installation
description: Build vapoursynth-plusplus and its tests with Meson.
---

# Installation

Build the project when you have a VapourSynth API 4 SDK discoverable by Meson, a C++23 compiler,
Meson, and Ninja.

## Requirements

- VapourSynth API 4.2 or newer
- A compiler with C++23 support
- Meson
- Ninja

The required API level is enforced by
[VapourSynthConfig.vxx](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/include/VapourSynthConfig.vxx):

~~~cpp
#define VS_USE_API_42
#include "VapourSynth4.h"

static_assert(VAPOURSYNTH_API_MAJOR == 4);
static_assert(VAPOURSYNTH_API_MINOR >= 2);
~~~

The SDK must also be discoverable as the Meson dependency named vapoursynth, because the project
uses dependency('vapoursynth') in meson.build.

## Configure and build

Clone the repository and configure a release build:

~~~bash
git clone https://github.com/PlaneSight/vapoursynth-plusplus.git
cd vapoursynth-plusplus
meson setup build
ninja -C build
~~~

The default build compiles the format-adapter test. Run it with:

~~~bash
meson test -C build
~~~

## Build the example plugin

The examples are compiled into a VapourSynth plugin when build_examples is enabled:

~~~bash
meson setup build -Dbuild_examples=true
ninja -C build
meson test -C build
~~~

The example plugin is installed under Meson's configured library directory when install_examples
is enabled:

~~~bash
meson setup build -Dbuild_examples=true -Dinstall_examples=true
ninja -C build
ninja -C build install
~~~

The plugin exports VapourSynthPluginInit2, the API 4 entry point. The registration test loads
that symbol and verifies the plugin descriptor and all nine registered functions.

## Use the headers

The public headers live in include/ and use the .vxx suffix. Include the entry-point or facility
you need from a target that also has the VapourSynth SDK include path:

~~~cpp
#include "PluginInstantiator.vxx"
#include "Core.vxx"
#include "VideoNode.vxx"
~~~

The Meson project installs the headers below the vsFilterScript include subdirectory and
generates a vsfilterscript pkg-config file. The project name is historical; the public
headers and examples are the current source of truth.

## Troubleshooting

| Symptom | Check |
| --- | --- |
| dependency('vapoursynth') is not found | Make the SDK's Meson or pkg-config metadata discoverable |
| API version static assertion fails | Install an API 4.2-or-newer SDK and check which header Meson selects |
| The example plugin is not built | Configure with -Dbuild_examples=true |
| Registration test is skipped on Windows | The test currently loads the shared library only on non-Windows builds |
