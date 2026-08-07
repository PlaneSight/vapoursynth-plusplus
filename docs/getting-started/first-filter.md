---
title: Your first filter
description: Implement, register, and run a small typed VapourSynth filter.
---

# Your first filter

This guide implements a floating-point point filter that inverts every sample. It demonstrates
the smallest useful filter contract:

- one VideoNode input;
- a signature;
- constructor validation;
- metadata passthrough;
- a strict spatial dependency;
- typed read-only input and writable output frames.

Unlike a throwaway tutorial snippet, this filter is part of the repository's example plugin and
is compiled and registration-tested with the rest of the examples.

[View `Invert.hxx` on GitHub](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/examples/src/Invert.hxx)

## Implement the filter

The repository version lives at `examples/src/Invert.hxx`:

~~~cpp
#pragma once

#include "Core.vxx"

struct Invert {
    field(InputClip, VideoNode{});

public:
    static constexpr auto Signature = "clip: vnode";

public:
    Invert(auto Arguments) {
        InputClip = Arguments["clip"];
        if (!InputClip.WithConstantFormat() ||
            !InputClip.WithConstantDimensions() ||
            !InputClip.IsSinglePrecision()) {
            throw std::runtime_error{
                "only single precision floating point clips with constant "
                "format and dimensions are supported."
            };
        }
    }

    auto SpecifyMetadata() const {
        return InputClip.ExtractMetadata();
    }

    auto SpecifyDependencies() const {
        return std::array{
            InputClip.SpecifyDependency(rpStrictSpatial)
        };
    }

    auto GenerateFrame(auto Index, auto GeneratorContext, auto Core) {
        auto InputFrame =
            InputClip.AcquireFrame<const float>(Index, GeneratorContext);
        auto OutputFrame = Core.CreateBlankFrameFrom(InputFrame);

        for (auto Plane : Range{InputFrame.PlaneCount})
            for (auto y : Range{InputFrame[Plane].Height})
                for (auto x : Range{InputFrame[Plane].Width})
                    OutputFrame[Plane][y][x] =
                        1.0f - InputFrame[Plane][y][x];

        return OutputFrame;
    }
};
~~~

`AcquireFrame<const float>` makes the read-only intent explicit. `CreateBlankFrameFrom` preserves
the input format, dimensions, and frame properties while returning writable storage.

The constructor rejects clips that this implementation cannot process safely. A point-wise
operation could be extended to more formats, but that should be a deliberate change to both the
pixel access and validation contract.

## Register the filter

The example plugin includes the header in `examples/src/EntryPoint.cxx` and registers the type:

~~~cpp
#include "Invert.hxx"

// ...
PluginInstantiator::RegisterFilter<Invert>();
~~~

The existing `InstantiatePluginFrom(Main);` macro supplies the exported
`VapourSynthPluginInit2` entry point. `RegisterFilter` derives the function name and parameter
list from `Signature` and publishes a `clip:vnode;` return value by default.

## Compile and test

Build and run the independent example tests:

~~~bash
uv run --group build meson setup build-examples examples
uv run --group build meson compile -C build-examples
uv run --group build meson test -C build-examples --print-errorlogs
~~~

The registration test loads the built plugin and verifies that `Invert` is registered with the
expected `clip:vnode;` argument and return signatures. This keeps the tutorial synchronized with
the plugin that users can actually build.

## Run the exact example

Once the resulting plugin is discoverable by VapourSynth, run the same filter implemented above:

~~~python
import vapoursynth as vs

core = vs.core

clip = core.std.BlankClip(
    width=640,
    height=360,
    format=vs.RGBS,
    color=[0.2, 0.45, 0.8],
)
inverted = core.test.Invert(clip)
inverted.set_output()
~~~

For normalized floating-point samples, `Invert` maps every value `x` to `1 - x`. The source color
`[0.2, 0.45, 0.8]` therefore becomes `[0.8, 0.55, 0.2]`. The visible result is a direct consequence
of the C++ loop above rather than a separate illustrative implementation.

!!! tip "Dogfood the tutorial"
    `Invert.hxx` is compiled into the example plugin, and `tests/examples/registration.cxx` checks
    that the tutorial filter is exported. When this example changes, update the implementation,
    registration test, and this page together so the getting-started path cannot silently drift
    away from working repository code.

## What to try next

`Invert` only needs the current pixel. The next example, [GaussBlur](../examples/gauss-blur.md),
uses `Plane::View` to read a neighborhood around each pixel and shows the result with a generated
before/after pair.
