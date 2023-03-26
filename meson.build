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

magic_enum_dep = dependency('magic_enum')
conflict_dep = dependency('conflict')
frozen_dep = dependency('frozen')
llvm_dep = dependency('llvm')
fmt_dep = dependency('fmt')

# ret = run_command('llvm-config', '--ldflags', '--system-libs', '--libs', 'all', capture : true, check : true)
# llvm_libs = ret.stdout().strip().replace('\n', ' ').split()

# ret = run_command('llvm-config', '--includedir', capture : true, check : true)
# llvm_inc = '-I' + ret.stdout().strip().replace('\n', ' ')

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