---
title: vapoursynth-plusplus
description: A C++23, header-oriented layer for authoring VapourSynth API 4 filters.
---

# vapoursynth-plusplus

vapoursynth-plusplus is a C++23, header-oriented layer for authoring VapourSynth API 4 filters.
It keeps the public C++ patterns independent of the VapourSynth ABI and concentrates the API
adapter at the handle, map, format, and plugin-callback boundaries.

The library is designed around a small, explicit filter contract:

1. Describe the filter arguments with a VapourSynth-style signature.
2. Validate arguments and capture the filter state in a C++ type.
3. Return output metadata.
4. Declare the input-frame dependencies.
5. Acquire and process frames in GenerateFrame.

!!! note "Project status"
    The current adapter targets VapourSynth API 4.2 or newer. The public surface is evolving,
    so treat the headers and examples in this repository as the authoritative reference.

## What the library provides

| Area | Facilities |
| --- | --- |
| Registration | PluginInstantiator, signature deduction, API 4 plugin entry point |
| Media objects | VideoNode, VideoFrame, AudioNode, AudioFrame |
| Resource safety | Reference-counted VapourSynth handles with ownership transfer |
| Frame scheduling | Explicit dependencies, two-phase acquisition, temporal frame requests |
| Pixel access | Typed planes, read-only border remapping, writable direct access |
| Core access | Format queries, frame allocation, plugin invocation, logging |
| API 4.2 extensions | Cache policy, node and core introspection, map extensions, plugin metadata, audio channel shuffling |
| Diagnostics | Exceptions translated into VapourSynth filter and map errors |

## Start here

- [Install the dependencies and build the tests](getting-started/installation.md)
- [Write a small typed filter](getting-started/first-filter.md)
- [Understand the filter lifecycle](concepts/filter-lifecycle.md)
- [Choose the correct frame-acquisition pattern](concepts/scheduling.md)
- [Browse the reference](reference/registration.md)
- [Use the API 4.2 extensions](reference/api42-extensions.md)
- [See how the included examples map to common filter designs](examples/catalog.md)

## Minimal shape of a filter

The following excerpt shows the core shape without committing to a particular pixel algorithm:

~~~cpp
struct Filter {
    field(InputClip, VideoNode{});

    static constexpr auto Signature = "clip: vnode";

    Filter(auto Arguments) : InputClip{Arguments["clip"]} {}

    auto SpecifyMetadata() const {
        return InputClip.ExtractMetadata();
    }

    auto SpecifyDependencies() const {
        return std::array{InputClip.SpecifyDependency(rpStrictSpatial)};
    }

    auto GenerateFrame(auto Index, auto GeneratorContext, auto Core) {
        auto InputFrame = InputClip.AcquireFrame<const float>(Index, GeneratorContext);
        auto OutputFrame = Core.CreateBlankFrameFrom(InputFrame);
        // Process InputFrame into OutputFrame.
        return OutputFrame;
    }
};
~~~

The type becomes a VapourSynth function when it is registered from the plugin entry point.
The [first-filter guide](getting-started/first-filter.md) expands this into a complete example.
