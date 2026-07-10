# Copyright (C) 2026 GuardSarm, Inc.
#
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

# Generate wmodules library
file(GLOB_RECURSE wmodules_files
    ${SRC_FOLDER}/build/guardsarm_modules/CMakeFiles/guardsarm_modulesd_lib.dir/src/*.c.o)
list(REMOVE_ITEM wmodules_files ${SRC_FOLDER}/build/guardsarm_modules/CMakeFiles/guardsarm_modulesd_lib.dir/src/main.c.o)

add_library(WMODULES_O STATIC ${wmodules_files})

set_source_files_properties(
    ${wmodules_files}
    PROPERTIES
    EXTERNAL_OBJECT true
    GENERATED true
)

set_target_properties(
    WMODULES_O
    PROPERTIES
    LINKER_LANGUAGE C
)

target_link_libraries(WMODULES_O ${GUARDSARMLIB} ${GUARDSARMEXT} -lpthread)

#include wrappers
include(${SRC_FOLDER}/unit_tests/wrappers/guardsarm/shared/shared.cmake)

# Generate wmodules tests
list(APPEND wmodules_names "test_wmodules")
list(APPEND wmodules_flags "-Wl,--wrap,w_query_agentd")

# Compilig tests
list(LENGTH wmodules_names count)
math(EXPR count "${count} - 1")
foreach(counter RANGE ${count})
    list(GET wmodules_names ${counter} test_name)
    list(GET wmodules_flags ${counter} test_flags)

    add_executable(${test_name} ${test_name}.c)

    target_link_libraries(
        ${test_name}
        ${GUARDSARMLIB}
        ${GUARDSARMEXT}
        WMODULES_O
        ${TEST_DEPS}
    )

    if(NOT test_flags STREQUAL " ")
        target_link_libraries(
            ${test_name}
            ${test_flags}
        )
    endif()
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
