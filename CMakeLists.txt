cmake_minimum_required(VERSION 3.10)
project(banking)

set(CMAKE_CXX_STANDARD 14)

include(FetchContent)
FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
FetchContent_MakeAvailable(googletest)

add_library(banking STATIC src/Account.cpp src/Transaction.cpp)
target_include_directories(banking PUBLIC src)

add_executable(banking_tests tests/test_account.cpp tests/test_transaction.cpp)

target_link_libraries(banking_tests PRIVATE banking gtest gmock_main)

enable_testing()
add_test(NAME BankingTests COMMAND banking_tests)

option(ENABLE_COVERAGE "Enable coverage" OFF)
if(ENABLE_COVERAGE)
    target_compile_options(banking PRIVATE -O0 -g --coverage)
    target_link_libraries(banking PRIVATE --coverage)
    target_compile_options(banking_tests PRIVATE -O0 -g --coverage)
    target_link_libraries(banking_tests PRIVATE --coverage)
endif()

