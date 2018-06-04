project('dbus-c++',
    'c++',
    version: '0.9.1pw'
)

if get_option('enable-ecore')
    ecore = dependency('ecore')
else
    ecore = dependency('ecore', required : false)
endif