openscan_doxyfile = configure_file(
    configuration: {
        'DOXYGEN_INPUT': ' \\\n    '.join(openscan_doxygen_inputs),
        'DOXYGEN_OUTPUT': join_paths(meson.current_build_dir(), 'apidoc'),
    },
    input: 'Doxyfile.in',
    output: 'Doxyfile',
    install: false,
)

openscan_docdir = join_paths(get_option('datadir'), 'doc', 'OpenScanLib')

if doxygen_prog.found()
    openscan_doc = custom_target(
        'openscan_doc',
        command: [doxygen_prog, openscan_doxyfile],
        depend_files: [openscan_documented_headers],
        output: ['apidoc'],
        install: true,
        install_dir: openscan_docdir,
    )
endif
