cmake_minimum_required(VERSION 3.12.4)

project(guardsarm-authd C)

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
  ${SRC_FOLDER}/shared/os_crypto/md5/
  ${SRC_FOLDER}/shared/os_crypto/md5_sha1_sha256/
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
  ${SRC_FOLDER}/config/include/
  ${SRC_FOLDER}/data_provider/include/
  ${SRC_FOLDER}/syscheckd/include/
  ${SRC_FOLDER}/guardsarm_modules/include/
  ${SRC_FOLDER}/guardsarm_modules/src/
  ${SRC_FOLDER}/guardsarm_modules/src/agent_upgrade/
  ${SRC_FOLDER}/guardsarm_modules/src/task_manager/
  ${SRC_FOLDER}/guardsarm_db/include/
  ${CMAKE_CURRENT_SOURCE_DIR}/include/
  ${CMAKE_CURRENT_SOURCE_DIR}/src/)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(GUARDSARM_RUNTIME_GROUP "guardsarm")
if(NOT "${TARGET}" STREQUAL "winagent" AND NOT "${TARGET}" STREQUAL "agent")
  set(GUARDSARM_RUNTIME_GROUP "guardsarm-manager")
endif()

add_definitions(-DARGV0="guardsarm-manager-authd")
add_definitions(-DGROUPGLOBAL="${GUARDSARM_RUNTIME_GROUP}")

set(LIB_DIR ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR})

# Collect source files
file(GLOB AUTHD_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

# Build static library (without main-server.c) for unit tests
list(REMOVE_ITEM AUTHD_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/main-server.c")
add_library(authd_lib STATIC ${AUTHD_SRC})
set_target_properties(authd_lib PROPERTIES LINKER_LANGUAGE C)

# Enable GUARDSARM_UNIT_TESTING to export static functions for unit tests
if(UNIT_TEST)
  target_compile_definitions(authd_lib PRIVATE GUARDSARM_UNIT_TESTING)
  target_compile_options(authd_lib PRIVATE -O0 -g)
endif()

target_link_libraries(authd_lib PUBLIC guardsarmext pthread router config
                                       guardsarm_modulesd_lib guardsarm)

if(UNIX)
  target_link_libraries(authd_lib PUBLIC dl)
  if(NOT APPLE)
    target_link_libraries(authd_lib PUBLIC rt)
  endif(NOT APPLE)
endif(UNIX)

# Only build Unix executable when not running unit tests (server-only, no
# Windows)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT UNIT_TEST)
  add_executable(guardsarm-manager-authd
                 "${CMAKE_CURRENT_SOURCE_DIR}/src/main-server.c")
  set_target_properties(
    guardsarm-manager-authd PROPERTIES LINKER_LANGUAGE C RUNTIME_OUTPUT_DIRECTORY
                                                     ${SRC_FOLDER}/build/bin)
  target_link_libraries(guardsarm-manager-authd PRIVATE authd_lib)

  guardsarm_set_runtime_rpath(guardsarm-manager-authd)
endif()
