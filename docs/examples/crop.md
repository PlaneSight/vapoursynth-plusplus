---
title: Crop typed dispatch
description: Change frame dimensions and dispatch over supported sample types.
---

# Crop typed dispatch

`Crop` demonstrates two responsibilities that point filters can avoid: the output dimensions differ from the input, and the same algorithm supports several physical sample types.

[View `Crop.hxx` on GitHub](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/examples/src/Crop.hxx)

## Before and after

| 240 × 128 input | 192 × 96 output |
| --- | --- |
| ![Full striped frame before Crop](../assets/example-catalog/crop-before.png) | ![Smaller striped frame after Crop](../assets/example-catalog/crop-after.png) |

The demonstration removes 32 pixels from the left, 16 from the right, and 16 from both the top and bottom. The output image is shown at its actual dimensions.

## Arguments and validation

```cpp
field(InputClip, VideoNode{});
field(Left, 0);
field(Top, 0);
field(CroppedWidth, 0);
field(CroppedHeight, 0);

static constexpr auto Signature =
    "clip: vnode, [left, right, top, bottom]: int?";

Crop(auto Arguments) {
    auto Right = 0, Bottom = 0;
    InputClip = Arguments["clip"];
    if (Arguments["left"].Exists())
        Left = Arguments["left"];
    if (Arguments["right"].Exists())
        Right = Arguments["right"];
    if (Arguments["top"].Exists())
        Top = Arguments["top"];
    if (Arguments["bottom"].Exists())
        Bottom = Arguments["bottom"];

    CroppedWidth = InputClip.Width - Left - Right;
    CroppedHeight = InputClip.Height - Top - Bottom;

    if (!InputClip.WithConstantFormat() ||
        !InputClip.WithConstantDimensions() || !InputClip.Is444())
        throw std::runtime_error{
            "clips with subsampled format not supported."
        };
    if (Left < 0 || Right < 0 || Top < 0 || Bottom < 0)
        throw std::runtime_error{ "cannot crop negative measures!" };
    if (CroppedWidth <= 0 || CroppedHeight <= 0)
        throw std::runtime_error{
            "dimensions must be positive after cropping!"
        };
}
```

The constructor converts optional arguments to a complete crop rectangle. It rejects subsampled formats because chroma-plane offsets would require alignment rules that this simple example does not implement.

## Publish changed metadata

```cpp
auto SpecifyMetadata() {
    auto Metadata = InputClip.ExtractMetadata();
    Metadata.Width = CroppedWidth;
    Metadata.Height = CroppedHeight;
    return Metadata;
}

auto SpecifyDependencies() const {
    return std::array{
        InputClip.SpecifyDependency(rpStrictSpatial)
    };
}
```

The output keeps the format, duration, and frame rate, but its dimensions must be rewritten before VapourSynth schedules any frames.

## Reuse one algorithm for three sample types

```cpp
auto GenerateFrame(auto Index, auto GeneratorContext, auto Core) {
    auto DrawGenericFrame = [&](auto&& InputFrame) {
        using PixelType =
            std::decay_t<decltype(InputFrame[0][0][0])>;
        auto ProcessedFrame = VideoFrame<PixelType>{
            Core.AllocateVideoFrame(
                InputFrame.ExtractFormat(), CroppedWidth, CroppedHeight)
        };
        ProcessedFrame.AbsorbPropertiesFrom(InputFrame);

        for (auto c : Range{ InputFrame.PlaneCount })
            for (auto y : Range{ CroppedHeight })
                for (auto x : Range{ CroppedWidth })
                    ProcessedFrame[c][y][x] =
                        InputFrame[c][y + Top][x + Left];
        return ProcessedFrame.Transfer();
    };

    if (InputClip.IsSinglePrecision())
        return DrawGenericFrame(
            InputClip.AcquireFrame<const float>(Index, GeneratorContext));
    else if (InputClip.BitsPerSample > 8)
        return DrawGenericFrame(
            InputClip.AcquireFrame<const std::uint16_t>(Index, GeneratorContext));
    else
        return DrawGenericFrame(
            InputClip.AcquireFrame<const std::uint8_t>(Index, GeneratorContext));
}
```

The generic lambda is instantiated with the concrete frame type selected from clip metadata. `PixelType` follows from that type, so allocation and copying agree. `AbsorbPropertiesFrom` preserves per-frame properties even though the pixel storage is newly allocated.

```python
cropped = core.test.Crop(clip, left=16, right=16, top=8, bottom=8)
```

!!! note "Why 4:4:4 is the honest boundary"
    In a subsampled format, luma coordinates do not map one-to-one to chroma coordinates. Supporting it requires explicit divisibility, plane-size, and chroma-location decisions rather than merely deleting the `Is444()` check.
