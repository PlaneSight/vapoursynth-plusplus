---
title: PropGain frame-property processing
description: Let a frame property control typed pixel work while preserving metadata.
---

# PropGain frame-property processing

`PropGain` demonstrates a useful boundary between a VapourSynth script and a
native filter: the script supplies a per-frame value in the property map, and
the C++ filter consumes it without adding another node argument. The filter
copies the input properties, applies the value to every floating-point sample,
and publishes the value it actually used as `AppliedGain`.

[View `PropGain.hxx` on GitHub](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/examples/src/PropGain.hxx)

## Before and after

The example sets `Gain=0.35` on an RGBS stripe pattern. The output is clamped
to the normalized floating-point range, so a property that would brighten a
highlight cannot produce an out-of-range sample.

| Input with `Gain=0.35` | `PropGain` output |
| --- | --- |
| ![RGB stripe input for PropGain](../assets/example-catalog/prop-gain-before.png) | ![RGB stripe output after PropGain](../assets/example-catalog/prop-gain-after.png) |

## Read the property at the frame boundary

```cpp
auto InputFrame =
    InputClip.AcquireFrame<const float>(Index, GeneratorContext);
auto ProcessedFrame = Core.CreateBlankFrameFrom(InputFrame);
auto Gain = 1.0;

if (InputFrame["Gain"].Exists())
    Gain = static_cast<double>(InputFrame["Gain"]);
if (!std::isfinite(Gain) || Gain < 0.0)
    throw std::runtime_error{
        "the Gain frame property must be finite and non-negative."
    };
```

The absent-property case is deliberate: `PropGain` uses unity gain when the
script does not provide `Gain`. Invalid values fail at frame generation rather
than silently turning into NaNs or negative brightness.

## Keep the pixel contract explicit

```cpp
for (auto c : Range{ InputFrame.PlaneCount })
    for (auto y : Range{ InputFrame[c].Height })
        for (auto x : Range{ InputFrame[c].Width })
            ProcessedFrame[c][y][x] = std::clamp(
                static_cast<float>(InputFrame[c][y][x] * Gain),
                0.0f, 1.0f);

ProcessedFrame.AbsorbPropertiesFrom(InputFrame);
ProcessedFrame["AppliedGain"] = Gain;
```

This example accepts only constant GrayS and RGBS clips. That restriction makes
the meaning of the normalized range unambiguous and lets one typed loop cover
both supported color families. The output retains the input format, dimensions,
timing, and existing properties.

## Run it

```python
import vapoursynth as vs

core = vs.core
source = core.std.BlankClip(
    format=vs.RGBS,
    width=320,
    height=180,
    color=[0.9, 0.4, 0.1],
)
source = core.std.SetFrameProps(source, Gain=0.35)
output = core.test.PropGain(source)

frame = output.get_frame(0)
assert frame.props.AppliedGain == 0.35
output.set_output()
```

For a changing value, place the property on each frame before calling the
filter. The property is data, not filter state: a single `PropGain` node can
process frames with different gains.

!!! warning "Properties are part of the contract"
    Document the property name, type, default, valid range, and whether it is
    copied to the output. A property-driven filter should not silently accept a
    spelling mistake or an incompatible value as if it were the intended input.

!!! tip "Try this next"
    Replace the fixed `Gain` key with a `property` string argument, then validate
    that the selected key is non-empty before requesting frames. The algorithm
    stays the same while the filter becomes reusable for different metadata
    producers.
