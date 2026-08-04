---
title: Maps and functions
description: Read VapourSynth argument maps and call API function objects.
---

# Maps and functions

VapourSynth arguments, return values, frame properties, and callable objects cross the API through
maps. The library provides typed views over those maps.

## ArgumentList

ArgumentList is the read-only view passed to filter constructors and registered functions:

~~~cpp
auto Clip = static_cast<VideoNode>(Arguments["clip"]);

if (Arguments["radius"].Exists()) {
    auto Radius = static_cast<int>(Arguments["radius"]);
}
~~~

An item may contain one value or a sequence. Convert sequences to a compatible container:

~~~cpp
auto Kernel = static_cast<std::vector<double>>(Arguments["kernel"]);
~~~

Supported conversions include integral and floating-point values, UTF-8 data, nodes, frames,
functions, and compatible containers of those values.

## Writable map items

Writable map items use replacement and append assignment:

~~~cpp
Output["value"] = 42;
Output["value"] += 43;
~~~

operator= replaces the existing key; operator+= appends a value. Use append only when the
function's return contract intentionally contains multiple values.

## Function

Function owns a VSFunction* and invokes it with alternating key/value arguments:

~~~cpp
auto Result = Evaluator("src", InputFrame);
auto OutputFrame = static_cast<FrameReference>(Result);
~~~

The function wrapper creates an argument map, invokes the function, checks the result map for an
API error, and returns the single result item or the inferred return key.

## Plugin

A Plugin resolves functions by name:

~~~cpp
auto Std = Core["std"];
auto Output = Std["Transpose"]("clip", InputClip);
~~~

The wrapper queries the function's argument signature from the API and uses it to normalize the
returned map item. Plugin::ListFunctions provides discovery for diagnostics and tooling.

## Error handling

Map errors are checked immediately after invocation. A failed API call becomes a
std::runtime_error; registered callbacks translate that exception to a VapourSynth error map.
Do not ignore a map's error state and continue with a partially populated result.
