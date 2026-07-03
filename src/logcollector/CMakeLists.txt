cmake_minimum_required(VERSION 3.12.4)

project(guardsarm-logcollector C)

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
  ${SRC_FOLDER}/guardsarm_modules/include/
  ${SRC_FOLDER}/guardsarm_modules/src/
  ${SRC_FOLDER}/guardsarm_modules/src/agent_upgrade/
  ${SRC_FOLDER}/guardsarm_modules/src/task_manager/
  ${CMAKE_CURRENT_SOURCE_DIR}/include/
  ${CMAKE_CURRENT_SOURCE_DIR}/src/)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_definitions(-DARGV0="guardsarm-logcollector")
add_definitions(-DGROUPGLOBAL="guardsarm")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_definitions(-DWIN32=1 -DWIN_EXPORT)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  add_definitions(-DDarwin -DHIGHFIRST)
endif()

set(LIB_DIR ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR})

# Collect source files
file(GLOB LOGCOLLECTOR_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

# Build static library (without main.c) for unit tests and Windows agent
list(REMOVE_ITEM LOGCOLLECTOR_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c")
add_library(logcollector_lib STATIC ${LOGCOLLECTOR_SRC})
set_target_properties(logcollector_lib PROPERTIES LINKER_LANGUAGE C)

# Enable GUARDSARM_UNIT_TESTING to export static functions for unit tests
if(UNIT_TEST)
  target_compile_definitions(logcollector_lib PRIVATE GUARDSARM_UNIT_TESTING)
  target_compile_options(logcollector_lib PRIVATE -O0 -g)
endif()

# Enable EVENTCHANNEL_SUPPORT for Windows builds
if(EVENTCHANNEL_SUPPORT)
  target_compile_definitions(logcollector_lib PUBLIC EVENTCHANNEL_SUPPORT
                                                     _WIN32_WINNT=0x600)
endif()

target_link_libraries(logcollector_lib PUBLIC guardsarmext pthread config
                                              guardsarm_modulesd_lib guardsarm)

if(UNIX)
  target_link_libraries(logcollector_lib PUBLIC dl)
  if(NOT APPLE)
    target_link_libraries(logcollector_lib PUBLIC rt)
  endif(NOT APPLE)
endif(UNIX)

# Build Windows libraries (needed by Makefile for final win32 executables)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_library(guardsarm-logcollector STATIC ${LOGCOLLECTOR_SRC})
  add_library(guardsarm-logcollector-event STATIC ${LOGCOLLECTOR_SRC})
  target_compile_definitions(guardsarm-logcollector PUBLIC -D_WIN32_WINNT=0x600)
  target_compile_definitions(guardsarm-logcollector-event
                             PUBLIC -D_WIN32_WINNT=0x600 -DEVENTCHANNEL_SUPPORT)
  if(UNIT_TEST)
    target_compile_definitions(guardsarm-logcollector PRIVATE GUARDSARM_UNIT_TESTING)
    target_compile_definitions(guardsarm-logcollector-event
                               PRIVATE GUARDSARM_UNIT_TESTING)
  endif()
  target_link_libraries(guardsarm-logcollector PRIVATE guardsarmext pthread config
                                                   guardsarm_modulesd_lib guardsarm)
  target_link_libraries(
    guardsarm-logcollector-event PRIVATE guardsarmext pthread config guardsarm_modulesd_lib
                                     guardsarm)
  # Only build Unix executable when not running unit tests
elseif(NOT UNIT_TEST)
  add_executable(guardsarm-logcollector "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c")
  set_target_properties(
    guardsarm-logcollector PROPERTIES LINKER_LANGUAGE C RUNTIME_OUTPUT_DIRECTORY
                                                    ${SRC_FOLDER}/build/bin)
  target_link_libraries(guardsarm-logcollector PRIVATE logcollector_lib)

  guardsarm_set_runtime_rpath(guardsarm-logcollector)
endif()
