generator_glue = custom_target('test.generator.glue',
    input: 'dbuscxx_test_generator.xml',
    output: ['dbuscxx_test_generator-server-glue.h',
             'dbuscxx_test_generator-client-glue.h'],
    command: [xml2cpp, '@INPUT@', '--adaptor=@OUTPUT0@', '--proxy=@OUTPUT1@'],
    install: false,
    build_always: true,
    depends: xml2cpp
)
generator_server = executable('dbuscxx_test_generator_server',
    ['dbuscxx_test_generator-server.h', 'dbuscxx_test_generator-server.cpp',
      generator_glue],
    link_with: libdbus_cpp,
    include_directories: include_directories('../../include'),
    install: false,
)
generator_client = executable('dbuscxx_test_generator_client',
    ['dbuscxx_test_generator-client.h', 'dbuscxx_test_generator-client.cpp',
     generator_glue],
    link_with: libdbus_cpp,
    include_directories: include_directories('../../include'),
    install: false,
)