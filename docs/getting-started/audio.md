---
title: Work with audio
description: Create, inspect, acquire, and rearrange audio in a typed filter.
---

# Work with audio

Audio follows the same filter lifecycle as video, but its frame geometry is
different: an audio frame contains a number of samples for each channel rather
than a two-dimensional plane. The library keeps that distinction explicit with
`AudioNode`, `AudioFrame<T>`, `AudioFormat`, and `AudioInfo`.

## Choose the operation

| Task | Operation |
| --- | --- |
| Read stream metadata | `AudioNode::QueryAudioInfo()` |
| Request and access a frame | `AudioNode::AcquireFrame<T>()` |
| Allocate a new frame | `CoreProxy::AllocateAudioFrame()` |
| Preserve an input format | `CoreProxy::CreateBlankFrameFrom()` |
| Reorder or duplicate channels | `CoreProxy::ShuffleChannels()` |

For the fields and ownership rules of each type, see the [audio reference](../reference/audio.md).

## Read audio samples

An audio node exposes the stream descriptor through the same node owner used by
video filters. Inside `GenerateFrame`, acquire the frame with a sample type
that matches the validated format:

```cpp
auto Input = InputAudio.AcquireFrame<const float>(Index, GeneratorContext);

for (auto Channel = 0; Channel < Input.ChannelCount; ++Channel) {
    auto& Samples = Input[Channel];
    for (auto Sample = 0; Sample < Samples.Width; ++Sample)
        Analyse(Samples[0][Sample]);
}
```

`AudioFrame<T>::operator[]` selects a channel. The channel view has height 1;
its `Width` is the number of samples and its `Stride` is measured in samples,
not bytes. Use a const sample type for read-only access; allocate or acquire a
non-const frame only when the filter owns writable output.

## Allocate output

Use the input format when the output has the same sample representation and
channel layout:

```cpp
auto Output = Core.CreateBlankFrameFrom(Input);
Output[0][0][0] = Input[0][0][0];
```

For a new descriptor, pass an `AudioFormat` and the number of samples:

```cpp
auto Output = AudioFrame<float>{
    Core.AllocateAudioFrame(OutputFormat, SampleCount)
};
```

Validate sample type, bit depth, channel count, sample rate, and frame length
before entering the processing loop. The wrapper reports invalid VapourSynth
allocation or frame handles as errors at the API boundary.

## Rearrange channels

`ShuffleChannels` takes source-channel indexes and produces a new owned audio
frame. The simplest form uses one source frame for every output channel:

```cpp
auto Stereo = AudioChannelLayout::FromChannels({
    AudioChannels::FrontLeft,
    AudioChannels::FrontRight,
});

auto Swapped = Core.ShuffleChannels(
    Input,
    std::array{ 1, 0 },
    Stereo);
```

The indexes are zero-based source channel indexes. A single source frame may
provide all requested channels, or one source frame may be supplied per output
channel. All source frames must be audio frames with equal sample counts. The
requested channel count must agree with the non-empty output layout.

## Keep audio and video synchronized

Audio and video nodes have separate frame indexes and metadata. Do not assume
that an audio frame index corresponds to a video frame index. Use each node's
`AudioInfo` or `VideoInfo` and make the conversion between time, samples, and
frames explicit in the filter contract.

For lifecycle, dependency, and ownership rules shared by both media types,
continue with [Filter lifecycle](../concepts/filter-lifecycle.md) and [Frames
and ownership](../concepts/frames-and-ownership.md). The complete processing
example is [AudioGain](../examples/audio-gain.md).
