---
title: Example catalog
description: Compare the included example filters by their scheduling and data contracts.
---

# Example catalog

| Example | Demonstrates | Input contract | Scheduling |
| --- | --- | --- | --- |
| [GaussBlur](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/Examples/GaussBlur.hxx) | Read-only neighborhood access through Plane::View | Constant dimensions and format; single-precision float | One strict spatial frame |
| [GaussBlurFast](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/Examples/GaussBlurFast.hxx) | Boundary-specialized loops and DirectAccess | Constant dimensions and format; single-precision float | One strict spatial frame |
| [Crop](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/Examples/Crop.hxx) | Metadata changes and typed dispatch | Constant dimensions and format; 4:4:4 input | One strict spatial frame |
| [TemporalMedian](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/Examples/TemporalMedian.hxx) | Temporal windows and relative frame indexes | Constant dimensions and format; single-precision float | radius frames; no frame reuse |
| [MaskedMerge](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/Examples/MaskedMerge.hxx) | Acquiring three nodes with one typed workflow | Matching single-precision 4:4:4 clips and GrayS mask | Three strict spatial frames |
| [ModifyFrame](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/Examples/ModifyFrame.hxx) | Calling a Function from a filter | Constant dimensions and format | One strict spatial frame; parallel resource acquisition |
| [SeparableConvolution](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/Examples/SeparableConvolution.hxx) | Composing horizontal and vertical passes | Constant dimensions and format; single-precision float | Nested filter workflow |
| [Rec601ToRGB](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/Examples/Rec601ToRGB.hxx) | Format conversion and frame-property checks | YUV444PS, full range, Rec. 601 matrix | One strict spatial frame |
| [Palette](https://github.com/PlaneSight/vapoursynth-plusplus/blob/master/Examples/Palette.hxx) | A source filter with generated metadata | Positive dimensions; non-empty shade list | No input dependency |

## Reading the examples

Each filter makes its assumptions executable in the constructor. The useful questions to ask
while reading are:

- What metadata does the filter promise?
- Which exact input frames does it request?
- Is each input frame read-only?
- How are borders or out-of-range temporal indexes handled?
- Which properties are copied or rewritten?
- Which resource owns each API handle after the callback returns?

These questions are also the checklist for a new filter.
