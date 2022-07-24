test_src = [
    'OpenScanLibTests.c',
]

test_exe = executable('tests',
    test_src,
    openscan_src,
    c_args: [
        '-D_CRT_SECURE_NO_WARNINGS',
    ],
    include_directories: [
        public_inc,
        private_inc,
        public_device_inc,
        private_device_inc,
    ],
    dependencies: [
        richerrors_dep,
    ],
)

test('OpenScanLibTests', test_exe)
