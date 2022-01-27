add_library(pch OBJECT)
add_library(crt_check OBJECT)
add_library(statement STATIC)
add_executable(statement_test)

target_link_libraries(crt_check pch)
target_link_libraries(statement pch)
target_link_libraries(statement_test pch crt_check statement gtest_main)

include(GoogleTest)
gtest_discover_tests(statement_test)

target_sources(
    pch
    PUBLIC
        pch.h
    PRIVATE
        pch.cpp
)

target_sources(
    crt_check
    PRIVATE
        crt_check.cpp)

target_sources(
    statement
    PUBLIC
        statement.h
    PRIVATE
        statement.cpp
        make_statement_data.h
        make_statement_data.cpp
)

target_sources(
    statement_test
    PRIVATE
        statement_test.cpp
)

target_precompile_headers(pch PRIVATE pch.h)
target_precompile_headers(crt_check REUSE_FROM pch)
target_precompile_headers(statement REUSE_FROM pch)
target_precompile_headers(statement_test REUSE_FROM pch)
