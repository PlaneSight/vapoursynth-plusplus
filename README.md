# vapoursynth-plusplus

A C++23, header-oriented layer for authoring VapourSynth API 4 filters with
typed maps, RAII resource ownership, compile-time signature recognition, and a
two-phase frame-acquisition model.

The public C++ patterns remain independent of the VapourSynth ABI. The API 4
adapter is concentrated at the handle, map, format, and plugin callback
boundaries.

## Requirements

- VapourSynth API 4.2 or newer
- C++23 compiler
- Meson and Ninja

The SDK header is selected through `VapourSynthConfig.vxx`, which requests the
API 4.2 declaration set and rejects older SDKs at compile time.

## Compilation
### Linux

```
$ meson setup build
$ ninja -C build
```

Build the example plugin explicitly when using a custom SDK installation:

```
meson setup build -Dbuild_examples=true
ninja -C build
```

The plugin exports `VapourSynthPluginInit2`, the API 4 entry point. Example
filters declare their input dependencies explicitly through
`Node::SpecifyDependency`, allowing API 4 to configure scheduling and cache
behavior without hidden global state.
