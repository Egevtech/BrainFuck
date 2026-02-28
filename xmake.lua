add_rules("mode.debug", "mode.release")

target("bfc")
set_kind("binary")
add_files("src/*.c")
set_targetdir(".")
add_defines('BFSTD="./libbfstd.a"')

target("bfstd")
set_kind("static")
add_files("src/stdlib/*.c")
set_targetdir(".")
