#include <gtest/gtest.h>

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

int main(int argc, char** argv)
{
#ifdef _MSC_VER
    // Suppress the MSVC assert dialog so death tests don't hang waiting for user input.
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#endif

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
