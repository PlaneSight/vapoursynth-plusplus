#include <type_traits>
#include <array>
#include <utility>
#include <vector>

#include "API4Extensions.vxx"
#include "PluginInstantiator.vxx"

struct StructuredCachePolicyProbe {
	static constexpr auto LinearAccess = true;
	static constexpr auto CachePolicy = API4::CachePolicy{
		.Mode = CacheModes::Automatic,
		.Options = API4::CacheConfiguration{ .FixedSize = 8, .MaxSize = 64, .MaxHistorySize = 4 }
	};
};

static_assert(static_cast<std::uint32_t>(VideoFormats::Gray9) == static_cast<std::uint32_t>(pfGray9));
static_assert(static_cast<std::uint32_t>(VideoFormats::Gray32) == static_cast<std::uint32_t>(pfGray32));
static_assert(static_cast<std::uint32_t>(VideoFormats::YUV420PH) == static_cast<std::uint32_t>(pfYUV420PH));
static_assert(static_cast<std::uint32_t>(VideoFormats::YUV422PS) == static_cast<std::uint32_t>(pfYUV422PS));
static_assert(static_cast<std::uint32_t>(VideoFormats::RGB36) == static_cast<std::uint32_t>(pfRGB36));
static_assert(static_cast<std::uint32_t>(VideoFormats::RGB42) == static_cast<std::uint32_t>(pfRGB42));

static_assert(static_cast<int>(CacheModes::Automatic) == cmAuto);
static_assert(static_cast<int>(CacheModes::Disabled) == cmForceDisable);
static_assert(static_cast<int>(CacheModes::Enabled) == cmForceEnable);
static_assert(static_cast<int>(DataTypeHints::Unknown) == dtUnknown);
static_assert(static_cast<int>(DataTypeHints::Binary) == dtBinary);
static_assert(static_cast<int>(DataTypeHints::Utf8) == dtUtf8);

constexpr auto StereoLayout = AudioChannelLayout::FromChannel(AudioChannels::FrontLeft).Mask |
	AudioChannelLayout::FromChannel(AudioChannels::FrontRight).Mask;
static_assert(AudioChannelLayout{ StereoLayout }.Count() == 2);
static_assert(AudioChannelLayout{ StereoLayout }.Contains(AudioChannels::FrontLeft));
static_assert(!AudioChannelLayout{ StereoLayout }.Contains(AudioChannels::BackLeft));

static_assert(requires(const Node& Node, FrameGenerator::ContextProxy Context, const FrameReference& Frame) {
	API4::SetCacheMode(Node, CacheModes::Automatic);
	API4::ConfigureCache(Node, API4::CacheConfiguration{});
	{ API4::EnableLinearAccess(Node) } -> std::same_as<int>;
	API4::ReleaseFrameEarly(Node, 0, Context);
	API4::CacheFrame(Frame, 0, Context);
	API4::ClearCache(Node);
	{ API4::QueryName(Node) } -> std::same_as<std::string_view>;
	{ API4::QueryFilterMode(Node) } -> std::same_as<VSFilterMode>;
	{ API4::QueryDependencies(Node) } -> std::same_as<std::vector<VSFilterDependency>>;
	{ API4::QueryProcessingTime(Node) } -> std::same_as<std::int64_t>;
});

static_assert(requires(const CoreProxy& Core) {
	API4::ClearCaches(Core);
	{ API4::NodeTimingEnabled(Core) } -> std::same_as<bool>;
	API4::SetNodeTiming(Core, true);
	{ API4::QueryFreedNodeProcessingTime(Core) } -> std::same_as<std::int64_t>;
	{ API4::QueryCoreInfo(Core) } -> std::same_as<API4::CoreInfo2>;
	{ Core.QueryInfo2() } -> std::same_as<CoreInfo2>;
	{ Core.NodeTimingEnabled() } -> std::same_as<bool>;
	Core.SetNodeTiming(true);
	{ Core.QueryFreedNodeProcessingTime() } -> std::same_as<std::int64_t>;
});

static_assert(requires(const Node& Node) {
	Node.SetCacheMode(CacheModes::Automatic);
	Node.ConfigureCache();
	{ Node.EnableLinearAccess() } -> std::same_as<int>;
	Node.ClearCache();
	{ Node.QueryName() } -> std::same_as<std::string_view>;
	{ Node.QueryFilterMode() } -> std::same_as<VSFilterMode>;
	{ Node.QueryDependencies() } -> std::same_as<std::vector<VSFilterDependency>>;
	{ Node.QueryProcessingTime() } -> std::same_as<std::int64_t>;
});

static_assert(requires(CoreProxy Core, FrameReference Frame, AudioFrame<void> AudioFrame, std::array<int, 2> Channels, AudioFormat Format, AudioChannelLayout Layout) {
	Core.ShuffleChannels(Frame, Channels, Format);
	Core.ShuffleChannels(AudioFrame, Channels, Layout);
});

static_assert(requires(Utility::Map::Item<false, false> Item, Node Node) {
	{ Item.IsPresent() } -> std::same_as<bool>;
	{ Item.IsEmpty() } -> std::same_as<bool>;
	{ Item.QueryDataTypeHint() } -> std::same_as<DataTypeHints>;
	Item.Consume(std::move(Node));
	Item.Convert<std::vector<std::int64_t>>();
});

static_assert(requires(const VideoFormat& Video, const AudioFormat& Audio, const Plugin& Plugin) {
	{ API4::QueryFormatName(Video) } -> std::same_as<std::string>;
	{ API4::QueryFormatName(Audio) } -> std::same_as<std::string>;
	{ API4::QueryPluginVersion(Plugin) } -> std::same_as<int>;
	{ API4::QueryFunctions(Plugin) } -> std::same_as<std::vector<API4::PluginFunctionInfo>>;
	{ Plugin.QueryVersion() } -> std::same_as<int>;
	{ Plugin.ListFunctionInfo() } -> std::same_as<std::vector<PluginFunctionInfo>>;
});

int main() {
	if (false) {
		auto Filter = StructuredCachePolicyProbe{};
		PluginInstantiator::ConfigureFilterNode(Filter, nullptr);
	}
	return 0;
}
