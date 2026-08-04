---
title: C++23 and compiler portability
description: The C++23 language baseline, portability rules, and compiler verification contract.
---

# C++23 and compiler portability

`vapoursynth-plusplus` treats C++23 as part of its public contract. It is a header-oriented C++
layer over VapourSynth API 4, so compiler behavior is part of the library's compatibility surface:
the same templates, concepts, callback bridges, and ownership wrappers must be accepted by every
supported compiler.

## Support contract

| Boundary | Required contract |
| --- | --- |
| Language | C++23 |
| VapourSynth SDK | API 4.2 or newer |
| API header | `VapourSynth4.h`, selected by `VapourSynthConfig.vxx` |
| Linux CI | GCC and Clang with `-std=c++23` |
| Windows CI | MSVC with `/std:c++23preview` |
| Warning policy | GCC and Clang `-Wall -Wextra -Wpedantic`; MSVC `/W4 /permissive-` |

The API requirement is enforced in the public configuration header:

~~~cpp
#define VS_USE_API_42
#include "VapourSynth4.h"

static_assert(VAPOURSYNTH_API_MAJOR == 4);
static_assert(VAPOURSYNTH_API_MINOR >= 2);
~~~

The language requirement is enforced by the build and CI configuration. A compiler that accepts
the headers only as an extension is not a supported build.

## Why the baseline is C++23

The library's public headers use C++23 as a coherent foundation for several related techniques:

- constrained templates, concepts, and `requires` expressions for compile-time interface recognition;
- abbreviated function templates and deduced parameters for the filter contract;
- designated initialization for small domain records and callback state;
- `consteval` helpers for signature and type-name work;
- the `z` and `uz` integer literal suffixes for `std::ptrdiff_t` and `std::size_t` values;
- standard type traits and concepts such as `std::constructible_from` and `std::default_initializable`.

Some of these facilities originate in C++20, but the project does not present a smaller language
subset as an alternative baseline. In particular, the `0z` and `0uz` literals require C++23.

## Portability rules

The following rules came from compiling the API 4 adapter with GCC, Clang, and MSVC. They are
design constraints for new public headers, not compiler-specific workarounds to copy blindly.

| Failure pattern | Portable rule |
| --- | --- |
| A deduced return type is used before its definition | Give the declaration an explicit return type when another template needs it before the definition is visible. |
| Alias-template class-template argument deduction differs between compilers | Name the concrete owner or wrapper type at deduction boundaries; do not make alias-template CTAD part of the public contract. |
| An unconstrained `operator auto()` participates in rewritten comparisons or conversions | Use an explicit conversion target or a constrained conversion template with a declared return type. |
| A nested lambda `requires` expression recursively probes the type being constructed | Prefer standard concepts and type traits such as `std::constructible_from` and `std::default_initializable`. |
| Returning or copying an indexed wrapper re-enters its own conversion constraints | Construct the wrapper directly from its shared state and index, with an explicit return type. |
| Type-name extraction assumes one compiler's function-signature spelling | Keep compiler branches together and extract markers with bounded, documented rules for Clang/GCC and MSVC. |

These rules preserve the library's existing abstractions. `ResourceManager::Owner`, typed map
items, the two-phase frame-acquisition model, signature deduction, and `PluginInstantiator` remain
the design surface; portability work belongs at the places where the implementation meets a
compiler or the C API.

## Keep the C API boundary narrow

VapourSynth callbacks use a C ABI even though filters are implemented as C++ types. The adapter
keeps the unsafe boundary in `PluginInstantiator.vxx`:

1. `VapourSynthPluginInit2` receives the plugin handle and API table.
2. The adapter installs the table for the current callback.
3. C++ argument and filter objects are constructed behind the callback bridge.
4. C++ exceptions are caught and translated into VapourSynth map errors.
5. Owned nodes and frames are transferred explicitly to the API.

No exception may escape through a VapourSynth callback. No borrowed API pointer should outlive the
callback that supplied it. C++23 makes the wrapper expressive, but it does not change the C ABI's
ownership or failure rules.

## Verification contract

The Build workflow checks more than whether one translation unit parses:

1. Compile the public adapter and examples with each compiler.
2. Run the format-adapter test.
3. Build the example plugin as a shared library.
4. Load the library and invoke `VapourSynthPluginInit2` with the registration test.
5. Verify the exported `VapourSynthPluginInit2` symbol.

The registration test exercises the plugin descriptor and registered function signatures without
requiring a full VapourSynth host on the CI worker. A real host-level integration test remains a
separate concern from compiler conformance.

## Contributor checklist

Before adding or changing a public header:

- compile it in `-std=c++23` mode; do not rely on an extension accepted by one compiler;
- include it through the same public include path used by a consumer;
- give public or cross-template return types an explicit type when lookup order matters;
- use standard concepts and traits before inventing a nested detection mechanism;
- keep compiler-specific reflection or ABI code in a narrow adapter;
- test both ownership behavior and the generated C callback boundary;
- run the same format, registration, shared-library, and export checks used by CI.

The [installation guide](../getting-started/installation.md) covers SDK discovery and the normal
build. The [registration reference](../reference/registration.md) documents the C++ filter-to-API
adapter that this portability contract protects. The [C++26 adoption roadmap](cpp26-roadmap.md)
records how reflection and other future facilities can enter without weakening this baseline.
