---
title: Formats and metadata
description: Describe video and audio formats and stream metadata.
---

# Formats and metadata

The descriptor types normalize API structures into C++ fields while retaining conversion functions
for the VapourSynth boundary.

## VideoFormat

| Field | Meaning |
| --- | --- |
| ColorFamily | Gray, RGB, YUV, or Undefined |
| SampleType | Integer or floating point samples |
| BitsPerSample | Logical sample precision |
| BytesPerSample | Storage size reported by the API |
| HorizontalSubsamplingFactor | Log2 horizontal subsampling |
| VerticalSubsamplingFactor | Log2 vertical subsampling |
| PlaneCount | Number of planes |

QueryID produces a stable normalized format identifier. AdjustToStandardLayout converts an API
VSVideoFormat; AdjustToLegacyLayout converts back to the API representation.

Useful predicates include:

~~~cpp
Format.IsGray();
Format.IsRGB();
Format.IsYUV();
Format.Is444();
Format.IsSinglePrecision();
~~~

The predefined VideoFormats values cover the API's common Gray, YUV, and RGB formats.

## VideoInfo

VideoInfo combines a format with stream metadata:

~~~cpp
auto Metadata = InputClip.ExtractMetadata();
Metadata.Width = NewWidth;
Metadata.Height = NewHeight;
return Metadata;
~~~

The fields are:

- Format
- FrameRateNumerator
- FrameRateDenominator
- Width
- Height
- FrameCount

WithConstantFormat() and WithConstantDimensions() distinguish fixed metadata from an
indeterminate stream.

## AudioFormat and AudioInfo

Audio descriptors contain sample type, bit depth, storage size, channel count, and channel layout.
AudioInfo adds sample rate, sample count, and frame count. The same FromAPI and
AdjustToLegacyLayout pattern is used for conversion.
