public_device_inc = include_directories('include')

private_device_inc = include_directories('.')

devicelib_documented_headers = files(
	'include/OpenScanDeviceLib.h',
	'OpenScanDeviceLibPrivate.h',
)

devicelib_doxygen_inputs = [
	join_paths(meson.current_source_dir(), 'include/OpenScanDeviceLib.h'),
	join_paths(meson.current_source_dir(), 'OpenScanDeviceLibPrivate.h'),
]

device_lib = static_library('OpenScanDeviceLib',
	'OpenScanDeviceLib.c',
	build_by_default: true,
	include_directories: [
		public_device_inc,
	],
)

# Dependency to be used by device modules
devicelib_dep = declare_dependency(
	link_with: device_lib,
	include_directories: [
		public_device_inc,
	]
)

subdir('doc')
