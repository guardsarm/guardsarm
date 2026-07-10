# Copyright (C) 2026 GuardSarm, Inc.
#
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

# Tests list and flags
list(APPEND tests_names "test_wmodules_scheduling")
list(APPEND tests_flags "-Wl,--wrap=time,--wrap=w_time_delay,--wrap=w_sleep_until,--wrap=_mwarn,--wrap=_minfo,--wrap=_merror -Wl,--wrap=w_query_agentd")
list(APPEND use_shared_libs 1)

list(APPEND shared_libs "wmodules_scheduling_helpers.h")
list(APPEND shared_libs "wmodules_scheduling_helpers.c")

# Generate modulesd library
file(GLOB_RECURSE modulesd_files ../../../build/guardsarm_modules/CMakeFiles/guardsarm_modulesd_lib.dir/src/*.c.o)
list(REMOVE_ITEM modulesd_files ../../../build/guardsarm_modules/CMakeFiles/guardsarm_modulesd_lib.dir/src/main.c.o)
file(GLOB_RECURSE syscollector_files ../../../build/guardsarm_modules/syscollector/CMakeFiles/syscollector.dir/src/*.cpp.o)

add_library(SCHEDULING_MODULESD_O STATIC ${modulesd_files})

set_source_files_properties(
  ${modulesd_files}
  ${syscollector_files}
  PROPERTIES
  EXTERNAL_OBJECT true
  GENERATED true
  )

set_target_properties(
  SCHEDULING_MODULESD_O
  PROPERTIES
  LINKER_LANGUAGE C
)

target_link_libraries(SCHEDULING_MODULESD_O ${GUARDSARMLIB} ${GUARDSARMEXT} -lpthread)


# Compiling tests
list(LENGTH tests_names count)
math(EXPR count "${count} - 1")
foreach(counter RANGE ${count})
    list(GET tests_names ${counter} test_name)
    list(GET tests_flags ${counter} test_flags)
    list(GET use_shared_libs ${counter} use_libs)

    if(use_libs EQUAL "1")
      add_executable(${test_name} ${test_name}.c ${shared_libs})
    else ()
      add_executable(${test_name} ${test_name}.c)
    endif()


    target_link_libraries(
        ${test_name}
        ${GUARDSARMLIB}
        ${GUARDSARMEXT}
        SCHEDULING_MODULESD_O
        -ldl
        -lcmocka
        -fprofile-arcs
        -ftest-coverage
    )
    if(NOT test_flags STREQUAL " ")
        target_link_libraries(
            ${test_name}
            ${test_flags}
        )
    endif()
    add_test(${test_name} ${test_name})
endforeach()
