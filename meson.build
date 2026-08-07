project(
    'vapoursynth-plusplus',
    'cpp',
    version : '3.0',
    default_options : [
        'b_ndebug=if-release',
        'buildtype=release',
        'cpp_std=c++23,c++latest',
        'warning_level=3',
    ],
)

sdk_include_dir = get_option('vapoursynth_include_dir')
if sdk_include_dir == ''
    vapoursynth = dependency('vapoursynth')
else
    vapoursynth = declare_dependency(
        include_directories : include_directories(sdk_include_dir, is_system : true),
    )
endif

public_headers = files(
    'include/API4Extensions.vxx',
    'include/AudioFrame.vxx',
    'include/AudioNode.vxx',
    'include/Core.vxx',
    'include/Descriptors.vxx',
    'include/Enumerations.vxx',
    'include/Frame.vxx',
    'include/FrameGenerator.vxx',
    'include/Function.vxx',
    'include/Infrastructure.vxx',
    'include/LoggerController.vxx',
    'include/Map.vxx',
    'include/Node.vxx',
    'include/Plane.vxx',
    'include/Plugin.vxx',
    'include/PluginInstantiator.vxx',
    'include/Range.vxx',
    'include/RemappingFunctions.vxx',
    'include/ResourceManager.vxx',
    'include/UniversalContext.vxx',
    'include/Utility.vxx',
    'include/VapourSynthConfig.vxx',
    'include/VideoFrame.vxx',
    'include/VideoNode.vxx',
)

vapoursynth_plusplus = declare_dependency(
    dependencies : vapoursynth,
    include_directories : include_directories('include'),
)

format_test = executable(
    'vapoursynth-plusplus-format-test',
    'tests/library/format_adapter.cxx',
    dependencies : vapoursynth_plusplus,
)
test('format-adapter', format_test)

api42_surface_test = executable(
    'vapoursynth-plusplus-api42-surface-test',
    'tests/library/api42_surface.cxx',
    dependencies : vapoursynth_plusplus,
)
test('api42-surface', api42_surface_test)

install_headers(public_headers, subdir : 'vsFilterScript')

import('pkgconfig').generate(
    name : 'vsfilterscript',
    filebase : 'vsfilterscript',
    subdirs : 'vsFilterScript',
    version : meson.project_version(),
    description : 'C++23 layer for authoring VapourSynth API 4 filters.',
)
