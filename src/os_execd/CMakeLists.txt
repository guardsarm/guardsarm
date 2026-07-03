cmake_minimum_required(VERSION 3.12.4)

project(guardsarm-execd C)

if(NOT DEFINED GUARDSARM_MAIN_PROJECT)
  get_filename_component(SRC_FOLDER ${CMAKE_SOURCE_DIR}/../ ABSOLUTE)
  list(APPEND CMAKE_MODULE_PATH "${SRC_FOLDER}/cmake")
  include(GuardSarmRpath)
endif()

# Include directories
include_directories(
  ${SRC_FOLDER}/shared/include/
  ${SRC_FOLDER}/shared/os_net/
  ${SRC_FOLDER}/shared/os_regex/
  ${SRC_FOLDER}/shared/os_xml/
  ${SRC_FOLDER}/shared/os_crypto/sha1/
  ${SRC_FOLDER}/shared/os_crypto/signature/
  ${SRC_FOLDER}/shared/include/error_messages/
  ${SRC_FOLDER}/external/cJSON/
  ${SRC_FOLDER}/external/curl/include/curl/
  ${SRC_FOLDER}/external/libpcre2/include/
  ${SRC_FOLDER}/external/libyaml/include/
  ${SRC_FOLDER}/external/bzip2/
  ${SRC_FOLDER}/external/sqlite/
  ${SRC_FOLDER}/external/openssl/include/
  ${SRC_FOLDER}/external/zlib/
  ${SRC_FOLDER}/config/include/
  ${SRC_FOLDER}/client-agent/include/
  ${SRC_FOLDER}/logcollector/include/
  ${SRC_FOLDER}/rootcheck/include/
  ${SRC_FOLDER}/active-response/include/
  ${SRC_FOLDER}/data_provider/include/
  ${SRC_FOLDER}/guardsarm_modules/include/
  ${SRC_FOLDER}/guardsarm_modules/src/
  ${SRC_FOLDER}/guardsarm_modules/src/agent_upgrade/
  ${SRC_FOLDER}/guardsarm_modules/src/task_manager/
  ${CMAKE_CURRENT_SOURCE_DIR}/include/)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Execd is agent-only and keeps the agent daemon name on all Unix targets.
set(GUARDSARM_EXECD_DAEMON_NAME "guardsarm-execd")
set(GUARDSARM_RUNTIME_GROUP "guardsarm")
add_definitions(-DARGV0="${GUARDSARM_EXECD_DAEMON_NAME}")
add_definitions(-DGROUPGLOBAL="${GUARDSARM_RUNTIME_GROUP}")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_definitions(-DWIN32=1 -DWIN_EXPORT)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  add_definitions(-DDarwin -DHIGHFIRST)
endif()

set(LIB_DIR ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR})

# Collect source files
file(GLOB EXECD_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

# Build static library (without main.c) for unit tests and Windows agent
list(REMOVE_ITEM EXECD_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c")
add_library(execd_lib STATIC ${EXECD_SRC})
set_target_properties(execd_lib PROPERTIES LINKER_LANGUAGE C)

# Enable GUARDSARM_UNIT_TESTING to export static functions for unit tests
if(UNIT_TEST)
  target_compile_definitions(execd_lib PRIVATE GUARDSARM_UNIT_TESTING)
  target_compile_definitions(execd_lib PRIVATE _FORTIFY_SOURCE=0)
  target_compile_options(execd_lib PRIVATE -O0 -g)
endif()

target_link_libraries(execd_lib PUBLIC guardsarmext pthread config
                                       guardsarm_modulesd_lib guardsarm)

if(UNIX)
  target_link_libraries(execd_lib PUBLIC dl)
  if(NOT APPLE)
    target_link_libraries(execd_lib PUBLIC rt)
  endif(NOT APPLE)
endif(UNIX)

# Build Windows libraries (needed by Makefile for final win32 executables)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_library(guardsarm-execd STATIC ${EXECD_SRC})
  target_compile_definitions(guardsarm-execd PUBLIC -D_WIN32_WINNT=0x600)
  if(UNIT_TEST)
    target_compile_definitions(guardsarm-execd PRIVATE GUARDSARM_UNIT_TESTING)
  endif()
  target_link_libraries(guardsarm-execd PRIVATE guardsarmext pthread config
                                            guardsarm_modulesd_lib guardsarm)
  # Only build Unix executable when not running unit tests
elseif(NOT UNIT_TEST)
  add_executable(${GUARDSARM_EXECD_DAEMON_NAME}
                 "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c")
  set_target_properties(
    ${GUARDSARM_EXECD_DAEMON_NAME}
    PROPERTIES LINKER_LANGUAGE C RUNTIME_OUTPUT_DIRECTORY
                                 ${SRC_FOLDER}/build/bin)
  target_link_libraries(${GUARDSARM_EXECD_DAEMON_NAME} PRIVATE execd_lib)

  guardsarm_set_runtime_rpath(${GUARDSARM_EXECD_DAEMON_NAME})
endif()
