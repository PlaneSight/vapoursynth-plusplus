---
title: Frames and ownership
description: Use typed frames, planes, properties, and API handle ownership safely.
---

# Frames and ownership

The wrappers are deliberately explicit about two different concerns:

- the C++ type used to access media data;
- ownership of the underlying VapourSynth handle.

## Read-only versus writable frames

VideoFrame<const T> and VideoFrame<T> expose the same logical frame with different access
contracts:

| Wrapper | Pixel access | Intended use |
| --- | --- | --- |
| VideoFrame<const T> | Bounds-aware read access and View | Input frames |
| VideoFrame<T> | Direct writable rows | Newly allocated output frames |
| Frame | Untyped read-only frame and properties | Generic API boundary |
| WritableFrame | Untyped writable frame and properties | Generic writable boundary |

The const sample type is not cosmetic. It selects read-only behavior at compile time and enables
border remapping for neighborhood reads.

## Planes

A typed video frame exposes planes with operator[]:

~~~cpp
auto InputFrame = InputClip.AcquireFrame<const float>(Index, GeneratorContext);
auto OutputFrame = Core.CreateBlankFrameFrom(InputFrame);

for (auto Plane : Range{InputFrame.PlaneCount})
    for (auto y : Range{InputFrame[Plane].Height})
        for (auto x : Range{InputFrame[Plane].Width})
            OutputFrame[Plane][y][x] = InputFrame[Plane][y][x];
~~~

For read-only neighborhood operations, View(y, x) creates a coordinate-relative access window:

~~~cpp
auto Center = InputFrame[Plane].View(y, x);
auto WeightedSum =
    Center[-1][-1] + Center[-1][0] + Center[0][-1] + Center[0][0];
~~~

The default read-only remapping function is reflection. That means a neighborhood access beyond
an edge is remapped instead of dereferencing outside the plane. A performance-oriented algorithm
can use DirectAccess() when it has already handled its boundaries.

## Frame properties

Frame properties use the same map-style interface as filter arguments:

~~~cpp
if (InputFrame["_Matrix"].Exists()) {
    auto Matrix = static_cast<int>(InputFrame["_Matrix"]);
}

OutputFrame.AbsorbPropertiesFrom(InputFrame);
OutputFrame["_Matrix"] = 0;
~~~

Read-only frames expose read-only properties. Writable frames can replace or append values through
operator= and operator+=.

## Handle ownership

ResourceManager::Owner<T> owns a VapourSynth frame, node, or function reference:

- copying clones the underlying API reference;
- moving transfers the reference without an extra clone;
- destruction releases the reference;
- Leak() transfers the raw handle to a new owner or an API boundary;
- Observe() borrows the handle without transferring ownership.

This is why returning OutputFrame by value is safe: the wrapper's move/copy operations preserve
the correct API reference count.

Maps and cores use the non-copyable ResourceManager::Tracker because their lifetime is tied to
the current callback or explicit shared state. Avoid storing borrowed API pointers beyond the
callback that supplied them.
