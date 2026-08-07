from __future__ import annotations

import os
from pathlib import Path

import vapoursynth as vs


PLUGIN_PATH_VARIABLE = "VAPOURSYNTH_PLUSPLUS_EXAMPLE"


def main() -> None:
    raw_path = os.environ.get(PLUGIN_PATH_VARIABLE)
    if raw_path is None:
        raise RuntimeError(f"{PLUGIN_PATH_VARIABLE} must name the built example plugin")

    plugin_path = Path(raw_path)
    if not plugin_path.is_file():
        raise RuntimeError(f"example plugin does not exist: {plugin_path}")

    core = vs.core
    core.std.LoadPlugin(path=os.fspath(plugin_path))
    source = core.std.BlankAudio(
        channels=[vs.FRONT_LEFT, vs.FRONT_RIGHT],
        sampletype=vs.FLOAT,
        bits=32,
        samplerate=48_000,
        length=256,
    )
    output = core.test.AudioGain(source, gain=0.5)
    frame = output.get_frame(0)

    if frame.sample_type != vs.FLOAT:
        raise RuntimeError("AudioGain changed the sample type")
    if frame.bits_per_sample != 32:
        raise RuntimeError("AudioGain changed the sample width")
    if frame.num_channels != 2:
        raise RuntimeError("AudioGain changed the channel count")
    if frame[0][0] != 0.0 or frame[1][0] != 0.0:
        raise RuntimeError("AudioGain changed a silent sample unexpectedly")

    print("AudioGain runtime smoke passed")


if __name__ == "__main__":
    main()
