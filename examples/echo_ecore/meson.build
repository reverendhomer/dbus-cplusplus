echo_glue = custom_target('examples.echo_ecore.glue',
    input: 'echo-introspect.xml',
    output: ['echo-server-glue.h', 'echo-client-glue.h'],
    command: [xml2cpp, '@INPUT@', '--adaptor=@OUTPUT0@', '--proxy=@OUTPUT1@'],
    install: false,
    build_always_stale: true,
    depends: xml2cpp
)
echo_server = executable('echo-server',
    ['echo-server.h', 'echo-server.cpp', echo_glue],
    link_with: [libdbus_cpp, libdbus_cpp_ecore],
    include_directories: include_directories('../../include'),
    dependencies: [ecore],
    install: false
)
echo_client = executable('echo-client-mt',
    ['echo-client.h', 'echo-client.cpp', echo_glue],
    link_with: [libdbus_cpp, libdbus_cpp_ecore],
    include_directories: include_directories('../../include'),
    dependencies: [ecore, pthread],
    install: false
)
