# Example catalog generator

This package is the executable source for the documentation images under
`docs/assets/example-catalog/`. It constructs deterministic clips with
VapourSynth, loads the built example plugin, requests output from all nine
example filters, and encodes the selected frames as RGB PNGs.

From the repository root, after building the plugin against the pinned runtime:

```bash
uv run --group runtime-test python -m tools.example_catalog \
    --plugin build-runtime/libvapoursynth-plusplus-example.so
```

Use the same command with `--check` to compare regenerated dimensions and RGB
pixels with the committed assets without rewriting them. This semantic check is
independent of PNG compression details and is the mode run by CI.

The synthetic sources are assembled from VapourSynth nodes rather than input
media, so generation requires no downloads, codecs, GPU, or display server.
The harness also verifies algorithm-level contracts that are not visible in a
successful render, including blur parity away from the deliberately different
border policies and Rec. 601 conversion against independently derived samples.
