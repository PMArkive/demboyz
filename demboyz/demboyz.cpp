
#include "io/demoreader.h"
#include "game/sourcecontext.h"
#include <cstdio>
#include <string>
#include <cassert>
#include <filesystem>

int main(const int argc, const char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <in>.dem [in2.dem] ...\n", argv[0]);
        return -1;
    }

    DemoReader::Init();
    SourceGameContext *context = nullptr;

    bool error = false;
    for (int i = 1; i < argc; i++)
    {
        std::filesystem::path inputFile(argv[i]);
        FILE* inputFp = fopen(inputFile.c_str(), "rb");
        if (!inputFp)
        {
            fprintf(stderr, "Error: Could not open input file\n");
            return -1;
        }

        static std::filesystem::path outputDir = inputFile.filename().replace_extension();
        static std::filesystem::path outputDirVoice = outputDir.string() + "/voice";
        if (i == 1)
        {
            std::filesystem::create_directory(outputDir);
            std::filesystem::create_directory(outputDirVoice);

            context = new SourceGameContext(outputDir, outputDirVoice);
            if(!context->init())
                return -1;
        }

        bool dirty = DemoReader::ProcessDem(inputFp, context);
        if(dirty)
            error = true;

        fclose(inputFp);
    }

    context->End();
    delete context;

    DemoReader::DeInit();

    return error;
}
