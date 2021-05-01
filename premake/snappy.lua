
local base_dir = (solution().basedir .. "/external/snappy-1.1.9/")

group "external"
    project "snappy"
        kind "StaticLib"
        language "C++"
        location (_ACTION .. "/" .. project().name)

        includedirs
        {
            base_dir
        }
        files
        {
            base_dir .. "snappy.cc",
            base_dir .. "snappy-sinksource.cc",
            base_dir .. "snappy-stubs-internal.cc"
        }
    project "*"
group ""
