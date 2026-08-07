---
title: Filter lifecycle
description: How a C++ filter type becomes a VapourSynth API 4 filter node.
---

# Filter lifecycle

PluginInstantiator::RegisterFilter turns a C++ type into a VapourSynth filter. The type stores
configuration and exposes a small set of convention-based operations that the adapter recognizes.

## The contract

| Stage | Recognized member | Responsibility |
| --- | --- | --- |
| Registration | Signature or SpecifySignature | Describe the VapourSynth function name and arguments |
| Construction | Filter(Arguments) or a supported variant | Read arguments and reject invalid configurations |
| Metadata | SpecifyMetadata() or SpecifyMetadata(Core) | Return output format, dimensions, frame rate, and frame count |
| Dependencies | SpecifyDependencies() | Declare which input frames the scheduler must make available |
| Generation | GenerateFrame(...) | Request or fetch inputs, process them, and return an owned frame |
| Optional policy | ExecutionPolicyForFrameGenerator or SpecifyExecutionPolicyForFrameGenerator() | Select the API scheduling mode |
| Optional scheduling | LinearAccess or SpecifyLinearAccess() | Mark a filter for linear frame access |
| Optional cache | CachePolicy or SpecifyCachePolicy() | Select cache mode and, with API 4.2, cache limits |

The adapter supports constructor and generator forms with or without CoreProxy, as appropriate
for the operation. Prefer the most explicit form that your filter needs.

## Signature

A filter signature uses VapourSynth-style parameter syntax:

~~~cpp
static constexpr auto Signature =
    "clip: vnode, [left, right, top, bottom]: int?";
~~~

The adapter uses the signature to derive:

- the registered function name;
- the parameter list passed to VapourSynth;
- the default return signature, clip:vnode;.

The Signature member must be available at compile time unless the filter supplies a compatible
SpecifySignature function.

## Construction and validation

The constructor receives an ArgumentList. Read required values directly and test optional
values with Exists():

~~~cpp
Crop(auto Arguments) {
    InputClip = Arguments["clip"];

    if (Arguments["left"].Exists())
        Left = Arguments["left"];

    if (Left < 0)
        throw std::runtime_error{"left cannot be negative"};
}
~~~

Exceptions are translated into a VapourSynth error during filter creation. Keep validation close
to construction so an invalid instance cannot reach frame generation.

## Metadata

Metadata is returned before frame generation. A filter that preserves its input stream can return
InputClip.ExtractMetadata(); a filter that changes shape or format must return the new values:

~~~cpp
auto SpecifyMetadata(auto Core) {
    auto Metadata = InputClip.ExtractMetadata();
    Metadata.Format = Core.Query(VideoFormats::RGBS);
    return Metadata;
}
~~~

The adapter converts the library's VideoInfo and VideoFormat representations to the API 4
structures when it creates the node.

## Frame generation

GenerateFrame returns an owned FrameReference or a derived frame type. The normal sequence is:

1. Acquire the required input frame(s).
2. Allocate or copy the output frame.
3. Read input planes and write output planes.
4. Return the output frame by value.

~~~cpp
auto GenerateFrame(auto Index, auto GeneratorContext, auto Core) {
    auto InputFrame = InputClip.AcquireFrame<const float>(Index, GeneratorContext);
    auto OutputFrame = Core.CreateBlankFrameFrom(InputFrame);
    // Transform InputFrame into OutputFrame.
    return OutputFrame;
}
~~~

Do not return a raw API pointer without transferring an owned reference. The frame wrappers make
that transfer explicit and release the handle when the wrapper leaves scope.
