xml2cpp_src = files('''
    xml.h
    xml.cpp
    xml2cpp.h
    xml2cpp.cpp
    generate_adaptor.cpp
    generate_adaptor.h
    generate_proxy.cpp
    generate_proxy.h
    generator_utils.cpp
    generator_utils.h
'''.split())

introspect_src = files('''
    introspect.h
    introspect.cpp
'''.split())

include = include_directories('../include')

xml2cpp = executable('dbusxx-xml2cpp',
    xml2cpp_src,
    link_with : libdbus_cpp,
    include_directories : include,
    dependencies : [expat],
    install : true,
)

introspect = executable('dbusxx-introspect',
    introspect_src,
    link_with: libdbus_cpp,
    include_directories: include,
    install: true,
)
