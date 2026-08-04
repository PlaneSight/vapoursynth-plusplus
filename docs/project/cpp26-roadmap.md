---
title: C++26 adoption roadmap
description: Evaluate C++26 reflection and library facilities without weakening the C++23 portability contract.
---

# C++26 adoption roadmap

C++26 can remove real metaprogramming and portability debt from `vapoursynth-plusplus`, but it is
not yet the project's language baseline. The shipping headers remain C++23 and must compile with
the GCC, Clang, and MSVC configurations in CI.

This page records where C++26 fits, which experiments are worth maintaining, and the evidence
required before a feature enters the public API. It is a roadmap, not permission to use arbitrary
`-std=c++26` facilities in production headers.

!!! note "Status as of August 2026"
    GCC documents reflection in GCC 16 behind `-freflection`, while Clang's published C++26
    status still lists reflection, annotations, and contracts as unsupported. C++26 support is
    experimental and incomplete across the project's compiler matrix.

## Decision summary

| Feature | Project fit | Earliest use | Decision |
| --- | --- | --- | --- |
| Static reflection | Replace compiler-signature parsing; inspect opt-in schemas | Experimental adapter | Highest priority |
| Reflection annotations | Attach VapourSynth parameter metadata to fields | Experimental schema only | Promising, never infer missing semantics |
| Expansion statements | Iterate reflected members with per-member static types | With reflection experiment | Use with reflection, not alone |
| User-generated `static_assert` messages and deleted-function reasons | Improve template diagnostics | When all supported compilers implement them | Adopt early after portability proof |
| Pack indexing | Simplify isolated parameter-pack selection | Opportunistically | Low-value cleanup, not a redesign reason |
| `std::function_ref` | Express a synchronous, non-owning callback | Internal APIs with proven lifetimes | Useful but not for stored or C callbacks |
| `std::inplace_vector` | Avoid allocation for a genuinely bounded small sequence | After a domain bound is established | Do not guess a dependency limit |
| `std::simd` | Portable data-parallel example and pixel kernels | Opt-in examples and benchmarks | Never promise speed without measurement |
| Contracts | State local C++ preconditions | Experimental builds only | Never replace boundary validation or exception translation |

## Reflection: the first concrete migration

The current `Utility::Reflection::ReifyTypeNameIntoString<T>()` extracts a type spelling from
`__PRETTY_FUNCTION__` on GCC and Clang and `__FUNCSIG__` on MSVC. `PluginInstantiator` uses that
string only as the fallback from which it deduces a registered function name.

Standard reflection provides the exact operation that boundary needs:

~~~cpp
#include <meta>
#include <string_view>

template<typename Type>
consteval auto TypeIdentifier() -> std::string_view {
    return std::meta::identifier_of(^^Type);
}
~~~

This should eventually replace the compiler-specific parser. It improves portability and removes
an implementation-defined spelling dependency without changing the public filter contract.

The migration must be narrow:

1. Keep one `TypeIdentifier<T>()` adapter as the only caller-visible operation.
2. Select standard reflection with the standardized feature-test macro, not a compiler-version
   guess.
3. Retain the C++23 implementation while C++23 remains supported.
4. Compile and compare both paths for ordinary, namespaced, nested, and template filter types.
5. Remove the signature parser only after C++26 is the minimum supported standard.

!!! warning "Identifier is not a display name"
    `std::meta::identifier_of` returns an unqualified identifier when the reflected entity has one.
    `std::meta::display_string_of` is implementation-defined and must not become a stable plugin
    name, cache key, ABI identifier, or test oracle.

An explicit name in the filter signature remains authoritative. Reflection is only the fallback.
Renaming a C++ type must not silently rename an intentionally named VapourSynth function.

## Reflection-driven parameter schemas

Reflection can enumerate a type's non-static data members, recover their identifiers and types,
and access the corresponding objects through splicing. Reflection annotations can attach typed
compile-time metadata to those members. Expansion statements provide the missing typed iteration:

~~~cpp
struct Arguments {
    [[=ParameterKind::VideoNode]] VideoNode clip;
    [[=Optional, =ParameterName{"radius"}]] std::int64_t radius = 1;
};

template<typename Schema>
consteval auto ValidateSchema() -> void {
    template for (constexpr auto Member :
                  std::meta::nonstatic_data_members_of(^^Schema)) {
        // Validate the reflected member type and its VapourSynth annotations.
    }
}
~~~

This is a plausible future replacement for parts of the string-signature parser, but not for the
whole filter contract. A C++ type does not by itself state every VapourSynth property:

- the public parameter name may differ from the C++ member identifier;
- optionality is not equivalent to default construction;
- arrays, empty arrays, and scalar-or-array policies need explicit representation;
- `data`, `vnode`, `anode`, `vframe`, `aframe`, and `func` are API-domain categories;
- argument order and compatibility aliases are public behavior;
- validation such as matching formats, dimensions, or sample types is runtime domain logic.

Therefore, a reflected schema must be opt-in and fully annotated where type information is
insufficient. The existing `Signature` or `SpecifySignature()` path remains valid. A migration
tool may compare the generated signature with the explicit one at compile time before generated
signatures are allowed to become authoritative.

Do not use `std::meta::define_aggregate` merely to avoid writing a small named argument type.
Generated types are justified only if they remove repeated correctness work while preserving
readable diagnostics, stable ownership, and a simple user model.

## Compile-time diagnostics

C++26 can make failures in the current detection-heavy registration path substantially clearer.
The best early candidates are user-generated `static_assert` messages and reasons on deleted
functions. They should turn errors such as "no supported constructor" into diagnostics that name
the filter and list the accepted constructor shapes.

Reflection makes richer diagnostics possible, but diagnostics are not an excuse to multiply
overload probes. Prefer a named concept or one validation function that reports the violated
contract. Continue to test failures with intentionally invalid filters; successful compilation
alone does not verify diagnostic quality.

## Runtime-facing library facilities

### `std::function_ref`

Use `std::function_ref` only for a callable that is borrowed for the duration of a synchronous
call. It can clarify internal visitors such as map-item dispatch without allocating or owning the
callable.

It is not suitable for a VapourSynth callback stored by the C API, a deferred frame request, or any
callable whose owner may expire. Those boundaries need an owning state object or the existing
explicit C callback plus context pointer.

### `std::inplace_vector`

`std::inplace_vector<T, N>` has fixed inline capacity and never allocates. It could replace the
temporary `std::vector<VSFilterDependency>` only if the library establishes and enforces a real
maximum dependency count. Choosing a convenient `N` is not a proof: temporal and multi-input
filters can legitimately produce variable dependency sets.

Keep `std::vector` until profiling identifies allocation as material and a domain-level bound is
part of the API. An overflow exception in plugin construction is not an acceptable hidden policy.

### `std::simd`

`std::simd` belongs first in an independent example or benchmark, not the core adapter. It can
express portable vectorized pixel kernels while preserving a scalar reference implementation.
Adoption requires:

- identical results at empty, tail, alignment, stride, plane, and format boundaries;
- documented floating-point tolerance and NaN behavior;
- optimized-build benchmarks on representative frame sizes and CPUs;
- generated-code inspection for the supported compilers;
- no SIMD type or implementation-specific ABI in the public plugin interface.

The VapourSynth scheduler already supplies frame-level concurrency. A SIMD example should focus
on data parallelism inside one frame and must not introduce a competing thread pool.

## Contracts and the C ABI

C++26 contracts may eventually state local preconditions on pure C++ functions, but they do not
replace validation of user arguments, VapourSynth handles, frame formats, dimensions, or callback
state. Those are fallible runtime boundaries and require defined error behavior.

No contract violation, exception, or implementation-specific contract handler may escape a
VapourSynth C callback. `PluginInstantiator` must continue to catch C++ exceptions and translate
them into map or frame errors. Contract build modes also vary, so externally observable behavior
must not depend on whether a caller enabled contract checking.

Until GCC, Clang, and MSVC agree on the implemented facility and CI can exercise the chosen
semantics, contracts remain an experiment rather than a public-header dependency.

## Adoption gates

A C++26 facility can move through these stages independently:

| Stage | Requirements |
| --- | --- |
| Research | Accepted C++26 wording identified; ownership, ABI, diagnostics, and runtime cost assessed |
| Experiment | Isolated under `experiments/cpp26/`; explicit compiler flags; no inclusion from installed headers |
| CI probe | Compile-only job on an exact compiler version; failure does not weaken the C++23 matrix |
| Dual path | Standard feature-test macro; C++23 reference path retained; equivalent behavior tested |
| Public use | GCC, Clang, and MSVC support; required standard libraries support it; normal tests and examples pass |
| Baseline | Migration guide, major-version decision, C++23 fallback removal, strict C++26 CI on every platform |

Feature detection must test the facility, not the marketing label of the language mode. Use the
relevant SD-6 macros such as `__cpp_impl_reflection`, `__cpp_expansion_statements`, and the
corresponding library macro. `__cplusplus >= 202600L` alone is insufficient.

The baseline changes only when all of the following are true:

1. The supported GCC, Clang, and MSVC releases implement the required language features.
2. libstdc++, libc++, and the MSVC standard library implement the required library facilities.
3. Meson can select strict C++26 mode consistently on every CI platform.
4. The library, standalone example, registration test, and documentation build pass.
5. Compile-time, binary-size, and diagnostic regressions are understood.
6. Consumers receive a documented migration path and deliberate major-version boundary.

## Proposed implementation order

1. Add an opt-in reflection compile probe containing `TypeIdentifier<T>()` and no installed code.
2. Test reflected identifiers against the C++23 fallback for representative filter types.
3. Prototype an annotated parameter schema for one small example such as `Palette`; generate and
   compare its signature, but continue registering the explicit signature.
4. Evaluate compile time and diagnostics before extending the schema.
5. Add one scalar-versus-`std::simd` benchmark only after standard-library availability is broad
   enough to keep the experiment readable.
6. Reassess the C++26 baseline after all three compiler families pass the same contract.

Pack indexing, `std::function_ref`, and `std::inplace_vector` should be adopted only when a nearby
design already benefits from them. They do not justify a baseline migration by themselves.

## Primary references

- [P2996R13: Reflection for C++26](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p2996r13.html)
- [P3394R4: Annotations for Reflection](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3394r4.html)
- [P1306R5: Expansion Statements](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p1306r5.html)
- [P2900R14: Contracts for C++](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p2900r14.pdf)
- [P2662R3: Pack Indexing](https://wg21.link/p2662r3)
- [P2741R3: User-generated `static_assert` messages](https://wg21.link/p2741r3)
- [P2573R2: Deleted function definitions with a reason](https://wg21.link/p2573r2)
- [P0792R14: `function_ref`](https://wg21.link/p0792r14)
- [P0843R14: `inplace_vector`](https://wg21.link/p0843r14)
- [P1928R15: `simd`](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p1928r15.pdf)
- [Clang C++ implementation status](https://clang.llvm.org/cxx_status.html)
- [GCC C++ implementation status](https://gcc.gnu.org/projects/cxx-status.html)

The [C++23 portability contract](cpp23-and-portability.md) remains normative until the baseline
gate above is deliberately completed.
