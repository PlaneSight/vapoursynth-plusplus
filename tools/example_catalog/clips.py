from __future__ import annotations

import os
import warnings
from collections.abc import Sequence
from pathlib import Path

import vapoursynth as vs

from .model import HEIGHT, TILE_SIZE, WIDTH, FrameSelection
from .png import clamp_unit


SAMPLE_TOLERANCE = 1e-6


def blank(
    core: vs.Core,
    format_id: int,
    color: Sequence[float],
    *,
    width: int,
    height: int,
) -> vs.VideoNode:
    return core.std.BlankClip(
        width=width,
        height=height,
        format=format_id,
        color=color,
        length=1,
    )


def horizontal_stripes(
    core: vs.Core,
    format_id: int,
    colors: Sequence[Sequence[float]],
    *,
    height: int = HEIGHT,
) -> vs.VideoNode:
    strip_width = WIDTH // len(colors)
    strips = [
        blank(core, format_id, color, width=strip_width, height=height)
        for color in colors
    ]
    return core.std.StackHorizontal(clips=strips)


def checkerboard(core: vs.Core, *, inverted: bool = False) -> vs.VideoNode:
    dark, light = (0.12, 0.88) if not inverted else (0.88, 0.12)
    columns = WIDTH // TILE_SIZE
    rows = HEIGHT // TILE_SIZE

    first_row = core.std.StackHorizontal(
        clips=[
            blank(
                core,
                vs.GRAYS,
                [light if column % 2 == 0 else dark],
                width=TILE_SIZE,
                height=TILE_SIZE,
            )
            for column in range(columns)
        ]
    )
    second_row = core.std.StackHorizontal(
        clips=[
            blank(
                core,
                vs.GRAYS,
                [dark if column % 2 == 0 else light],
                width=TILE_SIZE,
                height=TILE_SIZE,
            )
            for column in range(columns)
        ]
    )
    return core.std.StackVertical(
        clips=[first_row if row % 2 == 0 else second_row for row in range(rows)]
    )


def rgb_stripe_colors() -> tuple[tuple[float, float, float], ...]:
    colors: list[tuple[float, float, float]] = []
    for index in range(15):
        position = index / 14
        colors.append(
            (
                clamp_unit(0.10 + 0.82 * position),
                clamp_unit(0.18 + 0.70 * (1.0 - abs(2.0 * position - 1.0))),
                clamp_unit(0.88 - 0.72 * position),
            )
        )
    return tuple(colors)


def yuv_source(core: vs.Core) -> tuple[vs.VideoNode, vs.VideoNode, vs.VideoNode]:
    rgb_colors = rgb_stripe_colors()
    yuv_colors: list[tuple[float, float, float]] = []
    for red, green, blue in rgb_colors:
        luminance = 0.299 * red + 0.587 * green + 0.114 * blue
        yuv_colors.append(
            (
                luminance,
                (blue - luminance) / (2.0 * (1.0 - 0.114)),
                (red - luminance) / (2.0 * (1.0 - 0.299)),
            )
        )

    rgb = horizontal_stripes(core, vs.RGBS, rgb_colors)
    yuv_preview = horizontal_stripes(
        core,
        vs.RGBS,
        tuple(
            (luminance, chroma_blue + 0.5, chroma_red + 0.5)
            for luminance, chroma_blue, chroma_red in yuv_colors
        ),
    )
    yuv = horizontal_stripes(core, vs.YUV444PS, yuv_colors)

    def describe_rec601(n: int, f: vs.VideoFrame) -> vs.VideoFrame:
        del n
        output = f.copy()
        output.props._Matrix = 6
        with warnings.catch_warnings():
            warnings.filterwarnings(
                "ignore",
                message="The _ColorRange frame property has been deprecated.*",
                category=DeprecationWarning,
            )
            output.props._ColorRange = 0
        return output

    yuv = core.std.ModifyFrame(clip=yuv, clips=yuv, selector=describe_rec601)
    return yuv, rgb, yuv_preview


def invert_frame(src: vs.VideoFrame) -> vs.VideoFrame:
    output = src.copy()
    for plane_index in range(output.format.num_planes):
        plane = output[plane_index]
        for y in range(output.height):
            for x in range(output.width):
                plane[y, x] = 1.0 - float(plane[y, x])
    return output


def require_close(actual: float, expected: float, description: str) -> None:
    if abs(actual - expected) > SAMPLE_TOLERANCE:
        raise RuntimeError(f"{description}: expected {expected!r}, got {actual!r}")


def require_matching_frames(
    left: vs.VideoNode,
    right: vs.VideoNode,
    *,
    description: str,
    ignore_border: bool = False,
) -> None:
    left_frame = left.get_frame(0)
    right_frame = right.get_frame(0)
    if left_frame.width != right_frame.width or left_frame.height != right_frame.height:
        raise RuntimeError(f"{description} returned different dimensions")

    inset = 1 if ignore_border else 0
    for plane_index in range(left_frame.format.num_planes):
        for y in range(inset, left_frame.height - inset):
            for x in range(inset, left_frame.width - inset):
                require_close(
                    float(left_frame[plane_index][y, x]),
                    float(right_frame[plane_index][y, x]),
                    f"{description} at plane={plane_index}, x={x}, y={y}",
                )


def build_selections(core: vs.Core, plugin_path: Path) -> Sequence[FrameSelection]:
    core.std.LoadPlugin(path=os.fspath(plugin_path))
    plugin = core.test

    gray = checkerboard(core)
    rgb = horizontal_stripes(core, vs.RGBS, rgb_stripe_colors())

    gauss = plugin.GaussBlur(clip=gray)
    gauss_fast = plugin.GaussBlurFast(clip=gray)
    require_matching_frames(
        gauss,
        gauss_fast,
        description="GaussBlur interior parity",
        ignore_border=True,
    )

    crop = plugin.Crop(clip=rgb, left=32, right=16, top=16, bottom=16)
    if crop.width != WIDTH - 48 or crop.height != HEIGHT - 32:
        raise RuntimeError("Crop returned unexpected output dimensions")

    temporal = core.std.Splice(clips=[gray, checkerboard(core, inverted=True), gray])
    temporal_median = plugin.TemporalMedian(clip=temporal, radius=1)

    foreground = horizontal_stripes(
        core,
        vs.RGBS,
        tuple(reversed(rgb_stripe_colors())),
    )
    mask = horizontal_stripes(
        core,
        vs.GRAYS,
        tuple([clamp_unit(1.0 - abs(index - 7) / 5.0)] for index in range(15)),
    )
    merged = plugin.MaskedMerge(clipa=rgb, clipb=foreground, mask=mask)

    modified = plugin.ModifyFrame(clip=rgb, evaluator=invert_frame)
    convolved = plugin.SeparableConvolution(
        clip=gray,
        h_kernel=[1.0, 2.0, 1.0],
        v_kernel=[1.0, 2.0, 1.0],
    )
    yuv, rgb_reference, yuv_preview = yuv_source(core)
    converted = plugin.Rec601ToRGB(clip=yuv)
    require_matching_frames(
        converted,
        rgb_reference,
        description="Rec601ToRGB reference conversion",
    )

    palette = plugin.Palette(
        shades=[0.08, 0.28, 0.50, 0.72, 0.92],
        width=WIDTH,
        height=HEIGHT,
    )

    return (
        FrameSelection("palette-first.png", palette, 0),
        FrameSelection("palette-last.png", palette, 4),
        FrameSelection("gauss-blur-before.png", gray),
        FrameSelection("gauss-blur-after.png", gauss),
        FrameSelection("gauss-blur-fast-before.png", gray),
        FrameSelection("gauss-blur-fast-after.png", gauss_fast),
        FrameSelection("crop-before.png", rgb),
        FrameSelection("crop-after.png", crop),
        FrameSelection("temporal-median-before.png", temporal, 1),
        FrameSelection("temporal-median-after.png", temporal_median, 1),
        FrameSelection("masked-merge-before.png", rgb),
        FrameSelection("masked-merge-after.png", merged),
        FrameSelection("modify-frame-before.png", rgb),
        FrameSelection("modify-frame-after.png", modified),
        FrameSelection("separable-convolution-before.png", gray),
        FrameSelection("separable-convolution-after.png", convolved),
        FrameSelection("rec601-to-rgb-before.png", yuv_preview),
        FrameSelection("rec601-to-rgb-after.png", converted),
    )
