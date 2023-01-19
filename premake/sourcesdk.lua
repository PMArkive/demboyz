
local base_dir = (solution().basedir .. "/external/sourcesdk/")
local external_dir = (solution().basedir .. "/external/")

group "external"
    project "sourcesdk"
        kind "StaticLib"
        language "C++"
        location (_ACTION .. "/" .. project().name)
        filter "configurations:gmake"
            buildoptions { "-std=c++11" }
        filter {}

        includedirs
        {
            base_dir .. "include/",
            external_dir .. "snappy-1.1.9/"
        }
        files
        {
            base_dir .. "**.h",
            base_dir .. "**.cpp"
        }
        links
        {
            "snappy"
        }
    project "*"
group ""
