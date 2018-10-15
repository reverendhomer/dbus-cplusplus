dbus_browser_glue = custom_target('examples.glib.glue',
    input: '../../data/org.freedesktop.DBus.xml',
    output: 'dbus-glue.h',
    command: [xml2cpp, '@INPUT@', '--proxy=@OUTPUT@'],
    install: false,
    build_always_stale: true,
    depends: xml2cpp
)
dbus_browser = executable('dbus-browser',
    ['dbus-browser.cpp',
     'dbus-browser.h',
     '../../tools/xml.cpp',
     dbus_browser_glue],
    link_with: [libdbus_cpp, libdbus_cpp_glib],
    include_directories: include_directories('../../include', '../../tools'),
    dependencies: [gtkmm, expat],
    install: false
)
