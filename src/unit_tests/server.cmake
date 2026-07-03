# Find the guardsarm shared library
find_library(
  GUARDSARMLIB
  NAMES libguardsarm_test.a
  HINTS "${SRC_FOLDER}/build/lib")
find_library(
  GUARDSARMEXT
  NAMES libguardsarmext.so
  HINTS "${SRC_FOLDER}/build/lib")
set(uname "Linux")

if(NOT GUARDSARMLIB)
  message(FATAL_ERROR "libguardsarm_test.a not found in ${SRC_FOLDER}/build/lib! Aborting...")
endif()

if(NOT GUARDSARMEXT)
  message(FATAL_ERROR "libguardsarmext not found in ${SRC_FOLDER}/build/lib! Aborting...")
endif()

# Add compiling flags
add_compile_options(
  -ggdb
  -O0
  -g
  -coverage
  -DTEST_SERVER
  -DENABLE_AUDIT
  -DINOTIFY_ENABLED
  -fsanitize=address
  -fsanitize=undefined)
link_libraries(-fsanitize=address -fsanitize=undefined)

# Set tests dependencies - use linker groups to resolve circular dependencies
link_directories("${SRC_FOLDER}/build/lib/")
set(TEST_DEPS
    -Wl,--start-group
    ${GUARDSARMLIB}
    ${GUARDSARMEXT}
    -lrouter
    -lschema_validator
    -Wl,--end-group
    -lpthread
    -ldl
    -lcmocka
    -fprofile-arcs
    -ftest-coverage)

add_subdirectory(remoted)
add_subdirectory(guardsarm_db)
add_subdirectory(os_auth)
add_subdirectory(os_crypto)
add_subdirectory(guardsarm_modules)
add_subdirectory(monitord)
