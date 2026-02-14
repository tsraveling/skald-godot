#!/usr/bin/env python
import os

env = SConscript("godot-cpp/SConstruct")

# Shared include paths
env.Append(CPPPATH=[
    "src/",
    "skald/include/",
    "skald/deps/pegtl/include/",
])

# Skald core sources need exceptions enabled (PEGTL requires them).
# Clone the env so godot-cpp wrapper code stays exception-free.
skald_env = env.Clone()
skald_env.Append(CXXFLAGS=["-fexceptions"])

skald_sources = [
    skald_env.SharedObject("skald/src/skald.cpp"),
    skald_env.SharedObject("skald/src/debug.cpp"),
]

# GDExtension wrapper sources (no exceptions needed)
sources = Glob("src/*.cpp") + skald_sources

# Build shared library into the addon directory
library = env.SharedLibrary(
    "addons/skald/bin/libskald_godot{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
    source=sources,
)

Default(library)
