---
title: Palette source filter
description: Generate a finite GrayS clip without an input node.
---

# Palette source filter

`Palette` is the smallest example that creates video rather than transforming an input clip. Each value in `shades` becomes one constant-colour frame, so the argument list determines both the pixels and the clip length.

[View `Palette.hxx` on GitHub](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/examples/src/Palette.hxx)

## Complete filter

```cpp
#pragma once
#include "Core.vxx"

struct Palette {
    field(Shades, std::vector<double>{});
    field(Width, 640);
    field(Height, 480);

public:
    static constexpr auto Signature = "shades: float[], [width, height]: int?";

public:
    Palette(auto Arguments) {
        Shades = Arguments["shades"];
        if (Arguments["width"].Exists())
            Width = Arguments["width"];
        if (Arguments["height"].Exists())
            Height = Arguments["height"];
        if (Shades.empty())
            throw std::runtime_error{ "shades must contain at least one value." };
        if (Width <= 0 || Height <= 0)
            throw std::runtime_error{ "spatial dimensions must be positive!" };
    }
    auto SpecifyMetadata(auto Core) {
        auto Metadata = VideoInfo{
            .Format = Core.Query(VideoFormats::GrayS),
            .FrameRateNumerator = 30000, .FrameRateDenominator = 1001,
            .Width = Width, .Height = Height,
            .FrameCount = static_cast<int>(Shades.size())
        };
        return Metadata;
    }
    auto GenerateFrame(auto Index, auto GeneratorContext, auto Core) {
        auto ProcessedFrame = VideoFrame<float>{
            Core.AllocateVideoFrame(VideoFormats::GrayS, Width, Height)
        };
        for (auto y : Range{ Height })
            for (auto x : Range{ Width })
                ProcessedFrame[0][y][x] =
                    Shades[static_cast<std::size_t>(Index)];
        return ProcessedFrame;
    }
};
```

## How it works

The signature requires a floating-point array and makes the dimensions optional. The constructor copies those arguments into persistent filter state and rejects states that could not describe a valid clip.

`SpecifyMetadata` is especially important for a source filter: there is no input node from which metadata can be copied. It declares `GrayS`, dimensions, frame rate, and one frame per shade. Because there is no input dependency, the filter does not implement `SpecifyDependencies`.

For frame `Index`, `GenerateFrame` allocates a writable one-plane `float` frame and fills it with `Shades[Index]`. Returning `ProcessedFrame` transfers the owned frame to VapourSynth.

## Run it

```python
import vapoursynth as vs

core = vs.core
clip = core.test.Palette(shades=[0.0, 0.25, 0.5, 0.75, 1.0], width=320, height=180)
clip.set_output()
```

The result contains five frames. Frame 0 is black, frame 2 is middle gray, and frame 4 is white.

!!! tip "Try this next"
    Add a `fpsnum` argument. Validate that it is positive, use it in `SpecifyMetadata`, and leave frame generation unchanged. This separates clip-level metadata from per-frame pixel work.

