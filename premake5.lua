workspace "sandbox"
    architecture "x64"
    toolset "gcc"

    buildoptions "--std=c++23"

    configurations {
        "debug",
        "release",
        "dist"
    }

    filter "configurations:debug"
        defines {"_DEBUG", "_PROFILING"}
        symbols "On"
        filter {"system:linux", "action:gmake"}
            buildoptions {"-g"} -- For core files
    filter "configurations:release"
        defines {"_RELEASE"}
        optimize "On"
    filter "configurations:dist"
        defines {"_DIST"}
        optimize "On"

    filter "system:macosx"
        defines {"_MACOSX"}

    filter "system:linux"
        defines {"_LINUX"}

    filter "system:windows"
        defines {"_WINDOWS"}

outputdir = "%{cfg.buildcfg}/%{cfg.system}/%{cfg.architectrure}" 
project "sandbox"
    kind "ConsoleApp"
    language "C++"
    targetname "sandbox"
    targetdir ("sandbox/bin/" .. outputdir)
    objdir ("sandbox/bin-int/" .. outputdir)

    files {
        "sandbox/src/**.hpp",
        "sandbox/src/**.cpp",
        "sandbox/include/**.hpp",
        "sandbox/include/**.cpp",
        "sandbox/deps/src/**.cpp",
        "sandbox/deps/src/**.c",
        "sandbox/deps/include/**.h",
        "sandbox/deps/include/**.hpp"
    }

    includedirs {
        "sandbox/include",
        "sandbox/deps/include",
        "base/include"
        --"/usr/include"
    }

    links {
        "GL",
        "glfw",
        "base",
        "pthread",
        "dl",
    }

    libdirs {
        "sandbox/deps/libs",
        "base/bin/" .. outputdir
        --"/usr/lib"
    }

    postbuildcommands {
        "cp -r sandbox/res sandbox/bin/" .. outputdir,
        "cp -r base/shaders sandbox/bin/" .. outputdir
    }

project "base"
    kind "StaticLib"
    language "C++"
    targetname "base"
    targetdir ("base/bin/" .. outputdir)
    objdir ("base/bin-int/" .. outputdir)

    files {
        "base/src/**.hpp",
        "base/src/**.cpp",
        "base/include/**.hpp",
        "base/include/**.cpp",
        "base/deps/src/**.cpp",
        "base/deps/src/**.c",
        "base/deps/include/**.h",
        "base/deps/include/**.hpp"
    }

    includedirs {
        "base/include",
        "base/deps/include",
        --"/usr/include"
    }

    links {
        "GL",
        "glfw",
        "pthread",
        "dl",
    }

    libdirs {
        "base/deps/libs",
        --"/usr/lib"
    }

    postbuildcommands {
        "cp -r base/shaders base/bin/" .. outputdir,
    }

