if doxygen_prog.found()
	openscan_doxygen_conf = configuration_data()
	openscan_doxygen_conf.set('DOXYGEN_INPUT',
		' \\\n    '.join(openscan_doxygen_inputs),
	)
	openscan_doxygen_conf.set('DOXYGEN_OUTPUT',
		meson.current_build_dir() + '/doxygen',
	)

	openscan_doxyfile = configure_file(
		configuration: openscan_doxygen_conf,
		input: 'Doxyfile.in',
		output: 'Doxyfile',
		install: false,
	)

	custom_target('openscan_doc',
		build_by_default: true,
		command: [doxygen_prog, openscan_doxyfile],
		depend_files: [openscan_documented_headers],
		output: ['doxygen'],
	)
endif
