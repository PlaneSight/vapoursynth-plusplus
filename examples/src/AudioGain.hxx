#pragma once
#include "Core.vxx"

#include <algorithm>
#include <cmath>

struct AudioGain {
	field(InputClip, AudioNode{});
	field(Gain, 1.0);

public:
	static constexpr auto Signature = "clip: anode, gain: float?";
	static constexpr auto ReturnSignature = "clip:anode;";

public:
	AudioGain(auto Arguments) {
		InputClip = Arguments["clip"];
		if (Arguments["gain"].Exists())
			Gain = Arguments["gain"];

		auto Metadata = InputClip.QueryAudioInfo();
		if (Metadata.Format.SampleType != SampleTypes::Float ||
			Metadata.Format.BitsPerSample != 32)
			throw std::runtime_error{
				"only 32-bit floating-point audio is supported."
			};
		if (!std::isfinite(Gain))
			throw std::runtime_error{ "gain must be finite." };
	}

	auto SpecifyMetadata() {
		return InputClip.QueryAudioInfo();
	}

	auto SpecifyDependencies() const {
		return std::array{ InputClip.SpecifyDependency(rpStrictSpatial) };
	}

	auto GenerateFrame(auto Index, auto GeneratorContext, auto Core) {
		auto InputFrame =
			InputClip.AcquireFrame<const float>(Index, GeneratorContext);
		auto ProcessedFrame = Core.CreateBlankFrameFrom(InputFrame);

		for (auto Channel : Range{ InputFrame.ChannelCount })
			for (auto Sample : Range{ InputFrame[Channel].Width })
				ProcessedFrame[Channel][0][Sample] = std::clamp(
					InputFrame[Channel][0][Sample] * static_cast<float>(Gain),
					-1.0f, 1.0f);

		return ProcessedFrame;
	}
};
