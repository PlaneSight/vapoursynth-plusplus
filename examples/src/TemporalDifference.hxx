#pragma once
#include "Core.vxx"

#include <cmath>

struct TemporalDifference {
	field(InputClip, VideoNode{});

public:
	static constexpr auto Signature = "clip: vnode";

public:
	TemporalDifference(auto Arguments) {
		InputClip = Arguments["clip"];
		InputClip.OutOfBoundsRemapping = RemappingFunctions::Replicate;
		InputClip.FrameRequestor = [](auto Index) {
			return std::array{ Index - 1, Index };
		};
		if (!InputClip.WithConstantFormat() || !InputClip.WithConstantDimensions() ||
			!InputClip.IsSinglePrecision())
			throw std::runtime_error{
				"only single precision clips with constant format and dimensions "
				"are supported."
			};
	}

	auto SpecifyMetadata() {
		return InputClip.ExtractMetadata();
	}

	auto SpecifyDependencies() const {
		return std::array{ InputClip.SpecifyDependency(rpNoFrameReuse) };
	}

	auto GenerateFrame(auto Index, auto GeneratorContext, auto Core) {
		auto InputFrames =
			InputClip.AcquireFrames<const float>(Index, GeneratorContext);
		auto& PreviousFrame = InputFrames.at(-1);
		auto& CurrentFrame = InputFrames.at(0);
		auto ProcessedFrame = Core.CreateBlankFrameFrom(CurrentFrame);
		auto TotalDifference = 0.0;
		auto SampleCount = std::size_t{ 0 };

		for (auto c : Range{ CurrentFrame.PlaneCount })
			for (auto y : Range{ CurrentFrame[c].Height })
				for (auto x : Range{ CurrentFrame[c].Width }) {
					auto Difference = std::fabs(
						CurrentFrame[c][y][x] - PreviousFrame[c][y][x]);
					ProcessedFrame[c][y][x] = Difference;
					TotalDifference += Difference;
					++SampleCount;
				}

		ProcessedFrame.AbsorbPropertiesFrom(CurrentFrame);
		ProcessedFrame["DifferenceMean"] =
			TotalDifference / static_cast<double>(SampleCount);
		ProcessedFrame["ComparedFrame"] = static_cast<std::int64_t>(Index - 1);
		return ProcessedFrame;
	}
};
