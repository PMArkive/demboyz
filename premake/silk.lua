
local base_dir = (solution().basedir .. "/external/SILK_SDK_SRC_FLP_v1.0.9//")

group "external"
    project "SKP_SILK_SDK"
        kind "StaticLib"
        language "C"
        location (_ACTION .. "/" .. project().name)

        includedirs
        {
            base_dir .. "interface/"
        }
        files
        {

            base_dir .. "src/*.c"
        }
    project "*"
group ""
