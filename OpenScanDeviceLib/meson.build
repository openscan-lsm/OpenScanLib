public_device_inc = include_directories('include')

private_device_inc = include_directories('.')

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
