
#include "io/demoreader.h"
#include "game/sourcecontext.h"
#include <cstdio>
#include <string>
#include <cassert>
#include <filesystem>

int main(const int argc, const char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <in>.dem\n", argv[0]);
        return -1;
    }

    std::filesystem::path inputFile(argv[1]);
    FILE* inputFp = fopen(inputFile.c_str(), "rb");
    if (!inputFp)
    {
        fprintf(stderr, "Error: Could not open input file\n");
        return -1;
    }

    std::filesystem::path outputDir = inputFile.filename().replace_extension();
    std::filesystem::path outputDirVoice = outputDir.string() + "/voice";
    std::filesystem::create_directory(outputDir);
    std::filesystem::create_directory(outputDirVoice);

    SourceGameContext context = SourceGameContext(outputDir, outputDirVoice);
    if (!context.init())
        return -1;

    bool error = DemoReader::ProcessDem(inputFp, &context);

    fclose(inputFp);

    return error;
}
