set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

if (MSVC)
    add_compile_options(/FC)
    add_compile_options(/utf-8)
    add_compile_options(/W4)
    add_compile_options(/permissive-)
endif ()

# Declare dependency on GoogleTest
# It is recommended to update the commit hash often to point to the latest version.
# See https://google.github.io/googletest/quickstart-cmake.html
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/c29315dda476f195298ab8da180e564478649b9e.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
