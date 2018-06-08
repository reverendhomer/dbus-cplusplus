headers = files('''
    dbus-c++/api.h
    dbus-c++/connection.h
    dbus-c++/dbus.h
    dbus-c++/debug.h
    dbus-c++/dispatcher.h
    dbus-c++/error.h
    dbus-c++/eventloop-integration.h
    dbus-c++/eventloop.h
    dbus-c++/interface.h
    dbus-c++/introspection.h
    dbus-c++/message.h
    dbus-c++/object.h
    dbus-c++/pendingcall.h
    dbus-c++/pipe.h
    dbus-c++/property.h
    dbus-c++/refptr_impl.h
    dbus-c++/server.h
    dbus-c++/types.h
    dbus-c++/util.h
'''.split())
if enable_ecore
    headers += ['dbus-c++/ecore-integration.h']
endif
if enable_glib
    headers += ['dbus-c++/glib-integration.h']
endif
install_headers(headers, subdir: 'dbus-c++-1/dbus-c++')