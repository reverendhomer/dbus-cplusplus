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

libdbus_cpp = shared_library('dbus-c++-1',
    lib_sources,
    include_directories : include_directories('../include'),
    dependencies : [dbus, pthread],
    c_args : ['-Wno-unused-parameter'],
    install : true,
)