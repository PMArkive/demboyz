#pragma once

#include "game/sourcecontext.h"
#include <cstdio>

namespace DemoReader
{
    void Init();
    void DeInit();
    bool ProcessDem(std::FILE* inputFp, SourceGameContext* context);
}
