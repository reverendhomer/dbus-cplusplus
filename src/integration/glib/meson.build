libdbus_cpp_glib = shared_library('libdbus-c++-glib-1',
    'glib-integration.cpp',
    link_with: libdbus_cpp,
    include_directories: include_directories('../../../include'),
    dependencies: [glib, dbus],
    install: true,
    c_args : ['-Wno-unused-parameters']
)