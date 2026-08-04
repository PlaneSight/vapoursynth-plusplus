#include "Descriptors.vxx"
#include "AudioFrame.vxx"

#include <cassert>

int main() {
	constexpr auto APIFormat = VSVideoFormat{
		.colorFamily = cfYUV,
		.sampleType = stFloat,
		.bitsPerSample = 32,
		.bytesPerSample = 4,
		.subSamplingW = 0,
		.subSamplingH = 0,
		.numPlanes = 3
	};
	auto DomainFormat = VideoFormat::AdjustToStandardLayout(APIFormat);
	assert(DomainFormat.IsYUV());
	assert(DomainFormat.IsSinglePrecision());
	assert(VideoFormat::AdjustToLegacyLayout(DomainFormat) == APIFormat);

	static_assert(SubtypeOf<AudioFrame<float>, FrameReference>);
	static_assert(SubtypeOf<AudioFrame<const float>, FrameReference>);
	static_assert(SubtypeOf<AudioFrame<void>, FrameReference>);
	return 0;
}
