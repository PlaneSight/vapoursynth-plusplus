---
title: TemporalDifference frame windows
description: Request the previous frame, compute a difference image, and publish a motion score.
---

# TemporalDifference frame windows

`TemporalDifference` is a compact temporal filter with two outputs of interest:
the difference image and a `DifferenceMean` frame property. It requests the
current frame and its predecessor as one explicit plan, computes the absolute
per-sample difference, and preserves the current frame's properties.

[View `TemporalDifference.hxx` on GitHub](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/examples/src/TemporalDifference.hxx)

## Declare the request plan

```cpp
InputClip.FrameRequestor = [](auto Index) {
    return std::array{ Index - 1, Index };
};

auto SpecifyDependencies() const {
    return std::array{
        InputClip.SpecifyDependency(rpNoFrameReuse)
    };
}
```

The requestor returns absolute input indexes. The constructor selects
`RemappingFunctions::Replicate`, so an index outside the clip is clamped to its
nearest valid frame. Frame 0 therefore compares against itself and produces a
zero difference image.

`rpNoFrameReuse` is intentional. The filter's window overlaps between adjacent
outputs, but the dependency declaration does not promise that one acquired
frame can be reused by the host across output requests.

## Acquire by relative offset

```cpp
auto InputFrames =
    InputClip.AcquireFrames<const float>(Index, GeneratorContext);
auto& PreviousFrame = InputFrames.at(-1);
auto& CurrentFrame = InputFrames.at(0);
auto ProcessedFrame = Core.CreateBlankFrameFrom(CurrentFrame);
```

`AcquireFrames` exposes the requested window by offset from the output index.
Using `.at()` makes a missing request a visible programming error instead of
turning it into a silently empty frame.

## Produce pixels and metadata together

```cpp
auto TotalDifference = 0.0;
auto SampleCount = std::size_t{ 0 };

for (auto c : Range{ CurrentFrame.PlaneCount })
    for (auto y : Range{ CurrentFrame[c].Height })
        for (auto x : Range{ CurrentFrame[c].Width }) {
            auto Difference = std::fabs(
                CurrentFrame[c][y][x] - PreviousFrame[c][y][x]);
            ProcessedFrame[c][y][x] = Difference;
            TotalDifference += Difference;
            ++SampleCount;
        }

ProcessedFrame.AbsorbPropertiesFrom(CurrentFrame);
ProcessedFrame["DifferenceMean"] =
    TotalDifference / static_cast<double>(SampleCount);
ProcessedFrame["ComparedFrame"] = static_cast<std::int64_t>(Index - 1);
```

The mean is computed independently of the rendered difference image, so a
downstream script can use `DifferenceMean` as a threshold without scanning the
pixels again. The source is restricted to constant single-precision formats;
the sample code therefore has one well-defined numeric path.

## Run it

```python
import vapoursynth as vs

core = vs.core
normal = core.std.BlankClip(format=vs.GRAYS, color=0.15, length=2)
changed = core.std.BlankClip(format=vs.GRAYS, color=0.85, length=1)
source = core.std.Splice([normal, changed, normal])
output = core.test.TemporalDifference(source)

assert output.get_frame(0).props.DifferenceMean == 0.0
assert output.get_frame(1).props.DifferenceMean > 0.0
output.set_output()
```

The first frame is compared with itself. The middle frame changes from 0.15 to
0.85, so its difference image is bright and its mean score is non-zero.

!!! warning "A frame window is an edge policy"
    Reflection is only one boundary choice. A production filter may clamp,
    repeat, shorten the window, or reject requests outside the clip. Choose and
    document that policy before treating a temporal score as an objective
    measurement.

!!! tip "Try this next"
    Add a configurable threshold and write a `SceneChange` integer property.
    Keep the absolute-difference calculation independent from the classification
    rule so the numeric score remains useful to other filters.
