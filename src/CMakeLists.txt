add_library(statement STATIC)
add_executable(statement_test)

add_test(NAME run_statement_test COMMAND statement_test)

target_link_libraries(statement_test statement)

target_sources(
    statement
    PUBLIC
        statement.h
    PRIVATE
        pch.h pch.cpp statement.cpp)

target_sources(
    statement_test
    PRIVATE
        pch.h pch.cpp statement_test.cpp)

target_precompile_headers(statement PRIVATE pch.h)
target_precompile_headers(statement_test REUSE_FROM statement)
