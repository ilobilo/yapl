project('YAPL', ['c', 'cpp'],
    version : 'v0.1',
    default_options : ['cpp_std=c++20']
)

sources = files(
    'src/main.cpp',
    'src/lexer/lexer.cpp',
    'src/parser/parser.cpp'
)

includes = include_directories('src')

cmake = import('cmake')

fmt_opts = cmake.subproject_options()
fmt_opts.add_cmake_defines({ 'FMT_INSTALL' : false })

frozen_dep = cmake.subproject('frozen').dependency('frozen')
fmt_dep = cmake.subproject('fmt', options : fmt_opts).dependency('fmt')

magic_enum_dep = dependency('magic_enum')
conflict_dep = dependency('conflict')
llvm_dep = dependency('llvm')

executable('yapl',
    dependencies : [
        magic_enum_dep,
        conflict_dep,
        frozen_dep,
        llvm_dep,
        fmt_dep
    ],
    sources : sources,
    include_directories : includes
)