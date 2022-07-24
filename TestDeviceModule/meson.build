test_osdev_src = [
	'TestDevice.c',
]

test_osdev = shared_module('TestDevice',
	test_osdev_src,
	name_suffix: 'osdev',
	c_args: [
		'-D_CRT_SECURE_NO_WARNINGS',
	],
	dependencies: [
		devicelib_dep,
	]
)
