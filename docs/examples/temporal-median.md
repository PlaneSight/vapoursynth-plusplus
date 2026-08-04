---
title: TemporalMedian frame windows
description: Request a relative window of frames and compute a per-pixel median.
---

# TemporalMedian frame windows

`TemporalMedian` changes the scheduling model. Output frame *n* needs a window from *n − radius* through *n + radius*, rather than only input frame *n*.

[View `TemporalMedian.hxx` on GitHub](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/examples/src/TemporalMedian.hxx)

## Declare the frame request plan

```cpp
field(InputClip, VideoNode{});
field(Radius, 1);

static constexpr auto Signature = "clip: vnode, radius: int?";

TemporalMedian(auto Arguments) {
    InputClip = Arguments["clip"];
    if (Arguments["radius"].Exists())
        Radius = Arguments["radius"];

    InputClip.FrameRequestor = [this](auto Index) {
        return Range{ Index - Radius, Index + Radius + 1 };
    };

    if (!InputClip.WithConstantFormat() ||
        !InputClip.WithConstantDimensions() ||
        !InputClip.IsSinglePrecision())
        throw std::runtime_error{
            "only single precision floating point clips with constant "
            "format and dimensions supported."
        };
    if (Radius < 0)
        throw std::runtime_error{ "radius cannot be negative!" };
}
```

`Range` uses an exclusive upper bound, so radius 1 requests `n - 1`, `n`, and `n + 1`. Keeping the request rule on `InputClip` lets acquisition use the same relative-index model later.

```cpp
auto SpecifyMetadata() {
    return InputClip.ExtractMetadata();
}

auto SpecifyDependencies() const {
    return std::array{
        InputClip.SpecifyDependency(rpNoFrameReuse)
    };
}
```

The dependency pattern tells VapourSynth that one input frame may participate in several overlapping output windows.

## Select the median

```cpp
auto GenerateFrame(auto Index, auto GeneratorContext, auto Core) {
    auto InputFrames =
        InputClip.AcquireFrames<const float>(Index, GeneratorContext);
    auto ProcessedFrame = Core.CreateBlankFrameFrom(InputFrames[0]);
    auto Samples = std::vector<float>{};
    Samples.resize(2 * Radius + 1);

    for (auto c : Range{ ProcessedFrame.PlaneCount })
        for (auto y : Range{ ProcessedFrame[c].Height })
            for (auto x : Range{ ProcessedFrame[c].Width }) {
                for (auto t : Range{ -Radius, Radius + 1 })
                    Samples[t + Radius] = InputFrames[t][c][y][x];
                std::nth_element(
                    Samples.begin(), Samples.begin() + Radius, Samples.end());
                ProcessedFrame[c][y][x] = Samples[Radius];
            }
    return ProcessedFrame;
}
```

`AcquireFrames` returns the requested window, addressable by its relative offsets. For each output sample, the code gathers the same coordinate from every frame. `std::nth_element` puts the median in its final position without fully sorting the window.

```python
denoised = core.test.TemporalMedian(clip, radius=2)
```

!!! warning "Temporal boundaries are part of the algorithm"
    A production filter must deliberately define requests before frame 0 and after the final frame. Clamping, mirroring, shortening the window, and rejecting the request produce different results.

