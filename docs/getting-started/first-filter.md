---
title: Your first filter
description: Implement and register a small typed VapourSynth filter.
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

## Implement the filter

Create `examples/src/Invert.hxx`:

~~~cpp
#pragma once

#include "Core.vxx"

struct Invert {
    field(InputClip, VideoNode{});

    static constexpr auto Signature = "clip: vnode";

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

AcquireFrame<const float> makes the read-only intent explicit. CreateBlankFrameFrom preserves
the input format, dimensions, and frame properties while returning writable storage.

The constructor rejects clips that this implementation cannot process safely. A point-wise
operation could be extended to more formats, but that should be a deliberate change to both the
pixel access and validation contract.

## Register the filter

Include the header in `examples/src/EntryPoint.cxx` and register the type:

~~~cpp
#include "Invert.hxx"

auto Main() {
    auto Descriptor = PluginInfo{
        .Namespace = "test",
        .Identifier = "com.vsfilterscript.test",
        .Description = "Test filters for vsFilterScript"
    };

    PluginInstantiator::SpecifyConfigurations(Descriptor);
    PluginInstantiator::RegisterFilter<Invert>();
    // Register the other filters here.
}
~~~

The existing InstantiatePluginFrom(Main); macro supplies the exported
VapourSynthPluginInit2 entry point. RegisterFilter derives the function name and parameter
list from Signature and publishes a clip:vnode; return value by default.

## Compile and test

The example project discovers the header through `EntryPoint.cxx`. Update the
expected registration count in `tests/examples/registration.cxx`, then build
and run the independent example tests:

~~~bash
uv run --group build meson setup build-examples examples
uv run --group build meson compile -C build-examples
uv run --group build meson test -C build-examples --print-errorlogs
~~~

The registration test loads the built plugin and checks that each function
exposes a non-empty name, argument list, and `clip:vnode;` return signature.

## Use it from VapourSynth

Once the resulting plugin is discoverable by VapourSynth:

~~~python
import vapoursynth as vs

core = vs.core
clip = core.std.BlankClip(format=vs.RGBS)
clip = core.test.Invert(clip)
clip.set_output()
~~~

The example assumes the plugin is registered under the test namespace and that the input is
compatible with the filter's validation rules.
