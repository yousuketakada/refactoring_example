add_library(statement STATIC)
add_executable(statement_test)

target_link_libraries(statement_test statement gtest_main)

include(GoogleTest)
gtest_discover_tests(statement_test)

target_sources(
    statement
    PUBLIC
        statement.h
    PRIVATE
        pch.h
        pch.cpp
        statement.cpp
)

target_sources(
    statement_test
    PRIVATE
        pch.h
        pch.cpp
        statement_test.cpp
)

target_precompile_headers(statement PRIVATE pch.h)
target_precompile_headers(statement_test REUSE_FROM statement)
