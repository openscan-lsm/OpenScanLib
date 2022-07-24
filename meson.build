project('OpenScanLib', 'c',
	default_options: [
		'warning_level=3',
	],
)

doxygen_prog = find_program('doxygen',
	required: get_option('docs'),
)

subdir('OpenScanDeviceLib')
subdir('OpenScanLib')
subdir('SimulatedDeviceModule')
subdir('TestDeviceModule')

if get_option('tests').enabled()
	subdir('OpenScanLibTests')
	subdir('ModuleLoadTest')
endif

if doxygen_prog.found()
	alias_target('docs',
		devicelib_doc,
		openscan_doc,
	)
endif

meson.override_dependency('OpenScanLib', openscan_dep)
meson.override_dependency('OpenScanDeviceLib', devicelib_dep)
