---
title: ModifyFrame callbacks
description: Invoke a VapourSynth function from a frame-generating filter.
---

# ModifyFrame callbacks

`ModifyFrame` delegates the output decision to a VapourSynth `Function`. It shows how a filter can acquire a frame, cross the callback boundary, and recover the returned frame reference.

[View `ModifyFrame.hxx` on GitHub](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/examples/src/ModifyFrame.hxx)

## Before and after

| Input frame | Frame returned by the callback |
| --- | --- |
| ![Color stripes before ModifyFrame](../assets/example-catalog/modify-frame-before.png) | ![Inverted color stripes returned by the callback](../assets/example-catalog/modify-frame-after.png) |

For this visual test, the Python evaluator copies the acquired frame and inverts every floating-point sample. The C++ filter does not perform that pixel operation; its responsibility is to pass `src` across the function boundary and publish the returned frame.

## Complete filter

```cpp
#pragma once
#include "Core.vxx"

struct ModifyFrame {
    field(InputClip, VideoNode{});
    field(Evaluator, Function{});

public:
    static constexpr auto Signature = "clip: vnode, evaluator: func";
    static constexpr auto ExecutionPolicyForFrameGenerator =
        ExecutionSchemes::ParallelResourceAcquisition;

public:
    ModifyFrame(auto Arguments) {
        InputClip = Arguments["clip"];
        Evaluator = Arguments["evaluator"];
        if (!InputClip.WithConstantFormat() ||
            !InputClip.WithConstantDimensions())
            throw std::runtime_error{
                "only clips with constant format and dimensions supported."
            };
    }
    auto SpecifyMetadata() {
        return InputClip.ExtractMetadata();
    }
    auto SpecifyDependencies() const {
        return std::array{
            InputClip.SpecifyDependency(rpStrictSpatial)
        };
    }
    auto GenerateFrame(auto Index, auto GeneratorContext, auto Core) {
        auto InputFrame = InputClip.AcquireFrame(Index, GeneratorContext);
        return static_cast<FrameReference>(Evaluator("src", InputFrame));
    }
};
```

## Understand the boundary

The signature's `func` parameter becomes a persistent `Function`. Frame generation acquires input frame *n*, calls that function with a map entry named `src`, and converts its result to a `FrameReference`.

Unlike the pixel-processing examples, this filter does not need a concrete sample type because it never reads sample storage. Its contract concerns frames and functions rather than pixels.

`ParallelResourceAcquisition` is explicit because the callback is another resource-producing operation inside frame generation. The execution policy is part of correctness when work can cross back into VapourSynth.

## Run it

```python
import vapoursynth as vs

core = vs.core
clip = core.std.BlankClip(format=vs.GRAY8)

def annotate(src: vs.VideoFrame) -> vs.VideoFrame:
    output = src.copy()
    output.props.WasModified = 1
    return output

modified = core.test.ModifyFrame(clip=clip, evaluator=annotate)
modified.set_output()
```

The wrapper supplies the acquired frame as the callback's `src` argument. The
C++ side then requires the callback result to be convertible to a
`FrameReference`.

!!! warning "Validate callback results"
    A reusable filter should document the callback's accepted keys, return shape, format and dimension requirements, and error propagation. Treat a callback as an untrusted API boundary even when it originates in the same script.
