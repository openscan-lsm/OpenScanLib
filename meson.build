project(
    'OpenScanLib',
    'c',
    default_options: [
        'warning_level=3',
    ],
)

doxygen_prog = find_program(
    'doxygen',
    required: get_option('docs'),
)

if get_option('devicelib').enabled() or get_option('tests').enabled()
    subdir('OpenScanDeviceLib')
    meson.override_dependency('OpenScanDeviceLib', devicelib_dep)
endif

if get_option('apilib').enabled() or get_option('tests').enabled()
    subdir('OpenScanLib')
    meson.override_dependency('OpenScanLib', openscan_dep)
endif

if get_option('tests').enabled()
    subdir('OpenScanLibTests')
    subdir('SimulatedDeviceModule')
    subdir('TestDeviceModule')
    subdir('ModuleLoadTest')
endif

if doxygen_prog.found()
    alias_target(
        'docs',
        devicelib_doc,
        openscan_doc,
    )
endif
