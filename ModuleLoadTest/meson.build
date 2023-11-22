module_load_test_exe = executable(
    'module-load-test',
    'main.c',
    dependencies: [
        openscan_dep,
        ssstr_dep,
    ],
)

test(
    'ModuleLoadTest',
    module_load_test_exe,
    args: [
        test_osdev.full_path(),
    ],
    depends: [
        test_osdev,
    ],
)
