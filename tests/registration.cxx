#include "VapourSynthConfig.vxx"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

struct Registration {
	std::string Name;
	std::string Arguments;
	std::string ReturnType;
};

static std::string PluginIdentifier;
static std::string PluginNamespace;
static std::string PluginDescription;
static std::vector<Registration> Registrations;

static int VS_CC ConfigurePlugin(
	const char* Identifier,
	const char* Namespace,
	const char* Description,
	int,
	int,
	int,
	VSPlugin*) noexcept {
	PluginIdentifier = Identifier;
	PluginNamespace = Namespace;
	PluginDescription = Description;
	return 1;
}

static int VS_CC RegisterFunction(
	const char* Name,
	const char* Arguments,
	const char* ReturnType,
	VSPublicFunction,
	void*,
	VSPlugin*) noexcept {
	Registrations.push_back({ Name, Arguments, ReturnType });
	return 1;
}

static VSInitPlugin LoadEntryPoint(const char* LibraryPath) {
#if defined(_WIN32)
	auto Library = LoadLibraryA(LibraryPath);
	if (Library == nullptr)
		return nullptr;
	return reinterpret_cast<VSInitPlugin>(GetProcAddress(Library, "VapourSynthPluginInit2"));
#else
	auto* Library = dlopen(LibraryPath, RTLD_NOW | RTLD_LOCAL);
	if (Library == nullptr)
		return nullptr;
	return reinterpret_cast<VSInitPlugin>(dlsym(Library, "VapourSynthPluginInit2"));
#endif
}

int main(int ArgumentCount, char** Arguments) {
	if (ArgumentCount != 2) {
		std::cerr << "usage: plugin-registration <plugin-library>\n";
		return EXIT_FAILURE;
	}

	auto EntryPoint = LoadEntryPoint(Arguments[1]);
	if (EntryPoint == nullptr) {
		std::cerr << "VapourSynthPluginInit2 could not be loaded\n";
		return EXIT_FAILURE;
	}

	VSPLUGINAPI API{
		.getAPIVersion = nullptr,
		.configPlugin = ConfigurePlugin,
		.registerFunction = RegisterFunction
	};
	EntryPoint(nullptr, &API);

	assert(PluginIdentifier == "com.vsfilterscript.test");
	assert(PluginNamespace == "test");
	assert(PluginDescription == "Test filters for vsFilterScript");
	assert(Registrations.size() == 9);

	auto Names = std::set<std::string>{};
	for (const auto& Registration : Registrations) {
		assert(!Registration.Name.empty());
		assert(!Registration.Arguments.empty());
		assert(Registration.Arguments.back() == ';');
		assert(Registration.ReturnType == "clip:vnode;");
		Names.insert(Registration.Name);
	}
	assert(Names.size() == Registrations.size());
	return EXIT_SUCCESS;
}
