# Copyright (C) 2023, Wazuh Inc.
#
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

# Generate guardsarm modules library
if(${TARGET} STREQUAL "winagent")
    file(GLOB_RECURSE wm_exec ../../../build/guardsarm_modules/CMakeFiles/guardsarm_modulesd_lib.dir/src/*.obj)
    list(REMOVE_ITEM wm_exec ../../../build/guardsarm_modules/CMakeFiles/guardsarm_modulesd_lib.dir/src/main.c.obj)
else()
    file(GLOB_RECURSE wm_exec ../../../build/guardsarm_modules/CMakeFiles/guardsarm_modulesd_lib.dir/src/*.c.o)
    list(REMOVE_ITEM wm_exec ../../../build/guardsarm_modules/CMakeFiles/guardsarm_modulesd_lib.dir/src/main.c.o)
endif()

add_library(WM_EXEC_O STATIC ${wm_exec})

set_source_files_properties(
  ${wm_exec}
  PROPERTIES
  EXTERNAL_OBJECT true
  GENERATED true
)

set_target_properties(
  WM_EXEC_O
  PROPERTIES
  LINKER_LANGUAGE C
)

target_link_libraries(WM_EXEC_O ${GUARDSARMLIB} ${GUARDSARMEXT} -lpthread)

if(${TARGET} STREQUAL "winagent")
    link_directories(${SRC_FOLDER}/build/bin)
endif(${TARGET} STREQUAL "winagent")

#include wrappers
include(${SRC_FOLDER}/unit_tests/wrappers/guardsarm/shared/shared.cmake)

# Tests list and flags
list(APPEND tests_names "test_wm_exec")
list(APPEND WM_EXEC_BASE_FLAGS "-Wl,--wrap,OSList_AddData -Wl,--wrap,OSList_DeleteThisNode \
                                -Wl,--wrap,OSList_GetFirstNode -Wl,--wrap,OSList_Destroy -Wl,--wrap,w_query_agentd ${DEBUG_OP_WRAPPERS}")

if(${TARGET} STREQUAL "winagent")
    list(APPEND tests_flags "${WM_EXEC_BASE_FLAGS} -Wl,--wrap,fim_db_init -Wl,--wrap=syscom_dispatch \
                               -Wl,--wrap=fim_db_teardown -Wl,--wrap=Start_win32_Syscheck -Wl,--wrap=is_fim_shutdown")
else()
    list(APPEND tests_flags "${WM_EXEC_BASE_FLAGS} -Wl,--wrap,fork -Wl,--wrap,kill -Wl,--wrap,exit -Wl,--wrap,sleep")
endif()

# Compiling tests
list(LENGTH tests_names count)
math(EXPR count "${count} - 1")
foreach(counter RANGE ${count})
    list(GET tests_names ${counter} test_name)
    list(GET tests_flags ${counter} test_flags)

    add_executable(${test_name} ${test_name}.c)

    target_link_libraries(
        ${test_name}
        ${GUARDSARMLIB}
        ${GUARDSARMEXT}
        WM_EXEC_O
        ${TEST_DEPS}
    )

    if(${TARGET} STREQUAL "winagent")
        target_link_libraries(${test_name} fimdb)
    endif(${TARGET} STREQUAL "winagent")

    if(NOT test_flags STREQUAL " ")
        target_link_libraries(
          ${test_name}
          ${test_flags}
        )
    endif()
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
