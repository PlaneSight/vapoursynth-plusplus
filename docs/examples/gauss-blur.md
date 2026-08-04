---
title: GaussBlur neighborhood filter
description: Read a clamped 3x3 neighborhood through Plane::View.
---

# GaussBlur neighborhood filter

`GaussBlur` applies a small Gaussian kernel to every plane. It is the clearest example of a conventional single-input spatial filter and the best next step after [Your first filter](../getting-started/first-filter.md).

[View `GaussBlur.hxx` on GitHub](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/examples/src/GaussBlur.hxx)

## Complete filter

```cpp
#pragma once
#include "Core.vxx"

struct GaussBlur {
    field(InputClip, VideoNode{});

public:
    static constexpr auto Signature = "clip: vnode";

public:
    GaussBlur(auto Arguments) {
        InputClip = Arguments["clip"];
        if (!InputClip.WithConstantFormat() ||
            !InputClip.WithConstantDimensions() ||
            !InputClip.IsSinglePrecision())
            throw std::runtime_error{
                "only single precision floating point clips with constant "
                "format and dimensions supported."
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
        auto InputFrame =
            InputClip.AcquireFrame<const float>(Index, GeneratorContext);
        auto ProcessedFrame = Core.CreateBlankFrameFrom(InputFrame);
        auto GaussKernel = [](auto Center) {
            auto WeightedSum =
                Center[-1][-1] + Center[-1][0] * 2 + Center[-1][1] +
                Center[0][-1] * 2 + Center[0][0] * 4 + Center[0][1] * 2 +
                Center[1][-1] + Center[1][0] * 2 + Center[1][1];
            return WeightedSum / 16;
        };
        for (auto c : Range{ InputFrame.PlaneCount })
            for (auto y : Range{ InputFrame[c].Height })
                for (auto x : Range{ InputFrame[c].Width })
                    ProcessedFrame[c][y][x] =
                        GaussKernel(InputFrame[c].View(y, x));
        return ProcessedFrame;
    }
};
```

## The filter contract

The constructor restricts typed access to formats represented by `float` and requires stable format and dimensions. `SpecifyMetadata` promises an output with the same video information. `rpStrictSpatial` says output frame *n* depends only on input frame *n*.

`AcquireFrame<const float>` makes the input read-only. `CreateBlankFrameFrom` creates writable storage with compatible format, dimensions, and properties.

## Reading the kernel

The weights are the outer product of `[1, 2, 1]` with itself:

```text
1  2  1
2  4  2
1  2  1
```

They sum to 16, so division preserves a constant image. `Plane::View(y, x)` supplies relative neighborhood access and handles coordinates at the image boundary; the algorithm can use the same expression for edges and interior pixels.

## Run it

```python
import vapoursynth as vs

core = vs.core
clip = core.std.BlankClip(format=vs.RGBS, color=[1.0, 0.0, 0.0])
blurred = core.test.GaussBlur(clip)
blurred.set_output()
```

!!! note "Correct first, specialized later"
    This implementation favors a uniform, readable loop. Compare [GaussBlurFast](gauss-blur-fast.md) to see how explicit boundary regions trade simplicity for lower per-sample overhead.

