# Find the guardsarm shared library
find_library(GUARDSARMLIB NAMES libguardsarm_test.a HINTS "${SRC_FOLDER}/build/lib")

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  find_library(GUARDSARMEXT NAMES libguardsarmext.dylib HINTS "${SRC_FOLDER}/build/lib")
else()
  find_library(GUARDSARMEXT NAMES libguardsarmext.so HINTS "${SRC_FOLDER}/build/lib")
endif()

if(NOT GUARDSARMLIB)
    message(FATAL_ERROR "libguardsarm_test.a not found in ${SRC_FOLDER}/build/lib! Aborting...")
endif()

if(NOT GUARDSARMEXT)
    message(FATAL_ERROR "libguardsarmext not found in ${SRC_FOLDER}/build/lib! Aborting...")
endif()

# Add compiling flags and set tests dependencies
link_directories("${SRC_FOLDER}/build/lib/")
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(TEST_DEPS
        -Wl,-all_load
        ${GUARDSARMLIB} ${GUARDSARMEXT}
        -lagent_metadata -lagent_sync_protocol -ldbsync -lschema_validator -lfimdb
        -Wl,-noall_load
        -lpthread -ldl -fprofile-arcs -ftest-coverage)
    add_compile_options(-ggdb -O0 -g -coverage -DTEST_AGENT -I/usr/local/include -DGUARDSARM_UNIT_TESTING)
else()
    add_compile_options(-ggdb -O0 -g -coverage -DTEST_AGENT -DENABLE_AUDIT -DINOTIFY_ENABLED -fsanitize=address -fsanitize=undefined)
    link_libraries(-fsanitize=address -fsanitize=undefined)
    set(TEST_DEPS
        -Wl,--start-group
        ${GUARDSARMLIB} ${GUARDSARMEXT}
        -lagent_metadata -lfimebpf -lagent_sync_protocol -ldbsync -lschema_validator -lfimdb
        -Wl,--end-group
        -lpthread -lcmocka -ldl -fprofile-arcs -ftest-coverage)
endif()

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  add_subdirectory(client-agent)
  add_subdirectory(logcollector)
  add_subdirectory(os_execd)
endif()

add_subdirectory(guardsarm_modules)
add_subdirectory(os_crypto)
