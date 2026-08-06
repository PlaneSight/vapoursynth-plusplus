#include <type_traits>

#include "API4Extensions.vxx"

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
});

static_assert(requires(const VideoFormat& Video, const AudioFormat& Audio, const Plugin& Plugin) {
	{ API4::QueryFormatName(Video) } -> std::same_as<std::string>;
	{ API4::QueryFormatName(Audio) } -> std::same_as<std::string>;
	{ API4::QueryPluginVersion(Plugin) } -> std::same_as<int>;
	{ API4::QueryFunctions(Plugin) } -> std::same_as<std::vector<API4::PluginFunctionInfo>>;
});

int main() {
	return 0;
}
