---
title: AudioNode and AudioFrame
description: Reference for typed audio nodes, frames, formats, and channel access.
---

# AudioNode and AudioFrame

`AudioNode` is the owning wrapper for an audio stream. `AudioFrame<T>` owns an
audio frame handle and exposes one typed sample view per channel.

## AudioNode

| Member | Purpose |
| --- | --- |
| `QueryAudioInfo()` | Read format, sample rate, sample count, and frame count |
| `RequestFrame(index, context)` | Request one upstream audio frame |
| `FetchFrame(index, context)` | Fetch a frame after it is ready |
| `AcquireFrame<T>(index, context)` | Request on initial activation and fetch when ready |
| `SpecifyDependency(pattern)` | Declare how the filter consumes the node |

`AcquireFrame` is the normal operation inside `GenerateFrame`. Use the
lower-level request/fetch pair only when the filter implements a custom
multi-stage acquisition strategy.

## AudioFrame<T>

The sample type is part of the wrapper type. `AudioFrame<const float>` is
read-only; `AudioFrame<float>` provides writable channel views. `AudioFrame<void>`
is useful when the sample type is not known at compile time.

```cpp
auto Frame = InputAudio.AcquireFrame<const float>(Index, GeneratorContext);
auto Samples = Frame[0];

auto FirstSample = Samples[0];
auto SampleCount = Samples.Length;
auto SampleStride = Samples.Stride;
```

The channel index is zero-based. `Length` and `Stride` are measured in samples.
The frame also exposes its normalized `AudioFormat` through `ExtractFormat()`.

## AudioFormat

| Field | Meaning |
| --- | --- |
| `SampleType` | Integer or floating-point samples |
| `BitsPerSample` | Logical sample precision |
| `BytesPerSample` | Storage size reported by VapourSynth |
| `ChannelCount` | Number of interleaved-independent channel planes |
| `ChannelLayout` | Typed API channel mask |

`AudioInfo` adds `SampleRate`, `SampleCount`, and `FrameCount`. Use
`AudioFormat::FromAPI` and `AudioFormat::ToAPI` at explicit ABI boundaries;
normal filter code should keep the normalized descriptors.

## AudioChannelLayout

Build layouts from named channels instead of hand-writing masks:

```cpp
auto Layout = AudioChannelLayout::FromChannels({
    AudioChannels::FrontLeft,
    AudioChannels::FrontRight,
});

if (Layout.Contains(AudioChannels::FrontLeft)) {
    auto Count = Layout.Count();
}
```

`Channels()` returns the named channels represented by the mask. An empty
layout is valid as an unspecified layout; when a layout is supplied to
`ShuffleChannels`, its count must match the output channel count.

## Core allocation and channel shuffling

```cpp
auto Frame = AudioFrame<float>{
    Core.AllocateAudioFrame(Format, SampleCount)
};

auto Rearranged = Core.ShuffleChannels(
    Frame,
    std::array{ 0, 0 },
    AudioChannelLayout::FromChannels({
        AudioChannels::FrontLeft,
        AudioChannels::FrontRight,
    }));
```

`ShuffleChannels` returns an owning `AudioFrame<void>`. It validates that the
source frames are audio frames with equal sample counts, that indexes are
non-negative, and that the output layout and channel count agree.

See [Work with audio](../getting-started/audio.md) for a complete task-oriented
walkthrough and [Formats and metadata](formats.md) for the shared descriptor
conversion rules.
