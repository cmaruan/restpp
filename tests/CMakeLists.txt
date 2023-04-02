cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

list(APPEND UNITTESTS
        request_tests.cpp
        endpoint_tests.cpp
)

find_package(doctest REQUIRED)
find_package(magic_enum REQUIRED)
find_package(nlohmann_json REQUIRED)

link_libraries(doctest::doctest magic_enum::magic_enum nlohmann_json::nlohmann_json restpp)

foreach(CURRENT_TEST IN LISTS UNITTESTS)
    get_filename_component(TEST_NAME ${CURRENT_TEST} NAME_WE)
    add_executable(${TEST_NAME} ${CURRENT_TEST})
    target_link_libraries(${TEST_NAME} )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()