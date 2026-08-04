---
title: SeparableConvolution workflow
description: Compose two one-dimensional passes through nested filter calls.
---

# SeparableConvolution workflow

`SeparableConvolution` separates a two-dimensional convolution into horizontal and vertical passes. Its public workflow builds a graph from two private one-dimensional filter instances and `std.Transpose`.

[View `SeparableConvolution.hxx` on GitHub](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/examples/src/SeparableConvolution.hxx)

## Before and after

| Synthetic GrayS input | Two-pass output |
| --- | --- |
| ![Sharp checkerboard before SeparableConvolution](../assets/example-catalog/separable-convolution-before.png) | ![Smoothed checkerboard after SeparableConvolution](../assets/example-catalog/separable-convolution-after.png) |

Both passes use `[1, 2, 1]`. The horizontal result is transposed, processed by the same one-dimensional implementation, then transposed back.

## Public workflow

```cpp
static constexpr auto Signature =
    "clip: vnode, [h_kernel, v_kernel]: float[]?";

static auto InitiateWorkflow(auto Self, auto Arguments, auto Core) {
    auto InputClip = static_cast<VideoNode>(Arguments["clip"]);
    auto HorizontalKernel = std::array{ 1., 2., 1. };
    auto VerticalKernel = std::array{ 0., 0., 0. };

    if (!InputClip.WithConstantFormat() ||
        !InputClip.WithConstantDimensions() ||
        !InputClip.IsSinglePrecision())
        throw std::runtime_error{
            "only single precision floating point clips with constant "
            "format and dimensions supported."
        };

    if (Arguments["h_kernel"].Exists())
        if (Arguments["h_kernel"].size() == HorizontalKernel.size())
            HorizontalKernel = Arguments["h_kernel"];
        else
            throw std::runtime_error{
                "h_kernel must contain 3 scalar values."
            };

    if (Arguments["v_kernel"].Exists())
        if (Arguments["v_kernel"].size() == VerticalKernel.size())
            VerticalKernel = Arguments["v_kernel"];
        else
            throw std::runtime_error{
                "v_kernel must contain 3 scalar values."
            };
    else
        VerticalKernel = HorizontalKernel;

    InputClip = Self("clip", InputClip, "kernel", HorizontalKernel);
    InputClip = Core["std"]["Transpose"]("clip", InputClip);
    InputClip = Self("clip", InputClip, "kernel", VerticalKernel);
    return Core["std"]["Transpose"]("clip", InputClip);
}
```

`Self` invokes the underlying one-dimensional filter. Transposition turns columns into rows, allowing the same horizontal implementation to perform the vertical pass. The second transpose restores the original orientation.

## Private pass implementation

```cpp
field(InputClip, VideoNode{});
field(Kernel, std::array{ 0., 0., 0. });

SeparableConvolution(auto Arguments) {
    InputClip = Arguments["clip"];
    Kernel = Arguments["kernel"];
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
    auto HorizontalConvolution = [this](auto Center) {
        auto [LeftWeight, CenterWeight, RightWeight] = Kernel;
        auto NormalizationFactor =
            LeftWeight + CenterWeight + RightWeight;
        auto WeightedSum =
            LeftWeight * Center[0][-1] +
            CenterWeight * Center[0][0] +
            RightWeight * Center[0][1];
        return WeightedSum / NormalizationFactor;
    };

    for (auto c : Range{ InputFrame.PlaneCount })
        for (auto y : Range{ InputFrame[c].Height })
            for (auto x : Range{ InputFrame[c].Width })
                ProcessedFrame[c][y][x] =
                    HorizontalConvolution(InputFrame[c].View(y, x));
    return ProcessedFrame;
}
```

```python
blurred = core.test.SeparableConvolution(
    clip,
    h_kernel=[1.0, 2.0, 1.0],
    v_kernel=[1.0, 2.0, 1.0],
)
```

!!! warning "Kernel normalization"
    A kernel whose weights sum to zero makes `NormalizationFactor` zero. Derivative kernels need a different normalization policy; they cannot be enabled safely by accepting arbitrary values without changing this contract.
