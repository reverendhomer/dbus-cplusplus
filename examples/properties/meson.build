propsgs_glue = custom_target('examples.properties.glue',
    input: 'propsgs-introspect.xml',
    output: ['propsgs-glue-adaptor.h', 'propsgs-glue-proxy.h'],
    command: [xml2cpp, '@INPUT@', '--adaptor=@OUTPUT0@', '--proxy=@OUTPUT1@'],
    install: false,
    build_always: true,
    depends: xml2cpp
)
propsgs_server = executable('propsgs-server',
    ['propsgs-server.h', 'propsgs-server.cpp', propsgs_glue],
    link_with: libdbus_cpp,
    include_directories: include_directories('../../include'),
    install: false
)
propsgs_client = executable('propsgs-client',
    ['propsgs-client.h', 'propsgs-client.cpp', propsgs_glue],
    link_with: libdbus_cpp,
    include_directories: include_directories('../../include'),
    dependencies: [pthread],
    install: false
)