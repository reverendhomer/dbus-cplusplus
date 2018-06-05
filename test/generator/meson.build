test_generator_server_src = files('''
    dbuscxx_test_generator-server.h
    dbuscxx_test_generator-server.cpp
'''.split())

server_glue = custom_target('dbuscxx_test_generator-server-glue.h',
    input: 'dbuscxx_test_generator.xml',
    output: 'dbuscxx_test_generator-server-glue.h',
    command: [xml2cpp, '@INPUT@', '--adaptor=@OUTPUT@'],
    install: false,
    build_always: true,
    depends: xml2cpp,
)
test_generator_server_src += [server_glue]
generator_server = executable('dbuscxx_test_generator_server',
    test_generator_server_src,
    link_with: libdbus_cpp,
    include_directories: include_directories('../../include'),
    install: false,
)

test_generator_client_src = files('''
    dbuscxx_test_generator-client.h
    dbuscxx_test_generator-client.cpp
'''.split())
client_glue = custom_target('dbuscxx_test_generator-client-glue.h',
    input: 'dbuscxx_test_generator.xml',
    output: 'dbuscxx_test_generator-client-glue.h',
    command: [xml2cpp, '@INPUT@', '--proxy=@OUTPUT@'],
    install: false,
    build_always: true,
    depends: xml2cpp,
)
test_generator_client_src += [client_glue]
generator_client = executable('dbuscxx_test_generator_client',
    test_generator_client_src,
    link_with: libdbus_cpp,
    include_directories: include_directories('../../include'),
    install: false,
)