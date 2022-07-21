devicelib_doxyfile = configure_file(
	configuration: {
		'DOXYGEN_INPUT': ' \\\n    '.join(devicelib_doxygen_inputs),
		'DOXYGEN_OUTPUT': join_paths(meson.current_build_dir(), 'apidoc'),
	},
	input: 'Doxyfile.in',
	output: 'Doxyfile',
	install: false,
)

devicelib_docdir = join_paths(
	get_option('datadir'), 'doc', 'OpenScanDeviceLib'
)

if doxygen_prog.found()
	devicelib_doc = custom_target('devicelib_doc',
		command: [doxygen_prog, devicelib_doxyfile],
		depend_files: [devicelib_documented_headers],
		output: ['apidoc'],
		install: true,
		install_dir: devicelib_docdir,
	)
endif
