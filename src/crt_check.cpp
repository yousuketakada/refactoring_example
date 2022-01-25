#include "pch.h"

#if defined(_MSC_VER) && defined(_DEBUG)

#include <crtdbg.h>

struct check_for_memory_leaks
{
    check_for_memory_leaks()
    {
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_EVERY_1024_DF);
    }
    ~check_for_memory_leaks()
    {
        if (_CrtDumpMemoryLeaks()) { std::abort(); }
    }
} g_check_for_memory_leaks;

#endif
