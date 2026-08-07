#include "PluginInstantiator.vxx"
#include "GaussBlur.hxx"
#include "GaussBlurFast.hxx"
#include "TemporalMedian.hxx"
#include "Crop.hxx"
#include "Rec601ToRGB.hxx"
#include "SeparableConvolution.hxx"
#include "ModifyFrame.hxx"
#include "Palette.hxx"
#include "MaskedMerge.hxx"
#include "PropGain.hxx"
#include "TemporalDifference.hxx"
#include "AudioGain.hxx"

auto Main() {
	auto Descriptor = PluginInfo{
		.Namespace = "test",
		.Identifier = "com.vsfilterscript.test",
		.Description = "Test filters for vsFilterScript"
	};
	PluginInstantiator::SpecifyConfigurations(Descriptor);
	PluginInstantiator::RegisterFilter<GaussBlur>();
	PluginInstantiator::RegisterFilter<GaussBlurFast>();
	PluginInstantiator::RegisterFilter<TemporalMedian>();
	PluginInstantiator::RegisterFilter<Crop>();
	PluginInstantiator::RegisterFilter<Rec601ToRGB>();
	PluginInstantiator::RegisterFilter<SeparableConvolution>();
	PluginInstantiator::RegisterFilter<ModifyFrame>();
	PluginInstantiator::RegisterFilter<Palette>();
	PluginInstantiator::RegisterFilter<MaskedMerge>();
	PluginInstantiator::RegisterFilter<PropGain>();
	PluginInstantiator::RegisterFilter<TemporalDifference>();
	PluginInstantiator::RegisterFilter<AudioGain>();
}

InstantiatePluginFrom(Main);
