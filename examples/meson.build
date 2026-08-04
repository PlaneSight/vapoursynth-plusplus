project(
    'vapoursynth-plusplus-examples',
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

vapoursynth_plusplus = declare_dependency(
    dependencies : vapoursynth,
    include_directories : include_directories('../include'),
)

example_plugin = shared_module(
    'vapoursynth-plusplus-example',
    'src/EntryPoint.cxx',
    dependencies : vapoursynth_plusplus,
    gnu_symbol_visibility : 'hidden',
    install : false,
)

registration_dependencies = [vapoursynth_plusplus]
if host_machine.system() == 'linux'
    registration_dependencies += meson.get_compiler('cpp').find_library('dl')
endif

registration_test = executable(
    'example-registration-test',
    '../tests/examples/registration.cxx',
    dependencies : registration_dependencies,
)
test('example-registration', registration_test, args : [example_plugin.full_path()])
