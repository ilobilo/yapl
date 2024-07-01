set_project("yapl")
set_version("v0.1")

add_rules("mode.release")
add_rules("plugin.compile_commands.autoupdate", { outputdir = "build" })

add_requires("magic_enum", "argparse", "frozen master", "fmt master")
add_requires("llvm", { system = true, kind = "library" })

target("yapl")
    set_kind("binary")

    add_packages("magic_enum", "argparse", "frozen", "fmt", "llvm")
    add_links("LLVM")

    add_files("source/*.cpp")
    add_includedirs("include/")

    set_languages("c++20")
    set_warnings("all", "error")
    set_optimize("fastest")

    on_config(function (target)
        target:add("defines", "YAPL_VERSION=\"" .. target:version() .. "\"")
    end)