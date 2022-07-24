module_load_test_exe = executable('module-load-test',
	'main.c',
	c_args: [
		'-D_CRT_SECURE_NO_WARNINGS',
	],
	dependencies: [
		openscan_dep,
	],
)

test('ModuleLoadTest', module_load_test_exe,
	args: [
		test_osdev.full_path(),
	],
	depends: [
		test_osdev,
	],
)
