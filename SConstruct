#!/usr/bin/env python
import os

env = SConscript("godot-cpp/SConstruct")

# Shared include paths
env.Append(CPPPATH=[
    "src/",
    "skald/include/",
    "skald/deps/pegtl/include/",
])

# MSVC lacks the POSIX `uint` typedef — define it for all targets
# that include skald.h (both skald sources and the wrapper).
if env["platform"] == "windows":
    env.Append(CPPDEFINES=[("uint", "unsigned int")])

# Skald core sources need exceptions enabled (PEGTL requires them).
# Clone the env so godot-cpp wrapper code stays exception-free.
skald_env = env.Clone()

if env["platform"] == "windows":
    skald_env.Append(CXXFLAGS=["/EHsc", "/std:c++20"])
else:
    skald_env.Append(CXXFLAGS=["-fexceptions", "-std=c++20"])

skald_sources = [
    skald_env.SharedObject("skald/src/skald.cpp"),
    skald_env.SharedObject("skald/src/debug.cpp"),
]

# GDExtension wrapper sources (no exceptions needed)
sources = Glob("src/*.cpp") + skald_sources

# Embed class reference XML into editor/debug builds (Godot 4.3+)
if env["target"] in ["editor", "template_debug"]:
    try:
        doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
        sources.append(doc_data)
    except AttributeError:
        print("Not including class reference as we're targeting a pre-4.3 baseline.")

# Build shared library into the addon directory
library = env.SharedLibrary(
    "addons/skald/bin/libskald_godot{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
    source=sources,
)

Default(library)
