#pragma once

#include "Core.vxx"

struct Invert {
	field(InputClip, VideoNode{});

public:
	static constexpr auto Signature = "clip: vnode";

public:
	Invert(auto Arguments) {
		InputClip = Arguments["clip"];
		if (!InputClip.WithConstantFormat() || !InputClip.WithConstantDimensions() || !InputClip.IsSinglePrecision())
			throw std::runtime_error{
				"only single precision floating point clips with constant format and dimensions are supported."
			};
	}
	auto SpecifyMetadata() const {
		return InputClip.ExtractMetadata();
	}
	auto SpecifyDependencies() const {
		return std::array{ InputClip.SpecifyDependency(rpStrictSpatial) };
	}
	auto GenerateFrame(auto Index, auto GeneratorContext, auto Core) {
		auto InputFrame = InputClip.AcquireFrame<const float>(Index, GeneratorContext);
		auto OutputFrame = Core.CreateBlankFrameFrom(InputFrame);

		for (auto Plane : Range{ InputFrame.PlaneCount })
			for (auto y : Range{ InputFrame[Plane].Height })
				for (auto x : Range{ InputFrame[Plane].Width })
					OutputFrame[Plane][y][x] = 1.0f - InputFrame[Plane][y][x];

		return OutputFrame;
	}
};
