solution "demboyz"
    basedir ".."
    location (_ACTION)
    targetdir "../bin"
    startproject "demboyz"
    configurations { "Release", "Debug" }
    flags { "MultiProcessorCompile" }
    symbols "On"

    filter "configurations:Debug"
        defines { "DEBUG" }
    filter "configurations:Release"
        optimize "Full"
    filter {}

    project "demboyz"
        kind "ConsoleApp"
        language "C++"
        filter "configurations:gmake"
            buildoptions { "-std=c++17 -Wall" }
            linkoptions { "-flto -no-pie -Wall" }
        filter {}

        files
        {
            "../demboyz/**.h",
            "../demboyz/**.cpp"
        }
        includedirs
        {
            "../external/sourcesdk/include",
            "../external/celt-e18de77/include",
            "../external/SILK_SDK_SRC_FLP_v1.0.9/interface",
            "/usr/include/opus",
            "../demboyz"
        }
        links
        {
            "sourcesdk",
            "snappy",
            "celt",
            "opusenc",
            "SKP_SILK_SDK"
        }
    project "*"

    dofile "snappy.lua"
    dofile "sourcesdk.lua"
    dofile "celt.lua"
    dofile "silk.lua"
