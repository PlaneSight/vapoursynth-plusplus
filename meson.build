project('vsFilterScript', 'cpp',
    default_options : ['buildtype=release', 'b_ndebug=if-release', 'cpp_std=c++23', 'warning_level=3'],
    version : '3.0'
)

deps = dependency('vapoursynth')

incdir = include_directories('include')

test_plug = ['Examples/EntryPoint.cxx']

headers = [
	'include/AudioFrame.vxx',
	'include/AudioNode.vxx',
    'include/Core.vxx',
    'include/Frame.vxx',
	'include/Function.vxx',
	'include/Infrastructure.vxx',
	'include/Map.vxx',
	'include/Node.vxx',
	'include/Plane.vxx',
	'include/Plugin.vxx',
	'include/PluginInstantiator.vxx',
	'include/Range.vxx',
	'include/ResourceManager.vxx',
	'include/Utility.vxx',
	'include/VapourSynthConfig.vxx',
	'include/UniversalContext.vxx',
	'include/VideoFrame.vxx',
	'include/VideoNode.vxx',
	'include/Descriptors.vxx',
	'include/Enumerations.vxx',
	'include/FrameGenerator.vxx',
	'include/LoggerController.vxx',
	'include/RemappingFunctions.vxx',
]

examples = [
    'Examples/Crop.hxx',
    'Examples/EntryPoint.cxx',
    'Examples/GaussBlurFast.hxx',
    'Examples/GaussBlur.hxx',
	'Examples/ModifyFrameExample.vpy',
	'Examples/ModifyFrame.hxx',
	'Examples/Palette.hxx',
	'Examples/Rec601ToRGB.hxx',
	'Examples/SeparableConvolution.hxx',
	'Examples/TemporalMedian.hxx',
	'Examples/MaskedMerge.hxx',
]

if get_option('install_examples')
    install_data(examples, install_dir : 'share/vsFilterScript/Examples')
endif

if get_option('build_examples')
    example_plugin = shared_module('vsfilterscriptexample',
        test_plug,
        include_directories : incdir,
        dependencies : deps,
        gnu_symbol_visibility : 'hidden',
        install_dir : join_paths(get_option('libdir'), 'vapoursynth'),
        install : true
    )

    if host_machine.system() != 'windows'
        registration_test_dependencies = []
        if host_machine.system() == 'linux'
            registration_test_dependencies += meson.get_compiler('cpp').find_library('dl')
        endif
        registration_test = executable(
            'vsfilterscript-registration-test',
            'tests/registration.cxx',
            include_directories : incdir,
            dependencies : [deps] + registration_test_dependencies
        )
        test('plugin-registration', registration_test, args : [example_plugin.full_path()])
    endif
endif

format_test = executable(
    'vsfilterscript-format-test',
    'tests/compile.cxx',
    include_directories : incdir,
    dependencies : deps
)
test('format-adapter', format_test)

install_headers(headers, subdir : 'vsFilterScript')

import('pkgconfig').generate(
    name : 'vsfilterscript',
    subdirs : 'vsFilterScript',
    version : meson.project_version(),
    description : 'C++ wrapper for VSAPI to help you sketch your filter in the fastest possible way.'
)
