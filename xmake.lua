add_rules("mode.release", "mode.debug")
add_requires("libsdl","libsdl_image 2.6.2","lua 5.3")
add_requires("imgui",  {configs = {sdl2renderer = true}})
target("CasioEmuX")

set_kind("binary")
set_languages("c++17")
add_files("emulator/*.cpp","emulator/*/*.cpp")
add_packages("libsdl","libsdl_image","lua","python3.10")
set_rundir("./")
add_packages("imgui", {public = true})
    
    