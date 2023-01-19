
#include "io/demoreader.h"
#include "game/sourcecontext.h"
#include <base/argparse.hpp>
#include <cstdio>
#include <string>
#include <cassert>
#include <filesystem>

int main(const int argc, const char* argv[])
{
    DemoReader::Init();
    SourceGameContext *context = nullptr;

    argparse::ArgumentParser program("demboyz", "1.0", argparse::default_arguments::help);
    program.add_argument("-l", "--long")
        .default_value(false)
        .implicit_value(true)
        .help("Don't skip silence.");
    program.add_argument("files")
        .help("Demo files [.dem]")
        .nargs(argparse::nargs_pattern::at_least_one);
    program.add_description("Multiple demo files are only supported for split demos, supply in correct order.");

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return -1;
    }

    bool bSkipSilence = !program.get<bool>("--long");
    std::vector<std::string> files;
    try {
        files = program.get<std::vector<std::string>>("files");
    } catch (std::logic_error& e) {
        std::cerr << "ERROR: No demo files provided!" << std::endl << std::endl;
        std::cerr << program;
        return -1;
    }

    bool error = false;
    for (unsigned int i = 0; i < files.size(); i++)
    {
        std::filesystem::path inputFile(files[i]);
        FILE* inputFp = fopen((char *)inputFile.c_str(), "rb");
        if (!inputFp)
        {
            fprintf(stderr, "ERROR: Could not open input file '%s'\n", inputFile.c_str());
            return -1;
        }

        static std::filesystem::path outputDir = inputFile.filename().replace_extension();
        static std::filesystem::path outputDirVoice = outputDir.string() + "/voice";
        if (i == 0)
        {
            std::filesystem::create_directory(outputDir);
            std::filesystem::create_directory(outputDirVoice);

            context = new SourceGameContext(outputDir.string(), outputDirVoice.string(), bSkipSilence);
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
