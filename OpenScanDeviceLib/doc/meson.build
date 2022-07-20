if doxygen_prog.found()
	devicelib_doxygen_conf = configuration_data()
	devicelib_doxygen_conf.set('DOXYGEN_INPUT',
		' \\\n    '.join(devicelib_doxygen_inputs),
	)
	devicelib_doxygen_conf.set('DOXYGEN_OUTPUT',
		meson.current_build_dir() + '/doxygen',
	)

	devicelib_doxyfile = configure_file(
		configuration: devicelib_doxygen_conf,
		input: 'Doxyfile.in',
		output: 'Doxyfile',
		install: false,
	)

	custom_target('devicelib_doc',
		build_by_default: true,
		command: [doxygen_prog, devicelib_doxyfile],
		depend_files: [devicelib_documented_headers],
		output: ['doxygen'],
	)
endif
