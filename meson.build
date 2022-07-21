project('OpenScanLib', 'c')

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
