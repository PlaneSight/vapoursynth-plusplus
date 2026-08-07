from __future__ import annotations

import os
from collections.abc import Mapping
from pathlib import Path

import vapoursynth as vs


PLUGIN_PATH_VARIABLE = "VAPOURSYNTH_PLUSPLUS_EXAMPLE"
EXPECTED_WIDTH = 4
EXPECTED_HEIGHT = 4
EXPECTED_SHADE = 0.25
SAMPLE_TOLERANCE = 1e-6
EXPECTED_FUNCTIONS = frozenset(
    {
        "AudioGain",
        "Crop",
        "GaussBlur",
        "GaussBlurFast",
        "MaskedMerge",
        "ModifyFrame",
        "Palette",
        "PropGain",
        "Rec601ToRGB",
        "SeparableConvolution",
        "TemporalDifference",
        "TemporalMedian",
    }
)


def require_plugin_path(environment: Mapping[str, str]) -> Path:
    raw_path = environment.get(PLUGIN_PATH_VARIABLE)
    if raw_path is None:
        raise RuntimeError(f"{PLUGIN_PATH_VARIABLE} must name the built example plugin")

    plugin_path = Path(raw_path)
    if not plugin_path.is_file():
        raise RuntimeError(f"example plugin does not exist: {plugin_path}")
    return plugin_path


def require_dimension(actual: int, expected: int, description: str) -> None:
    if actual != expected:
        raise RuntimeError(f"{description}: expected {expected!r}, got {actual!r}")


def require_close(actual: float, expected: float, description: str) -> None:
    if abs(actual - expected) > SAMPLE_TOLERANCE:
        raise RuntimeError(f"{description}: expected {expected!r}, got {actual!r}")


def require_expected_functions(plugin: vs.Plugin) -> None:
    actual = frozenset(function.name for function in plugin.functions())
    if actual == EXPECTED_FUNCTIONS:
        return

    missing = sorted(EXPECTED_FUNCTIONS - actual)
    unexpected = sorted(actual - EXPECTED_FUNCTIONS)
    raise RuntimeError(
        f"unexpected example plugin exports: missing={missing!r}, unexpected={unexpected!r}"
    )


def build_verified_clip(core: vs.Core, plugin_path: Path) -> vs.VideoNode:
    core.std.LoadPlugin(path=os.fspath(plugin_path))
    plugin = core.test
    require_expected_functions(plugin)
    clip = plugin.Palette(
        shades=[EXPECTED_SHADE],
        width=EXPECTED_WIDTH,
        height=EXPECTED_HEIGHT,
    )
    frame = clip.get_frame(0)

    require_dimension(frame.width, EXPECTED_WIDTH, "frame width")
    require_dimension(frame.height, EXPECTED_HEIGHT, "frame height")
    require_close(float(frame[0][0, 0]), EXPECTED_SHADE, "first sample")
    return clip


build_verified_clip(vs.core, require_plugin_path(os.environ)).set_output()
