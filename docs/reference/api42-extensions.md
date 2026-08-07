---
title: API 4.2 extensions
description: Use VapourSynth API 4.1 and 4.2 facilities through the C++ façade.
---

# API 4.2 extensions

`API4Extensions.vxx` adds the API 4.1 and 4.2 facilities that are not part of the
original façade. The repository requires VapourSynth API 4.2 or newer.

Most operations are available directly on `Node`, `CoreProxy`, `Plugin`, and
`Utility::Map::Item`. The `API4` namespace provides free-function forms for
code that is already working at the API boundary.

```cpp
#include "API4Extensions.vxx"
```

The extension layer does not wrap the experimental graph-inspection calls that
the VapourSynth API documents as unsuitable for use inside filters.

## Configure a filter at creation time

`PluginInstantiator::RegisterFilter` applies optional scheduling and cache
policies immediately after the output node is created. A filter can declare
static policy members:

```cpp
struct Filter {
    static constexpr auto LinearAccess = true;
    static constexpr auto CachePolicy = API4::CachePolicy{
        .Mode = CacheModes::Automatic,
        .Options = API4::CacheConfiguration{
            .FixedSize = -1,
            .MaxSize = 64,
            .MaxHistorySize = 8,
        },
    };
};
```

The same policies can be supplied dynamically with
`SpecifyLinearAccess()` and `SpecifyCachePolicy()`:

```cpp
auto SpecifyLinearAccess() const {
    return InputClip.QueryVideoInfo().FrameCount > 0;
}

auto SpecifyCachePolicy() const {
    return API4::CachePolicy{
        .Mode = CacheModes::Enabled,
        .Options = API4::CacheConfiguration{ .MaxSize = 128 },
    };
}
```

The accepted cache policy forms are:

| Form | Meaning |
| --- | --- |
| `CacheOptions` | Existing automatic or disabled policy |
| `CacheModes` | API cache mode: automatic, disabled, or enabled |
| `API4::CacheConfiguration` | Cache sizing options without changing the mode |
| `API4::CachePolicy` | A cache mode together with cache sizing options |

`API4::EnableLinearAccess`, `API4::SetCacheMode`, and
`API4::ConfigureCache` are also available when a node must be configured
manually. The member equivalents are `Node::EnableLinearAccess`,
`Node::SetCacheMode`, and `Node::ConfigureCache`.

## Control and inspect nodes

`Node` exposes the API 4 node controls and diagnostic queries needed by filter
infrastructure and profiling tools:

```cpp
InputClip.SetCacheMode(CacheModes::Enabled);
InputClip.ConfigureCache(-1, 128, 8);
InputClip.EnableLinearAccess();

auto Name = InputClip.QueryName();
auto Mode = InputClip.QueryFilterMode();
auto Dependencies = InputClip.QueryDependencies();
auto ProcessingTime = InputClip.QueryProcessingTime();
```

Pass `true` to `QueryProcessingTime(true)` to read and reset the node's timing
counter. `ClearCache()` discards the node's cached frames.

During frame generation, the API 4 context helpers support explicit frame
lifetime control:

```cpp
API4::ReleaseFrameEarly(InputClip, Index, GeneratorContext);
API4::CacheFrame(InputFrame, Index, GeneratorContext);
```

Use these only when the filter's request and lifetime strategy makes the
ownership transition clear. The ordinary two-phase acquisition path remains the
default.

## Inspect core state and timing

`CoreProxy` retains the original `Query()` result and adds the API 4.2 core
descriptor:

```cpp
auto Info = Core.QueryInfo2();

Core.SetNodeTiming(true);
auto FreedNodeTime = Core.QueryFreedNodeProcessingTime();
Core.ClearCaches();
```

`CoreInfo2` contains the version string, core version, API version, creation
flags, thread count, maximum frame-buffer size, and current frame-buffer usage.
Pass `true` to `QueryFreedNodeProcessingTime(true)` to reset the accumulated
counter. `NodeTimingEnabled()` reports the current core setting.

The equivalent free functions are `API4::QueryCoreInfo`,
`API4::SetNodeTiming`, `API4::NodeTimingEnabled`,
`API4::QueryFreedNodeProcessingTime`, and `API4::ClearCaches`.

## Inspect formats and plugin functions

API format-name queries produce the canonical VapourSynth names for normalized
video and audio descriptors:

```cpp
auto VideoName = API4::QueryFormatName(Core.Query(VideoFormats::RGBS));
auto AudioName = API4::QueryFormatName(AudioFormat{
    .SampleType = SampleTypes::Float,
    .BitsPerSample = 32,
    .BytesPerSample = 4,
    .ChannelCount = 2,
});
```

Plugin metadata is available without invoking a function:

```cpp
auto Version = Plugin.QueryVersion();
auto Function = Plugin.QueryFunction("Transpose");

for (auto& Descriptor : Plugin.ListFunctionInfo()) {
    // Descriptor.Name, Descriptor.Arguments, Descriptor.ReturnType
}
```

`QueryFunction` throws `std::runtime_error` when the named function is not
registered. `ListFunctions()` remains available when only function names are
needed.

## Use extended map values

Map items distinguish a missing key, an empty value, and a value at a particular
index:

```cpp
auto Values = Arguments["values"];
if (!Values.IsPresent())
    throw std::runtime_error{ "values is required" };

if (Values.IsEmpty())
    return;

auto Hint = Values.QueryDataTypeHint();
auto First = Values[0];
if (!First.Exists())
    throw std::runtime_error{ "values has no first element" };
```

Use `Convert<T>()` for explicit portable container reads. Exact native integer
and floating-point arrays use `std::int64_t` and `double` elements:

```cpp
auto IntegerValues = Arguments["integer_values"].Convert<std::vector<std::int64_t>>();
auto FloatValues = Arguments["float_values"].Convert<std::vector<double>>();
```

The writable façade uses native array operations for those same element types:

```cpp
Output["integer_values"] = std::vector<std::int64_t>{ 1, 2, 3 };
Output["float_values"] = std::vector<double>{ 0.25, 0.5, 0.75 };
```

Other compatible containers are still supported, but they are represented as
individual map values. Explicit conversion makes the vector behavior portable
across compilers.

For data values, `MapData` preserves the data-type hint when writing a map:

```cpp
Output["payload"] = Utility::Map::MapData{
    .Bytes = "binary payload",
    .Hint = DataTypeHints::Binary,
};
```

Writable items can consume an owned node, frame, or function. Consumption
transfers the resource into the map; do not use the moved-from wrapper after the
call:

```cpp
Output["clip"].Consume(std::move(OutputNode));
```

The low-level `API4::Map` helpers expose empty values, saturated numeric reads,
native array spans, native array writes, and data-type hints when a `VSMap*` is
already available:

| Helper | Purpose |
| --- | --- |
| `SetEmpty` | Create a present value with zero elements |
| `GetIntSaturated` / `GetFloatSaturated` | Read with API saturation semantics |
| `GetIntArray` / `GetFloatArray` | Borrow the API's native array storage as a span |
| `SetIntArray` / `SetFloatArray` | Write a native array from a span |
| `QueryDataTypeHint` | Read the binary or UTF-8 data hint |

The spans returned by `GetIntArray` and `GetFloatArray` are borrowed from the
map. Keep the map alive and do not mutate it in a way that invalidates the span
while it is being used.

## Describe audio channels

`AudioChannelLayout` represents the API channel mask while keeping channel
operations typed:

```cpp
auto Stereo = AudioChannelLayout::FromChannels({
    AudioChannels::FrontLeft,
    AudioChannels::FrontRight,
});

if (Stereo.Contains(AudioChannels::FrontLeft)) {
    auto ChannelCount = Stereo.Count();
}
```

`CoreProxy::ShuffleChannels` creates an audio frame with a requested channel
order and layout:

```cpp
auto Swapped = Core.ShuffleChannels(
    InputFrame,
    std::array{ 1, 0 },
    Stereo);
```

The channel indexes select source channels. A single source frame can provide
all output channels, or one source frame can be supplied for each output
channel. Source frames must be audio frames with equal sample counts. The
requested channel count and non-empty layout must agree with the channel-index
count.

`ShuffleChannels` returns `AudioFrame<void>`, which carries the resulting
`AudioFormat` and owns the new frame handle.

## Keep the boundary explicit

The high-level wrappers remain the preferred interface for filter code. Use the
`API4` namespace when a callback already owns a `VSNode*`, `VSCore*`, or `VSMap*`,
or when an operation has no natural member owner. All calls require the
VapourSynth API table to have been installed by the normal plugin entry point.
