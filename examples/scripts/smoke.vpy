import os

import vapoursynth as vs


plugin_path = os.environ["VAPOURSYNTH_PLUSPLUS_EXAMPLE"]
core = vs.core
core.std.LoadPlugin(path=plugin_path)

clip = core.test.Palette(shades=[0.25], width=4, height=4)
frame = clip.get_frame(0)

assert frame.width == 4
assert frame.height == 4
assert abs(float(frame[0][0, 0]) - 0.25) < 1e-6

clip.set_output()
