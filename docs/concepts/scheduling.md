---
title: Scheduling and dependencies
description: Declare frame dependencies and use the two-phase acquisition model.
---

# Scheduling and dependencies

VapourSynth calls a filter's frame generator in two phases. The filter first requests its inputs;
after those inputs are ready, it is called again to fetch and process them. The library packages
this protocol behind AcquireFrame and AcquireFrames.

## Single-frame filters

For spatial filters, declare a strict spatial dependency and acquire the corresponding frame:

~~~cpp
auto SpecifyDependencies() const {
    return std::array{
        InputClip.SpecifyDependency(rpStrictSpatial)
    };
}

auto GenerateFrame(auto Index, auto GeneratorContext, auto Core) {
    auto InputFrame = InputClip.AcquireFrame<const float>(Index, GeneratorContext);
    // Process InputFrame.
    return Core.CreateBlankFrameFrom(InputFrame);
}
~~~

On the initial activation, AcquireFrame calls the API request function and raises an internal
FrameGenerator::ResourceAcquisitionInProgress sentinel. The adapter converts that sentinel into
the normal request-now, fetch-later return path. On arAllFramesReady, the same call fetches the
frame.

## Temporal filters

A temporal filter supplies the frame indexes it needs through FrameRequestor and usually uses
AcquireFrames:

~~~cpp
InputClip.FrameRequestor = [this](auto Index) {
    return Range{Index - Radius, Index + Radius + 1};
};

auto SpecifyDependencies() const {
    return std::array{
        InputClip.SpecifyDependency(rpNoFrameReuse)
    };
}

auto GenerateFrame(auto Index, auto GeneratorContext, auto Core) {
    auto Frames = InputClip.AcquireFrames<const float>(Index, GeneratorContext);
    // Frames[0], Frames[-1], and so on are relative to Index.
    return Process(Frames, Core);
}
~~~

VideoNode remaps out-of-range indexes through its OutOfBoundsRemapping function. The default
is reflection, which is useful for symmetric temporal windows. Change it only when the filter's
boundary behavior requires a different contract.

## Multiple input nodes

For several inputs, use Node::AcquireFrameGiven so the sample types and node sources remain paired:

~~~cpp
auto [BackgroundFrame, ForegroundFrame, MaskFrame] =
    Node::AcquireFrameGiven<const float>(Index, GeneratorContext)
        .From(Background, Foreground, Mask);
~~~

If the inputs use different sample types, specify one type per node:

~~~cpp
auto [VideoFrame, MaskFrame] =
    Node::AcquireFrameGiven<const float, const std::uint8_t>(
        Index, GeneratorContext)
        .From(Video, Mask);
~~~

## Execution policy

The default execution policy is fully parallel. A filter can opt into one of the library's
named policies:

| Policy | API mode | Typical use |
| --- | --- | --- |
| FullyParallel | fmParallel | Independent frame generation |
| ParallelResourceAcquisition | fmParallelRequests | Parallel work with coordinated resource requests |
| Concurrent | fmUnordered | Concurrent generation where order is not required |
| OneFrameAtATime | fmFrameState | Stateful generation |

For example:

~~~cpp
static constexpr auto ExecutionPolicyForFrameGenerator =
    ExecutionSchemes::ParallelResourceAcquisition;
~~~

Choose a policy only when the filter's state or acquisition behavior requires it. The policy is a
scheduling contract, not a substitute for declaring dependencies.
