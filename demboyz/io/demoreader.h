#pragma once

#include "game/sourcecontext.h"
#include <cstdio>

namespace DemoReader
{
    bool ProcessDem(std::FILE* inputFp, SourceGameContext* context);
}
