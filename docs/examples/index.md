---
title: Examples
description: Learn from the included VapourSynth filter examples.
---

# Examples

The Examples/ directory is a compact executable catalogue of the library's intended patterns.
They are registered by Examples/EntryPoint.cxx and can be built with:

~~~bash
meson setup build -Dbuild_examples=true
ninja -C build
~~~

Read examples in this order:

1. GaussBlur.hxx for a single-input spatial filter.
2. Crop.hxx for metadata changes and multiple supported sample types.
3. TemporalMedian.hxx for temporal frame requests.
4. MaskedMerge.hxx for multiple input nodes.
5. ModifyFrame.hxx for calling a VapourSynth function from a filter.
6. SeparableConvolution.hxx for a workflow that composes multiple filter instances.

The [example catalog](catalog.md) records each example's contract and source link.

!!! warning "Examples are contracts, not universal filters"
    Several examples intentionally restrict formats or dimensions to keep the implementation
    focused. Preserve those checks when adapting an example; widening support requires matching
    changes to validation, typed access, metadata, and boundary behavior.
