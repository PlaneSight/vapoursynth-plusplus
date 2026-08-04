from __future__ import annotations

import argparse
import os
from collections.abc import Mapping, Sequence
from pathlib import Path
from typing import assert_never

import vapoursynth as vs

from .clips import build_selections
from .model import (
    DEFAULT_OUTPUT_DIRECTORY,
    PLUGIN_PATH_VARIABLE,
    CatalogCommand,
    CatalogOperation,
    CheckAssets,
    FrameSelection,
    GenerateAssets,
)
from .png import decode_png, encode_png, render_frame


def parse_arguments() -> CatalogCommand:
    parser = argparse.ArgumentParser(
        description="Generate deterministic images from the example VapourSynth plugin."
    )
    parser.add_argument(
        "--plugin",
        type=Path,
        help=f"built plugin path; defaults to ${PLUGIN_PATH_VARIABLE}",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=DEFAULT_OUTPUT_DIRECTORY,
        help=f"catalog asset directory (default: {DEFAULT_OUTPUT_DIRECTORY})",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="compare rendered pixels with committed assets without rewriting them",
    )
    arguments = parser.parse_args()

    plugin_path = arguments.plugin
    output_directory = arguments.output
    if not isinstance(plugin_path, (Path, type(None))):
        raise RuntimeError("argparse returned an invalid plugin path")
    if not isinstance(output_directory, Path):
        raise RuntimeError("argparse returned an invalid output directory")
    operation: CatalogOperation = CheckAssets() if arguments.check else GenerateAssets()
    return CatalogCommand(plugin_path, output_directory, operation)


def resolve_plugin_path(explicit_path: Path | None, environment: Mapping[str, str]) -> Path:
    candidate = explicit_path
    if candidate is None:
        raw_path = environment.get(PLUGIN_PATH_VARIABLE)
        if raw_path is None:
            raise RuntimeError(
                f"pass --plugin or set {PLUGIN_PATH_VARIABLE} to the built example plugin"
            )
        candidate = Path(raw_path)

    plugin_path = candidate.resolve()
    if not plugin_path.is_file():
        raise RuntimeError(f"example plugin does not exist: {plugin_path}")
    return plugin_path


def write_or_check(
    selections: Sequence[FrameSelection],
    output_directory: Path,
    *,
    operation: CatalogOperation,
) -> None:
    rendered = {selection.filename: render_frame(selection) for selection in selections}
    expected_names = frozenset(rendered)

    match operation:
        case CheckAssets():
            actual_names = frozenset(
                path.name for path in output_directory.glob("*.png")
            )
            if actual_names != expected_names:
                missing = sorted(expected_names - actual_names)
                unexpected = sorted(actual_names - expected_names)
                raise RuntimeError(
                    f"catalog asset set differs: missing={missing!r}, "
                    f"unexpected={unexpected!r}"
                )
            for filename, expected in rendered.items():
                actual = decode_png((output_directory / filename).read_bytes())
                if actual != expected:
                    raise RuntimeError(
                        f"catalog asset is stale: {output_directory / filename}; "
                        "regenerate it"
                    )
        case GenerateAssets():
            output_directory.mkdir(parents=True, exist_ok=True)
            for stale_path in output_directory.glob("*.png"):
                if stale_path.name not in expected_names:
                    stale_path.unlink()
            for filename, image in rendered.items():
                (output_directory / filename).write_bytes(encode_png(image))
        case unreachable:
            assert_never(unreachable)


def main() -> None:
    command = parse_arguments()
    plugin_path = resolve_plugin_path(command.plugin_path, os.environ)
    selections = build_selections(vs.core, plugin_path)
    write_or_check(
        selections,
        command.output_directory,
        operation=command.operation,
    )


if __name__ == "__main__":
    main()
