# Copyright (C) 2026 GuardSarm, Inc.
#
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

cmake_minimum_required(VERSION 3.10)

list(APPEND aes_tests_names "test_aes_op")
list(APPEND aes_tests_flags " ")

# Compiling tests
list(LENGTH aes_tests_names count)
math(EXPR count "${count} - 1")
foreach(counter RANGE ${count})
    list(GET aes_tests_names ${counter} test_name)
    list(GET aes_tests_flags ${counter} test_flags)
    add_executable(${test_name} ${test_name}.c)
    if(${TARGET} STREQUAL "manager")
        target_link_libraries(
            ${test_name}
            ${TEST_DEPS}
        )
    else()
        target_link_libraries(
            ${test_name}
            ${TEST_DEPS}
        )
    endif()
    if(NOT test_flags STREQUAL " ")
        target_link_libraries(
            ${test_name}
            ${test_flags}
        )
    endif()
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
