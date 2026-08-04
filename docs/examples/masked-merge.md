---
title: MaskedMerge multi-input filter
description: Validate and acquire three clips as one typed operation.
---

# MaskedMerge multi-input filter

`MaskedMerge` combines a background, foreground, and grayscale mask. It demonstrates how to make compatibility across several nodes an explicit constructor invariant.

[View `MaskedMerge.hxx` on GitHub](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/examples/src/MaskedMerge.hxx)

## Establish compatible inputs

```cpp
field(Background, VideoNode{});
field(Foreground, VideoNode{});
field(Mask, VideoNode{});

static constexpr auto Signature = "[clipa, clipb, mask]: vnode";

MaskedMerge(auto Arguments) {
    Background = Arguments["clipa"];
    Foreground = Arguments["clipb"];
    Mask = Arguments["mask"];

    if (!Background.WithConstantFormat() ||
        !Background.WithConstantDimensions() ||
        !Background.IsSinglePrecision() || !Background.Is444())
        throw std::runtime_error{
            "only non-subsampled single precision floating point clips "
            "with constant format and dimensions supported."
        };
    if (Background.ExtractFormat() != Foreground.ExtractFormat() ||
        Background.Width != Foreground.Width ||
        Background.Height != Foreground.Height)
        throw std::runtime_error{
            "clipa and clipb must have the same format and dimensions."
        };
    if (!Mask.WithConstantFormat() ||
        Mask.ColorFamily != ColorFamilies::Gray ||
        !Mask.IsSinglePrecision() ||
        Mask.Width != Background.Width || Mask.Height != Background.Height)
        throw std::runtime_error{
            "mask must be GrayS and have the same dimensions as clipa."
        };
}
```

After construction, frame generation may safely assume matching spatial coordinates, compatible foreground/background planes, and a single floating-point mask plane.

## Declare and acquire all dependencies

```cpp
auto SpecifyMetadata() {
    return Background.ExtractMetadata();
}

auto SpecifyDependencies() const {
    return std::array{
        Background.SpecifyDependency(rpStrictSpatial),
        Foreground.SpecifyDependency(rpStrictSpatial),
        Mask.SpecifyDependency(rpStrictSpatial)
    };
}

auto GenerateFrame(auto Index, auto GeneratorContext, auto Core) {
    auto [BackgroundFrame, ForegroundFrame, MaskFrame] =
        Node::AcquireFrameGiven<const float>(Index, GeneratorContext)
            .From(Background, Foreground, Mask);
    auto ProcessedFrame = Core.CreateBlankFrameFrom(BackgroundFrame);

    for (auto c : Range{ BackgroundFrame.PlaneCount })
        for (auto y : Range{ BackgroundFrame[c].Height })
            for (auto x : Range{ BackgroundFrame[c].Width })
                ProcessedFrame[c][y][x] =
                    ForegroundFrame[c][y][x] * MaskFrame[0][y][x] +
                    BackgroundFrame[c][y][x] *
                        (1 - MaskFrame[0][y][x]);
    return ProcessedFrame;
}
```

Structured binding keeps each acquired frame associated with its node. A mask value of 0 selects the background, 1 selects the foreground, and intermediate values linearly blend them.

```python
merged = core.test.MaskedMerge(clipa=background, clipb=foreground, mask=mask)
```

!!! note "Range policy"
    The formula does not clamp the mask. Values outside `[0, 1]` extrapolate rather than blend. Whether to validate, clamp, or deliberately permit that behavior is an API decision.

