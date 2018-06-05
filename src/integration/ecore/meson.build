libdbus_cpp_ecore = shared_library('libdbus-c++-ecore-1',
    'ecore-integration.cpp',
    include_directories : include_directories('../../../include'),
    link_with : libdbus_cpp,
    dependencies : [dbus, ecore],
    install : true,
    c_args : ['-Wno-unused-parameter']
)