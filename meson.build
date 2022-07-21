project('OpenScanLib', 'c')

doxygen_prog = find_program('doxygen',
	required: get_option('docs'),
)

subdir('OpenScanDeviceLib')
subdir('OpenScanLib')
subdir('OpenScanLibTests')
subdir('SimulatedDeviceModule')
subdir('TestDeviceModule')
subdir('ModuleLoadTest')

if doxygen_prog.found()
	alias_target('docs',
		devicelib_doc,
		openscan_doc,
	)
endif
