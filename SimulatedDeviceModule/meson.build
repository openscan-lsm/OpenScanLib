simulated_osdev_src = [
    'SimulatedDevice.c',
]

legacy_simulated_osdev_src = [
    'SimulatedDevice_Legacy.c',
]

simulated_osdev = shared_module(
    'SimulatedDevice',
    simulated_osdev_src,
    name_suffix: 'osdev',
    c_args: [
        '-D_CRT_SECURE_NO_WARNINGS',
    ],
    dependencies: [
        devicelib_dep,
    ],
)

legacy_simulated_osdev = shared_module(
    'SimulatedDevice-Legacy',
    legacy_simulated_osdev_src,
    name_suffix: 'osdev',
    c_args: [
        '-D_CRT_SECURE_NO_WARNINGS',
    ],
    dependencies: [
        devicelib_dep,
    ],
)
