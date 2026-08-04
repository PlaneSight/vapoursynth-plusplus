#include "Descriptors.vxx"
#include "AudioFrame.vxx"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {
auto Require(bool Condition, const char* Message) -> bool {
	if (Condition)
		return true;

	std::cerr << Message << '\n';
	return false;
}
}

int main() {
	const auto ParsedSignature = Utility::SignatureParser::ExtractParameterList(
		"Filter(clip: vnode, frame: vframe, label: string, flag: bool?)");
	if (!Require(
			ParsedSignature == "clip:vnode;frame:vframe;label:data;flag:int:opt;",
			"signature parser must preserve API 4 node and frame types")) {
		return EXIT_FAILURE;
	}

	constexpr auto APIFormat = VSVideoFormat{
		.colorFamily = cfYUV,
		.sampleType = stFloat,
		.bitsPerSample = 32,
		.bytesPerSample = 4,
		.subSamplingW = 0,
		.subSamplingH = 0,
		.numPlanes = 3
	};
	const auto DomainFormat = VideoFormat::AdjustToStandardLayout(APIFormat);

	if (!Require(DomainFormat.IsYUV(), "standard format must preserve the YUV family") ||
		!Require(DomainFormat.IsSinglePrecision(), "standard format must preserve float32 samples") ||
		!Require(VideoFormat::AdjustToLegacyLayout(DomainFormat) == APIFormat, "format conversion must round trip")) {
		return EXIT_FAILURE;
	}

	static_assert(SubtypeOf<AudioFrame<float>, FrameReference>);
	static_assert(SubtypeOf<AudioFrame<const float>, FrameReference>);
	static_assert(SubtypeOf<AudioFrame<void>, FrameReference>);
	return EXIT_SUCCESS;
}
