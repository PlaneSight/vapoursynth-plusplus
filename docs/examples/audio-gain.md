---
title: AudioGain typed audio filter
description: Process floating-point audio frames with the same explicit ownership model as video.
---

# AudioGain typed audio filter

`AudioGain` is the first complete audio filter in the example plugin. It shows
where audio and video share the same filter lifecycle and where their frame
storage differs: an audio frame contains one one-dimensional sample plane per
channel, while a video frame contains two-dimensional image planes.

[View `AudioGain.hxx` on GitHub](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/examples/src/AudioGain.hxx)

## Establish the audio contract

```cpp
field(InputClip, AudioNode{});
field(Gain, 1.0);

static constexpr auto Signature = "clip: anode, gain: float?";

AudioGain(auto Arguments) {
    InputClip = Arguments["clip"];
    if (Arguments["gain"].Exists())
        Gain = Arguments["gain"];

    auto Metadata = InputClip.QueryAudioInfo();
    if (Metadata.Format.SampleType != SampleTypes::Float ||
        Metadata.Format.BitsPerSample != 32)
        throw std::runtime_error{
            "only 32-bit floating-point audio is supported."
        };
}
```

The example chooses one sample representation so the processing loop is easy to
inspect. It does not claim that integer audio or every possible sample width is
unsupported by VapourSynth; only this demonstration filter has that narrower
contract.

The output metadata is the input `AudioInfo`, and the dependency is one strict
audio frame:

```cpp
auto SpecifyMetadata() {
    return InputClip.QueryAudioInfo();
}

auto SpecifyDependencies() const {
    return std::array{
        InputClip.SpecifyDependency(rpStrictSpatial)
    };
}
```

## Access channels as typed planes

```cpp
auto InputFrame =
    InputClip.AcquireFrame<const float>(Index, GeneratorContext);
auto ProcessedFrame = Core.CreateBlankFrameFrom(InputFrame);

for (auto Channel : Range{ InputFrame.ChannelCount })
    for (auto Sample : Range{ InputFrame[Channel].Width })
        ProcessedFrame[Channel][0][Sample] = std::clamp(
            InputFrame[Channel][0][Sample] * static_cast<float>(Gain),
            -1.0f, 1.0f);
```

`AudioNode::AcquireFrame<const float>` performs the same two-phase acquisition
as the video path and returns an `AudioFrame<const float>`. `CreateBlankFrameFrom`
preserves the input audio format, sample count, and channel layout while giving
the filter writable sample storage. The final clamp prevents an amplified
sample from leaving the normalized floating-point range used by this example.

## Run it with synthetic audio

`std.BlankAudio` is useful for a deterministic script-level smoke test. Ask it
for floating-point samples, apply the native filter, and send the audio node to
output slot 1:

```python
import vapoursynth as vs

core = vs.core
audio = core.std.BlankAudio(
    channels=[vs.FRONT_LEFT, vs.FRONT_RIGHT],
    sampletype=vs.FLOAT,
    bits=32,
    samplerate=48_000,
    length=48_000,
)
output = core.test.AudioGain(audio, gain=0.5)
output.set_output(1)
```

For a real source, replace `BlankAudio` with an audio-source plugin while
keeping the `AudioGain` call unchanged. VapourSynth treats audio nodes as a
separate media type, so do not pass an audio node to a `vnode` parameter or
expect video-frame properties to describe an audio stream.

!!! note "Audio frame boundaries"
    An audio frame is an arbitrary block of samples selected by the host. Do not
    use frame properties as a substitute for sample-accurate timing metadata;
    carry timing through the audio node's sample rate, sample count, and frame
    scheduling contract.

!!! tip "Try this next"
    Add a channel-layout argument and use `Core.ShuffleChannels` to create a
    mono or reordered output before applying the gain. Validate that the output
    layout count matches the requested channel count.
