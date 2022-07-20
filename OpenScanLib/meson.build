public_inc = include_directories('include')

private_inc = include_directories('src')

openscan_src = files(
	'src/AcqTemplate.c',
	'src/Acquisition.c',
	'src/Array.c',
	'src/Device.c',
	'src/DeviceEnumeration.c',
	'src/DeviceInterface.c',
	'src/DeviceModule.c',
	'src/Error.c',
	'src/InternalErrors.c',
	'src/Logging.c',
	'src/LSM.c',
	'src/Module.c',
	'src/Setting.c',
	'src/Version.c',
)

richerrors_dep = dependency('RichErrors',
	fallback: 'RichErrors',
	static: true,
)

openscan_lib = library('OpenScanLib',
	openscan_src,
	build_by_default: true,
	c_args: [
		'-DOPENSCANLIB_EXPORTS',
	],
	include_directories: [
		public_inc,
		public_device_inc,
		private_device_inc,
	],
	dependencies: [
		richerrors_dep,
	]
)

# Dependency for applications calling OpenScanLib
openscan_dep = declare_dependency(
	link_with: openscan_lib,
	include_directories: [
		public_inc,
	],
)
