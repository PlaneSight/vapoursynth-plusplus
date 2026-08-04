---
title: CoreProxy
description: Query formats, allocate frames, invoke plugins, and write log messages.
---

# CoreProxy

CoreProxy wraps the VSCore* supplied to a callback. It provides typed operations for common
core interactions while keeping the API table behind the library boundary.

## Query core state

~~~cpp
auto Info = Core.Query();
Core.Print(static_cast<std::string>(Info));
~~~

CoreInfo contains the core version string, API version, thread count, and frame-buffer usage.

## Query formats

Query a predefined format:

~~~cpp
auto Format = Core.Query(VideoFormats::RGBS);
~~~

Or resolve a dynamically described format:

~~~cpp
auto Format = Core.Query(InputFrame.ExtractFormat());
~~~

If the core cannot resolve the format, the returned descriptor is indeterminate. Check
WithConstantFormat() or QueryID() before using it as fixed metadata.

## Allocate frames

Allocate a video frame from a predefined or queried format:

~~~cpp
auto Frame = VideoFrame<float>{
    Core.AllocateVideoFrame(VideoFormats::GrayS, Width, Height)
};
~~~

Allocate audio storage from an AudioFormat and sample count:

~~~cpp
auto Frame = AudioFrame<float>{
    Core.AllocateAudioFrame(AudioFormat, SampleCount)
};
~~~

For an output that should match an input, prefer:

~~~cpp
auto OutputFrame = Core.CreateBlankFrameFrom(InputFrame);
~~~

This preserves the input's format and dimensions while giving the filter writable storage.

## Copy and rearrange frames

CopyFrame makes an API-level copy while preserving the typed media representation:

~~~cpp
auto Copy = Core.CopyFrame(InputFrame);
~~~

ShufflePlanes constructs a new video frame from selected planes and a target color family. It
validates dimensions and rejects invalid subsampling combinations.

## Invoke plugins

Look up a plugin by namespace or identifier:

~~~cpp
auto Std = Core["std"];
auto Transposed = Std["Transpose"]("clip", InputClip);
~~~

Core.Query("com.vapoursynth.std") looks up a plugin by identifier. Plugin::ListFunctions
returns the names exposed by a plugin.

## Logging

Use the severity-specific helpers when writing diagnostics:

~~~cpp
Core.DebugPrint("details useful while developing");
Core.Print("normal informational message");
Core.Alert("recoverable assumption or warning");
Core.CriticalAlert("serious failure");
Core.Abort("fatal condition");
~~~

The helpers route messages through the API 4 core supplied to the current callback.
