---
title: VideoNode and VideoFrame
description: Reference for typed video nodes, frames, metadata, and pixel access.
---

# VideoNode and VideoFrame

VideoNode adds video metadata and frame-index handling to the generic Node owner.
VideoFrame<T> adds typed plane access and a synchronized VideoFormat to a frame owner.

## VideoNode

| Member | Purpose |
| --- | --- |
| Width, Height | Constant dimensions when known |
| FrameCount | Number of frames when known |
| Format fields | Color family, sample type, bit depth, and subsampling |
| RequestFrame(index, context) | Request one frame from the upstream node |
| FetchFrame(index, context) | Fetch a frame after it is ready |
| AcquireFrame(index, context) | Request on initial activation and fetch when ready |
| RequestFrames(index, context) | Request the configured temporal window |
| AcquireFrames(index, context) | Acquire a temporal window as relative-indexed frames |
| SpecifyDependency(pattern) | Create a VapourSynth filter dependency |

AcquireFrame and AcquireFrames are the normal operations inside GenerateFrame; use the
lower-level request and fetch methods only when implementing a custom acquisition flow.

## VideoFrame<T>

Access a frame's plane dimensions and pixels:

~~~cpp
auto Frame = InputClip.AcquireFrame<const float>(Index, GeneratorContext);

auto PlaneWidth = Frame[0].Width;
auto PlaneHeight = Frame[0].Height;
auto Sample = Frame[0][y][x];
~~~

The plane's Stride is expressed in samples, not bytes, after the wrapper converts the API stride
using sizeof(T).

For a writable frame:

~~~cpp
auto Output = Core.CreateBlankFrameFrom(Frame);
Output[0][y][x] = Sample;
~~~

## Frame properties

Properties are accessed by string key:

~~~cpp
for (auto Key : Frame.ListProperties()) {
    auto Value = Frame[Key];
    // Inspect Value.Type() or convert it to the expected type.
}
~~~

VideoFrame<T>::operator[] is overloaded for both plane indexes and property names. The pixel type
and the argument type select the intended operation at compile time.

## Format predicates

VideoFormat provides the predicates most filters need during validation:

~~~cpp
if (!InputClip.IsYUV() ||
    !InputClip.Is444() ||
    !InputClip.IsSinglePrecision()) {
    throw std::runtime_error{"unsupported input format"};
}
~~~

The predefined VideoFormats enumeration includes common Gray, YUV, and RGB formats such as
GrayS, YUV444PS, and RGBS.
