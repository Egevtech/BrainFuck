add_rules("mode.debug", "mode.release")
    
target("bfc")
    set_kind("binary")
    add_files("src/*.c")
    set_targetdir(".")
    set_languages('c23')

    set_installdir("/usr/local/")

target("bfstd")
    set_kind("static")
    add_files("src/stdlib/*.c")
    set_targetdir(".")
    set_languages('c23')
    set_installdir("/usr/local/")
