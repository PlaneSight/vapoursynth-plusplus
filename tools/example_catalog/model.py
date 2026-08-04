from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

import vapoursynth as vs


WIDTH = 240
HEIGHT = 128
TILE_SIZE = 16
PLUGIN_PATH_VARIABLE = "VAPOURSYNTH_PLUSPLUS_EXAMPLE"
DEFAULT_OUTPUT_DIRECTORY = Path("docs/assets/example-catalog")


@dataclass(frozen=True, slots=True)
class FrameSelection:
    filename: str
    clip: vs.VideoNode
    index: int = 0


@dataclass(frozen=True, slots=True)
class RenderedImage:
    width: int
    height: int
    pixels: bytes


@dataclass(frozen=True, slots=True)
class GenerateAssets:
    pass


@dataclass(frozen=True, slots=True)
class CheckAssets:
    pass


type CatalogOperation = GenerateAssets | CheckAssets


@dataclass(frozen=True, slots=True)
class CatalogCommand:
    plugin_path: Path | None
    output_directory: Path
    operation: CatalogOperation
