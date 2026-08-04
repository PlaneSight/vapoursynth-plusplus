---
title: Registration
description: Register a VapourSynth API 4 plugin, filter, or function.
---

# Registration

PluginInstantiator.vxx is the adapter's registration boundary. It configures a plugin, derives
function signatures, and supplies the VapourSynthPluginInit2 entry point.

## Plugin entry point

The conventional entry point is:

~~~cpp
#include "PluginInstantiator.vxx"

auto Main() {
    auto Descriptor = PluginInfo{
        .Namespace = "test",
        .Identifier = "com.vsfilterscript.test",
        .Description = "Example filters"
    };

    PluginInstantiator::SpecifyConfigurations(Descriptor);
    PluginInstantiator::RegisterFilter<MyFilter>();
}

InstantiatePluginFrom(Main);
~~~

InstantiatePluginFrom installs the API table and plugin handle supplied by VapourSynth before
calling Main. The implementation checks that later callbacks use the same API table.

## SpecifyConfigurations

~~~cpp
PluginInstantiator::SpecifyConfigurations(
    Descriptor,
    WriteProtectOptions::Rewritable
);
~~~

The one-argument form registers a write-once plugin. Use the second argument only when the plugin
must be reconfigured after registration.

## RegisterFilter

~~~cpp
PluginInstantiator::RegisterFilter<MyFilter>();
~~~

The filter must provide either Signature or a compatible SpecifySignature function.
RegisterFilter derives the name from the signature and passes the parameter list to
registerFunction.

The default output signature is clip:vnode;. A filter can provide a ReturnSignature member
when another return contract is required.

## RegisterFunction

Non-filter functions can be registered with a signature and callable object:

~~~cpp
PluginInstantiator::RegisterFunction(
    "DescribeClip(clip: vnode) -> data",
    [](auto Arguments, auto Core) {
        auto Clip = static_cast<VideoNode>(Arguments["clip"]);
        return static_cast<std::string>(Clip);
    }
);
~~~

The callable may accept no arguments, ArgumentList, or ArgumentList plus CoreProxy. A non-void
result is written to the return key derived from the signature; a void callable simply completes.

## Error behavior

Exceptions raised while constructing a filter, creating its metadata, or generating a frame are
converted to VapourSynth map or filter errors. Error messages are prefixed with the registered
function name at the callback boundary, so throw messages should describe the local failure rather
than repeat the function name.
