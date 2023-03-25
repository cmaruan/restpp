cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

list(APPEND UNITTESTS
    restpp_tests.cpp
)

find_package(doctest REQUIRED)

foreach(CURRENT_TEST IN LISTS UNITTESTS)
    get_filename_component(TEST_NAME ${CURRENT_TEST} NAME_WE)
    add_executable(${TEST_NAME} ${CURRENT_TEST})
    target_link_libraries(${TEST_NAME} doctest::doctest restpp)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()