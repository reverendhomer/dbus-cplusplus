project('dbus-c++',
    'cpp',
    version: '0.9.1pw'
)

conf = configuration_data()
cpp = meson.get_compiler('cpp')

dbus = dependency('dbus-1', version : '>= 0.60')
expat = dependency('expat')
pthread = dependency('threads', required : false)
conf.set10('HAVE_PTHREAD', pthread.found())

ecore = dependency('ecore', required : get_option('enable-ecore'))
enable_ecore = ecore.found()
conf.set10('ENABLE_ECORE', enable_ecore)

glib = dependency('glib-2.0', required : get_option('enable-glib'))
enable_glib = glib.found()
conf.set10('ENABLE_GLIB', enable_glib)
if enable_glib
    gtkmm = dependency('gtkmm-2.4', required : false)
    conf.set10('HAVE_GTKMM', gtkmm.found())
endif

have_clock_gettime = cpp.has_function('clock_gettime')
conf.set10('HAVE_CLOCK_GETTIME', have_clock_gettime)

enable_tests = get_option('enable-tests')
enable_examples = get_option('enable-examples')
conf.set10('ENABLE_TESTS', enable_tests)
conf.set10('ENABLE_EXAMPLES', enable_examples)


enable_doxygen_docs = get_option('enable-doxygen-docs')
if enable_doxygen_docs == 'auto'
    enable_doxygen_docs = 'no'
endif
if enable_doxygen_docs == 'yes'
    doxygen = find_program('doxygen')
    conf.set('HAVE_DOXYGEN', 1)
    conf.set('DBUS_DOXYGEN_DOCS_ENABLED', 1)
endif

subdir('src')
subdir('tools')
#subdir('data')

if enable_doxygen_docs == 'yes'
    subdir('doc')
endif
if enable_examples
    subdir('examples')
endif
if enable_tests
    subdir('test')
endif

status = [
    '@0@ @1@'.format(meson.project_name(), meson.project_version()),
    'Build information',
    '--------------------',
    '',
    'Mainloop integration support:',
    'ecore ......................... @0@'.format(enable_ecore),
    'glib........................... @0@'.format(enable_glib),
    '',
    'Optional build items:',
    'build tests.................... @0@'.format(enable_tests),
    'build examples................. @0@'.format(enable_examples),
    'build doxygen documentation.... @0@'.format(enable_doxygen_docs),
    'PThread support found.......... @0@'.format(conf.get('HAVE_PTHREAD'))
]

message('\n'.join(status))