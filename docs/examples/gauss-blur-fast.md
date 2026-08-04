---
title: GaussBlurFast specialized loops
description: Separate image boundaries from the hot interior loop.
---

# GaussBlurFast specialized loops

`GaussBlurFast` computes the same 3×3 Gaussian blur as [GaussBlur](gauss-blur.md), but exposes a performance-oriented structure: direct plane access, an uncomplicated interior loop, and compile-time-specialized edge handling.

[View `GaussBlurFast.hxx` on GitHub](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/examples/src/GaussBlurFast.hxx)

## Before and after

| Synthetic GrayS input | `GaussBlurFast` output |
| --- | --- |
| ![Sharp checkerboard before GaussBlurFast](../assets/example-catalog/gauss-blur-fast-before.png) | ![Smoothed checkerboard after GaussBlurFast](../assets/example-catalog/gauss-blur-fast-after.png) |

The generator proves parity with `GaussBlur` throughout the image interior. The examples deliberately use different one-pixel border policies: `Plane::View` remapping here versus explicit clamping below.

## Boundary-specialized kernel

```cpp
template<auto ClampAbove = false, auto ClampBelow = false,
         auto ClampLeft = false, auto ClampRight = false>
auto GaussKernel(auto& Channel, auto y, auto x) {
    auto Above = y - 1;
    auto Below = y + 1;
    auto Left = x - 1;
    auto Right = x + 1;
    if constexpr (ClampAbove)
        Above = 0;
    if constexpr (ClampBelow)
        Below = Channel.Height - 1;
    if constexpr (ClampLeft)
        Left = 0;
    if constexpr (ClampRight)
        Right = Channel.Width - 1;

    auto WeightedSum =
        Channel[Above][Left] + Channel[Above][x] * 2 + Channel[Above][Right] +
        Channel[y][Left] * 2 + Channel[y][x] * 4 + Channel[y][Right] * 2 +
        Channel[Below][Left] + Channel[Below][x] * 2 + Channel[Below][Right];
    return WeightedSum / 16;
}
```

`if constexpr` removes inactive clamps from each instantiation. Interior pixels call `GaussKernel` with every flag false; an upper-left corner calls `GaussKernel<true, false, true>`.

## Complete frame generator

The constructor, metadata, and dependency methods are identical in purpose to `GaussBlur`. The important difference is the frame loop:

```cpp
auto GenerateFrame(auto Index, auto GeneratorContext, auto Core) {
    auto InputFrame =
        InputClip.AcquireFrame<const float>(Index, GeneratorContext);
    auto ProcessedFrame = Core.CreateBlankFrameFrom(InputFrame);

    for (auto c : Range{ InputFrame.PlaneCount }) {
        auto& Canvas = InputFrame[c].DirectAccess();

        for (auto y : Range{ 1, Canvas.Height - 1 }) {
            ProcessedFrame[c][y][0] =
                GaussKernel<false, false, true>(Canvas, y, 0);
            for (auto x : Range{ 1, Canvas.Width - 1 })
                ProcessedFrame[c][y][x] = GaussKernel(Canvas, y, x);
            ProcessedFrame[c][y][Canvas.Width - 1] =
                GaussKernel<false, false, false, true>(
                    Canvas, y, Canvas.Width - 1);
        }

        for (auto x : Range{ 1, Canvas.Width - 1 }) {
            ProcessedFrame[c][0][x] =
                GaussKernel<true>(Canvas, 0, x);
            ProcessedFrame[c][Canvas.Height - 1][x] =
                GaussKernel<false, true>(Canvas, Canvas.Height - 1, x);
        }

        ProcessedFrame[c][0][0] =
            GaussKernel<true, false, true>(Canvas, 0, 0);
        ProcessedFrame[c][0][Canvas.Width - 1] =
            GaussKernel<true, false, false, true>(
                Canvas, 0, Canvas.Width - 1);
        ProcessedFrame[c][Canvas.Height - 1][0] =
            GaussKernel<false, true, true>(
                Canvas, Canvas.Height - 1, 0);
        ProcessedFrame[c][Canvas.Height - 1][Canvas.Width - 1] =
            GaussKernel<false, true, false, true>(
                Canvas, Canvas.Height - 1, Canvas.Width - 1);
    }
    return ProcessedFrame;
}
```

## Why split the image

Most pixels are interior pixels. Keeping their loop free of boundary decisions gives the compiler a simpler hot path. The four edges and four corners then select fixed clamp policies. `DirectAccess` is appropriate because every coordinate is made valid explicitly.

!!! warning "Preconditions belong with specialized code"
    This loop assumes each plane is at least 2×2. Production specialization should validate that precondition or include a small-plane path. Faster-looking code is not an improvement if it widens the set of invalid memory accesses.

## Compare behavior

```python
reference = core.test.GaussBlur(clip)
specialized = core.test.GaussBlurFast(clip)
```

Use representative clips and a numeric comparison before treating specialization as equivalent. Benchmark only after correctness has an independent oracle.
