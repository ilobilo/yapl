# Copyright (C) 2022-2024  ilobilo

project('YAPL', ['c', 'cpp'],
    version : 'v0.1',
    default_options : ['cpp_std=c++20']
)

sources = files(
    'source/main.cpp',
    'source/yapl.cpp',
    'source/lexer.cpp',
    'source/parser.cpp'
)

include = include_directories('include')

executable('yapl',
    dependencies : [
        dependency('magic_enum', default_options : [ 'test=false' ]),
        dependency('argparse'),
        dependency('llvm'),
        dependency('fmt'),
        import('cmake').subproject('frozen').dependency('frozen')
    ],
    sources : sources,
    include_directories : include,
    cpp_args : [
        '-DYAPL_VERSION="@0@"'.format(meson.project_version())
    ]
)