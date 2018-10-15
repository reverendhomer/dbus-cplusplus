dbus_ecore_glue = custom_target('examples.ecore.glue',
    input: '../../data/org.freedesktop.DBus.xml',
    output: 'dbus_ecore-glue.h',
    command: [xml2cpp, '@INPUT@', '--proxy=@OUTPUT@'],
    install: false,
    build_always_stale: true,
    depends: xml2cpp
)
dbus_ecore = executable('dbus_ecore',
    ['dbus_ecore.cpp', 'dbus_ecore.h', dbus_ecore_glue],
    link_with: [libdbus_cpp, libdbus_cpp_ecore],
    include_directories: include_directories('../../include'),
    dependencies: [pthread, ecore],
    install: false
)
