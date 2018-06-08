lib_sources = files('''
    connection.cpp
    connection_p.h
    debug.cpp
    dispatcher.cpp
    dispatcher_p.h
    error.cpp
    eventloop.cpp
    eventloop-integration.cpp
    interface.cpp
    internalerror.h
    introspection.cpp
    message.cpp
    message_p.h
    object.cpp
    pendingcall.cpp
    pendingcall_p.h
    pipe.cpp
    property.cpp
    server.cpp
    server_p.h
    types.cpp
'''.split())

lib_version = '0.0.0'
libdbus_cpp = shared_library('dbus-c++-1',
    lib_sources,
    version: lib_version,
    include_directories : include_directories('../include'),
    dependencies : [dbus, pthread],
    c_args : ['-Wno-unused-parameter'],
    install : true,
)

libdbus_cpp_ecore = shared_library('dbus-c++-ecore-1',
    'integration/ecore/ecore-integration.cpp',
    version: lib_version,
    include_directories : include_directories('../include'),
    link_with : libdbus_cpp,
    dependencies : [dbus, ecore],
    install : true,
    c_args : ['-Wno-unused-parameter']
)

libdbus_cpp_glib = shared_library('dbus-c++-glib-1',
    'integration/glib/glib-integration.cpp',
    version: lib_version,
    link_with: libdbus_cpp,
    include_directories: include_directories('../include'),
    dependencies: [dbus, glib],
    install: true,
    c_args : ['-Wno-unused-parameters']
)