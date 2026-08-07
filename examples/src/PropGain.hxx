#pragma once
#include "Core.vxx"

#include <algorithm>
#include <cmath>

struct PropGain {
	field(InputClip, VideoNode{});

public:
	static constexpr auto Signature = "clip: vnode";

public:
	PropGain(auto Arguments) {
		InputClip = Arguments["clip"];
		if (!InputClip.WithConstantFormat() || !InputClip.WithConstantDimensions() ||
			!InputClip.IsSinglePrecision() ||
			(!InputClip.IsGray() && !InputClip.IsRGB()))
			throw std::runtime_error{
				"only constant GrayS or RGBS clips are supported."
			};
	}

	auto SpecifyMetadata() {
		return InputClip.ExtractMetadata();
	}

	auto SpecifyDependencies() const {
		return std::array{ InputClip.SpecifyDependency(rpStrictSpatial) };
	}

	auto GenerateFrame(auto Index, auto GeneratorContext, auto Core) {
		auto InputFrame =
			InputClip.AcquireFrame<const float>(Index, GeneratorContext);
		auto ProcessedFrame = Core.CreateBlankFrameFrom(InputFrame);
		auto Gain = 1.0;

		if (InputFrame["Gain"].Exists())
			Gain = static_cast<double>(InputFrame["Gain"]);
		if (!std::isfinite(Gain) || Gain < 0.0)
			throw std::runtime_error{
				"the Gain frame property must be finite and non-negative."
			};

		for (auto c : Range{ InputFrame.PlaneCount })
			for (auto y : Range{ InputFrame[c].Height })
				for (auto x : Range{ InputFrame[c].Width })
					ProcessedFrame[c][y][x] = std::clamp(
						static_cast<float>(InputFrame[c][y][x] * Gain),
						0.0f, 1.0f);

		ProcessedFrame.AbsorbPropertiesFrom(InputFrame);
		ProcessedFrame["AppliedGain"] = Gain;
		return ProcessedFrame;
	}
};
